#ifndef NET_H
#define NET_H

#include <QObject>

class Net : public QObject
{
	Q_OBJECT
public:
	virtual ~Net();

	virtual int connect(const char *host, unsigned short port, int encrypt = 1) = 0;
	virtual int read(char *buff, int len) = 0;
	virtual int write(const char *buff, int len) = 0;
    virtual void close() = 0;
	virtual char *errorString(int err = 0);
	virtual void destroy();

signals:
	void connected();
	void disconnect();
	void error(int err);
	void readyRead();

protected:
	Net();

	bool terminated;
	int encryptNet;
};

#endif
