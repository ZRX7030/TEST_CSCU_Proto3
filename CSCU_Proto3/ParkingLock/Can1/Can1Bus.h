#ifndef CAN1BUS_H
#define CAN1BUS_H
#include <QObject>
#include <QThread>
#include <QString>
#include <QList>
#include <QTimer>
#include <QMutex>
#include <QDebug>
#include "Log.h"
#include "ParkingLock/Can1/Can1Socket.h"
#include "CommonFunc/commfunc.h"

class cCan1BusRecv : public QObject
{
    Q_OBJECT
public:
    bool bStopFlag;
private:
    cCan1Socket * pCan1Socket;
    QList < can_frame *> * pCan1BusRecvList;
    QThread * pThisThread;
    can_frame * pCan1FrameRecv;
    Log * pLog;
public:
    cCan1BusRecv(cCan1Socket * pCan1SocketIn,QList < can_frame *> * pCan1BusRecvListIn,Log * pLogIn);
    ~cCan1BusRecv();
private:
    void RecvData();
	QString _strLogName;

public slots:
    void ProcStart();
};

class cCan1BusSend : public QObject
{
    Q_OBJECT
public:
    bool bStopFlag;
private:
    cCan1Socket * pCan1Socket;
    Log * pLog;
	QString _strLogName;
public:
    cCan1BusSend(cCan1Socket * pCan1SocketIn, Log * pLogIn);
    ~cCan1BusSend();
    void SendData(can_frame *frame);
};


class cCan1Bus : public QObject
{
    Q_OBJECT

private:
    QList <can_frame *> * pCan1BusRecvList;
    cCan1Socket * pCan1Socket;
    QThread objThreadCan1Recv;
    cCan1BusSend * pCan1BusSend;
    cCan1BusRecv * pCan1BusRecv;
    QTimer * pMsecTimer;
    Log * pLog;
    char *pCanName;//can设备名称
    int iBitRate;//波特率
public:
    cCan1Bus( char *CanName,int BitRate,Log * pLogIn);
    ~cCan1Bus();
private:
    void CheckRecvList();
signals:
    void sigParseData(QList <can_frame *> *pTerminalRecvList);
public slots:
    void ProcSendData(can_frame *pCanFrame);
private slots:
    void ProcMsecTimeOut();
};

#endif // CAN1BUS_H
