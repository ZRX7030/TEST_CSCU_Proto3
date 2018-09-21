#include "EnergyPlanProtocol.h"

cEnergyPlanProtocol::cEnergyPlanProtocol()
{

    pTerminalSendList = new QList <can_frame *> ;//终端CAN帧发送列表
    pSendListMutex = new QMutex();//发送列表操作锁
    pModuleMap = new ModuleMap();    //CAN地址--长包模块
    pModuleMapToSend = new ModuleMap(); //准备发送长帧图(主动下发)
    pModuleMapMutex = new QMutex(); //长包模块操作锁
    pModuleSendMapMutex = new QMutex(); //发送长包操作锁
}

cEnergyPlanProtocol::~cEnergyPlanProtocol()
{
    delete pTerminalSendList ;//终端CAN帧发送列表
    delete pSendListMutex; //发送列表操作锁
    pModuleMap->clear();
    delete pModuleMap; //CAN地址--长包模块
    delete pModuleMapMutex; //长包模块操作锁
    delete pModuleMapToSend;
    delete pModuleSendMapMutex;
}

//获取协议版本号枚举
void cEnergyPlanProtocol::GetProVerEnum(unsigned char * pVer)
{
    if( (pVer[0] == 6)&&(pVer[1] == 1)&&(pVer[2] == 1) )
    {
        this->ucProVer = ProVer_Old_Energy_Plan;
    }
    else
    {
        this->ucProVer = ProVer_1_0_0_Energy_Plan;
    }
}
//获取参数设置结果类型
int cEnergyPlanProtocol::GetParamAckType(unsigned char ucPF)
{
    int iRet = 0;
    switch(ucPF)
    {
    default:
        iRet = 0;
        break;
    }
    return iRet;
}

//解析遥调指令<----控制中心
void cEnergyPlanProtocol::ParseTeleadjust(InfoMap CenterMap, unsigned char ucTermID)
{
    Q_UNUSED(ucTermID);
    InfoMap::iterator itTarget;
    QMap <unsigned char, can_frame*> PfMap;
    QMap <unsigned char, can_frame*>::iterator itPfMap;
    //解析Map内容
    for(itTarget = CenterMap.begin(); itTarget != CenterMap.end(); itTarget++)
    {
        switch (itTarget.key())
        {
        default:
            break;
        }
    }
    //发送遥调指令
    //解析Map内容
    pSendListMutex->lock();
    for(itPfMap = PfMap.begin(); itPfMap != PfMap.end(); ++itPfMap)
    {
        pTerminalSendList->append(itPfMap.value());
    }
    pSendListMutex->unlock();
}

//是否有帧需要处理
bool cEnergyPlanProtocol::HasFrameToDeal()
{
    if(HasLongFrameToDeal() == TRUE)
    {
        return TRUE;
    }
    return FALSE;
}

//接收CAN终端接收的数据并解析
void cEnergyPlanProtocol::ParseCanData(can_frame *pCanFrame)
{
    //传递给充电设备类图
    QList <CanMapNode> ToCenterList;
    QByteArray CanAddrArray;
    //解析帧头
    FrameHead strFrameHead;
    memcpy((unsigned char *)&strFrameHead, (unsigned char *)&pCanFrame->can_id, sizeof(strFrameHead));
    //添加CAN地址节点
    CanAddrArray.append(strFrameHead.ucSa);
    //解析其他内容
//    if(strFrameHead.ucPf>=0x52 &&strFrameHead.ucPf<=0x54)
    switch(strFrameHead.ucPf)
    {
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
        break;
    }
    case PF_CardApplyStopCharge:
    {
        break;
    }
    case PF_BillingPolicy:
    {
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
        ParseFrameUpdateRequestAck(pCanFrame->data, pModule, ToCenterList);
        break;
    }
    case PF_UpadateManage:
    {
        cLongPackageModule * pModule = CheckModule(strFrameHead.ucSa);
        ParseFrameUpdateManage(pCanFrame->data, pModule);
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
        ParseFramePackageRecvAck(pCanFrame->data, pModule, ToCenterList);
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
        ParseFrameLinkMange(pCanFrame->data, pModule);
        break;
    }
    case PF_DataTran:
    {
        cLongPackageModule * pModule = CheckModule(strFrameHead.ucSa);
        if(pModule != NULL)
        {
//            没有收到连接管理报文,则不接收处理
            if(pModule->ucPackTotalNum == 0)
            {
                pModule->bFreeFlag = TRUE;
                pModule->bUsedFlag = TRUE;
                pModule->bValidFlag = FALSE;
            }
            else
            {
                ParseFrameDataTran(pCanFrame->data, pModule);
            }
        }
        break;
    }
        //能效计划
    case PF_ModuleFaultInfo_EP:
    {
        ParseFrameModuleFaultInfo(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_FourQuadrantCabinet1://ACDC四象限柜信息１
        ParseFourQuadrantCabinetInfo1(pCanFrame->data, ToCenterList,PF_FourQuadrantCabinet1,strFrameHead.ucSa);
        break;
    case PF_ACDCInverterCabinet://ACDC独立逆变柜子信息
        ParseACDCInverterCabinetInfo(pCanFrame->data, ToCenterList, PF_ACDCInverterCabinet,strFrameHead.ucSa);
          break;
      case PF_EnergyStorageCabinetInfo1://储能柜信息１
          ParseEnergyStorageCabinetInfo(pCanFrame->data, ToCenterList,PF_EnergyStorageCabinetInfo1,strFrameHead.ucSa);
          break;
      case PF_EnergyStorageCabinetInfo2://储能柜信息２
          break;
      case PF_EnergyStorageCabinetCmdSet://储能柜命令设置，预留
          break;
      case PF_PhotoVoltaicCabinetInfo1://光伏柜信息１
          ParsePhotoVoltaicCabinetInfo1(pCanFrame->data, ToCenterList,PF_PhotoVoltaicCabinetInfo1,strFrameHead.ucSa);
          break;
      case PF_PhotoVoltaicCabinetInfo2://光伏柜信息２
          break;
      case  PF_ChargeDischargeCabinetInfo1://充放电柜信息１
          ParseChargeDischargeCabinetInfo1(pCanFrame->data, ToCenterList, PF_ChargeDischargeCabinetInfo1,strFrameHead.ucSa);
          break;
      case  PF_ChargeDischargeCabinetInfo2://充放电柜信息２
          break;
      case PF_SysControlCabinetInfo1://系统控制柜信息１
          ParseSysControlCabinetInfo1(pCanFrame->data, ToCenterList,PF_SysControlCabinetInfo1,strFrameHead.ucSa);
          break;
      case PF_SysControlCabinetInfo2://系统控制柜信息２
          ParseSysControlCabinetInfo2(pCanFrame->data, ToCenterList,PF_SysControlCabinetInfo1,strFrameHead.ucSa);
          break;
      case PF_ElectircCloset1://总配电柜信息１
          ParseMainDistributionCabinetInfo1(pCanFrame->data, ToCenterList, PF_ElectircCloset1,strFrameHead.ucSa);
          break;
      case PF_BothwayPDUInfo1://双向PDU信息１
      case PF_BothwayPDUInfo2://双向PDU信息２
          break;

      case PF_EnergyStorageCabinetBatteryInfo2://储能电池信息２
          break;
      case PF_PowerOptimizerCabinetInfo2://功率优化器信息２
          break;
      case PF_ModuleRePowerControl://四象限柜无功率设置
          break;
      case PF_HygrothermographInfo1://温湿度传感器信息１
          ParseHumitureInfo1(pCanFrame->data, ToCenterList,PF_HygrothermographInfo1,strFrameHead.ucSa);
          break;
      case PF_HygrothermographInfo2://温湿度传感器信息２
          break;
    case PF_EMSHumityTemp1:
        ParseEMSHumitureInfo1(pCanFrame->data, ToCenterList,PF_HygrothermographInfo1,strFrameHead.ucSa);
        break;
    case PF_EMSHumityTemp2:
        ParseEMSHumitureInfo2(pCanFrame->data, ToCenterList,PF_HygrothermographInfo1,strFrameHead.ucSa);
        break;
    case PF_EMSSwitchPoint:
        ParseEMSSwitchPointInfo(pCanFrame->data, ToCenterList,PF_HygrothermographInfo1,strFrameHead.ucSa);
        break;
    default:
        break;
    }
    //有数据发送
    for(unsigned char i = 0 ; i < ToCenterList.count(); i++)
    {
        ToCenterList.at(i).stCanMap.insert(Addr_CanID_Comm, CanAddrArray);
        SendCenterData(ToCenterList.at(i).enType, ToCenterList.at(i).stCanMap);
    }
}

//解析主动防护设置<----控制中心
void cEnergyPlanProtocol::ParseActiveProtect(InfoMap CenterMap, unsigned char ucTermID)
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
void cEnergyPlanProtocol::ParseFlexibleCharge(InfoMap CenterMap, unsigned char ucTermID)
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
void cEnergyPlanProtocol::ParseGeneralStaticArg(InfoMap CenterMap, unsigned char ucTermID)
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
void cEnergyPlanProtocol::ParseGeneralDynamicArg(InfoMap CenterMap, unsigned char ucTermID)
{
    if((!CenterMap.isEmpty())||(ucTermID!=0))
    {
        ;
    }
}



//解析查询主动防护设置<----控制中心
void cEnergyPlanProtocol::ParseQueryActiveProtect(InfoMap CenterMap, unsigned char ucTermID)
{
    if((!CenterMap.isEmpty())||(ucTermID!=0))
    {
        ;
    }
}

//解析查询柔性充电设置<----控制中心
void cEnergyPlanProtocol::ParseQueryFlexibleCharge(InfoMap CenterMap, unsigned char ucTermID)
{
    if((!CenterMap.isEmpty())||(ucTermID!=0))
    {
        ;
    }
}

//解析查询通用静态参数设置<----控制中心
void cEnergyPlanProtocol::ParseQueryGeneralStaticArg(InfoMap CenterMap, unsigned char ucTermID)
{
    if((!CenterMap.isEmpty())||(ucTermID!=0))
    {
        ;
    }
}
//解析查询通用动态参数设置<----控制中心
void cEnergyPlanProtocol::ParseQueryGeneralDynamicArg(InfoMap CenterMap, unsigned char ucTermID)
{
    if((!CenterMap.isEmpty())||(ucTermID!=0))
    {
        ;
    }
}

//解析CCU参数设置<----控制中心
void cEnergyPlanProtocol::ParseCCUArg(InfoMap CenterMap, unsigned char ucTermID)
{
    if((!CenterMap.isEmpty())||(ucTermID!=0))
    {
        ;
    }
}

//解析CCU参数查询<----控制中心
void cEnergyPlanProtocol::ParseCCUArgQuery(InfoMap CenterMap, unsigned char ucTermID)
{
    if((!CenterMap.isEmpty())||(ucTermID!=0))
    {
        ;
    }
}

void cEnergyPlanProtocol::MakeFrameCtrlModuleWork(InfoMap &CenterMap, can_frame *pFrame, unsigned char ucCanID)
{
    FrameCtrlModuleWork stFrame;
    memset(&stFrame, 0x00, sizeof(stFrame));

    stFrame.ucInnerID = CenterMap[Addr_DevID_DC_Comm].at(0);
    stFrame.ucCmd = CenterMap[Addr_Mod_Work_Ctrl].at(0);
    memcpy((char *)pFrame->data, &stFrame, sizeof(stFrame));

    SetFrameHead(pFrame, ID_DefaultControlCenterCanID, PF_CtrlModule, ucCanID, PriorityDefault, sizeof(stFrame));

}
void cEnergyPlanProtocol::MakeFrameSetModulePower(InfoMap &CenterMap, can_frame *pFrame, unsigned char ucCanID)
{
    FrameSetModulePower stFrame;
    float fPower = 0;
    memset(&stFrame, 0x00, sizeof(stFrame));

    ucCanID = CenterMap[Addr_CanID_Comm].at(0);
    fPower = *(float *)CenterMap[Addr_Mod_Power_Set].data();

    stFrame.ucInnerID = CenterMap[Addr_DevID_DC_Comm].at(0);
    stFrame.sPower = fPower * 100;
    memcpy((char *)pFrame->data, &stFrame, sizeof(stFrame));

    SetFrameHead(pFrame, ID_DefaultControlCenterCanID, PF_SetModulePower, ucCanID, PriorityDefault, sizeof(stFrame));

}

void cEnergyPlanProtocol::SendFrameCtrlModuleWork(InfoMap CenterMap, unsigned char ucTermID)
{
    can_frame *pCanFrame = new(can_frame);
    MakeFrameCtrlModuleWork(CenterMap, pCanFrame, ucTermID);
    pSendListMutex->lock();
    pTerminalSendList->append(pCanFrame);
    pSendListMutex->unlock();
}

void cEnergyPlanProtocol::SendFrameSetModulePower(InfoMap CenterMap, unsigned char ucTermID)
{
    can_frame *pCanFrame = new(can_frame);
    MakeFrameSetModulePower(CenterMap, pCanFrame, ucTermID);
    pSendListMutex->lock();
    pTerminalSendList->append(pCanFrame);
    pSendListMutex->unlock();
}

/*能效计划新增信息解析*/

//解析EMS节点信息
void cEnergyPlanProtocol::ParseEMSSwitchPointInfo(unsigned char * pData, QList<CanMapNode> &ToCenterList, char pgnID,char canAddr)
{
    CanMapNode newNode;
    //解帧
    FrameEMSSwitchPointInfo strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    char smokeSensor_lowVolIn = strFrame.stStatus.smokeSensor_lowVolIn;
    char frameFeedback = strFrame.stStatus.frameFeedback;
    char minorLoadbreaker_630A1 = strFrame.stStatus.minorLoadbreaker_630A1;
    char minorLoadbreaker_630A2 = strFrame.stStatus.minorLoadbreaker_630A2;
    char smokeSensor_lowVolOut = strFrame.stStatus.smokeSensor_lowVolOut;
    char minorLoadbreaker_400A1 = strFrame.stStatus.minorLoadbreaker_400A1;
    char minorLoadbreaker_400A2 = strFrame.stStatus.minorLoadbreaker_400A2;
    char minorLoadbreaker_400A3 = strFrame.stStatus.minorLoadbreaker_400A3;
    char acdcBreaker = strFrame.stStatus.acdcBreaker;
    char importBreaker1 = strFrame.stStatus.importBreaker1;
    char importBreaker2 = strFrame.stStatus.importBreaker2;
    char emergncyStop = strFrame.stStatus.emergncyStop;
    char mainModuleSwitchCMD = strFrame.stStatus.mainModuleSwitchCMD;
    char mainLoad_ACDCModuleSwitchCMD = strFrame.stStatus.mainLoad_ACDCModuleSwitchCMD;
    char storageUnit1_DCDCModuleSwitchCMD = strFrame.stStatus.storageUnit1_DCDCModuleSwitchCMD;
    char storageUnit2_DCDCModuleSwitchCMD = strFrame.stStatus.storageUnit2_DCDCModuleSwitchCMD;
    char gird_state = strFrame.stStatus.gird_state;

    InsertCanInfoMapLine((char *)&smokeSensor_lowVolIn, 1, Addr_smokeSensor_lowVolIn, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&frameFeedback, 1, Addr_frameFeedback, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&minorLoadbreaker_630A1, 1, Addr_minorLoadbreaker_630A1, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&minorLoadbreaker_630A2, 1, Addr_minorLoadbreaker_630A2, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&smokeSensor_lowVolOut, 1, Addr_smokeSensor_lowVolOut, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&minorLoadbreaker_400A1, 1, Addr_minorLoadbreaker_400A1, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&minorLoadbreaker_400A2, 1, Addr_minorLoadbreaker_400A2, newNode.stCanMap);

    InsertCanInfoMapLine((char *)&minorLoadbreaker_400A3, 1, Addr_minorLoadbreaker_400A3, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&acdcBreaker, 1, Addr_acdcBreaker, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&importBreaker1, 1, Addr_importBreaker1, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&importBreaker2, 1, Addr_importBreaker2, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&emergncyStop, 1, Addr_emergncyStop, newNode.stCanMap);

    InsertCanInfoMapLine((char *)&mainModuleSwitchCMD, 1, Addr_mainModuleSwitchCMD, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&mainLoad_ACDCModuleSwitchCMD, 1, Addr_mainLoad_ACDCModuleSwitchCMD, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&storageUnit1_DCDCModuleSwitchCMD, 1, Addr_storageUnit1_DCDCModuleSwitchCMD, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&storageUnit2_DCDCModuleSwitchCMD, 1, Addr_storageUnit2_DCDCModuleSwitchCMD, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&gird_state, 1, Addr_Grid_State_EMS, newNode.stCanMap);

    InsertCanInfoMapLine((char *)&pgnID, 1, Addr_PGN_ID, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&canAddr, 1, Addr_CanID_Comm, newNode.stCanMap);

    newNode.enType = AddrType_EMS_Info;
    ToCenterList.append(newNode);
}
//解析ACDC四象限柜子信息1
void cEnergyPlanProtocol::ParseFourQuadrantCabinetInfo1(unsigned char * pData, QList<CanMapNode> &ToCenterList, char pgnID,char canAddr)
{
    CanMapNode newNode;
    //解帧
    FrameFourQuadrantCabinetInfo1 strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    char ACbreaker_Cabinet2 = strFrame.stStatus.ACbreaker_Cabinet2;
    char DCbreaker_Cabinet2 = strFrame.stStatus.DCbreaker_Cabinet2;
    char ACbreaker_Cabinet3 = strFrame.stStatus.ACbreaker_Cabinet3;
    char DCbreaker_Cabinet3 = strFrame.stStatus.DCbreaker_Cabinet3;
    char surgeFeedback_Cabinet2 = strFrame.stStatus.surgeFeedback_Cabinet2;
    char fireExtinguisher_Cabinet2 = strFrame.stStatus.fireExtinguisher_Cabinet2;
    char fireExtinguisher_Cabinet3 = strFrame.stStatus.fireExtinguisher_Cabinet3;

    InsertCanInfoMapLine((char *)&ACbreaker_Cabinet2, 1, Addr_ACbreaker_Cabinet2, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&DCbreaker_Cabinet2, 1, Addr_DCbreaker_Cabinet2, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&ACbreaker_Cabinet3, 1, Addr_ACbreaker_Cabinet3, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&DCbreaker_Cabinet3, 1, Addr_DCbreaker_Cabinet3, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&surgeFeedback_Cabinet2, 1, Addr_surgeFeedback_Cabinet2, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&fireExtinguisher_Cabinet2, 1, Addr_fireExtinguisher_Cabinet2, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&fireExtinguisher_Cabinet3, 1, Addr_fireExtinguisher_Cabinet3, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&pgnID, 1, Addr_PGN_ID, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&canAddr, 1, Addr_CanID_Comm, newNode.stCanMap);

    newNode.enType = AddrType_FQ_Cab_Info;
    ToCenterList.append(newNode);
}
//解析ACDC独立逆变柜子信息
void cEnergyPlanProtocol::ParseACDCInverterCabinetInfo(unsigned char * pData, QList<CanMapNode> &ToCenterList, char pgnID,char canAddr)
{
    CanMapNode newNode;
    //解帧
    FrameACDCInverterCabinetInfo strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));
    char ACbreaker = strFrame.stStatus.ACbreaker;
    char DCbreaker = strFrame.stStatus.DCbreaker;
    char surgeFeedback_Cabinet2 = strFrame.stStatus.surgeFeedback_Cabinet2;
    char fireExtinguisher_Cabinet3 = strFrame.stStatus.fireExtinguisher_Cabinet3;

    InsertCanInfoMapLine((char *)&ACbreaker, 1, Addr_ACbreaker_Inverter, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&DCbreaker, 1, Addr_DCbreaker_Inverter, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&surgeFeedback_Cabinet2, 1, Addr_surgeFeedback_Cabinet2_Inverter, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&fireExtinguisher_Cabinet3, 1, Addr_fireExtinguisher_Cabinet3_Inverter, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&pgnID, 1, Addr_PGN_ID, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&canAddr, 1, Addr_CanID_Comm, newNode.stCanMap);

    newNode.enType = AddrType_SI_Cab_Info;
    ToCenterList.append(newNode);
}
//储能柜信息1
//void cEnergyPlanProtocol::ParseEnergyStorageCabinetInfo1(unsigned char * pData, QList<CanMapNode> &ToCenterList, char pgnID,char canAddr)
//{
//    CanMapNode newNode;
//    //解帧
//    FrameEnergyStorageCabinetInfo1 strFrame;
//    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

//    char DCbreaker = strFrame.stStatus.DCBreaker;
//    char fireExtinguisher = strFrame.stStatus.fireStop;
//    char runStatus = strFrame.stStatus.runStatus;

//    InsertCanInfoMapLine((char *)&DCbreaker, 1, Addr_DCbreaker, newNode.stCanMap);
//    InsertCanInfoMapLine((char *)&fireExtinguisher, 1, Addr_fireExtinguisher1, newNode.stCanMap);
//    InsertCanInfoMapLine((char *)&runStatus, 1, Addr_chargeDischargeMode, newNode.stCanMap);

//    InsertCanInfoMapLine((char *)&pgnID, 1, Addr_PGN_ID, newNode.stCanMap);
//    InsertCanInfoMapLine((char *)&canAddr, 1, Addr_CanID_Comm, newNode.stCanMap);
//    newNode.enType = AddrType_EnergyPlanEnvSignal;
//    ToCenterList.append(newNode);
//}
//解析光伏柜信息1
void cEnergyPlanProtocol::ParsePhotoVoltaicCabinetInfo1(unsigned char * pData, QList<CanMapNode> &ToCenterList, char pgnID,char canAddr)
{
    CanMapNode newNode;
    //解帧
    FramePhotoVoltaicCabinetInfo1 strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    char smoke = strFrame.stStatus.smokeSensor;
    char breaker1 = strFrame.stStatus.breaker1;
    char breaker2 = strFrame.stStatus.breaker2;
    char breaker3 = strFrame.stStatus.breaker3;
    char breaker4 = strFrame.stStatus.breaker4;
    char breaker5 = strFrame.stStatus.breaker5;
    char breaker6 = strFrame.stStatus.breaker6;
    char breaker7 = strFrame.stStatus.breaker7;

    InsertCanInfoMapLine((char *)&smoke, 1, Addr_fireExtinguisherVoltaicCabinet1, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&breaker1, 1, Addr_breaker1VoltaicCabinet1, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&breaker2, 1, Addr_breaker2VoltaicCabinet1, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&breaker3, 1, Addr_breaker3VoltaicCabinet1, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&breaker4, 1, Addr_breaker4VoltaicCabinet1, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&breaker5, 1, Addr_breaker5VoltaicCabinet1, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&breaker6, 1, Addr_breaker6VoltaicCabinet1, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&breaker7, 1, Addr_breaker7VoltaicCabinet1, newNode.stCanMap);

    InsertCanInfoMapLine((char *)&pgnID, 1, Addr_PGN_ID, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&canAddr, 1, Addr_CanID_Comm, newNode.stCanMap);
    newNode.enType = AddrType_PH_Cab_Info;
    ToCenterList.append(newNode);
}
//解析充放电柜信息１
void cEnergyPlanProtocol::ParseChargeDischargeCabinetInfo1(unsigned char * pData, QList<CanMapNode> &ToCenterList, char pgnID,char canAddr)
{
    CanMapNode newNode;
    //解帧
    FrameChargeDischargeCabinetInfo1 strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    char breaker1 = strFrame.stStatus.DCDC1breaker;
    char breaker2 = strFrame.stStatus.DCDC2breaker;
    char fireExtinguisher1 = strFrame.stStatus.DCDC1fireExtinguisher;
    char fireExtinguisher2 = strFrame.stStatus.DCDC2fireExtinguisher;
    float sumVol = strFrame.DCDCsumVol * 0.1;
    float sumCur = strFrame.DCDCsumCur * 0.1;

    InsertCanInfoMapLine((char *)&breaker1, 1, Addr_DCDC1breaker, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&breaker2, 1, Addr_DCDC2breaker, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&fireExtinguisher1, 1, Addr_DCDC1fireExtinguisher, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&fireExtinguisher2, 1, Addr_DCDC2fireExtinguisher, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&sumVol, sizeof(sumVol), Addr_DCDCsumVol, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&sumCur, sizeof(sumCur), Addr_DCDCsumCur, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&pgnID, 1, Addr_PGN_ID, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&canAddr, 1, Addr_CanID_Comm, newNode.stCanMap);

    newNode.enType = AddrType_CD_Cab_Info;
    ToCenterList.append(newNode);
}

//解析系统控制柜信息１
void cEnergyPlanProtocol::ParseSysControlCabinetInfo1(unsigned char * pData, QList<CanMapNode> &ToCenterList, char pgnID, char canAddr)
{
    CanMapNode newNode;
    //解帧
    FrameSysControlCabinetInfo1 strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    char inemergencyStop = strFrame.stStatus.centerContorlEmergencyStop;
    char outemergencyStop = strFrame.stStatus.outEmergencyStop;
    char lowSwitch = strFrame.stStatus.lowVolTravelSwitch;
    char highSwitch = strFrame.stStatus.highVolTravelSwitch;
    char dormSwitch = strFrame.stStatus.dormTravelSwitch;
    char lowSensor = strFrame.stStatus.lowVolSmokeSensor;
    char highSensor = strFrame.stStatus.highVolSmokeSensor;
    char transSensor = strFrame.stStatus.transformerSmokeSensor;
    InsertCanInfoMapLine((char *)&inemergencyStop, 1, Addr_inEmergencyStop, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&outemergencyStop, 1, Addr_outEmergencyStop, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&lowSwitch, 1, Addr_lowVolTravelSwitch, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&highSwitch, 1, Addr_highVolTravelSwitch, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&dormSwitch, 1, Addr_dormTravelSwitch, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&lowSensor, 1, Addr_lowVolSmokeSensor, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&highSensor, 1, Addr_highVolSmokeSensor, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&transSensor, 1, Addr_transformerSmokeSensor, newNode.stCanMap);

    InsertCanInfoMapLine((char *)&pgnID, 1, Addr_PGN_ID, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&canAddr, 1, Addr_CanID_Comm, newNode.stCanMap);
    newNode.enType = AddrType_SC_Cab_Info;
    ToCenterList.append(newNode);
}

//解析系统控制柜信息2
void cEnergyPlanProtocol::ParseSysControlCabinetInfo2(unsigned char * pData, QList<CanMapNode> &ToCenterList, char pgnID,char canAddr)
{
    CanMapNode newNode;
    //解帧
    FrameSysControlCabinetInfo2 strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    char overTemp = strFrame.stStatus.transformerOverTemp;
    char tempContolerFault = strFrame.stStatus.transformerTempControlerFault;
    char waterIn = strFrame.stStatus.waterIn;

    InsertCanInfoMapLine((char *)&overTemp, 1, Addr_transformerOverTemp, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&tempContolerFault, 1, Addr_transformerTempControlerFault, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&waterIn, 1, Addr_waterIn_SysControl2,newNode.stCanMap);

    InsertCanInfoMapLine((char *)&pgnID, 1, Addr_PGN_ID, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&canAddr, 1, Addr_CanID_Comm, newNode.stCanMap);
    newNode.enType = AddrType_SC_Cab_Info;
    ToCenterList.append(newNode);
}

//解析总配电柜信息１
void cEnergyPlanProtocol::ParseMainDistributionCabinetInfo1(unsigned char * pData, QList<CanMapNode> &ToCenterList, char pgnID,char canAddr)
{
    CanMapNode newNode;

    //解帧
    FrameDistributionCabinetInfo1 strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    char sumBreaker = strFrame.stStatus.sumBreaker;
    char loadBreaker1 = strFrame.stStatus.loadBreaker1;
    char loadBreaker2 = strFrame.stStatus.loadBreaker2;
    char loadBreaker3 = strFrame.stStatus.loadBreaker3;
    char loadBreaker4 = strFrame.stStatus.loadBreaker4;
    char acBreaker = strFrame.stStatus.acBreaker;
    char fireExtinguisher =  strFrame.stStatus.fireExtinguisher;

    InsertCanInfoMapLine((char *)&sumBreaker, 1, Addr_sumBreaker, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&loadBreaker1, 1, Addr_loadBreaker1, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&loadBreaker2, 1, Addr_loadBreaker2, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&loadBreaker3, 1, Addr_loadBreaker3, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&loadBreaker4, 1, Addr_loadBreaker4, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&acBreaker, 1, Addr_acBreaker, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&fireExtinguisher, 1,Addr_fireExtinguisherDisCabinet, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&pgnID, 1, Addr_PGN_ID, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&canAddr, 1, Addr_CanID_Comm, newNode.stCanMap);
    newNode.enType = AddrType_TD_Cab_Info;
    ToCenterList.append(newNode);
}


//储能柜信息
void cEnergyPlanProtocol::ParseEnergyStorageCabinetInfo(unsigned char * pData, QList<CanMapNode> &ToCenterList, char pgnID, char canAddr)
{
    CanMapNode newNode;
    FrameEnergyStorageCabinetInfo stFrame;
    //通用变量定义
    unsigned char ucTemp = 0;

    //帧解析, 转换为通用格式
    memset(&stFrame, 0x00, sizeof(stFrame));
    memcpy((unsigned char *)&stFrame, pData, sizeof(stFrame));
    //区分两个ｔｍ１０１
    if(canAddr == InnerID_ES_TM_101)
    {
        ucTemp = stFrame.state.stStatus.mainBreaker;
        InsertCanInfoMapLine((char *)&ucTemp, 1, Addr_mainBreaker_EnergyStore, newNode.stCanMap);
        ucTemp = stFrame.state.stStatus.slaveBreaker;
        InsertCanInfoMapLine((char *)&ucTemp, 1, Addr_slaveBreaker_EnergyStore, newNode.stCanMap);
        ucTemp = stFrame.state.stStatus.DCBreaker1;
        InsertCanInfoMapLine((char *)&ucTemp, 1, Addr_DCBreaker1_EnergyStore, newNode.stCanMap);
        ucTemp = stFrame.state.stStatus.DCBreaker2;
        InsertCanInfoMapLine((char *)&ucTemp, 1, Addr_DCBreaker2_EnergyStore, newNode.stCanMap);
        ucTemp = stFrame.state.stStatus.fireExtinguisher;
        InsertCanInfoMapLine((char *)&ucTemp, 1, Addr_fireExtinguisherStatus_EnergyStore, newNode.stCanMap);
    }
    else if(canAddr == InnerID_ES_TM_102)
    {
        ucTemp = stFrame.state.stStatus2.DCBreaker3;
        InsertCanInfoMapLine((char *)&ucTemp, 1, Addr_DCBreaker3_EnergyStore, newNode.stCanMap);
        ucTemp = stFrame.state.stStatus2.DCBreaker4;
        InsertCanInfoMapLine((char *)&ucTemp, 1, Addr_DCBreaker4_EnergyStore, newNode.stCanMap);
        ucTemp = stFrame.state.stStatus2.tripFeedback;
        InsertCanInfoMapLine((char *)&ucTemp, 1, Addr_travelSwitch_EnergyStore, newNode.stCanMap);
        ucTemp = stFrame.state.stStatus2.fireExtinguisher;
        InsertCanInfoMapLine((char *)&ucTemp, 1, Addr_fireExtinguisher_EnergyStore, newNode.stCanMap);
        ucTemp = stFrame.state.stStatus2.waterIn;
        InsertCanInfoMapLine((char *)&ucTemp, 1, Addr_founder_EnergyStore, newNode.stCanMap);
    }
    InsertCanInfoMapLine((char *)&pgnID, 1, Addr_PGN_ID, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&canAddr, 1, Addr_CanID_Comm, newNode.stCanMap);
    newNode.enType = AddrType_ES_Cab_Info;
    ToCenterList.append(newNode);
}

//ACDC信息１
void cEnergyPlanProtocol::ParseACDCInfo1(FrameLongPackage * pLongPackage, QList<CanMapNode> &ToCenterList, char pgnID, char canAddr)
{
    CanMapNode newNode;
    FrameACDCInfo stFrame;
    //通用变量定义
    unsigned short sTemp = 0;
    unsigned int uiTemp = 0;
    float fTemp = 0;
    if(pLongPackage->pData == NULL)
    {
        return;
    }
    //帧解析, 转换为通用格式
    memset(&stFrame, 0x00, sizeof(stFrame));
    memcpy((unsigned char *)&stFrame, pLongPackage->pData, sizeof(stFrame));

    sTemp = stFrame.moduleID;
    InsertCanInfoMapLine((char *)&sTemp, sizeof(sTemp), Addr_DevID_DC_Comm, newNode.stCanMap);
    fTemp = stFrame.vol_U * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_vol_U, newNode.stCanMap);
    fTemp = stFrame.cur_U * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_cur_U, newNode.stCanMap);
    fTemp = stFrame.vol_V * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_vol_V, newNode.stCanMap);
    fTemp = stFrame.cur_V * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_cur_V, newNode.stCanMap);
    fTemp = stFrame.vol_W * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_vol_W, newNode.stCanMap);
    fTemp = stFrame.cur_W * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_cur_W, newNode.stCanMap);
    fTemp = stFrame.frequency * 0.01;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_frequency, newNode.stCanMap);

    fTemp = stFrame.sysActivePower * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_sysActivePower, newNode.stCanMap);
    fTemp = stFrame.sysReActivePower * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_sysReActivePower, newNode.stCanMap);
    fTemp = stFrame.sysApparentPower * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_sysApparentPower, newNode.stCanMap);
    fTemp = stFrame.PF * 0.001;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_PF, newNode.stCanMap);

    fTemp = stFrame.DCpositiveCur * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_DCpositiveCur, newNode.stCanMap);
    fTemp = stFrame.DCnegativeCur * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_DCnegativeCur, newNode.stCanMap);
    fTemp = stFrame.DCpositiveBusBarVol * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_DCpositiveBusBarVol, newNode.stCanMap);
    fTemp = stFrame.DCnegativeBusBarVol * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_DCnegativeBusBarVol, newNode.stCanMap);
    fTemp = stFrame.DCbilateralBusBarVol * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_DCbilateralBusBarVol, newNode.stCanMap);
    fTemp = stFrame.DCpower * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_DCpower, newNode.stCanMap);

    sTemp = stFrame.devStatus & 0x07;
    InsertCanInfoMapLine((char *)&sTemp, sizeof(sTemp), Addr_devStatus0, newNode.stCanMap);
    sTemp = stFrame.warningStatus & 0x07;
    InsertCanInfoMapLine((char *)&sTemp, sizeof(sTemp), Addr_warningStatus0, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&stFrame.faultStatus, sizeof(stFrame.faultStatus), Addr_faultStatus0, newNode.stCanMap);

//    if(stFrame.devStatus&0x01)
//        sTemp = 1;
//    else
//        sTemp = 0;
//    InsertCanInfoMapLine((char *)&sTemp, sizeof(sTemp), Addr_devStatus0, newNode.stCanMap);
//    if(stFrame.devStatus&0x02)
//        sTemp = 1;
//    else
//        sTemp = 0;
//    InsertCanInfoMapLine((char *)&sTemp, sizeof(sTemp), Addr_devStatus1, newNode.stCanMap);
//    if(stFrame.devStatus&0x04)
//        sTemp = 1;
//    else
//        sTemp = 0;
//    InsertCanInfoMapLine((char *)&sTemp, sizeof(sTemp), Addr_devStatus2, newNode.stCanMap);

//    if(stFrame.warningStatus&0x01)
//        sTemp = 1;
//    else
//        sTemp = 0;
//    InsertCanInfoMapLine((char *)&sTemp, sizeof(sTemp), Addr_warningStatus0, newNode.stCanMap);
//    if(stFrame.warningStatus&0x02)
//        sTemp = 1;
//    else
//        sTemp = 0;
//    InsertCanInfoMapLine((char *)&sTemp, sizeof(sTemp), Addr_warningStatus1, newNode.stCanMap);
//    if(stFrame.warningStatus&0x04)
//        sTemp = 1;
//    else
//        sTemp = 0;
//    InsertCanInfoMapLine((char *)&sTemp, sizeof(sTemp), Addr_warningStatus2, newNode.stCanMap);

//    if(stFrame.faultStatus&0x01)
//        sTemp = 1;
//    else
//        sTemp = 0;
//    InsertCanInfoMapLine((char *)&sTemp, sizeof(sTemp), Addr_faultStatus0, newNode.stCanMap);
//    if(stFrame.faultStatus&0x02)
//        sTemp = 1;
//    else
//        sTemp = 0;
//    InsertCanInfoMapLine((char *)&sTemp, sizeof(sTemp), Addr_faultStatus1, newNode.stCanMap);
//    if(stFrame.faultStatus&0x04)
//        sTemp = 1;
//    else
//        sTemp = 0;
//    InsertCanInfoMapLine((char *)&sTemp, sizeof(sTemp), Addr_faultStatus2, newNode.stCanMap);
//    if(stFrame.faultStatus&0x08)
//        sTemp = 1;
//    else
//        sTemp = 0;
//    InsertCanInfoMapLine((char *)&sTemp, sizeof(sTemp), Addr_faultStatus3, newNode.stCanMap);
//    if(stFrame.faultStatus&0x10)
//        sTemp = 1;
//    else
//        sTemp = 0;
//    InsertCanInfoMapLine((char *)&sTemp, sizeof(sTemp), Addr_faultStatus4, newNode.stCanMap);
//    if(stFrame.faultStatus&0x20)
//        sTemp = 1;
//    else
//        sTemp = 0;
//    InsertCanInfoMapLine((char *)&sTemp, sizeof(sTemp), Addr_faultStatus5, newNode.stCanMap);
//    if(stFrame.faultStatus&0x40)
//        sTemp = 1;
//    else
//        sTemp = 0;
//    InsertCanInfoMapLine((char *)&sTemp, sizeof(sTemp), Addr_faultStatus6, newNode.stCanMap);
//    if(stFrame.faultStatus&0x80)
//        sTemp = 1;
//    else
//        sTemp = 0;
//    InsertCanInfoMapLine((char *)&sTemp, sizeof(sTemp), Addr_faultStatus7, newNode.stCanMap);
//    if(stFrame.faultStatus&0x100)
//        sTemp = 1;
//    else
//        sTemp = 0;
//    InsertCanInfoMapLine((char *)&sTemp, sizeof(sTemp), Addr_faultStatus8, newNode.stCanMap);
//    if(stFrame.faultStatus&0x200)
//        sTemp = 1;
//    else
//        sTemp = 0;
//    InsertCanInfoMapLine((char *)&sTemp, sizeof(sTemp), Addr_faultStatus9, newNode.stCanMap);
//    if(stFrame.faultStatus&0x400)
//        sTemp = 1;
//    else
//        sTemp = 0;
//    InsertCanInfoMapLine((char *)&sTemp, sizeof(sTemp), Addr_faultStatus10, newNode.stCanMap);
//    if(stFrame.faultStatus&0x800)
//        sTemp = 1;
//    else
//        sTemp = 0;
//    InsertCanInfoMapLine((char *)&sTemp, sizeof(sTemp), Addr_faultStatus11, newNode.stCanMap);
//    if(stFrame.faultStatus&0x1000)
//        sTemp = 1;
//    else
//        sTemp = 0;
//    InsertCanInfoMapLine((char *)&sTemp, sizeof(sTemp), Addr_faultStatus12, newNode.stCanMap);
//    if(stFrame.faultStatus&0x2000)
//        sTemp = 1;
//    else
//        sTemp = 0;
//    InsertCanInfoMapLine((char *)&sTemp, sizeof(sTemp), Addr_faultStatus13, newNode.stCanMap);
//    if(stFrame.faultStatus&0x4000)
//        sTemp = 1;
//    else
//        sTemp = 0;
//    InsertCanInfoMapLine((char *)&sTemp, sizeof(sTemp), Addr_faultStatus14, newNode.stCanMap);
//    if(stFrame.faultStatus&0x8000)
//        sTemp = 1;
//    else
//        sTemp = 0;
//    InsertCanInfoMapLine((char *)&sTemp, sizeof(sTemp), Addr_faultStatus15, newNode.stCanMap);

    uiTemp = stFrame.HWVersion_high << 16 + stFrame.HWVersion_low;
    InsertCanInfoMapLine((char *)&uiTemp, sizeof(uiTemp), Addr_HWVersion, newNode.stCanMap);
    uiTemp = stFrame.SWVersion_high << 16 + stFrame.SWVersion_low;
    InsertCanInfoMapLine((char *)&uiTemp, sizeof(uiTemp), Addr_SWVersion, newNode.stCanMap);
//    sTemp = stFrame.HWVersion_high;
//    InsertCanInfoMapLine((char *)&sTemp, sizeof(sTemp), Addr_HWVersion_high, newNode.stCanMap);
//    sTemp = stFrame.HWVersion_low;
//    InsertCanInfoMapLine((char *)&sTemp, sizeof(sTemp), Addr_HWVersion_low, newNode.stCanMap);
//    sTemp = stFrame.SWVersion_high;
//    InsertCanInfoMapLine((char *)&sTemp, sizeof(sTemp), Addr_SWVersion_high, newNode.stCanMap);
//    sTemp = stFrame.SWVersion_low;
//    InsertCanInfoMapLine((char *)&sTemp, sizeof(sTemp), Addr_SWVersion_low, newNode.stCanMap);
    fTemp = stFrame.tmp_IGBT1 * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_tmp_IGBT1, newNode.stCanMap);
    fTemp = stFrame.tmp_IGBT2 * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_tmp_IGBT2, newNode.stCanMap);
    fTemp = stFrame.tmp_IGBT3 * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_tmp_IGBT3, newNode.stCanMap);
    fTemp = stFrame.tmp_IGBT4 * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_tmp_IGBT4, newNode.stCanMap);
    fTemp = stFrame.tmp_IGBT5 * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_tmp_IGBT5, newNode.stCanMap);
    fTemp = stFrame.tmp_IGBT6 * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_tmp_IGBT6, newNode.stCanMap);
    fTemp = stFrame.tmp_IN * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_tmp_IN, newNode.stCanMap);
    fTemp = stFrame.tmp_OUT * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_tmp_OUT, newNode.stCanMap);
    fTemp = stFrame.inductance1_cur * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_inductance1_cur, newNode.stCanMap);
    fTemp = stFrame.inductance2_cur * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_inductance2_cur, newNode.stCanMap);
    fTemp = stFrame.inductance3_cur * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_inductance3_cur, newNode.stCanMap);
    fTemp = stFrame.inductance4_cur * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_inductance4_cur, newNode.stCanMap);
    fTemp = stFrame.inductance5_cur * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_inductance5_cur, newNode.stCanMap);
    fTemp = stFrame.inductance6_cur * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_inductance6_cur, newNode.stCanMap);

    InsertCanInfoMapLine((char *)&pgnID, 1, Addr_PGN_ID, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&canAddr, 1, Addr_CanID_Comm, newNode.stCanMap);
    newNode.enType = AddrType_ACDC_Mod_Info;
    ToCenterList.append(newNode);
}
//储能柜DCDC模块信息？？？？？？？？？？？？
//ACDC信息１
//解析充放电柜DCDC信息１
void cEnergyPlanProtocol::ParseChargeDisChargeCabinetDCDCInfo(FrameLongPackage * pLongPackage, QList<CanMapNode> &ToCenterList, char pgnID,char canAddr)
{
    CanMapNode newNode;
    char alarm = 0;
    float fTemp = 0;
    //解帧
    FrameChargeDischargeDCDCInfo strFrame;
    memcpy((unsigned char *)&strFrame, pLongPackage->pData, sizeof(strFrame));

    char workMode = strFrame.workMode.status;
    InsertCanInfoMapLine((char *)&strFrame.boardID, sizeof(strFrame.boardID), Addr_boardID, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&strFrame.moduleID, sizeof(strFrame.moduleID),Addr_DevID_DC_Comm , newNode.stCanMap);
//    InsertCanInfoMapLine((char *)&strFrame.moduleID, sizeof(strFrame.moduleID),Addr_moduleID , newNode.stCanMap);

    InsertCanInfoMapLine((char *)&workMode, sizeof(workMode), Addr_moduleMode, newNode.stCanMap);

    fTemp = strFrame.outVol * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp),Addr_outVol , newNode.stCanMap);
    fTemp = strFrame.outCur * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp),Addr_outCur , newNode.stCanMap);
    fTemp = strFrame.inVol * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp),Addr_inVol , newNode.stCanMap);
    fTemp = strFrame.inCur * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp),Addr_inCur , newNode.stCanMap);
    fTemp = strFrame.boardTemp_M1 - 50;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp),Addr_boardTmp_M1 , newNode.stCanMap);
    fTemp = strFrame.envTemp - 50;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp),Addr_envTmp , newNode.stCanMap);
    fTemp = strFrame.runTime;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp),Addr_runTime , newNode.stCanMap);
    fTemp = strFrame.chargeDisChargeTimes;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp),Addr_chargeDischargeTimes , newNode.stCanMap);

    alarm = strFrame.moduleStatus.alarm0;
    InsertCanInfoMapLine((char *)&alarm, 1, Addr_moduleStatus0, newNode.stCanMap);
    alarm = strFrame.moduleStatus.alarm1;
    InsertCanInfoMapLine((char *)&alarm, 1, Addr_moduleStatus1, newNode.stCanMap);
    alarm = strFrame.moduleStatus.alarm2;
    InsertCanInfoMapLine((char *)&alarm, 1, Addr_moduleStatus2, newNode.stCanMap);
    alarm = strFrame.moduleStatus.alarm3;
    InsertCanInfoMapLine((char *)&alarm, 1, Addr_moduleStatus3, newNode.stCanMap);
    alarm = strFrame.moduleStatus.alarm4;
    InsertCanInfoMapLine((char *)&alarm, 1, Addr_moduleStatus4, newNode.stCanMap);
    alarm = strFrame.moduleStatus.alarm5;
    InsertCanInfoMapLine((char *)&alarm, 1, Addr_moduleStatus5, newNode.stCanMap);
    alarm = strFrame.moduleStatus.alarm6;
    InsertCanInfoMapLine((char *)&alarm, 1, Addr_moduleStatus6, newNode.stCanMap);
    alarm = strFrame.moduleStatus.alarm7;
    InsertCanInfoMapLine((char *)&alarm, 1, Addr_moduleStatus7, newNode.stCanMap);
    alarm = strFrame.moduleStatus.alarm8;
    InsertCanInfoMapLine((char *)&alarm, 1, Addr_moduleStatus8, newNode.stCanMap);
    alarm = strFrame.moduleStatus.alarm9;
    InsertCanInfoMapLine((char *)&alarm, 1, Addr_moduleStatus9, newNode.stCanMap);
    alarm = strFrame.moduleStatus.alarm10;
    InsertCanInfoMapLine((char *)&alarm, 1, Addr_moduleStatus10, newNode.stCanMap);
    alarm = strFrame.moduleStatus.alarm11;
    InsertCanInfoMapLine((char *)&alarm, 1, Addr_moduleStatus11, newNode.stCanMap);
    alarm = strFrame.moduleStatus.alarm12;
    InsertCanInfoMapLine((char *)&alarm, 1, Addr_moduleStatus12, newNode.stCanMap);
    alarm = strFrame.moduleStatus.alarm13;
    InsertCanInfoMapLine((char *)&alarm, 1, Addr_moduleStatus13, newNode.stCanMap);
    alarm = strFrame.moduleStatus.alarm14;
    InsertCanInfoMapLine((char *)&alarm, 1, Addr_moduleStatus14, newNode.stCanMap);
    alarm = strFrame.moduleStatus.alarm15;
    InsertCanInfoMapLine((char *)&alarm, 1, Addr_moduleStatus15, newNode.stCanMap);
    alarm = strFrame.moduleStatus.alarm16;
    InsertCanInfoMapLine((char *)&alarm, 1, Addr_moduleStatus16, newNode.stCanMap);
    alarm = strFrame.moduleStatus.alarm17;
    InsertCanInfoMapLine((char *)&alarm, 1, Addr_moduleStatus17, newNode.stCanMap);
    alarm = strFrame.moduleStatus.alarm18;
    InsertCanInfoMapLine((char *)&alarm, 1, Addr_moduleStatus18, newNode.stCanMap);
    alarm = strFrame.moduleStatus.alarm19;
    InsertCanInfoMapLine((char *)&alarm, 1, Addr_moduleStatus19, newNode.stCanMap);
    alarm = strFrame.moduleStatus.alarm20;
    InsertCanInfoMapLine((char *)&alarm, 1, Addr_moduleStatus20, newNode.stCanMap);
    alarm = strFrame.moduleStatus.alarm21;
    InsertCanInfoMapLine((char *)&alarm, 1, Addr_moduleStatus21, newNode.stCanMap);
    alarm = strFrame.moduleStatus.alarm22;
    InsertCanInfoMapLine((char *)&alarm, 1, Addr_moduleStatus22, newNode.stCanMap);
    alarm = strFrame.moduleStatus.alarm23;
    InsertCanInfoMapLine((char *)&alarm, 1, Addr_moduleStatus23, newNode.stCanMap);
    alarm = strFrame.moduleStatus.alarm24;
    InsertCanInfoMapLine((char *)&alarm, 1, Addr_moduleStatus24, newNode.stCanMap);
    alarm = strFrame.moduleStatus.alarm25;
    InsertCanInfoMapLine((char *)&alarm, 1, Addr_moduleStatus25, newNode.stCanMap);
    alarm = strFrame.moduleStatus.alarm26;
    InsertCanInfoMapLine((char *)&alarm, 1, Addr_moduleStatus26, newNode.stCanMap);
    alarm = strFrame.moduleStatus.alarm27;
    InsertCanInfoMapLine((char *)&alarm, 1, Addr_moduleStatus27, newNode.stCanMap);
    alarm = strFrame.moduleStatus.alarm28;
    InsertCanInfoMapLine((char *)&alarm, 1, Addr_moduleStatus28, newNode.stCanMap);
    alarm = strFrame.moduleStatus.alarm29;
    InsertCanInfoMapLine((char *)&alarm, 1, Addr_moduleStatus29, newNode.stCanMap);
    alarm = strFrame.moduleStatus.alarm30;
    InsertCanInfoMapLine((char *)&alarm, 1, Addr_moduleStatus30, newNode.stCanMap);
    alarm = strFrame.moduleStatus.alarm31;
    InsertCanInfoMapLine((char *)&alarm, 1, Addr_moduleStatus31, newNode.stCanMap);

    InsertCanInfoMapLine((char *)&pgnID, 1, Addr_PGN_ID, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&canAddr, 1, Addr_CanID_Comm, newNode.stCanMap);
    newNode.enType = AddrType_DCDC_Mod_CD_Info;
    ToCenterList.append(newNode);
}
//储能柜DCDC信息
void cEnergyPlanProtocol::ParseEnergyStorageCabinetDCDCInfo(FrameLongPackage * pLongPackage, QList<CanMapNode> &ToCenterList, char pgnID,char canAddr)
{
    CanMapNode newNode;
    char tmpValue;
    float fTemp = 0;
    unsigned int uiTemp = 0;
    //解帧
    FrameEnergyStorageDCDCInfo strFrame;
    memcpy((unsigned char *)&strFrame, pLongPackage->pData, sizeof(strFrame));


//    InsertCanInfoMapLine((char *)&strFrame.moduleID, sizeof(strFrame.moduleID),Addr_moduleID , newNode.stCanMap);
    InsertCanInfoMapLine((char *)&strFrame.moduleID, sizeof(strFrame.moduleID),Addr_DevID_DC_Comm , newNode.stCanMap);

    fTemp = strFrame.vol_in * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_inVol_EnergyStorage, newNode.stCanMap);
    fTemp = strFrame.vol_out * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_outVol_EnergyStorage, newNode.stCanMap);
    fTemp = strFrame.cur_in * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_inCur_EnergyStorage, newNode.stCanMap);
    fTemp = strFrame.cur_out * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_outCur_EnergyStorage, newNode.stCanMap);
    fTemp = strFrame.vol_battery * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_batteryVol_EnergyStorage, newNode.stCanMap);
    fTemp = strFrame.power_dc * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_dcPower_EnergyStorage, newNode.stCanMap);
    tmpValue = strFrame.devStatus.warning;
    InsertCanInfoMapLine((char *)&tmpValue, 1, Addr_devStatus0_EnergyStorage, newNode.stCanMap);
    tmpValue = strFrame.devStatus.run;
    InsertCanInfoMapLine((char *)&tmpValue, sizeof(tmpValue), Addr_devStatus1_EnergyStorage, newNode.stCanMap);
    tmpValue = strFrame.devStatus.fault;
    InsertCanInfoMapLine((char *)&tmpValue, sizeof(tmpValue), Addr_devStatus2_EnergyStorage, newNode.stCanMap);
    tmpValue = strFrame.devStatus.offLine;
    InsertCanInfoMapLine((char *)&tmpValue, sizeof(tmpValue), Addr_devStatus3_EnergyStorage, newNode.stCanMap);
    tmpValue = strFrame.warningStatus.fun1;
    InsertCanInfoMapLine((char *)&tmpValue, sizeof(tmpValue), Addr_warningStatus0_EnergyStorage, newNode.stCanMap);
    tmpValue = strFrame.warningStatus.fun2;
    InsertCanInfoMapLine((char *)&tmpValue, sizeof(tmpValue), Addr_warningStatus1_EnergyStorage, newNode.stCanMap);
    tmpValue = strFrame.warningStatus.fun3;
    InsertCanInfoMapLine((char *)&tmpValue, sizeof(tmpValue), Addr_warningStatus2_EnergyStorage, newNode.stCanMap);
    tmpValue = strFrame.faultStatus.fault0;
    InsertCanInfoMapLine((char *)&tmpValue, sizeof(tmpValue), Addr_faultStatus0_EnergyStorage, newNode.stCanMap);
    tmpValue = strFrame.faultStatus.fault1;
    InsertCanInfoMapLine((char *)&tmpValue, sizeof(tmpValue), Addr_faultStatus1_EnergyStorage, newNode.stCanMap);
    tmpValue=strFrame.faultStatus.fault2;
    InsertCanInfoMapLine((char *)&tmpValue, sizeof(tmpValue), Addr_faultStatus2_EnergyStorage, newNode.stCanMap);
    tmpValue=strFrame.faultStatus.fault3;
    InsertCanInfoMapLine((char *)&tmpValue, sizeof(tmpValue), Addr_faultStatus3_EnergyStorage, newNode.stCanMap);
    tmpValue=strFrame.faultStatus.fault4;
    InsertCanInfoMapLine((char *)&tmpValue, sizeof(tmpValue), Addr_faultStatus4_EnergyStorage, newNode.stCanMap);
    tmpValue=strFrame.faultStatus.fault5;
    InsertCanInfoMapLine((char *)&tmpValue, sizeof(tmpValue), Addr_faultStatus5_EnergyStorage, newNode.stCanMap);
    tmpValue=strFrame.faultStatus.fault6;
    InsertCanInfoMapLine((char *)&tmpValue, sizeof(tmpValue), Addr_faultStatus6_EnergyStorage, newNode.stCanMap);
    tmpValue=strFrame.faultStatus.fault7;
    InsertCanInfoMapLine((char *)&tmpValue, sizeof(tmpValue), Addr_faultStatus7_EnergyStorage, newNode.stCanMap);
    tmpValue=strFrame.faultStatus.fault8;
    InsertCanInfoMapLine((char *)&tmpValue, sizeof(tmpValue), Addr_faultStatus8_EnergyStorage, newNode.stCanMap);
    tmpValue=strFrame.faultStatus.fault9;
    InsertCanInfoMapLine((char *)&tmpValue, sizeof(tmpValue), Addr_faultStatus9_EnergyStorage, newNode.stCanMap);
    tmpValue=strFrame.faultStatus.fault10;
    InsertCanInfoMapLine((char *)&tmpValue, sizeof(tmpValue), Addr_faultStatus10_EnergyStorage, newNode.stCanMap);
    tmpValue=strFrame.faultStatus.fault11;
    InsertCanInfoMapLine((char *)&tmpValue, sizeof(tmpValue), Addr_faultStatus11_EnergyStorage, newNode.stCanMap);
    tmpValue=strFrame.faultStatus.fault12;
    InsertCanInfoMapLine((char *)&tmpValue, sizeof(tmpValue), Addr_faultStatus12_EnergyStorage, newNode.stCanMap);
    tmpValue=strFrame.faultStatus.fault13;
    InsertCanInfoMapLine((char *)&tmpValue, sizeof(tmpValue), Addr_faultStatus13_EnergyStorage, newNode.stCanMap);
    tmpValue=strFrame.faultStatus.fault14;
    InsertCanInfoMapLine((char *)&tmpValue, sizeof(tmpValue), Addr_faultStatus14_EnergyStorage, newNode.stCanMap);
    tmpValue=strFrame.faultStatus.fault15;
    InsertCanInfoMapLine((char *)&tmpValue, sizeof(tmpValue), Addr_faultStatus15_EnergyStorage, newNode.stCanMap);

    uiTemp = (strFrame.HWVersion_high << 16) + strFrame.HWVersion_low;
    InsertCanInfoMapLine((char *)&uiTemp, sizeof(uiTemp), Addr_HWVersion, newNode.stCanMap);
    uiTemp = (strFrame.SWVersion_high << 16) + strFrame.SWVersion_low;
    InsertCanInfoMapLine((char *)&uiTemp, sizeof(uiTemp), Addr_SWVersion, newNode.stCanMap);

//    InsertCanInfoMapLine((char *)&strFrame.HWVersion_high, 2, Addr_HWVersion_high, newNode.stCanMap);
//    InsertCanInfoMapLine((char *)&strFrame.HWVersion_low, 2, Addr_HWVersion_low, newNode.stCanMap);
//    InsertCanInfoMapLine((char *)&strFrame.SWVersion_high, 2, Addr_SWVersion_high, newNode.stCanMap);
//    InsertCanInfoMapLine((char *)&strFrame.SWVersion_low, 2, Addr_SWVersion_low, newNode.stCanMap);

    fTemp = (float)strFrame.tmp_IGBT1 * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_tmp_IGBT1, newNode.stCanMap);
    fTemp = (float)strFrame.tmp_IGBT2 * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_tmp_IGBT2, newNode.stCanMap);
    fTemp = (float)strFrame.tmp_IGBT3 * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_tmp_IGBT3, newNode.stCanMap);
    fTemp = (float)strFrame.tmp_IGBT4 * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_tmp_IGBT4, newNode.stCanMap);
    fTemp = (float)strFrame.tmp_IGBT5 * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_tmp_IGBT5, newNode.stCanMap);
    fTemp = (float)strFrame.tmp_IGBT6 * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_tmp_IGBT6, newNode.stCanMap);
    fTemp = (float)strFrame.tmp_IN * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_tmp_IN, newNode.stCanMap);
    fTemp = (float)strFrame.tmp_OUT * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_tmp_OUT, newNode.stCanMap);

    InsertCanInfoMapLine((char *)&pgnID, 1, Addr_PGN_ID, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&canAddr, 1, Addr_CanID_Comm, newNode.stCanMap);
    newNode.enType = AddrType_DCDC_Mod_ES_Info;
    ToCenterList.append(newNode);
}

//解析温湿度计信息１
void cEnergyPlanProtocol::ParseHumitureInfo1(unsigned char * pData, QList<CanMapNode> &ToCenterList, char pgnID, char canAddr)
{
    CanMapNode newNode;
    float fTemp = 0;
    //解帧
    FrameHumitureInfo1 strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    InsertCanInfoMapLine((char *)&strFrame.addr, sizeof(strFrame.addr), Addr_DevID_DC_Comm, newNode.stCanMap);

    fTemp = (float)strFrame.temperature * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_envTemperature, newNode.stCanMap);
    fTemp = (float)strFrame.humidity * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_envHumidity, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&pgnID, 1, Addr_PGN_ID, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&canAddr, 1, Addr_CanID_Comm, newNode.stCanMap);
    newNode.enType = AddrType_HY_Mod_Info;
    ToCenterList.append(newNode);
}
//解析EMS温湿度计信息１
void cEnergyPlanProtocol::ParseEMSHumitureInfo1(unsigned char * pData, QList<CanMapNode> &ToCenterList, char pgnID, char canAddr)
{
    CanMapNode newNode;
    float fTemp = 0;
    //解帧
    FrameEMSHumitureInfo1 strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    char addr =1;
    InsertCanInfoMapLine((char *)&addr, 1, Addr_DevID_DC_Comm, newNode.stCanMap);
//    InsertCanInfoMapLine((char *)&strFrame.addr, sizeof(strFrame.addr), Addr_humitureID, newNode.stCanMap);
    fTemp = (float)strFrame.tempHighVol * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_tmpEMSHighVol, newNode.stCanMap);
    fTemp = (float)strFrame.humidityHighVol * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_humityEMSHighVol, newNode.stCanMap);
    fTemp = (float)strFrame.tempLowVol * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_tmpEMSLowVol, newNode.stCanMap);
    fTemp = (float)strFrame.humidityLowVol * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_humityEMSLowVol, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&pgnID, 1, Addr_PGN_ID, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&canAddr, 1, Addr_CanID_Comm, newNode.stCanMap);
    newNode.enType = AddrType_EMS_Info;
    ToCenterList.append(newNode);
}
//解析EMS温湿度计信息2
void cEnergyPlanProtocol::ParseEMSHumitureInfo2(unsigned char * pData, QList<CanMapNode> &ToCenterList, char pgnID, char canAddr)
{
    CanMapNode newNode;
    float fTemp = 0;
    //解帧
    FrameEMSHumitureInfo2 strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    char addr =2;
    InsertCanInfoMapLine((char *)&addr, 1, Addr_DevID_DC_Comm, newNode.stCanMap);
//    InsertCanInfoMapLine((char *)&strFrame.addr, sizeof(strFrame.addr), Addr_humitureID, newNode.stCanMap);
    fTemp = (float)strFrame.tempLowVol * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_tmpEMSLowVol_out, newNode.stCanMap);
    fTemp = (float)strFrame.humidityLowVol * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_humityEMSLowVol_out, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&pgnID, 1, Addr_PGN_ID, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&canAddr, 1, Addr_CanID_Comm, newNode.stCanMap);
    newNode.enType = AddrType_EMS_Info;
    ToCenterList.append(newNode);
}
//解析储能电池信息1
void cEnergyPlanProtocol::ParseEnergyStorageCabinetBatteryInfo(FrameLongPackage * pLongPackage, QList<CanMapNode> &ToCenterList,char pgnID,char canAddr)
{
    CanMapNode newNode;
    char tmpData;
    float ftemp = 0;
    FrameEnergyStorageCabinetBatteryInfo stFrame;

    if(pLongPackage->pData == NULL)
    {
        return;
    }
    //帧解析, 转换为通用格式
    memset(&stFrame, 0x00, sizeof(stFrame));
    memcpy((unsigned char *)&stFrame, pLongPackage->pData, sizeof(stFrame));

    InsertCanInfoMapLine((char *)&pgnID, 1, Addr_PGN_ID, newNode.stCanMap);

    InsertCanInfoMapLine((char *)&stFrame.B2C_STATUS.CRC, 1, Addr_B2C_STATUS_CRC, newNode.stCanMap);
    tmpData = stFrame.B2C_STATUS.bmsHeartBeat;
    InsertCanInfoMapLine((char *)&tmpData, 1, Addr_B2C_STATUS_bmsHeartBeat, newNode.stCanMap);
    tmpData = stFrame.B2C_STATUS.test;
    InsertCanInfoMapLine((char *)&tmpData, 1, Addr_B2C_STATUS_test, newNode.stCanMap);

     tmpData = stFrame.B2C_STATUS.tankSwitch;
     InsertCanInfoMapLine((char *)&tmpData, 1, Addr_B2C_STATUS_tankSwitch, newNode.stCanMap);

    tmpData = stFrame.B2C_STATUS.singleOverVolAlarm;
    InsertCanInfoMapLine((char *)&tmpData, 1, Addr_B2C_STATUS_singleOverVolAlarm, newNode.stCanMap);
    tmpData = stFrame.B2C_STATUS.singleLowVolAlarm;
    InsertCanInfoMapLine((char *)&tmpData, 1, Addr_B2C_STATUS_singleLowVolAlarm, newNode.stCanMap);
    tmpData = stFrame.B2C_STATUS.OverTempAlarm;
    InsertCanInfoMapLine((char *)&tmpData, 1, Addr_B2C_STATUS_OverTempAlarm, newNode.stCanMap);
    //Addr_B2C_STATUS_BelowTempAlarm
    tmpData = stFrame.B2C_STATUS.BelowTempAlarm;
    InsertCanInfoMapLine((char *)&tmpData, 1, Addr_B2C_STATUS_BelowTempAlarm, newNode.stCanMap);

    tmpData = stFrame.B2C_STATUS.insulationAlarm;
    InsertCanInfoMapLine((char *)&tmpData, 1, Addr_B2C_STATUS_insulationAlarm, newNode.stCanMap);
    tmpData = stFrame.B2C_STATUS.BMScommuFault;
    InsertCanInfoMapLine((char *)&tmpData, 1, Addr_B2C_STATUS_BMScommuFault, newNode.stCanMap);

     tmpData = stFrame.B2C_STATUS.BMScontrolPower;
     InsertCanInfoMapLine((char *)&tmpData, 1, Addr_B2C_STATUS_BMScontrolPower, newNode.stCanMap);
    tmpData = stFrame.B2C_STATUS.BMSfullPowerON;
    InsertCanInfoMapLine((char *)&tmpData, 1, Addr_B2C_STATUS_BMSfullPowerON, newNode.stCanMap);
    tmpData = stFrame.B2C_STATUS.BMSsysStatus;
    InsertCanInfoMapLine((char *)&tmpData, 1, Addr_B2C_STATUS_BMSsysStatus, newNode.stCanMap);
    tmpData = stFrame.B2C_STATUS.ESSfullEnergy;
    InsertCanInfoMapLine((char *)&tmpData, 1, Addr_B2C_STATUS_ESSfullEnergy, newNode.stCanMap);
    tmpData = stFrame.B2C_STATUS.ESSfullDisCharge;
    InsertCanInfoMapLine((char *)&tmpData, 1, Addr_B2C_STATUS_ESSfullDisCharge, newNode.stCanMap);

    tmpData = stFrame.B2C_STATUS.ApplyACInfo;
    InsertCanInfoMapLine((char *)&tmpData, 1, Addr_B2C_STATUS_ApplyACInfo, newNode.stCanMap);

    tmpData = stFrame.B2C_STATUS.ApplySysInfo;
    InsertCanInfoMapLine((char *)&tmpData, 1, Addr_B2C_STATUS_ApplySysInfo, newNode.stCanMap);

    ftemp = stFrame.B2C_STATUS.SOC * 0.1;
    InsertCanInfoMapLine((char *)&ftemp, sizeof(ftemp), Addr_B2C_STATUS_SOC, newNode.stCanMap);

    InsertCanInfoMapLine((char *)&stFrame.B2C_SUMDATA1.CRC, sizeof(stFrame.B2C_SUMDATA1.CRC), Addr_B2C_SUMDATA1_CRC, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&stFrame.B2C_SUMDATA1.tankNum, sizeof(stFrame.B2C_SUMDATA1.tankNum), Addr_B2C_SUMDATA1_tankNum, newNode.stCanMap);
    ftemp = (float)(stFrame.B2C_SUMDATA1.BMShighVol) * 0.1;
    InsertCanInfoMapLine((char *)&ftemp, sizeof(ftemp), Addr_B2C_SUMDATA1_BMShighVol, newNode.stCanMap);
    ftemp = (float)(stFrame.B2C_SUMDATA1.BMScur) * 0.1 - 2000;
    InsertCanInfoMapLine((char *)&ftemp, sizeof(ftemp), Addr_B2C_SUMDATA1_BMScur, newNode.stCanMap);

    InsertCanInfoMapLine((char *)&stFrame.B2C_SUMDATA2.CRC, sizeof(stFrame.B2C_SUMDATA2.CRC), Addr_B2C_SUMDATA2_CRC, newNode.stCanMap);
    ftemp = float(stFrame.B2C_SUMDATA2.BMSchargeEnergy) * 0.1;
    InsertCanInfoMapLine((char *)&ftemp, sizeof(ftemp), Addr_B2C_SUMDATA2_BMSchargeEnergy, newNode.stCanMap);
    ftemp = float(stFrame.B2C_SUMDATA2.BMSdisChargeEnergy) * 0.1;
    InsertCanInfoMapLine((char *)&ftemp, sizeof(ftemp), Addr_B2C_SUMDATA2_BMSdisChargeEnergy, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&stFrame.B2C_SUMDATA2.SOH, sizeof(stFrame.B2C_SUMDATA2.SOH), Addr_B2C_SUMDATA2_SOH, newNode.stCanMap);



    InsertCanInfoMapLine((char *)&stFrame.B2C_SUMDATA3.sysHumidity, sizeof(stFrame.B2C_SUMDATA3.sysHumidity), Addr_B2C_SUMDATA3_sysHumidity, newNode.stCanMap);

    ftemp = float(stFrame.B2C_SUMDATA3.singleMaxVol) * 0.001;
    InsertCanInfoMapLine((char *)&ftemp, sizeof(ftemp), Addr_B2C_SUMDATA3_singleMaxVol, newNode.stCanMap);
    ftemp = float(stFrame.B2C_SUMDATA3.singleMinVol) * 0.001;
    InsertCanInfoMapLine((char *)&ftemp, sizeof(ftemp), Addr_B2C_SUMDATA3_singleMinVol, newNode.stCanMap);

    ftemp = float(stFrame.B2C_SUMDATA3.singleMaxTem) - 50;
    InsertCanInfoMapLine((char *)&ftemp, sizeof(ftemp), Addr_B2C_SUMDATA3_singleMaxTem, newNode.stCanMap);

    ftemp = float(stFrame.B2C_SUMDATA3.singleMinTem) - 50;
    InsertCanInfoMapLine((char *)&ftemp, sizeof(ftemp), Addr_B2C_SUMDATA3_singleMinTem, newNode.stCanMap);
    ftemp = float(stFrame.B2C_SUMDATA3.sysTemp) - 50;
    InsertCanInfoMapLine((char *)&ftemp, sizeof(ftemp), Addr_B2C_SUMDATA3_sysTemp, newNode.stCanMap);

    ftemp = float(stFrame.B2C_LIMIT.BMSlimitDischargeCur) * 0.1;
    InsertCanInfoMapLine((char *)&ftemp, sizeof(ftemp), Addr_B2C_LIMIT_BMSlimitDischargeCur, newNode.stCanMap);
    ftemp = float(stFrame.B2C_LIMIT.BMSlimitChargeCur) * 0.1;
    InsertCanInfoMapLine((char *)&ftemp, sizeof(ftemp), Addr_B2C_LIMIT_BMSlimitChargeCur, newNode.stCanMap);
    ftemp = float(stFrame.B2C_LIMIT.BMSlimitChargeVol) * 0.1;
    InsertCanInfoMapLine((char *)&ftemp, sizeof(ftemp), Addr_B2C_LIMIT_BMSlimitChargeVol, newNode.stCanMap);
    ftemp = float(stFrame.B2C_LIMIT.BMSlimitDisChargeVol) * 0.1;
    InsertCanInfoMapLine((char *)&ftemp, sizeof(ftemp), Addr_B2C_LIMIT_BMSlimitDisChargeVol, newNode.stCanMap);

    ftemp = float(stFrame.outVol) * 0.1;
    InsertCanInfoMapLine((char *)&ftemp, sizeof(ftemp), Addr_BatteryOutVol, newNode.stCanMap);
    ftemp = float(stFrame.fuseVol) * 0.1;
    InsertCanInfoMapLine((char *)&ftemp, sizeof(ftemp), Addr_BatteryFuseVol, newNode.stCanMap);
    ftemp = float(stFrame.breakerVol) * 0.1;
    InsertCanInfoMapLine((char *)&ftemp, sizeof(ftemp), Addr_BatteryBreakVol, newNode.stCanMap);
    ftemp = float(stFrame.cur) * 0.1;
    InsertCanInfoMapLine((char *)&ftemp, sizeof(ftemp), Addr_BatteryCur, newNode.stCanMap);
    ftemp = float(stFrame.dcVol) * 0.1;
    InsertCanInfoMapLine((char *)&ftemp, sizeof(ftemp), Addr_BatteryDcVol, newNode.stCanMap);
    ftemp = float(stFrame.dcCur) * 0.1;
    InsertCanInfoMapLine((char *)&ftemp, sizeof(ftemp), Addr_BatteryDcCur, newNode.stCanMap);
    ftemp = float(stFrame.dcPower) * 0.01;
    InsertCanInfoMapLine((char *)&ftemp, sizeof(ftemp), Addr_BatteryDcPower, newNode.stCanMap);
    ftemp = float(stFrame.dcPositiveEnergy) * 0.01;
    InsertCanInfoMapLine((char *)&ftemp, sizeof(ftemp), Addr_BatteryDcPositiveEnergy, newNode.stCanMap);
    ftemp = float(stFrame.dcDisPositiveEnergy) * 0.01;
    InsertCanInfoMapLine((char *)&ftemp, sizeof(ftemp), Addr_BatteryDcDisPositiveEnergy, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&stFrame.dcPT, 2, Addr_BatteryDcPT, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&stFrame.dcCT, 2, Addr_BatteryDcCT, newNode.stCanMap);

    InsertCanInfoMapLine((char *)&canAddr, 1, Addr_CanID_Comm, newNode.stCanMap);

    newNode.enType = AddrType_ES_Bat_Info;//主题
    ToCenterList.append(newNode);
}

//功率优化器信息１
void cEnergyPlanProtocol::ParsePowerOptimizerInfo1(FrameLongPackage * pLongPackage, QList<CanMapNode> &ToCenterList, char pgnID, char canAddr)
{
    CanMapNode newNode;
    FramePowerOptimizerInfo1 stFrame;
    float fTemp = 0;
    unsigned uiTemp = 0;
    if(pLongPackage->pData == NULL)
    {
        return;
    }
    //帧解析, 转换为通用格式
    memset(&stFrame, 0x00, sizeof(stFrame));
    memcpy((unsigned char *)&stFrame, pLongPackage->pData, sizeof(stFrame));

    InsertCanInfoMapLine((char *)&pgnID, 1, Addr_PGN_ID, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&canAddr, 1, Addr_CanID_Comm, newNode.stCanMap);

//    InsertCanInfoMapLine((char *)&stFrame.PowerOptimizerID, sizeof(stFrame.PowerOptimizerID), Addr_PowerOptimizerID, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&stFrame.PowerOptimizerID, sizeof(stFrame.PowerOptimizerID), Addr_DevID_DC_Comm, newNode.stCanMap);
    fTemp = float(stFrame.inVol1) * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_inVol1, newNode.stCanMap);
    fTemp = float(stFrame.inVol2) * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_inVol2, newNode.stCanMap);
    fTemp = float(stFrame.inVol3) * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_inVol3, newNode.stCanMap);
    fTemp = float(stFrame.inVol4) * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_inVol4, newNode.stCanMap);

    fTemp = float(stFrame.curBranch1) * 0.01;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_curBranch1, newNode.stCanMap);
    fTemp = float(stFrame.curBranch2) * 0.01;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_curBranch2, newNode.stCanMap);
    fTemp = float(stFrame.curBranch3) * 0.01;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_curBranch3, newNode.stCanMap);
    fTemp = float(stFrame.curBranch4) * 0.01;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_curBranch4, newNode.stCanMap);
    fTemp = float(stFrame.curBranch5) * 0.01;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_curBranch5, newNode.stCanMap);
    fTemp = float(stFrame.curBranch6) * 0.01;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_curBranch6, newNode.stCanMap);
    fTemp = float(stFrame.curBranch7) * 0.01;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_curBranch7, newNode.stCanMap);
    fTemp = float(stFrame.curBranch8) * 0.01;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_curBranch8, newNode.stCanMap);
    fTemp = float(stFrame.curBranch9) * 0.01;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_curBranch9, newNode.stCanMap);
    fTemp = float(stFrame.curBranch10) * 0.01;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_curBranch10, newNode.stCanMap);
    fTemp = float(stFrame.curBranch11) * 0.01;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_curBranch11, newNode.stCanMap);
    fTemp = float(stFrame.curBranch12) * 0.01;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_curBranch12, newNode.stCanMap);
    fTemp = float(stFrame.curBranch13) * 0.01;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_curBranch13, newNode.stCanMap);
    fTemp = float(stFrame.curBranch14) * 0.01;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_curBranch14, newNode.stCanMap);
    fTemp = float(stFrame.curBranch15) * 0.01;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_curBranch15, newNode.stCanMap);
    fTemp = float(stFrame.curBranch16) * 0.01;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_curBranch16, newNode.stCanMap);
    fTemp = float(stFrame.realPower) * 0.01;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_realPower, newNode.stCanMap);
    fTemp = float(stFrame.radiatorTemp) * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_radiatorTemp, newNode.stCanMap);

    char fault1_bit0 = stFrame.fault1.bit0;
    char fault1_bit1 = stFrame.fault1.bit1;
    char fault1_bit2 = stFrame.fault1.bit2;
    char fault1_bit3 = stFrame.fault1.bit3;
    char fault1_bit4 = stFrame.fault1.bit4;
    char fault1_bit5 = stFrame.fault1.bit5;
    char fault1_bit6 = stFrame.fault1.bit6;
    char fault1_bit7 = stFrame.fault1.reserve;
    char reserve1 = 0;
    InsertCanInfoMapLine((char *)&fault1_bit0, sizeof(fault1_bit0), Addr_fault1_bit0, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&fault1_bit1, sizeof(fault1_bit1), Addr_fault1_bit1, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&fault1_bit2, sizeof(fault1_bit2), Addr_fault1_bit2, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&fault1_bit3, sizeof(fault1_bit3), Addr_fault1_bit3, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&fault1_bit4, sizeof(fault1_bit4), Addr_fault1_bit4, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&fault1_bit5, sizeof(fault1_bit5), Addr_fault1_bit5, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&fault1_bit6, sizeof(fault1_bit6), Addr_fault1_bit6, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&fault1_bit7, sizeof(fault1_bit7), Addr_fault1_reserve, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&reserve1, sizeof(reserve1), Addr_reserve1, newNode.stCanMap);
    char fault2_bit0 = stFrame.fault2.bit0;
    char fault2_bit1 = stFrame.fault2.bit1;
    char fault2_bit2 = stFrame.fault2.bit2;
    char fault2_bit3 = stFrame.fault2.bit3;
    char fault2_bit4 = stFrame.fault2.bit4;
    char fault2_bit5 = stFrame.fault2.bit5;
    char fault2_bit6 = stFrame.fault2.bit6;
    char fault2_bit7 = stFrame.fault2.bit7;
    char fault2_bit8 = stFrame.fault2.bit8;
    char fault2_bit9 = stFrame.fault2.bit9;
    char fault2_bit10 = stFrame.fault2.bit10;
    char fault2_bit11 = stFrame.fault2.bit11;
    char fault2_bit12 = stFrame.fault2.bit12;
    char fault2_bit13 = stFrame.fault2.bit13;
    char fault2_bit14 = stFrame.fault2.bit14;
    char fault2_bit15 = stFrame.fault2.bit15;

    InsertCanInfoMapLine((char *)&fault2_bit0, sizeof(fault2_bit0), Addr_fault2_bit0, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&fault2_bit1, sizeof(fault2_bit1), Addr_fault2_bit1, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&fault2_bit2, sizeof(fault2_bit2), Addr_fault2_bit2, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&fault2_bit3, sizeof(fault2_bit3), Addr_fault2_bit3, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&fault2_bit4, sizeof(fault2_bit4), Addr_fault2_bit4, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&fault2_bit5, sizeof(fault2_bit5), Addr_fault2_bit5, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&fault2_bit6, sizeof(fault2_bit6), Addr_fault2_bit6, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&fault2_bit7, sizeof(fault2_bit7), Addr_fault2_bit7, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&fault2_bit8, sizeof(fault2_bit8), Addr_fault2_bit8, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&fault2_bit9, sizeof(fault2_bit9), Addr_fault2_bit9, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&fault2_bit10, sizeof(fault2_bit10), Addr_fault2_bit10, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&fault2_bit11, sizeof(fault2_bit11), Addr_fault2_bit11, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&fault2_bit12, sizeof(fault2_bit12), Addr_fault2_bit12, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&fault2_bit13, sizeof(fault2_bit13), Addr_fault2_bit13, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&fault2_bit14, sizeof(fault2_bit14), Addr_fault2_bit14, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&fault2_bit15, sizeof(fault2_bit15), Addr_fault2_bit15, newNode.stCanMap);

    char warning_bit0 = stFrame.warning.bit0;
    char warning_bit1 = stFrame.warning.bit1;
    char warning_bit2 = stFrame.warning.bit2;
    char warning_bit3 = stFrame.warning.bit3;
    char warning_bit4 = stFrame.warning.bit4;
    char warning_bit5 = stFrame.warning.bit5;
    char warning_bit6 = stFrame.warning.bit6;
    char warning_bit7 = stFrame.warning.bit7;
    char warning_bit8 = stFrame.warning.bit8;
    char warning_bit9 = stFrame.warning.bit9;
    char warning_bit10 = stFrame.warning.bit10;
    char warning_bit11 = stFrame.warning.bit11;
    char warning_bit12 = stFrame.warning.bit12;
    char warning_bit13 = stFrame.warning.bit13;
    char warning_bit14 = stFrame.warning.bit14;
    char warning_bit15 = stFrame.warning.bit15;

    InsertCanInfoMapLine((char *)&warning_bit0, sizeof(warning_bit0), Addr_warning_bit0, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&warning_bit1, sizeof(warning_bit1), Addr_warning_bit1, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&warning_bit2, sizeof(warning_bit2), Addr_warning_bit2, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&warning_bit3, sizeof(warning_bit3), Addr_warning_bit3, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&warning_bit4, sizeof(warning_bit4), Addr_warning_bit4, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&warning_bit5, sizeof(warning_bit5), Addr_warning_bit5, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&warning_bit6, sizeof(warning_bit6), Addr_warning_bit6, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&warning_bit7, sizeof(warning_bit7), Addr_warning_bit7, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&warning_bit8, sizeof(warning_bit8), Addr_warning_bit8, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&warning_bit9, sizeof(warning_bit9), Addr_warning_bit9, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&warning_bit10, sizeof(warning_bit10), Addr_warning_bit10, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&warning_bit11, sizeof(warning_bit11), Addr_warning_bit11, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&warning_bit12, sizeof(warning_bit12), Addr_warning_bit12, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&warning_bit13, sizeof(warning_bit13), Addr_warning_bit13, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&warning_bit14, sizeof(warning_bit14), Addr_warning_bit14, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&warning_bit15, sizeof(warning_bit15), Addr_warning_bit15, newNode.stCanMap);


    InsertCanInfoMapLine((char *)&stFrame.combinerStatus, sizeof(stFrame.combinerStatus), Addr_combinerStatus, newNode.stCanMap);
    uiTemp = stFrame.softVer_L + (stFrame.softVer_H << 16);
    InsertCanInfoMapLine((char *)&uiTemp, sizeof(uiTemp), Addr_SoftVer_PO, newNode.stCanMap);
//    InsertCanInfoMapLine((char *)&stFrame.softVer_L, sizeof(stFrame.softVer_L), Addr_softVer_L, newNode.stCanMap);
//    InsertCanInfoMapLine((char *)&stFrame.softVer_H, sizeof(stFrame.softVer_H), Addr_softVer_H, newNode.stCanMap);
    char sysRequestStatus_bit0 = stFrame.sysRequestStatus.bit0;
    char sysRequestStatus_bit1 = stFrame.sysRequestStatus.bit1;
    char sysRequestStatus_reserve = stFrame.sysRequestStatus.reserve;
    InsertCanInfoMapLine((char *)&sysRequestStatus_bit0, sizeof(sysRequestStatus_bit0), Addr_sysRequestStatus_bit0, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&sysRequestStatus_bit1, sizeof(sysRequestStatus_bit1), Addr_sysRequestStatus_bit1, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&sysRequestStatus_reserve, sizeof(sysRequestStatus_reserve), Addr_sysRequestStatus_reserve, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&stFrame.reserve2, sizeof(stFrame.reserve2), Addr_reserve2, newNode.stCanMap);

    fTemp = (float)stFrame.inVol5 * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_inVol5, newNode.stCanMap);
    fTemp = (float)stFrame.inVol6 * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_inVol6, newNode.stCanMap);
    fTemp = (float)stFrame.inVol7 * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_inVol7, newNode.stCanMap);
    fTemp = (float)stFrame.inVol8 * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_inVol8, newNode.stCanMap);

    fTemp = (float)stFrame.outVol * 0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_outVol, newNode.stCanMap);

    newNode.enType = AddrType_PO_Mod_Info;//主题
    ToCenterList.append(newNode);
}

//解析模块故障信息
void cEnergyPlanProtocol::ParseFrameModuleFaultInfo(unsigned char * pData, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;
    //    unsigned char ucFaultState = 0;

    //解帧
    FrameModuleFaultInfo_EP strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));
    //    if(strFrame.ucFaultState == 0x55)
    //    {
    //        ucFaultState = 1;
    //    }
    //添加告警模块地址
    InsertCanInfoMapLine((char *)&strFrame.ucModuleID, sizeof(strFrame.ucModuleID), Addr_DevID_DC_Comm, newNode.stCanMap);
    //添加告警码
    InsertCanInfoMapLine((char *)&strFrame.ucAlarmCode, sizeof(strFrame.ucAlarmCode), Addr_DCcabFaultCode, newNode.stCanMap);
    //添加最小PDU地址
    InsertCanInfoMapLine((char *)&strFrame.ucMinPDUID, sizeof(strFrame.ucMinPDUID), Addr_DCcabMinPDU_ID, newNode.stCanMap);
    //添加最大PDU地址
    InsertCanInfoMapLine((char *)&strFrame.ucMaxPDUID, sizeof(strFrame.ucMaxPDUID), Addr_DCcabMaxPDU_ID, newNode.stCanMap);
    //添加故障状态
    InsertCanInfoMapLine((char *)&strFrame.ucFaultState, sizeof(strFrame.ucFaultState), Addr_DCcabFaultState, newNode.stCanMap);

    newNode.enType = AddrType_FaultState_DCcab;
    ToCenterList.append(newNode);
}

//解析能效系统模块控制<----控制中心
void cEnergyPlanProtocol::ParseCtrlModule_EP(InfoMap CenterMap, unsigned char ucTermID)
{
    SendFrameCtrlModuleWork(CenterMap, ucTermID);
}

//解析能效系统功率控制<----控制中心
void cEnergyPlanProtocol::ParseSetPower_EP(InfoMap CenterMap, unsigned char ucTermID)
{
    SendFrameSetModulePower(CenterMap, ucTermID);
}

//解析长帧
void cEnergyPlanProtocol::ParseLongFrame(FrameLongPackage * pLongPackage)
{
    QList <CanMapNode> ToCenterList;
    QByteArray canAddrArray;
    canAddrArray.append(pLongPackage->ucTermID);

    switch(pLongPackage->ucPGN[1])
    {
    case  PF_ACDCInfo1://四象限变换柜AC/DC信息１
    case PF_ACDCInfo2://独立逆变柜ACDC
      ParseACDCInfo1(pLongPackage, ToCenterList,PF_ACDCInfo1,pLongPackage->ucTermID);
      break;
    case PF_PowerOptimizerCabinetInfo1://功率优化器信息１
        ParsePowerOptimizerInfo1(pLongPackage, ToCenterList,PF_PowerOptimizerCabinetInfo1,pLongPackage->ucTermID);
        break;
    case PF_EnergyStorageCabinetBatteryInfo1://储能电池信息１
        ParseEnergyStorageCabinetBatteryInfo(pLongPackage, ToCenterList,PF_EnergyStorageCabinetBatteryInfo1,pLongPackage->ucTermID);
        break;
    case  PF_DCDCInfo1://充放电柜DC/DC模块信息
        ParseChargeDisChargeCabinetDCDCInfo(pLongPackage, ToCenterList,PF_DCDCInfo1,pLongPackage->ucTermID);
        break;
    case  PF_DCDCInfo2://储能柜DC/DC模块信息
        ParseEnergyStorageCabinetDCDCInfo(pLongPackage, ToCenterList,PF_DCDCInfo2,pLongPackage->ucTermID);
        break;
    default:
        break;
    }
    //有数据发送
    for(unsigned char i = 0; i < ToCenterList.count(); i++)
    {
        ToCenterList.at(i).stCanMap.insert(Addr_CanID_Comm, canAddrArray);//添加CAN地址
        SendCenterData(ToCenterList.at(i).enType, ToCenterList.at(i).stCanMap);
    }
}
