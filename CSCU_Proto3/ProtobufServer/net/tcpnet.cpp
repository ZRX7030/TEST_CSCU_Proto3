#include "tcpnet.h"
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <fcntl.h>

typedef enum {
	NET_DISCONNECT = 0,
	NET_CONNETING,
	NET_CONNECTED
}NET_STATE;

TcpNet::TcpNet()
{
	pthread_mutex_init(&_mutex,NULL);

	_host = "";
	_port = 0;
	_encrypt = 1;

	_state = NET_DISCONNECT;
    _sockFd = -1;

    _ctx = NULL;
	_ssl = NULL;

    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    _ctx = SSL_CTX_new(SSLv23_client_method());

	this->moveToThread(&_workThread);
	QObject::connect(&_workThread, SIGNAL(started()), this, SLOT(run()));
	_workThread.start();
}

TcpNet::~TcpNet()
{
	terminated = true;

	close();

	_workThread.quit();
	_workThread.wait();

	pthread_mutex_destroy(&_mutex);
}

void TcpNet::run()
{
    fd_set fd_read;
    int ret;

	while(!terminated){

        while(!terminated && _state <= NET_CONNETING){
			if(_state == NET_DISCONNECT){
				sleep(1);
				continue;
			}
			if(!_connect()){
				_state = NET_DISCONNECT;
				continue;
			}

			_state = NET_CONNECTED;
			break;
		}

    	struct timeval timeout = {1, 0};

		FD_ZERO(&fd_read);

		if(_sockFd > 0){
			FD_SET(_sockFd, &fd_read);
		}

		ret = select(_sockFd + 1, &fd_read, NULL, NULL, &timeout);
		switch(ret)
		{
			case 0:
				break;
			case -1:
				return;
			default:
				if(FD_ISSET(_sockFd, &fd_read))
				{
					int r;
					char buff[1024];

					pthread_mutex_lock(&_mutex);
					if(_encrypt){
						r = SSL_read(_ssl, buff, 1024);
					}else{
						r = recv(_sockFd, buff, 1024, 0);
					}
					pthread_mutex_unlock(&_mutex);

					if(r > 0){
						pthread_mutex_lock(&_mutex);
						_arBuff.append(buff, r);
						pthread_mutex_unlock(&_mutex);
						emit readyRead();
					}else{
						if(EAGAIN != errno){
							close();
							emit disconnect();
						}
					}
				}
				break;
		}
	}
}

int TcpNet::connect(const char *host, unsigned short port, int encrypt)
{
	close();

	_host = host;
	_port = port;
	_encrypt = encrypt;

	_state = NET_CONNETING;
	return 0;
}

int TcpNet::read(char *buff, int len)
{
	int r;

	if(!buff || len <= 0)
		return -1;

	if(!isConnected())
		return -1;

	pthread_mutex_lock(&_mutex);

	r = len >= _arBuff.length() ? _arBuff.length() : len;
	memcpy(buff, _arBuff.data(), r);
	_arBuff = _arBuff.right(_arBuff.length() - r);

	pthread_mutex_unlock(&_mutex);

    return r;
}

int TcpNet::write(const char *buff, int len)
{
	if(!isConnected())
		return -1;

	int w = -1;

	pthread_mutex_lock(&_mutex);

	if(_encrypt){
		w = SSL_write(_ssl, buff, len);
	}else{
		w = send(_sockFd, buff, len, 0);
	}

	pthread_mutex_unlock(&_mutex);

	return w;
}

void TcpNet::close()
{
	pthread_mutex_lock(&_mutex);

	if(_ssl){
		SSL_shutdown(_ssl);
		SSL_free(_ssl);
		_ssl = NULL;
	}
	if (_sockFd > 0) {
		::close(_sockFd);
		_sockFd = -1;
	}

	_arBuff.clear();
	_state = NET_DISCONNECT;

	pthread_mutex_unlock(&_mutex);
}

char *TcpNet::errorString(int err)
{
	if(err > 0){
    	return strerror(err);
	}else{
		return "";
	}
}


int TcpNet::connectTry()
{
	struct sockaddr_in addr;
    struct hostent *host;
	int ret;

    if((host = gethostbyname(_host.toAscii().data())) == NULL ) {
		return -1;
    }

    _sockFd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (_sockFd < 0) {
		return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr = *((struct in_addr *)host->h_addr);
    addr.sin_port = htons(_port);

	ret = ::connect(_sockFd, (struct sockaddr *)&addr, sizeof(struct sockaddr));

	if(ret == 0){
		return 1;
	}

    if (ret < 0 && errno == EINPROGRESS){
		fd_set  fd_write;
		int cnt = 0;

		while(!terminated){
			struct timeval timeout= {1, 0};
			FD_ZERO(&fd_write);
			FD_SET(_sockFd, &fd_write);

			ret = select(_sockFd + 1, NULL, &fd_write, NULL, &timeout);
			switch(ret){
				case 0:
					cnt++;
					if(cnt >= 30){
						errno = 110;
						return -1;
					}
					break;
				case -1:
					return -1;
				default:
					if(FD_ISSET(_sockFd, &fd_write)){
						int err = 0;
						socklen_t len = sizeof(err);

						if(getsockopt(_sockFd, SOL_SOCKET, SO_ERROR, (char*)&err, &len) < 0){
							return -1;
						}

						if(err == 0){
							return 1;
						}

						return -1;
					}
					break;
			}
		}
    }

	return 0;
}


int TcpNet::sslTry()
{
	fd_set  fd_read;
	int ret, ssl_err;

	_ssl = SSL_new(_ctx);
	SSL_set_fd(_ssl, _sockFd);

	ret = SSL_connect(_ssl);
	if(ret == 1)
		return 1;
	else if(ret == -1){
		ssl_err = SSL_get_error(_ssl, ret);
		if (ssl_err == SSL_ERROR_WANT_READ || ssl_err == SSL_ERROR_WANT_WRITE) {

		}else if(ssl_err == SSL_ERROR_NONE){
			return 1;
		}else{
			return -1; 
		}   
	} 

	int cnt = 0;
	while(!terminated){
		struct timeval timeout= {1, 0};

		FD_ZERO(&fd_read);
		FD_SET(_sockFd, &fd_read);

		ret = select(_sockFd + 1, &fd_read, NULL, NULL, &timeout);
		switch(ret){
			case 0:
				cnt++;
				if(cnt >= 15){
					errno = 110;
					return -1;
				}
				break;
			case -1:
				return -1;
			default:
				if(FD_ISSET(_sockFd, &fd_read)){
					ret = SSL_connect(_ssl);
					if(ret == 1){ 
						return 1;
					}else if(ret == -1){
						ssl_err = SSL_get_error(_ssl, ret);
						if (ssl_err == SSL_ERROR_WANT_READ || ssl_err == SSL_ERROR_WANT_WRITE) {
						}else if(ssl_err == SSL_ERROR_NONE){
							return 1;
						}else{
							return -1; 
						}
					}   
				}
				break;
		}
	}

	return 0;
}

bool TcpNet::_connect()
{
	int ret;
	pthread_mutex_lock(&_mutex);

	if((ret = connectTry()) <= 0){
		goto FAILED;
	}

	if(_encrypt && (ret = sslTry()) <= 0){
		goto FAILED;
	}

	pthread_mutex_unlock(&_mutex);

    sleep(1);
	emit connected();
    return true;

FAILED:
	pthread_mutex_unlock(&_mutex);

	emit error(errno);
	close();

    return false;
}

bool TcpNet::isConnected()
{
    return _state == NET_CONNECTED;
}

