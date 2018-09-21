#include "ammeterData.h"

/*
 *现状: 目前，平台界面不支持电表功能类型下发，只在通信层默认下发类型3(远程抄表类型)
 *集控对应策略：考虑到现场很可能只有一块表，集控默认支持远程抄表功能
 *　　　　　　　对于进线侧及功率监测功能两种实时运行功能，为互斥选项，只能选其一
 *　　　　　　　当进行远程抄表时(电表类型为3)，暂停功率监测或进线侧抄表
 */

AmmeterData::AmmeterData()
{
    pLog_Ammeterdlt64507 = Log::GetInstance();
    AmmeterInfoList = new stAllAmmeterConfig;

    pAmmeterLog = Log::GetInstance();
    pParamSet = ParamSet::GetInstance();
    pRealDataFilter = RealDataFilter::GetInstance();

    cDlt645_97 = new dlt645_97(pAmmeterLog);
    cDlt645_07 = new dlt645_07(pAmmeterLog);
    cModbus = new modbus(pAmmeterLog);

    readInlineAmmeterFlag = true;
    powerMonitorFlag = false;
    readRemoteAmmeterFlag = false;

    connect(cDlt645_97,SIGNAL(sendAmmeterData_645_97(InfoMap)),this,SLOT(slotParseAmmeterData(InfoMap)));
    connect(cDlt645_07,SIGNAL(sendAmmeterData_645_07(InfoMap)),this,SLOT(slotParseAmmeterData(InfoMap)));
    connect(cModbus,SIGNAL(sendAmmeterData_modbus(InfoMap)),this,SLOT(slotParseAmmeterData(InfoMap)));


    connect(cDlt645_97,SIGNAL(sigReadOver_645_97(int)),this,SLOT(slotParseReadResult(int)));
    connect(cDlt645_07,SIGNAL(sigReadOver_645_07(int)),this,SLOT(slotParseReadResult(int)));
    connect(cModbus,SIGNAL(sigReadOver_modbus(int)),this,SLOT(slotParseReadResult(int)));

    connect(cDlt645_07,SIGNAL(sig_readFail_645_07()),this,SLOT(slotReadFail()));
    connect(cDlt645_07,SIGNAL(sig_readSucess_645_07()),this,SLOT(slotReadSuccess()));
    connect(cDlt645_07,SIGNAL(sig_sendToBusRemoteAmmeterData_645_07(InfoMap)),this,SLOT(slotsendToBusRemoteAmmeterData(InfoMap)));

    connect(this,SIGNAL(sig_readRemoteAmmeter_dlt645_07(unsigned char *,int,unsigned char *,stAmmeterConfig)),cDlt645_07,SLOT(slot_readRemoteAmmeter_dlt645_07(unsigned char *,int,unsigned char *,stAmmeterConfig)),Qt::DirectConnection);
    connect(this,SIGNAL(sig_readAmmeter_dlt645_07(QList<stAmmeterConfig>)),cDlt645_07,SLOT(slot_readAmmeter_dlt645_07(QList<stAmmeterConfig>)),Qt::DirectConnection);
    connect(this,SIGNAL(sig_readAmmeter_dlt645_97(QList<stAmmeterConfig>)),cDlt645_97,SLOT(slot_readAmmeter_dlt645_97(QList<stAmmeterConfig>)),Qt::DirectConnection);
    connect(this,SIGNAL(sig_readAmmeter_modbus(QList<stAmmeterConfig>)),cModbus,SLOT(slot_readAmmeter_modbus(QList<stAmmeterConfig>)),Qt::DirectConnection);

    connect(this,SIGNAL(sig_readPowerMonitorAmmeter(stAmmeterConfig)),cDlt645_07,SLOT(slot_readPowerMonitorAmmeter(stAmmeterConfig)),Qt::DirectConnection);
//    connect(this,SIGNAL(sig_stopRead(bool)),cDlt645_07,SLOT(slot_stopRead(bool)),Qt::DirectConnection);
    connect(this,SIGNAL(sig_sendBoardType(int)),cDlt645_07,SLOT(slot_getBoardType(int)),Qt::DirectConnection);
    connect(this,SIGNAL(sig_sendBoardType(int)),cDlt645_97,SLOT(slot_getBoardType(int)),Qt::DirectConnection);
    connect(this,SIGNAL(sig_sendBoardType(int)),cModbus,SLOT(slot_getBoardType(int)),Qt::DirectConnection);

}

AmmeterData::~AmmeterData()
{
    if(actionTimer->isActive()){
        actionTimer->stop();
    }
    if(actionTimer != NULL)
    {
        delete actionTimer;
        actionTimer = NULL;
    }
    delete AmmeterInfoList;
    delete cDlt645_97;
    delete cDlt645_07;
    delete cModbus;
    //delete pLog_Ammeterdlt64507;
}

void AmmeterData::slotParseReadResult(int type)
{
    switch (type)
    {
    case 1://97电表
        if(AmmeterInfoList_modbus.length()>0)
        {
            emit sig_readAmmeter_modbus(AmmeterInfoList_modbus);
        }
        else
        {
            readInlineAmmeterFlag = true;
        }
        break;
    case 2://07电表
    {
        if(AmmeterInfoList_dlt645_97.length()>0)
        {
            emit sig_readAmmeter_dlt645_97(AmmeterInfoList_dlt645_97);
        }
        else if(AmmeterInfoList_modbus.length()>0)
        {
            emit sig_readAmmeter_modbus(AmmeterInfoList_modbus);
        }
        else
        {
            readInlineAmmeterFlag = true;
        }
    }
        break;
    case 3://modbus
        readInlineAmmeterFlag = true;
        break;
    case 4://07电表-远程抄表结果
        readInlineAmmeterFlag = true;
        readRemoteAmmeterFlag = false;
//        emit sig_stopRead(readRemoteAmmeterFlag);
        break;
    }
}
//更新进线侧电表数据到内存
void AmmeterData::slotParseAmmeterData(InfoMap item)
{
    pRealDataFilter->realDataUpdate(item, AddrType_Ammeter);
}

///
/// \brief AmmeterData::slotsendToBusRemoteAmmeterData
///发送数据给BUS
void AmmeterData::slotsendToBusRemoteAmmeterData(InfoMap readRemoteAmmeterMap)
{
    emit sigToBus(readRemoteAmmeterMap,AddrType_RemoteAmmeterSendType);
}

void AmmeterData::slotReadFail()
{
    InfoMap readAmmeterMap;
    emit sigToBus(readAmmeterMap,AddrType_Ammeter_Disable);
}

void AmmeterData::slotReadSuccess()
{
    InfoMap readAmmeterMap;
    emit sigToBus(readAmmeterMap,AddrType_Ammeter_Enable);
}

int AmmeterData::StartModule()
{

    m_pWorkThread->start();
    return 0;
}
//根据配置选项初始化
int AmmeterData::InitModule( QThread* pThread)
{
    m_pWorkThread = pThread;
    this->moveToThread(m_pWorkThread);
    QObject::connect(m_pWorkThread, SIGNAL(started()), this, SLOT(ProcStartWork()));

    return 0;
}

//启动模块
void AmmeterData::ProcStartWork()
{
    init();
    actionTimer = new QTimer();
    actionTimer->start(1000);//
    connect(actionTimer,SIGNAL(timeout()), SLOT(ProcTimeOut()));
}

//注册模块到总线
int AmmeterData::RegistModule()
{
	QList<int> list;

	list.append(AddrType_ParamChange);
	list.append(AddrType_RemoteAmmeterReadType);    //远程抄表数据主题
	CBus::GetInstance()->RegistDev(this, list);

	return 0;
}
void AmmeterData::init()
{
    getAmmeterConfig(pParamSet);
}
//到配置模块获取电表配置信息
int AmmeterData::getAmmeterConfig(ParamSet * &pPara)
{

    if(pPara)
    {
        //获取电表参数
        pPara->querySetting(AmmeterInfoList,PARAM_AMMETER);

        parseAmmeterInfo();

        unParamConfig *paramConfig = new unParamConfig;
        //获取底板型号       
        pPara->querySetting(paramConfig,PARAM_CSCU_SYS);
        emit sig_sendBoardType(paramConfig->cscuSysConfig.boardType);
        if(paramConfig->powerLimitConfig.sPowerLimit_Enable)
        {
            powerMonitorFlag = true;//负荷约束功能开启
        }

        delete paramConfig;
    }
    else
    {
    }

    return 0;
}
///
/// \brief AmmeterData::parseAmmeterInfo
///从数据库读取电表信息到内存
void AmmeterData::parseAmmeterInfo()
{
    for(int i=0;i<AmmeterInfoList->ammeterConfig.length();i++)
    {
        if(AmmeterInfoList->ammeterConfig.at(i).enable)
        {//只存使能的电表
            switch(AmmeterInfoList->ammeterConfig.at(i).devType)
            {//协议类型
            case 1://645_97
                AmmeterInfoList_dlt645_97.append(AmmeterInfoList->ammeterConfig.at(i));
                break;
            case 2://645_07
                AmmeterInfoList_dlt645_07.append(AmmeterInfoList->ammeterConfig.at(i));
                break;
            case 3://modbus
                AmmeterInfoList_modbus.append(AmmeterInfoList->ammeterConfig.at(i));
                break;
            default:
                break;
            }
        }
        else
            continue;
    }
}

void AmmeterData::ProcTimeOut()
{
    //如果负荷约束功能开启,规定只有一块电表实现该功能
    //负荷约束功能一般会与远程抄表功能共用一块电表
    int remoteAmmeterID = -1;
    if(powerMonitorFlag && !readRemoteAmmeterFlag)
    {
        for(int i=0;i<AmmeterInfoList_dlt645_07.length();i++)
        {
            if(AmmeterInfoList_dlt645_07.at(i).funType == 3)
            {//负荷约束功能开启／未进行远程抄表
                remoteAmmeterID = i;
                emit sig_readPowerMonitorAmmeter(AmmeterInfoList_dlt645_07.at(i));//功率监测
                break;
            }
        }
    }
    else if(powerMonitorFlag && readRemoteAmmeterFlag)
    {//远程抄表时，负荷约束读电表直接返回失败
        slotReadFail();
    }

    if((!readInlineAmmeterFlag)  && (readRemoteAmmeterFlag))//读进线侧电表尚未完成
    {
        procRemoterReadAmmeter(qInfoMapPrivate);
        return;
    }

//读电表顺序：07/97/modbus
    if(AmmeterInfoList_dlt645_07.length()>0)
    {
        if(remoteAmmeterID != -1)
        {
            readInlineAmmeterFlag = false;
            QList<stAmmeterConfig> list = AmmeterInfoList_dlt645_07;
            list.removeAt(remoteAmmeterID);//对支持负荷约束功能电表不再抄进线侧数据
            emit sig_readAmmeter_dlt645_07(list);
        }
        else
        {
            readInlineAmmeterFlag = false;
            emit sig_readAmmeter_dlt645_07(AmmeterInfoList_dlt645_07);
        }
    }
    else if(AmmeterInfoList_dlt645_97.length()>0)
    {
        readInlineAmmeterFlag = false;
        emit sig_readAmmeter_dlt645_97(AmmeterInfoList_dlt645_97);
    }
   else if(AmmeterInfoList_modbus.length()>0)
    {
        readInlineAmmeterFlag = false;
        emit sig_readAmmeter_modbus(AmmeterInfoList_modbus);
    }
}


int AmmeterData::StopModule()
{
    return 0;
}
//模块工作状态
int AmmeterData::ModuleStatus()
{
    return 0;
}

void AmmeterData::procParamChange(InfoMap &RecvBusDataMap)
{
    InfoMap::iterator itTarget;
    for(itTarget = RecvBusDataMap.begin(); itTarget != RecvBusDataMap.end(); itTarget++)
    {
        if (itTarget.key() == Addr_Param_Change)
        {
//            if(itTarget.value().at(0) == PARAM_AMMETER)
//            {
                getAmmeterConfig(pParamSet);
                break;
//            }
        }
    }
}

//解析远程抄表
void AmmeterData::procRemoterReadAmmeter(InfoMap &qInfoMap)
{
    int readDataType = 0;
    QByteArray qByteReadingTime;
    QByteArray qByteAmmeterId;
    unsigned char readingTime[6];
    unsigned char ammeterId[6];
    memset(ammeterId,0,6);
    memset(readingTime,0,6);

    if(qInfoMap.isEmpty())
    {
        pLog_Ammeterdlt64507->getLogPoint("ammeter07")->info("ERROR! AddrType_RemoteAmmeterType Empty");
        return;
    }

    if(qInfoMap.contains(Addr_RemoteAmeterAddr_Adj)){
        qByteAmmeterId = qInfoMap[Addr_RemoteAmeterAddr_Adj];
        memcpy(ammeterId,qByteAmmeterId.data(),6);
    }
    if(qInfoMap.contains(Addr_RemoteAmeterType_Adj)){
        readDataType = qInfoMap[Addr_RemoteAmeterType_Adj].at(0);   //电表电能数据类型
    }
    if(qInfoMap.contains(Addr_RemoteAmeterReadTime_Adj)){
        qByteReadingTime = qInfoMap[Addr_RemoteAmeterReadTime_Adj];
        memcpy(readingTime,qByteReadingTime.data(),6);
    }

        ConvertDataFormat((unsigned char *)ammeterId,6);      //大小端转换

        if(AmmeterInfoList_dlt645_07.length()>0)  //查询配置文件中是否有要查询的电表
        {
             for(int i=0;i<AmmeterInfoList_dlt645_07.length();i++){
                 if(strcmp((const char *)AmmeterInfoList_dlt645_07.at(i).addr,(char *)ammeterId) == 0){   //要查询的电表在配置文件中存在
                     emit sig_readRemoteAmmeter_dlt645_07((unsigned char *)ammeterId,readDataType,(unsigned char *)readingTime,AmmeterInfoList_dlt645_07.at(i));
                 }
                 else{
                     readInlineAmmeterFlag = true;
                     pLog_Ammeterdlt64507->getLogPoint("ammeter07")->info("Not Found query ammeters address !!!!!!!!!! ");
                 }
             }
        }
}

void AmmeterData::slotFromBus(InfoMap qInfoMap, InfoAddrType InfoType)
{
    switch(InfoType)
    {
        case AddrType_ParamChange:
            procParamChange(qInfoMap);
        break;
        case AddrType_RemoteAmmeterReadType:    //远程抄表数据主题
        {
            qInfoMapPrivate = qInfoMap;
            if(!readRemoteAmmeterFlag)//未在远程抄表
            {
                readInlineAmmeterFlag = false;
                readRemoteAmmeterFlag = true;
//                emit sig_stopRead(readRemoteAmmeterFlag);//通知其他正在抄读功率或进线侧停止
//                procRemoterReadAmmeter(qInfoMap);
            }
            else
               pLog_Ammeterdlt64507->getLogPoint("ammeter07")->info("The last time read RemoteAmmeter is unfinished !!!!!!!!!! ");
        }
        break;

    default:
        break;
    }
}

//SO库调用实现函数, 创建新实例返回
CModuleIO* CreateDevInstance()
{
    return new AmmeterData();
}

//实例销毁
void DestroyDevInstance(CModuleIO* pModule)
{

    if(pModule){
        delete pModule;
    }
}
