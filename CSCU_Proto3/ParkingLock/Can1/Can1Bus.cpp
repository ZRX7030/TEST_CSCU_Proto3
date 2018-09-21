#include "Can1Bus.h"

cCan1BusRecv::cCan1BusRecv(cCan1Socket *pCan1SocketIn, QList<can_frame *> *pCan1BusRecvListIn,  Log *pLogIn)
{
	_strLogName = "can1";
    bStopFlag = FALSE;
    pCan1Socket = pCan1SocketIn;
    pCan1BusRecvList = pCan1BusRecvListIn;
    pLog = pLogIn;
    pThisThread = this->thread();
    pCan1FrameRecv = new can_frame;
}
cCan1BusRecv::~cCan1BusRecv()
{
    if(pCan1FrameRecv!=NULL)
    {
        delete pCan1FrameRecv;
    }
}

void cCan1BusRecv::RecvData()
{
    QString StrFrame;
    while(bStopFlag == FALSE)
    {
        if(pCan1Socket->SocketCan1_Read(pCan1FrameRecv) > 0)
        {
            //写日志
            StrFrame = "Recv ";
            for(unsigned char ucCount = 0; ucCount < sizeof(pCan1FrameRecv->can_id); ucCount++)
            {
                StrFrame += QString::number(((unsigned char *) (&pCan1FrameRecv->can_id))[3 - ucCount], 16) + " ";
            }
            StrFrame += "|" + QString::number(pCan1FrameRecv->can_dlc, 16) + "| ";
            for(unsigned char ucCount = 0; ucCount < pCan1FrameRecv->can_dlc; ucCount++)
            {
                StrFrame += QString::number(pCan1FrameRecv->data[ucCount], 16) + " ";
            }
            pLog->getLogPoint(_strLogName)->info(StrFrame);
            //发送
            can_frame * pCan1FrameNew = pCan1FrameRecv;
            pCan1FrameRecv = new can_frame;
            pCan1BusRecvList->append(pCan1FrameNew);
        }
    }
}

void cCan1BusRecv::ProcStart()
{
    RecvData();
}

cCan1BusSend::cCan1BusSend(cCan1Socket *pCan1SocketIn, Log *pLogIn)
{
	_strLogName = "can1";
    pCan1Socket = pCan1SocketIn;
    pLog = pLogIn;
    bStopFlag = FALSE;
}

cCan1BusSend::~cCan1BusSend()
{
    ;
}

void cCan1BusSend::SendData(can_frame *frame)
{
    int iRet = 0;
    QString StrFrame;
    //发送
    iRet = pCan1Socket->SocketCan1_Write(frame);
    usleep(10000);
    if(iRet > 0)
    {
        //写日志
        StrFrame = "Send ";
        for(unsigned char ucCount = 0; ucCount < sizeof(frame->can_id); ucCount++)
        {
            StrFrame += QString::number(((unsigned char *) (&frame->can_id))[3 - ucCount], 16) + " ";
        }
        StrFrame += "|" + QString::number(frame->can_dlc, 16) + "| ";
        for(unsigned char ucCount = 0; ucCount < frame->can_dlc; ucCount++)
        {
            StrFrame += QString::number(frame->data[ucCount], 16) + " ";
        }
        pLog->getLogPoint(_strLogName)->info(StrFrame);

        delete frame;
    }
    else
    {
        usleep(5000);
        pCan1Socket->SocketCan1_Write(frame);
        //写日志
        StrFrame = "Secv ";
        for(unsigned char ucCount = 0; ucCount < sizeof(frame->can_id); ucCount++)
        {
            StrFrame += QString::number(((unsigned char *) (&frame->can_id))[3 - ucCount], 16) + " ";
        }
        StrFrame += "|" + QString::number(frame->can_dlc, 16) + "| ";
        for(unsigned char ucCount = 0; ucCount < frame->can_dlc; ucCount++)
        {
            StrFrame += QString::number(frame->data[ucCount], 16) + " ";
        }
        pLog->getLogPoint(_strLogName)->info(StrFrame);
        delete frame;
    }
}

cCan1Bus::cCan1Bus( char *CanName,int BitRate,Log * pLogIn)
{
    pCan1BusRecvList = new QList <can_frame *>;
    pLog = pLogIn;
    pCanName = CanName;
    iBitRate = BitRate;

    pCan1Socket = new cCan1Socket();
    pCan1Socket->SocketCan1_Init(pCanName,iBitRate);

    pCan1BusSend = new cCan1BusSend(pCan1Socket, pLog);
    pCan1BusRecv = new cCan1BusRecv(pCan1Socket,pCan1BusRecvList, pLog);
    pCan1BusRecv->moveToThread(&objThreadCan1Recv);
    this->connect(&objThreadCan1Recv,SIGNAL(started()),pCan1BusRecv,SLOT(ProcStart()));

    objThreadCan1Recv.start();

    pMsecTimer = new QTimer();
    connect(pMsecTimer, SIGNAL(timeout()), this,SLOT(ProcMsecTimeOut()));
    pMsecTimer->start(500);//500ms定时器
}

cCan1Bus::~cCan1Bus()
{
    if(objThreadCan1Recv.isRunning())
    {
        objThreadCan1Recv.quit();
        objThreadCan1Recv.wait(200);
    }
    pMsecTimer->stop();
    pCan1BusSend->bStopFlag = TRUE;
    pCan1BusRecv->bStopFlag = TRUE;
    sleep(1);
    delete pCan1BusRecv;
    delete pCan1Socket;
    for(int i = 0; i < pCan1BusRecvList->count(); i++)
    {
        delete pCan1BusRecvList->at(i);
    }
    delete pCan1BusRecvList;
    delete pMsecTimer;
}

void cCan1Bus::CheckRecvList()
{
    bool bRecvFlag = FALSE;
    if(!pCan1BusRecvList->isEmpty())
    {
        bRecvFlag = TRUE;
    }
    //如果接收列表不为空, 则发送解析CAN总线数据信号
    if(bRecvFlag)
    {     
    }
}

void cCan1Bus::ProcSendData(can_frame *pCanFrame)
{
    pCan1BusSend->SendData(pCanFrame);
}

void cCan1Bus::ProcMsecTimeOut()
{
    CheckRecvList();
}
