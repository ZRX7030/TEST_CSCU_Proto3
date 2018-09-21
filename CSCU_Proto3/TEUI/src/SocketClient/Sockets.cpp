#include <stdio.h>  
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <fcntl.h>

#include <unistd.h>
#include <iostream>

#include "Sockets.h"
#include <QDateTime>

Sockets::Sockets(char *host, int port, QObject *parent) :
	QThread(parent) 
{
	snprintf(this->host, sizeof(this->host), "%s", host);
	this->port = port;

	socketFd = -1;
	connectedStatus = false;
	runStatus = false;
	reconnectStatus = false;
}

Sockets::~Sockets()
{
	stopRun();
}

void Sockets::startRun()
{
	runStatus=true;
	this->start();
}

void Sockets::stopRun()
{
	runStatus=false;
}

/**
 *服务器重新连接
 */
void Sockets::reconnect()
{
	reconnectStatus = true;
}
/**
 *连接服务器
 */
int Sockets::connectServer()
{  
	struct sockaddr_in server_addr;
	char ip_buff[30];	
	struct hostent *hptr;

	if( (hptr = gethostbyname(this->host) ) == NULL ) {
		fprintf(stderr, "gethostbyname  fail.\n");
		return -1;
	}

	char *ip_addr = hptr->h_addr_list[0]; 
	if(ip_addr == NULL)
		return -1;

	inet_ntop(hptr->h_addrtype, ip_addr, ip_buff , sizeof(ip_buff));

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip_buff);
	server_addr.sin_port=htons(this->port);

	socketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFd < 0) {
		fprintf(stderr, "create socket error: %s\n", strerror(errno));
		return -1;  
	}  

	int ret = ::connect(socketFd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
	if (ret < 0) {  
		fprintf(stderr, "connect to %s:%d error: %s\n", ip_buff, port, strerror(errno));
		goto FAILED;
	}

	return socketFd;

FAILED:
	if (socketFd > 0) {
		close(socketFd);
		socketFd = -1;
	}

	return -1;
}  

void Sockets::run()
{
	fd_set fd_read;
	int select_ret;
	struct timeval timeout = {1,0};

	while(runStatus)
	{
		connectedStatus = false;

		if(socketFd > 0)
		{
			close(socketFd);
			printf("emit sockt close signal.\n");
			emit socketClosed();
		}

		if(connectServer() == -1)
		{
			sleep(10);
			continue;
		}

        connectedStatus = true;
        reconnectStatus = false;
		
		printf("emit sockt connected signal.\n");
        emit socketConnected();

        while(runStatus && reconnectStatus == false)
		{
			FD_ZERO(&fd_read); 
            FD_SET(socketFd, &fd_read);

			timeout.tv_sec = 1;
			timeout.tv_usec = 0;
            select_ret = select(socketFd + 1, &fd_read, NULL, NULL, &timeout);
			switch(select_ret)  
			{
				case 0:   //超时
					break;
				case -1:  //出错
					printf("socket close ,reconnect, error reason(%s).\n", strerror(errno));
					if(errno != EINTR)			//调用中断不处理
						reconnectStatus = true;
					break;
				default:  
					if(FD_ISSET(socketFd, &fd_read))
					{
						stDataBuffer data_buff;
						int len = recv(socketFd, data_buff.buff, sizeof(data_buff.buff)-1, 0);
						if(len <= 0) 
						{
							printf("socket server close.\n");
							reconnectStatus = true;
						}
						else 
						{
							QDateTime cur_time = QDateTime::currentDateTime();
                            printf("receive data (%s len =%d): ", cur_time.toString("yyyy-MM-dd hh:mm:ss").toLatin1().data(), len);
							for(int i=0; i< len; i++)
                                printf("%02x ", data_buff.buff[i]);
                            printf("\r\n");
							data_buff.len = len;
							
							QVariant var;
							var.setValue(data_buff);
							emit receiveDatas(var);
						}
					}
					break;
			}
		}
	}

    if(socketFd)
        close(socketFd);
}

bool Sockets::getConnectedStatus(void)
{
    return connectedStatus;
}
/**
 *发送数据函数
 */
int Sockets::sendDatas(unsigned char *data, int len)
{
	QDateTime cur_time = QDateTime::currentDateTime();
    printf("send data(%s len =%d): ", cur_time.toString("yyyy-MM-dd hh:mm:ss").toLatin1().data(), len);
	for(int i =0; i< len; i++)
        printf("%02x ",data[i]);
    printf("\r\n");
    if(connectedStatus)
        return ::send(socketFd, data, len, 0);
	else
	{
		printf("server not connect...................\n");
		return 0;
	}
}

