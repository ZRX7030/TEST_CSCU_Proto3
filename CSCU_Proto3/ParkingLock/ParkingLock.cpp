#include "ParkingLock/ParkingLock.h"

ParkingLock::ParkingLock()
{
    pRealDataFilter = RealDataFilter::GetInstance();
    pParamSet = ParamSet::GetInstance();
    pDevCache = DevCache::GetInstance(); 
    pLog = Log::GetInstance();
    bWorkFlag = FALSE;
}
ParkingLock::~ParkingLock()
{
    if(bWorkFlag == TRUE)
    {
        StopModule();
    }
}

void ParkingLock::ProcStartWork()
{
    Init();
    pMinutesTimer =new QTimer();
    connect(pMinutesTimer,SIGNAL(timeout()),this,SLOT(ProcMinutesTimeOut()));
    pMinutesTimer->start(3*60000);//定时发送数据
}

//初始化
void ParkingLock::Init()
{
    pCan1Bus = new cCan1Bus("can1",100000,pLog);//CAN总线类
    pParkingLockProtocol = new cParkingLockProtocol();
    pParkingLockProtocol->connect(pCan1Bus,SIGNAL(sigParseData(QList<can_frame*>*)),pParkingLockProtocol,SLOT(ProcParseData(QList<can_frame*>*)));
    connect(pParkingLockProtocol,SIGNAL(sigSendCanData(can_frame*)),pCan1Bus,SLOT(ProcSendData(can_frame*)));
    connect(pParkingLockProtocol,SIGNAL(sigSendToCenter(unsigned int,InfoMap)),this,SLOT(ProcRecvProtocolData(unsigned int,InfoMap)));
}

//接收协议解析后的数据
void ParkingLock::ProcRecvProtocolData(unsigned int uiInfoAddr , InfoMap TerminalDataMap)
{
    if(bWorkFlag == FALSE)
    {
        return;
    }
    switch(uiInfoAddr)
    {
    case AddrType_TermCarLock:   //发送车位锁状态
        CarLockStatusDataUpdate(TerminalDataMap, AddrType_TermCarLock,CarLockStatusCacheList);
        break;
    case AddrType_CarLock_Result:  //发送升降锁控制指令响应
        for(int i = 0;i < CmdCacheList.count();i++)
        {
            if(CmdCacheList[i].ParkingLockAddress == TerminalDataMap[Addr_CarLockID].at(0))
            {
                if(CmdCacheList[i].RecvOrderMap.contains(Addr_CarLockCmd))
                {
                    TerminalDataMap.insert(Addr_CarLockCmd,QByteArray( 1,CmdCacheList[i].RecvOrderMap[Addr_CarLockCmd].at(0)));
                }
                else{

                }
                emit sigToBus(TerminalDataMap, AddrType_CarLock_Result);
                CmdCacheList.removeAt(i);
            }
        }
        break;
    default:
        break;
    }
}

//更新车位锁状态数据
void ParkingLock::CarLockStatusDataUpdate(InfoMap TerminalDataMap, InfoAddrType enAddrType,QList <stRecvStatusCache> &CarLockStatusCacheList)
{
    unsigned char ucParkingLockAddr;
    bool HasCarLockAddr=FALSE;
    QByteArray Value;
    stRecvStatusCache recvCache,queryCache;
    if(TerminalDataMap.isEmpty())
    {
        return;
    }
    Value = TerminalDataMap.value(Addr_CarLockID);
    if(Value.size())
    {
        ucParkingLockAddr = *((unsigned char *)Value.data());
    }
    else
    {
        return;
    }
    if(enAddrType == AddrType_TermCarLock)
    {
        recvCache.ucParkingLockAddr = ucParkingLockAddr;
        recvCache.bActive=TRUE;
        recvCache.dt_RecvStatusTime =QDateTime::currentDateTime();
        recvCache.StatusInfoMap = TerminalDataMap;

        if(CarLockStatusCacheList.isEmpty())
        {
            CarLockStatusCacheList.append(recvCache);
            emit sigToBus(TerminalDataMap,AddrType_TermCarLock);
        }
        else
        {
            for(int i = 0; i < CarLockStatusCacheList.count() ; i++)
            {
                if(recvCache.ucParkingLockAddr == CarLockStatusCacheList[i].ucParkingLockAddr)
                {
                    HasCarLockAddr = TRUE;
                    queryCache = CarLockStatusCacheList[i];
                    if(recvCache.StatusInfoMap != queryCache.StatusInfoMap)
                    {
                        emit sigToBus(TerminalDataMap,AddrType_TermCarLock);
                    }
                    CarLockStatusCacheList[i].StatusInfoMap = recvCache.StatusInfoMap;
                    CarLockStatusCacheList[i].bActive=recvCache.bActive;
                    CarLockStatusCacheList[i].dt_RecvStatusTime = recvCache.dt_RecvStatusTime;
                }
            }
            if(!HasCarLockAddr)
            {
                CarLockStatusCacheList.append(recvCache);
                emit sigToBus(TerminalDataMap,AddrType_TermCarLock);
            }
        }
    }
    else
    {
        return;
    }
}


//接收总线数据
void ParkingLock::slotFromBus(InfoMap RecvCenterDataMap,  InfoAddrType enAddrType)
{
    if(bWorkFlag == FALSE)
    {
        return;
    }

    if(!RecvCenterDataMap.contains(Addr_CarLockID))
    {
        return;
    }
    unsigned char ucParkingLockAddr = 0;
    InfoMap::iterator itTarget;

    itTarget = RecvCenterDataMap.find(Addr_CarLockID); //确定车位锁地址
    ucParkingLockAddr = itTarget.value().at(0);
    RecvCenterDataMap.remove(Addr_CarLockID);
   InsertCmdCache(RecvCenterDataMap,enAddrType,ucParkingLockAddr,CmdCacheList);
  // for(int ucCount = 0;ucCount < CmdCacheList.count();ucCount++)
  // {
  //     pParkingLockProtocol->ParseCenterData(CmdCacheList[ucCount].RecvOrderMap, enAddrType, ucParkingLockAddr);//车位锁协议类处理总线数据
  // }
   // pParkingLockProtocol->ParseCenterData(RecvCenterDataMap, enAddrType, ucParkingLockAddr);//车位锁协议类处理总线数据
}

//将控制指令暂存
void ParkingLock::InsertCmdCache(InfoMap CenterMap,InfoAddrType enAddrType,unsigned char ucParkingLockAddr,QList <stRecvCmdCache> &CmdCacheList)
{
    bool bFlag=TRUE;
    stRecvCmdCache recvCache;
    recvCache.dt_RecvDataTime=QDateTime::currentDateTime();//指令接收时间
    recvCache.ParkingLockAddress=ucParkingLockAddr;
    recvCache.ucType=0;
    recvCache.RecvOrderMap=CenterMap;

    if(CmdCacheList.isEmpty())
    {
        CmdCacheList.append(recvCache);
        pParkingLockProtocol->ParseCenterData(CenterMap, enAddrType, ucParkingLockAddr);//车位锁协议类处理总线数据
    }
    else
    {
        for(int i = CmdCacheList.count()-1;i >= 0;i--)
        {
            QDateTime nowtime=QDateTime::currentDateTime();
            if(CmdCacheList[i].dt_RecvDataTime.secsTo(nowtime) >=5)
            {
                CmdCacheList.removeAt(i);
                continue;
            }
            if(CmdCacheList[i].ParkingLockAddress==recvCache.ParkingLockAddress)
            {
                bFlag =FALSE;
                break;
            }
            else
            {
                bFlag =TRUE;
            }
        }
        if(bFlag)
        {
            CmdCacheList.append(recvCache);
            pParkingLockProtocol->ParseCenterData(CenterMap, enAddrType, ucParkingLockAddr);//车位锁协议类处理总线数据
        }
    }
    return;
}

//定时处理函数
void ParkingLock::ProcMinutesTimeOut()
{
    if(bWorkFlag == FALSE)
    {
        return;
    }
    for(int i=0;i<CarLockStatusCacheList.count();i++)
    {
        QDateTime nowtime=QDateTime::currentDateTime();
        if(CarLockStatusCacheList[i].dt_RecvStatusTime.secsTo(nowtime) >=480)
        {
            CarLockStatusCacheList[i].bActive=FALSE;
        }
        if(CarLockStatusCacheList[i].bActive)
        {
            emit sigToBus(CarLockStatusCacheList[i].StatusInfoMap,AddrType_TermCarLock);
        }
    }
    return;
}

//初始化模块
int  ParkingLock::InitModule (QThread *pThread)
{
    m_pWorkThread = pThread;
    this->moveToThread (m_pWorkThread);
    QObject::connect (m_pWorkThread,SIGNAL(started()),this,SLOT(ProcStartWork()));

    return  0;
}

//注册车位锁模块到总线
 int ParkingLock::RegistModule()
 {
	 QList<int> list;

	 //----------------------------订阅主题------------------------------//
	 list.append (AddrType_CarLock_Apply);    //车位锁开关主题
	 list.append(AddrType_CarLock_ParamSet);//车位锁参数设置主题

	 CBus::GetInstance()->RegistDev (this,list);    //模块标志号-车位锁

	 return 0;
 }

 //启动模块
  int ParkingLock::StartModule ()
  {
      m_pWorkThread->start ();
      bWorkFlag = TRUE;
      return 0;
  }

  //停止模块
int ParkingLock::StopModule ()
{
    if(bWorkFlag == TRUE)
    {
        bWorkFlag = FALSE;
        delete pCan1Bus;
        delete pParkingLockProtocol;
    }
    bWorkFlag = FALSE;
    return 0;
}

//模块的状态
int ParkingLock::ModuleStatus ()
{
    return 0;
}
///SO库调用实现函数, 创建新实例返回
/// \brief CreateDevInstance
/// \param argc
/// \param pDepends
/// \return
///
CModuleIO* CreateDevInstance()
{
    return new ParkingLock();
}

///实例销毁
/// \brief DestroyDevInstance
/// \param pModule
///
void DestroyDevInstance(CModuleIO* pModule)
{
    if(pModule)
    {
        delete pModule;
    }
}
