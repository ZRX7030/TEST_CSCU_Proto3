#ifndef CANBUS_H
#define CANBUS_H
#include <QObject>
#include <QThread>
#include <QString>
#include <QList>
#include <QTimer>
#include <QMutex>
#include <QDebug>
#include "Log.h"
#include "CanSocket.h"
#include "CommonFunc/commfunc.h"

class cCanBusRecv : public QObject
{
    Q_OBJECT
public:
    bool bStopFlag;
private:
    QMutex * pRecvMutex;
    cCanSocket * pCanSocket;
    QList < can_frame *> * pCanBusRecvList;
    QThread * pThisThread;
    can_frame * pCanFrameRecv;
    Log * pLog;
public:
    cCanBusRecv(cCanSocket * pCanSocketIn,QList < can_frame *> * pCanBusRecvListIn,QMutex * pRecvMutexIn, Log * pLogIn);
    ~cCanBusRecv();
private:
    void RecvData();
	QString _strLogName;

public slots:
    void ProcStart();
};

class cCanBusSend : public QObject
{
    Q_OBJECT
public:
    bool bStopFlag;
private:
    QMutex *pSendMutex;
    cCanSocket * pCanSocket;
    QList < can_frame *> * pCanBusSendList;
    QThread * pThisThread;
    Log * pLog;
	QString _strLogName;
public:
    cCanBusSend(cCanSocket * pCanSocketIn,QList < can_frame *> * pCanBusSendListIn,QMutex *pSendMutexIn, Log * pLogIn);
    ~cCanBusSend();
    void SendData();
public slots:
    void ProcStart();
};


class cCanBus : public QObject
{
    Q_OBJECT

private:
    QList <can_frame *> * pCanBusRecvList;
    QList <can_frame *> * pCanBusSendList;
    QMutex * pRecvMutex;
    QMutex * pSendMutex;

    cCanSocket * pCanSocket;
    QThread objThreadCanRecv;
    QThread objThreadCanSend;
    cCanBusSend * pCanBusSend;
    cCanBusRecv * pCanBusRecv;
    QTimer * pMsecTimer;
    Log * pLog;
public:
    cCanBus(Log * pLogIn);
    ~cCanBus();
private:
    void Init();
    void CheckRecvList();
signals:
    void sigParseData(QList <can_frame *> *pTerminalRecvList, QMutex * pRecvListMutex);
public slots:
    void ProcStartWork();
    void ProcSendData(QList <can_frame *> *pTerminalSendList, QMutex * pSendListMutex);
private slots:
    void ProcMsecTimeOut();
};

#endif // CANBUS_H
