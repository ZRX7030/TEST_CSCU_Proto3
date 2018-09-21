#ifndef			__SOCKET_H__
#define			__SOCKET_H__

#include <QString>
#include <QTimer>

#include <QThread>
#include <QMetaType> 
#include <QVariant>

#include "Common.h"


class Sockets : public QThread
{
	Q_OBJECT
private:
    int port;
    char host[30];
    int socketFd;
    bool runStatus;
    bool reconnectStatus;
    bool connectedStatus;
	
	unsigned char buff[2048];		//接收数据缓冲区
public:
	
    Sockets(char *host, int port, QObject *parent=0);
    ~Sockets();

	void run();
    void startRun();
    void stopRun();
	
    int connectServer(void);
    bool getConnectedStatus(void);

signals:
    void receiveDatas(QVariant);
    void socketConnected(void);
    void socketClosed(void);

public slots:
    int sendDatas(unsigned char *data, int len);
	void reconnect(void);
};



#endif
