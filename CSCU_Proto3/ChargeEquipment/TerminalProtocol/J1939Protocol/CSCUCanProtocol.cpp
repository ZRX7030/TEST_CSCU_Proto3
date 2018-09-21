#include "CSCUCanProtocol.h"
#include "DevCache/DevCache.h"

cCSCUCanProtocol::cCSCUCanProtocol()
{
    pTerminalSendList = new QList <can_frame *> ;//终端CAN帧发送列表
    pSendListMutex = new QMutex;//发送列表操作锁
    pModuleMap = new ModuleMap;    //CAN地址--长包模块
    pModuleMapToSend = new ModuleMap(); //准备发送长帧图(主动下发)
    pModuleSendMapMutex = new QMutex(); //发送长包操作锁
    pModuleMapMutex = new QMutex(); //发送长包MAP锁
    ucLatestNetworkStatus = 0;
}

cCSCUCanProtocol::~cCSCUCanProtocol()
{
    delete pTerminalSendList ;//终端CAN帧发送列表
    delete pSendListMutex; //发送列表操作锁
    pModuleMap->clear();
    delete pModuleMap; //CAN地址--长包模块
    delete pModuleMapMutex; //长包模块操作锁
    delete pModuleMapToSend;
    delete pModuleSendMapMutex;
}

//接收CAN终端接收的数据并解析
void cCSCUCanProtocol::ParseCanData(can_frame *pCanFrame)
{
    //传递给充电设备类图
    QList <CanMapNode> ToCenterList;
    QByteArray CanAddrArray;
    //解析帧头
    FrameHead strFrameHead;
    memcpy((unsigned char *)&strFrameHead, (unsigned char *)&pCanFrame->can_id, sizeof(strFrameHead));

    //添加CAN地址节点
    CanAddrArray.append(strFrameHead.ucPs);

    //解析其他内容
    switch(strFrameHead.ucPf)
    {
    case PF_AccBAckInfo:
    {
        break;
    }
    case PF_ProVerInfo:
    {
        break;
    }
    case PF_ParamAck:
    {
        break;
    }
    case PF_CardNumber:
    {
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
    case PF_StartCharge:
    {
        break;
    }
    case PF_PauseCharge:   //暂停，排队
    {
        break;
    }
    case PF_BillingPolicy:
    {
        break;
    }
    case PF_CtrlCmdAck:
    {
        break;
    }
    case PF_UpdateRequestAck:
    {
        break;
    }
    case PF_UpadateManage:
    {
        break;
    }
    case PF_ProgramRecvFinsh:
    {
        break;
    }
    case PF_PackageRecvAck:
    {
        break;
    }
    case PF_UpdateAck:
    {
        break;
    }
    case PF_LinkManage:
    {
        break;
    }
    case PF_DataTran:
    {
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




//是否有帧需要处理
bool cCSCUCanProtocol::HasFrameToDeal()
{
    CheckBroadCastState();
    if(HasLongFrameToDeal() == TRUE)
    {
        return TRUE;
    }
    return FALSE;
}

void cCSCUCanProtocol::CheckBroadCastState()
{
//    //定时广播
//    if(uiBroadCastCount%ucBroadCastInterval == 0)
//    {
//        SendFrameAppyPGN(ID_BroadCastCanID,PF_ChargerState1_DC);
//        SendFrameAppyPGN(ID_BroadCastCanID,PF_ChargerState2_DC);
//        SendFrameAppyPGN(ID_BroadCastCanID,PF_ChargerState3_DC);
//    }

//    if(uiBroadCastCount%(ucBroadCastInterval * 60) == 0)
//    {
//        for(int i = 0;i < 10;i++)
//        {
//            if(ucCCUNum[i] == 0)
//                continue;
//            else if(ucCCUNum[i] >= 231 && ucCCUNum[i] <= 240)
//                SendFrameAppyPGN(ucCCUNum[i],PF_SpecificInfo);
//        }
//    }

    //定时对时
    if(uiBroadCastCount%(ucBroadCastInterval*60) == 0)
    {
        SendFrameTimeSync(ID_BroadCastCanID);
    }
    //向副集控发送联网状态
    unsigned char ucNetworkStatus = GetNetworkStatus();
    if(ucNetworkStatus != ucLatestNetworkStatus || \
            uiBroadCastCount % 6000 == 0)//10min
    {
        ucLatestNetworkStatus = ucNetworkStatus;
        SendFrameNetworkStatus(ucLatestNetworkStatus);
    }
    uiBroadCastCount++;
}

unsigned char cCSCUCanProtocol::GetNetworkStatus()
{
    unsigned char ucNetworkStatus = 0;
    QVariant var, param;
    if(DevCache::GetInstance()->QueryRealStatusMeter(var, CACHE_STATUS, param))
    {
        RealStatusData realData = var.value<RealStatusData>();
        if(realData.connectStatus && !realData.emergencyStatus)
            ucNetworkStatus = 0x01;
        else if(!realData.connectStatus && !realData.emergencyStatus)
            ucNetworkStatus = 0x02;
        else if(!realData.connectStatus && realData.emergencyStatus)
            ucNetworkStatus = 0x03;
        else if(realData.connectStatus && realData.emergencyStatus)
            ucNetworkStatus = 0x04;
    }
    return ucNetworkStatus;
}

//发送工作状态帧
void cCSCUCanProtocol::SendFrameNetworkStatus(unsigned char ucNetworkStatus)
{
    can_frame * pCanFrame = new(can_frame);
    MakeFrameNetworkStatus(pCanFrame, 0xFF, ucNetworkStatus);
    pSendListMutex->lock();
    pTerminalSendList->append(pCanFrame);
    pSendListMutex->unlock();
}

//生成--工作状态帧
void cCSCUCanProtocol::MakeFrameNetworkStatus(can_frame *pCanFrame, unsigned char ucCanID, unsigned char ucNetworkStatus)
{
    SetFrameHead(pCanFrame, ID_DefaultControlCenterCanID, PF_CSCUNetworkStatus, ucCanID, PriorityDefault, DL_NetworkStatus);
    memcpy(pCanFrame->data, &ucNetworkStatus,sizeof(ucNetworkStatus));
}

//解析主动防护设置<----控制中心
void cCSCUCanProtocol::ParseActiveProtect(InfoMap CenterMap, unsigned char ucTermID)
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
void cCSCUCanProtocol::ParseFlexibleCharge(InfoMap CenterMap, unsigned char ucTermID)
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
void cCSCUCanProtocol::ParseGeneralStaticArg(InfoMap CenterMap, unsigned char ucTermID)
{
    if(!CenterMap.isEmpty())
    {
        if(ucTermID!=0)
        {
            ;
        }
    }
}

//解析直流单桩的基本参数设置数据add by zrx
void cCSCUCanProtocol::ParseCenterDataSP(InfoMap CenterMap,unsigned char ucTermID)
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
void cCSCUCanProtocol::ParseGeneralDynamicArg(InfoMap CenterMap, unsigned char ucTermID)
{
    if((!CenterMap.isEmpty())||(ucTermID!=0))
    {
        ;
    }
}

//解析查询主动防护设置<----控制中心
void cCSCUCanProtocol::ParseQueryActiveProtect(InfoMap CenterMap, unsigned char ucTermID)
{
    if((!CenterMap.isEmpty())||(ucTermID!=0))
    {
        ;
    }
}

//解析查询柔性充电设置<----控制中心
void cCSCUCanProtocol::ParseQueryFlexibleCharge(InfoMap CenterMap, unsigned char ucTermID)
{
    if((!CenterMap.isEmpty())||(ucTermID!=0))
    {
        ;
    }
}

//解析查询通用静态参数设置<----控制中心
void cCSCUCanProtocol::ParseQueryGeneralStaticArg(InfoMap CenterMap, unsigned char ucTermID)
{
    if((!CenterMap.isEmpty())||(ucTermID!=0))
    {
        ;
    }
}
//解析查询通用动态参数设置<----控制中心
void cCSCUCanProtocol::ParseQueryGeneralDynamicArg(InfoMap CenterMap, unsigned char ucTermID)
{
    if((!CenterMap.isEmpty())||(ucTermID!=0))
    {
        ;
    }
}

//解析CCU参数设置<----控制中心
void cCSCUCanProtocol::ParseCCUArg(InfoMap CenterMap, unsigned char ucTermID)
{
    if((!CenterMap.isEmpty())||(ucTermID!=0))
    {
        ;
    }
}

//解析CCU参数查询<----控制中心
void cCSCUCanProtocol::ParseCCUArgQuery(InfoMap CenterMap, unsigned char ucTermID)
{
    if((!CenterMap.isEmpty())||(ucTermID!=0))
    {
        ;
    }
}

//解析能效系统模块控制<----控制中心
void cCSCUCanProtocol::ParseCtrlModule_EP(InfoMap CenterMap, unsigned char ucTermID)
{
    if((!CenterMap.isEmpty())||(ucTermID!=0))
    {
        ;
    }
}
//解析能效系统功率控制<----控制中心
void cCSCUCanProtocol::ParseSetPower_EP(InfoMap CenterMap, unsigned char ucTermID)
{
    if((!CenterMap.isEmpty())||(ucTermID!=0))
    {
        ;
    }
}
//解析能效系统功率控制<----控制中心
void cCSCUCanProtocol::ParseLongFrame(FrameLongPackage * pLongPackage)
{

}

//解析能效系统功率控制<----控制中心
int cCSCUCanProtocol::GetParamAckType(unsigned char ucPF)
{

}

//解析遥调指令<----控制中心
void cCSCUCanProtocol::ParseTeleadjust(InfoMap CenterMap, unsigned char ucTermID)
{
    ucTermID = 0;
    CenterMap.clear();
}

//获取协议版本号枚举
void cCSCUCanProtocol::GetProVerEnum(unsigned char * pVer)
{
//    if( (pVer[0] == 6)&&(pVer[1] == 1)&&(pVer[2] == 1) )
//    {
//        this->ucProVer = ProVer_1_1_6_AC_Sin;
//    }
//    else
//    {
//        this->ucProVer = ProVer_Old_AC_Sin;
//    }
}
