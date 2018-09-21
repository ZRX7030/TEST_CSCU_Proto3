#ifndef TCPNET_H
#define TCPNET_H

#include "net.h"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <pthread.h>
#include <QThread>
#include <QMap>

class TcpNet : public Net 
{
	Q_OBJECT
public:
	TcpNet();
	~TcpNet();

	int connect(const char *host, unsigned short port, int encrypt = 1);
	int read(char *buff, int len);
	int write(const char *buff, int len);
    void close();
	char *errorString(int err);

protected:
    bool _connect(void);
	bool isConnected();
	int connectTry();
	int sslTry();

	QString _host;
	ushort _port;
	int _encrypt;

	int _state;
    SSL_CTX *_ctx;
	SSL *_ssl;

    int _sockFd;
	QByteArray _arBuff;

	QThread _workThread;
	pthread_mutex_t _mutex;

private slots:
	void run();
};


#endif
