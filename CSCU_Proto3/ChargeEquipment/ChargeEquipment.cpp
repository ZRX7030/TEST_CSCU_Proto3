#include "ChargeEquipment.h"

int can0Addr;
cJ1939PreParse::cJ1939PreParse(QList<TermProNode *> *pTerminalProtocolListIn, Log *pLogIn)
{
    pLog = pLogIn;

    for(int i = 0; i < pTerminalProtocolListIn->count(); i++)
    {
        //关联交流单相协议
        if(pTerminalProtocolListIn->at(i)->enTermProType == Pro_J1939_ACSin)
        {
            pACSinCanPro = (cACSinCanProtocol *)pTerminalProtocolListIn->at(i)->pTerminalProtocol;
        }
        //关联交流三相协议
        else if(pTerminalProtocolListIn->at(i)->enTermProType == Pro_J1939_ACThr)
        {
            pACThrCanPro = (cACThrCanProtocol *)pTerminalProtocolListIn->at(i)->pTerminalProtocol;
        }
        //关联直流协议
        else if(pTerminalProtocolListIn->at(i)->enTermProType == Pro_J1939_DC)
        {
            pDCCanPro = (cDCCanProtocol *)pTerminalProtocolListIn->at(i)->pTerminalProtocol;
        }
        //关联能效CAN协议
        else if(pTerminalProtocolListIn->at(i)->enTermProType == Pro_J1939_EnergyPlan)
        {
            pEnergyPlanPro = (cEnergyPlanProtocol *)pTerminalProtocolListIn->at(i)->pTerminalProtocol;
        }
        //关联集控主从CAN协议
        else if(pTerminalProtocolListIn->at(i)->enTermProType == Pro_J1939_CSCU)
        {
            pCSCUCanPro = (cCSCUCanProtocol *)pTerminalProtocolListIn->at(i)->pTerminalProtocol;
        }
    }
}

void cJ1939PreParse::ProcParseData(QList <can_frame *> *pTerminalRecvList, QMutex * pRecvListMutex)
{
    FrameHead strFrameHead;
    pRecvListMutex->lock();
    if(!pTerminalRecvList->isEmpty())
    {
        for(int i = 0; i < pTerminalRecvList->count(); i++)
        {
            memcpy((unsigned char *)&strFrameHead, (unsigned char *) &pTerminalRecvList->at(i)->can_id, sizeof(strFrameHead));
//            if(strFrameHead.ucSa==99)
            //交流单相终端
            if((strFrameHead.ucSa >= ID_MinACSinCanID) &&(strFrameHead.ucSa <= ID_MaxACSinCanID))
            {
                pACSinCanPro->ParseCanData(pTerminalRecvList->at(i));
            }
            //EMS
            else if(strFrameHead.ucSa == ID_EMSCanID)
            {
                pEnergyPlanPro->ParseCanData(pTerminalRecvList->at(i));
            }
            //交流负载总配电柜
            else if((strFrameHead.ucSa >= ID_MinACLoadDistributionCabinet) &&(strFrameHead.ucSa <= ID_MaxACLoadDistributionCabinet))
            {
                pEnergyPlanPro->ParseCanData(pTerminalRecvList->at(i));
            }
            //系统配电柜
            else if((strFrameHead.ucSa >= ID_MinSystemDistributionCabinet) &&(strFrameHead.ucSa <= ID_MaxSystemDistributionCabinet))
            {
                pEnergyPlanPro->ParseCanData(pTerminalRecvList->at(i));
            }
            //直流光伏控制柜
            else if((strFrameHead.ucSa >= ID_MinDCPhotovoltaicControlCabinet) &&(strFrameHead.ucSa <= ID_MaxDCPhotovoltaicControlCabinet))
            {
                pEnergyPlanPro->ParseCanData(pTerminalRecvList->at(i));
            }
            //四象限变换柜
            else if((strFrameHead.ucSa >= ID_MinFourQuadrantConverterCabinet) &&(strFrameHead.ucSa <= ID_MaxFourQuadrantConverterCabinet))
            {
                pEnergyPlanPro->ParseCanData(pTerminalRecvList->at(i));
            }
            //独立逆变柜
            else if((strFrameHead.ucSa >= ID_MinIndependentInverterCabinet) &&(strFrameHead.ucSa <= ID_MaxIndependentInverterCabinet))
            {
                pEnergyPlanPro->ParseCanData(pTerminalRecvList->at(i));
            }
            //直流充放电柜
            else if((strFrameHead.ucSa >= ID_MinDCChargeDischargeCabinet) &&(strFrameHead.ucSa <= ID_MaxDCChargeDischargeCabinet))
            {
                pEnergyPlanPro->ParseCanData(pTerminalRecvList->at(i));
            }
            //直流储能柜
            else if((strFrameHead.ucSa >= ID_MinDCEnergyStorageCabinet) &&(strFrameHead.ucSa <= ID_MaxDCEnergyStorageCabinet))
            {
                pEnergyPlanPro->ParseCanData(pTerminalRecvList->at(i));
            }
            //交流三相终端
            else if((strFrameHead.ucSa >= ID_MinACThrCanID) &&(strFrameHead.ucSa <= ID_MaxACThrCanID) && (strFrameHead.ucPs == 250))
            {
                pACThrCanPro->ParseCanData(pTerminalRecvList->at(i));
            }
            //直流终端
            else if((strFrameHead.ucSa >= ID_MinDCCanID) &&(strFrameHead.ucSa <= ID_MaxDCCanID))
            {
                pDCCanPro->ParseCanData(pTerminalRecvList->at(i));
            }
            //CCU
            else if((strFrameHead.ucSa >= ID_MinCCUCanID) &&(strFrameHead.ucSa <= ID_MaxCCUCanID))
            {
                pDCCanPro->ParseCanData(pTerminalRecvList->at(i));
            }
            //副集控
            else if((strFrameHead.ucSa > ID_DefaultControlCenterCanID) &&(strFrameHead.ucSa <= ID_MaxControlCenterCanID))
            {
                pCSCUCanPro->ParseCanData(pTerminalRecvList->at(i));
            }

            delete pTerminalRecvList->at(i);
        }
        pTerminalRecvList->clear();
    }
    pRecvListMutex->unlock();
}

cChargeEquipment::cChargeEquipment()
{
    pRealDataFilter = RealDataFilter::GetInstance();
    pParamSet = ParamSet::GetInstance();
    pLog = Log::GetInstance();
    pDevCache = DevCache::GetInstance();

    bWorkFlag = FALSE;

    gpParamSet = ParamSet::GetInstance();
    gpParamSet->querySetting(&can0Config, PARAM_CAN0);
    can0Addr = can0Config.canAddr;
}

cChargeEquipment::~cChargeEquipment()
{
    if(bWorkFlag == TRUE)
    {
        StopModule();
    }
}

//根据配置选项初始化
int cChargeEquipment::InitModule( QThread* pThread)
{
    m_pWorkThread = pThread;

    this->moveToThread(m_pWorkThread);
    QObject::connect(m_pWorkThread, SIGNAL(started()), this, SLOT(ProcStartWork()));

    return 0;
}

//启动模块
void cChargeEquipment::ProcStartWork()
{
    stCSCUSysConfig stTempCCUsysConfig;

    Init();
    pOneSecTimer = new QTimer();
    pOneSecTimer->start(100);
    connect(pOneSecTimer,SIGNAL(timeout()), SLOT(ProcOneSecTimeOut()));

    pParamSet->querySetting(&stTempCCUsysConfig, PARAM_CSCU_SYS);
    pTerminalProtocolList->at(0)->pTerminalProtocol->ucACsingle = stTempCCUsysConfig.singlePhase;
    pTerminalProtocolList->at(1)->pTerminalProtocol->ucACthree = stTempCCUsysConfig.threePhase;
}

//注册设备到总线
int cChargeEquipment::RegistModule()
{
	QList<int> List;
	List.append(AddrType_CmdCtrl);//遥控_充电控制
	List.append(AddrType_TermAdjustment);//遥调_充电设备
	List.append(AddrType_LimitChargeCurrent);//错峰充电限制充电电流。负荷调度模块发布，充电设备模块订阅
	List.append(AddrType_ApplyAccountInfoResult_ToChargeEquipment);  //主题七：充电服务返回账户信息给充电设备
	List.append(AddrType_InApplyStartChargeResult_ToChargeEquipment);//主题十一：内部申请开始充电结果至充电设备
	List.append(AddrType_InApplyStopChargeResult_ToChargeEquipment);//主题十九：内部申请结束充电结果至充电设备
	List.append(AddrType_OutApplyStartChargeResult_ToChargeEquipment);//主题十五：远程申请开始充电结果至充电设备
	List.append(AddrType_OutApplyStopChargeResult_ToChargeEquipment);//主题二十三：远程申请结束充电结果至充电设备
	List.append(AddrType_UpdatePackDir_Dev);   //CAN设备升级包路径, 发布: 升级模块, 订阅, 充电设备

	List.append(AddrType_DetailParamSet_ACSin);   //交流单相详细参数设置, 设备管理模块发布, 充电设备模块订阅
	List.append(AddrType_DetailParamQuery_ACSin);   //交流单相详细参数查询, 其他模块发布,充电设备模块订阅
	List.append(AddrType_DetailParamSet_ACThr);   //交流三相详细参数设置, 设备管理模块发布, 充电设备模块订阅
	List.append(AddrType_DetailParamQuery_ACThr);   //交流三相详细参数查询, 其他模块发布,充电设备模块订阅

	List.append(AddrType_ActiveProtectSet);   //主动防护功能设置, 发布: 升级模块, 订阅, 充电设备
	List.append(AddrType_FlexibleChargeSet);   //柔性充电功能设置, 发布: 升级模块, 订阅, 充电设备
	List.append(AddrType_GeneralStaticArgSet);   //通用静态参数设置, 发布: 升级模块, 订阅, 充电设备
	List.append(AddrType_GeneralDynamicArgSet);   //通用动态参数设置, 发布: 升级模块, 订阅, 充电设备
	List.append(AddrType_ActiveProtectQuery);   //主动防护功能查询, 其他模块发布,充电设备模块订阅
	List.append(AddrType_FlexibleChargeQuery);  //柔性充电功能查询, 其他模块发布,充电设备模块订阅
	List.append(AddrType_GeneralStaticArgQuery);    //通用静态参数设置查询, 其他模块发布, 充电设备模块订阅(未用)
	List.append(AddrType_GeneralDynamicArgQuery);   //通用动态参数设置查询, 其他模块发布, 充电设备模块订阅(未用)

	List.append(AddrType_CCUArgSet);   //CCU参数设置, 设备管理模块发布, 充电设备模块订阅
	List.append(AddrType_CCUArgQuery);  //CCU参数设置查询, 其他模块发布, 充电设备模块订阅
	List.append(AddrType_MaxLoad);//终端最大功率
	List.append(AddrType_CmdCtrl_Apply);

	List.append(AddrType_Response_Result_IN);  //双抢模式结果, 其他模块发布, 充电设备模块订阅
	List.append(AddrType_ChargeGunGroup_Info_IN);  //多枪分组下发, 充电服务发布, 充电设备模块订阅
    List.append(AddrType_DoubleSys300kwSetting_Publish);

	CBus::GetInstance()->RegistDev(this, List);

    return 0;
}

//启动模块
int cChargeEquipment::StartModule()
{
    m_pWorkThread->start();
    bWorkFlag = TRUE;
    return 0;
}

//停止模块
int cChargeEquipment::StopModule()
{
//    m_pWorkThread->quit();
    if(bWorkFlag == TRUE)
    {
        bWorkFlag = FALSE;

        pOneSecTimer->stop();
        pCanThread->quit();
        delete pOneSecTimer;
        delete pJ1939PreParse;
        delete pCanBus;
        delete pCanThread;
        for(int i = 0; i < pTerminalProtocolList->count(); i++)
        {
            delete pTerminalProtocolList->at(i);
        }
        pTerminalProtocolList->clear();
        delete pTerminalProtocolList;
    }
    bWorkFlag = FALSE;
    return 0;
}

//模块工作状态
int cChargeEquipment::ModuleStatus()
{
    return 0;
}

//初始化
void cChargeEquipment::Init()
{
    //CAN线程初始化
    CanThreadInit();
    //协议列表初始化
    TerminalProtocolInit();
    //绑定CAN接口和相关协议
    BindCanPro();
}

bool cChargeEquipment::BindCanPro()
{
    if(pTerminalProtocolList->isEmpty())
    {
        return FALSE;
    }

    for(int i = 0; i < pTerminalProtocolList->count(); i++)
    {
        if(pTerminalProtocolList->at(i)->enTermProType == Pro_J1939_ACSin)
        {
            //关联CAN总线和交流单相CAN协议(发送CAN帧)
            connect((cACSinCanProtocol*)(pTerminalProtocolList->at(i)->pTerminalProtocol), SIGNAL(sigSendCanData(QList<can_frame*>*,QMutex*)), pCanBus, SLOT(ProcSendData(QList<can_frame*>*,QMutex*)));
            //关联交流单相协议和充电设备类(发送总线数据)
            connect((cACSinCanProtocol*)(pTerminalProtocolList->at(i)->pTerminalProtocol), SIGNAL(sigSendToCenter(uint,InfoMap)), SLOT(ProcRecvProtocolData(uint,InfoMap)));
        }
        if(pTerminalProtocolList->at(i)->enTermProType == Pro_J1939_ACThr)
        {
            //关联CAN总线和交流三相CAN协议(发送CAN帧)
            connect((cACThrCanProtocol*)(pTerminalProtocolList->at(i)->pTerminalProtocol), SIGNAL(sigSendCanData(QList<can_frame*>*,QMutex*)), pCanBus, SLOT(ProcSendData(QList<can_frame*>*,QMutex*)));
            //关联交流三相协议和充电设备类(发送总线数据)
            connect((cACThrCanProtocol*)(pTerminalProtocolList->at(i)->pTerminalProtocol), SIGNAL(sigSendToCenter(uint,InfoMap)), SLOT(ProcRecvProtocolData(uint,InfoMap)));
        }
        if(pTerminalProtocolList->at(i)->enTermProType == Pro_J1939_DC)
        {
            //关联CAN总线和直流CAN协议(发送CAN帧)
            connect((cDCCanProtocol*)(pTerminalProtocolList->at(i)->pTerminalProtocol), SIGNAL(sigSendCanData(QList<can_frame*>*,QMutex*)), pCanBus, SLOT(ProcSendData(QList<can_frame*>*,QMutex*)));
            //关联直流CAN协议和充电设备类(发送总线数据)
            connect((cDCCanProtocol*)(pTerminalProtocolList->at(i)->pTerminalProtocol), SIGNAL(sigSendToCenter(uint,InfoMap)), SLOT(ProcRecvProtocolData(uint,InfoMap)));
        }
        if(pTerminalProtocolList->at(i)->enTermProType == Pro_J1939_EnergyPlan)
        {
            //关联CAN总线和能效计划CAN协议(发送CAN帧)
            connect((cEnergyPlanProtocol*)(pTerminalProtocolList->at(i)->pTerminalProtocol), SIGNAL(sigSendCanData(QList<can_frame*>*,QMutex*)), pCanBus, SLOT(ProcSendData(QList<can_frame*>*,QMutex*)));
            //关联能效计划CAN协议和充电设备类(发送总线数据)
            connect((cEnergyPlanProtocol*)(pTerminalProtocolList->at(i)->pTerminalProtocol), SIGNAL(sigSendToCenter(uint,InfoMap)), SLOT(ProcRecvProtocolData(uint,InfoMap)));
        }
        if(pTerminalProtocolList->at(i)->enTermProType == Pro_J1939_CSCU)
        {
            //关联CAN总线和能效计划CAN协议(发送CAN帧)
            connect((cCSCUCanProtocol*)(pTerminalProtocolList->at(i)->pTerminalProtocol), SIGNAL(sigSendCanData(QList<can_frame*>*,QMutex*)), pCanBus, SLOT(ProcSendData(QList<can_frame*>*,QMutex*)));
            //关联能效计划CAN协议和充电设备类(发送总线数据)
            connect((cCSCUCanProtocol*)(pTerminalProtocolList->at(i)->pTerminalProtocol), SIGNAL(sigSendToCenter(uint,InfoMap)), SLOT(ProcRecvProtocolData(uint,InfoMap)));
        }
    }
    //关联CAN总线接收,和J1939接收预处理(接收CAN帧)
    connect(pCanBus, SIGNAL(sigParseData(QList<can_frame*>*,QMutex*)), pJ1939PreParse, SLOT(ProcParseData(QList<can_frame*>*,QMutex*)));
    return TRUE;
}

//CAN线程初始化并启动
void cChargeEquipment::CanThreadInit()
{
    pCanThread = new QThread();
    pCanBus = new cCanBus(pLog);
    pCanBus->moveToThread(pCanThread);
    pCanBus->connect(pCanThread,SIGNAL(started()),SLOT(ProcStartWork()));
    pCanThread->start();
}

//协议列表初始化
void cChargeEquipment::TerminalProtocolInit()
{
    //终端协议列表初始化
    pTerminalProtocolList = new QList <TermProNode *>;
    TermProNode * pTermProNode1 = new TermProNode;
    TermProNode * pTermProNode2 = new TermProNode;
    TermProNode * pTermProNode3 = new TermProNode;
    TermProNode * pTermProNode4 = new TermProNode;
    TermProNode * pTermProNode5 = new TermProNode;
    //添加交流单相协议----0
    pTermProNode1->enTermProType = Pro_J1939_ACSin;
    pTermProNode1->pTerminalProtocol = new cACSinCanProtocol();
    pTerminalProtocolList->append(pTermProNode1);
    //添加交流三相协议----1
    pTermProNode2->enTermProType = Pro_J1939_ACThr;
    pTermProNode2->pTerminalProtocol = new cACThrCanProtocol();
    pTerminalProtocolList->append(pTermProNode2);
    //添加直流协议----2
    pTermProNode3->enTermProType = Pro_J1939_DC;
    pTermProNode3->pTerminalProtocol = new cDCCanProtocol();
    pTerminalProtocolList->append(pTermProNode3);
    //添加能效计划CAN协议----3
    pTermProNode4->enTermProType = Pro_J1939_EnergyPlan;
    pTermProNode4->pTerminalProtocol = new cEnergyPlanProtocol();
    pTerminalProtocolList->append(pTermProNode4);
    //添加集控主从协议----4
    pTermProNode5->enTermProType = Pro_J1939_CSCU;
    pTermProNode5->pTerminalProtocol = new cCSCUCanProtocol();
    pTerminalProtocolList->append(pTermProNode5);
    //CAN协议预处理类初始化
    pJ1939PreParse = new cJ1939PreParse(pTerminalProtocolList, pLog);
}

void cChargeEquipment::ProcOneSecTimeOut()
{
    if(bWorkFlag == FALSE)
    {
        return;
    }

    for(int i = 0; i < pTerminalProtocolList->count(); i++)
    {

        //发送向对应终端发送指令
        if(pTerminalProtocolList->at(i)->pTerminalProtocol->HasFrameToSend())
        {
            pTerminalProtocolList->at(i)->pTerminalProtocol->SendFrame();
        }
        //检查长帧并处理
        if(pTerminalProtocolList->at(i)->pTerminalProtocol->HasFrameToDeal())
        {
            pTerminalProtocolList->at(i)->pTerminalProtocol->DealFrame();
        }
    }
}

//接收协议解析后数据 -- 发送给控制总线
void cChargeEquipment::ProcRecvProtocolData(unsigned int uiInfoAddr, InfoMap TerminalDataMap)
{
    if(bWorkFlag == FALSE)
    {
        return;
    }

    switch(uiInfoAddr)
    {
    case AddrType_TermSignal:   //发送遥信
        pRealDataFilter->realDataUpdate(TerminalDataMap, AddrType_TermSignal);
        if(TerminalDataMap.contains(Addr_ChargeEndCode_Term))
        {
            QString stInfo;
            unsigned char ucEndCode = TerminalDataMap[Addr_ChargeEndCode_Term].at(0);
            unsigned char ucCanID = TerminalDataMap[Addr_CanID_Comm].at(0);
            stInfo.sprintf("发送充电中止原因到总线, 终端CAN地址为: %d, 中止原因为: %d",ucCanID, ucEndCode);
//            pLog->getLogPoint(LOG_MODULE_CAN0)->info(stInfo);
        }
        break;
    case AddrType_TermMeasure:  //发送遥测
        pRealDataFilter->realDataUpdate(TerminalDataMap, AddrType_TermMeasure);
        break;
    case AddrType_TermBMS:  //发送BMS信息
        pRealDataFilter->realDataUpdate(TerminalDataMap, AddrType_TermBMS);
        break;
    case AddrType_CCUSignal_DCCab:  //CCU遥信
        pRealDataFilter->realDataUpdate(TerminalDataMap, AddrType_CCUSignal_DCCab);
        break;
    case AddrType_CCUMeasure_DCCab:  //CCU遥测
        pRealDataFilter->realDataUpdate(TerminalDataMap, AddrType_CCUMeasure_DCCab);
        break;
    case AddrType_PDUSignal_DCCab:  //PDU遥信
        pRealDataFilter->realDataUpdate(TerminalDataMap, AddrType_PDUSignal_DCCab);
        break;
    case AddrType_PDUMeasure_DCCab:  //PDU遥测
        pRealDataFilter->realDataUpdate(TerminalDataMap, AddrType_PDUMeasure_DCCab);
        break;
    case AddrType_BOXSignal_DCCab:  //分支箱遥信
        pRealDataFilter->realDataUpdate(TerminalDataMap, AddrType_BOXSignal_DCCab);
        break;
    case AddrType_BOXMeasure_DCCab:  //分支箱遥测
        pRealDataFilter->realDataUpdate(TerminalDataMap, AddrType_BOXMeasure_DCCab);
        break;
    case AddrType_MODSignal_DCCab:  //模块遥信
        pRealDataFilter->realDataUpdate(TerminalDataMap, AddrType_MODSignal_DCCab);
        break;
    case AddrType_MODMeasure_DCCab:  //模块遥测
        pRealDataFilter->realDataUpdate(TerminalDataMap, AddrType_MODMeasure_DCCab);
        break;

        //smm add
    case  AddrType_JiGuiTempHumidty:
            pRealDataFilter->realDataUpdate(TerminalDataMap, AddrType_JiGuiTempHumidty);
        break;
    case  AddrType_Qiang:
            pRealDataFilter->realDataUpdate(TerminalDataMap, AddrType_Qiang);
           break;
        //smm end
    case AddrType_TermAdjustmentAck:  //发送遥调回复
        emit sigToBus(TerminalDataMap, AddrType_TermAdjustmentAck);
        break;
    case AddrType_CmdCtrl_Ack:  //发送遥控回复
        emit sigToBus(TerminalDataMap, AddrType_CmdCtrl_Ack);
        break;
    case AddrType_SingleCardApplyAccountInfo:  //发送刷卡卡号
        emit sigToBus(TerminalDataMap, AddrType_SingleCardApplyAccountInfo);
        break;
    case AddrType_InApplyStartChargeByChargeEquipment:  //发送刷卡开始充电
        emit sigToBus(TerminalDataMap, AddrType_InApplyStartChargeByChargeEquipment);
        break;
    case AddrType_InApplyStopChargeByChargeEquipment:  //发送刷卡结束充电
        emit sigToBus(TerminalDataMap, AddrType_InApplyStopChargeByChargeEquipment);
        break;
    case AddrType_FaultState_DCcab:  //直流柜故障状态
        emit sigToBus(TerminalDataMap, AddrType_FaultState_DCcab);
        pRealDataFilter->realDataUpdate(TerminalDataMap, AddrType_FaultState_DCcab);
        break;
    case AddrType_UpdateResult_Dev: //CAN设备升级结果
        emit sigToBus(TerminalDataMap, AddrType_UpdateResult_Dev);
        break;
//    case AddrType_CheckChargeManner_Success://校验主副枪符合规则
//        emit sigToBus(TerminalDataMap, AddrType_CheckChargeManner_Success);
//        break;
    case AddrType_CheckChargeManner://发送多枪组合
        emit sigToBus(TerminalDataMap, AddrType_CheckChargeManner);

    case AddrType_DetailParamSetResult_ACSin://交流单相详细参数设置结果返回
        emit sigToBus(TerminalDataMap, AddrType_DetailParamSetResult_ACSin);
        break;
    case AddrType_DetailParamQueryResult_ACSin://交流单相详细参数查询结果
        emit sigToBus(TerminalDataMap, AddrType_DetailParamQueryResult_ACSin);
        break;
    case AddrType_DetailParamSetResult_ACThr://交流三相详细参数设置结果返回
        emit sigToBus(TerminalDataMap, AddrType_DetailParamSetResult_ACThr);
        break;
    case AddrType_ActiveProtectSetResult://主动防护功能设置结果返回
        emit sigToBus(TerminalDataMap, AddrType_ActiveProtectSetResult);
        break;
    case AddrType_ActiveProtectQueryResult://主动防护功能查询结果
        emit sigToBus(TerminalDataMap, AddrType_ActiveProtectQueryResult);
        break;
    case AddrType_FlexibleChargeSetResult://柔性充电功能设置结果返回
        emit sigToBus(TerminalDataMap, AddrType_FlexibleChargeSetResult);
        break;
    case AddrType_FlexibleChargeQueryResult://柔性充电功能查询结果
        emit sigToBus(TerminalDataMap, AddrType_FlexibleChargeQueryResult);
        break;
    case AddrType_GeneralStaticArgResult://通用静态参数设置返回结果
        emit sigToBus(TerminalDataMap, AddrType_GeneralStaticArgResult);
        break;
    case AddrType_GeneralStaticArgQueryResult://通用静态参数设置查询结果
        emit sigToBus(TerminalDataMap, AddrType_GeneralStaticArgQueryResult);
        break;
    case AddrType_GeneralDynamicArgResult://通用动态参数设置
        emit sigToBus(TerminalDataMap, AddrType_GeneralDynamicArgResult);
        break;
    case AddrType_GeneralDynamicArgQueryResult://通用动态参数设置查询结果
        emit sigToBus(TerminalDataMap, AddrType_GeneralDynamicArgQueryResult);
        break;
    case AddrType_CCUArgResult://CCU参数设置返回结果
        emit sigToBus(TerminalDataMap, AddrType_CCUArgResult);
        break;
    case AddrType_CCUQueryResult://CCU参数设置查询结果
        emit sigToBus(TerminalDataMap, AddrType_CCUQueryResult);
        break;
    case AddrType_ModuleSpecInfo:   //模块规格信息
        //emit sigToBus(TerminalDataMap, AddrType_ModuleSpecInfo);
        pRealDataFilter->realDataUpdate(TerminalDataMap, AddrType_ModuleSpecInfo);
        break;
    case AddrType_ES_Cab_Info:
        pRealDataFilter->realDataUpdate(TerminalDataMap, AddrType_ES_Cab_Info);
        break;
    case AddrType_ES_Bat_Info:
        pRealDataFilter->realDataUpdate(TerminalDataMap, AddrType_ES_Bat_Info);
        break;
    case AddrType_PH_Cab_Info:
        pRealDataFilter->realDataUpdate(TerminalDataMap, AddrType_PH_Cab_Info);
        break;
    case AddrType_SC_Cab_Info:
        pRealDataFilter->realDataUpdate(TerminalDataMap, AddrType_SC_Cab_Info);
        break;
    case AddrType_FQ_Cab_Info:
        pRealDataFilter->realDataUpdate(TerminalDataMap, AddrType_FQ_Cab_Info);
        break;
    case AddrType_CD_Cab_Info:
        pRealDataFilter->realDataUpdate(TerminalDataMap, AddrType_CD_Cab_Info);
        break;
    case AddrType_TD_Cab_Info:
        pRealDataFilter->realDataUpdate(TerminalDataMap, AddrType_TD_Cab_Info);
        break;
    case AddrType_PO_Mod_Info:
        pRealDataFilter->realDataUpdate(TerminalDataMap, AddrType_PO_Mod_Info);
        break;
    case AddrType_HY_Mod_Info:
        pRealDataFilter->realDataUpdate(TerminalDataMap, AddrType_HY_Mod_Info);
        break;
    case AddrType_SI_Cab_Info:
        pRealDataFilter->realDataUpdate(TerminalDataMap, AddrType_SI_Cab_Info);
        break;
    case AddrType_ACDC_Mod_Info:
        pRealDataFilter->realDataUpdate(TerminalDataMap, AddrType_ACDC_Mod_Info);
        break;
    case AddrType_DCDC_Mod_CD_Info:
//        pRealDataFilter->realDataUpdate(TerminalDataMap, AddrType_DCDC_Mod_CD_Info);
        break;
    case AddrType_DCDC_Mod_ES_Info:
        pRealDataFilter->realDataUpdate(TerminalDataMap, AddrType_DCDC_Mod_ES_Info);
        break;
    case AddrType_EMS_Info:
        pRealDataFilter->realDataUpdate(TerminalDataMap, AddrType_EMS_Info);
    case AddrType_TempHumi:         //高防护采集温度
        pRealDataFilter->realDataUpdate(TerminalDataMap, AddrType_TempHumi);
        break;
//    case AddrType_ModuleSpecInfo: //设备规格信息 9F
//        pRealDataFilter->realDataUpdate(TerminalDataMap, AddrType_ModuleSpecInfo);
//        break;
    default:
        break;
    }
}

//接收总线发送的数据
void cChargeEquipment::slotFromBus(InfoMap RecvCenterDataMap, InfoAddrType enAddrType)
{
    if(bWorkFlag == FALSE)
    {
        return;
    }
    //Addr_BatteryVIN_BMS
    if(!RecvCenterDataMap.contains(Addr_CanID_Comm))
    {
        return;
    }
    unsigned char ucCanID = 0;
    InfoMap::iterator itTarget;
    //确定CAN地址
    itTarget = RecvCenterDataMap.find(Addr_CanID_Comm);
    ucCanID = itTarget.value().at(0);
    RecvCenterDataMap.remove(Addr_CanID_Comm);
    //找对应的协议去处理总线数据
    if ((ucCanID >= ID_MinACSinCanID) &&(ucCanID <= ID_MaxACSinCanID))//交流单相终端----交流单相协议处理
    {
        pTerminalProtocolList->at(0)->pTerminalProtocol->ParseCenterData(RecvCenterDataMap, enAddrType, ucCanID);
    }
    //交流负载总配电柜
    else if((ucCanID >= ID_MinACLoadDistributionCabinet) &&(ucCanID <= ID_MaxACLoadDistributionCabinet))
    {
        pTerminalProtocolList->at(3)->pTerminalProtocol->ParseCenterData(RecvCenterDataMap, enAddrType, ucCanID);
    }
    //系统配电柜
    else if((ucCanID >= ID_MinSystemDistributionCabinet) &&(ucCanID <= ID_MaxSystemDistributionCabinet))
    {
        pTerminalProtocolList->at(3)->pTerminalProtocol->ParseCenterData(RecvCenterDataMap, enAddrType, ucCanID);
    }
    //直流光伏控制柜
    else if((ucCanID >= ID_MinDCPhotovoltaicControlCabinet) &&(ucCanID <= ID_MaxDCPhotovoltaicControlCabinet))
    {
        pTerminalProtocolList->at(3)->pTerminalProtocol->ParseCenterData(RecvCenterDataMap, enAddrType, ucCanID);
    }
    //四象限变换柜
    else if((ucCanID >= ID_MinFourQuadrantConverterCabinet) &&(ucCanID <= ID_MaxFourQuadrantConverterCabinet))
    {
        pTerminalProtocolList->at(3)->pTerminalProtocol->ParseCenterData(RecvCenterDataMap, enAddrType, ucCanID);
    }
    //直流充放电柜
    else if((ucCanID >= ID_MinDCChargeDischargeCabinet) &&(ucCanID <= ID_MaxDCChargeDischargeCabinet))
    {
        pTerminalProtocolList->at(3)->pTerminalProtocol->ParseCenterData(RecvCenterDataMap, enAddrType, ucCanID);
    }
    //直流储能柜
    else if((ucCanID >= ID_MinDCEnergyStorageCabinet) &&(ucCanID <= ID_MaxDCEnergyStorageCabinet))
    {
        pTerminalProtocolList->at(3)->pTerminalProtocol->ParseCenterData(RecvCenterDataMap, enAddrType, ucCanID);
    }
    else if ((ucCanID >= ID_MinACThrCanID) &&(ucCanID <= ID_MaxACThrCanID))//交流三相终端----交流三相协议处理
    {
        pTerminalProtocolList->at(1)->pTerminalProtocol->ParseCenterData(RecvCenterDataMap, enAddrType, ucCanID);
    }
    else if ((ucCanID >= ID_MinDCCanID) &&(ucCanID <= ID_MaxDCCanID))//直流终端----直流协议处理
    {
        pTerminalProtocolList->at(2)->pTerminalProtocol->ParseCenterData(RecvCenterDataMap, enAddrType, ucCanID);
    }
    else if ((ucCanID >= ID_MinCCUCanID) &&(ucCanID <= ID_MaxCCUCanID))//CCU----直流协议处理
    {
        pTerminalProtocolList->at(2)->pTerminalProtocol->ParseCenterData(RecvCenterDataMap, enAddrType, ucCanID);
    }
    else if ((ucCanID > ID_DefaultControlCenterCanID) &&(ucCanID <= ID_MaxControlCenterCanID))//CSCU----集控主从协议处理
    {
        pTerminalProtocolList->at(4)->pTerminalProtocol->ParseCenterData(RecvCenterDataMap, enAddrType, ucCanID);
    }
    else if(ucCanID == ID_BroadCastCanID)   //广播   --直流协议
    {
        pTerminalProtocolList->at(2)->pTerminalProtocol->ParseCenterData(RecvCenterDataMap, enAddrType, ucCanID);
    }
}

//SO库调用实现函数, 创建新实例返回
CModuleIO* CreateDevInstance()
{
    return new cChargeEquipment();
}

//实例销毁
void DestroyDevInstance(CModuleIO* pModule)
{
    if(pModule){
        delete pModule;
    }
}
