#include "TerminalProtocol.h"

cTerminalProtocol::cTerminalProtocol()
{
}
//是否有帧需要发送
bool cTerminalProtocol::HasFrameToSend()
{
    return FALSE;
}

//是否有帧需要处理
bool cTerminalProtocol::HasFrameToDeal()
{
    return FALSE;
}

//解析控制中心数据--总函数
void cTerminalProtocol::ParseCenterData(InfoMap CenterMap, InfoAddrType enAddrType, unsigned char ucTermID)
{
    switch(enAddrType)
    {
    case AddrType_CmdCtrl_Apply://能效设备控制临时添加
    {
        if(ucTermID == 147 || ucTermID == 148|| (ucTermID>=117 && ucTermID<=123))
        {
           ParseTelecontrol(CenterMap, ucTermID);
        }
        break;
    }
    case AddrType_MaxLoad:
    {
        ParseLimitPower(CenterMap,ucTermID);
//        ParseConrolEMSBreaker(CenterMap, ucTermID);对EMS断路器控制，另加主题
        break;
    }
    case AddrType_CmdCtrl: //解析遥控指令
    {
        ParseTelecontrol(CenterMap, ucTermID);
        break;
    }
    case AddrType_TermAdjustment:   //解析遥调指令
    {
        ParseTeleadjust(CenterMap, ucTermID);
        break;
    }
    case AddrType_Ctrl_Module_EP: //能效系统，模块控制主题
    {
        break;
    }
    case AddrType_Set_Power_EP: //能效系统，功率设置主题
    {
        break;
    }
    case AddrType_ApplyAccountInfoResult_ToChargeEquipment:   //主题七：充电服务返回账户信息给充电设备
    {
        ParseAccountInfoList(CenterMap, ucTermID);
        break;
    }
    case AddrType_InApplyStartChargeResult_ToChargeEquipment:   //主题十一：内部申请开始充电结果至充电设备
    case AddrType_InApplyStopChargeResult_ToChargeEquipment:    //主题十九：内部申请结束充电结果至充电设备
    {
        ParseCardInResult(CenterMap, ucTermID);
        break;
    }
    case AddrType_OutApplyStartChargeResult_ToChargeEquipment:  //主题十五：远程申请开始充电结果至充电设备
    case AddrType_OutApplyStopChargeResult_ToChargeEquipment:   //主题二十三：远程申请结束充电结果至充电设备
    {
        ParseCardOutResult(CenterMap, ucTermID);
        break;
    }
    case AddrType_LimitChargeCurrent:   //错峰充电限制充电电流。负荷调度模块发布，充电设备模块订阅
    {
        ParseLimitChargeCurrent(CenterMap, ucTermID);
        break;
    }
    case AddrType_UpdatePackDir_Dev: //模块升级下载已经完成
    {
        ParseMoudleUpdateDir(CenterMap, ucTermID);
        break;
    }
    case AddrType_ActiveProtectSet: //主动防护功能设置, 设备管理模块发布, 充电设备模块订阅
    {
        ParseActiveProtect(CenterMap, ucTermID);
        break;
    }
    case AddrType_FlexibleChargeSet: //柔性充电功能设置, 设备管理模块发布, 充电设备模块订阅
    {
        ParseFlexibleCharge(CenterMap, ucTermID);
        break;
    }
    case AddrType_GeneralStaticArgSet:  //通用静态参数设置, 设备管理模块发布, 充电设备模块订阅
    {
        ParseGeneralStaticArg(CenterMap, ucTermID);
        break;
    }
    case AddrType_GeneralDynamicArgSet:  //通用动态参数设置, 设备管理模块发布, 充电设备模块订阅
    {
        ParseGeneralDynamicArg(CenterMap, ucTermID);
        break;
    }
    case AddrType_CCUArgSet:    //CCU参数设置, 设备管理模块发布, 充电设备模块订阅
    {
        ParseCCUArg(CenterMap, ucTermID);
        break;
    }
    case AddrType_ActiveProtectQuery: //主动防护功能设置查询, 设备管理模块发布, 充电设备模块订阅
    {
        ParseQueryActiveProtect(CenterMap, ucTermID);
        break;
    }
    case AddrType_FlexibleChargeQuery: //柔性充电功能设置查询, 设备管理模块发布, 充电设备模块订阅
    {
        ParseQueryFlexibleCharge(CenterMap, ucTermID);
        break;
    }
    case AddrType_GeneralStaticArgQuery:  //通用静态参数设置查询, 设备管理模块发布, 充电设备模块订阅
    {
        ParseQueryGeneralStaticArg(CenterMap, ucTermID);
        break;
    }
    case AddrType_GeneralDynamicArgQuery:  //通用动态参数设置查询, 设备管理模块发布, 充电设备模块订阅
    {
        ParseQueryGeneralDynamicArg(CenterMap, ucTermID);
        break;
    }
    case AddrType_CCUArgQuery:  //CCU参数设置查询, 其他模块发布, 充电设备模块订阅
    {
        ParseCCUArgQuery(CenterMap, ucTermID);
        break;
    }
    case AddrType_Response_Result_IN:    //内部数据项响应结果
        ParseChargeTypeReault(CenterMap, ucTermID);
        break;
    case AddrType_ChargeGunGroup_Info_IN:     //内部多枪分组信息下发 充电服务发布，充电设备订阅
        ParseChargeGunGroupInfo(CenterMap, ucTermID);
        break;
    case AddrType_DoubleSys300kwSetting_Publish:  //下发直流充电终端双系统300kw参数设置
        ParseDoubleSys300kwSetting(CenterMap, ucTermID);
        break;
    default:
        break;
    }
}

void cTerminalProtocol::InsertCanInfoMapLine(char * pData, int iDataLength, unsigned int uiInfoAddr, InfoMap &ToCenterMap)
{
    //定义终端协议传递结构体
    QByteArray termArray;
    termArray.append(pData,iDataLength);
    ToCenterMap.insert(uiInfoAddr, termArray);
}


//解析主动防护设置<----控制中心
//void cTerminalProtocol::ParseActiveProtect(InfoMap CenterMap, unsigned char ucTermID)
//{
//    if(!CenterMap.isEmpty())
//    {
//        if(ucTermID!=0)
//        {
//            ;
//        }
//    }
//}

//解析柔性充电设置<----控制中心
//void cTerminalProtocol::ParseFlexibleCharge(InfoMap CenterMap, unsigned char ucTermID)
//{
//    if(!CenterMap.isEmpty())
//    {
//        if(ucTermID!=0)
//        {
//            ;
//        }
//    }
//}

//解析通用静态参数设置<----控制中心
//void cTerminalProtocol::ParseGeneralStaticArg(InfoMap CenterMap, unsigned char ucTermID)
//{
//    if(!CenterMap.isEmpty())
//    {
//        if(ucTermID!=0)
//        {
//            ;
//        }
//    }
//}

//解析通用动态参数设置<----控制中心
//void cTerminalProtocol::ParseGeneralDynamicArg(InfoMap CenterMap, unsigned char ucTermID)
//{
//    if(!CenterMap.isEmpty())
//    {
//        if(ucTermID!=0)
//        {
//            ;
//        }
//    }
//}

//发送帧到对应的通信端口, 各子类向各自关联的通信接口发送信号
void cTerminalProtocol::SendEquipData()
{
    ;
}

//发送帧到对应控制中心
void cTerminalProtocol::SendCenterData(unsigned int &uiInfoAddr, InfoMap &TermMap)
{
    emit sigSendToCenter(uiInfoAddr ,TermMap);
}

void cTerminalProtocol::ParseChargeGunGroupInfo(InfoMap CenterMap, unsigned char ucTermID)
{
    Q_UNUSED(CenterMap);
    Q_UNUSED(ucTermID);
}

void cTerminalProtocol::ParseDoubleSys300kwSetting(InfoMap CenterMap, unsigned char ucTermID)
{
    Q_UNUSED(CenterMap);
    Q_UNUSED(ucTermID);
}
