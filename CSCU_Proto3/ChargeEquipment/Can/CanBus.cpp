#include "CanBus.h"

cCanBusRecv::cCanBusRecv(cCanSocket *pCanSocketIn, QList<can_frame *> *pCanBusRecvListIn, QMutex *pRecvMutexIn, Log *pLogIn)
{
	_strLogName = "can0";
    bStopFlag = FALSE;
    pCanSocket = pCanSocketIn;
    pCanBusRecvList = pCanBusRecvListIn;
    pRecvMutex = pRecvMutexIn;
    pLog = pLogIn;
    pThisThread = this->thread();
    pCanFrameRecv = new can_frame;
}
cCanBusRecv::~cCanBusRecv()
{
    if(pCanFrameRecv!=NULL)
    {
        delete pCanFrameRecv;
    }
}

void cCanBusRecv::RecvData()
{
    QString StrFrame;
    while(bStopFlag == FALSE)
    {
        if(pCanSocket->SocketCan_Read(pCanFrameRecv) > 0)
        {
            //写日志
            StrFrame = "Recv ";
            for(unsigned char ucCount = 0; ucCount < sizeof(pCanFrameRecv->can_id); ucCount++)
            {
                StrFrame += QString::number(((unsigned char *) (&pCanFrameRecv->can_id))[3 - ucCount], 16) + " ";
            }
            StrFrame += "|" + QString::number(pCanFrameRecv->can_dlc, 16) + "| ";
            for(unsigned char ucCount = 0; ucCount < pCanFrameRecv->can_dlc; ucCount++)
            {
                StrFrame += QString::number(pCanFrameRecv->data[ucCount], 16) + " ";
            }
            pLog->getLogPoint(_strLogName)->info(StrFrame);
			uchar c = ((unsigned char *) (&pCanFrameRecv->can_id))[2];
			//if(c == 0x30 || c == 0x32)
            //	pLog->getLogPoint(LOG_MODULE_CAN0)->info(StrFrame);
            //发送
            can_frame * pCanFrameNew = pCanFrameRecv;
            pCanFrameRecv = new can_frame;
            pRecvMutex->lock();
            pCanBusRecvList->append(pCanFrameNew);
            pRecvMutex->unlock();
        }
    }
}

void cCanBusRecv::ProcStart()
{
    RecvData();
}

cCanBusSend::cCanBusSend(cCanSocket *pCanSocketIn, QList<can_frame *> *pCanBusSendListIn, QMutex *pSendMutexIn, Log *pLogIn)
{
	_strLogName = "can0";
    pCanSocket = pCanSocketIn;
    pCanBusSendList = pCanBusSendListIn;
    pSendMutex = pSendMutexIn;
    pLog = pLogIn;
    pThisThread = this->thread();
    bStopFlag = FALSE;
}

cCanBusSend::~cCanBusSend()
{
    ;
}

void cCanBusSend::SendData()
{
    int iRet = 0;
    QString StrFrame;
    while (TRUE)
    {
        if(bStopFlag == TRUE)
        {
            break;
        }
        pSendMutex->lock();
        if(!pCanBusSendList->isEmpty())
        {
            for(int i = 0; i < pCanBusSendList->count(); i++)
            {
                //写日志
                StrFrame = "Send ";
                for(unsigned char ucCount = 0; ucCount < sizeof(pCanBusSendList->at(i)->can_id); ucCount++)
                {
                    StrFrame += QString::number(((unsigned char *) (&pCanBusSendList->at(i)->can_id))[3 - ucCount], 16) + " ";
                }
                StrFrame += "|" + QString::number(pCanBusSendList->at(i)->can_dlc, 16) + "| ";
                for(unsigned char ucCount = 0; ucCount < pCanBusSendList->at(i)->can_dlc; ucCount++)
                {
                    StrFrame += QString::number(pCanBusSendList->at(i)->data[ucCount], 16) + " ";
                }

                pLog->getLogPoint(_strLogName)->info(StrFrame);
                //临时调试添加
//                if(StrFrame.contains("Send b8 52") || StrFrame.contains("Send b8 53") || StrFrame.contains("Send b8 59") ||StrFrame.contains("Send b8 60"))
//                    pLog->getLogPoint(LOG_MODULE_EVENT)->info(StrFrame);
                //发送
                iRet = pCanSocket->SocketCan_Write(pCanBusSendList->at(i));
                usleep(10000);
                if(iRet > 0)
                {
                    delete pCanBusSendList->at(i);
                }
                else
                {
                    usleep(5000);
                    pCanSocket->SocketCan_Write(pCanBusSendList->at(i));
                    delete pCanBusSendList->at(i);
                }
            }
            pCanBusSendList->clear();
        }
        pSendMutex->unlock();
        pThisThread->wait(100);
    }
}

void cCanBusSend::ProcStart()
{
    SendData();
}

cCanBus::cCanBus(Log * pLogIn)
{
    pCanBusRecvList = new QList <can_frame *>;
    pCanBusSendList = new QList <can_frame *>;
    pSendMutex = new QMutex();
    pRecvMutex = new QMutex();
    pMsecTimer = new QTimer();
    pLog = pLogIn;
}

cCanBus::~cCanBus()
{
    pMsecTimer->stop();
    pCanBusSend->bStopFlag = TRUE;
    pCanBusRecv->bStopFlag = TRUE;

    if(objThreadCanSend.isRunning())
    {
        objThreadCanSend.quit();
    }
    if(objThreadCanRecv.isRunning())
    {
        objThreadCanRecv.quit();
    }
    pMsecTimer->stop();
    pCanBusSend->bStopFlag = TRUE;
    pCanBusRecv->bStopFlag = TRUE;
    objThreadCanSend.wait();
    objThreadCanRecv.wait();
//    sleep(1);

    delete pCanBusSend;
    delete pCanBusRecv;
    delete pCanSocket;
    pRecvMutex->lock();
    for(int i = 0; i < pCanBusRecvList->count(); i++)
    {
        delete pCanBusRecvList->at(i);
    }
    delete pCanBusRecvList;
    pRecvMutex->unlock();
    pSendMutex->lock();
    for(int i = 0; i < pCanBusSendList->count(); i++)
    {
        delete pCanBusSendList->at(i);
    }
    delete pCanBusSendList;
    pSendMutex->unlock();
    delete pMsecTimer;
    delete pRecvMutex;
    delete pSendMutex;
}

void cCanBus::Init()
{
    pCanSocket = new cCanSocket();
    pCanSocket->SocketCan_Init("can0",100000);

    pCanBusSend = new cCanBusSend(pCanSocket,pCanBusSendList,pSendMutex, pLog);
    pCanBusSend->moveToThread(&objThreadCanSend);
    this->connect(&objThreadCanSend,SIGNAL(started()),pCanBusSend,SLOT(ProcStart()));

    pCanBusRecv = new cCanBusRecv(pCanSocket,pCanBusRecvList,pRecvMutex, pLog);
    pCanBusRecv->moveToThread(&objThreadCanRecv);
    this->connect(&objThreadCanRecv,SIGNAL(started()),pCanBusRecv,SLOT(ProcStart()));

    objThreadCanSend.start();
    objThreadCanRecv.start();

    connect(pMsecTimer, SIGNAL(timeout()), SLOT(ProcMsecTimeOut()));
    //50ms定时器
    pMsecTimer->start(50);
}

void cCanBus::CheckRecvList()
{
    bool bRecvFlag = FALSE;
    pRecvMutex->lock();
    if(!pCanBusRecvList->isEmpty())
    {
        bRecvFlag = TRUE;
    }
    pRecvMutex->unlock();
    //如果接收列表不为空, 则发送解析CAN总线数据信号
    if(bRecvFlag)
    {
        emit sigParseData(pCanBusRecvList, pRecvMutex);
    }
}

void cCanBus::ProcStartWork()
{
    Init();
}

//将协议生成的帧列表拷贝到CAN发送列表, 并清空协议帧列表
void cCanBus::ProcSendData(QList <can_frame *> *pTerminalSendList, QMutex * pSendListMutex)
{
    pSendMutex->lock();
    pSendListMutex->lock();
    if(!pTerminalSendList->isEmpty())
    {
        for(int i = 0; i < pTerminalSendList->count(); i++)
        {
            pCanBusSendList->append(pTerminalSendList->at(i));
        }
        pTerminalSendList->clear();
    }
    pSendListMutex->unlock();
    pSendMutex->unlock();
}

void cCanBus::ProcMsecTimeOut()
{
    CheckRecvList();
}
