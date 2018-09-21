#include <QDebug>
#include "CardChargeFun.h"
#include "CommonFunc/commfunc.h"

__u8 TimeoutChargeStep[] = {255, 5, 60, 60, 60, 60, 60, 60, 60, 60,15, 15, 15, 15, 2};//充电业务状态机超时时间定义


CardChargeFun::CardChargeFun()
{
    stCSCUSysConfig stTempCCUsysConfig;
	_strLogName = "charge";

    gpDevCache = DevCache::GetInstance();
    gpParamSet = ParamSet::GetInstance();
    gpLog = Log::GetInstance();

    gpParamSet->querySetting(&stTempCCUsysConfig, PARAM_CSCU_SYS);
    ucNumTerminal[0] = stTempCCUsysConfig.singlePhase;
    ucNumTerminal[1] = stTempCCUsysConfig.threePhase;
    ucNumTerminal[2] = stTempCCUsysConfig.directCurrent;

    InitTerminalStatus(stEmptyTerminalStatus);//创建一个空的终端状态
    InitChargeStep(stEmptyChargeStep);//创建一个空的终端状态
}

CardChargeFun::~CardChargeFun()
{

}
///
/// \brief netThread::CheckCanAddrValid 判断CAN地址是否在配置范围内
/// \param ucCanAddr
/// \return
///
bool CardChargeFun::CheckCanAddrValid(__u8 ucCanAddr)
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

void CardChargeFun::InitTerminalStatus(TerminalStatus &st_TerminalStatus)
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
    //st_TerminalStatus.stChangeDisplay =0;
}


void CardChargeFun::InitChargeStep(CHARGE_STEP &stChargeStep)
{
    //-------------------指令订单共用---------------------------------//
    stChargeStep.ucCanAddr = 0;
    memset(&stChargeStep.sEventNo, 0, LENGTH_EVENT_NO);
    memset(&stChargeStep.sBillCode, 0, LENGTH_BILL_CODE);
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
/// \brief CardChargeFun::InitOrderStatus 初始化状态机中订单相关变量
/// \param stChargeStep 待初始化的状态机
///
void CardChargeFun::InitOrderStatus(CHARGE_STEP &stChargeStep)
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
	stChargeStep.cChargeWay = CARD_START;
	stChargeStep.fLimitEnergy = 0;
	stChargeStep.ucQueueMsgFromServer = 0;
	stChargeStep.cGunNum = 1;
	stChargeStep.cOrderSync = 0;
   // stChargeStep.bChargeFreezeEnery = false;
}
///
/// \brief ChargeService::LogOut 日志记录
/// \param str 日志串
/// \param Level 级别
///
inline void  CardChargeFun::LogOut(QString str,int Level)
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

//------------------------------------------------------------刷卡远程充电相关---------------------------------------------------//

///
/// \brief CardChargeFun::ProcCardChargeSub 处理刷卡远程充电相关主题
/// \param qInfoMap  内容
/// \param InfoType 主题
///
void CardChargeFun::ProcCardChargeSub(InfoMap &qInfoMap, InfoAddrType &InfoType)
{
    switch (InfoType) {
    case AddrType_SingleCardApplyAccountInfo://单桩申请账户信息
    case AddrType_CenterCardApplyAccountInfo://集中刷卡申请账户信息
        LogOut(QString("收到申请账户信息主题!"), 1);
        ProcApplyAccountInfoSub(qInfoMap, InfoType);
        break;
    case AddrType_ChargeServicApplyAccountInfo_Result://服务器返回申请账户信息结果
        LogOut(QString("收到服务器返回申请账户信息结果主题!"), 1);
        ProcOutApplyAccountInfoResult(qInfoMap, InfoType);
        break;
    case AddrType_InApplyStartChargeByScreen://显示屏内部申请开始充电
    case AddrType_InApplyStartChargeByChargeEquipment://充电设备内部申请开始充电
    case AddrType_InApplyStopChargeByScreen://显示屏内部申请结束充电
    case AddrType_InApplyStopChargeByChargeEquipment://充电设备内部申请结束充电
        LogOut(QString("收到内部申请充电主题!"), 1);
        ProcInApplyCharge(qInfoMap, InfoType);
        break;
    case AddrType_OutApplyStartChargeByChargeServic_Result://服务器返回申请开始充电结果
    case AddrType_OutApplyStopChargeByChargeServic_Result://服务器返回申请结束充电结果
        LogOut(QString("收到服务器返回充电结果主题!"), 1);
        ProcOutApplyChargeResult(qInfoMap, InfoType);
        break;
    default:
        break;
    }
}

///
/// \brief CardChargeFun::ProcApplyAccountInfoSub  处理申请账户信息相关主题
/// \param qInfoMap 内容  信息体：卡号,CAN地址,刷卡业务查询账户信息类型
/// \param InfoType 主题
/// \return
///
bool CardChargeFun::ProcApplyAccountInfoSub(InfoMap &qInfoMap, InfoAddrType &InfoType)
{
    CHARGE_STEP stTempChargeStep = stEmptyChargeStep;//存放解析后的数据
    CHARGE_STEP stChargeStep = stEmptyChargeStep;//存放缓存查询出的数据

    if(ParseCardChargeSub(qInfoMap, stTempChargeStep) == -1){
        return false;
    }
    //校验CAN地址
    if(CheckCanAddrValid(stTempChargeStep.ucCanAddr) == false){
        LogOut(QString("用户只查询余额, CAN = %1!").arg(stTempChargeStep.ucCanAddr), 1);
        //用户只查询余额,CAN地址为0,充电服务根据配置赋默认值
        if(stTempChargeStep.ucCanAddr == 0){
            if(ucNumTerminal[0] > 0){                
                stTempChargeStep.ucCanAddr = 1;
            }
            else if(ucNumTerminal[1] > 0){
                stTempChargeStep.ucCanAddr = 151;
            }
            else if(ucNumTerminal[2] > 0){
                stTempChargeStep.ucCanAddr = 181;
            }
        }
        LogOut(QString("用户只查询余额, 添加默认地址CAN = %1!").arg(stTempChargeStep.ucCanAddr), 2);
    }

    if(gpDevCache->QueryChargeStep(stTempChargeStep.ucCanAddr, stChargeStep) == false){
        stChargeStep.ucCanAddr = stTempChargeStep.ucCanAddr;
        strcpy(stChargeStep.sCardNo, stTempChargeStep.sCardNo);
        strcpy(stChargeStep.sScanCodeNo, stTempChargeStep.sScanCodeNo);
        if(gpDevCache->AddChargeStep(stTempChargeStep.ucCanAddr, stTempChargeStep) == false){
            LogOut(QString("ProcApplyAccountInfoSub add chargestep fail!"), 3);
            return false;
        }
    }
    else{
        strcpy(stChargeStep.sCardNo, stTempChargeStep.sCardNo);
        strcpy(stChargeStep.sScanCodeNo, stTempChargeStep.sScanCodeNo);
    }

    switch (InfoType) {
    case AddrType_CenterCardApplyAccountInfo://集中刷卡申请账户信息
        LogOut(QString("集中申请账户信息!"), 1);
        stChargeStep.CardSrcType = 2;
        break;
    case AddrType_SingleCardApplyAccountInfo://单桩申请账户信息
        LogOut(QString("单桩申请账户信息!"), 1);
        stChargeStep.CardSrcType = 1;
        break;
    default:
        break;
    }
    if(gpDevCache->UpateChargeStep(stTempChargeStep.ucCanAddr, stChargeStep) == false){
        LogOut(QString("UpateChargeStep  fail!"), 3);
        return false;
    }
    if(SendApplyAccountInfoToServer(qInfoMap, stChargeStep) == false){
        LogOut(QString("Send ApplyAccountInfoToServer fail!"), 3);
        return false;
    }

    return true;
}

bool CardChargeFun::SendApplyAccountInfoToServer(InfoMap &qInfoMap, CHARGE_STEP &stChargeStep)
{
    LogOut(QString("Send ApplyAccountInfoToServer!"), 1);
    InfoMap TempqInfoMap;
    InfoAddrType TempInfoType;

    TempqInfoMap = qInfoMap;
    PackageApplyAccountInfoToServer(TempqInfoMap, TempInfoType, stChargeStep);
    emit sigToChargeService(TempqInfoMap, TempInfoType);
    return true;
}


void CardChargeFun::PackageApplyAccountInfoToServer(InfoMap &qInfoMap, InfoAddrType &InfoType, CHARGE_STEP &stChargeStep)
{
    QByteArray qTempByteArray;

    InfoType = AddrType_ChargeServicApplyAccountInfo; //充电服务申请账户信息

    if(qInfoMap.contains(Addr_CanID_Comm) == false){
        qTempByteArray.append(stChargeStep.ucCanAddr);
        qInfoMap.insert(Addr_CanID_Comm, qTempByteArray);//CAN地址
    }
    else if(qInfoMap[Addr_CanID_Comm].at(0) == 0){
        qInfoMap.insert(Addr_CanID_Comm,  qTempByteArray.append(stChargeStep.ucCanAddr));//CAN地址
    }

    if(qInfoMap.contains(Addr_CardAccount) == false){
        LogOut(QString("收到申请账户信息主题,但是没有卡号,添加默认值!"), 3);
        qTempByteArray.clear();
        qTempByteArray = QByteArray::fromHex(QString(stChargeStep.sCardNo).toAscii());
        qInfoMap.insert(Addr_CardAccount, qTempByteArray);//刷卡卡号
    }
    if(qInfoMap.contains(Addr_ScanCode_customerID) == false){
        LogOut(QString("收到申请账户信息主题,但是没有扫码客户ID,添加默认值!"), 3);
        qTempByteArray.clear();
        qTempByteArray = QByteArray::fromHex(QString(stChargeStep.sScanCodeNo).toAscii());
        qInfoMap.insert(Addr_ScanCode_customerID, qTempByteArray);//扫码客户ID
    }

    if(qInfoMap.contains(Addr_CardAccountType) == false){
        LogOut(QString("收到申请账户信息主题,但是没有申请账户信息类型,添加默认值!"), 3);
        qTempByteArray.clear();
        qTempByteArray.append(1);//1默认全部信息
        qInfoMap.insert(Addr_CardAccountType, qTempByteArray);//刷卡业务查询账户信息类型
    }
    if(qInfoMap.contains(Addr_ScanCode_Type) == false){
        LogOut(QString("收到申请账户信息主题,但是没有申请账户信息类型,添加默认值!"), 3);
        qTempByteArray.clear();
        qTempByteArray.append(1);//1默认全部信息
        qInfoMap.insert(Addr_ScanCode_Type, qTempByteArray);//扫码业务查询账户信息类型
    }
    LogOut(QString("打包申请账户信息, 卡号:%1, 扫码客户ID:%2 ,刷卡申请账户类型:%3, 扫码申请账户类型:%4,CAN:%5")\
           .arg(QString(stChargeStep.sCardNo))\
           .arg(QString(stChargeStep.sScanCodeNo))\
           .arg((__u8)qInfoMap[Addr_CardAccountType].at(0))\
           .arg((__u8)qInfoMap[Addr_ScanCode_Type].at(0))\
           .arg(stChargeStep.ucCanAddr), 2);
}


bool CardChargeFun::UpdateChargeStepBySendOutCardApplyStartCharge(__u8 ucCanAddr, char CardSrcType)
{
    LogOut(QString("UpdateChargeStepBySendOutCardApplyStartCharge  CAN = %1!").arg(ucCanAddr), 1);
    CHARGE_STEP stChargeStep;

    if(gpDevCache->QueryChargeStep(ucCanAddr, stChargeStep) == false){
        return false;
    }

	stChargeStep.cChargeWay = CARD_START;//刷卡启动

    stChargeStep.CardSrcType = CardSrcType;//卡号来源类型1.单桩刷卡, 2.集中刷卡
    switch (CardSrcType) {
    case 1:
        stChargeStep.ucCmdSrc = CHARGE_CMD_SRC_CAN_DEV;//充电设备
        stChargeStep.ucStartReason = START_CHARGE_CMD_RSN_DEV_CARD_REMOTE;//106 单桩刷卡（远程）
        LogOut(QString("记录启动原因单桩刷卡 CAN = %1!").arg(ucCanAddr), 1);
        break;
    case 2:
        stChargeStep.ucCmdSrc = CHARGE_CMD_SRC_CSCU;//充电系统控制器
        stChargeStep.ucStartReason = START_CHARGE_CMD_RSN_CARD_REMOTE;//102 集中刷卡（远程）
        LogOut(QString("记录启动原因集中刷卡 CAN = %1!").arg(ucCanAddr), 1);
        break;
    default:
        break;
    }
//    stChargeStep.enChargeStep = CHARGE_STEP_WAITSEVER_ACK_APPLY_START_CHARGE_CARD;
//    stChargeStep.stChargeStepValue.uc_charge_step_timeout = CHARGE_STEP_NORMAL;
//    stChargeStep.stChargeStepValue.uc_hold_time = TimeoutChargeStep[stChargeStep.enChargeStep];

    if(gpDevCache->UpateChargeStep(stChargeStep.ucCanAddr, stChargeStep) == false){
        return false;
    }

    if(gpDevCache->SaveChargeStep(ucCanAddr) == false){
        return false;
    }
    return true;
}

bool CardChargeFun::UpdateChargeStepBySendOutCardApplyStopCharge(__u8 ucCanAddr, char CardSrcType)
{
    CHARGE_STEP stChargeStep;

    if(gpDevCache->QueryChargeStep(ucCanAddr, stChargeStep) == false){
        return false;
    }

    stChargeStep.CardSrcType = CardSrcType;
    switch (stChargeStep.CardSrcType) {//卡号来源类型1.单桩刷卡, 2.集中刷卡
    case 1:
        stChargeStep.ucStopReasonCSCU = 112;//单桩刷卡（远程）
        break;
    case 2:
        stChargeStep.ucStopReasonCSCU = 103;//集中刷卡（远程）
        break;
    default:
        stChargeStep.ucStopReasonCSCU = 103;//默认 集中刷卡（远程）
        break;
    }

//    stChargeStep.enChargeStep = CHARGE_STEP_WAITSEVER_ACK_APPLY_STOP_CHARGE_CARD;
//    stChargeStep.stChargeStepValue.uc_charge_step_timeout = CHARGE_STEP_NORMAL;
//    stChargeStep.stChargeStepValue.uc_hold_time = TimeoutChargeStep[stChargeStep.enChargeStep];

    if(gpDevCache->UpateChargeStep(stChargeStep.ucCanAddr, stChargeStep) == false){
        return false;
    }

    if(gpDevCache->SaveChargeStep(ucCanAddr) == false){
        return false;
    }
    return true;

}

bool CardChargeFun::UpdateChargeStepByRecvOutCardApplyStartChargeResult(CHARGE_STEP &stChargeStep, int ApplyChargeResult)
{
    if(ApplyChargeResult != 0xff){
        LogOut(QString("申请充电结果不成功, 清空启动原因 ApplyChargeResult = %1!").arg(ApplyChargeResult), 3);
        stChargeStep.ucStartReason = START_CHARGE_CMD_RSN_NON;
    }
//    stChargeStep.enChargeStep = CHARGE_STEP_NORMAL;
//    stChargeStep.stChargeStepValue.uc_charge_step_timeout = CHARGE_STEP_NORMAL;
//    stChargeStep.stChargeStepValue.uc_hold_time = TimeoutChargeStep[stChargeStep.enChargeStep];

    if(gpDevCache->UpateChargeStep(stChargeStep.ucCanAddr, stChargeStep) == false){
        return false;
    }

    return true;
}


bool CardChargeFun::UpdateChargeStepByRecvOutCardApplyStopChargeResult(CHARGE_STEP &stChargeStep, int ApplyChargeResult)
{
    if(ApplyChargeResult != 0xff){
        stChargeStep.ucStopReasonCSCU = 0;//CSCU 未终止
    }
    stChargeStep.enChargeStep = CHARGE_STEP_NORMAL;
    stChargeStep.stChargeStepValue.uc_charge_step_timeout = CHARGE_STEP_NORMAL;
    stChargeStep.stChargeStepValue.uc_hold_time = TimeoutChargeStep[stChargeStep.enChargeStep];

    if(gpDevCache->UpateChargeStep(stChargeStep.ucCanAddr, stChargeStep) == false){
        return false;
    }

    return true;
}

///
/// \brief CardChargeFun::ParseCardChargeSub
/// \param qInfoMap 接收到的数据
/// \param stTempChargeStep 存放解析后的数据
/// \return 0则正常返回,-1错误返回,其他申请结果返回.
///
int CardChargeFun::ParseCardChargeSub(InfoMap &qInfoMap, CHARGE_STEP &stTempChargeStep)
{
    int ApplyChargeResult = 0;

    if(qInfoMap.contains(Addr_CanID_Comm)){//CAN地址
        stTempChargeStep.ucCanAddr = qInfoMap[Addr_CanID_Comm].at(0);
        LogOut(QString("CAN = %1!").arg(stTempChargeStep.ucCanAddr), 1);
    }
    else{//必备项,无则返回失败
        return -1;
    }
    if(qInfoMap.contains(Addr_CardAccount)){//卡号
        strcpy(stTempChargeStep.sCardNo, qInfoMap[Addr_CardAccount].toHex().data());
        LogOut(QString("接收卡号:%1 CAN = %2!").arg(qInfoMap[Addr_CardAccount].toHex().data()).arg(stTempChargeStep.ucCanAddr),2);
    }
    if(qInfoMap.contains(Addr_ScanCode_customerID)){//扫码客户ID
        strcpy(stTempChargeStep.sScanCodeNo, qInfoMap[Addr_ScanCode_customerID].data());
//        strcpy(stTempChargeStep.sScanCodeNo, qInfoMap[Addr_ScanCode_customerID].data());
        LogOut(QString("扫码客户ID:%1 CAN = %2!").arg(qInfoMap[Addr_ScanCode_customerID].data()).arg(stTempChargeStep.ucCanAddr),2);
    }
    if(qInfoMap.contains(Addr_CardApplyCharge_Result)){//刷卡申请充电返回结果
        ApplyChargeResult = qInfoMap[Addr_CardApplyCharge_Result].at(0);
        LogOut(QString("服务器返回申请开始充电结果 = %1!").arg(ApplyChargeResult), 2);
        return ApplyChargeResult;
    }
    if(qInfoMap.contains(Addr_CardStopCharge_Result)){//刷卡申请终止充电返回结果
        ApplyChargeResult = qInfoMap[Addr_CardStopCharge_Result].at(0);
        LogOut(QString("服务器返回申请结束充电结果 = %1!").arg(ApplyChargeResult), 2);
        return ApplyChargeResult;
    }
    if(qInfoMap.contains(Addr_ScanCode_StartCharge_Result)){//扫码申请充电返回结果
        ApplyChargeResult = qInfoMap[Addr_ScanCode_StartCharge_Result].at(0);
        LogOut(QString("服务器返回申请开始充电结果 = %1!").arg(ApplyChargeResult), 2);
        return ApplyChargeResult;
    }
    if(qInfoMap.contains(Addr_ScanCode_StopCharge_Result)){//扫码申请终止充电返回结果
        ApplyChargeResult = qInfoMap[Addr_ScanCode_StopCharge_Result].at(0);
        LogOut(QString("服务器返回申请结束充电结果 = %1!").arg(ApplyChargeResult), 2);
        return ApplyChargeResult;
    }
    //对于账户列表对应余额,充电服务无需关注,所以无需解析,只需要根据申请方直接转发即可
    return 0;
}


bool CardChargeFun::ProcOutApplyAccountInfoResult(InfoMap &qInfoMap, InfoAddrType &InfoType)
{
    LogOut(QString(" ProcApplyAccountInfoResult!"), 1);
    CHARGE_STEP stTempChargeStep = stEmptyChargeStep;
    CHARGE_STEP stChargeStep = stEmptyChargeStep;
    InfoMap TempqInfoMap;
    InfoAddrType TempInfoType;

    ParseCardChargeSub(qInfoMap, stTempChargeStep);
    //申请账户信息结果不校验CAN地址(用户只查询余额)
//    if(CheckCanAddrValid(stTempChargeStep.ucCanAddr) == false){
//        return false;
//    }

    if(gpDevCache->QueryChargeStep(stTempChargeStep.ucCanAddr, stChargeStep) == false){//缓存中没有该终端状态机
//        return false;//屏蔽(用户只查询余额)
    }
    if(strcmp(stTempChargeStep.sCardNo, stChargeStep.sCardNo) !=0 && strcmp(stTempChargeStep.sScanCodeNo, stChargeStep.sScanCodeNo) !=0){//卡号或者扫码customerID跟之前存放不一致
        LogOut(QString("账户信息中,卡号跟之前存放不一致,返回错误!"), 3);
        return false;
    }
    switch (stChargeStep.CardSrcType) {
    case 1://单桩读卡卡号
        LogOut(QString("发送账户信息至单桩的主题!"), 1);
        InfoType = AddrType_ApplyAccountInfoResult_ToChargeEquipment;
        break;
    case 2://集中读卡卡号
        LogOut(QString("发送账户信息至显示屏的主题!"), 1);
        InfoType = AddrType_ApplyAccountInfoResult_ToScreen;
        break;
    default://默认返回给集中读卡卡号
        LogOut(QString("默认,发送账户信息至单桩的主题!"), 1);
        InfoType = AddrType_ApplyAccountInfoResult_ToScreen;
        break;
    }

    TempqInfoMap = qInfoMap;
    TempInfoType = InfoType;
    emit sigToChargeService(TempqInfoMap, TempInfoType);
    return true;
}

//处理内部显示屏申请开始充电
bool CardChargeFun::ProcInApplyStartChargeByScreen(InfoMap &qInfoMap)
{
    LogOut(QString("ProcInApplyStartChargeByScreen!"), 1);
    CHARGE_STEP stTempChargeStep = stEmptyChargeStep;//存放总线解析后的数据
    int InApplyStartChargeResult = -1;

    if(ParseCardChargeSub(qInfoMap, stTempChargeStep) == -1){
        return false;
    }
    //校验CAN地址
    if(CheckCanAddrValid(stTempChargeStep.ucCanAddr) == false){
        return false;
    }
    //校验是否允许充电
    InApplyStartChargeResult = CheckInApplyStartChargeValidByCard(stTempChargeStep);
    if(SendInApplyStartChargeResultByCardToScreen(stTempChargeStep, InApplyStartChargeResult) == false){//返回校验结果
        return false;
    }
    if(InApplyStartChargeResult != 0xff){
        return false;
    }
    //向服务器发送申请充电
    if(SendOuApplyStartChargeByCard(qInfoMap) == false){
        return false;
    }

    if(UpdateChargeStepBySendOutCardApplyStartCharge(stTempChargeStep.ucCanAddr, 2)){
        return false;
    }

    return true;
}


bool CardChargeFun::ProcInApplyStopChargeByScreen(InfoMap &qInfoMap)
{
    LogOut(QString("ProcInApplyStopChargeByScreen!"), 1);
    CHARGE_STEP stTempChargeStep = stEmptyChargeStep;//存放总线解析后的数据
    CHARGE_STEP stChargeStep = stEmptyChargeStep;//存放状态机中已有的数据

    int InApplyStopChargeResult = -1;

    if(ParseCardChargeSub(qInfoMap, stTempChargeStep) == -1){
        return false;
    }
    //校验CAN地址
    if(CheckCanAddrValid(stTempChargeStep.ucCanAddr) == false){
        return false;
    }

    gpDevCache->QueryChargeStep(stTempChargeStep.ucCanAddr, stChargeStep);
    //校验是否允许结束充电
    InApplyStopChargeResult = CheckInApplyStopChargeValidByCard(stTempChargeStep);
    if(SendInApplyStopChargeResultByCardToScreen(stTempChargeStep, InApplyStopChargeResult) == false){//返回校验结果
        return false;
    }
    if(InApplyStopChargeResult == 0x01){//无状态机
        LogOut(QString("卡号校验失败,没找到状态机!"), 2);
        SendApplyStopChargeResultToScreen(stChargeStep, 251);//没有找到订单
        return false;
    }
    else if(InApplyStopChargeResult == 0xff && qInfoMap.contains(Addr_ScanCode_customerID) != 1){//卡号校验通过(扫码结束，必须等待平台返回)
        LogOut(QString("卡号校验通过,直接下发结束指令!"), 2);
        //记录结束原因,更新状态机,下发结束充电指令,返回结果
        stChargeStep.ucStopReasonCSCU = 111;//集中刷卡（本地）
        stChargeStep.ucCmdValue = CHARGE_CMD_TYPE_STOP_CHARGE;//终止充电
        UpdateChargeStepBySendStopChargeCmd(stChargeStep);//更新状态机
        //发送指令
        SendChargeCMDToChargeEquipment(stChargeStep);
         //返回结果
        SendApplyStopChargeResultToScreen(stChargeStep, 255);//申请结束充电成功
        //持久化状态机
        if(gpDevCache->SaveChargeStep(stChargeStep.ucCanAddr) == false){
            LogOut(QString("SaveChargeStep  fail!"), 3);
        }
        return true;
    }
    //向服务器发送申请结束充电

	//增加订单号
    qInfoMap.insert(Addr_Bill_Code, QByteArray().append(stChargeStep.sBillCode, strlen(stChargeStep.sBillCode)));
	//应急充电不向服务器校验
    if(stChargeStep.ucCmdSrc != CHARGE_CMD_SRC_CSCU_EMERGENCY && SendOuApplyStopChargeByCard(qInfoMap) == false){
        return false;
    }

    if(UpdateChargeStepBySendOutCardApplyStopCharge(stTempChargeStep.ucCanAddr, 2)){
        return false;
    }

    return true;
}


bool CardChargeFun::ProcInApplyStopChargeByChargeEquipment(InfoMap &qInfoMap)
{
    LogOut(QString("ProcInApplyStopChargeByChargeEquipment!"), 1);
    CHARGE_STEP stTempChargeStep = stEmptyChargeStep;//存放总线解析后的数据
    CHARGE_STEP stChargeStep = stEmptyChargeStep;//存放状态机中已有的数据
    int InApplyStopChargeResult = -1;

    if(ParseCardChargeSub(qInfoMap, stTempChargeStep) == -1){
        return false;
    }
    //校验CAN地址
    if(CheckCanAddrValid(stTempChargeStep.ucCanAddr) == false){
        return false;
    }

    gpDevCache->QueryChargeStep(stTempChargeStep.ucCanAddr, stChargeStep);

    //校验是否允许充电
    InApplyStopChargeResult = CheckInApplyStopChargeValidByCard(stTempChargeStep);
    if(InApplyStopChargeResult ==2)
    {
         LogOut(QString("设备刷卡结束由设备保证开始与结束卡号一致此处不做校验直接结束"), 3);
        InApplyStopChargeResult = 0xff;
    }
    if(SendInApplyStoptChargeResultByCardToChargeEquipment(stTempChargeStep, InApplyStopChargeResult) == false){//返回校验结果
        return false;
    }
    if(InApplyStopChargeResult == 0x01){//无状态机
        return false;
    }
    else if(InApplyStopChargeResult == 0xff){//卡号校验通过
        //记录结束原因,更新状态机,下发结束充电指令,返回结果
        stChargeStep.ucStopReasonCSCU = 113;//单桩刷卡（本地）
        stChargeStep.ucCmdValue = CHARGE_CMD_TYPE_STOP_CHARGE;//终止充电
        UpdateChargeStepBySendStopChargeCmd(stChargeStep);//更新状态机
        //发送指令
        SendChargeCMDToChargeEquipment(stChargeStep);
         //返回结果
        SendApplyStopChargeResultToChargeEquipment(stChargeStep, 255);//申请结束充电成功
        //持久化状态机
        if(gpDevCache->SaveChargeStep(stChargeStep.ucCanAddr) == false){
            LogOut(QString("SaveChargeStep  fail!"), 3);
        }
        return true;
    }
    //向服务器发送申请结束充电
    if(SendOuApplyStopChargeByCard(qInfoMap) == false){
        return false;
    }

    if(UpdateChargeStepBySendOutCardApplyStopCharge(stTempChargeStep.ucCanAddr, 1)){
        return false;
    }

    return true;
}


///
/// \brief CardChargeFun::UpdateChargeStepBySendStopChargeCmd 由充电控制指令触发更新充电步骤状态机
/// \param stChargeStep 收到的指令
/// \return
///
bool CardChargeFun::UpdateChargeStepBySendStopChargeCmd(CHARGE_STEP &stChargeStep)
{
    //状态机中指令状态机相关变量更新
    QDateTime dtQDateTime= QDateTime::currentDateTime();
    QDateTime2CharArray(dtQDateTime, stChargeStep.sRecvTime);
    stChargeStep.enChargeStep = CHARGE_STEP_WAITCMD_ACK;
    QDateTime2CharArray(dtQDateTime, stChargeStep.stChargeStepValue.sStartTimeStep);
    stChargeStep.stChargeStepValue.uc_hold_time  = 5;
    stChargeStep.stRepeatCmd.NowCount = 0;
    stChargeStep.stRepeatCmd.MaxCount = 5;
    stChargeStep.stChargeStepValue.uc_charge_step_timeout = CHARGE_STEP_NORMAL;
    gpDevCache->UpateChargeStep(stChargeStep.ucCanAddr, stChargeStep);
    return true;
}

///
/// \brief CardChargeFun::PackageTelecontrol2ChargeEquipmentr 将指令发送给充电模块
/// \param stChargeStep 充电业务状态机
///
void CardChargeFun::PackageTelecontrol2ChargeEquipment(CHARGE_STEP &stChargeStep,InfoMap &qInfoMap, InfoAddrType &InfoType)
{
    QByteArray qTempByteArray;

    InfoType = AddrType_CmdCtrl;
    qTempByteArray.append(stChargeStep.ucCanAddr);
    qInfoMap.insert(Addr_CanID_Comm, qTempByteArray);

    qTempByteArray.clear();
    qTempByteArray.append(stChargeStep.ucCmdValue);
    qInfoMap.insert(Addr_ChargeCmd_Ctrl, qTempByteArray);
}


bool CardChargeFun::SendChargeCMDToChargeEquipment(CHARGE_STEP &stChargeStep)
{
    InfoMap TempqInfoMap;
    InfoAddrType TempInfoType;

    TempqInfoMap.clear();
    PackageTelecontrol2ChargeEquipment(stChargeStep, TempqInfoMap, TempInfoType);
    emit sigToChargeService(TempqInfoMap, TempInfoType);
    return true;
}

//将外部申请结束充电结果发送给充电设备
void CardChargeFun::SendApplyStopChargeResultToChargeEquipment(CHARGE_STEP &stChargeStep, unsigned char ucResult)
{
    InfoMap TempqInfoMap;
    QByteArray qTempByteArray;

    qTempByteArray.clear();
    qTempByteArray.append(stChargeStep.ucCanAddr);
    TempqInfoMap.insert(Addr_CanID_Comm, qTempByteArray);//CAN地址

    qTempByteArray.clear();
    qTempByteArray.append(ucResult);
    TempqInfoMap.insert(Addr_CardStopCharge_Result, qTempByteArray);//刷卡申请终止充电返回结果

    qTempByteArray.clear();
    qTempByteArray = QByteArray::fromHex(QString(stChargeStep.sCardNo).toAscii());
    TempqInfoMap.insert(Addr_CardAccount, qTempByteArray);//刷卡卡号

    qTempByteArray.clear();
    qTempByteArray = QByteArray::fromHex(QString(stChargeStep.sScanCodeNo).toAscii());
    TempqInfoMap.insert(Addr_ScanCode_customerID, qTempByteArray);//扫码客户ID
    emit sigToChargeService(TempqInfoMap, AddrType_OutApplyStopChargeResult_ToChargeEquipment);
}

//将外部申请结束充电结果发送给显示屏
void CardChargeFun::SendApplyStopChargeResultToScreen(CHARGE_STEP &stChargeStep, unsigned char ucResult)
{
    InfoMap TempqInfoMap;
    QByteArray qTempByteArray;

    qTempByteArray.clear();
    qTempByteArray.append(stChargeStep.ucCanAddr);
    TempqInfoMap.insert(Addr_CanID_Comm, qTempByteArray);//CAN地址

    qTempByteArray.clear();
    qTempByteArray.append(ucResult);
    TempqInfoMap.insert(Addr_CardStopCharge_Result, qTempByteArray);//刷卡申请终止充电返回结果

    qTempByteArray.clear();
    qTempByteArray = QByteArray::fromHex(QString(stChargeStep.sCardNo).toAscii());
    TempqInfoMap.insert(Addr_CardAccount, qTempByteArray);//刷卡卡号

    qTempByteArray.clear();
    qTempByteArray = QByteArray::fromHex(QString(stChargeStep.sScanCodeNo).toAscii());
    TempqInfoMap.insert(Addr_ScanCode_customerID, qTempByteArray);//扫码客户ID

    emit sigToChargeService(TempqInfoMap, AddrType_OutApplyStopChargeResult_ToScreen);
}

int CardChargeFun::CheckInApplyStartChargeValidByCard(CHARGE_STEP &stTempChargeStep)
{
    CHARGE_STEP stChargeStep = stEmptyChargeStep;//状态机中的数据

    if(gpDevCache->QueryChargeStep(stTempChargeStep.ucCanAddr, stChargeStep) == false){//缓存中没有该终端状态机
        LogOut(QString("收到BUS 屏幕内部申请充电,无状态机,创建一个!"), 3);
        strcpy(stChargeStep.sCardNo, stTempChargeStep.sCardNo);
        strcpy(stChargeStep.sScanCodeNo, stTempChargeStep.sScanCodeNo);//扫码
        stChargeStep.ucCanAddr = stTempChargeStep.ucCanAddr;
        if(gpDevCache->AddChargeStep(stTempChargeStep.ucCanAddr, stTempChargeStep) == false){
            return 1;
        }
        return 1;
    }

    //add by FJC 未收到平台确认，不响应充电指令
    TerminalStatus stTerminalStatus = stEmptyTerminalStatus;//存放缓存中的数据
    stChargeConfig charge;
    memset(&charge, 0, sizeof(stChargeConfig));
    gpParamSet->querySetting(&charge, PARAM_CHARGE);
    //更新状态机
    if(gpDevCache->QueryTerminalStatus(stTempChargeStep.ucCanAddr, stTerminalStatus) == false){
        LogOut(QString("刷卡充电，获取终端状态失败!!!!!!!!!!"), 3);
        return 1;
    }
    //多枪使能，没有经过平台确认，CAN地址是直流则判断为不允许充电2017-8-16
    if((charge.coupleGun != 0) && (stTerminalStatus.chargeResponseFlag == false) &&\
            (stTempChargeStep.ucCanAddr>ID_MaxACThrCanID )&& (stTempChargeStep.ucCanAddr<ID_MinCCUCanID))
    {
        LogOut(QString("单双枪充电方式未获得平台确认!!!!!!!!!!"), 2);
        return 246;
    }

    return 0xff;
}

int CardChargeFun::CheckInApplyStopChargeValidByCard(CHARGE_STEP &stTempChargeStep)
{
    CHARGE_STEP stChargeStep = stEmptyChargeStep;//状态机中的数据

    if(gpDevCache->QueryChargeStep(stTempChargeStep.ucCanAddr, stChargeStep) == false){//缓存中没有该终端状态机
        LogOut(QString("收到BUS 屏幕内部申请结束充电,无状态机,直接返回!"), 3);
        return 1;
    }
    //比较当前卡号与状态机中的卡号是否一致
    if(strcmp(stChargeStep.sCardNo, stTempChargeStep.sCardNo) != 0 || strcmp(stChargeStep.sScanCodeNo, stTempChargeStep.sScanCodeNo) != 0){
        LogOut(QString("申请结束充电卡卡号与之前存储不一致,转给服务器校验!"), 3);
        return 2;//2表示当前卡号与状态机中的卡号不一致.
    }
//    if(strcmp(stChargeStep.sScanCodeNo, stTempChargeStep.sScanCodeNo) != 0){
//        LogOut(QString("申请结束充电扫码customerID与之前存储不一致,转给服务器校验!"), 3);
//        return 2;//2表示当前扫码customerID与状态机中的扫码customerID不一致.
//    }

    return 0xff;//0xff代表当前卡号与状态机中的一致
}

int CardChargeFun::CheckInApplyStopChargeValidByCardFromChargeEquipment(CHARGE_STEP &stTempChargeStep)
{
    CHARGE_STEP stChargeStep = stEmptyChargeStep;//状态机中的数据

    if(gpDevCache->QueryChargeStep(stTempChargeStep.ucCanAddr, stChargeStep) == false){//缓存中没有该终端状态机
        LogOut(QString("收到BUS 充电设备内部申请结束充电,无状态机,直接返回!"), 3);
        return 1;
    }
    //由于单桩申请结束充电没有卡号,所以这里不对卡号做校验

    return 0xff;
}


bool CardChargeFun::SendInApplyStartChargeResultByCardToScreen(CHARGE_STEP &stChargeStep, int InApplyStartChargeResult)
{
    LogOut(QString("发送内部刷卡申请开始充电结果给显示屏!"), 1);
    InfoMap TempqInfoMap;
    QByteArray qTempByteArray;

    qTempByteArray.append(stChargeStep.ucCanAddr);
    TempqInfoMap.insert(Addr_CanID_Comm, qTempByteArray);//CAN地址

    qTempByteArray.clear();
    qTempByteArray = QByteArray::fromHex(QString(stChargeStep.sCardNo).toAscii());
    if(qTempByteArray.length() >=6)
        TempqInfoMap.insert(Addr_CardAccount, qTempByteArray);//卡号
    qTempByteArray.clear();
    qTempByteArray = QByteArray::fromHex(QString(stChargeStep.sScanCodeNo).toAscii());
    if(qTempByteArray.length() >=16)
        TempqInfoMap.insert(Addr_ScanCode_customerID, qTempByteArray);//扫码customerID

    qTempByteArray.clear();
    qTempByteArray.append(InApplyStartChargeResult);
    TempqInfoMap.insert(Addr_InApplyStartCharge_Result, qTempByteArray);//CSCU内部申请开始充电返回结果
    emit sigToChargeService(TempqInfoMap, AddrType_InApplyStartChargeResult_ToScreen);
    return true;
}


bool CardChargeFun::SendInApplyStopChargeResultByCardToScreen(CHARGE_STEP &stChargeStep, int InApplyStopChargeResult)
{
    LogOut(QString("发送内部刷卡申请结束充电结果给显示屏!"), 1);
    InfoMap TempqInfoMap;
    QByteArray qTempByteArray;

    qTempByteArray.append(stChargeStep.ucCanAddr);
    TempqInfoMap.insert(Addr_CanID_Comm, qTempByteArray);//CAN地址

    qTempByteArray.clear();
    qTempByteArray = QByteArray::fromHex(QString(stChargeStep.sCardNo).toAscii());
    if(qTempByteArray.length() >=6)
        TempqInfoMap.insert(Addr_CardAccount, qTempByteArray);//卡号
    qTempByteArray.clear();
    qTempByteArray = QByteArray::fromHex(QString(stChargeStep.sScanCodeNo).toAscii());
    if(qTempByteArray.length() >=16)
        TempqInfoMap.insert(Addr_ScanCode_customerID, qTempByteArray);//扫码customerID

    qTempByteArray.clear();
    qTempByteArray.append(InApplyStopChargeResult);
    TempqInfoMap.insert(Addr_InApplyStopCharge_Result, qTempByteArray);//CSCU内部申请结束充电返回结果
    emit sigToChargeService(TempqInfoMap, AddrType_InApplyStopChargeResult_ToScreen);
    return true;
}


bool CardChargeFun::SendInApplyStoptChargeResultByCardToChargeEquipment(CHARGE_STEP &stChargeStep, int InApplyStopChargeResult)
{
    LogOut(QString("发送内部刷卡申请结束充电结果给单桩!"), 1);
    InfoMap TempqInfoMap;
    QByteArray qTempByteArray;

    qTempByteArray.append(stChargeStep.ucCanAddr);
    TempqInfoMap.insert(Addr_CanID_Comm, qTempByteArray);//CAN地址

    qTempByteArray.clear();
    qTempByteArray = QByteArray::fromHex(QString(stChargeStep.sCardNo).toAscii());
    TempqInfoMap.insert(Addr_CardAccount, qTempByteArray);//卡号

    qTempByteArray.clear();
    qTempByteArray = QByteArray::fromHex(QString(stChargeStep.sScanCodeNo).toAscii());
    TempqInfoMap.insert(Addr_ScanCode_customerID, qTempByteArray);//扫码客户ID

    qTempByteArray.clear();
    qTempByteArray.append(InApplyStopChargeResult);
    TempqInfoMap.insert(Addr_InApplyStopCharge_Result, qTempByteArray);//CSCU内部申请开始充电返回结果
    emit sigToChargeService(TempqInfoMap, AddrType_InApplyStopChargeResult_ToChargeEquipment);

    return true;
}

bool CardChargeFun::SendInApplyStartChargeResultByCardToChargeEquipment(CHARGE_STEP &stChargeStep, int InApplyStartChargeResult)
{
    LogOut(QString("发送内部刷卡申请开始充电结果给单桩!"), 1);
    InfoMap TempqInfoMap;
    QByteArray qTempByteArray;

    qTempByteArray.append(stChargeStep.ucCanAddr);
    TempqInfoMap.insert(Addr_CanID_Comm, qTempByteArray);//CAN地址

    qTempByteArray.clear();
    qTempByteArray = QByteArray::fromHex(QString(stChargeStep.sCardNo).toAscii());
    if(qTempByteArray.length() >= 8)
        TempqInfoMap.insert(Addr_CardAccount, qTempByteArray);//卡号
    qTempByteArray.clear();
    qTempByteArray = QByteArray::fromHex(QString(stChargeStep.sScanCodeNo).toAscii());
    if(qTempByteArray.length() >= 16)
        TempqInfoMap.insert(Addr_ScanCode_customerID, qTempByteArray);//扫码客户ID

    qTempByteArray.clear();
    qTempByteArray.append(InApplyStartChargeResult);
    TempqInfoMap.insert(Addr_InApplyStartCharge_Result, qTempByteArray);//CSCU内部申请开始充电返回结果
    emit sigToChargeService(TempqInfoMap, AddrType_InApplyStartChargeResult_ToChargeEquipment);

    return true;
}

bool CardChargeFun::SendOuApplyStartChargeByCard(InfoMap &qInfoMap)
{
    LogOut(QString("SendOuApplyStartChargeByCard!"), 1);
    InfoMap TempqInfoMap;

    TempqInfoMap = qInfoMap;
    emit sigToChargeService(TempqInfoMap, AddrType_OutApplyStartChargeByChargeServic);
    return true;
}

bool CardChargeFun::SendOuApplyStopChargeByCard(InfoMap &qInfoMap)
{
    LogOut(QString("SendOuApplyStopChargeByCard!"), 1);
    InfoMap TempqInfoMap;

    TempqInfoMap = qInfoMap;
    emit sigToChargeService(TempqInfoMap, AddrType_OutApplyStopChargeByChargeServic);

    return true;
}

///
/// \brief CardChargeFun::ProcInApplyStartChargeByChargeEquipment
/// \param qInfoMap 信息体：CAN地址、刷卡申请充电类型、充电金额、充电电量、充电时间 (注意没有卡号)
/// \return
///
bool CardChargeFun::ProcInApplyStartChargeByChargeEquipment(InfoMap &qInfoMap)
{
    LogOut(QString("ProcInApplyStartChargeByChargeEquipment!"), 1);
    CHARGE_STEP stTempChargeStep = stEmptyChargeStep;//存放总线解析后的数据
    CHARGE_STEP stChargeStep = stEmptyChargeStep;//存放缓存查询的状态机

    int InApplyStartChargeResult = -1;

    if(ParseCardChargeSub(qInfoMap, stTempChargeStep) == -1){
        return false;
    }
    //校验CAN地址
    if(CheckCanAddrValid(stTempChargeStep.ucCanAddr) == false){
        return false;
    }
    //校验是否允许充电
    InApplyStartChargeResult = CheckInApplyStartChargeValidByCard(stTempChargeStep);
    if(SendInApplyStartChargeResultByCardToChargeEquipment(stTempChargeStep, InApplyStartChargeResult) == false){//返回校验结果
        return false;
    }
    if(InApplyStartChargeResult != 0xff){
        return false;
    }

    //单桩刷卡,申请开始充电报文中没有卡号,这里人工添加上
    if(gpDevCache->QueryChargeStep(stTempChargeStep.ucCanAddr, stChargeStep) == false){
        return false;
    }
    qInfoMap.insert(Addr_CardAccount, QByteArray::fromHex(QString(stChargeStep.sCardNo).toAscii()));
    qInfoMap.insert(Addr_ScanCode_customerID, QByteArray::fromHex(QString(stChargeStep.sScanCodeNo).toAscii()));
    //向服务器发送申请充电
    if(SendOuApplyStartChargeByCard(qInfoMap) == false){
        return false;
    }

    if(UpdateChargeStepBySendOutCardApplyStartCharge(stTempChargeStep.ucCanAddr, 1)){
        return false;
    }

    return true;
}


bool CardChargeFun::ProcInApplyCharge(InfoMap &qInfoMap, InfoAddrType &InfoType)
{
    LogOut(QString("ProcInApplyCharge!"), 1);
    switch (InfoType) {
    case AddrType_InApplyStartChargeByScreen://显示屏内部申请开始充电, 集中申请充电
        if(ProcInApplyStartChargeByScreen(qInfoMap) == true){
            return false;
        }
        break;
    case AddrType_InApplyStopChargeByScreen://显示屏内部申请结束充电
        if(ProcInApplyStopChargeByScreen(qInfoMap) == true){
            return false;
        }
        break;
    case AddrType_InApplyStartChargeByChargeEquipment://充电设备内部申请开始充电
        if(ProcInApplyStartChargeByChargeEquipment(qInfoMap) == true){
            return false;
        }
        break;
    case AddrType_InApplyStopChargeByChargeEquipment://充电设备内部申请结束充电
        if(ProcInApplyStopChargeByChargeEquipment(qInfoMap) == true){
            return false;
        }
        break;
    default:
        break;
    }
    return true;
}



bool CardChargeFun::ProcOutApplyChargeResult(InfoMap &qInfoMap, InfoAddrType &InfoType)
{
    LogOut(QString("ProcApplyChargeResult!"), 1);
    CHARGE_STEP stTempChargeStep = stEmptyChargeStep;//存放总线解析后的数据
    CHARGE_STEP stChargeStep = stEmptyChargeStep;//存放状态机中已经有的数据
    InfoMap TempqInfoMap;
    InfoAddrType TempInfoType;
    int ApplyChargeResult = 0;

    ApplyChargeResult = ParseCardChargeSub(qInfoMap, stTempChargeStep);
    if(ApplyChargeResult == -1 || ApplyChargeResult == 0){
        return false;
    }
    if(CheckCanAddrValid(stTempChargeStep.ucCanAddr) == false){
        return false;
    }
    if(gpDevCache->QueryChargeStep(stTempChargeStep.ucCanAddr, stChargeStep) == false){//缓存中没有该终端状态机
        return false;
    }
    if(qInfoMap.contains(Addr_ScanCode_customerID) != 1)
    {
        if(strcmp(stTempChargeStep.sCardNo, stChargeStep.sCardNo) !=0){//卡号跟之前存放不一致
            LogOut(QString("服务器返回刷卡申请开始/结束充电结果,卡号不一致,已存在卡号:%1,新接收卡号:%2!")
                   .arg(QString(stChargeStep.sCardNo)) \
                   .arg(QString(stTempChargeStep.sCardNo)), 3);
    //        return false;
        }
    }
    else
    {
        if(strcmp(stTempChargeStep.sScanCodeNo, stChargeStep.sScanCodeNo) !=0){//扫码客户ID跟之前存放不一致
            LogOut(QString("服务器返回扫码customerID申请开始/结束充电结果,卡号不一致,已存在卡号:%1,新接收卡号:%2!")
                   .arg(QString(stChargeStep.sScanCodeNo)) \
                   .arg(QString(stTempChargeStep.sScanCodeNo)), 3);
    //        return false;
        }
    }

    switch (stChargeStep.CardSrcType) {
    case 1://单桩刷卡
        if(InfoType == AddrType_OutApplyStartChargeByChargeServic_Result){//服务器返回申请开始充电结果
            LogOut(QString("Send ApplyStartChargeResult to ChargeEquipment!"), 1);
            InfoType = AddrType_OutApplyStartChargeResult_ToChargeEquipment;
            UpdateChargeStepByRecvOutCardApplyStartChargeResult(stChargeStep, ApplyChargeResult);

        }
        else if( InfoType == AddrType_OutApplyStopChargeByChargeServic_Result){//服务器返回申请结束充电结果
            LogOut(QString("Send ApplyStopChargeResult to ChargeEquipment!"), 1);
            InfoType = AddrType_OutApplyStopChargeResult_ToChargeEquipment;
            UpdateChargeStepByRecvOutCardApplyStopChargeResult(stChargeStep, ApplyChargeResult);
        }
        break;
    case 2://集中刷卡
        if(InfoType == AddrType_OutApplyStartChargeByChargeServic_Result){
            LogOut(QString("Send ApplyStartChargeResult to Screen!"), 1);
            InfoType = AddrType_OutApplyStartChargeResult_ToScreen;
            UpdateChargeStepByRecvOutCardApplyStartChargeResult(stChargeStep, ApplyChargeResult);
        }
        else if(InfoType == AddrType_OutApplyStopChargeByChargeServic_Result){
            LogOut(QString("Send ApplyStopChargeResult to Screen!"), 1);
            InfoType = AddrType_OutApplyStopChargeResult_ToScreen;
            UpdateChargeStepByRecvOutCardApplyStopChargeResult(stChargeStep, ApplyChargeResult);
        }
        break;
    default://默认返回给集中刷卡
        if(InfoType == AddrType_OutApplyStartChargeByChargeServic_Result){
            LogOut(QString("Send ApplyStartChargeResult to default!"), 1);
            InfoType = AddrType_OutApplyStartChargeResult_ToScreen;
            UpdateChargeStepByRecvOutCardApplyStartChargeResult(stChargeStep, ApplyChargeResult);
        }
        else if(InfoType == AddrType_OutApplyStopChargeByChargeServic_Result){
            LogOut(QString("Send ApplyStopChargeResult to default!"), 1);
            InfoType = AddrType_OutApplyStopChargeResult_ToScreen;
            UpdateChargeStepByRecvOutCardApplyStopChargeResult(stChargeStep, ApplyChargeResult);
        }
        break;
    }

    TempqInfoMap = qInfoMap;
    TempInfoType = InfoType;
    emit sigToChargeService(TempqInfoMap, TempInfoType);
    return true;
}
