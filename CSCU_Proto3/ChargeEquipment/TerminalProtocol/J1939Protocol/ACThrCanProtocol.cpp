#include "ACThrCanProtocol.h"

cACThrCanProtocol::cACThrCanProtocol()
{
    uiBroadCastCount = 0;
    pTerminalSendList = new QList <can_frame *> ;//终端CAN帧发送列表
    pSendListMutex = new QMutex;//发送列表操作锁
    pModuleMap = new ModuleMap;    //CAN地址--长包模块
    pModuleMapMutex = new QMutex(); //长包模块操作锁
    pModuleMapToSend = new ModuleMap(); //准备发送长帧图(主动下发)
    pModuleSendMapMutex = new QMutex(); //发送长包操作锁
}

cACThrCanProtocol::~cACThrCanProtocol()
{
    delete pTerminalSendList ;//终端CAN帧发送列表
    delete pSendListMutex; //发送列表操作锁
    pModuleMap->clear();
    delete pModuleMap; //CAN地址--长包模块
    delete pModuleMapMutex; //长包模块操作锁
    delete pModuleMapToSend;//准备发送长帧图(主动下发)
    delete pModuleSendMapMutex; //发送长包操作锁
}

unsigned char cACThrCanProtocol::CheckLinkState(unsigned char ucLinkStateIn)
{
    unsigned char ucRet = 0;
    switch(ucLinkStateIn)
    {
    case 0:
        ucRet = Unlinked_Can;
        break;
    case 1:
        ucRet = EVEnsured_Can;//兼容交流三相在未确认时，平台可以下发允许充电。by FJC&Handi 2016-1-23
        break;
    case 2:
        ucRet = EVEnsured_Can;
        break;
    default:
        break;
    }
    return ucRet;
}

//解析遥调指令<----控制中心
void cACThrCanProtocol::ParseTeleadjust(InfoMap CenterMap, unsigned char ucTermID)
{
    ucTermID = 0;
    CenterMap.clear();
    Q_UNUSED(ucTermID);
}

//获取参数设置结果类型
int cACThrCanProtocol::GetParamAckType(unsigned char ucPF)
{
    int iRet = 0;
    switch(ucPF)
    {
    case PF_DetailParamSet_ACThr:
        iRet = AddrType_DetailParamSetResult_ACThr;
        break;
    default:
        iRet = AddrType_DetailParamSetResult_ACThr;
        break;
    }
    return iRet;
}

//获取协议版本号枚举
void cACThrCanProtocol::GetProVerEnum(unsigned char * pVer)
{
    if( (pVer[0] == 12)&&(pVer[1] == 0)&&(pVer[2] == 1) )
    {
        this->ucProVer = ProVer_1_0_12_AC_Thr;
    }
    else
    {
        this->ucProVer = ProVer_Old_AC_Thr;
    }
}

//是否有帧需要处理
bool cACThrCanProtocol::HasFrameToDeal()
{
    CheckBroadCastState();
    if(HasLongFrameToDeal() == TRUE)
    {
        return TRUE;
    }
    return FALSE;
}

void cACThrCanProtocol::CheckBroadCastState()
{
    //定时广播  1min
    if(uiBroadCastCount%(ucBroadCastInterval * 6) == 0)
    {
        for(int i = 0;i < ucACthree;i++)
        {
            SendFrameAppyPGN(i + 151,PF_SpecificInfo);
        }
        uiBroadCastCount = 0;
    }
    uiBroadCastCount++;
}

//接收CAN终端接收的数据并解析
void cACThrCanProtocol::ParseCanData(can_frame *pCanFrame)
{
    //传递给充电设备类图
//    CanInfoMap  ToCenterMap;
//    CanInfoTag  strCanInfoTag;
    QList <CanMapNode> ToCenterList;
    QByteArray CanAddrArray;
    //解析帧头
    FrameHead strFrameHead;
    memcpy((unsigned char *)&strFrameHead, (unsigned char *)&pCanFrame->can_id, sizeof(strFrameHead));

    //添加CAN地址节点
    CanAddrArray.append(strFrameHead.ucSa);
//    strCanInfoTag.ArrayData.append(strFrameHead.ucSa);
//    strCanInfoTag.enAddrType = AddrType_Common;
//    ToCenterMap.insert(Addr_CanID_Comm, strCanInfoTag);
    //解析其他内容
    switch(strFrameHead.ucPf)
    {
    case PF_ProVerInfo:
    {
        ParseFrameProVer(pCanFrame->data, strFrameHead.ucSa);
        break;
    }
    case PF_ChargerState1_ACThr:
    {
        ParseFrameState1(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_ChargerState2_ACThr:
    {
        ParseFrameState2(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_ChargerState3_ACThr:
    {
        ParseFrameState3(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_ChargerState4_ACThr:
    {
        ParseFrameState4(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_ChargerState5_ACThr:
    {
        ParseFrameState5(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_StopReason_ACThr:
    {
        ParseStopReason(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_ParamAck:
    {
        ParseFrameParamAck(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_CardNumber:
    {
        ParseFrameCardNum(pCanFrame->data, pCanFrame->can_dlc, ToCenterList);
        break;
    }
    case PF_CardApplyStartCharge:
    {
        ParseFrameCardStart(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_CardApplyStopCharge:
    {
        ParseFrameCardStop(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_BillingPolicy:
    {
        ParseFrameApplyAccount(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_ChargeManner_ACThr://单双枪充电
    {
       // CtrlCmdAck result =Ack_CmdAck;
//        ParseFrameChargeManner(pCanFrame, ToCenterList);    //屏蔽哈尔滨双枪逻辑解析add by zrx 2018-02-10
        //SendFrameAck(pCanFrame->can_id,result);//应答E8

         ParseFrameChargeManner(pCanFrame, ToCenterList);  //add by zrx
        break;
    }
    case PF_CtrlCmdAck:
    {
        ParseFrameCmdAck(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_UpdateRequestAck:
    {
        cLongPackageModule * pModule = CheckModule(strFrameHead.ucSa);
        if(pModule != NULL)
        {
            ParseFrameUpdateRequestAck(pCanFrame->data, pModule, ToCenterList);
        }
        break;
    }
    case PF_UpadateManage:
    {
        cLongPackageModule * pModule = CheckModule(strFrameHead.ucSa);
        if(pModule != NULL)
        {
            ParseFrameUpdateManage(pCanFrame->data, pModule);
        }
        break;
    }
    case PF_ProgramRecvFinsh:
    {
        ParseFrameProgramRecvFinsh(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_PackageRecvAck:
    {
        cLongPackageModule * pModule = CheckModule(strFrameHead.ucSa);
        if(pModule != NULL)
        {
            ParseFramePackageRecvAck(pCanFrame->data, pModule, ToCenterList);
        }
        break;
    }
    case PF_UpdateAck:
    {
        ParseFrameUpdateAck(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_LinkManage:
    {
        cLongPackageModule * pModule = CheckModule(strFrameHead.ucSa);
        if(pModule != NULL)
        {
            ParseFrameLinkMange(pCanFrame->data, pModule);
        }
        break;
    }
    case PF_DataTran:
    {
        cLongPackageModule * pModule = CheckModule(strFrameHead.ucSa);
        if(pModule != NULL)
        {
            //没有收到连接管理报文,则不接收处理
            if(pModule->ucPackTotalNum == 0)
            {
                pModule->bFreeFlag = TRUE;
                pModule->bUsedFlag = TRUE;
                pModule->bValidFlag = FALSE;
                break;
            }
            else
            {
                ParseFrameDataTran(pCanFrame->data, pModule);
            }
        }
        break;
    }
    default:
    {
        break;
    }
    }
    for(unsigned char i = 0 ; i < ToCenterList.count(); i++)
    {
        ToCenterList.at(i).stCanMap.insert(Addr_CanID_Comm, CanAddrArray);
        SendCenterData(ToCenterList.at(i).enType, ToCenterList.at(i).stCanMap);
    }
    //有数据发送
//    if(ToCenterMap.count() > 1)
//    {
//        SendCenterData(ToCenterMap);
//    }
}

//解析充电机状态1
void cACThrCanProtocol::ParseFrameState1(unsigned char * pData, QList<CanMapNode>  &ToCenterList)
{
    CanMapNode newNode;

    //定义相关数据
    float fAPhaseChargeVoltage;//A相充电电压
    float fBPhaseChargeVoltage;//B相充电电压
    float fCPhaseChargeVoltage;//C相充电电压
    float fAPhaseChargeCurrent;//A相充电电流

    //解帧
    FrameChargerState1_ACThr strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    fAPhaseChargeVoltage = (float)strFrame.sAPhaseChargeVoltage / 10.0;
    fBPhaseChargeVoltage = (float)strFrame.sBPhaseChargeVoltage / 10.0;
    fCPhaseChargeVoltage = (float)strFrame.sCPhaseChargeVoltage / 10.0;
    fAPhaseChargeCurrent = (float)(strFrame.sAPhaseChargeCurrent - 4000) / 10.0;

    //添加A相充电电压
    InsertCanInfoMapLine((char *)&fAPhaseChargeVoltage, sizeof(fAPhaseChargeVoltage), Addr_AVoltage_Term, newNode.stCanMap);
    //添加B相充电电压
    InsertCanInfoMapLine((char *)&fBPhaseChargeVoltage, sizeof(fBPhaseChargeVoltage), Addr_BVoltage_Term, newNode.stCanMap);
    //添加C相充电电压
    InsertCanInfoMapLine((char *)&fCPhaseChargeVoltage, sizeof(fCPhaseChargeVoltage), Addr_CVoltage_Term, newNode.stCanMap);
    //添加A相充电电流
    InsertCanInfoMapLine((char *)&fAPhaseChargeCurrent, sizeof(fAPhaseChargeCurrent), Addr_ACurrent_Term, newNode.stCanMap);

    newNode.enType = AddrType_TermMeasure;
    ToCenterList.append(newNode);
}

//解析充电机状态2
void cACThrCanProtocol::ParseFrameState2(unsigned char * pData, QList<CanMapNode>  &ToCenterList)
{
    CanMapNode newNode;

    //定义相关数据
    float fBPhaseChargeCurrent;//B相充电电流
    float fCPhaseChargeCurrent;//C相充电电流
    float fZeroLineCurrent;//零线充电电流
    float fTotalActivePower;//总有功功率

    //解帧
    FrameChargerState2_ACThr strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    fBPhaseChargeCurrent = (float)(strFrame.sBPhaseChargeCurrent - 4000) / 10.0;
    fCPhaseChargeCurrent = (float)( strFrame.sCPhaseChargeCurrent - 4000) / 10.0;
    fZeroLineCurrent = (float)(strFrame.usZeroLineCurrent - 4000) / 10.0;
    fTotalActivePower = (float)(strFrame.usTotalActivePower - 32000) / 100.0;//单位:kw, 精度0.01kw    //偏移处理  hd
    //添加A相充电电压
    InsertCanInfoMapLine((char *)&fBPhaseChargeCurrent, sizeof(fBPhaseChargeCurrent), Addr_BCurrent_Term, newNode.stCanMap);
    //添加B相充电电压
    InsertCanInfoMapLine((char *)&fCPhaseChargeCurrent, sizeof(fCPhaseChargeCurrent), Addr_CCurrent_Term, newNode.stCanMap);
    //添加C相充电电压
    InsertCanInfoMapLine((char *)&fZeroLineCurrent, sizeof(fZeroLineCurrent), Addr_ZeroLineCurrent_Term, newNode.stCanMap);
    //添加A相充电电流
    InsertCanInfoMapLine((char *)&fTotalActivePower, sizeof(fTotalActivePower), Addr_TotalActivePower_Term, newNode.stCanMap);

    newNode.enType = AddrType_TermMeasure;
    ToCenterList.append(newNode);
}

//解析充电机状态3
void cACThrCanProtocol::ParseFrameState3(unsigned char * pData, QList<CanMapNode>  &ToCenterList)
{
    CanMapNode newNode;

    //定义相关数据
    float fTotalReactivePower;//总无功功率
    float fTotalPowerFactor;//总f功率因数
    float fVoltageUnbalance;//电压不平衡率(预留)
    float fCurrentUnbalance;//电流不平衡率(预留)

    //解帧
    FrameChargerState3_ACThr strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    fTotalReactivePower = (float)(strFrame.usTotalReactivePower - 32000) / 100.0;
    fTotalPowerFactor = (float)(strFrame.usTotalPowerFactor -1000) / 1000.0;  //fix by zjq
    fVoltageUnbalance = strFrame.sVoltageUnbalance;
    fCurrentUnbalance = strFrame.sCurrentUnbalance;
    if(pData[7] ==0xff && pData[6] ==0xff)   //0xff会被处理成非0负数  hd
    {
        fCurrentUnbalance =0;
    }
    if(pData[5] ==0xff && pData[4] ==0xff)
    {
        fVoltageUnbalance =0;
    }

    //添加总无功功率
    InsertCanInfoMapLine((char *)&fTotalReactivePower, sizeof(fTotalReactivePower), Addr_TotalReactivePower_Term, newNode.stCanMap);
    //添加总功率因数
    InsertCanInfoMapLine((char *)&fTotalPowerFactor, sizeof(fTotalPowerFactor), Addr_TotalPowerFactor_Term, newNode.stCanMap);
    //添加电压不平衡率
    InsertCanInfoMapLine((char *)&fVoltageUnbalance, sizeof(fVoltageUnbalance), Addr_VoltageUnbalanceRate_Term, newNode.stCanMap);
    //添加电流不平衡率
    InsertCanInfoMapLine((char *)&fCurrentUnbalance, sizeof(fCurrentUnbalance), Addr_CurrentUnbalanceRate_Term, newNode.stCanMap);

    newNode.enType = AddrType_TermMeasure;
    ToCenterList.append(newNode);
}

//解析充电机状态4
void cACThrCanProtocol::ParseFrameState4(unsigned char * pData, QList<CanMapNode>  &ToCenterList)
{
    CanMapNode newNode;

    //定义相关数据
    uint uiTotalActiveEnergy;//总有功电能
    uint uiTotalReactiveEnergy;//总无功电能

    //解帧
    FrameChargerState4_ACThr strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    uiTotalActiveEnergy = strFrame.uiTotalActiveEnergy;
    uiTotalReactiveEnergy = strFrame.uiTotalReactiveEnergy;

    //添加总有功电能
    InsertCanInfoMapLine((char *)&uiTotalActiveEnergy, sizeof(uiTotalActiveEnergy), Addr_TotalActiveEnergy_Term, newNode.stCanMap);
    //添加总无功电能
    InsertCanInfoMapLine((char *)&uiTotalReactiveEnergy, sizeof(uiTotalReactiveEnergy), Addr_TotalReactiveEnergy_Term, newNode.stCanMap);

    newNode.enType = AddrType_TermMeasure;
    ToCenterList.append(newNode);
}

//解析充电机状态5
void cACThrCanProtocol::ParseFrameState5(unsigned char * pData, QList<CanMapNode>  &ToCenterList)
{
    CanMapNode newNode;
    //定义相关数据
    char cFaultInfo;	//故障状态	1欠压故障/2过压故障/3过流故障
    char cWorkState;//工作状态
    float fChargeTime;//充电时间
    char cInterFaceType;	//充电接口标识
    char cLinkState;	//连接确认开关状态 0关 1开
    char cRelyState;	//输出继电器状态   0关  1开


    //解帧
    FrameChargerState5_ACThr strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    cFaultInfo = (char)strFrame.usFaultInfo;
    cWorkState = (char)strFrame.usWorkState;
    fChargeTime = strFrame.usChargeTime;
    cInterFaceType = strFrame.stByte7.ucInterFaceType;
    cLinkState = CheckLinkState(strFrame.stByte7.ucLinkState);
    cRelyState = strFrame.stByte7.ucRelyState;

    //添加故障状态
    InsertCanInfoMapLine(&cFaultInfo, sizeof(cFaultInfo), Addr_FaultCode_Term, newNode.stCanMap);
    //添加工作状态
    InsertCanInfoMapLine(&cWorkState, sizeof(cWorkState), Addr_WorkState_Term, newNode.stCanMap);
    //添加充电接口标识
    InsertCanInfoMapLine(&cInterFaceType, sizeof(cInterFaceType), Addr_ChargeGunNum_Term, newNode.stCanMap);
    //连接确认开关状态
    InsertCanInfoMapLine(&cLinkState, sizeof(cLinkState), Addr_LinkState_Term, newNode.stCanMap);
    //输出继电器状态
    InsertCanInfoMapLine(&cRelyState, sizeof(cRelyState), Addr_RelyState_Term, newNode.stCanMap);

    newNode.enType = AddrType_TermSignal;
    ToCenterList.append(newNode);

    newNode.stCanMap.clear();
    //添加充电时间
    InsertCanInfoMapLine((char *)&fChargeTime, sizeof(fChargeTime), Addr_ChargeTime_Term, newNode.stCanMap);
    newNode.enType = AddrType_TermMeasure;
    ToCenterList.append(newNode);
}
//解析充电中止原因
void cACThrCanProtocol::ParseStopReason(unsigned char * pData, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;

    //解帧
    FrameStopReason_ACThr strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    //添加充电中止原因
    InsertCanInfoMapLine((char *)&strFrame.ucCode, sizeof(strFrame.ucCode), Addr_ChargeEndCode_Term, newNode.stCanMap);
    newNode.enType = AddrType_TermSignal;
    ToCenterList.append(newNode);
}

//校验单双枪充电信息  屏蔽哈尔滨双枪逻辑解析add by zrx 2018-02-10
//bool cACThrCanProtocol::CheckChargeMannerInfo(FrameChargerMannerInfo_ACThr strFrame, unsigned char canID)
//{
//    if(strFrame.chargeManner == SINGLE_CHARGE)//单枪充电
//    {//校验主枪与源地址是否相同
//        if(strFrame.canID_master == canID)
//        {
//            return true;
//        }
//    }
//    else if(strFrame.chargeManner == COUPLE_CHARGE)//双枪充电
//    {//校验主副枪是否相邻
//         if(qAbs(strFrame.canID_master - strFrame.canID_slave) == 1)
//             return true;
//    }

//    return false;
//}

//解析充电方式(单枪充电／双枪充电)信息帧  屏蔽哈尔滨双枪逻辑解析add by zrx 2018-02-10
//CtrlCmdAck  cACThrCanProtocol::ParseFrameChargeManner(can_frame *srcFrame, QList<CanMapNode>  &ToCenterList)
//{
//    CanMapNode newNode;
//    CtrlCmdAck result =Ack_CmdAck;

//    //解帧
//    FrameChargerMannerInfo_ACThr strFrame;
//    unsigned char canID = srcFrame->can_id & 0xFF;
//    memcpy((unsigned char *)&strFrame, srcFrame->data, sizeof(strFrame));
//    if(CheckChargeMannerInfo(strFrame,canID))
//    {
//        newNode.enType = AddrType_CheckChargeManner_Success;
//        result = Ack_CmdAck;
//    }
//    else
//    {
//        newNode.enType = AddrType_CheckChargeManner;
//         result = Ack_CmdNAck;
//    }

//    //添加单双枪充电方式
//    InsertCanInfoMapLine((char *)&strFrame.chargeManner, sizeof(strFrame.chargeManner), Addr_DCChargeManner_Term, newNode.stCanMap);
//    //添加主副枪CAN地址
//    InsertCanInfoMapLine((char *)&strFrame.canID_master, sizeof(strFrame.canID_master), Addr_DCChargeMasterCANID_Term, newNode.stCanMap);
//     InsertCanInfoMapLine((char *)&strFrame.canID_slave, sizeof(strFrame.canID_slave), Addr_DCChargeSlaveCANID_Term, newNode.stCanMap);

//    ToCenterList.append(newNode);
//    return result;
//}

//解析充电方式(单枪充电／双枪充电)信息帧
void cACThrCanProtocol::ParseFrameChargeManner(can_frame *srcFrame, QList<CanMapNode>  &ToCenterList)
{
    CanMapNode newNode;
   // CtrlCmdAck result = Ack_CmdAck;

    //解帧
    FrameChargerMannerInfo_ACThr strFrame;
    //unsigned char canID = srcFrame->can_id & 0xFF;
    memcpy((unsigned char *)&strFrame, srcFrame->data, sizeof(strFrame));
    //if(CheckChargeMannerInfo(strFrame,canID))
    //{
     //   newNode.enType = AddrType_CheckChargeManner_Success;
    //    result = Ack_CmdAck;
  //  }
   // else
  //  {
        newNode.enType = AddrType_CheckChargeManner;
    //    result = Ack_CmdNAck;
   // }
    //添加单双枪充电方式
    InsertCanInfoMapLine((char *)&strFrame.chargeManner, sizeof(strFrame.chargeManner), Addr_ChargeGunType, newNode.stCanMap);
    //添加主副枪CAN地址
    InsertCanInfoMapLine((char *)&strFrame.canID_master, sizeof(strFrame.canID_master),Addr_ChargeGun_Master , newNode.stCanMap);
    InsertCanInfoMapLine((char *)&strFrame.canID_slave1, sizeof(strFrame.canID_slave1), Addr_ChargeGun_Slave1, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&strFrame.canID_slave2, sizeof(strFrame.canID_slave2), Addr_ChargeGun_Slave2, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&strFrame.canID_slave3, sizeof(strFrame.canID_slave3), Addr_ChargeGun_Slave3, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&strFrame.canID_slave4, sizeof(strFrame.canID_slave4), Addr_ChargeGun_Slave4, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&strFrame.canID_slave5, sizeof(strFrame.canID_slave5), Addr_ChargeGun_Slave5, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&strFrame.canID_slave6, sizeof(strFrame.canID_slave6), Addr_ChargeGun_Slave6, newNode.stCanMap);

    ToCenterList.append(newNode);
   // return result;
}

//解析主动防护设置<----控制中心
void cACThrCanProtocol::ParseActiveProtect(InfoMap CenterMap, unsigned char ucTermID)
{
    if(!CenterMap.isEmpty())
    {
        if(ucTermID!=0)
        {
            ;
        }
    }
}

//解析柔性充电设置<----控制中心
void cACThrCanProtocol::ParseFlexibleCharge(InfoMap CenterMap, unsigned char ucTermID)
{
    if(!CenterMap.isEmpty())
    {
        if(ucTermID!=0)
        {
            ;
        }
    }
}

//解析通用静态参数设置<----控制中心
void cACThrCanProtocol::ParseGeneralStaticArg(InfoMap CenterMap, unsigned char ucTermID)
{
    if(!CenterMap.isEmpty())
    {
        if(ucTermID!=0)
        {
            ;
        }
    }
}

//解析通用动态参数设置<----控制中心
void cACThrCanProtocol::ParseGeneralDynamicArg(InfoMap CenterMap, unsigned char ucTermID)
{
    if(!CenterMap.isEmpty())
    {
        if(ucTermID!=0)
        {
            ;
        }
    }
}


//解析查询主动防护设置<----控制中心
void cACThrCanProtocol::ParseQueryActiveProtect(InfoMap CenterMap, unsigned char ucTermID)
{
    if((!CenterMap.isEmpty())||(ucTermID!=0))
    {
        ;
    }
}

//解析查询柔性充电设置<----控制中心
void cACThrCanProtocol::ParseQueryFlexibleCharge(InfoMap CenterMap, unsigned char ucTermID)
{
    if((!CenterMap.isEmpty())||(ucTermID!=0))
    {
        ;
    }
}

//解析查询通用静态参数设置<----控制中心
void cACThrCanProtocol::ParseQueryGeneralStaticArg(InfoMap CenterMap, unsigned char ucTermID)
{
    if((!CenterMap.isEmpty())||(ucTermID!=0))
    {
        ;
    }
}

//解析查询通用动态参数设置<----控制中心
void cACThrCanProtocol::ParseQueryGeneralDynamicArg(InfoMap CenterMap, unsigned char ucTermID)
{
    if((!CenterMap.isEmpty())||(ucTermID!=0))
    {
        ;
    }
}

//解析CCU参数设置<----控制中心
void cACThrCanProtocol::ParseCCUArg(InfoMap CenterMap, unsigned char ucTermID)
{
    if((!CenterMap.isEmpty())||(ucTermID!=0))
    {
        ;
    }
}

//解析CCU参数查询<----控制中心
void cACThrCanProtocol::ParseCCUArgQuery(InfoMap CenterMap, unsigned char ucTermID)
{
    if((!CenterMap.isEmpty())||(ucTermID!=0))
    {
        ;
    }
}

//解析能效系统模块控制<----控制中心
void cACThrCanProtocol::ParseCtrlModule_EP(InfoMap CenterMap, unsigned char ucTermID)
{
    if((!CenterMap.isEmpty())||(ucTermID!=0))
    {
        ;
    }
}

//解析能效系统功率控制<----控制中心
void cACThrCanProtocol::ParseSetPower_EP(InfoMap CenterMap, unsigned char ucTermID)
{
    if((!CenterMap.isEmpty())||(ucTermID!=0))
    {
        ;
    }
}

//解析长帧
void cACThrCanProtocol::ParseLongFrame(FrameLongPackage * pLongPackage)
{
    QList <CanMapNode> ToCenterList;
    QByteArray CanAddrArray;
    CanAddrArray.append(pLongPackage->ucTermID);
    //传递给充电设备类图

    switch(pLongPackage->ucPGN[1])
    {
    case PF_SpecificInfo:
        ParseSpecificInfo(pLongPackage, ToCenterList);
        break;
    default:
        break;
    }

    //有数据发送
    for(unsigned char i = 0; i < ToCenterList.count(); i++)
    {
        ToCenterList.at(i).stCanMap.insert(Addr_CanID_Comm, CanAddrArray);
        SendCenterData(ToCenterList.at(i).enType, ToCenterList.at(i).stCanMap);
    }
}
