#include "ACSinCanProtocol.h"

cACSinCanProtocol::cACSinCanProtocol()
{
    uiBroadCastCount = 0;
    pTerminalSendList = new QList <can_frame *> ;//终端CAN帧发送列表
    pSendListMutex = new QMutex;//发送列表操作锁
    pModuleMap = new ModuleMap;    //CAN地址--长包模块
    pModuleMapToSend = new ModuleMap(); //准备发送长帧图(主动下发)
    pModuleSendMapMutex = new QMutex(); //发送长包操作锁
    pModuleMapMutex = new QMutex(); //发送长包MAP锁

}

cACSinCanProtocol::~cACSinCanProtocol()
{
    delete pTerminalSendList ;//终端CAN帧发送列表
    delete pSendListMutex; //发送列表操作锁
    pModuleMap->clear();
    delete pModuleMap; //CAN地址--长包模块
    delete pModuleMapMutex; //长包模块操作锁
    delete pModuleMapToSend;
    delete pModuleSendMapMutex;
}

//解析遥调指令<----控制中心
void cACSinCanProtocol::ParseTeleadjust(InfoMap CenterMap, unsigned char ucTermID)
{
    ucTermID = 0;
    CenterMap.clear();
    Q_UNUSED(ucTermID);
}

//获取参数设置结果类型
int cACSinCanProtocol::GetParamAckType(unsigned char ucPF)
{
    int iRet = 0;
    switch(ucPF)
    {
    case PF_DetailParamSet_ACSin:
        iRet = AddrType_DetailParamSetResult_ACSin;
        break;
    default:
        iRet = AddrType_DetailParamSetResult_ACSin;
        break;
    }
    return iRet;
}

//获取协议版本号枚举
void cACSinCanProtocol::GetProVerEnum(unsigned char * pVer)
{
    if( (pVer[0] == 6)&&(pVer[1] == 1)&&(pVer[2] == 1) )
    {
        this->ucProVer = ProVer_1_1_6_AC_Sin;
    }
    else
    {
        this->ucProVer = ProVer_Old_AC_Sin;
    }
}

//是否有帧需要处理
bool cACSinCanProtocol::HasFrameToDeal()
{
    CheckBroadCastState();
    if(HasLongFrameToDeal() == TRUE)
    {
        return TRUE;
    }
    return FALSE;
}

void cACSinCanProtocol::CheckBroadCastState()
{
    //定时广播  1min
    if(uiBroadCastCount%(ucBroadCastInterval * 6) == 0)
    {
        for(int i = 0;i < ucACsingle;i++)
        {
            SendFrameAppyPGN(i + 1,PF_SpecificInfo);
        }
        uiBroadCastCount = 0;
    }
    uiBroadCastCount++;
}

//接收CAN终端接收的数据并解析
void cACSinCanProtocol::ParseCanData(can_frame *pCanFrame)
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
    switch(strFrameHead.ucPf)
    {
    case PF_ProVerInfo:
    {
        ParseFrameProVer(pCanFrame->data, strFrameHead.ucSa);
        break;
    }
    case PF_ChargerState1_ACSin:
    {
        ParseFrameState1(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_ChargerState2_ACSin:
    {
        ParseFrameState2(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_ChargerState3_ACSin:
    {
        ParseFrameState3(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_ChargerState5_ACSin:
    {
        ParseFrameState5(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_StopReason_ACSin:
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
    //有数据发送
    for(unsigned char i = 0 ; i < ToCenterList.count(); i++)
    {
        ToCenterList.at(i).stCanMap.insert(Addr_CanID_Comm, CanAddrArray);
        SendCenterData(ToCenterList.at(i).enType, ToCenterList.at(i).stCanMap);
    }
}

//解析主动防护设置<----控制中心
void cACSinCanProtocol::ParseActiveProtect(InfoMap CenterMap, unsigned char ucTermID)
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
void cACSinCanProtocol::ParseFlexibleCharge(InfoMap CenterMap, unsigned char ucTermID)
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
void cACSinCanProtocol::ParseGeneralStaticArg(InfoMap CenterMap, unsigned char ucTermID)
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
void cACSinCanProtocol::ParseGeneralDynamicArg(InfoMap CenterMap, unsigned char ucTermID)
{
    if((!CenterMap.isEmpty())||(ucTermID!=0))
    {
        ;
    }
}

//解析查询主动防护设置<----控制中心
void cACSinCanProtocol::ParseQueryActiveProtect(InfoMap CenterMap, unsigned char ucTermID)
{
    if((!CenterMap.isEmpty())||(ucTermID!=0))
    {
        ;
    }
}

//解析查询柔性充电设置<----控制中心
void cACSinCanProtocol::ParseQueryFlexibleCharge(InfoMap CenterMap, unsigned char ucTermID)
{
    if((!CenterMap.isEmpty())||(ucTermID!=0))
    {
        ;
    }
}

//解析查询通用静态参数设置<----控制中心
void cACSinCanProtocol::ParseQueryGeneralStaticArg(InfoMap CenterMap, unsigned char ucTermID)
{
    if((!CenterMap.isEmpty())||(ucTermID!=0))
    {
        ;
    }
}
//解析查询通用动态参数设置<----控制中心
void cACSinCanProtocol::ParseQueryGeneralDynamicArg(InfoMap CenterMap, unsigned char ucTermID)
{
    if((!CenterMap.isEmpty())||(ucTermID!=0))
    {
        ;
    }
}

//解析CCU参数设置<----控制中心
void cACSinCanProtocol::ParseCCUArg(InfoMap CenterMap, unsigned char ucTermID)
{
    if((!CenterMap.isEmpty())||(ucTermID!=0))
    {
        ;
    }
}

//解析CCU参数查询<----控制中心
void cACSinCanProtocol::ParseCCUArgQuery(InfoMap CenterMap, unsigned char ucTermID)
{
    if((!CenterMap.isEmpty())||(ucTermID!=0))
    {
        ;
    }
}

//解析能效系统模块控制<----控制中心
void cACSinCanProtocol::ParseCtrlModule_EP(InfoMap CenterMap, unsigned char ucTermID)
{
    if((!CenterMap.isEmpty())||(ucTermID!=0))
    {
        ;
    }
}
//解析能效系统功率控制<----控制中心
void cACSinCanProtocol::ParseSetPower_EP(InfoMap CenterMap, unsigned char ucTermID)
{
    if((!CenterMap.isEmpty())||(ucTermID!=0))
    {
        ;
    }
}

//解析长帧
void cACSinCanProtocol::ParseLongFrame(FrameLongPackage * pLongPackage)
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

//解析充电机状态1
void cACSinCanProtocol::ParseFrameState1(unsigned char * pData, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;

    //定义相关数据
    float fChargeVoltage = 0;//充电电压
    float fChargeCurrent = 0;//充电电压
    float fActivePower = 0;//有功功率
    unsigned char ucWorkState = 0;//工作状态

    //解帧
    FrameChargerState1_ACSin strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));
    fChargeVoltage =  (float)strFrame.sChargeVoltage / 100.0;
    fChargeCurrent =  0 - (float)strFrame.sChargeCurrent / 100.0;
    fActivePower = (float)strFrame.usActivePower/1000.0;//单位kw,精度0.001kw
//    ucWorkState = (unsigned char)strFrame.usWorkState;
    ucWorkState = strFrame.usWorkState;

    //添加充电电压
    InsertCanInfoMapLine((char *)&fChargeVoltage, sizeof(fChargeVoltage), Addr_ACSinVoltage_Term, newNode.stCanMap);
    //添加充电电流
    InsertCanInfoMapLine((char *)&fChargeCurrent, sizeof(fChargeCurrent), Addr_ACSinCurrent_Term, newNode.stCanMap);
    //添加有功功率
    InsertCanInfoMapLine((char *)&fActivePower, sizeof(fActivePower), Addr_TotalActivePower_Term, newNode.stCanMap);
    newNode.enType = AddrType_TermMeasure;
    ToCenterList.append(newNode);

    newNode.stCanMap.clear();

    //添加充电机状态
    InsertCanInfoMapLine((char *)&ucWorkState, sizeof(ucWorkState), Addr_WorkState_Term, newNode.stCanMap);
    newNode.enType = AddrType_TermSignal;
    ToCenterList.append(newNode);
}

//解析充电机状态2
void cACSinCanProtocol::ParseFrameState2(unsigned char * pData, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;
    //定义相关数据
    unsigned char ucFaultInfo = 0;//故障状态
    unsigned char ucInterFaceType = 0;//充电接口标识
    unsigned char ucLinkState = 0;//连接确认开关状态 0关 1开
    unsigned char ucRelyState = 0;//输出继电器状态   0关  1开
    uint uiForwordActiveEnergy = 0;//正向有功总电能
    //解帧
    FrameChargerState2_ACSin strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    ucFaultInfo = (unsigned char) strFrame.usFaultInfo;
    ucInterFaceType = (unsigned char) strFrame.stByte7.ucInterFaceType;
    ucLinkState = CheckLinkState((unsigned char) strFrame.stByte7.ucLinkState);
    ucRelyState = (unsigned char) strFrame.stByte7.ucRelyState;
    uiForwordActiveEnergy = strFrame.uiForwordActiveEnergy;

    //添加故障状态
    InsertCanInfoMapLine((char *)&ucFaultInfo, sizeof(ucFaultInfo), Addr_FaultCode_Term, newNode.stCanMap);
    //添加充电接口标识
    InsertCanInfoMapLine((char *)&ucInterFaceType, sizeof(ucInterFaceType), Addr_ChargeGunNum_Term, newNode.stCanMap);
    //添加连接确认开关状态
    InsertCanInfoMapLine((char *)&ucLinkState, sizeof(ucLinkState), Addr_LinkState_Term, newNode.stCanMap);
    //添加输出继电器状态
    InsertCanInfoMapLine((char *)&ucRelyState, sizeof(ucRelyState), Addr_RelyState_Term, newNode.stCanMap);
    newNode.enType = AddrType_TermSignal;
    ToCenterList.append(newNode);

    newNode.stCanMap.clear();
    //添加正向有功总电能到终端数据图
    InsertCanInfoMapLine((char *)&uiForwordActiveEnergy,sizeof(uiForwordActiveEnergy),  Addr_TotalActiveEnergy_Term, newNode.stCanMap);
    newNode.enType = AddrType_TermMeasure;
    ToCenterList.append(newNode);
}

//解析充电机状态3
void cACSinCanProtocol::ParseFrameState3(unsigned char * pData, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;
    //定义相关数据
    unsigned int uiChargeTime;//充电时间 min
    float fReactivePower;//无功功率
    float fPowerFactor;//功率因数
    float fZeroLineCurrent;//零线电流

    //解帧
    FrameChargerState3_ACSin strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    uiChargeTime = strFrame.usChargeTime;
    fReactivePower = (float)strFrame.sReactivePower/10.0;
    fPowerFactor = (float)strFrame.usPowerFactor/1000.0 - 1;
    fZeroLineCurrent = (float)(4000 - strFrame.usZeroLineCurrent) /10.0;

    //添加充电时间
    InsertCanInfoMapLine((char *)&uiChargeTime, sizeof(uiChargeTime),  Addr_ChargeTime_Term, newNode.stCanMap);
    //添加无功功率
    InsertCanInfoMapLine((char *)&fReactivePower, sizeof(fReactivePower),  Addr_TotalReactivePower_Term, newNode.stCanMap);
    //添加功率因数
    InsertCanInfoMapLine((char *)&fPowerFactor, sizeof(fPowerFactor),  Addr_TotalPowerFactor_Term, newNode.stCanMap);
    //添加零线电流
    InsertCanInfoMapLine((char *)&fZeroLineCurrent, sizeof(fZeroLineCurrent),  Addr_ZeroLineCurrent_Term, newNode.stCanMap);

    newNode.enType = AddrType_TermMeasure;
    ToCenterList.append(newNode);
}

//解析充电机状态5
void cACSinCanProtocol::ParseFrameState5(unsigned char * pData, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;
    //定义相关数据
    uint uiReverseActiveEnergy;//反向有功总电能
    uint uiReverseReactiveEnergy;//反向无功总电能

    //解帧
    FrameChargerState5_ACSin strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    uiReverseActiveEnergy = strFrame.uiReverseActiveEnergy;
    uiReverseReactiveEnergy = strFrame.uiReverseReactiveEnergy;
    //添加反向有功总电能
    InsertCanInfoMapLine((char *)&uiReverseActiveEnergy, sizeof(uiReverseActiveEnergy), Addr_TotalReverseActiveEnergy_Term, newNode.stCanMap);
    //添加反向无功总电能
    InsertCanInfoMapLine((char *)&uiReverseReactiveEnergy, sizeof(uiReverseReactiveEnergy), Addr_TotalReverseReactiveEnergy_Term, newNode.stCanMap);

    newNode.enType = AddrType_TermMeasure;
    ToCenterList.append(newNode);
   }

//解析充电中止原因
void cACSinCanProtocol::ParseStopReason(unsigned char * pData, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;

    //解帧
    FrameStopReason_ACSin strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));
    char code = 0;
    switch(strFrame.ucCode)
    {
    case 1:
        code = 2;
        break;
    case 2:
        code = 41;
        break;
    case 3:
        code =  17;
        break;
    case 4:
        code = 32;
        break;
    case  5:
        code = 33;
        break;
    case 6:
        code =  34;
        break;
    case 7:
        code = 35;
        break;
    case 8:
        code = 36;
        break;
    case 9:
        code = 37;
        break;
    case 0x0a:
        code = 38;
        break;
    case  0x0b:
        code = 39;
        break;
    case 0x0c:
        code =  42;
        break;
    default:
        break;
    }

    //添加充电中止原因
    InsertCanInfoMapLine((char *)&(code), sizeof(code), Addr_ChargeEndCode_Term, newNode.stCanMap);
    newNode.enType = AddrType_TermSignal;
    ToCenterList.append(newNode);

}
