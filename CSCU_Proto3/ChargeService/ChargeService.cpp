#include <stdio.h>
#include <stdlib.h>
#include <QDebug>
#include <QUuid>
#include <QFile>
#include "math.h"
#include "ChargeService.h"
#include "CommonFunc/commfunc.h"

#define DB_EMERGENCY_ORDER_PATH (char*)"/mnt/nandflash/database/emergency_order.db"
#define DB_EMERGENCY_REAL_PATH (char*)"/mnt/nandflash/database/emergency_real.db"

typedef QList<InfoTAG>  QListInfoTAG;
__u8 g_TimeoutChargeStep[] = {255, 5, 85, 60, 60, 60, 60, 60, 60, 60,15, 15, 15, 15, 2};//充电业务状态机超时时间定义

//----------------------------------------------Other private----------------------------------------------//
///
/// \brief ChargeService::SaveChargeStep 缓存数据持久化
/// \param stChargeStep 待持久化的状态机
///
void ChargeService::SaveChargeStep(CHARGE_STEP &stChargeStep)
{
    if(gpDevCache->SaveChargeStep(stChargeStep.ucCanAddr) == false){
        LogOut(QString("SaveChargeStep  fail!"), 3);
    }
}

///
/// \brief ChargeService::LogOut 日志记录
/// \param str 日志串
/// \param Level 级别
///
inline void  ChargeService::LogOut(QString str,int Level)
{
    switch (Level) {
    case 1:
        gpLog->getLogPoint(_strLogName)->debug(str);
        break;
    case 2:
        gpLog->getLogPoint(_strLogName)->info(str);
        break;
    case 3:
        gpLog->getLogPoint(_strLogName)->warn(str);
        break;
    case 4:
        gpLog->getLogPoint(_strLogName)->error(str);
        break;
    default:
        break;
    }
}

void ChargeService::TempOutPutTerminalStatus(TerminalStatus &st_TerminalStatus)
{
    //return;
    LogOut(QString("CAN = %1").arg(st_TerminalStatus.cCanAddr), 1);
    LogOut(QString("cStatus = %1").arg(st_TerminalStatus.cStatus), 1);
    LogOut(QString("ChangeType = %1").arg(st_TerminalStatus.en_ChargeStatusChangeType), 1);
    LogOut(QString("RTstatus = %1").arg((__u8)st_TerminalStatus.stFrameRemoteSingle.charge_status), 1);
    LogOut(QString("LINK = %1").arg((__u8)st_TerminalStatus.stFrameRemoteSingle.link_status), 1);
}

void ChargeService::TempOutPutChargeStep(CHARGE_STEP &stChargeStep)
{
    //return;
    LogOut(QString("CAN = %1").arg(stChargeStep.ucCanAddr), 1);
    LogOut(QString("ucCmdValue = %1").arg(stChargeStep.ucCmdValue), 1);
    LogOut(QString("sEventNo = %1").arg(QByteArray::fromRawData(stChargeStep.sEventNo, LENGTH_EVENT_NO).toHex().data()), 1);
    LogOut(QString("enChargeStep = %1").arg(stChargeStep.enChargeStep), 1);
    LogOut(QString("uc_hold_time = %1").arg(stChargeStep.stChargeStepValue.uc_hold_time), 1);
    LogOut(QString("NowCount = %1").arg(stChargeStep.stRepeatCmd.NowCount), 1);
    LogOut(QString("MaxCount = %1").arg(stChargeStep.stRepeatCmd.MaxCount), 1);
    LogOut(QString("enOrderStatus = %1").arg(stChargeStep.enOrderStatus), 1);
    LogOut(QString("sStartTime = %1").arg(QString(QLatin1String(stChargeStep.sStartTime))), 1);
    LogOut(QString("ucStartReason = %1").arg((stChargeStep.ucStartReason)), 1);
    LogOut(QString("ucCmdSrc = %1").arg(stChargeStep.ucCmdSrc), 1);
    QByteArray array;
    array = QByteArray(stChargeStep.sOrderUUID, LENGTH_GUID_NO);
    LogOut(QString("OrderUUID = %1").arg(array.toHex().data()), 1);
    array = QByteArray(stChargeStep.sVIN, LENGTH_VIN_NO);
    LogOut(QString("sVIN = %1").arg(QString(stChargeStep.sVIN)), 1);
    LogOut(QString("icurvestatus = %1").arg(stChargeStep.iCurveState), 1);
}

//------------------------------------------------------类基础------------------------------------------------------//
///
/// \brief ChargeService::ChargeService 构造函数
/// \param parent
///
ChargeService::ChargeService()
{    
    stCSCUSysConfig stTempCCUsysConfig;

	_strLogName = "charge";

    pDBOperate = DBOperate::GetInstance();
    gpDevCache = DevCache::GetInstance();
    gpParamSet = ParamSet::GetInstance();
    gpLog = Log::GetInstance();

    iStartOrStopFlag = 0;
    DoorOpenStopCharge = false; //add by zjq 初始化，防止没有初始化导致为true
    bStopFlag = false;

    bChargeFreezeEnery.clear();

    gpParamSet->querySetting(&stTempCCUsysConfig, PARAM_CSCU_SYS);
    ucNumTerminal[0] = stTempCCUsysConfig.singlePhase;
    ucNumTerminal[1] = stTempCCUsysConfig.threePhase;
    ucNumTerminal[2] = stTempCCUsysConfig.directCurrent;
    LogOut(QString("Config Terminal Num: AC=%1, 3AC=%2, DC=%3")\
           .arg(ucNumTerminal[0]).arg(ucNumTerminal[1]).arg(ucNumTerminal[2]), 2);

    InitTerminalStatus(stEmptyTerminalStatus);//创建一个空的终端状态
    InitChargeStep(stEmptyChargeStep);//创建一个空的终端状态
    pCardChargeFun = new CardChargeFun();
    QObject::connect( pCardChargeFun, SIGNAL(sigToChargeService(InfoMap, InfoAddrType)), this, SLOT(slot_RecvFromCardCharge(InfoMap, InfoAddrType)) );

    map_LogicStatus.insert(0, "启动中");
    map_LogicStatus.insert(1, "待机-插抢");
    map_LogicStatus.insert(2, "充电-限制");
    map_LogicStatus.insert(3, "充电-暂停");
    map_LogicStatus.insert(4, "充电-充电中");
    map_LogicStatus.insert(5, "待机-切换中");
    map_LogicStatus.insert(6, "待机-空闲");
    map_LogicStatus.insert(7, "离线");
    map_LogicStatus.insert(8, "待机-已完成");
    map_LogicStatus.insert(9, "故障");
    map_LogicStatus.insert(10, "放电中");
    map_LogicStatus.insert(11, "11");
    map_LogicStatus.insert(12, "12");
    map_LogicStatus.insert(13, "待机-已充满");
    map_LogicStatus.insert(14, "14");
    map_LogicStatus.insert(15, "待机-排队1");
    map_LogicStatus.insert(16, "待机-排队2");
    map_LogicStatus.insert(17, "待机-排队3");
    map_LogicStatus.insert(18, "待机-排队4");
    map_LogicStatus.insert(19, "待机-排队5");
    map_LogicStatus.insert(20, "待机-排队6");
    map_LogicStatus.insert(21, "待机-排队7");
    map_LogicStatus.insert(25, "等待中");
    map_LogicStatus.insert(30, "暂停-车辆");
    map_LogicStatus.insert(31, "暂停-设备");
    map_LogicStatus.insert(32, "暂停-CSCU");
    map_LogicStatus.insert(33,"调度中");

}


///
/// \brief ChargeService::~ChargeService 析构函数
///
ChargeService::~ChargeService()
{
    delete tmChargeStep;
    delete tmSaveChargingData;
    map_LogicStatus.clear();
    LogOut(QString("充电服务退出成功 !"), 3);
}


//------------------------------------------------------模块对外部统一接口相关------------------------------------------------------//

int ChargeService::InitModule(QThread* pThread)
{
    m_pWorkThread = pThread;
    this->moveToThread(m_pWorkThread);
    QObject::connect(m_pWorkThread, SIGNAL(started()), this, SLOT(slotThreadStart()));

    LogOut(QString("充电服务初始化成功 !"), 3);
    return 0;
}

///
/// \brief ChargeService::RegistModule 将本模块订阅的主题发送给BUS
/// \param pBus
/// \return
///
int ChargeService::RegistModule()
{
	QList<int> list;
	//-----------------APP远程充电相关主题--------------//
	list.append(AddrType_CmdCtrl_Apply);
	list.append(AddrType_TermSignal);
	list.append(AddrType_CmdCtrl_Ack);
	//-----------------刷卡远程充电相关主题--------------//
	list.append(AddrType_SingleCardApplyAccountInfo);//单桩申请账户信息
	list.append(AddrType_CenterCardApplyAccountInfo);//集中刷卡申请账户信息
	list.append(AddrType_ChargeServicApplyAccountInfo_Result);//服务器返回申请账户信息结果
	list.append(AddrType_InApplyStartChargeByScreen);//显示屏内部申请开始充电
	list.append(AddrType_InApplyStartChargeByChargeEquipment);//充电设备内部申请开始充电
	list.append(AddrType_OutApplyStartChargeByChargeServic_Result);//服务器返回申请开始充电结果
	list.append(AddrType_InApplyStopChargeByScreen);//显示屏内部申请结束充电
	list.append(AddrType_InApplyStopChargeByChargeEquipment);//充电设备内部申请结束充电
	// list.append(AddrType_InApplyStartChargeByChargeEquipment);//充电设备内部申请结束充电
	list.append(AddrType_OutApplyStopChargeByChargeServic_Result);//服务器返回申请结束充电结果
	//-----------------VIN远程充电相关主题--------------//
	list.append(AddrType_VinNum);//主题一：VIN号
	list.append(AddrType_VinApplyStartCharge_Result);//主题三：服务器返回VIN申请开始充电结果
	list.append(AddrType_VinApplyStopCharge_Result);//主题五：服务器返回VIN申请结束充电结果
	list.append(AddrType_InVinApplyStartCharge);//主题：VIN内部申请开始充电
	list.append(AddrType_InVinApplyStopCharge);//主题：VIN内部申请结束充电
	//-----------------车牌号远程充电相关主题--------------//
	list.append(AddrType_CarLicence);//主题十：车牌号
	list.append(AddrType_CarLicenceApplyStartCharge_Result);//主题七：车牌号外部申请开始充电结果
	//-----------------充电排队相关主题--------------//
	list.append(AddrType_QueueInfo);//下发充电排队信息。服务器发布，充电服务模块订阅
	//-----------------VIN后六位充电相关主题--------------//
	list.append(AddrType_VINViaScreenApplyCharge);//VIN后6位申请开始充电, 信息体：CAN地址, VIN码  发出方：显示屏, 订阅方：充电服务
	//-----------------点击界面按钮结束充电--------------//
	list.append(AddrType_ScreenApplyStopCharge);//显示屏申请结束充电, 信息体：CAN地址  发出方：显示屏, 订阅方：充电服务
	list.append(AddrType_QueueGroup_State);//排队信息带流水号
	//-----------------有功电能值告警--------------//
	list.append(AddrType_ActiveEnergyFault_Term);//终端有功电能告警, 信息体: CAN地址, 异常电量, 实时数据模块发布, 充电服务模块订阅
	list.append(AddrType_ActiveDefend_StopCharge);//主动防御停止充电
	//-----------------多枪充电---------------------//
	//list.append(AddrType_CheckChargeManner_Success);//直流机上传单双枪信息符合规则
	list.append(AddrType_CheckChargeManner);//直流机上传单双枪信息不符合规则
	list.append(AddrType_Response_Result);//单双枪信息响应结果
	list.append(AddrType_ChargeGunGroup_Info);//多枪分组信息下发
	list.append(AddrType_VinApplyStartChargeImmed);
	list.append(AddrType_MagneticSwitch_State);//门磁开关
	//3.0协议相关
	list.append(AddrType_Power_Curve);//功率曲线下发
	list.append(AddrType_Offline_Time);//功率曲线离网时间

	CBus::GetInstance()->RegistDev(this, list);

	LogOut(QString("充电服务注册成功 !"), 3);
	return 0;
}

int ChargeService::StartModule()
{
    m_pWorkThread->start();
    LogOut(QString("充电服务启动成功 !"), 3);
    return 0;
}

int ChargeService::StopModule()
{
    if(tmChargeStep->isActive()){
        tmChargeStep->stop();
    }
    if(tmSaveChargingData->isActive()){
        tmSaveChargingData->stop();
    }
    LogOut(QString("充电服务停止成功 !"), 3);
    return 0;
}

int ChargeService::ModuleStatus()
{
    if(tmChargeStep->isActive() == false){
        tmChargeStep->start();
        LogOut(QString("充电状态机定时器停止,重新激活 !"), 3);
    }
    if(tmSaveChargingData->isActive() == false){
        tmSaveChargingData->start();
    }
    LogOut(QString("ChargeService status ok !"), 1);
    return 0;
}

//CModuleIO* CreateDevInstance(int argc, void *pDepends[])
//{
//    return new ChargeService(pDepends);
//}

//void DestroyDevInstance(CModuleIO* pModule)
//{
//    if(pModule){
//        delete pModule;
//    }
//}

//------------------------------------------------------CSCU BUS接口相关------------------------------------------------------//

///
/// \brief ChargeService::slotFromBus 接收BUS数据
/// \param qInfoMap
/// \param InfoType
///
void ChargeService::slotFromBus(InfoMap qInfoMap, InfoAddrType InfoType)
{
    LogOut(QString("收到BUS数据!"), 1);

    switch (InfoType) {
    case AddrType_TermSignal:
        LogOut(QString("收到BUS摇信数据!"), 1);
        ProcTeleindicationDataTerm(qInfoMap);
        break;
    case AddrType_CmdCtrl_Apply:
        LogOut(QString("收到BUS控制指令!"), 1);
        ProcTeleControlChargeCmd(qInfoMap);
        break;
    case AddrType_CmdCtrl_Ack:
        LogOut(QString("收到BUS控制指令ACK!"), 1);
        ProcTeleControlChargeCmdAck(qInfoMap);
        break;
        //-----------------刷卡远程充电相关主题--------------//
    case AddrType_CenterReadCard://集中读卡卡号
    case AddrType_SingleCardApplyAccountInfo://单桩申请账户信息
    case AddrType_CenterCardApplyAccountInfo://集中刷卡申请账户信息
    case AddrType_ChargeServicApplyAccountInfo_Result://服务器返回申请账户信息结果
    case AddrType_InApplyStartChargeByScreen://显示屏内部申请开始充电
    case AddrType_InApplyStartChargeByChargeEquipment://充电设备内部申请开始充电
    case AddrType_OutApplyStartChargeByChargeServic_Result://服务器返回申请开始充电结果
    case AddrType_InApplyStopChargeByScreen://显示屏内部申请结束充电
    case AddrType_InApplyStopChargeByChargeEquipment://充电设备内部申请结束充电
    case AddrType_OutApplyStopChargeByChargeServic_Result://服务器返回申请结束充电结果
        pCardChargeFun->ProcCardChargeSub(qInfoMap, InfoType);
        break;
        //-----------------VIN充电相关主题--------------//
    case AddrType_VinNum://主题一：VIN号
    case AddrType_VinApplyStartCharge_Result://主题三：服务器返回VIN申请开始充电结果
    case AddrType_VinApplyStopCharge_Result://主题五：服务器返回VIN申请结束充电结果
    case AddrType_InVinApplyStartCharge://主题：VIN内部申请开始充电
    case AddrType_InVinApplyStopCharge://主题：VIN内部申请结束充电
        ProcVinChargeSub(qInfoMap, InfoType);
        break;
        //-----------------车牌号充电相关主题--------------//
    case AddrType_CarLicence://主题十：车牌号
    case AddrType_CarLicenceApplyStartCharge_Result://主题七：车牌号外部申请开始充电结果
        ProcCarLisenceChargeSub(qInfoMap, InfoType);
        break;
        //-----------------充电排队相关主题--------------//
    case AddrType_QueueInfo://下发充电排队信息。服务器发布，充电服务模块订阅
        //2.5使用新的排队信息
        break;
        ProcChargeQueueSub(qInfoMap);
        break;
        //-----------------VIN后六位充电相关主题--------------//
    case AddrType_VINViaScreenApplyCharge://VIN后6位申请开始充电, 信息体：CAN地址, VIN码  发出方：显示屏, 订阅方：充电服务
        ProcVINViaChargeSub(qInfoMap);
        break;
        //-----------------点击界面按钮结束充电--------------//
    case AddrType_ScreenApplyStopCharge://显示屏申请结束充电, 信息体：CAN地址  发出方：显示屏, 订阅方：充电服务
        ProcScreenButtonStopChargeSub(qInfoMap);
        break;
        //-----------------有功电能值告警--------------//
    case AddrType_ActiveEnergyFault_Term:    //终端有功电能告警, 信息体: CAN地址, 异常电量, 实时数据模块发布, 充电服务模块订阅
        ProcActiveEnergyFaultSub(qInfoMap);
        break;
	case AddrType_ActiveDefend_StopCharge://主动防御停止充电
		ProcActiveDefendSub(qInfoMap);
		break;
    case AddrType_CheckChargeManner:
        ProcTerminalChargeMannerInfo(qInfoMap);
        break;
    case AddrType_Response_Result:
        ProcResponseResult(qInfoMap);
        break;
    case AddrType_ChargeGunGroup_Info:
        ProcChargeGunGroupInfo(qInfoMap);
        break;
    case AddrType_VinApplyStartChargeImmed:
        ProcVinChargeSubImmed(qInfoMap);
        break;
        //轮充组信息下发
    case AddrType_QueueGroup_State:
        UpdateQueueState(qInfoMap);
        break;
        //门磁开关报警
    case AddrType_MagneticSwitch_State:
        ProcDoorOpenAlarmSub(qInfoMap);
        break;
    case AddrType_Power_Curve://功率曲线策略
        updatePowerCurve(qInfoMap);
        break;
	case AddrType_Offline_Time://离线时间
		clearPowerCurve(qInfoMap);
		break;
    default:
        break;
    }
}

///
/// \brief ChargeService::slotThreadStart main线程启动信号的槽函数
///
void ChargeService::slotThreadStart()
{
    LogOut(QString("充电服务线程启动 !"), 3);

    tmChargeStep = new QTimer();
    connect(tmChargeStep,SIGNAL(timeout()), SLOT(slot_ProcChargeStepTimeOut()));
    tmChargeStep->start(1000);

    tmSaveChargingData = new QTimer();
    connect(tmSaveChargingData,SIGNAL(timeout()), SLOT(slot_ProcSaveChargingDataTimeOut()));
    tmSaveChargingData->start(1000*60);

    ChargeServiceLaunchTime = QDateTime::currentDateTime();
    tmActiveUpateLogicStatus = new QTimer();
    connect(tmActiveUpateLogicStatus,SIGNAL(timeout()), SLOT(slot_ProcActiveUpateLogicStatusTimeOut()));
    tmActiveUpateLogicStatus->start(1000*15);
}



void ChargeService::InitTerminalStatus(TerminalStatus &st_TerminalStatus)
{
    st_TerminalStatus.cCanAddr = 0;//
    memset(st_TerminalStatus.psTermianlVer, 0 , LENGTH_TERMINAL_VER);//
    st_TerminalStatus.bEnabled = false;//
    st_TerminalStatus.cType = 0;//
    st_TerminalStatus.cStatus = 0;//
    st_TerminalStatus.en_ChargeStatusChangeType = CHARGE_STATUS_CHANGE_NON;//
    //    st_TerminalStatus.dtLastActiveTime;//
    memset(&st_TerminalStatus.stFrameRemoteSingle, 0, sizeof(FRAME_REMOTE_SINGLE));//
    memset(&st_TerminalStatus.stFrameRemoteMeSurement1, 0, sizeof(FRAME_REMOTE_MEASUREMENT1));//
    memset(&st_TerminalStatus.stFrameRemoteMeSurement2, 0, sizeof(FRAME_REMOTE_MEASUREMENT2));//
    memset(&st_TerminalStatus.stFrameBmsInfo, 0, sizeof(FRAME_BMS_INFO));//
    st_TerminalStatus.FreeModule = 0;//

    st_TerminalStatus.chargeResponseFlag = true;
    st_TerminalStatus.gunType = UNKNOWN;
    st_TerminalStatus.chargeManner = UNKNOWN;
    st_TerminalStatus.bTicketPrint = 0;
}

///
/// \brief ChargeService::InitChargeStep 初始化状态机
/// \param stChargeStep 待初始化的状态机
///
void ChargeService::InitChargeStep(CHARGE_STEP &stChargeStep)
{
    //-------------------指令订单共用---------------------------------//
    stChargeStep.ucCanAddr = 0;
    memset(&stChargeStep.sEventNo, 0, LENGTH_EVENT_NO);
    memset(&stChargeStep.sCardNo, 0, LENGTH_CARDNO);
    memset(&stChargeStep.sScanCodeNo, 0, LENGTH_GUID_NO);
    memset(&stChargeStep.sGUIDNo, 0, LENGTH_GUID_NO);
    memset(&stChargeStep.sVIN, 0, LENGTH_VIN_NO);
    memset(&stChargeStep.sCarLisence, 0, LENGTH_CAR_LISENCE);
    stChargeStep.ucQueueMsgFromServer = 0;
    //-------------------指令相关---------------------------------//
    stChargeStep.ucCmdSrc = 0;
    stChargeStep.ucCmdValue = 0;
    stChargeStep.ucStartReason = 0;
    memset(&stChargeStep.stZigeeAddr, 0, sizeof(ZIGBEE_ADDR));
    //-------------------指令状态机相关---------------------------------//
    QDateTime dtQDateTime= QDateTime::currentDateTime();
    QDateTime2CharArray(dtQDateTime, stChargeStep.sRecvTime);
    stChargeStep.enChargeStep = CHARGE_STEP_NORMAL;
    memset(&stChargeStep.stChargeStepValue, 0 ,sizeof(CHARGE_STEP_VALUE));
    stChargeStep.enCmdAck = CMD_ACK_TYPE_SUCCESS;
    memset(&stChargeStep.stRepeatCmd, 0 ,sizeof(REPEAT_CMD));
    stChargeStep.enCmdEndReason = CMD_END_REASON_NULL;
    //-------------------订单相关---------------------------------//
    InitOrderStatus(stChargeStep);
    //-------------------3.0协议--------------------------------//
    memset(stChargeStep.sBillCode, 0, LENGTH_BILL_CODE);
    stChargeStep.iCurveState = 0;
    stChargeStep.iCurveStart = 0;
    stChargeStep.iCurveStop = 0;
    stChargeStep.cCurveType = 1;
    stChargeStep.iCurveValue = 0.0;
}

///
/// \brief ChargeService::InitOrderStatus 初始化状态机中订单相关变量
/// \param stChargeStep 待初始化的状态机
///
void ChargeService::InitOrderStatus(CHARGE_STEP &stChargeStep)
{
    LogOut(QString("  UpateChargeStepOrder!"), 1);
    stChargeStep.enOrderStatus = ORDER_STATUS_NON;
    stChargeStep.u32EnergyStartCharge = 0;
    stChargeStep.u32EnergyEndCharge = 0;
    stChargeStep.u32TotalChargeEnergy = 0;
    stChargeStep.u32EnergyPausetCharge = 0;
    QDateTime dtQDateTime= QDateTime::currentDateTime();
    QDateTime2CharArray(dtQDateTime, stChargeStep.sStartTime);
    QDateTime2CharArray(dtQDateTime, stChargeStep.sEndTime);
    stChargeStep.sLastRecordEnergy[0] = 1;
    stChargeStep.sLastRecordEnergy[1] = 1;
    stChargeStep.u12TotalChargeTime = 0;
    stChargeStep.ucStartSOC = 0;
    stChargeStep.ucEndSOC = 0;
    stChargeStep.ucQueueMsg = 0;
    stChargeStep.ucStopReasonDev = 0;
    stChargeStep.ucStopReasonCloud = 0;
    stChargeStep.ucStopReasonCSCU = 0;
    stChargeStep.cOrderType = ORDER_NORMAL;
    stChargeStep.cQueueGroup = 0;
    stChargeStep.cChargeType = 0;
    stChargeStep.cChargeWay = CLOUD_START;
    stChargeStep.fLimitEnergy = 0;
    stChargeStep.ucQueueMsgFromServer = 0;
    stChargeStep.cGunNum = 1;
	stChargeStep.cOrderSync = 0;
    //stChargeStep.bChargeFreezeEnery = false;
}
//----------------------------------------------与刷卡充电接口----------------------------------------------//
void ChargeService::slot_RecvFromCardCharge(InfoMap qInfoMap, InfoAddrType InfoType)
{
    emit sigToBus(qInfoMap, InfoType);
}

//----------------------------------------------ChargeStep  timer private----------------------------------------------//

void ChargeService::ProcChargeStepLoop(CHARGE_STEP &stChargeStep)
{
    switch (stChargeStep.enChargeStep) {
    case CHARGE_STEP_WAITCMD_ACK://收到服务器指令后通过CAN总线发送CMD，等待ACK回复,超时时间5S
    case CHARGE_STEP_WAITDEV_START_CHARGE://等待启动充电，超时时间60S
    case CHARGE_STEP_WAITDEV_STOP_CHARGE: //等待结束充电，超时时间60S
    case CHARGE_STEP_WAITDEV_LIMIT_CHARGE://等待限制充电成功，超时时间60S
    case CHARGE_STEP_WAITDEV_PAUSH_CHARGE://等待暂停充电成功，超时时间60s
    case CHARGE_STEP_WAITDEV_RESUME_CHARGE://等待恢复充电成功，超时时间60S，让暂停充电的模块继续充电
    case CHARGE_STEP_WAITDEV_RESET_CHARGE://等待复位充电，超时时间60S，让限制充电的模块取消限制
    case CHARGE_STEP_WAITDEV_START_DISCHARGE://等待启动放电，超时时间60S
    case CHARGE_STEP_WAITDEV_STOP_DISCHARGE://等待结束放电，超时时间60S
    case CHARGE_STEP_WAITSEVER_ACK_APPLY_START_CHARGE_VIN://等待服务器返回VIN申请充电结果，超时时间15S
    case CHARGE_STEP_WAITSEVER_ACK_APPLY_STOP_CHARGE_VIN://等待服务器返回VIN申请充电结果，超时时间15S
    case CHARGE_STEP_WAITSEVER_ACK_APPLY_START_CHARGE_CARD://等待服务器返回刷卡申请充电结果，超时时间15S
    case CHARGE_STEP_WAITSEVER_ACK_APPLY_STOP_CHARGE_CARD://等待服务器返回刷卡申请结束充电结果，超时时间15S
    case CHARGE_STEP_WAITDEV_STOP_REASON://等待中止原因，超时时间2S,直流
        if(stChargeStep.stChargeStepValue.uc_hold_time-- <= 0){
            ProcChargeStepTimeOut(stChargeStep);
        }
        else{
            ProcChargeStepNoTimeOut(stChargeStep);
        }
        break;
    default:
        break;
    }
}


///
/// \brief ChargeService::ProcChargeStepTimeOut 充电状态机超时处理
/// \param stChargeStep 已有的状态机
///
void ChargeService::ProcChargeStepTimeOut(CHARGE_STEP &stChargeStep)
{
    switch (stChargeStep.enChargeStep) {
    case CHARGE_STEP_WAITCMD_ACK://收到服务器指令后通过CAN总线发送CMD，等待ACK回复,超时时间5S
        stChargeStep.enCmdEndReason = CMD_END_REASON_WAITACK_TIMEOUT;
//        switch (stChargeStep.ucCmdValue) {
//        case CHARGE_CMD_TYPE_START_CHARGE_NOW://01H 立即充电
//        case CHARGE_CMD_TYPE_START_CHARGE_ECONOMIC:// 02H 经济充电
//        case CHARGE_CMD_TYPE_START_DISCHARGE://0AH 开始放电
//            stChargeStep.ucStopReasonCSCU = 28;
//            break;
//        default:
//            break;
//        }
        LogOut(QString("等待ACK回复,超时  CAN = %1!").arg(stChargeStep.ucCanAddr), 3);
        break;
    case CHARGE_STEP_WAITDEV_START_CHARGE://等待启动充电，超时时间85S
    case CHARGE_STEP_WAITDEV_RESUME_CHARGE://等待恢复充电成功，超时时间85S，让暂停充电的模块继续充电
        stChargeStep.enCmdEndReason = CMD_END_REASON_START_CHARGE_TIMEOUT;
        stChargeStep.ucStopReasonCSCU = 80;
        LogOut(QString("等待启动充电，超时时间85S  CAN = %1!").arg(stChargeStep.ucCanAddr), 3);
        break;
    case CHARGE_STEP_WAITDEV_STOP_CHARGE: //等待结束充电，超时时间60S
        stChargeStep.enCmdEndReason = CMD_END_REASON_STOP_CHARGE_TIMEOUT;
        LogOut(QString("等待结束充电，超时时间60S  CAN = %1!").arg(stChargeStep.ucCanAddr), 3);
        break;
    case CHARGE_STEP_WAITDEV_LIMIT_CHARGE://等待限制充电成功，超时时间60S
    case CHARGE_STEP_WAITDEV_PAUSH_CHARGE://等待暂停充电成功，超时时间60s
    case CHARGE_STEP_WAITDEV_RESET_CHARGE://等待复位充电，超时时间60S，让限制充电的模块取消限制
    case CHARGE_STEP_WAITDEV_START_DISCHARGE://等待启动放电，超时时间60S
        stChargeStep.enCmdEndReason = CMD_END_REASON_START_DISCHARGE_TIMEOUT;
        stChargeStep.ucStopReasonCSCU = 81;
        LogOut(QString("等待限制\\暂停\\复位\\启动放电，超时时间60S  CAN = %1!").arg(stChargeStep.ucCanAddr), 3);
        break;
    case CHARGE_STEP_WAITDEV_STOP_DISCHARGE://等待结束放电，超时时间60S
        stChargeStep.enCmdEndReason = CMD_END_REASON_STOP_DISCHARGE_TIMEOUT;
        break;
    case CHARGE_STEP_WAITSEVER_ACK_APPLY_START_CHARGE_VIN://等待服务器返回VIN申请充电结果，超时时间15S
    case CHARGE_STEP_WAITSEVER_ACK_APPLY_STOP_CHARGE_VIN://等待服务器返回VIN申请充电结果，超时时间15S
        stChargeStep.ucStartReason = START_CHARGE_CMD_RSN_NON;
        stChargeStep.enCmdEndReason = CMD_END_REASON_NULL;
        LogOut(QString("等待服务器返回VIN申请充电结果，超时时间15S  CAN = %1!").arg(stChargeStep.ucCanAddr), 3);
        break;
    case CHARGE_STEP_WAITSEVER_ACK_APPLY_START_CHARGE_CARD://等待服务器返回刷卡申请充电结果，超时时间15S
    case CHARGE_STEP_WAITSEVER_ACK_APPLY_STOP_CHARGE_CARD://等待服务器返回刷卡申请结束充电结果，超时时间15S
        stChargeStep.ucStartReason = START_CHARGE_CMD_RSN_NON;
        stChargeStep.enCmdEndReason = CMD_END_REASON_NULL;
        LogOut(QString("等待服务器返回刷卡申请充电结果，超时时间15S  CAN = %1!").arg(stChargeStep.ucCanAddr), 3);
        break;
    case CHARGE_STEP_WAITDEV_STOP_REASON://等待中止原因，超时时间2S,直流
        break;
    default:
        break;
    }
    stChargeStep.enChargeStep = stChargeStep.stChargeStepValue.uc_charge_step_timeout;
    QDateTime dtQDateTime = QDateTime::currentDateTime();
    QDateTime2CharArray(dtQDateTime, stChargeStep.stChargeStepValue.sStartTimeStep);
    stChargeStep.stChargeStepValue.uc_charge_step_timeout = CHARGE_STEP_NORMAL;
    stChargeStep.stChargeStepValue.uc_hold_time = g_TimeoutChargeStep[stChargeStep.enChargeStep];

    //发送指令结果(正常或者异常)
    //if(stChargeStep.enCmdEndReason != CMD_END_REASON_NULL){
     if(stChargeStep.enCmdEndReason != CMD_END_REASON_NULL &&
              stChargeStep.enCmdEndReason != CMD_END_REASON_WAITACK_TIMEOUT){
        //发送给server的结果
        SendCmdResult(stChargeStep);
        //发送给显示屏的结果
        if(stChargeStep.ucCmdSrc == CHARGE_CMD_SRC_CSCU && stChargeStep.ucStartReason == START_CHARGE_CMD_RSN_VIN6){
            SendVINViaApplyChargeResult(stChargeStep);
        }
    }

    switch (stChargeStep.enCmdEndReason) {
    case CMD_END_REASON_START_CHARGE_TIMEOUT:
    case CMD_END_REASON_START_DISCHARGE_TIMEOUT:
    //case CMD_END_REASON_WAITACK_TIMEOUT://对于开始充电指令,但是回复ACK超时的,也算作失败订单
        switch (stChargeStep.ucCmdValue) {
        case CHARGE_CMD_TYPE_START_CHARGE_NOW://01H 立即充电
        case CHARGE_CMD_TYPE_START_CHARGE_ECONOMIC:// 02H 经济充电
        case CHARGE_CMD_TYPE_START_DISCHARGE://0AH 开始放电
        {
            TerminalStatus stTerminalStatus = stEmptyTerminalStatus;
            gpDevCache->QueryTerminalStatus(stChargeStep.ucCanAddr, stTerminalStatus);
            SaveStartChargeOrderMsg(stTerminalStatus, stChargeStep);
            stChargeStep.enOrderStatus = ORDER_STATUS_FAIL;//更新订单相关
            SaveStopChargeOrderMsg(stTerminalStatus, stChargeStep);
            TerminateOrder(stChargeStep);
        }
            break;
        default:
            break;
        }
        break;
    case CMD_END_REASON_WAITACK_TIMEOUT://对于开始充电指令,但是回复ACK超时的,不算作失败订单 hd 2018-8-21
    {
        switch (stChargeStep.ucCmdValue) {
        case CHARGE_CMD_TYPE_START_CHARGE_NOW://01H 立即充电
        case CHARGE_CMD_TYPE_START_CHARGE_ECONOMIC:// 02H 经济充电
            //LogOut(QString("  !"), 1);
            //交流终端没有等待开始充电这一步骤
            //if((stTempChargeStep.ucCanAddr <= ID_MaxACSinCanID) && (stTempChargeStep.ucCanAddr >= ID_MinACSinCanID))
           // {
           //     stTempChargeStep.enChargeStep = CHARGE_STEP_NORMAL ;
          //  }
          //  else
           // {
                stChargeStep.enChargeStep = CHARGE_STEP_WAITDEV_START_CHARGE ;
           // }
            break;
        case CHARGE_CMD_TYPE_STOP_CHARGE://03H 终止充电
        case CHARGE_CMD_TYPE_STOP_CHARGE_URGENCY://04H 紧急停止充电
            //交流终端没有等待开始充电这一步骤
//            if((stTempChargeStep.ucCanAddr <= ID_MaxACSinCanID) && (stTempChargeStep.ucCanAddr >= ID_MinACSinCanID))
//            {
//                stTempChargeStep.enChargeStep = CHARGE_STEP_NORMAL ;
//            }
//            else
//            {
                stChargeStep.enChargeStep =  CHARGE_STEP_WAITDEV_STOP_CHARGE;
//            }
            break;
        case CHARGE_CMD_TYPE_LIMIT_CHARGE://05H 限制充电,对充电中的设备进行限功率
            stChargeStep.enChargeStep =  CHARGE_STEP_WAITDEV_LIMIT_CHARGE;
            break;
        case CHARGE_CMD_TYPE_PAUSH_CHARGE://06H 暂停充电
            stChargeStep.enChargeStep =  CHARGE_STEP_WAITDEV_PAUSH_CHARGE;
            break;
        case CHARGE_CMD_TYPE_ADJUST_CHARGE://07H 调整充电电流
            stChargeStep.enChargeStep =  CHARGE_STEP_NORMAL;
            break;
        case CHARGE_CMD_TYPE_RESUME://08H 恢复充电，让暂停充电的模块继续充电
            stChargeStep.enChargeStep =  CHARGE_STEP_WAITDEV_RESUME_CHARGE;
            break;
        case CHARGE_CMD_TYPE_RESET://09H 复位，让限制充电的模块取消限制
            stChargeStep.enChargeStep =  CHARGE_STEP_WAITDEV_RESET_CHARGE;
            break;
        case CHARGE_CMD_TYPE_START_DISCHARGE://0AH 开始放电
            stChargeStep.enChargeStep =  CHARGE_STEP_WAITDEV_START_DISCHARGE;
            break;
        case CHARGE_CMD_TYPE_STOP_DISCHARGE://0BH 结束放电
            stChargeStep.enChargeStep =  CHARGE_STEP_WAITDEV_STOP_DISCHARGE;
            break;
        default:
            break;
        }
        stChargeStep.stChargeStepValue.uc_hold_time  = g_TimeoutChargeStep[stChargeStep.enChargeStep];
        stChargeStep.stChargeStepValue.uc_charge_step_timeout = CHARGE_STEP_NORMAL;
        QDateTime dtQDateTime = QDateTime::currentDateTime();
        QDateTime2CharArray(dtQDateTime, stChargeStep.stChargeStepValue.sStartTimeStep);
        //stChargeStep.enCmdEndReason = CMD_END_REASON_ACK_OK;//响应成功
        LogOut(QString("ACK超时,更新队列!"), 1);
        //stChargeStep = stTempChargeStep;
        //TempOutPutChargeStep(stTempChargeStep);
        //gpDevCache->UpateChargeStep(stTempChargeStep.ucCanAddr, stTempChargeStep);
        //二期可以根据状态机指令来源,选择是否发送响应结果
       // SendCmdResult(stTempChargeStep);
        //发送给显示屏的结果
        //if(stChargeStep.ucCmdSrc == CHARGE_CMD_SRC_CSCU && stChargeStep.ucStartReason == START_CHARGE_CMD_RSN_VIN6){
        //    SendVINViaApplyChargeResult(stChargeStep);
      //  }
    }
        break;
    default:
        break;
    }
    TempOutPutChargeStep(stChargeStep);
    gpDevCache->UpateChargeStep(stChargeStep.ucCanAddr, stChargeStep);
    //数据持久化
    SaveChargeStep(stChargeStep);
}


void ChargeService::ProcChargeStepNoTimeOut(CHARGE_STEP &stChargeStep)
{
    LogOut(QString("ProcChargeStepNoTimeOut!"), 1);
    switch (stChargeStep.enChargeStep) {
    case CHARGE_STEP_WAITCMD_ACK://收到服务器指令后通过CAN总线发送CMD，等待ACK回复,超时时间5S
        if(stChargeStep.stRepeatCmd.NowCount < stChargeStep.stRepeatCmd.MaxCount
                && stChargeStep.stRepeatCmd.NowCount > 0 && stChargeStep.stRepeatCmd.NowCount % 2 == 0){//指令重发设置为2秒发送一次
            InfoMap TempqInfoMap;
            InfoAddrType TempInfoType;

            LogOut(QString("指令重发,第%1次!").arg(stChargeStep.stRepeatCmd.NowCount / 2), 3);
            PackageTelecontrol2ChargeEquipment(stChargeStep, TempqInfoMap, TempInfoType);
            emit sigToBus(TempqInfoMap, TempInfoType);
        }
        else if(stChargeStep.stRepeatCmd.NowCount != 0
                && stChargeStep.stRepeatCmd.NowCount >= stChargeStep.stRepeatCmd.MaxCount){
            LogOut(QString("指令重发已完成!"), 1);
        }
        stChargeStep.stRepeatCmd.NowCount ++;
        break;
    case CHARGE_STEP_WAITDEV_START_CHARGE://等待启动充电，超时时间60S
    case CHARGE_STEP_WAITDEV_STOP_CHARGE: //等待结束充电，超时时间60S
    case CHARGE_STEP_WAITDEV_LIMIT_CHARGE://等待限制充电成功，超时时间60S
    case CHARGE_STEP_WAITDEV_PAUSH_CHARGE://等待暂停充电成功，超时时间60s
    case CHARGE_STEP_WAITDEV_RESUME_CHARGE://等待恢复充电成功，超时时间60S，让暂停充电的模块继续充电
    case CHARGE_STEP_WAITDEV_RESET_CHARGE://等待复位充电，超时时间60S，让限制充电的模块取消限制
    case CHARGE_STEP_WAITDEV_START_DISCHARGE://等待启动放电，超时时间60S
    case CHARGE_STEP_WAITDEV_STOP_DISCHARGE://等待结束放电，超时时间60S
    case CHARGE_STEP_WAITSEVER_ACK_APPLY_START_CHARGE_VIN://等待服务器返回VIN申请充电结果，超时时间15S
    case CHARGE_STEP_WAITSEVER_ACK_APPLY_STOP_CHARGE_VIN://等待服务器返回VIN申请充电结果，超时时间15S
    case CHARGE_STEP_WAITSEVER_ACK_APPLY_START_CHARGE_CARD://等待服务器返回刷卡申请充电结果，超时时间15S
    case CHARGE_STEP_WAITSEVER_ACK_APPLY_STOP_CHARGE_CARD://等待服务器返回刷卡申请结束充电结果，超时时间15S
    case CHARGE_STEP_WAITDEV_STOP_REASON://等待中止原因，超时时间2S,直流
        break;
    default:
        break;
    }
    TempOutPutChargeStep(stChargeStep);
    gpDevCache->UpateChargeStep(stChargeStep.ucCanAddr, stChargeStep);
}

///
/// \brief ChargeService::ProChargeEnergyRecordLoop 循环记录充电过程中的电量
/// \param stChargeStep
/// \return
///
bool ChargeService::ProChargeEnergyRecordLoop(CHARGE_STEP &stChargeStep)
{
    QDateTime dtNowTime = QDateTime::currentDateTime();
    if(stChargeStep.enOrderStatus == ORDER_STATUS_ING){
        __u8 hh= dtNowTime.toString("hh").toInt();
        __u8 mm= dtNowTime.toString("mm").toInt();

        if((mm==0||mm==30)&&((stChargeStep.sLastRecordEnergy[0] != hh)||(stChargeStep.sLastRecordEnergy[1] != mm)) ){

            //查询数据库中最后一笔冻结电量,判断是否在同一时刻,防止同一个零点或半点时刻多次插入
            if(!CheckProChargeEnergyRecordLoop(stChargeStep))
            {
                return false;
            }
            if(!CheckTimeAcrossIsMoreThan30(stChargeStep,dtNowTime))
            {
                return false;
            }

            LogOut(QString("冻结 CAN = %1!").arg(stChargeStep.ucCanAddr), 2);
            stChargeStep.sLastRecordEnergy[0] = hh;
            stChargeStep.sLastRecordEnergy[1] = mm;
            TerminalStatus stTerminalStatus;
            if(gpDevCache->QueryTerminalStatus(stChargeStep.ucCanAddr, stTerminalStatus) == 0){
                return false;
            }
            iStartOrStopFlag = 0;
            SaveChargeEnergyToDB(stChargeStep, stTerminalStatus,iStartOrStopFlag);
            LogOut(QString("ProChargeEnergyRecordLoop SaveChargeEnergyToDB CAN = %1").arg(stChargeStep.ucCanAddr), 2);
            return true;
        }
    }
    return false;
}

///
/// \brief ChargeService::ProStartSocCheckLoop 循环判断是否获取到起始SOC
/// \param stChargeStep
/// \return
///
bool ChargeService::ProStartSocCheckLoop(CHARGE_STEP &stChargeStep)
{
    if(stChargeStep.enOrderStatus == ORDER_STATUS_ING){
        if(stChargeStep.ucStartSOC == 0){
            TerminalStatus stTerminalStatus;
            if(gpDevCache->QueryTerminalStatus(stChargeStep.ucCanAddr, stTerminalStatus) == 0){
                return false;
            }
            if(stTerminalStatus.stFrameBmsInfo.batery_SOC != 0){
                stChargeStep.ucStartSOC = stTerminalStatus.stFrameBmsInfo.batery_SOC;
                LogOut(QString("UPDATE StartSOC OK CAN = %1!").arg(stChargeStep.ucCanAddr), 2);
                return true;
            }
        }
    }
    return false;
}


bool ChargeService::DoorOpenStopChargeCheckLoop(CHARGE_STEP &stChargeStep)
{
    //if(stChargeStep.enOrderStatus == ORDER_STATUS_ING){//有正在进行的充电业务
    //change by wbw 2018-08-22 有正在进行的充电业务 且 之前的开机指令完成
    if(stChargeStep.enOrderStatus == ORDER_STATUS_ING \
            && stChargeStep.enChargeStep == CHARGE_STEP_NORMAL \
            && stChargeStep.ucCmdValue == CHARGE_CMD_TYPE_START_CHARGE_NOW){
        return true;
    }
    else{
        return false;
    }
}

bool ChargeService::ProEnergyTrustableCheckLoop(CHARGE_STEP &stChargeStep)
{
    if(stChargeStep.enOrderStatus == ORDER_STATUS_NON){//非有效业务,直接返回
        return false;
    }
    if(DataTrustableFlagMap.contains(stChargeStep.ucCanAddr) == false){//无需判断是否可信,直接返回
        return false;
    }
    else if(DataTrustableFlagMap[stChargeStep.ucCanAddr] == true){//已经为可信,则直接返回
        return false;
    }

    TerminalStatus stTerminalStatus;
    if(gpDevCache->QueryTerminalStatus(stChargeStep.ucCanAddr, stTerminalStatus) == 0){//终端未查到,直接返回
        return false;
    }
    else if(stTerminalStatus.validFlag != ALL_VALID_NUMBER){//数据仍然无效,直接返回
        return false;
    }

    switch (stTerminalStatus.en_ChargeStatusChangeType) {
    case CHARGE_STATUS_CHANGE_START_CHARGE:
        stChargeStep.u32EnergyStartCharge = stTerminalStatus.stFrameRemoteMeSurement2.active_electric_energy;
		UpdateBeginBill(stChargeStep);
    	sendOrderStatus(stChargeStep, 1);
        break;
    case CHARGE_STATUS_CHANGE_STOP_CHARGE:
        stChargeStep.u32EnergyEndCharge = stTerminalStatus.stFrameRemoteMeSurement2.active_electric_energy;
		UpdateEndBill(stChargeStep);
    	sendOrderStatus(stChargeStep, 2);
        break;
    case CHARGE_STATUS_CHANGE_START_DISCHARGE:
        stChargeStep.u32EnergyStartCharge = stTerminalStatus.stFrameRemoteMeSurement2.ReverseActiveEnergy;
		UpdateBeginBill(stChargeStep);
    	sendOrderStatus(stChargeStep, 1);
        break;
    case CHARGE_STATUS_CHANGE_STOP_DISCHARGE:
        stChargeStep.u32EnergyEndCharge = stTerminalStatus.stFrameRemoteMeSurement2.ReverseActiveEnergy;
		UpdateEndBill(stChargeStep);
    	sendOrderStatus(stChargeStep, 2);
        break;
    default:
        stChargeStep.u32EnergyStartCharge = stTerminalStatus.stFrameRemoteMeSurement2.active_electric_energy;
        break;
    }

    DataTrustableFlagMap.remove(stChargeStep.ucCanAddr);

    iStartOrStopFlag = 0;
    QDateTime dtNowTime = QDateTime::currentDateTime();
    if(SaveChargeEnergyToDB(stChargeStep, stTerminalStatus,iStartOrStopFlag) == false){
        LogOut(QString("ProEnergyTrustableCheckLoop UPDATE Energy Failed CAN = %1!").arg(stChargeStep.ucCanAddr), 2);
        return false;
    }
    return true;
}


///
/// \brief ChargeService::ProStopReasonDevCheckLoop 循环判断是否获取到设备上传的中止原因
/// \param stChargeStep
/// \return
///
bool ChargeService::ProStopReasonDevCheckLoop(CHARGE_STEP &stChargeStep)
{
    if(stChargeStep.enOrderStatus == ORDER_STATUS_FAIL || stChargeStep.enOrderStatus == ORDER_STATUS_OK){

        if(stChargeStep.ucStopReasonDev == 0){
            TerminalStatus stTerminalStatus;
            if(gpDevCache->QueryTerminalStatus(stChargeStep.ucCanAddr, stTerminalStatus) == 0){
                return false;
            }

            if(stTerminalStatus.stFrameRemoteSingle.Stop_Result != 0){
                stChargeStep.ucStopReasonDev = stTerminalStatus.stFrameRemoteSingle.Stop_Result;
                if(UpdateStopReasonDev2DB(stChargeStep) == false){
                    return false;
                }else{//更新数据库成功,返回成功
                    LogOut(QString("UPDATE DevStopReason OK CAN = %1 CODE = %2!")\
                           .arg(stChargeStep.ucCanAddr).arg(stChargeStep.ucStopReasonDev), 2);
                    return true;
                }
            }

        }
    }
    return false;
}

///
/// \brief ChargeService::UpdateStopReasonDev2DB 将设备中止原因更新至充电订单数据库
/// \param stChargeStep
/// \return
///
bool ChargeService::UpdateStopReasonDev2DB(CHARGE_STEP &stChargeStep)
{
    QString todo = QString("UPDATE charge_order SET DevStopReason = %1 where UUIDOwn = '%2'")\
            .arg(stChargeStep.ucStopReasonDev).arg(QByteArray::fromRawData(stChargeStep.sOrderUUID, LENGTH_GUID_NO).toHex().data());
    if(pDBOperate->DBSqlExec(todo.toAscii().data(), DB_PROCESS_RECORD) !=0){
        LogOut(QString("UPDATE DevStopReason fail CAN = %1!").arg(stChargeStep.ucCanAddr), 3);
        return false;
    }
    return true;
}


//----------------------------------------------ChargingData Save  timer private----------------------------------------------//
///
/// \brief ChargeService::ProChargingDataRecordLoop 定时保存充电过程中摇测数据
/// \param stChargeStep
/// \return 保存成功,保存失败
///
bool ChargeService::ProChargingDataRecordLoop(CHARGE_STEP &stChargeStep)
{
    QDateTime dtNowTime = QDateTime::currentDateTime();
    if(stChargeStep.enOrderStatus == ORDER_STATUS_ING){
        QDateTime dtStartChargeTime;
        CharArray2QDateTime(stChargeStep.sStartTime, dtStartChargeTime);
        if(dtStartChargeTime.isValid()){
            if((dtStartChargeTime.secsTo(dtNowTime)/60)%5 == 0){//从开始充电每5分钟记录一次
                TerminalStatus stTerminalStatus;
                if(gpDevCache->QueryTerminalStatus(stChargeStep.ucCanAddr, stTerminalStatus) == 0){
                    return false;
                }
                if(SaveChargingDataToDB(stChargeStep, stTerminalStatus) == true){
                    return true;
                }
            }
        }
    }
    return false;
}

///
/// \brief ChargeService::ProChargingBMSRecordLoop 定时保存充电过程中BMS数据
/// \param stChargeStep
/// \return
///
bool ChargeService::ProChargingBMSRecordLoop(CHARGE_STEP &stChargeStep)
{
    QDateTime dtNowTime = QDateTime::currentDateTime();
    if(stChargeStep.enOrderStatus == ORDER_STATUS_ING){
        QDateTime dtStartChargeTime;
        CharArray2QDateTime(stChargeStep.sStartTime, dtStartChargeTime);
        if(dtStartChargeTime.isValid()){
            if((dtStartChargeTime.secsTo(dtNowTime)/60)%6 == 0){//从开始充电每6分钟记录一次
                TerminalStatus stTerminalStatus;
                if(gpDevCache->QueryTerminalStatus(stChargeStep.ucCanAddr, stTerminalStatus) == 0){
                    return false;
                }
                SaveChargingBMSToDB(stChargeStep, stTerminalStatus);
                return true;
            }
        }
    }
    return false;
}

///
/// \brief ChargeService::SaveChargingDataToDB 保存摇测数据
/// \param stChargeStep
/// \param stTerminalStatus
/// \return
///
bool ChargeService::SaveChargingDataToDB(CHARGE_STEP &stChargeStep, TerminalStatus &stTerminalStatus)
{
    QString qstrChargingDataTableName = "charge_process_"+QString::number(stChargeStep.ucCanAddr, 10) +"_table";
    QString TableColumnsNames = \
            "record_time, guid_number,voltage_a, voltage_b, voltage_c, current_a, current_b, current_c\
            , total_active_power, total_reactive_power\
            , total_power_factor, zero_line_current, voltage_unbalance_rate, current_unbalance_rate, voltage_dc, current_dc, stage_status\
            ,total_active_energy, total_reactive_energy";
            QString todo = QString("INSERT INTO '%1' (%2) values ('%3', '%4', %5, %6, %7, %8, %9, %10, %11, %12, %13, %14, %15, %16, %17, %18, %19, %20, %21)")\
            .arg(qstrChargingDataTableName)\
            .arg(TableColumnsNames)\
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))\
            .arg(QByteArray::fromRawData(stChargeStep.sOrderUUID, LENGTH_GUID_NO).toHex().data())\
            .arg(stTerminalStatus.stFrameRemoteMeSurement1.A_voltage)\
            .arg(stTerminalStatus.stFrameRemoteMeSurement1.B_voltage)\
            .arg(stTerminalStatus.stFrameRemoteMeSurement1.C_voltage)\
            .arg(stTerminalStatus.stFrameRemoteMeSurement1.A_current)\
            .arg(stTerminalStatus.stFrameRemoteMeSurement1.B_current)\
            .arg(stTerminalStatus.stFrameRemoteMeSurement1.C_current)\
            .arg(stTerminalStatus.stFrameRemoteMeSurement1.active_power)\
            .arg(stTerminalStatus.stFrameRemoteMeSurement1.reactive_power)\
            .arg(stTerminalStatus.stFrameRemoteMeSurement1.power_factor)\
            .arg(stTerminalStatus.stFrameRemoteMeSurement1.neutralLine_current)\
            .arg(stTerminalStatus.stFrameRemoteMeSurement1.voltage_unbalance_rate)\
            .arg(stTerminalStatus.stFrameRemoteMeSurement1.current_unbalance_rate)\
            .arg(stTerminalStatus.stFrameRemoteMeSurement1.voltage_of_dc)\
            .arg(stTerminalStatus.stFrameRemoteMeSurement1.current_of_dc)\
            .arg(0)\
            .arg(QString::number((double)stTerminalStatus.stFrameRemoteMeSurement2.active_electric_energy / 100.0, 'f', 2))\
            .arg(QString::number((double)stTerminalStatus.stFrameRemoteMeSurement2.ReverseActiveEnergy / 100.0, 'f', 2));
    if(pDBOperate->DBSqlExec(todo.toAscii().data(), DB_REAL_RECORD) != 0){
        LogOut(QString("Insert table %1 false!").arg(qstrChargingDataTableName), 3);
        return false;
    }

    LogOut(QString("SaveChargingDataToDB OK CAN = %1").arg(stChargeStep.ucCanAddr), 1);
    return true;
}

///
/// \brief ChargeService::SaveChargingBMSToDB 保存BMS数据
/// \param stChargeStep
/// \param stTerminalStatus
/// \return
///
bool ChargeService::SaveChargingBMSToDB(CHARGE_STEP &stChargeStep, TerminalStatus &stTerminalStatus)
{
    QString qstrBmsDynamicTableName = "bms_dynamic_"+QString::number(stChargeStep.ucCanAddr, 10) +"_table";
    QString TableColumnsNames = \
            "record_time, guid_number, need_voltage, need_current, charge_mode, voltage_measure\
            , current_measure, max_single_voltage, max_single_voltage_num, current_soc, charge_left_time\
            , max_tempeture, max_tempeture_num, min_tempeture, min_tempeture_num, bms_charge_enable";
            QString todo = QString("INSERT INTO '%1' (%2) values ('%3', '%4', %5, %6, %7, %8, %9, %10, %11, %12, %13, %14, %15, %16, %17,%18)")\
            .arg(qstrBmsDynamicTableName)\
            .arg(TableColumnsNames)\
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))\
            .arg(QByteArray::fromRawData(stChargeStep.sOrderUUID, LENGTH_GUID_NO).toHex().data())\
            .arg(stTerminalStatus.stFrameBmsInfo.BMS_need_voltage)\
            .arg(stTerminalStatus.stFrameBmsInfo.BMS_need_current )\
            .arg(0)\
            .arg(0)\
            .arg(0)\
            .arg(stTerminalStatus.stFrameBmsInfo.max_batery_voltage)\
            .arg(0)\
            .arg(stTerminalStatus.stFrameBmsInfo.batery_SOC)\
            .arg(0)\
            .arg(stTerminalStatus.stFrameBmsInfo.max_batery_temperature)\
            .arg(0)\
            .arg(stTerminalStatus.stFrameBmsInfo.lowest_battery_temperature)\
            .arg(0)\
            .arg(0);
    if(pDBOperate->DBSqlExec(todo.toAscii().data(), DB_REAL_RECORD) != 0){
        LogOut(QString("Insert table %1 false!").arg(qstrBmsDynamicTableName), 3);
        return false;
    }
    LogOut(QString("SaveChargingBMSToDB  OK CAN = %1").arg(stChargeStep.ucCanAddr), 1);
    return true;
}





///
/// \brief ChargeService::slot_ProcChargeStepTimeOut 充电业务状态机定时超时处理函数
///
void ChargeService::slot_ProcChargeStepTimeOut()
{
    CHARGE_STEP stChargeStep;//存放从缓存中读出的状态机
    //TerminalStatus stTerminalStatus;
    __u8 ucCanAddr = 0;
    __u8 ucIndexID = 0;//起始地址

    for(int i = 0; i< 3 ; i++){
        switch (i) {
        case 0://单相
            ucIndexID  = 1;
            break;
        case 1://三相
            ucIndexID  = 151;
            break;
        case 2://直流
            ucIndexID  = 181;
            break;
        default:
            break;
        }
        if(ucNumTerminal[i] > 0){
            for(int j = 0; j < ucNumTerminal[i]; j++){
                ucCanAddr = ucIndexID+j;
                if(gpDevCache->QueryChargeStep(ucCanAddr, stChargeStep)){
                    //循环处理充电状态机
                    ProcChargeStepLoop(stChargeStep);
                    //循环记录充电过程中的电量
                    if(ProChargeEnergyRecordLoop(stChargeStep)){
                        gpDevCache->UpateChargeStep(ucCanAddr, stChargeStep);
                    }
                    //循环判断是否获取到起始SOC
                    if(ProStartSocCheckLoop(stChargeStep)){
                        gpDevCache->UpateChargeStep(ucCanAddr, stChargeStep);
                    }
                    //循环判断是否获取到设备上传的中止原因
                    if(ProStopReasonDevCheckLoop(stChargeStep)){
                        gpDevCache->UpateChargeStep(ucCanAddr, stChargeStep);
                    }
                    //循环判断数据有效性
                    if(ProEnergyTrustableCheckLoop(stChargeStep)){
                        gpDevCache->UpateChargeStep(ucCanAddr, stChargeStep);
                    }
                    //开门报警触发情况下,循环执行动作
                    if(DoorOpenStopCharge){
                        if(DoorOpenStopChargeCheckLoop(stChargeStep)){
                            StopChargeByDoorOpen(stChargeStep);
                        }
                    }

                    //集控启动后判断是否有冻结电量时间点缺失
                    //if(!stChargeStep.bChargeFreezeEnery)
                    if(!bChargeFreezeEnery.contains(ucCanAddr)  || (bChargeFreezeEnery.contains(ucCanAddr) && bChargeFreezeEnery[ucCanAddr] ==false))
                    {
                        bStopFlag = false;
                        ChargeFreezeEneryCheck(bStopFlag,stChargeStep);
                    }
                    //else

                    //执行功率曲线
                    execPowerCurve(stChargeStep);
                }
                else{
                    continue;
                }
            }
        }
    }

}

///
/// \brief ChargeService::slot_ProcSaveChargingDataTimeOut
///
///存放1.充电过程摇测数据 2.充电过程中BMS数据
///
void ChargeService::slot_ProcSaveChargingDataTimeOut()
{
    CHARGE_STEP stChargeStep;//存放从缓存中读出的状态机
    __u8 ucCanAddr = 0;
    __u8 ucIndexID = 0;//起始地址

    for(int i = 0; i< 3 ; i++){//一层循环,按照充电终端种类
        switch (i) {
        case 0://单相
            ucIndexID  = 1;
            break;
        case 1://三相
            ucIndexID  = 151;
            break;
        case 2://直流
            ucIndexID  = 181;
            break;
        default:
            break;
        }
        if(ucNumTerminal[i] > 0){
            for(int j = 0; j < ucNumTerminal[i]; j++){//二层循环,按照每种终端个数
                ucCanAddr = ucIndexID+j;
                if(gpDevCache->QueryChargeStep(ucCanAddr, stChargeStep)){
                    if(ProChargingDataRecordLoop(stChargeStep)){
                        LogOut(QString("Save Charging data ok CAN = %1").arg(stChargeStep.ucCanAddr), 1);
                    }
                    if(i == 2){//直流终端记录BMS信息
                        if(ProChargingBMSRecordLoop(stChargeStep)){
                            LogOut(QString("Save Charging BMS data ok CAN = %1").arg(stChargeStep.ucCanAddr), 1);
                        }
                    }
                }

            }//二层循环结束
        }

    }//一层循环结束
}

void ChargeService::slot_ProcActiveUpateLogicStatusTimeOut()
{
    TerminalStatus stTerminalStatus;//存放从缓存中读出的充电终端状态
    __u8 ucCanAddr = 0;
    __u8 ucIndexID = 0;//起始地址
    QDateTime NowTime = QDateTime::currentDateTime();
    static QMap <unsigned char  , char > UpadateStatusMap;

    if(ChargeServiceLaunchTime.secsTo(NowTime) > ACTIVE_UPDATE_LOGIC_STATUS_TIMEOUT){//超时后,停止计数,删除定时器
        LogOut(QString("定时器超时返回, 共主动更新过%1个充电终端逻辑工作状态").arg(UpadateStatusMap.size()), 2);
        UpadateStatusMap.clear();
        if(tmActiveUpateLogicStatus->isActive()){
            tmActiveUpateLogicStatus->stop();
            delete tmActiveUpateLogicStatus;
        }
        return;
    }
    else if(ChargeServiceLaunchTime.secsTo(NowTime) < 0){//为了防止系统对时后,导致启动时间一直早于当前时间
        ChargeServiceLaunchTime = QDateTime::currentDateTime();
    }

    for(int i = 0; i< 3 ; i++){//一层循环,按照充电终端种类
        switch (i) {
        case 0://单相
            ucIndexID  = 1;
            break;
        case 1://三相
            ucIndexID  = 151;
            break;
        case 2://直流
            ucIndexID  = 181;
            break;
        default:
            break;
        }
        if(ucNumTerminal[i] > 0){
            for(int j = 0; j < ucNumTerminal[i]; j++){//二层循环,按照每种终端个数
                ucCanAddr = ucIndexID+j;
                if(gpDevCache->QueryTerminalStatus(ucCanAddr, stTerminalStatus) == true){
                    if(stTerminalStatus.validFlag != ALL_VALID_NUMBER){
                        continue;
                    }
                    if(stTerminalStatus.stFrameRemoteSingle.charge_status == 0
                            && stTerminalStatus.stFrameRemoteSingle.link_status == 0){//若为默认值,则需要主动更新

                        if(UpadateStatusMap.contains(ucCanAddr)){//查看是否有更新记录
                            if(UpadateStatusMap[ucCanAddr] == true){//已经更新成功,跳过
                                continue;
                            }
                            else{//没有更新成功,更新
                                UpadeLogicChargeStatusForTerminalSingle(stTerminalStatus, false,stTerminalStatus.stFrameRemoteSingle.charge_status);
                                UpadateStatusMap[ucCanAddr] = true;//修改成更新成功
                            }
                        }
                        else{//若没有更新记录,则先添加更新记录,然后更新
                            UpadateStatusMap.insert(ucCanAddr, false);
                            if(UpadeLogicChargeStatusForTerminalSingle(stTerminalStatus, false,stTerminalStatus.stFrameRemoteSingle.charge_status) == true){
                                UpadateStatusMap.insert(ucCanAddr, true);
                            }
                        }

                    }
                }

            }//二层循环结束
        }

    }//一层循环结束

}


///
/// \brief ChargeService::TerminateOrder 订单结束
/// \param stChargeStep 已经有的业务状态
///
void ChargeService::TerminateOrder(CHARGE_STEP &stChargeStep)
{
	UpdateEndBill(stChargeStep);
    sendOrderStatus(stChargeStep, 2);

    //订单结算完成后,清除开始原因,防止影响下一个指令周期
    stChargeStep.ucStartReason = START_CHARGE_CMD_RSN_NON;
	//清空订单类型
	stChargeStep.cOrderType = ORDER_NORMAL;
	stChargeStep.cQueueGroup = 0;
	stChargeStep.ucQueueMsgFromServer = 0;
	//清空功率曲线
    stChargeStep.iCurveCnt = 0;
}

///
/// \brief ChargeService::TerminateChargeCmd
/// \param stChargeStep
///
void ChargeService::TerminateChargeCmd(CHARGE_STEP &stChargeStep)
{
    //给发起充电业务方返回信息,二期增加对指令来源的判断.
    SendCmdResult(stChargeStep);
}


///
/// \brief ChargeService::SendCmdResult 发送指令结果
/// \param stChargeStep 状态机
///
void ChargeService::SendCmdResult(CHARGE_STEP &stChargeStep)
{
    InfoMap TempqInfoMap;
    InfoAddrType TempInfoType;

    LogOut(QString("发送指令结果!"), 1);
    TempOutPutChargeStep(stChargeStep);
    switch (stChargeStep.enCmdEndReason) {
    //-------------------触发类---------------------------------//
    case CMD_END_REASON_ACK_OK://响应成功
        PackageAckResult2Server(stChargeStep, CMD_ACK_TYPE_SUCCESS, TempqInfoMap, TempInfoType);
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
    case CMD_END_REASON_WAITACK_WRONG://非允许ACK
        PackageAckResult2Server(stChargeStep, stChargeStep.enCmdAck, TempqInfoMap, TempInfoType);
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
    case CMD_END_REASON_EXE_OK://执行成功
        PackageExecResult2Server(stChargeStep, CMD_ACK_EXE_TYPE_SUCCESS, TempqInfoMap, TempInfoType);
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
    case CMD_END_REASON_START_CHARGE_FAIL://启动失败
        PackageExecResult2Server(stChargeStep, CMD_ACK_EXE_TYPE_START_CHARGE_FAIL, TempqInfoMap, TempInfoType);
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
        //-------------------超时类---------------------------------//
    case CMD_END_REASON_WAITACK_TIMEOUT://接收ACK回复超时
        PackageAckResult2Server(stChargeStep, CMD_ACK_TYPE_TIMEOUT, TempqInfoMap, TempInfoType);
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
    case CMD_END_REASON_START_CHARGE_TIMEOUT://启动充电超时
        PackageExecResult2Server(stChargeStep, CMD_ACK_EXE_TYPE_START_CHARGE_TIMEOUT, TempqInfoMap, TempInfoType);
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
    case CMD_END_REASON_STOP_CHARGE_TIMEOUT: //结束充电超时
        PackageExecResult2Server(stChargeStep, CMD_ACK_EXE_TYPE_STOP_CHARGE_TIMEOUT, TempqInfoMap, TempInfoType);
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
    case CMD_END_REASON_START_DISCHARGE_TIMEOUT: //启动放电超时
        PackageExecResult2Server(stChargeStep, CMD_ACK_EXE_TYPE_START_DISCHARGE_TIMEOUT, TempqInfoMap, TempInfoType);
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
    case CMD_END_REASON_STOP_DISCHARGE_TIMEOUT: //结束放电超时
        PackageExecResult2Server(stChargeStep, CMD_ACK_EXE_TYPE_STOP_DISCHARGE_TIMEOUT, TempqInfoMap, TempInfoType);
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
        //-------------------逻辑加工类---------------------------------//
    case CMD_END_REASON_IERR://内部错误
        PackageAckResult2Server(stChargeStep, CMD_ACK_TYPE_IERR, TempqInfoMap, TempInfoType);
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
    case CMD_END_REASON_ALREADY_EXE://已经有指令执行
        PackageAckResult2Server(stChargeStep, CMD_ACK_TYPE_ALREADY_EXE, TempqInfoMap, TempInfoType);
        PackageExecResult2Server(stChargeStep, CMD_ACK_EXE_TYPE_FAIL, TempqInfoMap, TempInfoType);//加工执行结果,上传南京平台  add by zjq
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
    case CMD_END_REASON_ALREADY_CHARGING://已经在充电
        PackageAckResult2Server(stChargeStep, CMD_ACK_TYPE_ALREADY_CHARGING, TempqInfoMap, TempInfoType);
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
    case CMD_END_REASON_GUN_NOLINK://未插抢
        PackageAckResult2Server(stChargeStep, CMD_ACK_TYPE_GUN_NOLINK, TempqInfoMap, TempInfoType);
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
    case CMD_END_REASON_NOT_CHARGING://没有在充电
        PackageAckResult2Server(stChargeStep, CMD_ACK_TYPE_NOT_CHARGING, TempqInfoMap, TempInfoType);
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
    case CMD_END_REASON_DEV_OFFLINE://设备离线
        PackageAckResult2Server(stChargeStep, CMD_ACK_TYPE_DEV_OFFLINE, TempqInfoMap, TempInfoType);
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
    case CMD_END_REASON_DEV_FAULT://设备故障
        PackageAckResult2Server(stChargeStep, CMD_ACK_TYPE_DEV_FAULT, TempqInfoMap, TempInfoType);
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
    case CMD_END_REASON_NOW_LIMIT://设备限制充电中
        PackageAckResult2Server(stChargeStep, CMD_ACK_TYPE_NOW_LIMIT, TempqInfoMap, TempInfoType);
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
    case CMD_END_REASON_NOW_PAUSH://设备暂停充电中
        PackageAckResult2Server(stChargeStep, CMD_ACK_TYPE_NOW_PAUSH, TempqInfoMap, TempInfoType);
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
    case CMD_END_REASON_NOW_DISCHARGING://设备放电中
        PackageAckResult2Server(stChargeStep, CMD_ACK_TYPE_NOW_DISCHARGING, TempqInfoMap, TempInfoType);
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
    default:
        break;
    }
}

///
/// \brief ChargeService::PackageTelecontrol2ChargeEquipmentr 将指令发送给充电模块
/// \param stChargeStep 充电业务状态机
///
void ChargeService::PackageTelecontrol2ChargeEquipment(CHARGE_STEP &stChargeStep,InfoMap &qInfoMap, InfoAddrType &InfoType)
{
    QByteArray qTempByteArray;

    InfoType = AddrType_CmdCtrl;
    qTempByteArray.append(stChargeStep.ucCanAddr);
    qInfoMap.insert(Addr_CanID_Comm, qTempByteArray);

    qTempByteArray.clear();
    qTempByteArray.append(stChargeStep.ucCmdValue);
    qInfoMap.insert(Addr_ChargeCmd_Ctrl, qTempByteArray);
}


///
/// \brief ChargeService::ProcTeleControlChargeCmdAck
/// \param InfoTagList
///
void ChargeService::ProcTeleControlChargeCmdAck(InfoMap qInfoMap)
{
    CHARGE_STEP st_NowChargeStep = stEmptyChargeStep;
    __s32 AckVlaidCheckRet = -1;

    //解析传递参数.
    if(ParseTeleControlChargeCmdAck(qInfoMap, st_NowChargeStep) == false){
        return;
    }
    if(!CheckCanAddrValid(st_NowChargeStep.ucCanAddr)){
        return;
    }
    //根据当前状态校验遥控充电指令ACK
    CheckControlChargeCmdAckValid(st_NowChargeStep, AckVlaidCheckRet);
    LogOut(QString("AckVlaidCheckRet = %1!").arg((__u8)AckVlaidCheckRet), 1);
    if(AckVlaidCheckRet != 0){//ACK校验合法不代表ACK的内容成功
        LogOut(QString("收到ACK非有效 ACK = %1!").arg(AckVlaidCheckRet), 3);
        //记录异常返回
        return;
    }
    //更新充电步骤状态机.
    if(UpdateChargeStepAck(st_NowChargeStep) == false){
        LogOut(QString("收到ACK后更新状态机失败!"), 3);
        //记录异常返回
        return;
    }
    //充电状态机持久化,防止掉电丢失.
    SaveChargeStep(st_NowChargeStep);
    //可选择是否发送给调度模块.

}

///
/// \brief ChargeService::ParseTeleControlChargeCmdAck 解析传递指令ACK参数
/// \param qInfoMap
/// \param stChargeStep 存放解析后的指令
/// \return
///
bool ChargeService::ParseTeleControlChargeCmdAck(InfoMap qInfoMap,  CHARGE_STEP &stChargeStep)
{
    if(qInfoMap.contains(Addr_CanID_Comm)){
        stChargeStep.ucCanAddr = qInfoMap[Addr_CanID_Comm].at(0);
    }
    else{
        return false;
    }
    if(qInfoMap.contains(Addr_ChargeCmd_Ctrl)){
        stChargeStep.ucCmdValue = qInfoMap[Addr_ChargeCmd_Ctrl].at(0);
    }
    else{
        return false;
    }
    if(qInfoMap.contains(Addr_AckResult_Ctrl)){
        stChargeStep.enCmdAck = (CMD_ACK_TYPE)qInfoMap[Addr_AckResult_Ctrl].at(0);
    }
    else{
        return false;
    }
    LogOut(QString("ACK: %1!").arg((__u8)stChargeStep.enCmdAck), 1);
    TempOutPutChargeStep(stChargeStep);
    return true;
}




///
/// \brief netThread::CheckCanAddrValid 判断CAN地址是否在配置范围内
/// \param ucCanAddr
/// \return
///
bool ChargeService::CheckCanAddrValid(__u8 ucCanAddr)
{
    __u8 ucIndexID = 0;//起始地址

    for(int i = 0; i< 3 ; i++){
        switch (i) {
        case 0://单相
            ucIndexID  = 1;
            break;
        case 1://三相
            ucIndexID  = 151;
            break;
        case 2://直流
            ucIndexID  = 181;
            break;
        default:
            break;
        }

        if(ucCanAddr >= ucIndexID && ucCanAddr < ucNumTerminal[i] + ucIndexID){
            return true;
        }
    }

    LogOut(QString("CAN地址不再配置范围 CAN = %1!").arg(ucCanAddr), 3);
    return false;
}



//------------------------------------------------------------VIN远程充电相关---------------------------------------------------//
bool ChargeService::ProcVinChargeSub(InfoMap &qInfoMap, InfoAddrType &InfoType)
{
    switch (InfoType) {
    case AddrType_VinNum://主题一：VIN号
        LogOut(QString("Recv AddrType_VinApplyStartCharge from bus!"), 1);
        if(ProcReadVinSub(qInfoMap) == false){
            return false;
        }
        break;
    case AddrType_VinApplyStartCharge_Result://主题三：服务器返回VIN申请开始充电结果
        LogOut(QString("收到总线-VIN外部申请开始充电-结果!"), 1);
        if(ProcRecvOutVinApplyStartChargeResult(qInfoMap) == false){
            return false;
        }
        break;
    case AddrType_VinApplyStopCharge_Result://主题五：服务器返回VIN申请结束充电结果
        LogOut(QString("收到总线-VIN外部申请结束充电-结果!"), 1);
        if(ProcRecvOutVinApplyStopChargeResult(qInfoMap) == false){
            return false;
        }
        break;
    case AddrType_InVinApplyStartCharge://主题：VIN内部申请开始充电
        LogOut(QString("收到总线-VIN内部申请开始充电!"), 1);
        if(ProcRecvInVinApplyStartCharge(qInfoMap) == false){
            return false;
        }
        break;
    case AddrType_InVinApplyStopCharge://主题：VIN内部申请结束充电
        LogOut(QString("收到总线-VIN内部申请结束充电!"), 1);
        //LogOut(QString("接收VIN号:%1 CAN = %2!").arg(QString(qInfoMap[Addr_BatteryVIN_BMS])).arg(QString::number(qInfoMap[Addr_CanID_Comm].at(0))),2);
        if(ProcRecvInVinApplyStopCharge(qInfoMap) == false){
            return false;
        }
        break;
    default:
        break;
    }

    return true;
}

bool ChargeService::ProcRecvInVinApplyStartCharge(InfoMap &qInfoMap)
{
    CHARGE_STEP stTempChargeStep = stEmptyChargeStep;//存放解析出的数据
    CHARGE_STEP stChargeStep = stEmptyChargeStep;//存放缓存中的数据

    if(ParseVinChargeSub(qInfoMap, stTempChargeStep) == -1){
        LogOut(QString("ProcRecvInVinApplyStartCharge 解析有错误,返回!"), 3);
        return false;
    }
    if(CheckCanAddrValid(stTempChargeStep.ucCanAddr) == false){
        return false;
    }

    if(gpDevCache->QueryChargeStep(stTempChargeStep.ucCanAddr, stChargeStep) == false){
        stChargeStep.ucCanAddr = stTempChargeStep.ucCanAddr;
        stChargeStep.cChargeWay = VIN_START;//VIN启动
        memcpy(stChargeStep.sVIN, stTempChargeStep.sVIN, LENGTH_VIN_NO);
        if(gpDevCache->AddChargeStep(stTempChargeStep.ucCanAddr, stTempChargeStep) == false){
            LogOut(QString("ProcRecvInVinApplyStartCharge add chargestep fail!"), 3);
            return false;
        }
    }
    else{
        stChargeStep.cChargeWay = VIN_START;//VIN启动
        memcpy(stChargeStep.sVIN, stTempChargeStep.sVIN, LENGTH_VIN_NO);
        if(CheckVinApplyStartChargeValid(stChargeStep) == false){
            LogOut(QString("错峰充电-申请开始充电-校验失败!"), 3);
            //++++++返回内部申请结果
            return false;
        }
    }

    if(SendOutVinApplyStartCharge(stChargeStep) == false){
        return false;
    }

    if(UpdateChargeStepByInVinApplyStartCharge(stChargeStep) == false){
        return false;
    }

    //数据持久化
    SaveChargeStep(stChargeStep);
    return true;
}


bool ChargeService::ProcRecvInVinApplyStopCharge(InfoMap &qInfoMap)
{
    CHARGE_STEP stTempChargeStep = stEmptyChargeStep;//存放解析出的数据
    CHARGE_STEP stChargeStep = stEmptyChargeStep;//存放缓存中的数据

    //解析数据
    if(ParseVinChargeSub(qInfoMap, stTempChargeStep) == -1){
        LogOut(QString("处理-VIN内部申请结束充电- 解析有错误,返回!"), 3);
        return false;
    }
    //校验CAN地址
    if(CheckCanAddrValid(stTempChargeStep.ucCanAddr) == false){
        return false;
    }
    //逻辑处理
    if(gpDevCache->QueryChargeStep(stTempChargeStep.ucCanAddr, stChargeStep) == false){
        LogOut(QString("处理-VIN内部申请结束充电- 没有找到状态机,返回!"), 3);
        return false;
    }
    else{
        if(memcmp(stChargeStep.sVIN, stTempChargeStep.sVIN, LENGTH_VIN_NO) != 0){
            LogOut(QString("处理-VIN内部申请结束充电- 前后VIN不一致,以后面为准!"), 3);
            memcpy(stChargeStep.sVIN, stTempChargeStep.sVIN, LENGTH_VIN_NO);
        }
    }
    //    //发送-外部申请结束充电
    //    if(SendOutVinApplyStopCharge(stChargeStep) == false){
    //        return false;
    //    }
    //更新状态机
    if(UpdateChargeStepByInVinApplyStopCharge(stChargeStep) == false){
        return false;
    }
    //发送停止充电指令(由于目前平台不支持申请结束,由CSCU直接结束)
    SendToChargeEquipmentStopCharge(stChargeStep);
    //模拟返回申请结束充电结果
    SendInVinApplyStopChargeResult(stChargeStep, 0xff);
    //数据持久化
    SaveChargeStep(stChargeStep);
    return true;
}

bool ChargeService::ProcReadVinSub(InfoMap &qInfoMap)
{
    LogOut(QString("处理-VIN号主题!"), 1);

    CHARGE_STEP stTempChargeStep = stEmptyChargeStep;//存放解析出的数据
    CHARGE_STEP stChargeStep = stEmptyChargeStep;//存放缓存中的数据

    if(ParseVinChargeSub(qInfoMap, stTempChargeStep) == -1){
        LogOut(QString("ProcReadVinSub 解析有错误,返回!"), 3);
        return false;
    }
    if(CheckCanAddrValid(stTempChargeStep.ucCanAddr) == false){
        return false;
    }


    if(gpDevCache->QueryChargeStep(stTempChargeStep.ucCanAddr, stChargeStep) == false){
        stChargeStep.ucCanAddr = stTempChargeStep.ucCanAddr;
        stChargeStep.cChargeWay = VIN_START;//VIN启动
        memcpy(stChargeStep.sVIN, stTempChargeStep.sVIN, LENGTH_VIN_NO);
        if(gpDevCache->AddChargeStep(stTempChargeStep.ucCanAddr, stChargeStep) == false){
            LogOut(QString("ProcReadVinSub add chargestep fail!"), 3);
            return false;
        }
    }
    else{
        stChargeConfig charge;
        memset(&charge, 0, sizeof(stChargeConfig));
        gpParamSet->querySetting(&charge, PARAM_CHARGE);

        stChargeStep.cChargeWay = VIN_START;//VIN启动

        memcpy(stChargeStep.sVIN, stTempChargeStep.sVIN, LENGTH_VIN_NO);

        //VIN触发充电关闭时不进行校验
        if(charge.vinAuto && CheckVinApplyStartChargeValid(stChargeStep) == false){
            LogOut(QString("处理-VIN号主题-校验失败!"), 3);
            return false;
        }
    }


    if(SendOutVinApplyStartCharge(stChargeStep) == false){
        return false;
    }

    if(UpdateChargeStepBySendOutVinApplyStartCharge(stChargeStep) == false){
        return false;
    }

    //数据持久化
    SaveChargeStep(stChargeStep);
    return true;
}

bool ChargeService::ProcRecvOutVinApplyStartChargeResult(InfoMap &qInfoMap)
{
    CHARGE_STEP stTempChargeStep = stEmptyChargeStep;//存放解析出的数据
    CHARGE_STEP stChargeStep = stEmptyChargeStep;//存放缓存中的数据
    int ApplyChargeResult = 0;

    //解析数据
    ApplyChargeResult = ParseVinChargeSub(qInfoMap, stTempChargeStep);
    if(ApplyChargeResult == -1 || ApplyChargeResult == 0){
        LogOut(QString("ProcRecvOutVinApplyStartChargeResult = %1, 无效!").arg(ApplyChargeResult), 1);
        return false;
    }
    //校验CAN地址
    if(CheckCanAddrValid(stTempChargeStep.ucCanAddr) == false){
        return false;
    }
    //逻辑处理
    if(gpDevCache->QueryChargeStep(stTempChargeStep.ucCanAddr, stChargeStep) == false){
        LogOut(QString("处理-VIN外部申请开始充电-结果 没有找到状态机,返回!"), 3);
        return false;
    }
    else{
		//屏蔽VIN号复制，3.0协议中应答包中不包含VIN号
		/*
        if(memcmp(stChargeStep.sVIN, stTempChargeStep.sVIN, LENGTH_VIN_NO) != 0){
            LogOut(QString("处理-VIN外部申请开始充电-结果 前后VIN不一致,以后面为准!"), 3);
            memcpy(stChargeStep.sVIN, stTempChargeStep.sVIN, LENGTH_VIN_NO);
        }
		*/
    }

    if(stChargeStep.ucStartReason == START_CHARGE_CMD_RSN_SMART_CHARGE_VIN || \
            stChargeStep.ucStartReason ==  START_CHARGE_CMD_RSN_COUPLE_CHARGE_VIN){
        SendInVinApplyStartChargeResult(stChargeStep, ApplyChargeResult);
    }
    if(UpdateChargeStepByRecvOutVinApplyStartChargeResult(stChargeStep, ApplyChargeResult) == false){
        return false;
    }

    //数据持久化
    SaveChargeStep(stChargeStep);
    return true;
}


bool ChargeService::ProcRecvOutVinApplyStopChargeResult(InfoMap &qInfoMap)
{
    CHARGE_STEP stTempChargeStep = stEmptyChargeStep;//存放解析出的数据
    CHARGE_STEP stChargeStep = stEmptyChargeStep;//存放缓存中的数据
    int ApplyChargeResult = 0;

    //解析数据
    ApplyChargeResult = ParseVinChargeSub(qInfoMap, stTempChargeStep);
    if(ApplyChargeResult == -1 || ApplyChargeResult == 0){
        LogOut(QString("处理-VIN外部申请结束充电-结果 = %1, 无效!").arg(ApplyChargeResult), 1);
        return false;
    }
    //校验CAN地址
    if(CheckCanAddrValid(stTempChargeStep.ucCanAddr) == false){
        return false;
    }
    //逻辑处理
    if(gpDevCache->QueryChargeStep(stTempChargeStep.ucCanAddr, stChargeStep) == false){
        LogOut(QString("处理-VIN外部申请结束充电-结果 没有找到状态机,返回!"), 3);
        return false;
    }else{
        if(memcmp(stChargeStep.sVIN, stTempChargeStep.sVIN, LENGTH_VIN_NO) != 0){
            LogOut(QString("处理-VIN外部申请结束充电-结果 前后VIN不一致,以后面为准!"), 3);
            memcpy(stChargeStep.sVIN, stTempChargeStep.sVIN, LENGTH_VIN_NO);
        }
    }

    if(stChargeStep.ucStopReasonCSCU == 110){//错峰充电申请结束
        SendInVinApplyStopChargeResult(stChargeStep, ApplyChargeResult);
    }
    if(UpdateChargeStepByRecvOutVinApplyStopChargeResult(stChargeStep, ApplyChargeResult) == false){
        return false;
    }

    //数据持久化
    SaveChargeStep(stChargeStep);
    return true;
}


int ChargeService::ParseVinChargeSub(InfoMap &qInfoMap, CHARGE_STEP &stTempChargeStep)
{
    int ApplyChargeResult = 0;

    if(qInfoMap.contains(Addr_CanID_Comm)){//CAN地址
        stTempChargeStep.ucCanAddr = qInfoMap[Addr_CanID_Comm].at(0);
        LogOut(QString("CAN = %1!").arg(stTempChargeStep.ucCanAddr), 1);
    }
    else{//必备项,无则返回失败
        return -1;
    }
    if(qInfoMap.contains(Addr_BatteryVIN_BMS)){//VIN号
        memcpy(stTempChargeStep.sVIN, qInfoMap[Addr_BatteryVIN_BMS].data(), qInfoMap[Addr_BatteryVIN_BMS].size());
        LogOut(QString("接收VIN号:%1 CAN = %2!").arg(QString(qInfoMap[Addr_BatteryVIN_BMS])).arg(stTempChargeStep.ucCanAddr),2);
    }
    else{
        return -1;
    }
    if(qInfoMap.contains(Addr_VINApplyStartChargeType_Result)){//VIN申请开始充电结果
        ApplyChargeResult = qInfoMap[Addr_VINApplyStartChargeType_Result].at(0);
        LogOut(QString("Addr_VINApplyStartChargeType_Result = %1!").arg(ApplyChargeResult), 2);
        return ApplyChargeResult;
    }
    if(qInfoMap.contains(Addr_VINApplyStopChargeType_Result)){//VIN申请结束充电结果
        ApplyChargeResult = qInfoMap[Addr_VINApplyStopChargeType_Result].at(0);
        LogOut(QString("Addr_VINApplyStopChargeType_Result = %1!").arg(ApplyChargeResult), 2);
        return ApplyChargeResult;
    }

    return 0;
}

bool ChargeService::SendInVinApplyStartChargeResult(CHARGE_STEP &stChargeStep, int ApplyChargeResult)
{
    InfoMap qInfoMap;
    InfoAddrType InfoType;
    QByteArray qTempByteArray;

    InfoType = AddrType_InVinApplyStartCharge_Result; //主题：VIN内部申请开始充电结果

    qTempByteArray.append(stChargeStep.ucCanAddr);
    qInfoMap.insert(Addr_CanID_Comm, qTempByteArray);//CAN地址

    qTempByteArray.clear();
    qTempByteArray = QByteArray(stChargeStep.sVIN, LENGTH_VIN_NO);
    qInfoMap.insert(Addr_BatteryVIN_BMS, qTempByteArray);//VIN号

    qTempByteArray.clear();
    qTempByteArray.append((unsigned char)ApplyChargeResult);//
    qInfoMap.insert(Addr_InVINApplyStartChargeType_Result, qTempByteArray);//VIN内部申请开始充电结果

    emit sigToBus(qInfoMap, InfoType);
    return true;
}

bool ChargeService::SendInVinApplyStopChargeResult(CHARGE_STEP &stChargeStep, int ApplyChargeResult)
{
    InfoMap qInfoMap;
    InfoAddrType InfoType;
    QByteArray qTempByteArray;

    InfoType = AddrType_InVinApplyStopCharge_Result; //主题：VIN内部申请结束充电结果

    qTempByteArray.append(stChargeStep.ucCanAddr);
    qInfoMap.insert(Addr_CanID_Comm, qTempByteArray);//CAN地址

    qTempByteArray.clear();
    qTempByteArray = QByteArray(stChargeStep.sVIN, LENGTH_VIN_NO);
    qInfoMap.insert(Addr_BatteryVIN_BMS, qTempByteArray);//VIN号

    qTempByteArray.clear();
    qTempByteArray.append((unsigned char)ApplyChargeResult);//
    qInfoMap.insert(Addr_InVINApplyStopChargeType_Result, qTempByteArray);//VIN内部申请结束充电结果

    emit sigToBus(qInfoMap, InfoType);
    return true;
}

bool ChargeService::SendOutVinApplyStartCharge(CHARGE_STEP &stChargeStep)
{
    LogOut(QString("发送-VIN号外部申请充电主题!"), 1);
    InfoMap qInfoMap;
    InfoAddrType InfoType;

    if(PackageOutVinApplyStartCharge(qInfoMap, InfoType, stChargeStep)){
        emit sigToBus(qInfoMap, InfoType);
    }
    else{
        return false;
    }

    return true;
}

bool ChargeService::SendOutVinApplyStopCharge(CHARGE_STEP &stChargeStep)
{
    InfoMap qInfoMap;
    InfoAddrType InfoType;
    QByteArray qTempByteArray;

    LogOut(QString("发送-外部申请结束充电!"), 1);
    InfoType = AddrType_VinApplyStopCharge; //主题：VIN外部申请结束充电

    qTempByteArray.append(stChargeStep.ucCanAddr);
    qInfoMap.insert(Addr_CanID_Comm, qTempByteArray);//CAN地址

    qTempByteArray.clear();
    qTempByteArray = QByteArray(stChargeStep.sVIN, LENGTH_VIN_NO);
    qInfoMap.insert(Addr_BatteryVIN_BMS, qTempByteArray);//VIN号

    //订单号 南京3.0新协议
    qInfoMap.insert(Addr_Bill_Code, QByteArray().append(stChargeStep.sBillCode, strlen(stChargeStep.sBillCode)));

    emit sigToBus(qInfoMap, InfoType);
    return true;
}

bool ChargeService::SendToChargeEquipmentStopCharge(CHARGE_STEP &stChargeStep)
{
    LogOut(QString("发送结束充电指令!"), 1);
    InfoMap qInfoMap;
    InfoAddrType InfoType;
    QByteArray qTempByteArray;

    InfoType = AddrType_CmdCtrl;

    qTempByteArray.append(stChargeStep.ucCanAddr);
    qInfoMap.insert(Addr_CanID_Comm, qTempByteArray);

    qTempByteArray.clear();
    qTempByteArray.append((unsigned char )CHARGE_CMD_TYPE_STOP_CHARGE);
    qInfoMap.insert(Addr_ChargeCmd_Ctrl, qTempByteArray);

    emit sigToBus(qInfoMap, InfoType);
    return true;
}


bool ChargeService::PackageOutVinApplyStartCharge(InfoMap &qInfoMap, InfoAddrType &InfoType, CHARGE_STEP &stChargeStep)
{
    LogOut(QString("PackageOutVinApplyStartCharge!"), 1);
    QByteArray qTempByteArray;

    stChargeConfig charge;
    memset(&charge, 0, sizeof(stChargeConfig));
    gpParamSet->querySetting(&charge, PARAM_CHARGE);

    InfoType = AddrType_VinApplyStartCharge; //主题二：VIN申请开始充电

    qTempByteArray.append(stChargeStep.ucCanAddr);
    qInfoMap.insert(Addr_CanID_Comm, qTempByteArray);//CAN地址

    qTempByteArray.clear();
    qTempByteArray = QByteArray(stChargeStep.sVIN, LENGTH_VIN_NO);
    qInfoMap.insert(Addr_BatteryVIN_BMS, qTempByteArray);//VIN号

    qTempByteArray.clear();
    LogOut(QString("VIN自动申请充电选项:%1, CAN = %2 !").arg(charge.vinAuto).arg(stChargeStep.ucCanAddr), 1);
    if(charge.vinAuto == 1){
        //add by FJC
        //双枪充电功能开启时，单枪充电或未收到平台响应则不申请充电
        TerminalStatus stTerminalStatus = stEmptyTerminalStatus;//存放缓存中的数据
        if(gpDevCache->QueryTerminalStatus(stChargeStep.ucCanAddr, stTerminalStatus) == false){
            LogOut(QString("获取终端状态失败1!!!!!!!!!!"), 3);
        }
        if(!GetMultType(charge,stTerminalStatus,1))
        {
            ////////1
            //            QByteArray qTempByteArray;
            //            qTempByteArray.clear();
            //            qTempByteArray.append(248);//
            //            qInfoMap.insert(Addr_InVINApplyStartChargeType_Result, qTempByteArray);//VIN内部申请开始充电结果
            //            emit sigToBus(qInfoMap, AddrType_InVinApplyStartCharge_Result);

            LogOut(QString("VIN号外部申请充电，因未获分组确认或无自动申请权限而停止申请!"), 2);
            return false;
        }
        else
        {
            qTempByteArray.append(1);//VIN申请开始充电类型,默认1,充满为止
        }
    }
    else if(charge.vinAuto == 0){
        qTempByteArray.append(3);//VIN申请开始充电类型,03不申请充电，只发送数据。
    }

    qInfoMap.insert(Addr_VINApplyStartChargeType, qTempByteArray);

    return true;
}

bool ChargeService::UpdateChargeStepBySendOutVinApplyStartCharge(CHARGE_STEP &stChargeStep)
{
    stChargeStep.ucCmdSrc = CHARGE_CMD_SRC_CAN_DEV;//102 充电设备
    stChargeStep.ucStartReason = START_CHARGE_CMD_RSN_DEV_VIN_REMOTE;//108 VIN号（远程）
    //    stChargeStep.enChargeStep = CHARGE_STEP_WAITSEVER_ACK_APPLY_START_CHARGE_VIN;
    //    stChargeStep.stChargeStepValue.uc_charge_step_timeout = CHARGE_STEP_NORMAL;
    //    stChargeStep.stChargeStepValue.uc_hold_time = g_TimeoutChargeStep[stChargeStep.enChargeStep];

    if(gpDevCache->UpateChargeStep(stChargeStep.ucCanAddr, stChargeStep) == false){
        return false;
    }

    return true;
}

bool ChargeService::UpdateChargeStepByInVinApplyStartCharge(CHARGE_STEP &stChargeStep)
{
    stChargeStep.ucCmdSrc = CHARGE_CMD_SRC_CSCU;//101 充电系统控制器
    stChargeStep.ucStartReason = START_CHARGE_CMD_RSN_SMART_CHARGE_VIN;//114 错峰充电VIN申请
    //    stChargeStep.enChargeStep = CHARGE_STEP_WAITSEVER_ACK_APPLY_START_CHARGE_VIN;
    //    stChargeStep.stChargeStepValue.uc_charge_step_timeout = CHARGE_STEP_NORMAL;
    //    stChargeStep.stChargeStepValue.uc_hold_time = g_TimeoutChargeStep[stChargeStep.enChargeStep];

    if(gpDevCache->UpateChargeStep(stChargeStep.ucCanAddr, stChargeStep) == false){
        return false;
    }

    return true;
}

bool ChargeService::UpdateChargeStepByInVinApplyStopCharge(CHARGE_STEP &stChargeStep)
{
    stChargeStep.ucStopReasonCSCU = 110;//110 错峰充电申请结束
    //stChargeStep.ucStopReasonDev = 94; //集控错峰充电终止2018-7-7hd

    //    stChargeStep.enChargeStep = CHARGE_STEP_WAITSEVER_ACK_APPLY_START_CHARGE_VIN;
    //    stChargeStep.stChargeStepValue.uc_charge_step_timeout = CHARGE_STEP_NORMAL;
    //    stChargeStep.stChargeStepValue.uc_hold_time = g_TimeoutChargeStep[stChargeStep.enChargeStep];

    if(gpDevCache->UpateChargeStep(stChargeStep.ucCanAddr, stChargeStep) == false){
        return false;
    }

    return true;
}

bool ChargeService::UpdateChargeStepByRecvOutVinApplyStartChargeResult(CHARGE_STEP &stChargeStep, int ApplyChargeResult)
{
    if(ApplyChargeResult != 0xff){//不允许充电,重置状态机变量
        stChargeStep.ucCmdSrc = CHARGE_CMD_SRC_NON;
        stChargeStep.ucStartReason = START_CHARGE_CMD_RSN_NON;
    }
    //返回结果不处理充电状态机  hd 2018-5-9
//    stChargeStep.enChargeStep = CHARGE_STEP_NORMAL;
//    stChargeStep.stChargeStepValue.uc_charge_step_timeout = CHARGE_STEP_NORMAL;
//    stChargeStep.stChargeStepValue.uc_hold_time = g_TimeoutChargeStep[stChargeStep.enChargeStep];

    if(gpDevCache->UpateChargeStep(stChargeStep.ucCanAddr, stChargeStep) == false){
        return false;
    }

    return true;
}

bool ChargeService::UpdateChargeStepByRecvOutVinApplyStopChargeResult(CHARGE_STEP &stChargeStep, int ApplyChargeResult)
{
    if(ApplyChargeResult != 0xff){//不允许结束充电,重置状态机变量
        stChargeStep.ucStopReasonCSCU = 0;
    }
    stChargeStep.enChargeStep = CHARGE_STEP_NORMAL;
    stChargeStep.stChargeStepValue.uc_charge_step_timeout = CHARGE_STEP_NORMAL;
    stChargeStep.stChargeStepValue.uc_hold_time = g_TimeoutChargeStep[stChargeStep.enChargeStep];

    if(gpDevCache->UpateChargeStep(stChargeStep.ucCanAddr, stChargeStep) == false){
        return false;
    }

    return true;
}

bool ChargeService::CheckVinApplyStartChargeValid(CHARGE_STEP &stChargeStep)
{
    TerminalStatus stTerminalStatus = stEmptyTerminalStatus;

    if(gpDevCache->QueryTerminalStatus(stChargeStep.ucCanAddr, stTerminalStatus) == false){
        return false;
    }

    if(stTerminalStatus.cStatus != CHARGE_STATUS_FAULT){//校验终端状态
        if(stChargeStep.enChargeStep == CHARGE_STEP_NORMAL
                && stChargeStep.enOrderStatus != ORDER_STATUS_ING ){//校验状态机
            //            if(strlen(stChargeStep.sVIN) == LENGTH_VIN_NO){//校验VIN是否为数组
//            if(stChargeStep.ucStartReason !=START_CHARGE_CMD_RSN_SMART_CHARGE_VIN
//                    && stChargeStep.ucStartReason != START_CHARGE_CMD_RSN_COUPLE_CHARGE_VIN
//                    && stChargeStep.ucStartReason !=START_CHARGE_CMD_RSN_DEV_CAR_LICENCE_REMOTE) { //已经进行VIN申请了的设备不允许再次申请 hd 2018-5-9
            LogOut(QString("校验VIN内部申请开始充电-校验通过"), 1);
            return true;
//                        }
        }
    }
    return false;
}
///
/// \brief ChargeService::ProcTeleindicationDataTerm 处理充电设备摇信数据
/// \param qInfoMap
/// \param InfoType
///
void ChargeService::ProcTeleindicationDataTerm(InfoMap &qInfoMap)
{
    TerminalStatus st_TempStatusNow = stEmptyTerminalStatus;//最新的终端状态
    TerminalStatus st_TempStatusOld = stEmptyTerminalStatus;//变化前的终端状态
    CHARGE_STEP stTempChargeStep = stEmptyChargeStep;//存放已有的状态机
    CHARGE_STATUS_CHANGE_TYPE enChargeStatusChangeType = CHARGE_STATUS_CHANGE_NON;

    bool b_HasChargeStep = false;//是否含有该终端的充电状态机
    //解析
    if(!ParseTeleindicationData(qInfoMap, st_TempStatusOld, st_TempStatusNow)){
		return;
	}

    if(!CheckCanAddrValid(st_TempStatusNow.cCanAddr)){
        return;
    }

    TerminalStatus stTerminalStatus = stEmptyTerminalStatus;

    if(gpDevCache->QueryTerminalStatus(st_TempStatusNow.cCanAddr, stTerminalStatus) == 0){
        LogOut(QString("UpadeLogicChargeStatus  QueryTerminalStatus fail1!"), 3);
        return ;
    }
    TempOutPutTerminalStatus(stTerminalStatus);
//    //拔枪时，清除单双枪信息 add by FJC　2017/3/27
//    //平台要求,拔枪后直流机不上传单双枪信息，集控自清除
    ClearChargeManner(st_TempStatusNow,st_TempStatusOld);
    //有充电机工作状态有变化
    if(st_TempStatusNow.stFrameRemoteSingle.charge_status
            != st_TempStatusOld.stFrameRemoteSingle.charge_status){
        enChargeStatusChangeType = ProcChargeStatusChange(st_TempStatusOld, st_TempStatusNow);//判断何种变化
        LogOut(QString("NEW = %1 OLD= %2!")\
               .arg((unsigned char) st_TempStatusNow.stFrameRemoteSingle.charge_status)\
               .arg((unsigned char) st_TempStatusOld.stFrameRemoteSingle.charge_status), 1);
    }
    //判断是否有充电状态机
    b_HasChargeStep = gpDevCache->QueryChargeStep(st_TempStatusNow.cCanAddr, stTempChargeStep);
    TempOutPutChargeStep(stTempChargeStep);
    //对各种变化进行处理
    switch (enChargeStatusChangeType) {
    case CHARGE_STATUS_CHANGE_NON://无
        break;
        //有触发充电业务改变的变化
    case CHARGE_STATUS_CHANGE_START_CHARGE://开始充电
    case CHARGE_STATUS_CHANGE_STOP_CHARGE://停止充电
    case CHARGE_STATUS_CHANGE_START_DISCHARGE://开始放电
    case CHARGE_STATUS_CHANGE_STOP_DISCHARGE://结束放电
    case CHARGE_STATUS_CHANGE_START_CHARGE_FAIL://启动充电失败
    case CHARGE_STATUS_CHANGE_START_PAUSE_CHARGE://暂停充电-->>由插枪到暂停
    {
        if(b_HasChargeStep == false){//没有充电状态机
            //自动启动的充电业务,二期要增加相应的描述信息
            b_HasChargeStep = true;
            stTempChargeStep.ucCanAddr = st_TempStatusNow.cCanAddr;
            //若为开始充/放电,则创建订单
            switch (enChargeStatusChangeType) {
            case CHARGE_STATUS_CHANGE_START_CHARGE:
            case CHARGE_STATUS_CHANGE_START_DISCHARGE:
            case CHARGE_STATUS_CHANGE_START_PAUSE_CHARGE:
                CreatChargeOrder(stTempChargeStep, 2);
                gpDevCache->AddChargeStep(st_TempStatusNow.cCanAddr,stTempChargeStep);//创建一条充电订单记录
                StopChargeTerminalSelfStart(stTempChargeStep);
                break;
            default:
                break;
            }
           // gpDevCache->AddChargeStep(st_TempStatusNow.cCanAddr,stTempChargeStep);//创建一条充电订单记录
        }
        else{//已有充电状态机
            //若为开始充/放电,则创建订单
            switch (enChargeStatusChangeType) {
            case CHARGE_STATUS_CHANGE_START_CHARGE:
            case CHARGE_STATUS_CHANGE_START_DISCHARGE:
            case CHARGE_STATUS_CHANGE_START_PAUSE_CHARGE:
                if(stTempChargeStep.enOrderStatus != ORDER_STATUS_ING &&
                        stTempChargeStep.enOrderStatus != ORDER_STATUS_QUEUE){//状态机中没有进行的订单且不在排队,则创建一个
					//清理缓存中查询到的step，保持订单为空白
                    uchar canAddr = stTempChargeStep.ucCanAddr;
                    char sVIN[LENGTH_VIN_NO];//VIN号
                    memcpy(sVIN,stTempChargeStep.sVIN,LENGTH_VIN_NO);
                    memcpy(&stTempChargeStep, &stEmptyChargeStep, sizeof(CHARGE_STEP));
                    stTempChargeStep.ucCanAddr = canAddr;
                    memcpy(stTempChargeStep.sVIN,sVIN,LENGTH_VIN_NO);
                    CreatChargeOrder(stTempChargeStep, 2);
                    StopChargeTerminalSelfStart(stTempChargeStep);
                }
                break;
            default:
                break;
            }
        }

        TempOutPutChargeStep(stTempChargeStep);
        LogOut(QString("有引起充电业务改变的突变!"), 1);
        //更新充电业务状态机
        UpdateChargeStepByStatusChange(enChargeStatusChangeType, stTempChargeStep);

        //++南京3.0协议上报订单开始状态
        switch(enChargeStatusChangeType){
        case CHARGE_STATUS_CHANGE_START_CHARGE:
        case CHARGE_STATUS_CHANGE_START_DISCHARGE:
            sendOrderStatus(stTempChargeStep, 1);
            break;
        default:
            break;
        }
        //--
    }
        break;
    case CHARGE_STATUS_CHANGE_STARTING://启动中
        //判断是否需要通知
        switch (stTempChargeStep.ucStartReason) {
        case START_CHARGE_CMD_RSN_CARD_REMOTE://集中刷卡（远程）
        case START_CHARGE_CMD_RSN_CARD_LOCAL://集中刷卡（本地）
            //通知屏幕跳转页面
            //            SendApplyStopChargeResultToScreen(stTempChargeStep, 252);//设备启动中
            break;
        default:
            break;
        }
        break;
    case CHARGE_STATUS_CHANGE_PAUSE_CHARGE://停止充电
    case CHARGE_STATUS_CHANGE_RECOVER_PAUSE_CHARGE://恢复停止充电
        LogOut(QString("有引起充电业务改变的突变--1 !"), 1);
        //更新充电业务状态机
        UpdateChargeStepByStatusChange(enChargeStatusChangeType, stTempChargeStep);
        break;
    default:
        break;
    }

    UpadeLogicChargeStatusForTerminalSingle(stTerminalStatus, b_HasChargeStep,st_TempStatusNow.stFrameRemoteSingle.charge_status);  //充电机工作状态使用通过MAP传递过来的状态而不是query的，防止query的时候已经被realdatefilter更改过了  hd 2018-9-1
    gpDevCache->SaveTerminalStatus(st_TempStatusNow.cCanAddr); //终端状态数据持久化,防止掉电丢失
    if(b_HasChargeStep == true){
        //充电状态机持久化,防止掉电丢失.
        TempOutPutChargeStep(stTempChargeStep);
        SaveChargeStep(stTempChargeStep);
    }

}

//void ChargeService::SendChargeResultData2Server(CHARGE_STEP &stChargeStep, CHARGE_STATUS_CHANGE_TYPE enChargeStatusChange)
//{
//    InfoMap qInfoMap;
//    InfoAddrType InfoType;
//    __u8 uc_ExecRet;
//    switch (enChargeStatusChange) {
//    case CHARGE_STATUS_CHANGE_START_CHARGE_FAIL:
//        uc_ExecRet = (__u8)CMD_ACK_EXE_TYPE_START_CHARGE_FAIL;
//        break;
//    default:
//        uc_ExecRet = (__u8)CMD_ACK_EXE_TYPE_SUCCESS;
//        break;
//    }
//    //发送执行结果
//    PackageExecResult2Server(stChargeStep, uc_ExecRet, qInfoMap, InfoType);
//    emit sigToBus(qInfoMap, InfoType);
//    //若结束充电要突发冻结电度数
//    if(enChargeStatusChange == CHARGE_STATUS_CHANGE_STOP_CHARGE
//            || enChargeStatusChange == CHARGE_STATUS_CHANGE_STOP_DISCHARGE){
//        ;
//    }
//}

bool ChargeService::ParseTeleindicationData(InfoMap &qInfoMap, TerminalStatus &st_OldStatus, TerminalStatus &st_NowStatus)
{
	int link = 0, status = 0;

    //CAN地址
    if(qInfoMap.contains(Addr_CanID_Comm)){
        st_OldStatus.cCanAddr = st_NowStatus.cCanAddr = qInfoMap[Addr_CanID_Comm].at(0);
        LogOut(QString("CANAddr = %1!").arg((__u8)st_OldStatus.cCanAddr), 1);
    }
    else{
        LogOut(QString("ParseTeleindicationData no CanAddr!"), 3);
        return false;
    }
    //老工作状态
    if(qInfoMap.contains(Addr_WorkState_Term)){
		status++;
        st_OldStatus.stFrameRemoteSingle.charge_status = qInfoMap[Addr_WorkState_Term].at(0);
        LogOut(QString("Old Status = %1!").arg((__u8)st_OldStatus.stFrameRemoteSingle.charge_status), 1);
    }
    //老枪连接状态
    if(qInfoMap.contains(Addr_LinkState_Term)){
		link++;
        st_OldStatus.stFrameRemoteSingle.link_status = qInfoMap[Addr_LinkState_Term].at(0);
        LogOut(QString("Old Link = %1!").arg((__u8)st_OldStatus.stFrameRemoteSingle.link_status) ,1);
    }
    //新工作状态
    if(qInfoMap.contains(Addr_WorkState_Sudden)){
		status++;	
        st_NowStatus.stFrameRemoteSingle.charge_status = qInfoMap[Addr_WorkState_Sudden].at(0);
        LogOut(QString("New Status = %1!").arg((__u8)st_NowStatus.stFrameRemoteSingle.charge_status), 1);
    }
    //新枪连接状态
    if(qInfoMap.contains(Addr_LinkState_Sudden)){
		link++;
        st_NowStatus.stFrameRemoteSingle.link_status = qInfoMap[Addr_LinkState_Sudden].at(0);
        LogOut(QString("New Link = %1!").arg((__u8)st_NowStatus.stFrameRemoteSingle.link_status), 1);
    }

	if(link > 1 || status > 1){
    	return true;
	}

    LogOut(QString("ParseTeleindicationData drop data with linkcount = %1 statuscount = %2!").arg(link).arg(status), 1);

	return false;
}


///
/// \brief ChargeService::ProcTeleControlChargeCmd 处理遥控充电指令
/// \param qInfoMap 外部发来的数据
///
void ChargeService::ProcTeleControlChargeCmd(InfoMap &qInfoMap)
{
    CHARGE_STEP stTempChargeStep = stEmptyChargeStep;//存放解析后的指令
    InfoMap TempqInfoMap;
    InfoAddrType TempInfoType;

    //解析传递参数
    ParseTeleControlChargeCmd(qInfoMap, stTempChargeStep);
    if(!CheckCanAddrValid(stTempChargeStep.ucCanAddr)){
        return;
    }
    //根据当前状态校验遥控充电指令

    CheckControlChargeCmdValid(stTempChargeStep);

    LogOut(QString("控制指令校验结果:%1!").arg((__u8) stTempChargeStep.enCmdEndReason), 1);
    //二期根据指令来源判断是否需要发送指令处理结果
    if(stTempChargeStep.enCmdEndReason != CMD_END_REASON_NULL){
        LogOut(QString("非法指令, 返回失败 CAN = %1, 指令编号: %2 !").arg(stTempChargeStep.ucCanAddr).arg((int)stTempChargeStep.enCmdEndReason), 2);
        SendCmdResult(stTempChargeStep);
        //RecordFailOrder(stTempChargeStep);
        return;
    }
    else    //多枪判断
    {
        if(CardGetCheck(stTempChargeStep.ucCanAddr) == false)
        {
            stTempChargeStep.enCmdEndReason  = CMD_END_REASON_IERR;//缓存中无该设备)
            LogOut(QString("Double非法指令, 返回失败 CAN = %1!").arg(stTempChargeStep.ucCanAddr), 2);
            SendCmdResult(stTempChargeStep);
            //RecordFailOrder(stTempChargeStep);
            return;
        }
    }

    //根据当前功率曲线确定下发指令2018-6-25功率曲线功能处理方式调整，限制功率为0直接停止充电所以取消开始充电时对功率曲线的检测
//    if(stTempChargeStep.iCurveState > 0){
//        LogOut(QString().sprintf("CAN=%d 遥控指令下发功率曲线", stTempChargeStep.ucCanAddr), 2);

//        if(findPowerCurve(stTempChargeStep)){
//            //限值为0下发暂停充电指令
//            if(stTempChargeStep.iCurveValue == 0){
//                stTempChargeStep.ucCmdValue = CHARGE_CMD_TYPE_PAUSH_CHARGE;
//                LogOut(QString().sprintf("CAN=%d 功率曲线修改遥控指令为暂停", stTempChargeStep.ucCanAddr), 2);
//            }
//        }else{
//            //未找到当前时段的策略，记录失败订单
//            //stTempChargeStep.ucStopReasonCSCU = 116;
//            //RecordFailOrder(stTempChargeStep);
//            LogOut(QString().sprintf("CAN=%d 功率曲线未发现策略，失败订单", stTempChargeStep.ucCanAddr), 2);
//            return;
//        }
//    }

    //更新充电步骤状态机.
    UpdateChargeStepCtrlChargeCmd(stTempChargeStep);
    //充电状态机持久化,防止掉电丢失.
    SaveChargeStep(stTempChargeStep);
    //二期可选择是否发送给调度模块.

    //将指令发送给总线
    TempqInfoMap.clear();
    PackageTelecontrol2ChargeEquipment(stTempChargeStep, TempqInfoMap, TempInfoType);
    LogOut(QString("Send Charge CMD = %1!").arg((__u8 )qInfoMap[Addr_ChargeCmd_Ctrl].at(0)), 1);
    emit sigToBus(TempqInfoMap, TempInfoType);
}

///
/// \brief ChargeService::ParseTeleControlChargeCmd 解析传递参数
/// \param qInfoMap 传递参数
/// \param stChargeStep 存放解析后的指令内容
///
bool ChargeService::ParseTeleControlChargeCmd(InfoMap &qInfoMap, CHARGE_STEP &stChargeStep)
{
    CHARGE_STEP stTempChargeStep;

    if(qInfoMap.contains(Addr_CanID_Comm)){//CAN地址
        stChargeStep.ucCanAddr = qInfoMap[Addr_CanID_Comm].at(0);
    }
    if(qInfoMap.contains(Addr_ChargeCmd_Ctrl)){//指令
        stChargeStep.ucCmdValue = qInfoMap[Addr_ChargeCmd_Ctrl].at(0);
    }
    if(qInfoMap.contains(Addr_OrderNumber_Ctrl)){//流水号
        memcpy(stChargeStep.sEventNo, qInfoMap[Addr_OrderNumber_Ctrl].data(), qInfoMap[Addr_OrderNumber_Ctrl].size());
    }
    if(qInfoMap.contains(Addr_CmdSrc_Ctrl)){//指令来源
        stChargeStep.ucCmdSrc = qInfoMap[Addr_CmdSrc_Ctrl].at(0);
    }
    if(qInfoMap.contains(Addr_ChargeStartReason_Ctrl)){//指令开始原因
        stChargeStep.ucStartReason = qInfoMap[Addr_ChargeStartReason_Ctrl].at(0);
    }
    if(qInfoMap.contains(Addr_ChargeStopReason_Ctrl)){//指令结束原因
        stChargeStep.ucStopReasonCloud = qInfoMap[Addr_ChargeStopReason_Ctrl].at(0);
    }
    if(qInfoMap.contains(Addr_ScanCode_customerID)){//扫码客户ID
        strcpy(stChargeStep.sScanCodeNo, qInfoMap[Addr_ScanCode_customerID].data());
    }
    if(qInfoMap.contains(Addr_Local_Emergency)){//应急充电
        stChargeStep.ucCmdSrc = CHARGE_CMD_SRC_CSCU_EMERGENCY;
        stChargeStep.cOrderType = ORDER_EMERGENCY;
	}
	if(qInfoMap.contains(Addr_Energy_ChargeType)){//充电类型
		stChargeStep.cChargeType = qInfoMap[Addr_Energy_ChargeType].at(0);
	}

    if(qInfoMap.contains(Addr_ServerType_Comm)){//服务器类型 0:云平台 1:本地服务器
        switch (qInfoMap[Addr_ServerType_Comm].at(0)) {
        case 0://云平台
            if(stChargeStep.ucCmdSrc == CHARGE_CMD_SRC_NON){
                stChargeStep.ucCmdSrc = CHARGE_CMD_SRC_APP;//1 手机APP
            }
            if(stChargeStep.ucStartReason == START_CHARGE_CMD_RSN_NON){
                stChargeStep.ucStartReason = START_CHARGE_CMD_RSN_USER_MAKE;//1 用户主动操作
            }
            break;
        case 1://本地服务器
            stChargeStep.ucCmdSrc = CHARGE_CMD_SRC_STATION_MONITOR;//2 场站监控
            if(stChargeStep.ucStartReason == START_CHARGE_CMD_RSN_NON){
                stChargeStep.ucStartReason =  START_CHARGE_CMD_RSN_STATION_ADMIN; //101场站管理员
            }
            break;
        default:
            break;
        }
    }
    if(qInfoMap.contains(Addr_Bill_Code)){//订单号 南京3.0协议
        memcpy(stChargeStep.sBillCode, qInfoMap[Addr_Bill_Code].data(), qInfoMap[Addr_Bill_Code].size());
    }
	if(qInfoMap.contains(Addr_Power_Curve_State)){//启用功率曲线 南京3.0协议
		stChargeStep.iCurveState = *((uint*)qInfoMap[Addr_Power_Curve_State].data());
	}

    LogOut(QString("接收104流水号:%1  Protobuf流水号:%2 CAN = %3 CMD = %4!")\
           .arg(qInfoMap[Addr_OrderNumber_Ctrl].toHex().data())\
           .arg(qInfoMap[Addr_Bill_Code].data())
           .arg(stChargeStep.ucCanAddr)\
           .arg(stChargeStep.ucCmdValue),2);
    stTempChargeStep.ucCanAddr = stChargeStep.ucCanAddr;
    stTempChargeStep.ucCmdValue = stChargeStep.ucCmdValue;
    memcpy(stTempChargeStep.sEventNo, stChargeStep.sEventNo, LENGTH_EVENT_NO);

    return true;
}

///
/// \brief ChargeService::CheckControlChargeCmdAckValid 根据当前状态校验遥控充电指令
/// \param stChargeStep 接收到的充电指令
/// \param ret 0正常  2.该终端没有正在进行的充电业务 3.无效ACK
///
///
void ChargeService::CheckControlChargeCmdAckValid(CHARGE_STEP stChargeStep, __s32 &ret)
{
    CHARGE_STEP stTempChargeStep = stEmptyChargeStep;

    if(gpDevCache->QueryChargeStep(stChargeStep.ucCanAddr, stTempChargeStep) == false){//充电状态机缓存
        ret = 2;//该终端没有正在进行的充电业务
        return;
    }
    else{//该终端有正在进行的充电业务
        if(stTempChargeStep.enChargeStep == CHARGE_STEP_WAITCMD_ACK){//状态机挂起
            ret = 0;
        }
        else{//状态机进行中
            ret = 3;
            return;
        }
    }
}


///
/// \brief ChargeService::CheckControlChargeCmdValid 根据当前状态校验遥控充电指令
/// \param stChargeStep 收到的指令 校验结果存放入 enCmdEndReason
///
void ChargeService::CheckControlChargeCmdValid(CHARGE_STEP &stChargeStep)
{
    TerminalStatus st_TerminalStatus = stEmptyTerminalStatus;
    CHARGE_STEP stTempChargeStep = stEmptyChargeStep;//状态机中已有的指令

    if(gpDevCache->QueryTerminalStatus(stChargeStep.ucCanAddr, st_TerminalStatus) == false){//充电设备数据缓存
        stChargeStep.enCmdEndReason = CMD_END_REASON_IERR;//缓存中无该设备
        return;
    }

    if(gpDevCache->QueryChargeStep(stChargeStep.ucCanAddr, stTempChargeStep) == false){//无充电状态机缓存
        CheckControlChargeCmdValidNoCharging(stChargeStep,  st_TerminalStatus);//该终端没有正在进行的充电业务
    }
    else{//该终端有正在进行的充电业务
        if(stTempChargeStep.enChargeStep == CHARGE_STEP_NORMAL){//状态机挂起
            CheckControlChargeCmdValidCharging(stChargeStep,  st_TerminalStatus);
        }
        else{//状态机进行中
            stChargeStep.enCmdEndReason = CMD_END_REASON_ALREADY_EXE;
        }
    }
}



///
/// \brief ChargeService::CheckControlChargeCmdValidCharging 充电控制指令校验,有充电业务的终端
/// \param stChargeStep 新收到的指令
///
void ChargeService::CheckControlChargeCmdValidCharging(CHARGE_STEP &stChargeStep, TerminalStatus &st_TerminalStatus)
{
    //对正在充电的模块下达充电指令，或对没有充电的模块下达结束充电指令,若有充电业务的订单,只允许进行充电遥控指令.
    //状态分三大类:A属于充电业务 B非充电业务 C为止
    //A类 属于充电业务
    switch (st_TerminalStatus.cStatus) {
    case CHARGE_STATUS_LIMIT://2充电-限制
        CheckControlChargeCmdValidNowLimit(stChargeStep);
        break;
    case CHARGE_STATUS_PAUSH://3充电-暂停
    case CHARGE_STATUS_CARPAUSH://车辆暂停
    case CHARGE_STATUS_DEVPAUSH://充电设备暂停
    case CHARGE_STATUS_CSCUPAUSH://CSCU暂停
        CheckControlChargeCmdValidNowPaush(stChargeStep);
        break;
    case CHARGE_STATUS_CHARGING://4充电-充电中
        CheckControlChargeCmdValidNowCharging(stChargeStep);
        break;
    case CHARGE_STATUS_DISCHARGING://10放电
        CheckControlChargeCmdValidNowDisCharging(stChargeStep);
        break;
    case CHARGE_STATUS_FINISH://8待机-已完成 A
    case CHARGE_STATUS_FULL://13待机-车已充满
        CheckControlChargeCmdValidNowChargeFinish(stChargeStep);
        break;
        //C类
    case CHARGE_STATUS_DISCONNECT://7离线-未通信
        stChargeStep.enCmdEndReason = CMD_END_REASON_DEV_OFFLINE;
        break;
    default:
        break;
    }
}

///
/// \brief ChargeService::CheckControlChargeCmdValidNoCharging 充电控制指令校验,没有充电业务的终端
/// \param stChargeStep 收到的指令
/// \param ret
///
void ChargeService::CheckControlChargeCmdValidNoCharging(CHARGE_STEP &stChargeStep, TerminalStatus &st_TerminalStatus)
{
    switch (st_TerminalStatus.cStatus) {
    //状态分三大类:A属于充电业务 B非充电业务 C为止
    //B类 非充电业务
    case CHARGE_STATUS_GUN_STANDBY://1待机-枪已连接，等待充电
    case CHARGE_STATUS_FINISH://8待机-已完成
    case CHARGE_STATUS_FULL://13待机-车已充满
    case CHARGE_STATUS_FINISH1://14待机-手动断开
    case CHARGE_STATUS_QUEUE1://15充电-排队1
    case CHARGE_STATUS_QUEUE2://16充电-排队2
    case CHARGE_STATUS_QUEUE3://17充电-排队3
    case CHARGE_STATUS_QUEUE4://18充电-排队3
    case CHARGE_STATUS_QUEUE5://19充电-排队5
    case CHARGE_STATUS_QUEUE6://20充电-排队6
    case CHARGE_STATUS_QUEUE7://21充电-排队7
    case CHARGE_STATUS_WAITING://等待中
        CheckControlChargeCmdValidNowStandby(stChargeStep);
        break;
        //B类 非充电业务
    case CHARGE_STATUS_FREE://6待机-空闲
        stChargeStep.enCmdEndReason = CMD_END_REASON_GUN_NOLINK;//未插抢
        break;
        //C类
    case CHARGE_STATUS_DISCONNECT://7离线-未通信
        stChargeStep.enCmdEndReason = CMD_END_REASON_DEV_OFFLINE;//设备离线
        break;
        //B类
    case CHARGE_STATUS_FAULT://9故障
        stChargeStep.enCmdEndReason = CMD_END_REASON_DEV_FAULT;//设备离线
        break;
    default:
        break;
    }
}


///
/// \brief ChargeService::CheckControlChargeCmdValidNowStandby 充电控制指令校验, 当前状态为插抢待机
/// \param stChargeStep 收到的指令
///
void ChargeService::CheckControlChargeCmdValidNowStandby(CHARGE_STEP &stChargeStep)
{
    switch (stChargeStep.ucCmdValue) {
    case CHARGE_CMD_TYPE_STOP_CHARGE://03H 终止充电
    case CHARGE_CMD_TYPE_STOP_CHARGE_URGENCY://04H 紧急停止充电
    case CHARGE_CMD_TYPE_LIMIT_CHARGE://05H 限制充电,对充电中的设备进行限功率
    case CHARGE_CMD_TYPE_PAUSH_CHARGE://06H 暂停充电
    case CHARGE_CMD_TYPE_RESUME://08H 恢复充电，让暂停充电的模块继续充电
    case CHARGE_CMD_TYPE_RESET://09H 复位，让限制充电的模块取消限制
    case CHARGE_CMD_TYPE_STOP_DISCHARGE://0BH 结束放电
        stChargeStep.enCmdEndReason = CMD_END_REASON_NOT_CHARGING;
        break;
    default:
        break;
    }
}


///
/// \brief ChargeService::CheckControlChargeCmdValidNowCharging 当前状态为充电中,不允许执行的指令
/// \param ControlChargeCmd
///
void ChargeService::CheckControlChargeCmdValidNowCharging(CHARGE_STEP &stChargeStep)
{
    //除了停止充电,暂停充电,限制充电,都不合法
    switch (stChargeStep.ucCmdValue) {
    case CHARGE_CMD_TYPE_START_CHARGE_NOW://01H 立即充电
    case CHARGE_CMD_TYPE_START_CHARGE_ECONOMIC:// 02H 经济充电
    case CHARGE_CMD_TYPE_RESUME://08H 恢复充电，让暂停充电的模块继续充电
    case CHARGE_CMD_TYPE_RESET://09H 复位，让限制充电的模块取消限制
    case CHARGE_CMD_TYPE_START_DISCHARGE://0AH 开始放电
    case CHARGE_CMD_TYPE_STOP_DISCHARGE://0BH 结束放电
        stChargeStep.enCmdEndReason = CMD_END_REASON_ALREADY_CHARGING;
        break;
    default:
        break;
    }
}

///
/// \brief ChargeService::CheckControlChargeCmdValidNowLimit 充电控制指令校验,当前为限制
/// \param stChargeStep 新收到的指令
///
void ChargeService::CheckControlChargeCmdValidNowLimit(CHARGE_STEP &stChargeStep)
{
    //除了复位和停止充电,都不合法
    switch (stChargeStep.ucCmdValue) {
    case CHARGE_CMD_TYPE_START_CHARGE_NOW://01H 立即充电
    case CHARGE_CMD_TYPE_START_CHARGE_ECONOMIC:// 02H 经济充电
    case CHARGE_CMD_TYPE_LIMIT_CHARGE://05H 限制充电,对充电中的设备进行限功率
    case CHARGE_CMD_TYPE_PAUSH_CHARGE://06H 暂停充电
    case CHARGE_CMD_TYPE_ADJUST_CHARGE://07H 调整充电电流
    case CHARGE_CMD_TYPE_RESUME://08H 恢复充电，让暂停充电的模块继续充电
    case CHARGE_CMD_TYPE_START_DISCHARGE://0AH 开始放电
    case CHARGE_CMD_TYPE_STOP_DISCHARGE://0BH 结束放电
        stChargeStep.enCmdEndReason = CMD_END_REASON_NOW_LIMIT;//设备限制充电中
        break;
    default:
        break;
    }
}


///
/// \brief ChargeService::CheckControlChargeCmdValidNowChargeFinish
/// \param stChargeStep
///
void ChargeService::CheckControlChargeCmdValidNowChargeFinish(CHARGE_STEP &stChargeStep)
{
    //除了开始充/放电,都不合法
    switch (stChargeStep.ucCmdValue) {
    case CHARGE_CMD_TYPE_START_CHARGE_NOW://01H 立即充电
    case CHARGE_CMD_TYPE_START_CHARGE_ECONOMIC:// 02H 经济充电
    case CHARGE_CMD_TYPE_START_DISCHARGE://0AH 开始放电
        stChargeStep.enCmdEndReason = CMD_END_REASON_NULL;
        break;
    case CHARGE_CMD_TYPE_STOP_CHARGE://03H 终止充电
    case CHARGE_CMD_TYPE_STOP_CHARGE_URGENCY://04H 紧急停止充电
    case CHARGE_CMD_TYPE_LIMIT_CHARGE://05H 限制充电,对充电中的设备进行限功率
    case CHARGE_CMD_TYPE_PAUSH_CHARGE://06H 暂停充电
    case CHARGE_CMD_TYPE_ADJUST_CHARGE://07H 调整充电电流
    case CHARGE_CMD_TYPE_RESUME://08H 恢复充电，让暂停充电的模块继续充电
    case CHARGE_CMD_TYPE_RESET://09H 复位，让限制充电的模块取消限制
    case CHARGE_CMD_TYPE_STOP_DISCHARGE://0BH 结束放电
        stChargeStep.enCmdEndReason = CMD_END_REASON_NOT_CHARGING;
        break;
    default:
        break;
    }
}


///
/// \brief ChargeService::CheckControlChargeCmdValidNowPaush
/// \param stChargeStep
///
void ChargeService::CheckControlChargeCmdValidNowPaush(CHARGE_STEP &stChargeStep)
{
    //除了恢复充电和停止充电,都不合法
    switch (stChargeStep.ucCmdValue) {
    case CHARGE_CMD_TYPE_START_CHARGE_NOW://01H 立即充电
    case CHARGE_CMD_TYPE_START_CHARGE_ECONOMIC:// 02H 经济充电
    case CHARGE_CMD_TYPE_LIMIT_CHARGE://05H 限制充电,对充电中的设备进行限功率
    case CHARGE_CMD_TYPE_PAUSH_CHARGE://06H 暂停充电
    case CHARGE_CMD_TYPE_ADJUST_CHARGE://07H 调整充电电流
    case CHARGE_CMD_TYPE_RESET://09H 复位，让限制充电的模块取消限制
    case CHARGE_CMD_TYPE_START_DISCHARGE://0AH 开始放电
    case CHARGE_CMD_TYPE_STOP_DISCHARGE://0BH 结束放电
        stChargeStep.enCmdEndReason = CMD_END_REASON_NOW_PAUSH;
        break;
    default:
        break;
    }
}

///
/// \brief ChargeService::CheckControlChargeCmdValidNowDisCharging
/// \param stChargeStep
///
void ChargeService::CheckControlChargeCmdValidNowDisCharging(CHARGE_STEP &stChargeStep)
{
    //除了停止放电,都不合法
    switch (stChargeStep.ucCmdValue) {
    case CHARGE_CMD_TYPE_START_CHARGE_NOW://01H 立即充电
    case CHARGE_CMD_TYPE_START_CHARGE_ECONOMIC:// 02H 经济充电
    case CHARGE_CMD_TYPE_STOP_CHARGE://03H 终止充电
    case CHARGE_CMD_TYPE_STOP_CHARGE_URGENCY://04H 紧急停止充电
    case CHARGE_CMD_TYPE_LIMIT_CHARGE://05H 限制充电,对充电中的设备进行限功率
    case CHARGE_CMD_TYPE_PAUSH_CHARGE://06H 暂停充电
    case CHARGE_CMD_TYPE_ADJUST_CHARGE://07H 调整充电电流
    case CHARGE_CMD_TYPE_RESUME://08H 恢复充电，让暂停充电的模块继续充电
    case CHARGE_CMD_TYPE_RESET://09H 复位，让限制充电的模块取消限制
    case CHARGE_CMD_TYPE_START_DISCHARGE://0AH 开始放电
        stChargeStep.enCmdEndReason = CMD_END_REASON_NOW_DISCHARGING;
        break;
    default:
        break;
    }
}

///
/// \brief ChargeService::UpdateChargeStepCtrlChargeCmd 由充电控制指令触发更新充电步骤状态机
/// \param stChargeStep 收到的指令
/// \return
///
bool ChargeService::UpdateChargeStepCtrlChargeCmd(CHARGE_STEP &stChargeStep)
{
    LogOut(QString("更新充电状态机 CAN = %1!").arg(stChargeStep.ucCanAddr), 1);
    bool bHasChargeStep = false;
    CHARGE_STEP stTempChargeStep = stEmptyChargeStep;//已经存在的指令

    bHasChargeStep = gpDevCache->QueryChargeStep(stChargeStep.ucCanAddr, stTempChargeStep);
    //状态机中指令状态机相关变量更新
    QDateTime dtQDateTime= QDateTime::currentDateTime();
    QDateTime2CharArray(dtQDateTime, stChargeStep.sRecvTime);
    stChargeStep.enChargeStep = CHARGE_STEP_WAITCMD_ACK;
    QDateTime2CharArray(dtQDateTime, stChargeStep.stChargeStepValue.sStartTimeStep);
    stChargeStep.stChargeStepValue.uc_hold_time  = g_TimeoutChargeStep[stChargeStep.enChargeStep];
    stChargeStep.stRepeatCmd.NowCount = 0;
    stChargeStep.stRepeatCmd.MaxCount = MAX_CAN_CMD_REPEAT;

    switch (stChargeStep.ucCmdValue) {
    case CHARGE_CMD_TYPE_START_CHARGE_NOW://01H 立即充电
    case CHARGE_CMD_TYPE_START_CHARGE_ECONOMIC:// 02H 经济充电
    case CHARGE_CMD_TYPE_STOP_CHARGE://03H 终止充电
    case CHARGE_CMD_TYPE_STOP_CHARGE_URGENCY://04H 紧急停止充电
    case CHARGE_CMD_TYPE_LIMIT_CHARGE://05H 限制充电,对充电中的设备进行限功率
    case CHARGE_CMD_TYPE_PAUSH_CHARGE://06H 暂停充电
    case CHARGE_CMD_TYPE_ADJUST_CHARGE://07H 调整充电电流
    case CHARGE_CMD_TYPE_RESUME://08H 恢复充电，让暂停充电的模块继续充电
    case CHARGE_CMD_TYPE_RESET://09H 复位，让限制充电的模块取消限制
    case CHARGE_CMD_TYPE_START_DISCHARGE://0AH 开始放电
    case CHARGE_CMD_TYPE_STOP_DISCHARGE://0BH 结束放电
        stChargeStep.stChargeStepValue.uc_charge_step_timeout = CHARGE_STEP_NORMAL;//接收ACK超时后,转到默认状态
        break;
    default:
        stChargeStep.stChargeStepValue.uc_charge_step_timeout = CHARGE_STEP_NORMAL;
        break;
    }

    if(bHasChargeStep == false){//充电业务状态机中没有该终端
        //订单相关变量初始化
        switch (stChargeStep.ucCmdValue) {
        case CHARGE_CMD_TYPE_START_CHARGE_NOW://01H 立即充电
        case CHARGE_CMD_TYPE_START_CHARGE_ECONOMIC:// 02H 经济充电
        case CHARGE_CMD_TYPE_START_DISCHARGE://0AH 开始放电
        case CHARGE_CMD_TYPE_PAUSH_CHARGE://06H 暂停充电
            CreatChargeOrder(stChargeStep, 1);
            break;
        default:
            break;
        }
        //stChargeStep.enOrderStatus = ORDER_STATUS_ING;
        gpDevCache->AddChargeStep(stChargeStep.ucCanAddr, stChargeStep);
    }
    else{//若缓存队列中已经有,则进行更新
        CopyChargeStepOrderValue(stChargeStep, stTempChargeStep);
        TempOutPutChargeStep(stChargeStep);
        //订单相关变量更新
        switch (stChargeStep.ucCmdValue) {
        case CHARGE_CMD_TYPE_START_CHARGE_NOW://01H 立即充电
        case CHARGE_CMD_TYPE_START_CHARGE_ECONOMIC:// 02H 经济充电
        case CHARGE_CMD_TYPE_START_DISCHARGE://0AH 开始放电
            CreatChargeOrder(stChargeStep, 1);
            break;
        case CHARGE_CMD_TYPE_STOP_CHARGE://03H 终止充电
        case CHARGE_CMD_TYPE_STOP_CHARGE_URGENCY://04H 紧急停止充电
            if(stChargeStep.ucStopReasonCSCU == 0){
                switch (stChargeStep.ucCmdSrc) {
                case CHARGE_CMD_SRC_APP://手机APP
                case CHARGE_CMD_SRC_CSCU://101 充电系统控制器
                case CHARGE_CMD_SRC_CAN_DEV://102 充电设备
                    stChargeStep.ucStopReasonCSCU = 101;//网络云平台指令中止
                    break;
                case CHARGE_CMD_SRC_STATION_MONITOR://场站监控
                    stChargeStep.ucStopReasonCSCU = 102;//网络场站监控中止
                    break;
                default:
                    break;
                }
            }
            break;
        default:
            break;
        }
        gpDevCache->UpateChargeStep(stChargeStep.ucCanAddr, stChargeStep);
    }

    TempOutPutChargeStep(stChargeStep);
    return true;
}

///
/// \brief ChargeService::CreatChargeOrder 创建充电订单
/// \param stChargeStep 已有的状态机
/// \param CreatReason 创建原因 1由指令创建 2设备自己启动
///
void ChargeService::CreatChargeOrder(CHARGE_STEP &stChargeStep, __u8 CreatReason)
{
    QUuid uuid;
    stChargeStep.enOrderStatus = ORDER_STATUS_ING;

	uuid = QUuid::createUuid();
	memcpy(stChargeStep.sOrderUUID, uuid.toRfc4122().data(), uuid.toRfc4122().length());

	if(CreatReason == 2){//设备自己启动
		stChargeStep.ucCmdSrc = CHARGE_CMD_SRC_CAN_DEV;//102
		stChargeStep.ucStartReason = START_CHARGE_CMD_RSN_DEV_SELF;

		stChargeStep.cChargeWay = UNKNOWN_START;
        stChargeStep.cOrderSync = 255;//设备自启动订单不上传 hd 2018-9-10
	}
	else{//1由指令创建的订单
		if(stChargeStep.ucCmdSrc == CHARGE_CMD_SRC_NON){//指令来源
			LogOut(QString("无指令来源,取默认值! CAN = %1!").arg(stChargeStep.ucCanAddr), 2);
			stChargeStep.ucCmdSrc = CHARGE_CMD_SRC_APP;
		}
		if(stChargeStep.ucStartReason == START_CHARGE_CMD_RSN_NON){
			LogOut(QString("无开始充电原因,取默认值! CAN = %1!").arg(stChargeStep.ucCanAddr), 1);
			stChargeStep.ucStartReason = START_CHARGE_CMD_RSN_USER_MAKE;
		}
	}

	CreateBill(stChargeStep);
}

///
/// \brief ChargeService::CopyChargeStepOrderValue
/// \param stChargeStep 刚组建的充电状态机
/// \param stTempChargeStep 已经有的充电状态机
///
void ChargeService::CopyChargeStepOrderValue(CHARGE_STEP &stChargeStep, CHARGE_STEP &stTempChargeStep)
{
    LogOut(QString("收到充电指令,复制已有的充电状态机 CAN = %1!").arg(stChargeStep.ucCanAddr), 1);
    switch (stChargeStep.ucCmdValue) {
    case CHARGE_CMD_TYPE_START_CHARGE_NOW://01H 立即充电
    case CHARGE_CMD_TYPE_START_CHARGE_ECONOMIC:// 02H 经济充电
    case CHARGE_CMD_TYPE_START_DISCHARGE://0AH 开始放电
        //对于刷卡,VIN车牌号等申请充电类型,卡号和VIN号要用申请之前的.
        switch (stTempChargeStep.ucStartReason) {
        case START_CHARGE_CMD_RSN_CARD_REMOTE://102 集中刷卡（远程）
        case START_CHARGE_CMD_RSN_DEV_CARD_REMOTE://106 单桩刷卡（远程）
        case START_CHARGE_CMD_RSN_SMART_CHARGE_CARD://错峰充电卡号申请
            LogOut(QString("开始充电类型为刷卡远程, 复制卡号 CAN = %1!").arg(stChargeStep.ucCanAddr), 1);
            strcpy(stChargeStep.sCardNo, stTempChargeStep.sCardNo);//复制卡号
            strcpy(stChargeStep.sScanCodeNo, stTempChargeStep.sScanCodeNo);//复制客户ID
            stChargeStep.CardSrcType = stTempChargeStep.CardSrcType;//复制卡号来源
            stChargeStep.ucStartReason = stTempChargeStep.ucStartReason;//复制启动原因
			if(stChargeStep.ucCmdSrc == CHARGE_CMD_SRC_NON)
            	stChargeStep.ucCmdSrc = stTempChargeStep.ucCmdSrc;//复制指令来源
            //stChargeStep.cChargeType = stTempChargeStep.cChargeType;//复制充电类型
            stChargeStep.cChargeWay = stTempChargeStep.cChargeWay;//复制充电方式
			if(stChargeStep.cOrderType == ORDER_NORMAL)
				stChargeStep.cOrderType = stTempChargeStep.cOrderType;//复制订单类型
			stChargeStep.cQueueGroup = stTempChargeStep.cQueueGroup;//复制轮重组
			stChargeStep.fLimitEnergy = stTempChargeStep.fLimitEnergy;//复制限制电量
			stChargeStep.cGunNum = stTempChargeStep.cGunNum;//复制枪数量
			stChargeStep.cOrderSync = stTempChargeStep.cOrderSync;//订单同步状态
            break;
        case START_CHARGE_CMD_RSN_DEV_VIN_REMOTE://108 VIN号（远程）
        case START_CHARGE_CMD_RSN_SMART_CHARGE_VIN://114 错峰充电VIN申请
        case START_CHARGE_CMD_RSN_COUPLE_CHARGE_VIN:
            LogOut(QString("开始充电类型为VIN远程, 复制VIN号 CAN = %1!").arg(stChargeStep.ucCanAddr), 1);
            memcpy(stChargeStep.sVIN, stTempChargeStep.sVIN, LENGTH_VIN_NO);
            stChargeStep.ucStartReason = stTempChargeStep.ucStartReason;//复制启动原因
			if(stChargeStep.ucCmdSrc == CHARGE_CMD_SRC_NON)
            	stChargeStep.ucCmdSrc = stTempChargeStep.ucCmdSrc;//复制指令来源
            //stChargeStep.cChargeType = stTempChargeStep.cChargeType;//复制充电类型
            stChargeStep.cChargeWay = stTempChargeStep.cChargeWay;//复制充电方式
			if(stChargeStep.cOrderType == ORDER_NORMAL)
				stChargeStep.cOrderType = stTempChargeStep.cOrderType;//复制订单类型
			stChargeStep.cQueueGroup = stTempChargeStep.cQueueGroup;//复制轮重组
			stChargeStep.fLimitEnergy = stTempChargeStep.fLimitEnergy;//复制限制电量
			stChargeStep.cGunNum = stTempChargeStep.cGunNum;//复制枪数量
			stChargeStep.cOrderSync = stTempChargeStep.cOrderSync;//订单同步状态
            break;
        case START_CHARGE_CMD_RSN_DEV_CAR_LICENCE_REMOTE://110 车牌号（远程）
        case START_CHARGE_CMD_RSN_SMART_CHARGE_CAR_LICENCE://115 错峰充电车牌号申请
            LogOut(QString("开始充电类型为车牌号远程, 复制车牌号 CAN = %1!").arg(stChargeStep.ucCanAddr), 1);
            memcpy(stChargeStep.sCarLisence, stTempChargeStep.sCarLisence, LENGTH_CAR_LISENCE);
            stChargeStep.ucStartReason = stTempChargeStep.ucStartReason;//复制启动原因
			if(stChargeStep.ucCmdSrc == CHARGE_CMD_SRC_NON)
            	stChargeStep.ucCmdSrc = stTempChargeStep.ucCmdSrc;//复制指令来源
            //stChargeStep.cChargeType = stTempChargeStep.cChargeType;//复制充电类型
            stChargeStep.cChargeWay = stTempChargeStep.cChargeWay;//复制充电方式
			if(stChargeStep.cOrderType == ORDER_NORMAL)
				stChargeStep.cOrderType = stTempChargeStep.cOrderType;//复制订单类型
			stChargeStep.cQueueGroup = stTempChargeStep.cQueueGroup;//复制轮重组
			stChargeStep.fLimitEnergy = stTempChargeStep.fLimitEnergy;//复制限制电量
			stChargeStep.cGunNum = stTempChargeStep.cGunNum;//复制枪数量
			stChargeStep.cOrderSync = stTempChargeStep.cOrderSync;//订单同步状态
            break;
        default:
            if(stChargeStep.ucStartReason == START_CHARGE_CMD_RSN_NON){
                stChargeStep.ucStartReason =  START_CHARGE_CMD_RSN_USER_MAKE;//默认值
            }
            break;
        }
        return;//收到开始充电指令,则以当前指令信息为准
        break;
    default://否则进行订单信息的还原
        //-------------------指令订单共用---------------------------------//
        stChargeStep.ucCmdSrc = stTempChargeStep.ucCmdSrc;//指令源以起始为准
        stChargeStep.ucStartReason = stTempChargeStep.ucStartReason;//开始充电原因以起始为准
        if(memcmp(stChargeStep.sEventNo, stTempChargeStep.sEventNo, LENGTH_EVENT_NO) != 0){//若流水号不同,则以新流水号为准
            LogOut(QString("流水号开始结束不一致 CAN = %1!").arg(stChargeStep.ucCanAddr), 2);
        }
        if(strcmp(stChargeStep.sCardNo, stTempChargeStep.sCardNo) != 0){//
            LogOut(QString("卡号开始结束不一致,以之前为准! CAN = %1!").arg(stChargeStep.ucCanAddr), 2);
            LogOut(QString("新卡 %1， 老卡%2!").arg(stChargeStep.sCardNo).arg (stTempChargeStep.sCardNo), 2);
            strcpy(stChargeStep.sCardNo, stTempChargeStep.sCardNo);
        }
        if(strcmp(stChargeStep.sScanCodeNo, stTempChargeStep.sScanCodeNo) != 0){//
            LogOut(QString("扫码客户ID开始结束不一致,以之前为准! CAN = %1!").arg(stChargeStep.ucCanAddr), 2);
            LogOut(QString("新卡 %1， 老卡%2!").arg(stChargeStep.sScanCodeNo).arg (stTempChargeStep.sScanCodeNo), 2);
            strcpy(stChargeStep.sScanCodeNo, stTempChargeStep.sScanCodeNo);
        }
        if(memcmp(stChargeStep.sGUIDNo, stTempChargeStep.sGUIDNo,LENGTH_GUID_NO ) != 0){//
            LogOut(QString("平台GUID开始结束不一致 CAN = %1!").arg(stChargeStep.ucCanAddr), 2);
        }
        if(memcmp(stChargeStep.sVIN, stTempChargeStep.sVIN, LENGTH_VIN_NO) != 0){//
            LogOut(QString("VIN开始结束不一致,以之前为准! CAN = %1!").arg(stChargeStep.ucCanAddr), 2);
            memcpy(stChargeStep.sVIN, stTempChargeStep.sVIN, LENGTH_VIN_NO);
        }
        if(memcmp(stChargeStep.sCarLisence, stTempChargeStep.sCarLisence, LENGTH_CAR_LISENCE) != 0){//
            LogOut(QString("车牌号开始结束不一致,以之前为准! CAN = %1!").arg(stChargeStep.ucCanAddr), 2);
            memcpy(stChargeStep.sCarLisence, stTempChargeStep.sCarLisence, LENGTH_CAR_LISENCE);
        }
        stChargeStep.ucQueueMsgFromServer = stTempChargeStep.ucQueueMsgFromServer;
        //-------------------订单相关复制---------------------------------//
        //    stChargeStep.OrderUUID = stTempChargeStep.OrderUUID;
        memcpy(stChargeStep.sOrderUUID, stTempChargeStep.sOrderUUID, LENGTH_GUID_NO);
        stChargeStep.enOrderStatus = stTempChargeStep.enOrderStatus;
        stChargeStep.u32EnergyStartCharge = stTempChargeStep.u32EnergyStartCharge;
        stChargeStep.u32EnergyEndCharge = stTempChargeStep.u32EnergyEndCharge;
        stChargeStep.u32TotalChargeEnergy = stTempChargeStep.u32TotalChargeEnergy;
        stChargeStep.u32EnergyPausetCharge = stTempChargeStep.u32EnergyPausetCharge;
        memcpy(stChargeStep.sStartTime, stTempChargeStep.sStartTime, 20);
        memcpy(stChargeStep.sEndTime, stTempChargeStep.sEndTime, 20);

        stChargeStep.u12TotalChargeTime = stTempChargeStep.u12TotalChargeTime;
        stChargeStep.ucStartSOC = stTempChargeStep.ucStartSOC;
        stChargeStep.ucEndSOC = stTempChargeStep.ucEndSOC;
        stChargeStep.ucQueueMsg = stTempChargeStep.ucQueueMsg;
        stChargeStep.ucStopReasonDev = stTempChargeStep.ucStopReasonDev;
        stChargeStep.ucStopReasonCloud = stTempChargeStep.ucStopReasonCloud;
        if(stChargeStep.ucStopReasonCSCU == 0){
            stChargeStep.ucStopReasonCSCU = stTempChargeStep.ucStopReasonCSCU;
        }
        stChargeStep.CardSrcType = stTempChargeStep.CardSrcType;
        //stChargeStep.cChargeType = stTempChargeStep.cChargeType;//复制充电类型
        stChargeStep.cChargeWay = stTempChargeStep.cChargeWay;//复制充电方式
		stChargeStep.cOrderType = stTempChargeStep.cOrderType;//复制订单类型
		stChargeStep.cQueueGroup = stTempChargeStep.cQueueGroup;//复制轮重组
		stChargeStep.fLimitEnergy = stTempChargeStep.fLimitEnergy;//复制限制电量
		stChargeStep.cGunNum = stTempChargeStep.cGunNum;//复制枪数量
		stChargeStep.cOrderSync = stTempChargeStep.cOrderSync;//订单同步状态

		//复制功率曲线相关数据
		memcpy(stChargeStep.sBillCode, stTempChargeStep.sBillCode, LENGTH_BILL_CODE);
		stChargeStep.iCurveState = stTempChargeStep.iCurveState;
		stChargeStep.iCurveStart = stTempChargeStep.iCurveStart;
		stChargeStep.iCurveStop = stTempChargeStep.iCurveStop;
		stChargeStep.cCurveType = stTempChargeStep.cCurveType;
		stChargeStep.iCurveValue = stTempChargeStep.iCurveValue;
    }
}

///
/// \brief ChargeService::UpdateChargeStepAck 由控制指令ACK触发更新充电步骤状态机
/// \param stChargeStep 收到的指令
/// \return
///
bool ChargeService::UpdateChargeStepAck(CHARGE_STEP &stChargeStep)
{
    CHARGE_STEP stTempChargeStep;//存放已有的状态机
    __u8 ucTempAddr = stChargeStep.ucCanAddr;

    if(gpDevCache->QueryChargeStep(ucTempAddr, stTempChargeStep) == false ){//队列中无
        LogOut(QString("ACK校验正确,但队列中无!"), 1);
        return false;
    }
    else{
        TempOutPutChargeStep(stTempChargeStep);
        //若缓存队列中已经有,则更新指令相关
        if( stTempChargeStep.enChargeStep == CHARGE_STEP_WAITCMD_ACK){
            if(stChargeStep.enCmdAck == CMD_ACK_TYPE_SUCCESS){
                LogOut(QString("ACK校验正确,且允许!"), 1);
                switch (stChargeStep.ucCmdValue) {
                case CHARGE_CMD_TYPE_START_CHARGE_NOW://01H 立即充电
                case CHARGE_CMD_TYPE_START_CHARGE_ECONOMIC:// 02H 经济充电
                    LogOut(QString("  !"), 1);
                    //交流终端没有等待开始充电这一步骤
                    if((stTempChargeStep.ucCanAddr <= ID_MaxACSinCanID) && (stTempChargeStep.ucCanAddr >= ID_MinACSinCanID))
                    {
                        stTempChargeStep.enChargeStep = CHARGE_STEP_NORMAL ;
                    }
                    else
                    {
                        stTempChargeStep.enChargeStep = CHARGE_STEP_WAITDEV_START_CHARGE ;
                    }
                    break;
                case CHARGE_CMD_TYPE_STOP_CHARGE://03H 终止充电
                case CHARGE_CMD_TYPE_STOP_CHARGE_URGENCY://04H 紧急停止充电
                    //交流终端没有等待开始充电这一步骤
                    if((stTempChargeStep.ucCanAddr <= ID_MaxACSinCanID) && (stTempChargeStep.ucCanAddr >= ID_MinACSinCanID))
                    {
                        stTempChargeStep.enChargeStep = CHARGE_STEP_NORMAL ;
                    }
                    else
                    {
                        stTempChargeStep.enChargeStep =  CHARGE_STEP_WAITDEV_STOP_CHARGE;
                    }
                    break;
                case CHARGE_CMD_TYPE_LIMIT_CHARGE://05H 限制充电,对充电中的设备进行限功率
                    stTempChargeStep.enChargeStep =  CHARGE_STEP_WAITDEV_LIMIT_CHARGE;
                    break;
                case CHARGE_CMD_TYPE_PAUSH_CHARGE://06H 暂停充电
                    stTempChargeStep.enChargeStep =  CHARGE_STEP_WAITDEV_PAUSH_CHARGE;
                    break;
                case CHARGE_CMD_TYPE_ADJUST_CHARGE://07H 调整充电电流
                    stTempChargeStep.enChargeStep =  CHARGE_STEP_NORMAL;
                    break;
                case CHARGE_CMD_TYPE_RESUME://08H 恢复充电，让暂停充电的模块继续充电
                    stTempChargeStep.enChargeStep =  CHARGE_STEP_WAITDEV_RESUME_CHARGE;
                    break;
                case CHARGE_CMD_TYPE_RESET://09H 复位，让限制充电的模块取消限制
                    stTempChargeStep.enChargeStep =  CHARGE_STEP_WAITDEV_RESET_CHARGE;
                    break;
                case CHARGE_CMD_TYPE_START_DISCHARGE://0AH 开始放电
                    stTempChargeStep.enChargeStep =  CHARGE_STEP_WAITDEV_START_DISCHARGE;
                    break;
                case CHARGE_CMD_TYPE_STOP_DISCHARGE://0BH 结束放电
                    stTempChargeStep.enChargeStep =  CHARGE_STEP_WAITDEV_STOP_DISCHARGE;
                    break;
                default:
                    break;
                }
                stTempChargeStep.stChargeStepValue.uc_hold_time  = g_TimeoutChargeStep[stTempChargeStep.enChargeStep];
                stTempChargeStep.stChargeStepValue.uc_charge_step_timeout = CHARGE_STEP_NORMAL;
                QDateTime dtQDateTime = QDateTime::currentDateTime();
                QDateTime2CharArray(dtQDateTime, stTempChargeStep.stChargeStepValue.sStartTimeStep);
                stTempChargeStep.enCmdEndReason = CMD_END_REASON_ACK_OK;//响应成功
                LogOut(QString("ACK正确,更新队列!"), 1);
                stChargeStep = stTempChargeStep;
                TempOutPutChargeStep(stTempChargeStep);
                gpDevCache->UpateChargeStep(stTempChargeStep.ucCanAddr, stTempChargeStep);
                //二期可以根据状态机指令来源,选择是否发送响应结果
                SendCmdResult(stTempChargeStep);
                //发送给显示屏的结果
                if(stChargeStep.ucCmdSrc == CHARGE_CMD_SRC_CSCU && stChargeStep.ucStartReason == START_CHARGE_CMD_RSN_VIN6){
                    SendVINViaApplyChargeResult(stChargeStep);
                }
            }
            else{
                LogOut(QString("非允许ACK CAN = %1!").arg(stChargeStep.ucCanAddr), 2);
                stTempChargeStep.enChargeStep =  CHARGE_STEP_NORMAL;
                stTempChargeStep.enCmdEndReason = CMD_END_REASON_WAITACK_WRONG;//非允许ACK,结束指令
                stTempChargeStep.enCmdAck = stChargeStep.enCmdAck;
                stTempChargeStep.ucStopReasonCSCU = (__u8)stTempChargeStep.enCmdAck;//CCU记录失败原因
                stChargeStep = stTempChargeStep;
                TempOutPutChargeStep(stTempChargeStep);
                 SaveStopResult(stTempChargeStep);
                //二期可以根据状态机指令来源,选择是否发送响应结果
                SendCmdResult(stTempChargeStep);
                //发送给显示屏的结果
                if(stChargeStep.ucCmdSrc == CHARGE_CMD_SRC_CSCU && stChargeStep.ucStartReason == START_CHARGE_CMD_RSN_VIN6){
                    SendVINViaApplyChargeResult(stChargeStep);
                }

                TerminalStatus stTerminalStatus = stEmptyTerminalStatus;
                gpDevCache->QueryTerminalStatus(stChargeStep.ucCanAddr, stTerminalStatus);
                if(stChargeStep.ucCmdValue == CHARGE_CMD_TYPE_PAUSH_CHARGE ||
                        stChargeStep.ucCmdValue == CHARGE_CMD_TYPE_RESUME)
                {
                    LogOut(QString("暂停或恢复充电 ACK回复失败不记录冻结电量点  CAN = %1!").arg(stChargeStep.ucCanAddr), 2);
                }
                else
                {
                    SaveStartChargeOrderMsg(stTerminalStatus, stTempChargeStep);
                    SaveStopChargeOrderMsg(stTerminalStatus, stTempChargeStep);
                    stTempChargeStep.enOrderStatus = ORDER_STATUS_FAIL;
                    TerminateOrder(stTempChargeStep);
                }
                gpDevCache->UpateChargeStep(stTempChargeStep.ucCanAddr, stTempChargeStep);
            }

        }
        else{
            LogOut(QString("ACK正确,但与状态机不匹配! CAN = %1!").arg(stChargeStep.ucCanAddr), 2);
            return false;
        }
    }
    return true;
}


///
/// \brief ChargeService::UpdateChargeStepByStatusChange 由充电状态变化引起的充电业务状态机变化
/// \param ChargeStatusChangeType 充电机状态变化类型
/// \param stChargeStep 充电业务状态机
/// \return
///
bool ChargeService::UpdateChargeStepByStatusChange(CHARGE_STATUS_CHANGE_TYPE ChargeStatusChangeType, CHARGE_STEP &stChargeStep)
{
    CHARGE_STEP_TYPE OldStepType = CHARGE_STEP_NORMAL;
    TerminalStatus stTerminalStatus = stEmptyTerminalStatus;//为了提取设备的中止原因
    bool bEndCmd = false;//指令周期是否结束 false 否, true是

    gpDevCache->QueryTerminalStatus(stChargeStep.ucCanAddr, stTerminalStatus);
    OldStepType = stChargeStep.enChargeStep;

    switch (ChargeStatusChangeType) {
    case CHARGE_STATUS_CHANGE_NON: //无
        break;
    case CHARGE_STATUS_CHANGE_START_CHARGE://开始充电
    case CHARGE_STATUS_CHANGE_START_PAUSE_CHARGE://暂停开始充电->适用于,枪连接到暂停  add by zjq
        stChargeStep.enCmdEndReason = CMD_END_REASON_EXE_OK;
        bEndCmd = UpdateChargeStepByStatusChangeStartCharge(stChargeStep);
        break;
    case CHARGE_STATUS_CHANGE_STOP_CHARGE:          //停止充电
        stChargeStep.enCmdEndReason = CMD_END_REASON_EXE_OK;
        bEndCmd = UpdateChargeStepByStatusChangeStopCharge(stChargeStep);
        break;
    case CHARGE_STATUS_CHANGE_START_DISCHARGE:   //开始放电
        stChargeStep.enCmdEndReason = CMD_END_REASON_EXE_OK;
        bEndCmd = UpdateChargeStepByStatusChangeStartDischarge(stChargeStep);
        break;
    case CHARGE_STATUS_CHANGE_STOP_DISCHARGE: //结束放电
        stChargeStep.enCmdEndReason = CMD_END_REASON_EXE_OK;
        bEndCmd = UpdateChargeStepByStatusChangeStopDischarge(stChargeStep);
        break;
    case CHARGE_STATUS_CHANGE_START_CHARGE_FAIL: //启动充电失败
        stChargeStep.enCmdEndReason = CMD_END_REASON_START_CHARGE_FAIL;
        bEndCmd = UpdateChargeStepByStatusChangeStartChargeFail(stChargeStep);
        break;
    case CHARGE_STATUS_CHANGE_PAUSE_CHARGE://暂停充电->适用于,充电中到暂停
        stChargeStep.enCmdEndReason = CMD_END_REASON_EXE_OK;
        bEndCmd = UpdateChargeStepByStatusChangePauseCharge(stChargeStep);
        break;
    case CHARGE_STATUS_CHANGE_RECOVER_PAUSE_CHARGE: //恢复暂停充电
        stChargeStep.enCmdEndReason = CMD_END_REASON_EXE_OK;
        bEndCmd = UpdateChargeStepByStatusChangeRecoverPauseCharge(stChargeStep);
        break;
    default:
        break;
    }
    if(OldStepType != stChargeStep.enChargeStep){
        stChargeStep.stChargeStepValue.uc_hold_time  = g_TimeoutChargeStep[stChargeStep.enChargeStep];
        stChargeStep.stChargeStepValue.uc_charge_step_timeout = CHARGE_STEP_NORMAL;
        QDateTime dtQDateTime = QDateTime::currentDateTime();
        QDateTime2CharArray(dtQDateTime, stChargeStep.stChargeStepValue.sStartTimeStep);
    }

    //指令周期结束
    if(bEndCmd == true){
        //发送结束指令
        SendCmdResult(stChargeStep);
        LogOut(QString("指令来源 %1, 开始原因 %2 can = %3!").arg(stChargeStep.ucCmdSrc).arg(stChargeStep.ucStartReason).arg(stChargeStep.ucCanAddr), 1);
        if(stChargeStep.ucCmdSrc == CHARGE_CMD_SRC_CSCU && stChargeStep.ucStartReason == START_CHARGE_CMD_RSN_VIN6){
            SendVINViaApplyChargeResult(stChargeStep);
        }
    }

    //结算订单
    switch (ChargeStatusChangeType) {
    case CHARGE_STATUS_CHANGE_STOP_CHARGE:          //停止充电
    case CHARGE_STATUS_CHANGE_STOP_DISCHARGE:   //结束放电
    {
        stChargeStep.enOrderStatus = ORDER_STATUS_OK;
        //记录订单结算原因
        if(stChargeStep.ucStopReasonCSCU == 0){
            stChargeStep.ucStopReasonCSCU = 106;//充电设备主动中止
        }

         stChargeStep.ucStopReasonDev =  stTerminalStatus.stFrameRemoteSingle.Stop_Result;
        TerminateOrder(stChargeStep);
    }
        break;
    case CHARGE_STATUS_CHANGE_START_CHARGE_FAIL: //启动充电失败
    {
        stChargeStep.ucStopReasonCSCU = 82;//启动充电失败
        stChargeStep.ucStopReasonDev =  stTerminalStatus.stFrameRemoteSingle.Stop_Result;
        TerminalStatus stTerminalStatus = stEmptyTerminalStatus;
        gpDevCache->QueryTerminalStatus(stChargeStep.ucCanAddr, stTerminalStatus);
        SaveStartChargeOrderMsg(stTerminalStatus, stChargeStep);
        stChargeStep.enOrderStatus = ORDER_STATUS_FAIL;
        SaveStopChargeOrderMsg(stTerminalStatus, stChargeStep);
        TerminateOrder(stChargeStep);
    }
        break;
    default:
        break;
    }

    //写入充电状态缓存
    if(gpDevCache->UpateChargeStep(stChargeStep.ucCanAddr, stChargeStep) == false){
        LogOut(QString("更新状态机失败 can = %1!").arg(stChargeStep.ucCanAddr), 3);
    }

    return true;
}

bool ChargeService::UpdateChargeStepByStatusChangeRecoverPauseCharge(CHARGE_STEP &stChargeStep)
{
    bool bEndCmd = false;//指令周期是否结束 false 否, true是

    switch (stChargeStep.enChargeStep) {
    case CHARGE_STEP_WAITDEV_START_CHARGE://等待恢暂停充电，超时时间60S
        LogOut(QString("变化为恢复暂停开始充电,且状态机为等待恢复暂停状态"), 1);
        stChargeStep.enChargeStep = CHARGE_STEP_NORMAL;
        bEndCmd =  true;
        break;
    case CHARGE_STEP_WAITCMD_ACK://等待ACK回复,超时时间5S
        if(stChargeStep.ucCmdValue == CHARGE_CMD_TYPE_RESUME ){
            LogOut(QString("收到ACK指令前充电机恢复暂停充电成功"), 1);
            stChargeStep.enChargeStep = CHARGE_STEP_NORMAL;
            bEndCmd =  true;
        }
        break;
    default:
        LogOut(QString("状态机与摇信变化有冲突"), 1);
        //增加冲突处理.
        bEndCmd =  false;
        break;
    }
    //记录开始充电信息
    TerminalStatus st_TempTerminalStatus = stEmptyTerminalStatus;
    gpDevCache->QueryTerminalStatus(stChargeStep.ucCanAddr,st_TempTerminalStatus);
    SaveRecoverPauseChargeOrderMsg(st_TempTerminalStatus, stChargeStep);
    return bEndCmd;
}

bool ChargeService::UpdateChargeStepByStatusChangeStartCharge(CHARGE_STEP &stChargeStep)
{
    bool bEndCmd = false;//指令周期是否结束 false 否, true是

    switch (stChargeStep.enChargeStep) {
    case CHARGE_STEP_WAITDEV_START_CHARGE://等待启动充电，超时时间60S
        LogOut(QString("变化为开始充电,且状态机为等待启动状态"), 1);
        stChargeStep.enChargeStep = CHARGE_STEP_NORMAL;
        bEndCmd =  true;
        break;
    case CHARGE_STEP_WAITCMD_ACK://等待ACK回复,超时时间5S
        if(stChargeStep.ucCmdValue == CHARGE_CMD_TYPE_START_CHARGE_NOW
                || stChargeStep.ucCmdValue == CHARGE_CMD_TYPE_START_CHARGE_ECONOMIC ){
            LogOut(QString("收到ACK指令前充电机充电成功"), 1);
            stChargeStep.enChargeStep = CHARGE_STEP_NORMAL;
            bEndCmd =  true;
        }
        break;
    default:
        LogOut(QString("状态机与摇信变化有冲突"), 1);
        //增加冲突处理.
        bEndCmd =  false;
        break;
    }
    //记录开始充电信息
    TerminalStatus st_TempTerminalStatus = stEmptyTerminalStatus;
    gpDevCache->QueryTerminalStatus(stChargeStep.ucCanAddr,st_TempTerminalStatus);
    SaveStartChargeOrderMsg(st_TempTerminalStatus, stChargeStep);
    return bEndCmd;
}

///
/// \brief ChargeService::UpdateChargeStepByStatusChangePauseCharge
/// \param stChargeStep
/// \return
///
bool ChargeService::UpdateChargeStepByStatusChangePauseCharge(CHARGE_STEP &stChargeStep)
{
    bool bEndCmd = false;
    switch (stChargeStep.enChargeStep) {
    case CHARGE_STEP_WAITDEV_PAUSH_CHARGE:
        LogOut(QString("变化为暂停充电,且状态机为等待暂停状态"), 1);
        stChargeStep.enChargeStep = CHARGE_STEP_NORMAL;
        bEndCmd =  true;
        break;
    case CHARGE_STEP_WAITCMD_ACK://等待ACK回复,超时时间5S
        if(stChargeStep.ucCmdValue == CHARGE_CMD_TYPE_STOP_CHARGE
                || stChargeStep.ucCmdValue == CHARGE_CMD_TYPE_STOP_CHARGE_URGENCY ){
            LogOut(QString("收到ACK指令前充电机暂停充电成功"), 1);
            stChargeStep.enChargeStep = CHARGE_STEP_NORMAL;
        }
        bEndCmd =  true;
        break;
    default:
        LogOut(QString("状态机与摇信变化有冲突"), 1);
        //增加冲突处理.
        bEndCmd =  false;
        break;
    }
    //记录暂停充电信息
    TerminalStatus st_TempTerminalStatus = stEmptyTerminalStatus;
    gpDevCache->QueryTerminalStatus(stChargeStep.ucCanAddr,st_TempTerminalStatus);
    SavePauseChargeOrderMsg(st_TempTerminalStatus, stChargeStep);
    return bEndCmd;
}

///
/// \brief ChargeService::UpdateChargeStepByStatusChangeStopCharge
/// \param stChargeStep
/// \return
///
bool ChargeService::UpdateChargeStepByStatusChangeStopCharge(CHARGE_STEP &stChargeStep)
{
    bool bEndCmd = false;
    switch (stChargeStep.enChargeStep) {
    case CHARGE_STEP_WAITDEV_STOP_CHARGE:
        LogOut(QString("变化为停止充电,且状态机为等待结束状态"), 1);
        stChargeStep.enChargeStep = CHARGE_STEP_NORMAL;
        bEndCmd =  true;
        break;
    case CHARGE_STEP_WAITCMD_ACK://等待ACK回复,超时时间5S
        if(stChargeStep.ucCmdValue == CHARGE_CMD_TYPE_STOP_CHARGE
                || stChargeStep.ucCmdValue == CHARGE_CMD_TYPE_STOP_CHARGE_URGENCY ){
            LogOut(QString("收到ACK指令前充电机结束充电成功"), 1);
            stChargeStep.enChargeStep = CHARGE_STEP_NORMAL;
        }
        bEndCmd =  true;
        break;
    default:
        LogOut(QString("状态机与摇信变化有冲突"), 1);
        //增加冲突处理.
        bEndCmd =  false;
        break;
    }
    //记录停止充电信息
    TerminalStatus st_TempTerminalStatus = stEmptyTerminalStatus;
    gpDevCache->QueryTerminalStatus(stChargeStep.ucCanAddr,st_TempTerminalStatus);
    SaveStopChargeOrderMsg(st_TempTerminalStatus, stChargeStep);
    return bEndCmd;
}


bool ChargeService::UpdateChargeStepByStatusChangeStartDischarge(CHARGE_STEP &stChargeStep)
{
    bool bEndCmd = false;

    switch (stChargeStep.enChargeStep) {
    case CHARGE_STEP_WAITDEV_START_DISCHARGE://等待启动放电，超时时间60S
        stChargeStep.enChargeStep = CHARGE_STEP_NORMAL;
        bEndCmd =  true;
        break;
    case CHARGE_STEP_WAITCMD_ACK://等待ACK回复,超时时间5S
        if(stChargeStep.ucCmdValue == CHARGE_CMD_TYPE_START_DISCHARGE ){
            stChargeStep.enChargeStep = CHARGE_STEP_NORMAL;
            bEndCmd =  true;
        }
        break;
    default:
        LogOut(QString("状态机与摇信变化有冲突"), 1);
        //增加冲突处理.
        bEndCmd =  false;
        break;
    }
    //记录开始放电信息
    TerminalStatus st_TempTerminalStatus = stEmptyTerminalStatus;
    gpDevCache->QueryTerminalStatus(stChargeStep.ucCanAddr,st_TempTerminalStatus);
    SaveStartChargeOrderMsg(st_TempTerminalStatus, stChargeStep);
    return bEndCmd;
}

bool ChargeService::UpdateChargeStepByStatusChangeStopDischarge(CHARGE_STEP &stChargeStep)
{
    bool bEndCmd = false;

    switch (stChargeStep.enChargeStep) {
    case CHARGE_STEP_WAITDEV_STOP_DISCHARGE://等待停止放电，超时时间60S
        stChargeStep.enChargeStep = CHARGE_STEP_NORMAL;
        bEndCmd =  true;
        break;
    case CHARGE_STEP_WAITCMD_ACK://等待ACK回复,超时时间5S
        if(stChargeStep.ucCmdValue == CHARGE_CMD_TYPE_STOP_DISCHARGE ){
            stChargeStep.enChargeStep = CHARGE_STEP_NORMAL;
            bEndCmd =  true;
        }
        break;
    default:
        LogOut(QString("状态机与摇信变化有冲突"), 1);
        //增加冲突处理.
        bEndCmd =  false;
        break;
    }
    return bEndCmd;
    //记录结束放电信息
    TerminalStatus st_TempTerminalStatus = stEmptyTerminalStatus;
    gpDevCache->QueryTerminalStatus(stChargeStep.ucCanAddr,st_TempTerminalStatus);
    SaveStartChargeOrderMsg(st_TempTerminalStatus, stChargeStep);
}

bool ChargeService::UpdateChargeStepByStatusChangeStartChargeFail(CHARGE_STEP &stChargeStep)
{
    bool bEndCmd = false;
    switch (stChargeStep.enChargeStep) {
    case CHARGE_STEP_WAITDEV_START_CHARGE://等待启动电，超时时间60S
        stChargeStep.enChargeStep = CHARGE_STEP_NORMAL;
        bEndCmd =  true;
        break;
    default:
        LogOut(QString("状态机与摇信变化有冲突"), 1);
        //增加冲突处理.
        bEndCmd =  false;
        break;
    }
    return bEndCmd;
}

///
/// \brief ChargeService::ProcChargeStatusChange 处理充电机工作状态字段的变化
/// \param st_OldStatus 变化前的状态
/// \param st_NowStatus 有变化的终端数据结构的引用
/// \return CHARGE_STATUS_CHANGE_TYPE
///
CHARGE_STATUS_CHANGE_TYPE ChargeService::ProcChargeStatusChange(TerminalStatus &st_OldStatus, TerminalStatus &st_NowStatus)
{
    CHARGE_STATUS_CHANGE_TYPE ChargeStatusChangeType = CHARGE_STATUS_CHANGE_NON;

    LogOut(QString("ProcChargeStatusChange can = %1 !").arg((__u8)st_OldStatus.cCanAddr), 1);
    st_NowStatus.en_ChargeStatusChangeType = CHARGE_STATUS_CHANGE_NON;

    switch (st_NowStatus.stFrameRemoteSingle.charge_status){
    //1.实时工作状态为待机
    case CHARGE_STATUS_REALTIME_STANDBY: //0 待机
        ChargeStatusChangeType=  ProcChargeStatus2Standby( st_OldStatus, st_NowStatus);
        break;
        //2.实时工作状态为充电中
    case CHARGE_STATUS_REALTIME_CHARGING://1.充电中
        ChargeStatusChangeType= ProcChargeStatus2Charging(st_OldStatus, st_NowStatus);
        break;
        //3.实时工作状态为故障
    case CHARGE_STATUS_REALTIME_FAULT://2.故障
        ChargeStatusChangeType= ProcChargeStatus2Fault( st_OldStatus, st_NowStatus);
        break;
        //4.实时工作状态为启动中
    case CHARGE_STATUS_REALTIME_STARTING://3.启动中
        ChargeStatusChangeType = CHARGE_STATUS_CHANGE_STARTING;
        break;
        //5.实时工作状态为暂停
    case CHARGE_STATUS_REALTIME_PAUSE://4.暂停
    case CHARGE_STATUS_REALTIME_CARPAUSE://11.车辆暂停
    case CHARGE_STATUS_REALTIME_DEVPAUSE://12.充电设备暂停
        ChargeStatusChangeType=  ProcChargeStatus2Pause( st_OldStatus, st_NowStatus);
        break;
        //6.实时工作状态为限制
    case CHARGE_STATUS_REALTIME_LIMIT://5.限制
        break;
        //7.实时工作状态为切换中
    case CHARGE_STATUS_REALTIME_SWITCH://7.切换中
        break;
        //8.实时工作状态为放电中
    case CHARGE_STATUS_REALTIME_DISCHARGING: //8放电
        ChargeStatusChangeType= ProcChargeStatus2Discharge(st_OldStatus, st_NowStatus);
        break;
        //10.实时工作状态为等待中
    case CHARGE_STATUS_REALTIME_WAITING://10.等待中
        ChargeStatusChangeType= ProcChargeStatus2Waiting(st_OldStatus, st_NowStatus);

        break;
    default:
        break;
    }

    //更新至缓存
    TerminalStatus & stTerminalStatus = gpDevCache->GetUpdateTerminalStatus(st_NowStatus.cCanAddr);
    stTerminalStatus.en_ChargeStatusChangeType = ChargeStatusChangeType;
    gpDevCache->FreeUpdateTerminalStatus();

    return ChargeStatusChangeType;
}

///
/// \brief ChargeService::ProcChargeStatus2Standby 充电机工作状态由其他状态变为待机
/// \param st_OldStatus 变化前的状态
/// \param st_NowStatus 有变化的终端数据结构的引用
/// \return CHARGE_STATUS_CHANGE_TYPE
///
CHARGE_STATUS_CHANGE_TYPE ChargeService::ProcChargeStatus2Standby(TerminalStatus &st_OldStatus, TerminalStatus &st_NowStatus)
{
    CHARGE_STATUS_CHANGE_TYPE ChargeStatusChangeType = CHARGE_STATUS_CHANGE_NON;

    //根据跳变前的充电终端的“实时工作状态”，更新“充电业务状态机”
    switch (st_OldStatus.stFrameRemoteSingle.charge_status) {
    //1.1 由充电变成待机
    case CHARGE_STATUS_REALTIME_CHARGING://1.充电中
        ChargeStatusChangeType = st_NowStatus.en_ChargeStatusChangeType = CHARGE_STATUS_CHANGE_STOP_CHARGE;
        break;
        //1.2 由故障变成待机
    case CHARGE_STATUS_REALTIME_FAULT://2.故障
        break;
        //1.3 由启动中变成待机
    case CHARGE_STATUS_REALTIME_STARTING://3.启动中
        ChargeStatusChangeType = st_NowStatus.en_ChargeStatusChangeType = CHARGE_STATUS_CHANGE_START_CHARGE_FAIL;
        break;
        //1.4 由暂停变成待机 add by XX 2017-05-03
    case CHARGE_STATUS_REALTIME_PAUSE://4.暂停
    case CHARGE_STATUS_REALTIME_CARPAUSE://11.车辆暂停
    case CHARGE_STATUS_REALTIME_DEVPAUSE://12.充电设备暂停
        ChargeStatusChangeType = st_NowStatus.en_ChargeStatusChangeType = CHARGE_STATUS_CHANGE_STOP_CHARGE;
        break;
        //1.5 由限制变成待机
    case CHARGE_STATUS_REALTIME_LIMIT://5.限制
        break;
        //1.6 由离线变成待机
    case CHARGE_STATUS_REALTIME_OFFLINE://6.离线
        ChargeStatusChangeType = ProcChargeStatusOffline2Standby(st_OldStatus.cCanAddr);
        break;
        //1.7 由切换中变成待机
    case CHARGE_STATUS_REALTIME_SWITCH://7.切换中
        break;
        //1.8 由放电变成待机
    case CHARGE_STATUS_REALTIME_DISCHARGING: //8放电
        ChargeStatusChangeType = st_NowStatus.en_ChargeStatusChangeType = CHARGE_STATUS_CHANGE_STOP_DISCHARGE;
        break;
        //1.9 由等待中变成待机 add by XX 2017-05-03
    case CHARGE_STATUS_REALTIME_WAITING: //等待中
        ChargeStatusChangeType = st_NowStatus.en_ChargeStatusChangeType = CHARGE_STATUS_CHANGE_STOP_CHARGE;//CHARGE_STATUS_CHANGE_START_CHARGE_FAIL;hd 2018-9-4
        break;
    default:
        break;
    }
    return ChargeStatusChangeType;
}

///
/// \brief ChargeService::ProcChargeStatus2Charging 充电机工作状态由其他状态变为充电中
/// \param st_OldStatus 变化前的状态
/// \param TermianlStatus &st_NowStatus
/// \return
///
CHARGE_STATUS_CHANGE_TYPE ChargeService::ProcChargeStatus2Charging(TerminalStatus &st_OldStatus, TerminalStatus &st_NowStatus)
{
    CHARGE_STATUS_CHANGE_TYPE ChargeStatusChangeType = CHARGE_STATUS_CHANGE_NON;

    switch (st_OldStatus.stFrameRemoteSingle.charge_status) {
    //2.1 由待机变成充电中
    case CHARGE_STATUS_REALTIME_STANDBY: //0 待机
        ChargeStatusChangeType = st_NowStatus.en_ChargeStatusChangeType = CHARGE_STATUS_CHANGE_START_CHARGE;
        break;
        //2.2 由故障变成充电中
    case CHARGE_STATUS_REALTIME_FAULT://2.故障
        break;
        //2.3 由启动中变成充电中
    case CHARGE_STATUS_REALTIME_STARTING://3.启动中
        ChargeStatusChangeType = st_NowStatus.en_ChargeStatusChangeType = CHARGE_STATUS_CHANGE_START_CHARGE;
        break;
        //2.4 由暂停变成充电中
    case CHARGE_STATUS_REALTIME_PAUSE://4.暂停
    case CHARGE_STATUS_REALTIME_CARPAUSE://11.车辆暂停
    case CHARGE_STATUS_REALTIME_DEVPAUSE://12.充电设备暂停
         ChargeStatusChangeType = st_NowStatus.en_ChargeStatusChangeType = CHARGE_STATUS_CHANGE_RECOVER_PAUSE_CHARGE;
        break;
        //2.5 由限制变成充电中
    case CHARGE_STATUS_REALTIME_LIMIT://5.限制
        break;
        //2.6 由切换中变成充电中
    case CHARGE_STATUS_REALTIME_SWITCH://6.切换中
        break;
        //2.7 由放电变成充电中
    case CHARGE_STATUS_REALTIME_DISCHARGING: //7放电
        ChargeStatusChangeType = st_NowStatus.en_ChargeStatusChangeType = CHARGE_STATUS_CHANGE_START_CHARGE;
        break;
        //2.8 由等待中变成充电中 add by XX 2017-05-03
    case CHARGE_STATUS_REALTIME_WAITING://10 等待中
        ChargeStatusChangeType = st_NowStatus.en_ChargeStatusChangeType = CHARGE_STATUS_CHANGE_RECOVER_PAUSE_CHARGE;//CHARGE_STATUS_CHANGE_START_CHARGE;
        break;
    default:
        break;
    }
    return ChargeStatusChangeType;
}


///
/// \brief ChargeService::ProcChargeStatus2Charging 充电机工作状态由其他状态变为等待中
/// \param st_OldStatus 变化前的状态
/// \param TermianlStatus &st_NowStatus
/// \return
///
CHARGE_STATUS_CHANGE_TYPE ChargeService::ProcChargeStatus2Waiting(TerminalStatus &st_OldStatus, TerminalStatus &st_NowStatus)
{
    CHARGE_STATUS_CHANGE_TYPE ChargeStatusChangeType = CHARGE_STATUS_CHANGE_NON;

    switch (st_OldStatus.stFrameRemoteSingle.charge_status) {
    //2.1 由待机变成等待中
    case CHARGE_STATUS_REALTIME_STANDBY: //0 待机
        ChargeStatusChangeType = st_NowStatus.en_ChargeStatusChangeType = CHARGE_STATUS_CHANGE_START_PAUSE_CHARGE;
        break;
        //2.2 由故障变成等待中
    case CHARGE_STATUS_REALTIME_FAULT://2.故障
        break;
        //2.3 由启动中变成等待中
    case CHARGE_STATUS_REALTIME_STARTING://3.启动中
        ChargeStatusChangeType = st_NowStatus.en_ChargeStatusChangeType = CHARGE_STATUS_CHANGE_START_PAUSE_CHARGE;
        break;
        //2.4 由暂停变成充电中
    case CHARGE_STATUS_REALTIME_PAUSE://4.暂停
    case CHARGE_STATUS_REALTIME_CARPAUSE://11.车辆暂停
    case CHARGE_STATUS_REALTIME_DEVPAUSE://12.充电设备暂停
         //ChargeStatusChangeType = st_NowStatus.en_ChargeStatusChangeType = CHARGE_STATUS_CHANGE_RECOVER_PAUSE_CHARGE;
        break;
        //2.5 由限制变成等待中
    case CHARGE_STATUS_REALTIME_LIMIT://5.限制
        break;
        //2.6 由切换中变成等待中
    case CHARGE_STATUS_REALTIME_SWITCH://6.切换中
        break;
        //2.7 由放电变成等待中
    case CHARGE_STATUS_REALTIME_DISCHARGING: //7放电
        //ChargeStatusChangeType = st_NowStatus.en_ChargeStatusChangeType = CHARGE_STATUS_CHANGE_START_CHARGE;
        break;
        //2.8 由等待中变成等待中 add by XX 2017-05-03
    case CHARGE_STATUS_REALTIME_WAITING://10 等待中
        //ChargeStatusChangeType = st_NowStatus.en_ChargeStatusChangeType = CHARGE_STATUS_CHANGE_RECOVER_PAUSE_CHARGE;//CHARGE_STATUS_CHANGE_START_CHARGE;
        break;
    default:
        break;
    }
    return ChargeStatusChangeType;
}
///
/// \brief ChargeService::ProcChargeStatus2Pause 充电机工作状态由其他状态变为暂停
/// \param st_OldStatus 变化前的状态
/// \param can_addr 变化的充电终端CAN地址
/// \return
///
CHARGE_STATUS_CHANGE_TYPE ChargeService::ProcChargeStatus2Pause(TerminalStatus &st_OldStatus, TerminalStatus &st_NowStatus)
{
    CHARGE_STATUS_CHANGE_TYPE ChargeStatusChangeType = CHARGE_STATUS_CHANGE_NON;

    switch (st_OldStatus.stFrameRemoteSingle.charge_status)
    {
    //3.1 由待机变成暂停
    case CHARGE_STATUS_REALTIME_STANDBY: //0 待机
        ChargeStatusChangeType = st_NowStatus.en_ChargeStatusChangeType = CHARGE_STATUS_CHANGE_START_PAUSE_CHARGE;
        break;
        //3.2 由充电中变成暂停
    case CHARGE_STATUS_REALTIME_CHARGING://1.充电中
        ChargeStatusChangeType = st_NowStatus.en_ChargeStatusChangeType = CHARGE_STATUS_CHANGE_PAUSE_CHARGE;
        break;
        //3.3 由故障变成暂停
    case CHARGE_STATUS_REALTIME_FAULT://2.故障
        break;
        //3.4 由启动中变成暂停
    case CHARGE_STATUS_REALTIME_STARTING://3.启动中
        ChargeStatusChangeType = st_NowStatus.en_ChargeStatusChangeType = CHARGE_STATUS_CHANGE_PAUSE_CHARGE;
        break;
        //3.5 由限制变成暂停
    case CHARGE_STATUS_REALTIME_LIMIT://5.限制
        break;
        //3.6 由离线变成暂停
    case CHARGE_STATUS_REALTIME_OFFLINE://6.离线
        ChargeStatusChangeType = st_NowStatus.en_ChargeStatusChangeType = CHARGE_STATUS_CHANGE_PAUSE_CHARGE;
        break;
        //3.7 由切换中变成暂停
    case CHARGE_STATUS_REALTIME_SWITCH://7.切换中
        break;
        //3.8 由放电变成暂停
    case CHARGE_STATUS_REALTIME_DISCHARGING://8.放电
        ChargeStatusChangeType = st_NowStatus.en_ChargeStatusChangeType = CHARGE_STATUS_CHANGE_PAUSE_CHARGE;
        break;
    default:
        break;
    }

    return ChargeStatusChangeType;
}

///
/// \brief ChargeService::ProcChargeStatus2Fault 充电机工作状态由其他状态变为故障
/// \param st_OldStatus 变化前的状态
/// \param can_addr 变化的充电终端CAN地址
/// \return
///
CHARGE_STATUS_CHANGE_TYPE ChargeService::ProcChargeStatus2Fault(TerminalStatus &st_OldStatus, TerminalStatus &st_NowStatus)
{
    CHARGE_STATUS_CHANGE_TYPE ChargeStatusChangeType = CHARGE_STATUS_CHANGE_NON;

    switch (st_OldStatus.stFrameRemoteSingle.charge_status){
    //3.1 由待机变成故障
    case CHARGE_STATUS_REALTIME_STANDBY: //0 待机
        break;
        //3.2 由充电中变成故障(这里默认为只要故障，充电业务已经停止)
    case CHARGE_STATUS_REALTIME_CHARGING://1.充电中
        ChargeStatusChangeType = st_NowStatus.en_ChargeStatusChangeType = CHARGE_STATUS_CHANGE_STOP_CHARGE;
        break;
        //3.3 由启动中变成故障
    case CHARGE_STATUS_REALTIME_STARTING://3.启动中
        ChargeStatusChangeType = st_NowStatus.en_ChargeStatusChangeType = CHARGE_STATUS_CHANGE_START_CHARGE_FAIL;
        break;
        //3.4 由暂停变成故障
    case CHARGE_STATUS_REALTIME_PAUSE://4.暂停
    case CHARGE_STATUS_REALTIME_CARPAUSE://11.车辆暂停
    case CHARGE_STATUS_REALTIME_DEVPAUSE://12.充电设备暂停
        ChargeStatusChangeType = st_NowStatus.en_ChargeStatusChangeType = CHARGE_STATUS_CHANGE_STOP_CHARGE;
        break;
        //3.5 由限制变成故障
    case CHARGE_STATUS_REALTIME_LIMIT://5.限制
        break;
        //3.6 由切换中变成故障
    case CHARGE_STATUS_REALTIME_SWITCH://6.切换中
        break;
        //3.7 由放电变成故障
    case CHARGE_STATUS_REALTIME_DISCHARGING: //7放电
        ChargeStatusChangeType = st_NowStatus.en_ChargeStatusChangeType = CHARGE_STATUS_CHANGE_STOP_DISCHARGE;
        break;
        //3.8 由等待中变成故障 add by XX 2017-05-03
    case CHARGE_STATUS_REALTIME_WAITING://10.等待中
        ChargeStatusChangeType = st_NowStatus.en_ChargeStatusChangeType = CHARGE_STATUS_CHANGE_START_CHARGE_FAIL;
        break;
    default:
        break;
    }
    return ChargeStatusChangeType;
}


///
/// \brief ChargeService::ProcChargeStatus2Discharge 充电机工作状态由其他状态变为放电中
/// \param st_OldStatus 变化前的状态
/// \param can_addr 变化的充电终端CAN地址
/// \return
///
CHARGE_STATUS_CHANGE_TYPE ChargeService::ProcChargeStatus2Discharge(TerminalStatus &st_OldStatus, TerminalStatus &st_NowStatus)
{
    CHARGE_STATUS_CHANGE_TYPE ChargeStatusChangeType = CHARGE_STATUS_CHANGE_NON;

    switch (st_OldStatus.stFrameRemoteSingle.charge_status) {
    //8.1 由待机变为放电中
    case CHARGE_STATUS_REALTIME_STANDBY: //0 待机
        ChargeStatusChangeType = st_NowStatus.en_ChargeStatusChangeType = CHARGE_STATUS_CHANGE_START_DISCHARGE;
        break;
        //8.2 由充电中变成放电中
    case CHARGE_STATUS_REALTIME_CHARGING://1.充电中
        break;
        //8.3 由故障变成放电中
    case CHARGE_STATUS_REALTIME_FAULT://2.故障
        break;
        //8.4 由启动中变成放电中
    case CHARGE_STATUS_REALTIME_STARTING://3.启动中
        ChargeStatusChangeType = st_NowStatus.en_ChargeStatusChangeType = CHARGE_STATUS_CHANGE_START_DISCHARGE;
        break;
        //8.5 由暂停变成放电中
    case CHARGE_STATUS_REALTIME_PAUSE://4.暂停
    case CHARGE_STATUS_REALTIME_CARPAUSE://11.车辆暂停
    case CHARGE_STATUS_REALTIME_DEVPAUSE://12.充电设备暂停
        break;
        //8.6 由限制变成放电中
    case CHARGE_STATUS_REALTIME_LIMIT://5.限制
        break;
        //8.7 由切换中变成放电中
    case CHARGE_STATUS_REALTIME_SWITCH://6.切换中
        break;
        //8.8 由等待中变成放电中 add by XX 2017-05-03
    case CHARGE_STATUS_REALTIME_WAITING://10.等待中
        ChargeStatusChangeType = st_NowStatus.en_ChargeStatusChangeType = CHARGE_STATUS_CHANGE_START_DISCHARGE;
        break;
    default:
        break;
    }
    return ChargeStatusChangeType;
}

CHARGE_STATUS_CHANGE_TYPE ChargeService::ProcChargeStatusOffline2Standby(__u8 ucCanAddr)
{
    CHARGE_STATUS_CHANGE_TYPE ChargeStatusChangeType = CHARGE_STATUS_CHANGE_NON;
    CHARGE_STEP stChargeStep = stEmptyChargeStep;
    if(gpDevCache->QueryChargeStep(ucCanAddr, stChargeStep) == false){
        return ChargeStatusChangeType;
    }
    if(stChargeStep.enOrderStatus == ORDER_STATUS_ING){
        LogOut(QString("离线之前是在充电, 当前为待机! CAN = %1!").arg(stChargeStep.ucCanAddr) ,2);
        ChargeStatusChangeType = CHARGE_STATUS_CHANGE_STOP_CHARGE;
    }
    return ChargeStatusChangeType;
}


CHARGE_STATUS_CHANGE_TYPE ChargeService::ProcChargeStatusOffline2Fault(__u8 ucCanAddr)
{
    CHARGE_STATUS_CHANGE_TYPE ChargeStatusChangeType = CHARGE_STATUS_CHANGE_NON;
    CHARGE_STEP stChargeStep = stEmptyChargeStep;
    if(gpDevCache->QueryChargeStep(ucCanAddr, stChargeStep) == false){
        return ChargeStatusChangeType;
    }
    if(stChargeStep.enOrderStatus == ORDER_STATUS_ING){
        LogOut(QString("离线之前是在充电, 当前为故障! CAN = %1!").arg(stChargeStep.ucCanAddr) ,2);
        ChargeStatusChangeType = CHARGE_STATUS_CHANGE_STOP_CHARGE;
    }
    return ChargeStatusChangeType;
}


CHARGE_STATUS_CHANGE_TYPE ChargeService::ProcChargeStatusOffline2Charging(__u8 ucCanAddr)
{
    CHARGE_STATUS_CHANGE_TYPE ChargeStatusChangeType = CHARGE_STATUS_CHANGE_NON;
    CHARGE_STEP stChargeStep = stEmptyChargeStep;
    if(gpDevCache->QueryChargeStep(ucCanAddr, stChargeStep) == false){
        return ChargeStatusChangeType;
    }
    switch (stChargeStep.enOrderStatus) {
    case ORDER_STATUS_NON:
    case ORDER_STATUS_FAIL:
    case ORDER_STATUS_OK:
        LogOut(QString("离线之前没有充电, 当前为充电中! CAN = %1!").arg(stChargeStep.ucCanAddr) ,2);
        ChargeStatusChangeType = CHARGE_STATUS_CHANGE_START_CHARGE;
        break;
    default:
        break;
    }
    return ChargeStatusChangeType;
}

///
/// \brief ChargeService::SaveStartChargeOrderMsg 记录订单恢复暂停起始信息
/// \param stTerminalStatus
/// \param stChargeStep
/// \return
///
bool ChargeService::SaveRecoverPauseChargeOrderMsg(TerminalStatus &stTerminalStatus, CHARGE_STEP &stChargeStep)
{
    LogOut(QString("恢复暂停开始SOC:%1!").arg(stTerminalStatus.stFrameBmsInfo.batery_SOC), 2);
    //起始电量
    switch (stTerminalStatus.en_ChargeStatusChangeType) {
    case CHARGE_STATUS_CHANGE_START_CHARGE:
        LogOut(QString("恢复暂停开始电量:%1!").arg(QString::number((double)stTerminalStatus.stFrameRemoteMeSurement2.active_electric_energy / 100.0, 'f', 2)), 2);
        break;
    case CHARGE_STATUS_CHANGE_START_DISCHARGE:
        break;
    default:
        LogOut(QString("默认,恢复暂停开始电量:%1!").arg(QString::number((double)stTerminalStatus.stFrameRemoteMeSurement2.active_electric_energy / 100.0, 'f', 2)), 2);
        break;
    }
//stChargeStep.bChargeFreezeEnery= false;
    return true;
}

///
/// \brief ChargeService::SaveStartChargeOrderMsg 记录订单起始信息
/// \param stTerminalStatus
/// \param stChargeStep
/// \return
///
bool ChargeService::SaveStartChargeOrderMsg(TerminalStatus &stTerminalStatus, CHARGE_STEP &stChargeStep)
{
    if(stChargeStep.ucCmdValue != CHARGE_CMD_TYPE_RESUME)//当指令是恢复暂停充电的话则不进行电量存储
    {
        QDateTime dtQDateTime = QDateTime::currentDateTime();//起始时间
        QDateTime2CharArray(dtQDateTime, stChargeStep.sStartTime);
        stChargeStep.ucStartSOC = stTerminalStatus.stFrameBmsInfo.batery_SOC;//开始时的SOC add by YCZ 2015-12-18
        LogOut(QString("开始SOC:%1!").arg(stChargeStep.ucStartSOC), 2);
        //起始电量
        switch (stTerminalStatus.en_ChargeStatusChangeType) {
        case CHARGE_STATUS_CHANGE_START_CHARGE:
            stChargeStep.u32EnergyStartCharge = stTerminalStatus.stFrameRemoteMeSurement2.active_electric_energy;
            LogOut(QString("开始电量:%1!").arg(QString::number((double)stChargeStep.u32EnergyStartCharge / 100.0, 'f', 2)), 2);
            break;
        case CHARGE_STATUS_CHANGE_START_DISCHARGE:
            stChargeStep.u32EnergyStartCharge = stTerminalStatus.stFrameRemoteMeSurement2.ReverseActiveEnergy;
            break;
        case CHARGE_STATUS_CHANGE_START_PAUSE_CHARGE:
            stChargeStep.u32EnergyStartCharge = stTerminalStatus.stFrameRemoteMeSurement2.active_electric_energy;
            LogOut(QString("插枪->暂停,开始电量:%1!").arg(QString::number((double)stChargeStep.u32EnergyStartCharge / 100.0, 'f', 2)), 2);
            break;
        default:
            stChargeStep.u32EnergyStartCharge = stTerminalStatus.stFrameRemoteMeSurement2.active_electric_energy;
            LogOut(QString("默认,开始电量:%1!").arg(QString::number((double)stChargeStep.u32EnergyStartCharge / 100.0, 'f', 2)), 2);
            break;
        }

		//副枪订单标记为不上传
		if(stTerminalStatus.gunType == 2){
            stChargeStep.cOrderSync = 255;
		}

        if(stTerminalStatus.validFlag != ALL_VALID_NUMBER){//保存电量时检测数据可信性
            if(DataTrustableFlagMap.contains(stTerminalStatus.cCanAddr)){
                DataTrustableFlagMap.insert(stTerminalStatus.cCanAddr, false);
            }
            else{
                DataTrustableFlagMap[stTerminalStatus.cCanAddr] = false;
            }
            LogOut(QString("开始计量数据无效 CAN=%1!").arg(stChargeStep.ucCanAddr), 2);
        }
        else{//数据无效时不去更新"电度数统计表"
            iStartOrStopFlag = 1;
            SaveChargeEnergyToDB(stChargeStep, stTerminalStatus,iStartOrStopFlag);
			UpdateBeginBill(stChargeStep);
        }
    }	

    //更新"充电订单数据表"
    return true;
}

///
/// \brief ChargeService::SavePauseChargeOrderMsg 记录订单暂停信息
/// \param stTerminalStatus
/// \param stChargeStep
/// \return
///
bool ChargeService::SavePauseChargeOrderMsg(TerminalStatus &stTerminalStatus, CHARGE_STEP &stChargeStep)
{
    QDateTime dtQDateTime = QDateTime::currentDateTime();//暂停时间

    LogOut(QString("暂停SOC:%1!").arg(stTerminalStatus.stFrameBmsInfo.batery_SOC), 2);
    //结束电量
    switch (stTerminalStatus.en_ChargeStatusChangeType) {
    case CHARGE_STATUS_CHANGE_PAUSE_CHARGE:
        stChargeStep.u32EnergyPausetCharge = stTerminalStatus.stFrameRemoteMeSurement2.active_electric_energy;
        LogOut(QString("暂停电量:%1!").arg(QString::number((double)stTerminalStatus.stFrameRemoteMeSurement2.active_electric_energy / 100.0, 'f', 2)), 2);
        break;
    default:
        stChargeStep.u32EnergyPausetCharge = stTerminalStatus.stFrameRemoteMeSurement2.active_electric_energy;
        LogOut(QString("默认,暂停电量:%1!").arg(QString::number((double)stChargeStep.u32EnergyPausetCharge / 100.0, 'f', 2)), 2);
        break;
    }

    if(stTerminalStatus.validFlag != ALL_VALID_NUMBER){//保存电量时检测数据可信性
        if(DataTrustableFlagMap.contains(stTerminalStatus.cCanAddr)){
            DataTrustableFlagMap.insert(stTerminalStatus.cCanAddr, false);
        }
        else{
            DataTrustableFlagMap[stTerminalStatus.cCanAddr] = false;
        }
        LogOut(QString("SavePauseChargeOrderMsg Data Invalid CAN = %1").arg(stChargeStep.ucCanAddr), 2);
    }
    else{//数据无效时不去更新"电度数统计表"
        LogOut(QString("Don't SavePauseChargeOrderMsg SaveChargeEnergyToDB CAN = %1").arg(stChargeStep.ucCanAddr), 2);
    }
    //stChargeStep.bChargeFreezeEnery = false;
    return true;
}

///
/// \brief ChargeService::SaveStopChargeOrderMsg 记录订单结束信息
/// \param stTerminalStatus
/// \param stChargeStep
/// \return
///
bool ChargeService::SaveStopChargeOrderMsg(TerminalStatus &stTerminalStatus, CHARGE_STEP &stChargeStep)
{
    QDateTime dtQDateTimeTmp;

    QDateTime dtQDateTime = QDateTime::currentDateTime();//结束时间
    dtQDateTimeTmp = dtQDateTime;
    QDateTime2CharArray(dtQDateTime, stChargeStep.sEndTime);
    CharArray2QDateTime(stChargeStep.sStartTime, dtQDateTime);
    stChargeStep.u12TotalChargeTime = dtQDateTime.secsTo(QDateTime::currentDateTime())/60;//总充电时长

    stChargeStep.ucEndSOC = stTerminalStatus.stFrameBmsInfo.batery_SOC;//结束时的SOC add by YCZ 2015-12-18
    LogOut(QString("结束SOC:%1!").arg(stChargeStep.ucEndSOC), 2);
    //结束电量
    switch (stTerminalStatus.en_ChargeStatusChangeType) {
    case CHARGE_STATUS_CHANGE_STOP_CHARGE:
        stChargeStep.u32EnergyEndCharge = stTerminalStatus.stFrameRemoteMeSurement2.active_electric_energy;
        LogOut(QString("结束电量:%1!").arg(QString::number((double)stChargeStep.u32EnergyEndCharge / 100.0, 'f', 2)), 2);
        break;
    case CHARGE_STATUS_CHANGE_STOP_DISCHARGE:
        stChargeStep.u32EnergyEndCharge = stTerminalStatus.stFrameRemoteMeSurement2.ReverseActiveEnergy;
        LogOut(QString("结束电量:%1!").arg(QString::number((double)stChargeStep.u32EnergyEndCharge / 100.0, 'f', 2)), 2);
        break;
    default:
        stChargeStep.u32EnergyEndCharge = stTerminalStatus.stFrameRemoteMeSurement2.active_electric_energy;
        LogOut(QString("默认,结束电量:%1!").arg(QString::number((double)stChargeStep.u32EnergyEndCharge / 100.0, 'f', 2)), 2);
        break;
    }

    int iTempTotalChargeEnergy = 0;
    double dAmmeterRange = gpParamSet->getAmmeterRange(stChargeStep.ucCanAddr);

    iTempTotalChargeEnergy = stChargeStep.u32EnergyEndCharge - stChargeStep.u32EnergyStartCharge;
    if(iTempTotalChargeEnergy >= 0){
        stChargeStep.u32TotalChargeEnergy = iTempTotalChargeEnergy;//总充放电电能
    }else {
        if((stChargeStep.u32EnergyStartCharge > ((uint)(dAmmeterRange * 100.0) - 60000) ) \
                && (stChargeStep.u32EnergyEndCharge  <  60000) ){
            iTempTotalChargeEnergy = (int)(dAmmeterRange * 100.0) - stChargeStep.u32EnergyStartCharge + stChargeStep.u32EnergyEndCharge;
            stChargeStep.u32TotalChargeEnergy = iTempTotalChargeEnergy;//总充放电电能
            LogOut("电表超过最大值,循环从头开始!", 2);
        }
    }

	//副枪订单标记为不上传
	if(stTerminalStatus.gunType == 2){
        stChargeStep.cOrderSync = 255;
	}

    if(stTerminalStatus.validFlag != ALL_VALID_NUMBER){//保存电量时检测数据可信性
        if(DataTrustableFlagMap.contains(stTerminalStatus.cCanAddr)){
            DataTrustableFlagMap.insert(stTerminalStatus.cCanAddr, false);
        }
        else{
            DataTrustableFlagMap[stTerminalStatus.cCanAddr] = false;
        }

        LogOut(QString("结束计量数据无效 CAN=%1!").arg(stChargeStep.ucCanAddr), 2);
    }
    else{//数据无效时不去更新"电度数统计表"
        //if(!stChargeStep.bChargeFreezeEnery)
        if(!bChargeFreezeEnery.contains(stChargeStep.ucCanAddr)  || (bChargeFreezeEnery.contains(stChargeStep.ucCanAddr) && bChargeFreezeEnery[stChargeStep.ucCanAddr] ==false))
        {
            bStopFlag = true;
            ChargeFreezeEneryCheck(bStopFlag,stChargeStep);
        }

        iStartOrStopFlag = 2;
        SaveChargeEnergyToDB(stChargeStep, stTerminalStatus,iStartOrStopFlag); //更新"电度数统计表"
		UpdateEndBill(stChargeStep);
    }	

    //更新"充电结算数据表"
    return true;
}

//----------------------------------------------2Server private----------------------------------------------//

bool ChargeService::PackageAckResult2Server(CHARGE_STEP &stChargeStep, __u8 ret, InfoMap &qInfoMap, InfoAddrType &InfoType)
{
    LogOut(QString("打包响应结果! CAN = %1!").arg(stChargeStep.ucCanAddr) ,2);
    QByteArray qTempByteArray;

    InfoType = AddrType_CmdCtrl_AckResult;

    qInfoMap.insert(Addr_CanID_Comm,  qTempByteArray.append(stChargeStep.ucCanAddr));//CAN地址

    qTempByteArray.clear();
    qInfoMap.insert(Addr_ChargeCmd_Ctrl, qTempByteArray.append(stChargeStep.ucCmdValue)); //指令

    qTempByteArray.clear();
    qInfoMap.insert(Addr_OrderNumber_Ctrl, qTempByteArray.append(QByteArray(stChargeStep.sEventNo, LENGTH_EVENT_NO)));//流水号

    //订单号 南京3.0新协议
    qInfoMap.insert(Addr_Bill_Code, QByteArray().append(stChargeStep.sBillCode, strlen(stChargeStep.sBillCode)));

    qTempByteArray.clear();
    qInfoMap.insert(Addr_AckResult_Ctrl, qTempByteArray.append(ret));//返回值

    return true;
}


bool ChargeService::PackageExecResult2Server(CHARGE_STEP &stChargeStep, __u8 ret, InfoMap &qInfoMap, InfoAddrType &InfoType)
{
    LogOut(QString("打包执行结果! CAN = %1!").arg(stChargeStep.ucCanAddr), 2);
    QByteArray qTempByteArray;

    InfoType = AddrType_CmdCtrl_ExecResult;

    qInfoMap.insert(Addr_CanID_Comm,  qTempByteArray.append(stChargeStep.ucCanAddr));//CAN地址

    qTempByteArray.clear();
    qInfoMap.insert(Addr_ChargeCmd_Ctrl, qTempByteArray.append(stChargeStep.ucCmdValue)); //指令

    qTempByteArray.clear();
    qInfoMap.insert(Addr_OrderNumber_Ctrl, qTempByteArray.append(QByteArray(stChargeStep.sEventNo, LENGTH_EVENT_NO)));//流水号

    //订单号 南京3.0新协议
    qInfoMap.insert(Addr_Bill_Code, QByteArray().append(stChargeStep.sBillCode, strlen(stChargeStep.sBillCode)));

    qTempByteArray.clear();
    qInfoMap.insert(Addr_ExecResult_Ctrl, qTempByteArray.append(ret));//返回值

    return true;
}

bool ChargeService::SaveChargeEnergyToDB(CHARGE_STEP &stChargeStep, TerminalStatus &stTerminalStatus,int iStartOrStopFlag)
{
    uint u32NowEnegy = 0;
    __u8 ucChargeType = 0;
    QDateTime dtChargeTime;
    QString currentDateTimeStr;

    switch (stTerminalStatus.en_ChargeStatusChangeType) {
    case CHARGE_STATUS_CHANGE_START_CHARGE:
    case CHARGE_STATUS_CHANGE_STOP_CHARGE:
    case CHARGE_STATUS_CHANGE_START_PAUSE_CHARGE:
        u32NowEnegy = stTerminalStatus.stFrameRemoteMeSurement2.active_electric_energy;
        ucChargeType = 1;
        break;
    case CHARGE_STATUS_CHANGE_START_DISCHARGE:
    case CHARGE_STATUS_CHANGE_STOP_DISCHARGE:
        u32NowEnegy = stTerminalStatus.stFrameRemoteMeSurement2.ReverseActiveEnergy;
        ucChargeType = 2;
        break;
    case CHARGE_STATUS_CHANGE_PAUSE_CHARGE:
        u32NowEnegy = stChargeStep.u32EnergyPausetCharge;
        ucChargeType = 1;
        break;
    default:
        u32NowEnegy = stTerminalStatus.stFrameRemoteMeSurement2.active_electric_energy;
        ucChargeType = 1;
        break;
    }

    if(iStartOrStopFlag == 1)//开始时间
    {
        CharArray2QDateTime(stChargeStep.sStartTime, dtChargeTime);
        currentDateTimeStr = dtChargeTime.toString("yyyy-MM-dd hh:mm:ss");
    }
    else if(iStartOrStopFlag == 2)//结束时间
    {
        CharArray2QDateTime(stChargeStep.sEndTime, dtChargeTime);
        currentDateTimeStr = dtChargeTime.toString("yyyy-MM-dd hh:mm:ss");
    }
    else//如果不是开始或者结束时记录电量,则抹秒
    {
        //抹去秒,变成00秒
        currentDateTimeStr = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        currentDateTimeStr.replace(17,2,"00");
    }

    QString qstrChargeEnergyTableName = "charge_energy_"+QString::number(stChargeStep.ucCanAddr, 10) + "_table";
    QString todo = QString("INSERT INTO '%1' (UUIDOwn, NowTime, NowEnergy, ChargeType, TimeRank, RealTime) values ('%2', '%3', %4, %5, %6, '%7')")\
            .arg(qstrChargeEnergyTableName)\
            .arg(QByteArray::fromRawData(stChargeStep.sOrderUUID, LENGTH_GUID_NO).toHex().data())\
            .arg(currentDateTimeStr)\
            .arg(u32NowEnegy)\
            .arg(ucChargeType)\
            .arg(1)\
            .arg(QString(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")));
    if(pDBOperate->DBSqlExec(todo.toAscii().data(), DB_REAL_RECORD) != 0){
        LogOut(QString("Insert table %1 false!").arg(qstrChargeEnergyTableName), 3);
        return false;
    }

    return true;
}

//------------------------------------------------------------车牌号远程充电相关---------------------------------------------------//
bool ChargeService::ProcCarLisenceChargeSub(InfoMap &qInfoMap, InfoAddrType &InfoType)
{
    switch (InfoType) {
    case AddrType_CarLicence://主题十：车牌号, 信息体：CAN地址、车牌号, 发出方：实时数据处理模块, 订阅方：充电服务
        LogOut(QString("收到总线-车牌号!"), 1);
        if(ProcRecvCarLisenceSub(qInfoMap) == false){
            return false;
        }
        break;
    case AddrType_CarLicenceApplyStartCharge_Result://主题七：车牌号外部申请开始充电结果, 信息体：CAN地址、车牌号、车牌号申请开始充电结果, 发出方：服务器, 订阅方：充电服务
        LogOut(QString("收到总线-车牌号外部申请开始充电-结果!"), 1);
        if(ProcRecvOutCarLisenceApplyStartChargeResult(qInfoMap) == false){
            return false;
        }
        break;
    default:
        break;
    }

    return true;
}


bool ChargeService::ProcRecvCarLisenceSub(InfoMap &qInfoMap)
{
    LogOut(QString("处理-车牌号主题!"), 1);

    CHARGE_STEP stTempChargeStep = stEmptyChargeStep;//存放解析出的数据
    CHARGE_STEP stChargeStep = stEmptyChargeStep;//存放缓存中的数据

    if(qInfoMap.contains(Addr_CanID_Comm)){//CAN地址
        stTempChargeStep.ucCanAddr = qInfoMap[Addr_CanID_Comm].at(0);
        LogOut(QString("CAN = %1!").arg(stTempChargeStep.ucCanAddr), 1);
    }
    else{//必备项,无则返回失败
        return false;
    }

    //解析车牌号
    if(qInfoMap.contains(Addr_CarLicense)){//车牌号号
        memcpy(stTempChargeStep.sCarLisence, qInfoMap[Addr_CarLicense].data(), qInfoMap[Addr_CarLicense].size());
        QString CarLisenceName = "";

        if(QueryCarLisenceName(stTempChargeStep.sCarLisence, CarLisenceName) == false){
            CarLisenceName = "未知";
            memset(stTempChargeStep.sCarLisence, 0, LENGTH_CAR_LISENCE);

            LogOut(QString("接收车牌号:%1 CAN = %2!").arg(CarLisenceName).arg(stTempChargeStep.ucCanAddr),2);
            return false;
        }else{
            if(CarLisenceName.length() <= LENGTH_CAR_LISENCE){
                memcpy(stTempChargeStep.sCarLisence, CarLisenceName.toAscii().data(), CarLisenceName.toAscii().length());
                LogOut(QString("接收车牌号:%1 CAN = %2!").arg(CarLisenceName).arg(stTempChargeStep.ucCanAddr),2);
            }else{
                memcpy(stTempChargeStep.sCarLisence, CarLisenceName.toAscii().data(), LENGTH_CAR_LISENCE);
                LogOut(QString("接收车牌号:%1 CAN = %2!").arg(CarLisenceName).arg(stTempChargeStep.ucCanAddr),2);
                return false;
            }
        }
    }
    else{//必备项,无则返回失败
        return false;
    }

    if(ParseCarLisenceChargeSub(qInfoMap, stTempChargeStep) == -1){
        LogOut(QString("解析车牌号相关主题有错误,返回!"), 3);
        return false;
    }
    if(CheckCanAddrValid(stTempChargeStep.ucCanAddr) == false){
        return false;
    }

    if(gpDevCache->QueryChargeStep(stTempChargeStep.ucCanAddr, stChargeStep) == false){
        stChargeStep.ucCanAddr = stTempChargeStep.ucCanAddr;
        stChargeStep.cChargeWay = PLATE_START;//车牌号启动
        memcpy(stChargeStep.sCarLisence, stTempChargeStep.sCarLisence, LENGTH_CAR_LISENCE);
        if(gpDevCache->AddChargeStep(stTempChargeStep.ucCanAddr, stChargeStep) == false){
            LogOut(QString("处理-车牌号主题 add chargestep fail!"), 3);
            return false;
        }
    }
    else{
        stChargeConfig charge;
        memset(&charge, 0, sizeof(stChargeConfig));
        gpParamSet->querySetting(&charge, PARAM_CHARGE);

        stChargeStep.cChargeWay = PLATE_START;//车牌号启动

        memcpy(stChargeStep.sCarLisence, stTempChargeStep.sCarLisence, LENGTH_CAR_LISENCE);

        //VIN触发充电关闭时不进行校验
        if(charge.vinAuto && CheckVinApplyStartChargeValid(stChargeStep) == false){
            LogOut(QString("处理-车牌号主题-校验失败!"), 3);
            return false;
        }
    }


    if(SendOutCarLisenceApplyStartCharge(stChargeStep) == false){
        return false;
    }

    if(UpdateChargeStepBySendOutCarLisenceApplyStartCharge(stChargeStep) == false){
        return false;
    }

    //数据持久化
    SaveChargeStep(stChargeStep);
    return true;
}


int ChargeService::ParseCarLisenceChargeSub(InfoMap &qInfoMap, CHARGE_STEP &stTempChargeStep)
{
    int ApplyChargeResult = 0;

//    if(qInfoMap.contains(Addr_CanID_Comm)){//CAN地址
//        stTempChargeStep.ucCanAddr = qInfoMap[Addr_CanID_Comm].at(0);
//        LogOut(QString("CAN = %1!").arg(stTempChargeStep.ucCanAddr), 1);
//    }
//    else{//必备项,无则返回失败
//        return -1;
//    }

//    if(qInfoMap.contains(Addr_CarLicense)){//车牌号号
//        memcpy(stTempChargeStep.sCarLisence, qInfoMap[Addr_CarLicense].data(), qInfoMap[Addr_CarLicense].size());
//        QString CarLisenceName = "";
//        for(int i =0;i < LENGTH_CAR_LISENCE;i ++)
//        {
//        }

//        if(QueryCarLisenceName(stTempChargeStep.sCarLisence, CarLisenceName) == false){
//            CarLisenceName = "未知";
//			memset(stTempChargeStep.sCarLisence, 0, LENGTH_CAR_LISENCE);

//        	LogOut(QString("接收车牌号:%1 CAN = %2!").arg(CarLisenceName).arg(stTempChargeStep.ucCanAddr),2);
//			return -1;
//        }else{
//			if(CarLisenceName.length() <= LENGTH_CAR_LISENCE){
//                memcpy(stTempChargeStep.sCarLisence, CarLisenceName.toAscii().data(), CarLisenceName.toAscii().length());
//                LogOut(QString("接收车牌号:%1 CAN = %2!").arg(CarLisenceName).arg(stTempChargeStep.ucCanAddr),2);
//			}else{
//				memcpy(stTempChargeStep.sCarLisence, CarLisenceName.toAscii().data(), LENGTH_CAR_LISENCE);
//        		LogOut(QString("接收车牌号:%1 CAN = %2!").arg(CarLisenceName).arg(stTempChargeStep.ucCanAddr),2);
//				return -1;
//			}
//		}
//    }
//    else{//必备项,无则返回失败
//        return -1;
//    }

    if(qInfoMap.contains(Addr_CarLicenseApplyStartChargeType_Result)){//车牌号 申请开始充电结果
        ApplyChargeResult = qInfoMap[Addr_CarLicenseApplyStartChargeType_Result].at(0);
        LogOut(QString("车牌号-申请开始充电结果 = %1!").arg(ApplyChargeResult), 2);
        return ApplyChargeResult;
    }
    if(qInfoMap.contains(Addr_CarLicenseApplyStopChargeType_Result)){//车牌号 申请结束充电结果
        ApplyChargeResult = qInfoMap[Addr_CarLicenseApplyStopChargeType_Result].at(0);
        LogOut(QString("车牌号-申请结束充电结果 = %1!").arg(ApplyChargeResult), 2);
        return ApplyChargeResult;
    }

    return 0;
}


bool ChargeService::SendOutCarLisenceApplyStartCharge(CHARGE_STEP &stChargeStep)
{
    LogOut(QString("发送-车牌号外部申请充电主题!"), 1);
    InfoMap qInfoMap;
    InfoAddrType InfoType;

    if(PackageOutCarLisenceApplyStartCharge(qInfoMap, InfoType, stChargeStep)){
        emit sigToBus(qInfoMap, InfoType);
    }
    else{
        return false;
    }

    return true;
}


bool ChargeService::PackageOutCarLisenceApplyStartCharge(InfoMap &qInfoMap, InfoAddrType &InfoType, CHARGE_STEP &stChargeStep)
{
    LogOut(QString("PackageOutCarLisenceApplyStartCharge!"), 1);
    QByteArray qTempByteArray;
    stChargeConfig charge;

    memset(&charge, 0, sizeof(stChargeConfig));
    gpParamSet->querySetting(&charge, PARAM_CHARGE);

    InfoType = AddrType_CarLicenceApplyStartCharge; //主题六：车牌号外部申请开始充电

    qTempByteArray.append(stChargeStep.ucCanAddr);
    qInfoMap.insert(Addr_CanID_Comm, qTempByteArray);//CAN地址

    qTempByteArray.clear();
    qTempByteArray = QByteArray(stChargeStep.sCarLisence, LENGTH_CAR_LISENCE);
    qInfoMap.insert(Addr_CarLicense, qTempByteArray);//车牌号

    qTempByteArray.clear();
    LogOut(QString("VIN自动申请充电选项:%1, CAN = %2 !").arg(charge.vinAuto).arg(stChargeStep.ucCanAddr), 1);
    if(charge.vinAuto == 1){
        qTempByteArray.append(1);//车牌号申请开始充电类型,默认1,充满为止
    }
    else if(charge.vinAuto == 0){
        qTempByteArray.append(3);//车牌号申请开始充电类型,03不申请充电，只发送数据。
    }

    qInfoMap.insert(Addr_CarLicenseApplyStartChargeType, qTempByteArray);

    return true;
}


bool ChargeService::UpdateChargeStepBySendOutCarLisenceApplyStartCharge(CHARGE_STEP &stChargeStep)
{
    stChargeStep.ucCmdSrc = CHARGE_CMD_SRC_CAN_DEV;//102 充电设备
    stChargeStep.ucStartReason = START_CHARGE_CMD_RSN_DEV_CAR_LICENCE_REMOTE;//110 车牌号（远程）
    //    stChargeStep.enChargeStep = CHARGE_STEP_WAITSEVER_ACK_APPLY_START_CHARGE_VIN;
    //    stChargeStep.stChargeStepValue.uc_charge_step_timeout = CHARGE_STEP_NORMAL;
    //    stChargeStep.stChargeStepValue.uc_hold_time = g_TimeoutChargeStep[stChargeStep.enChargeStep];

    if(gpDevCache->UpateChargeStep(stChargeStep.ucCanAddr, stChargeStep) == false){
        return false;
    }

    return true;
}


bool ChargeService::ProcRecvOutCarLisenceApplyStartChargeResult(InfoMap &qInfoMap)
{
    CHARGE_STEP stTempChargeStep = stEmptyChargeStep;//存放解析出的数据
    CHARGE_STEP stChargeStep = stEmptyChargeStep;//存放缓存中的数据
    QString CarLisenceName = "";
    int ApplyChargeResult = 0;

    if(qInfoMap.contains(Addr_CanID_Comm)){//CAN地址
        stTempChargeStep.ucCanAddr = qInfoMap[Addr_CanID_Comm].at(0);
        LogOut(QString("CAN = %1!").arg(stTempChargeStep.ucCanAddr), 1);
    }
    else{//必备项,无则返回失败
        return false;
    }

    if(qInfoMap.contains(Addr_CarLicense)){//车牌号号
        CarLisenceName = qInfoMap[Addr_CarLicense].data();
       LogOut(QString("接收车牌号:%1 CAN = %2!").arg(CarLisenceName).arg(stTempChargeStep.ucCanAddr),2);
    }
    else{
       ApplyChargeResult = -1;
    }
    //解析数据
    ApplyChargeResult = ParseCarLisenceChargeSub(qInfoMap, stTempChargeStep);
    if(ApplyChargeResult == -1 || ApplyChargeResult == 0){
        LogOut(QString("ProcRecvOutCarLisenceApplyStartChargeResult = %1, 无效!").arg(ApplyChargeResult), 1);
        return false;
    }
    //校验CAN地址
    if(CheckCanAddrValid(stTempChargeStep.ucCanAddr) == false){
        return false;
    }
    //逻辑处理
    if(gpDevCache->QueryChargeStep(stTempChargeStep.ucCanAddr, stChargeStep) == false){
        LogOut(QString("处理-车牌号外部申请开始充电-结果 没有找到状态机,返回!"), 3);
        return false;
    }
    else{
		//屏蔽结果中的车牌号复制，3.0协议中回应中无车牌号
		/*
        stChargeStep.cChargeWay = PLATE_START;//车牌号启动
        if(memcmp(stChargeStep.sCarLisence, stTempChargeStep.sCarLisence, LENGTH_CAR_LISENCE) != 0){
            LogOut(QString("处理-车牌号外部申请开始充电-结果 前后车牌号不一致,以后面为准!"), 3);
            memcpy(stChargeStep.sCarLisence, stTempChargeStep.sCarLisence, LENGTH_CAR_LISENCE);
        }
		*/
    }

    if(stChargeStep.ucStartReason == START_CHARGE_CMD_RSN_SMART_CHARGE_CAR_LICENCE){
        //        SendInVinApplyStartChargeResult(stChargeStep, ApplyChargeResult);
    }
    //    if(UpdateChargeStepByRecvOutVinApplyStartChargeResult(stChargeStep, ApplyChargeResult) == false){
    //        return false;
    //    }

    //数据持久化
    SaveChargeStep(stChargeStep);
    return true;
}


//----------------------------------------------充电排队相关----------------------------------------------//
bool ChargeService::ProcChargeQueueSub(InfoMap &qInfoMap)
{
    LogOut(QString("收到总线-排队信息!"), 1);
    CHARGE_STEP stQuery = stEmptyChargeStep;
    CHARGE_STEP step = stEmptyChargeStep;
    QByteArray ar;
    uchar cCanAddr, cIndex;

    if(!qInfoMap.contains(Addr_QueueInfo_Publish))
        return false;

    ar = qInfoMap[Addr_QueueInfo_Publish];

    for(int i = 0; i < ar.count(); i = i + 2){
        cCanAddr = ar.at(i);
        cIndex = ar.at(i + 1);
        step.ucCanAddr = cCanAddr;

        if(!gpDevCache->QueryChargeStep(step.ucCanAddr, step)){
            if(!gpDevCache->AddChargeStep(step.ucCanAddr, step)){
                return false;
            }
        }

        LogOut(QString("Receive Queue Info CAN=%1 QueueIndex=%2").arg(cCanAddr).arg(cIndex), 2);
        step.ucQueueMsgFromServer = cIndex;

        struct db_result_st result;
        QString strSql;

        strSql.sprintf("SELECT group_id FROM queue_group_table WHERE gun1=%d OR gun2=%d OR gun3=%d OR gun4=%d OR gun5=%d;",
                       cCanAddr, cCanAddr, cCanAddr, cCanAddr, cCanAddr);
        if(pDBOperate->DBSqlQuery(strSql.toAscii().data(), &result, DB_PARAM) == 0){
            if(result.row > 0){
                if(step.ucQueueMsgFromServer > 0 || step.enOrderStatus == ORDER_STATUS_ING){
                    step.cQueueGroup = atoi(result.result[0]);
                    step.cChargeType = ALTERNATE_CHARGE;
                    if(step.enOrderStatus != ORDER_STATUS_ING)
                        step.enOrderStatus = ORDER_STATUS_QUEUE;
                    LogOut(QString("Update Queue CAN=%1 QueueIndex=%2 QueueGroup=%3")
                           .arg((int)step.ucCanAddr).arg((int)step.ucQueueMsgFromServer).arg((int)step.cQueueGroup), 2);
                }
            }
            pDBOperate->DBQueryFree(&result);
        }

        gpDevCache->UpateChargeStep(step.ucCanAddr, step);
        gpDevCache->SaveChargeStep(step.ucCanAddr);

        if(step.ucQueueMsgFromServer > 0){
            TerminalStatus & stTempTerminalStatus = gpDevCache->GetUpdateTerminalStatus(step.ucCanAddr);
            stTempTerminalStatus.cStatus = CHARGE_STATUS_QUEUE1 + (step.ucQueueMsgFromServer - 1);
            gpDevCache->FreeUpdateTerminalStatus();
            gpDevCache->SaveTerminalStatus(step.ucCanAddr);
        }
    }

    return true;
}


bool ChargeService::ParseChargeQueueSub(InfoMap &qInfoMap, CHARGE_STEP &stTempChargeStep)
{

    if(qInfoMap.contains(Addr_CanID_Comm)){//CAN地址
        stTempChargeStep.ucCanAddr = qInfoMap[Addr_CanID_Comm].at(0);
        LogOut(QString("CAN = %1!").arg(stTempChargeStep.ucCanAddr), 1);
    }
    else{//必备项,无则返回失败
        return false;
    }
    if(qInfoMap.contains(Addr_QueueInfo_Publish)){//排队号
        QByteArray ar;
        ar = qInfoMap[Addr_QueueInfo_Publish];
        for(int i = 0; i < ar.count(); i++)
            stTempChargeStep.ucQueueMsgFromServer = qInfoMap[Addr_QueueInfo_Publish].at(0);

        LogOut(QString("排队号 = %1!").arg(stTempChargeStep.ucQueueMsgFromServer), 1);
    }
    else{//必备项,无则返回失败
        return false;
    }

    return true;
}

//----------------------------------------------VIN后六位充电相关----------------------------------------------//
bool ChargeService::ProcVINViaChargeSub(InfoMap &qInfoMap)
{
    LogOut(QString("收到总线-VIN后六位充电信息!"), 1);
    CHARGE_STEP stTempChargeStep = stEmptyChargeStep;//存放解析出的数据

    //解析传递参数
    if(ParseVINViaChargeSub(qInfoMap, stTempChargeStep) == false){
        return false;
    }
    //校验CAN地址
    if(!CheckCanAddrValid(stTempChargeStep.ucCanAddr)){
        return false;
    }
    //校验指令有效性
    SetChargeStepByRecvVINViaApplyStartCharge(stTempChargeStep);
    CheckControlChargeCmdValid(stTempChargeStep);
    LogOut(QString("控制指令校验结果:%1!").arg((__u8) stTempChargeStep.enCmdEndReason), 1);
    if(stTempChargeStep.enCmdEndReason != CMD_END_REASON_NULL){
        LogOut(QString("非法指令, 返回失败 CAN = %1!").arg(stTempChargeStep.ucCanAddr), 2);
        SendVINViaApplyChargeResult(stTempChargeStep);
        //RecordFailOrder(stTempChargeStep);
        return false;
    }
    //更新状态机
    UpdateChargeStepBySendVINViaStartCharge(stTempChargeStep);
    //充电状态机持久化,防止掉电丢失
    SaveChargeStep(stTempChargeStep);
    //发送开始充电指令-VIN后六位申请
    SendStartChargeCMDByVINVia(stTempChargeStep);

    return true;
}


bool ChargeService::ParseVINViaChargeSub(InfoMap &qInfoMap, CHARGE_STEP &stTempChargeStep)
{
    if(qInfoMap.contains(Addr_CanID_Comm)){//CAN地址
        stTempChargeStep.ucCanAddr = qInfoMap[Addr_CanID_Comm].at(0);
        LogOut(QString("CAN = %1!").arg(stTempChargeStep.ucCanAddr), 1);
    }
    else{//必备项,无则返回失败
        return false;
    }

    if(qInfoMap.contains(Addr_BatteryVIN_BMS)){//VIN号
        memcpy(stTempChargeStep.sVIN, qInfoMap[Addr_BatteryVIN_BMS].data(), qInfoMap[Addr_BatteryVIN_BMS].size());
        LogOut(QString("接收VIN号:%1 CAN = %2!").arg(QString(qInfoMap[Addr_BatteryVIN_BMS])).arg(stTempChargeStep.ucCanAddr),2);
    }
    else{//必备项,无则返回失败
        return false;
    }

    return true;
}


void ChargeService::SetChargeStepByRecvVINViaApplyStartCharge(CHARGE_STEP &stTempChargeStep)
{
    //-------------------指令订单共用---------------------------------//
    stTempChargeStep.ucCmdSrc = CHARGE_CMD_SRC_CSCU;//101 充电系统控制器
    stTempChargeStep.ucStartReason = START_CHARGE_CMD_RSN_VIN6;//113VIN号后6位（本地）
    //-------------------指令相关---------------------------------//
    stTempChargeStep.ucCmdValue = CHARGE_CMD_TYPE_START_CHARGE_NOW;//立即充电
}


bool ChargeService::SendVINViaApplyChargeResult(CHARGE_STEP &stChargeStep)
{
    InfoMap TempqInfoMap;
    InfoAddrType TempInfoType;

    LogOut(QString("发送指令结果-VIN后6位申请开始充电!"), 2);
    switch (stChargeStep.enCmdEndReason) {
    //-------------------触发类---------------------------------//
    case CMD_END_REASON_ACK_OK://响应成功,不动作
        break;
    case CMD_END_REASON_WAITACK_WRONG://非允许ACK
        PackageResultByVINViaApplyStartCharge(stChargeStep, stChargeStep.enCmdAck, TempqInfoMap, TempInfoType);
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
    case CMD_END_REASON_EXE_OK://执行成功
        PackageResultByVINViaApplyStartCharge(stChargeStep, CMD_ACK_TYPE_SUCCESS, TempqInfoMap, TempInfoType);
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
    case CMD_END_REASON_START_CHARGE_FAIL://启动失败
        PackageResultByVINViaApplyStartCharge(stChargeStep, CMD_ACK_TYPE_WAIT_START_CHARGE_FAIL, TempqInfoMap, TempInfoType);
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
        //-------------------超时类---------------------------------//
    case CMD_END_REASON_WAITACK_TIMEOUT://接收ACK回复超时
        PackageResultByVINViaApplyStartCharge(stChargeStep, CMD_ACK_TYPE_TIMEOUT, TempqInfoMap, TempInfoType);
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
    case CMD_END_REASON_START_CHARGE_TIMEOUT://启动充电超时
        PackageResultByVINViaApplyStartCharge(stChargeStep, CMD_ACK_TYPE_WAIT_START_CHARGE_TIMEOUT, TempqInfoMap, TempInfoType);
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
    case CMD_END_REASON_STOP_CHARGE_TIMEOUT: //结束充电超时
        PackageResultByVINViaApplyStartCharge(stChargeStep, CMD_ACK_TYPE_WAIT_STOP_CHARGE_TIMEOUT, TempqInfoMap, TempInfoType);
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
    case CMD_END_REASON_START_DISCHARGE_TIMEOUT: //启动放电超时
        PackageResultByVINViaApplyStartCharge(stChargeStep, CMD_ACK_TYPE_WAIT_START_DISCHARGE_TIMEOUT, TempqInfoMap, TempInfoType);
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
    case CMD_END_REASON_STOP_DISCHARGE_TIMEOUT: //结束放电超时
        PackageResultByVINViaApplyStartCharge(stChargeStep, CMD_ACK_TYPE_WAIT_STOP_DISCHARGE_TIMEOUT, TempqInfoMap, TempInfoType);
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
        //-------------------逻辑加工类---------------------------------//
    case CMD_END_REASON_IERR://内部错误
        PackageResultByVINViaApplyStartCharge(stChargeStep, CMD_ACK_TYPE_IERR, TempqInfoMap, TempInfoType);
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
    case CMD_END_REASON_ALREADY_EXE://已经有指令执行
        PackageResultByVINViaApplyStartCharge(stChargeStep, CMD_ACK_TYPE_ALREADY_EXE, TempqInfoMap, TempInfoType);
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
    case CMD_END_REASON_ALREADY_CHARGING://已经在充电
        PackageResultByVINViaApplyStartCharge(stChargeStep, CMD_ACK_TYPE_ALREADY_CHARGING, TempqInfoMap, TempInfoType);
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
    case CMD_END_REASON_GUN_NOLINK://未插抢
        PackageResultByVINViaApplyStartCharge(stChargeStep, CMD_ACK_TYPE_GUN_NOLINK, TempqInfoMap, TempInfoType);
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
    case CMD_END_REASON_NOT_CHARGING://没有在充电
        PackageResultByVINViaApplyStartCharge(stChargeStep, CMD_ACK_TYPE_NOT_CHARGING, TempqInfoMap, TempInfoType);
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
    case CMD_END_REASON_DEV_OFFLINE://设备离线
        PackageResultByVINViaApplyStartCharge(stChargeStep, CMD_ACK_TYPE_DEV_OFFLINE, TempqInfoMap, TempInfoType);
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
    case CMD_END_REASON_DEV_FAULT://设备故障
        PackageResultByVINViaApplyStartCharge(stChargeStep, CMD_ACK_TYPE_DEV_FAULT, TempqInfoMap, TempInfoType);
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
    case CMD_END_REASON_NOW_LIMIT://设备限制充电中
        PackageResultByVINViaApplyStartCharge(stChargeStep, CMD_ACK_TYPE_NOW_LIMIT, TempqInfoMap, TempInfoType);
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
    case CMD_END_REASON_NOW_PAUSH://设备暂停充电中
        PackageResultByVINViaApplyStartCharge(stChargeStep, CMD_ACK_TYPE_NOW_PAUSH, TempqInfoMap, TempInfoType);
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
    case CMD_END_REASON_NOW_DISCHARGING://设备放电中
        PackageResultByVINViaApplyStartCharge(stChargeStep, CMD_ACK_TYPE_NOW_DISCHARGING, TempqInfoMap, TempInfoType);
        emit sigToBus( TempqInfoMap, TempInfoType);
        break;
    default:
        break;
    }
    return true;
}

bool ChargeService::PackageResultByVINViaApplyStartCharge(CHARGE_STEP &stChargeStep, __u8 ret, InfoMap &qInfoMap, InfoAddrType &InfoType)
{
    LogOut(QString("打包VIN后6位申请充电结果! CAN = %1!").arg(stChargeStep.ucCanAddr) ,1);
    QByteArray qTempByteArray;

    InfoType = AddrType_VINViaScreenApplyCharge_Result;

    qInfoMap.insert(Addr_CanID_Comm,  qTempByteArray.append(stChargeStep.ucCanAddr));//CAN地址

    qTempByteArray.clear();
    qInfoMap.insert(Addr_VINViaApplyStartCharge_Result, qTempByteArray.append(ret));//返回值

    return true;
}


bool ChargeService::UpdateChargeStepBySendVINViaStartCharge(CHARGE_STEP &stTempChargeStep)
{
    LogOut(QString("收到VIN后6位指令,更新充电状态机 CAN = %1!").arg(stTempChargeStep.ucCanAddr), 1);
    bool bHasChargeStep = false;
    CHARGE_STEP stChargeStep = stEmptyChargeStep;//已经存在的指令

    bHasChargeStep = gpDevCache->QueryChargeStep(stTempChargeStep.ucCanAddr, stChargeStep);
    //状态机中指令状态机相关变量更新
    QDateTime dtQDateTime= QDateTime::currentDateTime();
    QDateTime2CharArray(dtQDateTime, stTempChargeStep.sRecvTime);
    stTempChargeStep.enChargeStep = CHARGE_STEP_WAITCMD_ACK;
    QDateTime2CharArray(dtQDateTime, stTempChargeStep.stChargeStepValue.sStartTimeStep);
    stTempChargeStep.stChargeStepValue.uc_hold_time  = g_TimeoutChargeStep[stTempChargeStep.enChargeStep];
    stTempChargeStep.stRepeatCmd.NowCount = 0;
    stTempChargeStep.stRepeatCmd.MaxCount = MAX_CAN_CMD_REPEAT;
    switch (stTempChargeStep.ucCmdValue) {
    case CHARGE_CMD_TYPE_START_CHARGE_NOW://01H 立即充电
    case CHARGE_CMD_TYPE_START_CHARGE_ECONOMIC:// 02H 经济充电
    case CHARGE_CMD_TYPE_STOP_CHARGE://03H 终止充电
    case CHARGE_CMD_TYPE_STOP_CHARGE_URGENCY://04H 紧急停止充电
    case CHARGE_CMD_TYPE_LIMIT_CHARGE://05H 限制充电,对充电中的设备进行限功率
    case CHARGE_CMD_TYPE_PAUSH_CHARGE://06H 暂停充电
    case CHARGE_CMD_TYPE_ADJUST_CHARGE://07H 调整充电电流
    case CHARGE_CMD_TYPE_RESUME://08H 恢复充电，让暂停充电的模块继续充电
    case CHARGE_CMD_TYPE_RESET://09H 复位，让限制充电的模块取消限制
    case CHARGE_CMD_TYPE_START_DISCHARGE://0AH 开始放电
    case CHARGE_CMD_TYPE_STOP_DISCHARGE://0BH 结束放电
        stTempChargeStep.stChargeStepValue.uc_charge_step_timeout = CHARGE_STEP_NORMAL;//接收ACK超时后,转到默认状态
        break;
    default:
        stTempChargeStep.stChargeStepValue.uc_charge_step_timeout = CHARGE_STEP_NORMAL;
        break;
    }

    if(bHasChargeStep == false){//充电业务状态机中没有该终端
        //订单相关变量初始化
        switch (stTempChargeStep.ucCmdValue) {
        case CHARGE_CMD_TYPE_START_CHARGE_NOW://01H 立即充电
        case CHARGE_CMD_TYPE_START_CHARGE_ECONOMIC:// 02H 经济充电
        case CHARGE_CMD_TYPE_START_DISCHARGE://0AH 开始放电
            CreatChargeOrder(stTempChargeStep, 1);
            break;
        default:
            break;
        }
        //stTempChargeStep.enOrderStatus = ORDER_STATUS_ING;
        gpDevCache->AddChargeStep(stTempChargeStep.ucCanAddr, stTempChargeStep);
    }
    else{//若缓存队列中已经有,则进行更新
        //        CopyChargeStepOrderValue(stTempChargeStep, stChargeStep);
        //        TempOutPutChargeStep(stTempChargeStep);
        //订单相关变量更新
        switch (stTempChargeStep.ucCmdValue) {
        case CHARGE_CMD_TYPE_START_CHARGE_NOW://01H 立即充电
        case CHARGE_CMD_TYPE_START_CHARGE_ECONOMIC:// 02H 经济充电
        case CHARGE_CMD_TYPE_START_DISCHARGE://0AH 开始放电
            CreatChargeOrder(stTempChargeStep, 1);
            break;
        default:
            break;
        }
        gpDevCache->UpateChargeStep(stTempChargeStep.ucCanAddr, stTempChargeStep);
    }

    TempOutPutChargeStep(stTempChargeStep);
    return true;
}

bool ChargeService::SendStartChargeCMDByVINVia(CHARGE_STEP &stChargeStep)
{
    InfoMap TempqInfoMap;
    InfoAddrType TempInfoType;

    TempqInfoMap.clear();
    PackageTelecontrol2ChargeEquipment(stChargeStep, TempqInfoMap, TempInfoType);
    emit sigToBus(TempqInfoMap, TempInfoType);
    return true;
}

bool ChargeService::SendChargeCMDToChargeEquipment(CHARGE_STEP &stChargeStep)
{
    InfoMap TempqInfoMap;
    InfoAddrType TempInfoType;

    TempqInfoMap.clear();
    PackageTelecontrol2ChargeEquipment(stChargeStep, TempqInfoMap, TempInfoType);
    emit sigToBus(TempqInfoMap, TempInfoType);
    return true;
}

//将外部申请结束充电结果发送给显示屏
void ChargeService::SendApplyStopChargeResultToScreen(CHARGE_STEP &stChargeStep, unsigned char ucResult)
{
    InfoMap TempqInfoMap;
    QByteArray qTempByteArray;

    qTempByteArray.clear();
    qTempByteArray.append(stChargeStep.ucCanAddr);
    TempqInfoMap.insert(Addr_CanID_Comm, qTempByteArray);//CAN地址

    qTempByteArray.clear();
    qTempByteArray.append(ucResult);

    TempqInfoMap.insert(Addr_CardStopCharge_Result, qTempByteArray);//刷卡申请终止充电返回结果
    TempqInfoMap.insert(Addr_ScanCode_StopCharge_Result, qTempByteArray);//刷卡申请终止充电返回结果(扫码)

    qTempByteArray.clear();
    qTempByteArray = QByteArray::fromHex(QString(stChargeStep.sCardNo).toAscii());
    TempqInfoMap.insert(Addr_CardAccount, qTempByteArray);//刷卡卡号
    qTempByteArray.clear();
    qTempByteArray = QByteArray::fromHex(QString(stChargeStep.sScanCodeNo).toAscii());
    TempqInfoMap.insert(Addr_ScanCode_customerID, qTempByteArray);//扫码卡号()

    emit sigToBus(TempqInfoMap, AddrType_OutApplyStopChargeResult_ToScreen);
}

//----------------------------------------------点击界面按钮结束充电----------------------------------------------//
bool ChargeService::ProcScreenButtonStopChargeSub(InfoMap &qInfoMap)
{

    LogOut(QString("收到总线-点击屏幕按钮结束充电信息!"), 2);
    CHARGE_STEP stTempChargeStep = stEmptyChargeStep;//存放解析出的数据
    CHARGE_STEP stChargeStep = stEmptyChargeStep;//存放缓存中的数据

    //解析传递参数
    if(qInfoMap.contains(Addr_CanID_Comm)){//CAN地址
        stTempChargeStep.ucCanAddr = qInfoMap[Addr_CanID_Comm].at(0);
        LogOut(QString("CAN = %1!").arg(stTempChargeStep.ucCanAddr), 1);
    }
    else{//必备项,无则返回失败
        return false;
    }

    //校验CAN地址
    if(!CheckCanAddrValid(stTempChargeStep.ucCanAddr)){
        return false;
    }

    //更新状态机
    if(gpDevCache->QueryChargeStep(stTempChargeStep.ucCanAddr, stChargeStep) == false){
        LogOut(QString("处理-点击屏幕按钮结束充电信息 没有找到状态机,返回!"), 3);;
        return false;
    }

    stChargeStep.ucStopReasonCSCU = 104;//本地界面按钮
    //stChargeStep.ucStopReasonDev = 91; //集控显示屏终止2018-7-7hd
    stChargeStep.ucCmdValue = CHARGE_CMD_TYPE_STOP_CHARGE;//终止充电

    //校验结束指令
    CHARGE_STEP stCheck = stEmptyChargeStep;
    stCheck.ucCanAddr = stChargeStep.ucCanAddr;
    stCheck.ucCmdValue = stChargeStep.ucCmdValue;
    CheckControlChargeCmdValid(stCheck);
    if(stCheck.enCmdEndReason != CMD_END_REASON_NULL){
        LogOut(QString().sprintf("Terminate Button Stop CMD Invalid REASON=%d", stCheck.enCmdEndReason), 2);
        return false;
    }
    UpdateChargeStepCtrlChargeCmd(stChargeStep);//更新状态机
    stChargeStep.ucStopReasonCSCU = 104;//本地界面按钮
   // stChargeStep.ucStopReasonDev = 91; //集控显示屏终止2018-7-7hd
        if(gpDevCache->UpateChargeStep(stChargeStep.ucCanAddr, stChargeStep) == false){
            return false;
        }
    //充电状态机持久化,防止掉电丢失
    SaveChargeStep(stChargeStep);

    //发送结束充电指令
    SendControlCmdToBus(stChargeStep);

    return true;
}

bool ChargeService::ProcDoorOpenAlarmSub(InfoMap &qInfoMap)
{
    LogOut(QString("收到总线-门磁报警关机主题!"), 2);

    //解析传递参数
    if(qInfoMap.contains(Addr_MagneticSwitch_Status)){//报警详情
        if( qInfoMap[Addr_MagneticSwitch_Status].at(0)){
            LogOut(QString("收到总线-门磁报警关机开始"), 2);
            DoorOpenStopCharge = true;
        }
        else{
            LogOut(QString("收到总线-门磁报警关机结束"), 2);
            DoorOpenStopCharge = false;
        }
    }
    else{//必备项,无则返回失败
        return false;
    }
    return true;
}

void ChargeService::SendControlCmdToBus(CHARGE_STEP &stChargeStep)
{
    InfoAddrType TempInfoType;//存放主题
    InfoMap TempqInfoMap; //存放信息体
    QByteArray qTempByteArray;//存放信息体值

    TempInfoType = AddrType_CmdCtrl;
    qTempByteArray.append(stChargeStep.ucCanAddr);
    TempqInfoMap.insert(Addr_CanID_Comm, qTempByteArray);

    qTempByteArray.clear();
    qTempByteArray.append(stChargeStep.ucCmdValue);
    TempqInfoMap.insert(Addr_ChargeCmd_Ctrl, qTempByteArray);
    emit sigToBus(TempqInfoMap, TempInfoType);
    LogOut(QString("Send Charge CMD = %1 CAN = %2!").arg(stChargeStep.ucCmdValue).arg(stChargeStep.ucCanAddr), 1);
}

bool ChargeService::StopChargeByDoorOpen(CHARGE_STEP &stChargeStep)
{
    //更新状态机
    stChargeStep.ucStopReasonCSCU = 115;//门磁报警
    stChargeStep.ucCmdValue = CHARGE_CMD_TYPE_STOP_CHARGE;//终止充电
    UpdateChargeStepCtrlChargeCmd(stChargeStep);

    //充电状态机持久化,防止掉电丢失
    SaveChargeStep(stChargeStep);

    //发送结束充电指令
    SendControlCmdToBus(stChargeStep);
    LogOut(QString("Send Charge CMD = 3 门磁报警"),2);

    return true;
}
//----------------------------------------------有功电能值告警----------------------------------------------//
bool ChargeService::ProcActiveEnergyFaultSub(InfoMap &qInfoMap)
{
    LogOut(QString("收到总线-有功电能值告警!"), 3);
    CHARGE_STEP stTempChargeStep = stEmptyChargeStep;//存放解析出的数据
    CHARGE_STEP stChargeStep = stEmptyChargeStep;//存放缓存中的数据

    //解析传递参数
    if(ParseActiveEnergyFaultSub(qInfoMap, stTempChargeStep) == false){
        return false;
    }
    //校验CAN地址
    if(!CheckCanAddrValid(stTempChargeStep.ucCanAddr)){
        return false;
    }
    //操作状态机
    if(gpDevCache->QueryChargeStep(stTempChargeStep.ucCanAddr, stChargeStep) == false){
        LogOut(QString("处理-有功电能值告警 没有找到状态机,返回!"), 3);;
        return false;
    }
    stChargeStep.ucStopReasonCSCU = 114;//异常有功电能结束
   // stChargeStep.ucStopReasonDev = 93; //集控电度数异常终止2018-7-7hd
    stChargeStep.ucCmdValue = CHARGE_CMD_TYPE_STOP_CHARGE;//终止充电
    if(gpDevCache->UpateChargeStep(stChargeStep.ucCanAddr, stChargeStep) == false){
        return false;
    }
    //充电状态机持久化,防止掉电丢失
    SaveChargeStep(stChargeStep);

    //发送结束充电指令
    InfoMap TempqInfoMap;
    InfoAddrType TempInfoType;
    QByteArray qTempByteArray;

    TempInfoType = AddrType_CmdCtrl;
    qTempByteArray.append(stChargeStep.ucCanAddr);
    TempqInfoMap.insert(Addr_CanID_Comm, qTempByteArray);

    qTempByteArray.clear();
    qTempByteArray.append(stChargeStep.ucCmdValue);
    TempqInfoMap.insert(Addr_ChargeCmd_Ctrl, qTempByteArray);
    emit sigToBus(TempqInfoMap, TempInfoType);

    return true;
}


bool ChargeService::ParseActiveEnergyFaultSub(InfoMap &qInfoMap, CHARGE_STEP &stTempChargeStep)
{

    if(qInfoMap.contains(Addr_CanID_Comm)){//CAN地址
        stTempChargeStep.ucCanAddr = qInfoMap[Addr_CanID_Comm].at(0);
        LogOut(QString("CAN = %1!").arg(stTempChargeStep.ucCanAddr), 1);
    }
    else{//必备项,无则返回失败
        return false;
    }
    if(qInfoMap.contains(Addr_AbnormalActiveEnergy)){//异常电度数告警
		uint uiEnergy = *(uint*)qInfoMap[Addr_AbnormalActiveEnergy].data();
        LogOut(QString("异常电度数:%1 CAN = %2!").arg(QString::number((double)uiEnergy / 100.0, 'f', 2)).arg(stTempChargeStep.ucCanAddr), 2);
    }
    else{//必备项,无则返回失败
        return false;
    }

    return true;
}

/*
 * 主动防御告警
*/
bool ChargeService::ProcActiveDefendSub(InfoMap &qInfoMap)
{
	LogOut(QString("收到总线-主动防御告警!"), 3);
    CHARGE_STEP stTempChargeStep = stEmptyChargeStep;//存放解析出的数据
    CHARGE_STEP stChargeStep = stEmptyChargeStep;//存放缓存中的数据

    if(!qInfoMap.contains(Addr_CanID_Comm)){//CAN地址
		return false;
    }
		
    stTempChargeStep.ucCanAddr = qInfoMap[Addr_CanID_Comm].at(0);
    LogOut(QString("CAN = %1!").arg(stTempChargeStep.ucCanAddr), 1);

    //校验CAN地址
    if(!CheckCanAddrValid(stTempChargeStep.ucCanAddr)){
        return false;
    }
    //操作状态机
    if(gpDevCache->QueryChargeStep(stTempChargeStep.ucCanAddr, stChargeStep) == false){
        LogOut(QString("处理-有功电能值告警 没有找到状态机,返回!"), 3);;
        return false;
    }
    stChargeStep.ucStopReasonCSCU = 116;//主动防御结束
    stChargeStep.ucCmdValue = CHARGE_CMD_TYPE_STOP_CHARGE;//终止充电
    if(gpDevCache->UpateChargeStep(stChargeStep.ucCanAddr, stChargeStep) == false){
        return false;
    }
    //充电状态机持久化,防止掉电丢失
    SaveChargeStep(stChargeStep);

    //发送结束充电指令
    InfoMap TempqInfoMap;
    InfoAddrType TempInfoType;
    QByteArray qTempByteArray;

    TempInfoType = AddrType_CmdCtrl;
    qTempByteArray.append(stChargeStep.ucCanAddr);
    TempqInfoMap.insert(Addr_CanID_Comm, qTempByteArray);

    qTempByteArray.clear();
    qTempByteArray.append(stChargeStep.ucCmdValue);
    TempqInfoMap.insert(Addr_ChargeCmd_Ctrl, qTempByteArray);
    emit sigToBus(TempqInfoMap, TempInfoType);

	return true;
}

/*
 * 根据平台下发的排队信息更新排队状态，排队信息携带流水号
 */
bool ChargeService::UpdateQueueState(InfoMap &qInfoMap)
{
    CHARGE_STEP stQuery = stEmptyChargeStep;
    CHARGE_STEP step = stEmptyChargeStep;
    QByteArray arGroup;
    uchar cCanAddr, cQueue;
    struct db_result_st result;
    QString strSql;

    if(!qInfoMap.contains(Addr_Emergency_Queue))
        return false;

    arGroup = qInfoMap[Addr_Emergency_Queue];

    for(int i = 0; i < arGroup.count(); i = i + 11){
        cCanAddr = arGroup.at(i);
        cQueue = arGroup.at(i + 1);

        //CAN地址
        step.ucCanAddr = cCanAddr;

        //查询不到且创建失败不再处理
        if(!gpDevCache->QueryChargeStep(step.ucCanAddr, step) &&
                !gpDevCache->AddChargeStep(step.ucCanAddr, step))
            return false;

        //排队号
        step.ucQueueMsgFromServer = cQueue;
        //流水号
        memcpy(step.sEventNo, arGroup.mid(i + 2, LENGTH_EVENT_NO).data(), LENGTH_EVENT_NO);

        LogOut(QString("Receive Queue Info CAN=%1 QueueIndex=%2 EventNo=%3")
               .arg((int)cCanAddr).arg((int)cQueue).arg(QByteArray(step.sEventNo, LENGTH_EVENT_NO).toHex().data()), 2);

        //需根据排队状态更新ChargeStep，以备在进入应急模式时能创建应急订单
        strSql.sprintf("SELECT group_id FROM queue_group_table WHERE gun1=%d OR gun2=%d OR gun3=%d OR gun4=%d OR gun5=%d;",
                       cCanAddr, cCanAddr, cCanAddr, cCanAddr, cCanAddr);
        if(pDBOperate->DBSqlQuery(strSql.toAscii().data(), &result, DB_PARAM) == 0){
            if(result.row > 0){
                //某一轮充组状态分三种：1、充电中 2、排队 3、未排队
                //只需要处理排队和充电中的枪
                if(step.ucQueueMsgFromServer > 0 || step.enOrderStatus == ORDER_STATUS_ING){
                    //轮充组编号
                    step.cQueueGroup = atoi(result.result[0]);
                    //充电方式更新为排队
                    step.cChargeType = ALTERNATE_CHARGE;
                    //充电中的枪不能更改其订单状态
                    if(step.enOrderStatus != ORDER_STATUS_ING)
                        step.enOrderStatus = ORDER_STATUS_QUEUE;

                    LogOut(QString("Update Queue CAN=%1 QueueIndex=%2 QueueGroup=%3")
                           .arg((int)step.ucCanAddr).arg((int)step.ucQueueMsgFromServer).arg((int)step.cQueueGroup), 2);
                }
            }
            pDBOperate->DBQueryFree(&result);
        }

        gpDevCache->UpateChargeStep(step.ucCanAddr, step);
        gpDevCache->SaveChargeStep(step.ucCanAddr);

        //更新实时状态
        if(step.ucQueueMsgFromServer > 0){
            TerminalStatus & stTempTerminalStatus = gpDevCache->GetUpdateTerminalStatus(step.ucCanAddr);
            stTempTerminalStatus.cStatus = CHARGE_STATUS_QUEUE1 + (step.ucQueueMsgFromServer - 1);
            gpDevCache->FreeUpdateTerminalStatus();
            gpDevCache->SaveTerminalStatus(step.ucCanAddr);
        }
    }
    return true;
}

//双枪模式返回结果和厦门冻结电量返回结果共用一个主题，这里根据信息″进行区分
void ChargeService::ProcResponseResult(InfoMap &qInfoMap)
{
    if(qInfoMap.contains(Addr_DCChargeManner_Term) || qInfoMap.contains(Addr_ChargeGunType) )
    {
        ProcChargeMannerResponseResult(qInfoMap);
    }
}


bool ChargeService::Getm_bServiceOffline()
{
    return false;
}
bool ChargeService::Getm_bNetOffline()
{
    return false;
}

void ChargeService::sendOrderStatus(CHARGE_STEP &step, int iOrderState)
{
    InfoMap map;

	if(DataTrustableFlagMap.contains(step.ucCanAddr)){
		return;
	}

    map.insert(Addr_CanID_Comm, QByteArray(1, step.ucCanAddr));
    map.insert(Addr_Bill_Code, QByteArray().append(step.sBillCode, strlen(step.sBillCode)));
    map.insert(Addr_ChargeOrder_State, QByteArray(1, iOrderState));
    map.insert(Addr_Order_GUID, QByteArray().append(step.sOrderUUID, LENGTH_GUID_NO).toHex());

    emit sigToBus(map, AddrType_ChargeOrder_State);

	QString str = iOrderState == 1 ? "开始" : "结束";
    LogOut(QString("%1订单通知 CAN = %2").arg(str).arg(step.ucCanAddr), 2);
}

void ChargeService::clearPowerCurve(InfoMap &map)
{
    __u8 ucCanAddr = 0;
    __u8 ucIndexID = 0;//起始地址

	if(!map.contains(Addr_Clear_Power_Curve))
		return;

    for(int i = 0; i< 3 ; i++){
        switch (i) {
        case 0://单相
            ucIndexID  = 1;
            break;
        case 1://三相
            ucIndexID  = 151;
            break;
        case 2://直流
            ucIndexID  = 181;
            break;
        default:
            break;
        }

        if(ucNumTerminal[i] > 0){
            for(int j = 0; j < ucNumTerminal[i]; j++){
                ucCanAddr = ucIndexID+j;

				CHARGE_STEP step;
                if(!gpDevCache->QueryChargeStep(ucCanAddr, step))
					continue;

				if(step.iCurveState > 0){
                    //step.iCurveState = 0;    //清除功率曲线后需要再回复一下当前状态
					step.iCurveStart = 0;
					step.iCurveStop = 0;
					step.iCurveValue = 0;
					step.iCurveCnt = 0;
					gpDevCache->UpateChargeStep(step.ucCanAddr, step);
					gpDevCache->SaveChargeStep(step.ucCanAddr);

					deletePowerCurve(step);
					LogOut(QString().sprintf("CAN=%d 清理功率曲线!", step.ucCanAddr), 2);
				}
			}
		}
	}
}

void ChargeService::updatePowerCurve(InfoMap &map)
{
	CHARGE_STEP step;
	uchar canAddr;
	QString strBillCode;

	if(!map.contains(Addr_CanID_Comm))
		return;

	if(!map.contains(Addr_Bill_Code))
		return;

    if(!map.contains(Addr_Power_Curve_State))
        return;

	canAddr = map[Addr_CanID_Comm].at(0);
	strBillCode = map[Addr_Bill_Code].data();

	if(!gpDevCache->QueryChargeStep(canAddr, step))
		return;

    //hd 2018-9-11 功率曲线不判断订单号
//	if(memcmp(map[Addr_Bill_Code].data(), step.sBillCode, sizeof(step.sBillCode) != 0)){
//		LogOut(QString().sprintf("CAN=%d 订单号=%s 更新功率曲线，订单号=%s不一致!",
//					step.ucCanAddr, step.sBillCode, map[Addr_Bill_Code].data()), 2);
//		return;
//	}

	//清空功率曲线的状态，人为造成时段切换
	//不在此处查询时段，在定时器中统一处理查询不到的情况
	step.iCurveState = *((uint*)map[Addr_Power_Curve_State].data());
	step.iCurveStart = 0;
	step.iCurveStop = 0;
	step.iCurveValue = 0;
	step.iCurveCnt = 0;
    gpDevCache->UpateChargeStep(step.ucCanAddr, step);
    gpDevCache->SaveChargeStep(step.ucCanAddr);

    LogOut(QString().sprintf("CAN=%d 更新功率曲线，重置当前时段=%d-%d 限值=%lf",
                step.ucCanAddr, step.iCurveStart, step.iCurveStop, step.iCurveValue), 2);
}

bool ChargeService::findPowerCurve(CHARGE_STEP &step)
{
    QDateTime dt = QDateTime::currentDateTime();
    struct db_result_st result;
    QString strSql;
    int time, row;

    if(step.iCurveState <= 0)
        return false;

    time = dt.toString("HHmmss").toInt();
    
    strSql.sprintf("SELECT curve_type, begin_time, end_time, suggest_value FROM power_curve \
                   WHERE curve_state = %d AND can_addr = %d AND \
            ((begin_time <= end_time AND begin_time <= %d AND %d < end_time) OR \
             (begin_time > end_time AND begin_time <= %d AND %d < 240000) OR \
             (begin_time > end_time AND 0 <= %d AND %d < end_time));",
            step.iCurveState, step.ucCanAddr, time, time, time, time, time, time);
    LogOut(strSql,1);
    if(pDBOperate->DBSqlQuery(strSql.toAscii().data(), &result, DB_PROCESS_RECORD) != 0)
        return false;

    row = result.row;
    if(row > 0){
        step.cCurveType = atoi(result.result[0]);
        step.iCurveStart = atoi(result.result[1]);
        step.iCurveStop = atoi(result.result[2]);
        step.iCurveValue = atof(result.result[3]);
        LogOut(QString().sprintf("CAN=%d 功率曲线工作时段=%d-%d 限值=%lf",
                                 step.ucCanAddr, step.iCurveStart, step.iCurveStop, step.iCurveValue), 2);
    }
    pDBOperate->DBQueryFree(&result);

    return row > 0 ? true : false;
}

bool ChargeService::deletePowerCurve(CHARGE_STEP &step)
{
    QString strSql;

	strSql.sprintf("DELETE FROM power_curve WHERE can_addr = %d;", step.ucCanAddr);
    if(pDBOperate->DBSqlExec(strSql.toAscii().data(), DB_PROCESS_RECORD) != 0)
        return false;

    return true;
}

bool ChargeService::deleteChargePolicy(CHARGE_STEP &step)
{
    QString strSql;

	strSql.sprintf("DELETE FROM charge_policy WHERE can_addr = %d;", step.ucCanAddr);
    if(pDBOperate->DBSqlExec(strSql.toAscii().data(), DB_PROCESS_RECORD) != 0)
        return false;

    return true;
}

bool ChargeService::powerCurvePeriodChanged(QDateTime dt, CHARGE_STEP &step)
{
    int iTime, iStart, iStop;

    iTime = dt.toString("hhmmss").toInt();
    iStart = step.iCurveStart;
    iStop = step.iCurveStop;

	if(iStart <= iStop){
		if(iTime >= iStart && iTime < iStop)
			return false;
	}else{
		if(iTime >= iStart && iTime < 240000)	
			return false;
		else if(iTime >= 0 && iTime < iStop)
			return false;
		else 
			return true;
	}

    return true;
}

//功率曲线限制为0时下发暂停充电
//void ChargeService::execPowerCurve(CHARGE_STEP &step)
//{
//	TerminalStatus status;
//	InfoMap map;
//	QDateTime dt;

//	//无功率曲线不执行
//	if(step.iCurveState <= 0)
//		return;

//	if(gpDevCache->QueryTerminalStatus(step.ucCanAddr, status) == 0)
//		return;

//	//充电中或暂停状态下执行功率曲线检查
//	switch(status.cStatus){
//		case CHARGE_STATUS_DEVPAUSH:
//		case CHARGE_STATUS_CSCUPAUSH:
//		case CHARGE_STATUS_CHARGING:
//			break;
//		default:
//			return;
//	}

//	dt = QDateTime::currentDateTime();

//	//时段以秒检查
//	if(powerCurvePeriodChanged(dt, step)){
//        LogOut(QString().sprintf("CAN=%d 功率曲线时段切换, 当前时段=%d-%d 限值=%lf",
//					step.ucCanAddr, step.iCurveStart, step.iCurveStop, step.iCurveValue), 2);

//		//未找到时段停止充电
//		if(!findPowerCurve(step)){
//    		LogOut(QString().sprintf("CAN=%d 功率曲线未发现策略", step.ucCanAddr), 2);

//			step.iCurveCnt = 0;
//			step.iCurveState = 0;
//			float maxCurrent = 1000.0;
			
//			switch(status.cStatus){
//				case CHARGE_STATUS_CHARGING://充电中则全速充电
//					map.clear();
//					map.insert(Addr_CanID_Comm, QByteArray(1, step.ucCanAddr));
//					map.insert(Addr_AdjustCurrent_Adj, QByteArray().append((char*)&maxCurrent, sizeof(float)));
//					emit sigToBus(map, AddrType_LimitChargeCurrent);
//    				LogOut(QString().sprintf("CAN=%d 当前充电中，功率曲线调整电流=%0.2fA", step.ucCanAddr, maxCurrent), 2);
//					break;

//				case CHARGE_STATUS_DEVPAUSH://暂停则恢复充电
//				case CHARGE_STATUS_CSCUPAUSH:
//					step.ucCmdValue = CHARGE_CMD_TYPE_RESUME;
//					gpDevCache->SaveChargeStep(step.ucCanAddr);
//					UpdateChargeStepCtrlChargeCmd(step);

//					map.clear();
//					map.insert(Addr_CanID_Comm, QByteArray(1, step.ucCanAddr));
//					map.insert(Addr_ChargeCmd_Ctrl, QByteArray(1, step.ucCmdValue));
//					emit sigToBus(map, AddrType_CmdCtrl);
//                    LogOut(QString().sprintf("CAN=%d 当前暂停中，功率曲线恢复充电", step.ucCanAddr), 2);
//					break;
//				default:
//					break;
//			}

//			gpDevCache->UpateChargeStep(step.ucCanAddr, step);
//			gpDevCache->SaveChargeStep(step.ucCanAddr);
//			return;

//			/*
//			step.ucStopReasonCSCU = 117;
//			step.ucCmdValue = CHARGE_CMD_TYPE_STOP_CHARGE;
//			gpDevCache->UpateChargeStep(step.ucCanAddr, step);
//			gpDevCache->SaveChargeStep(step.ucCanAddr);
//			UpdateChargeStepCtrlChargeCmd(step);

//			map.insert(Addr_CanID_Comm, QByteArray(1, step.ucCanAddr));
//			map.insert(Addr_ChargeCmd_Ctrl, QByteArray(1, step.ucCmdValue));
//			emit sigToBus(map, AddrType_CmdCtrl);
//			return;
//			*/
//		}

//		//策略变化重置计数，尽快下发指令
//		step.iCurveCnt = 0;
//		gpDevCache->UpateChargeStep(step.ucCanAddr, step);
//		gpDevCache->SaveChargeStep(step.ucCanAddr);
//	}

//	//策略以60秒执行一次，在计数开始时先执行功率限值检查
//	if(step.iCurveCnt >= 60){
//		step.iCurveCnt = 0;
//	}
//	step.iCurveCnt++;
//	gpDevCache->UpateChargeStep(step.ucCanAddr, step);
//	if(step.iCurveCnt > 1){
//		return;
//	}
	
//	//充电中且限值为0下发暂停充电
//	if(step.iCurveValue == 0 && status.cStatus == CHARGE_STATUS_CHARGING){
//		step.ucCmdValue = CHARGE_CMD_TYPE_PAUSH_CHARGE;

//    	LogOut(QString().sprintf("CAN=%d 功率曲线暂停充电", step.ucCanAddr), 2);

//		gpDevCache->SaveChargeStep(step.ucCanAddr);
//		UpdateChargeStepCtrlChargeCmd(step);

//		map.insert(Addr_CanID_Comm, QByteArray(1, step.ucCanAddr));
//		map.insert(Addr_ChargeCmd_Ctrl, QByteArray(1, step.ucCmdValue));
//		emit sigToBus(map, AddrType_CmdCtrl);

//		return;
//	}

//	//平台暂停或集控暂停且限值大于0下发恢复充电
//	if(step.iCurveValue > 0 &&
//		(status.cStatus == CHARGE_STATUS_DEVPAUSH ||
//		 status.cStatus == CHARGE_STATUS_CSCUPAUSH)){

//    	LogOut(QString().sprintf("CAN=%d 功率曲线恢复充电", step.ucCanAddr), 2);

//		step.ucCmdValue = CHARGE_CMD_TYPE_RESUME;
//		gpDevCache->SaveChargeStep(step.ucCanAddr);
//		UpdateChargeStepCtrlChargeCmd(step);

//		map.insert(Addr_CanID_Comm, QByteArray(1, step.ucCanAddr));
//		map.insert(Addr_ChargeCmd_Ctrl, QByteArray(1, step.ucCmdValue));
//		emit sigToBus(map, AddrType_CmdCtrl);

//		return;
//	}

//	float activePower, directVoltage, directCurrent;
//	activePower = status.stFrameRemoteMeSurement1.active_power;
//	directVoltage = status.stFrameRemoteMeSurement1.voltage_of_dc;

//	if(activePower <= 0.0 || directVoltage <= 0.0)
//		return;

//	//功率变化超过15%，进行电流调整
//	if(fabs(activePower - (float)step.iCurveValue) / (float)step.iCurveValue > 0.15){
//		directCurrent = (float)(step.iCurveValue * 1000) / directVoltage;

//		map.clear();
//		map.insert(Addr_CanID_Comm, QByteArray(1, step.ucCanAddr));
//		map.insert(Addr_AdjustCurrent_Adj, QByteArray().append((char*)&directCurrent, sizeof(float)));
//		emit sigToBus(map, AddrType_LimitChargeCurrent);

//		LogOut(QString().sprintf("CAN=%d 功率曲线调整电流=%f 当前功率=%f 电压=%f ",
//					step.ucCanAddr, directCurrent, activePower, directVoltage), 2);
//	}
//}

int flag=60;
//功率曲线限制为0时，停止充电
void ChargeService::execPowerCurve(CHARGE_STEP &step)
{
    TerminalStatus status;
    InfoMap map;
    QDateTime dt;

    //无功率曲线不执行
    if(step.iCurveState <= 0)
        return;

    if(gpDevCache->QueryTerminalStatus(step.ucCanAddr, status) == 0)
        return;
    //充电中或暂停状态下执行功率曲线检查
    switch(status.cStatus){
        //case CHARGE_STATUS_DEVPAUSH:
        //case CHARGE_STATUS_CSCUPAUSH:
        case CHARGE_STATUS_CHARGING:
        case CHARGE_STATUS_SCHEDUING:
            break;
        default:
            return;
    }

    dt = QDateTime::currentDateTime();
    //时段以秒检查
    if(powerCurvePeriodChanged(dt, step)){
        LogOut(QString().sprintf("CAN=%d 功率曲线时段切换, 当前时段=%d-%d 限值=%lf",
                    step.ucCanAddr, step.iCurveStart, step.iCurveStop, step.iCurveValue), 2);

        //未找到时段停止充电
        if(!findPowerCurve(step)){
            LogOut(QString().sprintf("CAN=%d 功率曲线未发现策略", step.ucCanAddr), 2);

            step.iCurveCnt = 0;
           // step.iCurveState = 0;
            float maxCurrent = 400.0;   //下发1000只有部分版本的设备程序支持。另一部分版本支持400,暂定下发400

            switch(status.cStatus){
                case CHARGE_STATUS_CHARGING://充电中则全速充电
                    flag =60;
                    map.clear();
                    map.insert(Addr_CanID_Comm, QByteArray(1, step.ucCanAddr));
                    map.insert(Addr_AdjustCurrent_Adj, QByteArray().append((char*)&maxCurrent, sizeof(float)));
                    emit sigToBus(map, AddrType_LimitChargeCurrent);
                    step.iCurveState = 0;
                    LogOut(QString().sprintf("CAN=%d 当前充电中，功率曲线调整电流=%0.2fA", step.ucCanAddr, maxCurrent), 2);
                    break;

                //case CHARGE_STATUS_DEVPAUSH://暂停则恢复充电
                //case CHARGE_STATUS_CSCUPAUSH:
                case CHARGE_STATUS_SCHEDUING:  //调度中，VIN申请开始充电
                if(flag>59)
                {
                    flag =0;
                    //step.ucCmdValue = CHARGE_CMD_TYPE_RESUME;
//                    step.ucCmdValue = CHARGE_CMD_TYPE_START_CHARGE_NOW;
//					gpDevCache->SaveChargeStep(step.ucCanAddr);
//					UpdateChargeStepCtrlChargeCmd(step);

//					map.clear();
//					map.insert(Addr_CanID_Comm, QByteArray(1, step.ucCanAddr));
//					map.insert(Addr_ChargeCmd_Ctrl, QByteArray(1, step.ucCmdValue));
//					emit sigToBus(map, AddrType_CmdCtrl);
                    LogOut(QString().sprintf("CAN=%d 当前调度中，功率曲线恢复充电", step.ucCanAddr), 2);
                    CurveStartVinStartCharge(step);   //VIN申请开始充电
                    break;
                }
                flag++;
                default:
                    break;
            }

            gpDevCache->UpateChargeStep(step.ucCanAddr, step);
            gpDevCache->SaveChargeStep(step.ucCanAddr);
            return;

            /*
            step.ucStopReasonCSCU = 117;
            step.ucCmdValue = CHARGE_CMD_TYPE_STOP_CHARGE;
            gpDevCache->UpateChargeStep(step.ucCanAddr, step);
            gpDevCache->SaveChargeStep(step.ucCanAddr);
            UpdateChargeStepCtrlChargeCmd(step);

            map.insert(Addr_CanID_Comm, QByteArray(1, step.ucCanAddr));
            map.insert(Addr_ChargeCmd_Ctrl, QByteArray(1, step.ucCmdValue));
            emit sigToBus(map, AddrType_CmdCtrl);
            return;
            */
        }

        //策略变化重置计数，尽快下发指令
        step.iCurveCnt = 0;
        gpDevCache->UpateChargeStep(step.ucCanAddr, step);
        gpDevCache->SaveChargeStep(step.ucCanAddr);
    }

    //策略以60秒执行一次，在计数开始时先执行功率限值检查
    if(step.iCurveCnt >= 60){
        step.iCurveCnt = 0;
    }
    step.iCurveCnt++;
    gpDevCache->UpateChargeStep(step.ucCanAddr, step);
    if(step.iCurveCnt > 1){
        return;
    }

    //充电中且限值为0下发暂停充电
    if(step.iCurveValue == 0 && status.cStatus == CHARGE_STATUS_CHARGING){
        //step.ucCmdValue = CHARGE_CMD_TYPE_PAUSH_CHARGE;
        step.ucCmdValue = CHARGE_CMD_TYPE_STOP_CHARGE;
        step.ucStopReasonCSCU = 117;//功率曲线停止

        LogOut(QString().sprintf("CAN=%d 功率曲线暂停充电", step.ucCanAddr), 2);

        //gpDevCache->SaveChargeStep(step.ucCanAddr);
        UpdateChargeStepCtrlChargeCmd(step);
         gpDevCache->SaveChargeStep(step.ucCanAddr);//save after update     6-26

        map.insert(Addr_CanID_Comm, QByteArray(1, step.ucCanAddr));
        map.insert(Addr_ChargeCmd_Ctrl, QByteArray(1, step.ucCmdValue));
        emit sigToBus(map, AddrType_CmdCtrl);

        return;
    }

    //平台暂停或集控暂停且限值大于0下发恢复充电
    if(step.iCurveValue > 0 &&
//		(status.cStatus == CHARGE_STATUS_DEVPAUSH ||
//		 status.cStatus == CHARGE_STATUS_CSCUPAUSH))
        (status.cStatus == CHARGE_STATUS_SCHEDUING))
    {

        LogOut(QString().sprintf("CAN=%d 功率曲线恢复充电", step.ucCanAddr), 2);

//		step.ucCmdValue = CHARGE_CMD_TYPE_RESUME;
//		gpDevCache->SaveChargeStep(step.ucCanAddr);
//		UpdateChargeStepCtrlChargeCmd(step);

//		map.insert(Addr_CanID_Comm, QByteArray(1, step.ucCanAddr));
//		map.insert(Addr_ChargeCmd_Ctrl, QByteArray(1, step.ucCmdValue));
//		emit sigToBus(map, AddrType_CmdCtrl);
        CurveStartVinStartCharge(step);   //VIN申请开始充电

        return;
    }

    float activePower, directVoltage, directCurrent;
    activePower = status.stFrameRemoteMeSurement1.active_power;
    directVoltage = status.stFrameRemoteMeSurement1.voltage_of_dc;

    if(activePower <= 0.0 || directVoltage <= 0.0)
        return;

    //功率变化超过15%，进行电流调整
    if(fabs(activePower - (float)step.iCurveValue) / (float)step.iCurveValue > 0.15){
        directCurrent = (float)(step.iCurveValue * 1000) / directVoltage;

        map.clear();
        map.insert(Addr_CanID_Comm, QByteArray(1, step.ucCanAddr));
        map.insert(Addr_AdjustCurrent_Adj, QByteArray().append((char*)&directCurrent, sizeof(float)));
        emit sigToBus(map, AddrType_LimitChargeCurrent);

        LogOut(QString().sprintf("CAN=%d 功率曲线调整电流=%f 当前功率=%f 电压=%f ",
                    step.ucCanAddr, directCurrent, activePower, directVoltage), 2);
    }
}

void ChargeService::CurveStartVinStartCharge(CHARGE_STEP &stChargeStep)
{
    InfoMap qInfoMap;
    QByteArray tem;

    qInfoMap.insert(Addr_CanID_Comm, QByteArray(1,stChargeStep.ucCanAddr));

    tem.append((char *)stChargeStep.sVIN,LENGTH_VIN_NO);
    qInfoMap.insert(Addr_BatteryVIN_BMS,tem);
    qInfoMap.insert(Addr_VINApplyStartChargeType,QByteArray(1,1));//VIN申请开始充电类型,默认1,充满为止
    //emit sigToBus(qInfoMap,AddrType_InVinApplyStartCharge);
    emit sigToBus(qInfoMap,AddrType_VinNum);
}

bool ChargeService::ChargeFreezeEneryCheck(bool bStopFlag,CHARGE_STEP &step)
{
    bool ret = false;
    bool bValue = false;
    double fLastTempEnergy,fNowTempEnergy;
    struct db_result_st result;
    QByteArray LastRecordTime,LastRecordEnery,NowRecordEnery;
    TerminalStatus stTerminalStatus;
    if(gpDevCache->QueryTerminalStatus(step.ucCanAddr, stTerminalStatus) == 0){//终端未查到,直接返回
        LogOut(QString("ChargeFreezeEneryCheck queryterminalstatus faild can=%1").arg(step.ucCanAddr),3);
        return false;
    }
    else if(stTerminalStatus.validFlag != ALL_VALID_NUMBER){//数据仍然无效,直接返回
        LogOut(QString("ChargeFreezeEneryCheck stTerminalStatus.validFlag no can=%1 ").arg(step.ucCanAddr),3);
        return false;
    }
    if(step.enOrderStatus != ORDER_STATUS_ING)
    {
        //LogOut(QString("ChargeFreezeEneryCheck enOrderStatus=%1 can=%2").arg(step.enOrderStatus).arg(step.ucCanAddr),3);
        return false;
    }

    QString qstrChargeEnergyTableName = "charge_energy_"+QString::number(step.ucCanAddr, 10) + "_table";
    QString todo = QString("select NowEnergy, NowTime from '%1' where UUIDOwn = '%2';")\
            .arg(qstrChargeEnergyTableName)\
            .arg(QByteArray::fromRawData(step.sOrderUUID, LENGTH_GUID_NO).toHex().data());

    if(pDBOperate->DBSqlQuery(todo.toAscii().data(), &result, DB_REAL_RECORD) == 0)
    {
        if(result.row <= 0)
        {
            pDBOperate->DBQueryFree(&result);
            return false;
        }
        else
        {
            fNowTempEnergy = (double)stTerminalStatus.stFrameRemoteMeSurement2.active_electric_energy / 100.0;
            QDateTime NowTime = QDateTime::currentDateTime();

            fLastTempEnergy = (double)(atoi(result.result[(result.row-1) * result.column])) / 100.0;
            LastRecordTime = QByteArray(result.result[(result.row-1) * result.column+1], 20);
            QDateTime qLastRecordTime = QDateTime::fromString(QString(LastRecordTime), "yyyy-MM-dd hh:mm:ss");

            unsigned int uiNowTime = NowTime.toTime_t();
            unsigned int uiLastTime = qLastRecordTime.toTime_t();
            int sTempTime = 0;
            sTempTime = uiNowTime - uiLastTime;

            if(sTempTime < 0 && bStopFlag == true)//判断当前时间是否小于上一次冻结点时间
            {
                pDBOperate->DBQueryFree(&result);
                return false;
            }
            else if(sTempTime < 0)
            {
                pDBOperate->DBQueryFree(&result);
                return false;
            }

            ret = ChackFreezeEneryTime(qLastRecordTime,NowTime);

            if(!ret)
            {
                bValue = false;
                //step.bChargeFreezeEnery = true;
                bChargeFreezeEnery[step.ucCanAddr] = true;
                LogOut(QString("nbchargeferezeenergy=true  1"), 1);
                gpDevCache->UpateChargeStep(step.ucCanAddr,step);
            }
            else
            {
                if(AddFreezeEnrey(result.row ,qLastRecordTime,NowTime,fLastTempEnergy,fNowTempEnergy,step))
                {
                    bValue = true;
                    //step.bChargeFreezeEnery = true;
                    bChargeFreezeEnery[step.ucCanAddr] = true;
                    LogOut(QString("nbchargeferezeenergy=true  2"), 1);
                    gpDevCache->UpateChargeStep(step.ucCanAddr,step);
                }
                else
                {
                    bValue = false;
                    //step.bChargeFreezeEnery = false;
                    bChargeFreezeEnery[step.ucCanAddr] = false;
                    LogOut(QString("nbchargeferezeenergy=false"), 1);
                    gpDevCache->UpateChargeStep(step.ucCanAddr,step);
                }
            }
            pDBOperate->DBQueryFree(&result);
        }
    }

    return bValue;
}

bool ChargeService::AddFreezeEnrey(int row,QDateTime LastTime,QDateTime NowTime,double fLastEnery,double fNowEnery,CHARGE_STEP &step)
{
    int iFreezePoint = 0,SustainTime = 0;
    bool bSaveSuccess = false;
    unsigned int uiLastTime,uiNowTime,uiRecordTime;
    double fRecordEnery = 0.0;
    QDateTime RecordTime;
    QByteArray RecordEnery;

    uiLastTime = 0;
    uiNowTime = 0;
    SustainTime = 0;
    iFreezePoint = 0;
	uiRecordTime = 0;

    short mmBefore = LastTime.toString("mm").toInt();
    short ssBefore = LastTime.toString("ss").toInt();

    short ssNow = NowTime.toString("ss").toInt();
    //上一时刻时间和当前时间转换成秒,并且去除其二者自身的秒(时间)
    uiLastTime = LastTime.toTime_t() - ssBefore;
    uiNowTime = NowTime .toTime_t() - ssNow;

    if(row == 1)//数据库中只有一个冻结点
    {
        SustainTime = (uiNowTime - uiLastTime)/60;//计算当前和上一个记录点的持续时间(单位为分钟),并且去除开始充电时间的分钟的干扰(秒的干扰上一步已经去除)
        if(mmBefore >= 0 && mmBefore < 30)
            iFreezePoint = (SustainTime  + mmBefore)/30;
        else if(mmBefore >= 30 && mmBefore <= 59)
            iFreezePoint = (SustainTime  + mmBefore - 30)/30;

        for(int i = 0;i < iFreezePoint;i++)// 需要补的半点个数
        {
            if(mmBefore >= 0 && mmBefore < 30)
                uiRecordTime = (uiLastTime - (mmBefore * 60 )+ (30 * (i + 1) * 60));//计算应该记录的冻结点时间  最后一个点的时间-分钟数（×60秒）+30分钟（×60秒）
            else if(mmBefore >= 30 && mmBefore <= 59)
                uiRecordTime = (uiLastTime - ((mmBefore - 30) * 60) + (30 * (i + 1) * 60));//计算应该记录的冻结点时间


            RecordTime = RecordTime.fromTime_t(uiRecordTime);
            //电表超量程处理
            int iTempTotalChargeEnergy = 0;
            double dAmmeterRange = gpParamSet->getAmmeterRange(step.ucCanAddr);

            iTempTotalChargeEnergy = (int)(fNowEnery*100) - (int)(fLastEnery*100) ;

            if(iTempTotalChargeEnergy >= 0)
            {
                fRecordEnery = fLastEnery + (((double)(uiRecordTime - uiLastTime))/((double)(SustainTime * 60)) * (fNowEnery - fLastEnery));//计算应该记录的冻结点电量
            }
            else
            {
                if(((int)(fLastEnery*100) > ((int)(dAmmeterRange * 100) - 60000) ) && ((int)(fNowEnery*100)  <  60000) )
                {
                    iTempTotalChargeEnergy = (int)(dAmmeterRange * 100) - (int)(fLastEnery * 100) + (int)(fNowEnery * 100);
                    fRecordEnery = fLastEnery + (((double)(uiRecordTime - uiLastTime))/((double)(SustainTime * 60)) * ((double)(iTempTotalChargeEnergy/100.00)));//计算应该记录的冻结点电量
                }

                if(fRecordEnery > dAmmeterRange)//如果应记录的数据超量程,则减去量程
                    fRecordEnery = fRecordEnery - dAmmeterRange;

            }
            if(SaveChargeFreezeEnergyToDB(step,RecordTime,fRecordEnery))
            {
                bSaveSuccess = true;
            }
            else
                return false;
        }
    }
    else if(row > 1)
    {
        SustainTime = (uiNowTime - uiLastTime)/60;//计算当前和上一个记录点的持续时间(单位为分钟)
        iFreezePoint = SustainTime/30;//默认超过一个的冻结电，则最后一个冻结电一定是半点冻结的点


        for(int i = 0;i < iFreezePoint;i++)
        {
            uiRecordTime = (uiLastTime + 30 * (i + 1) * 60 ) ;//计算应该记录的冻结点时间
            RecordTime = RecordTime.fromTime_t(uiRecordTime);

            //电表超量程处理
            int iTempTotalChargeEnergy = 0;
            double dAmmeterRange = gpParamSet->getAmmeterRange(step.ucCanAddr);

            iTempTotalChargeEnergy = (int)(fNowEnery*100) - (int)(fLastEnery*100) ;

            if(iTempTotalChargeEnergy >= 0)
            {
                fRecordEnery = fLastEnery + (((double)(uiRecordTime - uiLastTime))/((double)(SustainTime * 60)) * (fNowEnery - fLastEnery));//计算应该记录的冻结点电量
            }
            else
            {
                if(((int)(fLastEnery*100) > ((int)(dAmmeterRange * 100) - 60000) ) && ((int)(fNowEnery*100)  <  60000) )
                {
                    iTempTotalChargeEnergy = (int)(dAmmeterRange * 100) - (int)(fLastEnery * 100) + (int)(fNowEnery * 100);
                    fRecordEnery = fLastEnery + (((double)(uiRecordTime - uiLastTime))/((double)(SustainTime * 60)) * ((double)(iTempTotalChargeEnergy/100.00)));//计算应该记录的冻结点电量
                }

                if(fRecordEnery > dAmmeterRange)//如果应记录的数据超量程,则减去量程
                    fRecordEnery = fRecordEnery - dAmmeterRange;
            }

            if(SaveChargeFreezeEnergyToDB(step,RecordTime,fRecordEnery))
            {
                bSaveSuccess = true;
            }
            else
                return false;
        }
    }

    if(bSaveSuccess == true)
        return true;
    else
        return false;
}

bool ChargeService::ChackFreezeEneryTime(QDateTime LastTime,QDateTime NowTime)
{
    short yearBefore=LastTime.toString("yy").toInt();
    short monthBefore = LastTime.toString("MM").toInt();
    short dayBefore = LastTime.toString("dd").toInt();
    short hhBefore = LastTime.toString("hh").toInt();
    short mmBefore = LastTime.toString("mm").toInt();

    short yearAfter=NowTime.toString("yy").toInt();
    short monthAfter = NowTime.toString("MM").toInt();
    short dayAfter = NowTime.toString("dd").toInt();
    short hhAfter = NowTime.toString("hh").toInt();
    short mmAfter = NowTime.toString("mm").toInt();

    if((yearBefore == yearAfter) && (monthBefore == monthAfter) &&
            (dayBefore == dayAfter) && (hhBefore == hhAfter))
    {
        if(((mmBefore >= 0 && mmBefore <= 30) && (mmAfter >= 0 && mmAfter < 30)) ||
                ((mmBefore >= 30 && mmBefore <= 59) && (mmAfter >= 30 && mmAfter <= 59)))
        {
            if(NowTime.toTime_t() - LastTime.toTime_t() < 60)
            {
                LogOut(QString("nowtime -lasttime <60"), 2);
                return false;
            }
            LogOut(QString("This is same zero point"), 2);
            return false;
        }
        else
        {
            LogOut(QString("This is not same zero point1"), 2);
            return true;
        }
    }
    else
    {
        LogOut(QString("This is not same zero point2"), 2);
        return true;
    }
}

bool ChargeService::SaveChargeFreezeEnergyToDB(CHARGE_STEP &stChargeStep,QDateTime RecordTime,double RecordEnery)
{
    __u8 ucChargeType = 0;
    TerminalStatus stTerminalStatus;

    if(!gpDevCache->QueryTerminalStatus(stChargeStep.ucCanAddr, stTerminalStatus))
        return false;

    switch (stTerminalStatus.en_ChargeStatusChangeType)
    {
    case CHARGE_STATUS_CHANGE_START_CHARGE:
    case CHARGE_STATUS_CHANGE_STOP_CHARGE:
        ucChargeType = 1;
        break;
    case CHARGE_STATUS_CHANGE_START_DISCHARGE:
    case CHARGE_STATUS_CHANGE_STOP_DISCHARGE:
        ucChargeType = 2;
        break;
    default:
        ucChargeType = 1;
        break;
    }
    QString qstrChargeEnergyTableName = "charge_energy_"+QString::number(stChargeStep.ucCanAddr, 10) + "_table";
    QString todo = QString("INSERT INTO '%1' (UUIDOwn, NowTime, NowEnergy, ChargeType, TimeRank, RealTime) values ('%2', '%3', %4, %5, %6, '%7')")\
            .arg(qstrChargeEnergyTableName)\
            .arg(QByteArray::fromRawData(stChargeStep.sOrderUUID, LENGTH_GUID_NO).toHex().data())\
            .arg(RecordTime.toString("yyyy-MM-dd hh:mm:ss"))\
            .arg((uint)(RecordEnery * 100.0))\
            .arg(ucChargeType)\
            .arg(1)\
            .arg(RecordTime.toString("yyyy-MM-dd hh:mm:ss"));

    LogOut(QString("Repair Freeze CAN = %1!").arg(stChargeStep.ucCanAddr), 2);

    if(pDBOperate->DBSqlExec(todo.toAscii().data(), DB_REAL_RECORD) != 0){
        LogOut(QString("FreezeEnergy Insert table %1 false!").arg(qstrChargeEnergyTableName), 3);
        return false;
    }
    return true;
}

bool ChargeService::CheckProChargeEnergyRecordLoop(CHARGE_STEP &step)
{
    bool ret = false;
    float fLastTempEnergy,fNowTempEnergy;
    struct db_result_st result;
    QByteArray LastRecordTime,LastRecordEnery,NowRecordEnery;
    TerminalStatus stTerminalStatus;

    if(gpDevCache->QueryTerminalStatus(step.ucCanAddr, stTerminalStatus) == 0){//终端未查到,直接返回
        return false;
    }
    else if(stTerminalStatus.validFlag != ALL_VALID_NUMBER){//数据仍然无效,直接返回
        return false;
    }
    //    else if(stTerminalStatus.stFrameRemoteSingle.charge_status != 1)
    //        return false;

    QString qstrChargeEnergyTableName = "charge_energy_"+QString::number(step.ucCanAddr, 10) + "_table";
    QString todo = QString("select NowEnergy, NowTime from '%1' where UUIDOwn = '%2';")\
            .arg(qstrChargeEnergyTableName)\
            .arg(QByteArray::fromRawData(step.sOrderUUID, LENGTH_GUID_NO).toHex().data());

    if(pDBOperate->DBSqlQuery(todo.toAscii().data(), &result, DB_REAL_RECORD) == 0)
    {
        if(result.row <= 0){
        	pDBOperate->DBQueryFree(&result);
            return false;
		}
        else
        {
            fNowTempEnergy = (double)stTerminalStatus.stFrameRemoteMeSurement2.active_electric_energy / 100.0;
            NowRecordEnery = QByteArray((char*)&(fNowTempEnergy), sizeof(float));
            QDateTime NowTime = QDateTime::currentDateTime();

            LastRecordTime = QByteArray(result.result[(result.row-1) * result.column+1], 20);
            fLastTempEnergy = atof(result.result[(result.row-1) * result.column]);
            LastRecordEnery = QByteArray((char*)&(fLastTempEnergy), sizeof(float));
            QDateTime qLastRecordTime = QDateTime::fromString(QString(LastRecordTime), "yyyy-MM-dd hh:mm:ss");
            ret = ChackFreezeEneryTime(qLastRecordTime,NowTime);
        }
        pDBOperate->DBQueryFree(&result);

        if(ret == false)
            return false;
        else
            return true;
    }
    else
        return false;
}

bool ChargeService::CheckTimeAcrossIsMoreThan30(CHARGE_STEP &step,QDateTime dtNowTime)
{
    int CountTime = 0;
    struct db_result_st result;
    QByteArray LastRecordTime;

    QString qstrChargeEnergyTableName = "charge_energy_"+QString::number(step.ucCanAddr, 10) + "_table";
    QString todo = QString("select NowEnergy, NowTime from '%1' where UUIDOwn = '%2';")\
            .arg(qstrChargeEnergyTableName)\
            .arg(QByteArray::fromRawData(step.sOrderUUID, LENGTH_GUID_NO).toHex().data());

    if(pDBOperate->DBSqlQuery(todo.toAscii().data(), &result, DB_REAL_RECORD) == 0)
    {
        LastRecordTime = QByteArray(result.result[(result.row-1) * result.column+1], 20);
        QDateTime qLastRecordTime = QDateTime::fromString(QString(LastRecordTime), "yyyy-MM-dd hh:mm:ss");

        CountTime = (dtNowTime.toTime_t() - qLastRecordTime.toTime_t()) / (60 * 30);

        if(CountTime < 0 || CountTime > 1)
        {
            pDBOperate->DBQueryFree(&result);
            return false;
        }
        else if(CountTime <= 1 && CountTime >= 0)
        {
            pDBOperate->DBQueryFree(&result);
        return true;
        }
    }
    else
    {
        pDBOperate->DBQueryFree(&result);
        return false;
    }

	return false;
}

///
/// \brief ChargeService::UpadeLogicChargeStatus 更新逻辑充电机工作状态
/// \param b_HasChargeBill 标识终端是否有充电状态机
/// \return
///
bool ChargeService::UpadeLogicChargeStatusForTerminalSingle(TerminalStatus &stTerminalStatustemp, bool b_HasChargeBill,char chargestatus)
{
    LogOut(QString("UpadeLogicChargeStatusTerminalSingle  CAN = %1!").arg(stTerminalStatustemp.cCanAddr), 1);

    unsigned char ucCanAddr =stTerminalStatustemp.cCanAddr;
    CHARGE_STEP stChargeStep = stEmptyChargeStep;//已经存在的状态机
    TerminalStatus stTerminalStatus = stEmptyTerminalStatus;

    if(gpDevCache->QueryTerminalStatus(stTerminalStatustemp.cCanAddr, stTerminalStatus) == 0){
        LogOut(QString("UpadeLogicChargeStatus  QueryTerminalStatus fail1!"), 3);
        return false;
    }
    TempOutPutTerminalStatus(stTerminalStatus);
    if(b_HasChargeBill){
        if(gpDevCache->QueryChargeStep(stTerminalStatustemp.cCanAddr, stChargeStep) == false){
            return false;
        }
        TempOutPutChargeStep(stChargeStep);
    }
   // switch (stTerminalStatustemp.stFrameRemoteSingle.charge_status)//判断充电机的实时状态
    switch(chargestatus)
    {
    case CHARGE_STATUS_REALTIME_STANDBY://当前为待机
        switch (stTerminalStatus.stFrameRemoteSingle.link_status) {
        case 1://连接
        case 2://插抢且车辆未确认
        case 3://插抢且车辆确认
            stTerminalStatus.cStatus = CHARGE_STATUS_GUN_STANDBY;

            if(b_HasChargeBill){//有订单
                if(stChargeStep.ucQueueMsgFromServer > 0){//有服务器下发的排队信息,以排队信息为准
                    stTerminalStatus.cStatus = CHARGE_STATUS_QUEUE1 + (stChargeStep.ucQueueMsgFromServer - 1);
                }
                else{//没有服务器下发的排队信息
                    if(stChargeStep.enOrderStatus == ORDER_STATUS_OK || stChargeStep.enOrderStatus == ORDER_STATUS_FAIL){//订单非待结算状态
                        if(stTerminalStatus.stFrameBmsInfo.batery_SOC != 100){
                            stTerminalStatus.cStatus = CHARGE_STATUS_FINISH;
                        }
                        else{
                            stTerminalStatus.cStatus = CHARGE_STATUS_FULL;
                        }
                        //if((stChargeStep.iCurveState >0) && ( stChargeStep.ucStopReasonCSCU == 117))////调度中   hd
                        if((stChargeStep.iCurveState >0) &&
                                (( stChargeStep.ucStopReasonCSCU == 117 )))     //待机+订单成功+有功率曲线+限值=0
                                 //|| (stChargeStep.enOrderStatus == ORDER_STATUS_FAIL)))//  待机+订单失败+有功率曲线
                        {
                             stTerminalStatus.cStatus = CHARGE_STATUS_SCHEDUING;
                        }

                        //产生订单后给screen模块发送信号，允许打印一次 add by zrx
                        InfoMap ToCenterMap;
                        QByteArray qTempByteArray;
                        qTempByteArray.append((char)ucCanAddr);
                        ToCenterMap.insert(Addr_CanID_Comm, qTempByteArray);//can地址

                        //发送
                        emit sigToBus(ToCenterMap, AddrType_ApplyPrintTicket);
                    }

                }
            }
            else{//无订单
                stTerminalStatus.cStatus = CHARGE_STATUS_GUN_STANDBY;
            }
            break;
        case 0://未连接
        case 4://半连接,modify by YCZ 2016-03-04 海汇德半连接也是未插抢
        {
            stTerminalStatus.cStatus = CHARGE_STATUS_FREE;
            if(b_HasChargeBill){//若有订单,则1结束订单,2清空状态机
                if(stChargeStep.enOrderStatus == ORDER_STATUS_ING){
                    //根据订单是否生效来判断处理方式
                    if(stChargeStep.enChargeStep == CHARGE_STEP_WAITDEV_START_CHARGE
                            || stChargeStep.enChargeStep == CHARGE_STEP_WAITDEV_START_DISCHARGE
                            || stChargeStep.enChargeStep == CHARGE_STEP_WAITCMD_ACK){//等待充电启动(订单还没有正式生效)

                        //2017-01-10 添加默认值
                        TerminalStatus stTerminalStatus = stEmptyTerminalStatus;
                        gpDevCache->QueryTerminalStatus(stChargeStep.ucCanAddr, stTerminalStatus);
                        SaveStartChargeOrderMsg(stTerminalStatus,stChargeStep);
                        SaveStopChargeOrderMsg(stTerminalStatus,stChargeStep);
                        stChargeStep.enOrderStatus = ORDER_STATUS_FAIL;
                        stChargeStep.enCmdEndReason = CMD_END_REASON_START_CHARGE_FAIL;
                        stChargeStep.ucStopReasonDev = 33;//二期增加
                        stChargeStep.ucStopReasonCSCU = 100;//用户拔枪
                        TerminateOrder(stChargeStep);
                        SendCmdResult(stChargeStep);
                    }
                    else{//订单已经正式生效
                        stChargeStep.enOrderStatus = ORDER_STATUS_OK;
                        stChargeStep.enCmdEndReason = CMD_END_REASON_EXE_OK;
                        stChargeStep.ucStopReasonDev = 1;//二期增加
                        stChargeStep.ucStopReasonCSCU = 100;//用户拔枪
                        TerminateOrder(stChargeStep);
                        SendCmdResult(stChargeStep);
                    }

                }
                else if(stChargeStep.enOrderStatus == ORDER_STATUS_OK){
                    ;
                }

                //拔枪删除终端的功率曲线
                deletePowerCurve(stChargeStep);
                //拔枪删除终端的充电策略
				deleteChargePolicy(stChargeStep);

                LogOut(QString("Delete step!"), 1);
                if(gpDevCache->DeleteChargeStep(stTerminalStatus.cCanAddr) == false){
                    LogOut(QString("Delete step fail CAN = %1!").arg(stChargeStep.ucCanAddr), 3);
                }

			}
		}
		break;
        default:
            break;
        }
        break;
    case CHARGE_STATUS_REALTIME_CHARGING://当前为充电中
        stTerminalStatus.cStatus = CHARGE_STATUS_CHARGING;
        break;
    case CHARGE_STATUS_REALTIME_FAULT://当前为故障
        stTerminalStatus.cStatus = CHARGE_STATUS_FAULT;
        break;
    case CHARGE_STATUS_REALTIME_STARTING://当前为启动中
        stTerminalStatus.cStatus = CHARGE_STATUS_STARTING;
        break;
    case CHARGE_STATUS_REALTIME_PAUSE://当前为暂停
        stTerminalStatus.cStatus = CHARGE_STATUS_PAUSH;
        break;
    case CHARGE_STATUS_REALTIME_CARPAUSE://当前为暂停--车辆暂停
        stTerminalStatus.cStatus = CHARGE_STATUS_CARPAUSH;
        break;
    case CHARGE_STATUS_REALTIME_DEVPAUSE://当前为暂停--充电设备暂停
        stTerminalStatus.cStatus = CHARGE_STATUS_DEVPAUSH;
        break;
    case CHARGE_STATUS_REALTIME_LIMIT://当前为限制
        stTerminalStatus.cStatus = CHARGE_STATUS_LIMIT;
        break;
    case CHARGE_STATUS_REALTIME_OFFLINE://当前为离线
        stTerminalStatus.cStatus = CHARGE_STATUS_DISCONNECT;
        break;
    case CHARGE_STATUS_REALTIME_SWITCH://当前为切换中
        stTerminalStatus.cStatus = CHARGE_STATUS_SWITCH;
        break;
    case CHARGE_STATUS_REALTIME_DISCHARGING://当前为放电
        stTerminalStatus.cStatus = CHARGE_STATUS_DISCHARGING;
        break;
    case CHARGE_STATUS_REALTIME_WAITING:    //当前为等待中 add by XX 2017-05-03
        stTerminalStatus.cStatus = CHARGE_STATUS_WAITING;
        break;
    default:
        break;
    }

    LogOut(QString("逻辑工作状态 = %2, CAN = %1 !").arg(ucCanAddr).arg(map_LogicStatus[stTerminalStatus.cStatus]), 2);

    //更新至缓存
    TerminalStatus & stTempTerminalStatus = gpDevCache->GetUpdateTerminalStatus(ucCanAddr);
	if(stTempTerminalStatus.cStatus != stTerminalStatus.cStatus){
        //++++南京新协议增加逻辑工作状态突发
        InfoMap map;
        map.insert(Addr_CanID_Comm, QByteArray(1, ucCanAddr));
        map.insert(Addr_LogicState_Term, QByteArray(1, stTempTerminalStatus.cStatus));
        map.insert(Addr_LogicState_Burst, QByteArray(1, stTerminalStatus.cStatus));
        emit sigToBus(map, AddrType_LogicState);
        //----
    }
    stTempTerminalStatus.cStatus = stTerminalStatus.cStatus;
    TempOutPutTerminalStatus(stTempTerminalStatus);
    gpDevCache->FreeUpdateTerminalStatus();
    return true;
}


bool ChargeService::CreateBill(CHARGE_STEP &step)
{
	QString strSql;

	strSql.sprintf("INSERT INTO charge_order (UUIDOwn, OrderStatus, CanAddr, CmdSrc, StartReason, EventNo, BillCode, CardNo, \
					customerID, VIN, CarLisence, ChargeType, ChargeWay) values ('%s', %d, %d, %d, %d, '%s', '%s', '%s', '%s', '%s', '%s', %d, %d);", 
					QByteArray(step.sOrderUUID, LENGTH_GUID_NO).toHex().data(), 
					step.enOrderStatus, step.ucCanAddr, step.ucCmdSrc, step.ucStartReason, 
                    QByteArray(step.sEventNo, LENGTH_EVENT_NO).toHex().data(),
                	step.sBillCode, step.sCardNo, step.sScanCodeNo, step.sVIN,
                    QByteArray(step.sCarLisence, strlen(step.sCarLisence)).data(),
					step.cChargeType, step.cChargeWay);
    if(pDBOperate->DBSqlExec(strSql.toAscii().data(), DB_PROCESS_RECORD) != 0){
    	LogOut(QString("创建订单失败！CAN = %1").arg(step.ucCanAddr), 2);
		return false;
	}

    LogOut(QString("创建订单 CAN = %1").arg(step.ucCanAddr), 2);
	return true;
}

bool ChargeService::UpdateBeginBill(CHARGE_STEP &step)
{
	QString strSql;

	strSql.sprintf("UPDATE charge_order SET OrderStatus = %d, StartTime = \'%s\', StartEnergy = %u, StartSoc = %d, \
				OrderType = %d, GunNum = %d, OrderSync = %d WHERE UUIDOwn = \'%s\';",
				step.enOrderStatus, step.sStartTime, step.u32EnergyStartCharge, step.ucStartSOC, 
                step.cOrderType, step.cGunNum, step.cOrderSync,
				QByteArray(step.sOrderUUID, LENGTH_GUID_NO).toHex().data());

    if(pDBOperate->DBSqlExec(strSql.toAscii().data(), DB_PROCESS_RECORD) != 0){
        LogOut(QString("更新开始订单失败! CAN = %1").arg(step.ucCanAddr), 2);
		return false;
    }

    LogOut(QString("更新开始订单 CAN = %1").arg(step.ucCanAddr), 2);
	return true;
}

bool ChargeService::UpdateEndBill(CHARGE_STEP &step)
{
	QString strSql;
	
	strSql.sprintf("UPDATE charge_order SET OrderStatus = %d, StartTIme = \'%s\', EndTime = \'%s\', TotalChargeTime = %d, \
                       StartEnergy = %u, EndEnergy = %u, TotalChargeEnergy = %u, StartSoc = %d, StopSoc = %d, \
                       CSCUStopReason = %d, CloudStopReason = %d, DevStopReason = %d, OrderType = %d, LimitEnergy = %u, \
                       GunNum = %d, QueueGroup = %d, QueueIndex = %d, OrderSync = %d WHERE UUIDOwn = \'%s\';",
                	   step.enOrderStatus, step.sStartTime, step.sEndTime, step.u12TotalChargeTime,
                       step.u32EnergyStartCharge, step.u32EnergyEndCharge, step.u32TotalChargeEnergy,
                       step.ucStartSOC, step.ucEndSOC, step.ucStopReasonCSCU, step.ucStopReasonCloud, step.ucStopReasonDev,
                       step.cOrderType, step.fLimitEnergy, step.cGunNum, step.cQueueGroup, step.ucQueueMsgFromServer, step.cOrderSync,
                       QByteArray(step.sOrderUUID, LENGTH_GUID_NO).toHex().data());

    if(pDBOperate->DBSqlExec(strSql.toAscii().data(), DB_PROCESS_RECORD) != 0){
        LogOut(QString("更新结束订单失败! CAN = %1").arg(step.ucCanAddr), 2);
		return false;
    }

    LogOut(QString("更新结束订单 CAN = %1").arg(step.ucCanAddr), 2);
	return true;
}

//设备NACK带原因，将NACK的原因填到设备终止原因中并在回复响应结果之前上传平台
void ChargeService::SaveStopResult(CHARGE_STEP &stTempChargeStep)
{
    //TerminalStatus stTerminalStatus = stEmptyTerminalStatus;
    //gpDevCache->QueryTerminalStatus(stChargeStep.ucCanAddr, stTerminalStatus);
    unsigned char ucStopReason=0;
    switch (stTempChargeStep.enCmdAck)
    {
        case CMD_ACK_TYPE_PDUFault:
            ucStopReason = 0x05;
            break;
    case CMD_ACK_TYPE_NoPowerAccept:
        ucStopReason = 0x3B;
        break;
    case CMD_ACK_TYPE_Scram:
        ucStopReason = 0x39;
        break;
    case CMD_ACK_TYPE_LinkBreak:
        ucStopReason = 0x07;
        break;
    case CMD_ACK_TYPE_LastProtectNoEnd:
        ;
        break;
    case CMD_ACK_TYPE_LaskBMSOutTime:
        ucStopReason = 0x36;
        break;
    case CMD_ACK_TYPE_ChargerInUsed:
        ucStopReason = 0x40;
        break;
     default:
            break;
    }
    if(ucStopReason !=0)
    {
        TerminalStatus & Status = gpDevCache->GetUpdateTerminalStatus(stTempChargeStep.ucCanAddr);
        Status.stFrameRemoteSingle.Stop_Result = ucStopReason;
        gpDevCache->FreeUpdateTerminalStatus();
    }
}


void ChargeService::StopChargeTerminalSelfStart(CHARGE_STEP &stChargeStep)
{
    LogOut(QString().sprintf("CAN=%d 自启动设备终止", stChargeStep.ucCanAddr), 2);

    //更新状态机
    stChargeStep.ucStopReasonCSCU = 118;//自启动设备终止
    stChargeStep.ucCmdValue = CHARGE_CMD_TYPE_STOP_CHARGE;//终止充电
    //UpdateChargeStepCtrlChargeCmd(stChargeStep);
    QDateTime dtQDateTime= QDateTime::currentDateTime();
    QDateTime2CharArray(dtQDateTime, stChargeStep.sRecvTime);
    stChargeStep.enChargeStep = CHARGE_STEP_WAITCMD_ACK;
    QDateTime2CharArray(dtQDateTime, stChargeStep.stChargeStepValue.sStartTimeStep);
    stChargeStep.stChargeStepValue.uc_hold_time  = g_TimeoutChargeStep[stChargeStep.enChargeStep];
    stChargeStep.stRepeatCmd.NowCount = 0;
    stChargeStep.stRepeatCmd.MaxCount = MAX_CAN_CMD_REPEAT;

        stChargeStep.stChargeStepValue.uc_charge_step_timeout = CHARGE_STEP_NORMAL;//接收ACK超时后,转到默认状态

         gpDevCache->UpateChargeStep(stChargeStep.ucCanAddr, stChargeStep);
    //充电状态机持久化,防止掉电丢失
    SaveChargeStep(stChargeStep);

    //发送结束充电指令
    SendControlCmdToBus(stChargeStep);
}
