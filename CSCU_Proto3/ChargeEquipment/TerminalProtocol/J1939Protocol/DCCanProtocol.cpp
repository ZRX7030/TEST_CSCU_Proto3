#include "DCCanProtocol.h"

cDCCanProtocol::cDCCanProtocol()
{
    //广播召唤设备状态计数器
    uiBroadCastCount = 0;
    memset(ucCCUNum,0,10);
    //    ucProVer = ProVer_Old_DC;
    ucProVer = ProVer_2_0_19_DC;
    bCheckProVerFlag = FALSE;//校验协议标志位: 未校验完成
    pTerminalSendList = new QList <can_frame *> ;//终端CAN帧发送列表
    pSendListMutex = new QMutex();//发送列表操作锁
    pModuleMap = new ModuleMap();    //CAN地址--长包模块
    pModuleMapToSend = new ModuleMap(); //准备发送长帧图(主动下发)
    pModuleSendMapMutex = new QMutex(); //发送长包操作锁
    pModuleMapMutex = new QMutex(); //发送长包MAP锁
}

cDCCanProtocol::~cDCCanProtocol()
{
    delete pTerminalSendList ;//终端CAN帧发送列表
    delete pSendListMutex; //发送列表操作锁
    pModuleMap->clear();
    delete pModuleMap; //CAN地址--长包模块
    delete pModuleMapMutex; //长包模块操作锁
    delete pModuleMapToSend;
    delete pModuleSendMapMutex;
}

unsigned char cDCCanProtocol::CheckLinkState(unsigned char ucLinkStateIn)
{
    unsigned char ucRet = 0;
    switch(ucLinkStateIn)
    {
    case 0://未连接, 断开
        ucRet = Unlinked_Can;
        break;
    case 1://半连接
        ucRet = Unlinked_Can;
        break;
    case 2://连接
        ucRet = Linked_Can;
        break;
    default:
        break;
    }
    return ucRet;
}

//解析遥调指令<----控制中心
void cDCCanProtocol::ParseTeleadjust(InfoMap CenterMap, unsigned char ucTermID)
{
    InfoMap::iterator itTarget;
    QMap <unsigned char, can_frame*> PfMap;
    QMap <unsigned char, can_frame*>::iterator itPfMap;
    //解析Map内容
    for(itTarget = CenterMap.begin(); itTarget != CenterMap.end(); itTarget++)
    {
        switch (itTarget.key())
        {
        //解析直流柜最大输出功率
        case Addr_CabMaxPower_Adj:
        {
            if(!PfMap.contains(PF_CCUParamSet_CCU))
            {
                can_frame * pFrame = new can_frame;
                memset(pFrame, 0x00, sizeof(can_frame));
                PfMap.insert(PF_CCUParamSet_CCU, pFrame);
            }
            MakeFrameCCUParamSet(Addr_CabMaxPower_Adj, itTarget.value(), PfMap[PF_CCUParamSet_CCU], ucTermID);
            break;
        }
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

//获取参数设置结果类型
int cDCCanProtocol::GetParamAckType(unsigned char ucPF)
{
    int iRet = 0;
    switch(ucPF)
    {
    case PF_ActiveProtectionSet_DC:
        iRet = AddrType_ActiveProtectSetResult;
        break;
    case PF_FlexibleChargeSet_DC:
        iRet = AddrType_FlexibleChargeSetResult;
        break;
    case PF_GeneralStaticParamSet_DC:
        iRet = AddrType_GeneralStaticArgResult;
        break;
    case PF_GeneralDynamicParamSet_DC:
        iRet = AddrType_GeneralDynamicArgResult;
        break;
    case PF_CCUParamSet_CCU:
        iRet = AddrType_CCUArgResult;
        break;
    default:
        iRet = AddrType_CCUArgResult;
        break;
    }
    return iRet;
}

//获取协议版本号枚举
void cDCCanProtocol::GetProVerEnum(unsigned char * pVer)
{
    if( (pVer[0] == 18)&&(pVer[1] == 0)&&(pVer[2] == 2) )
    {
        this->ucProVer = ProVer_2_0_19_DC;
    }
    else
    {
        this->ucProVer = ProVer_Old_DC;
    }
}

//是否有帧需要处理
bool cDCCanProtocol::HasFrameToDeal()
{
    CheckBroadCastState();
    if(HasLongFrameToDeal() == TRUE)
    {
        return TRUE;
    }
    return FALSE;
}

//接收CAN终端接收的数据并解析
void cDCCanProtocol::ParseCanData(can_frame *pCanFrame)
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
    case PF_ChargerState1_DC:
    {
        ParseFrameState1(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_ChargerState2_DC:
    {
        ParseFrameState2(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_ChargerState3_DC:
    {
        ParseFrameState3(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_ChargerState4_DC:
    {
        ParseFrameState4(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_ChargerState5_DC:
    {
        ParseFrameState5(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_ChargerState6_DC:
    {
        ParseFrameState6_old(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_ChargerState7_DC:
    {
        ParseFrameState7(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_ChargerState8_DC:
    {
        ParseFrameState8(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_ChargerState9_DC:
    {
        ParseFrameState9(pCanFrame->data, ToCenterList);
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
    case PF_ChargeManner_DC://单双枪充电
    {
        // CtrlCmdAck result =Ack_CmdAck;
        ParseFrameChargeManner(pCanFrame, ToCenterList);
        //SendFrameAck(pCanFrame->can_id,result);//应答E8
        break;
    }
    case PF_CtrlCmdAck:
    {
        ParseFrameCmdAck(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_LoadDispatch_DC:
    {
        ParseFrameLoadDispatch(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_ModuleFaultInfo_DC:
    {
        ParseFrameModuleFaultInfo(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_BMS_HandShake1_DC:
    {
        ParseFrameBMSHandShake1(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_BMS_HandShake2_DC:
    {
        ParseFrameBMSHandShake2(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_BMS_ParamSet2_DC:
    {
        ParseFrameBMSParamSet2(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_BMS_Charging1_DC:
    {
        ParseFrameBMSCharging1(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_BMS_Charging2_DC:
    {
        ParseFrameBMSCharging2(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_BMS_Charging3_DC:
    {
        ParseFrameBMSCharging3(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_BMS_ChargeEnd1_DC:
    {
        ParseFrameBMSChargeEnd1(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_BMS_ChargeEnd2_DC:
    {
        ParseFrameBMSChargeEnd2(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_BMS_ChargeEnd3_DC:
    {
        ParseFrameBMSChargeEnd3(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_BMS_ChargeEnd4_DC:
    {
        ParseFrameBMSChargeEnd4(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_BMS_ChargeEnd5_DC:
    {
        ParseFrameBMSChargeEnd5(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_ModuleInfo1_CCU:
    {
        ParseFrameModuleInfo1(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_ModuleInfo2_CCU:
    {
        ParseFrameModuleInfo2(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_ModuleInfo3_CCU:
    {
        ParseFrameModuleInfo3(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_PDUInfo1_CCU:
    {
        ParseFramePDUInfo1(pCanFrame->data, ToCenterList);
        ParseCCUNum(strFrameHead.ucSa);
        break;
    }
    case PF_PDUInfo2_CCU:
    {
        ParseFramePDUInfo2(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_PDUInfo3_CCU:
    {
        ParseFramePDUInfo3(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_BranchInfo1_CCU:
    {
        ParseFrameBOXInfo1(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_BranchInfo2_CCU:
    {
        ParseFrameBOXInfo2(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_CCUInfo1_CCU:
    {
        ParseFrameCCUInfo1(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_CCUInfo2_CCU:
    {
        ParseFrameCCUInfo2(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_PDUInfo4_CCU:
    {
        ParseFramePDUInfo4(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_GeneralStaticParamSet_DC:
    {
        ParseTermGeneralStaticArg(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_GeneralDynamicParamSet_DC:
    {
        ParseTermGeneralDynamicArg(pCanFrame->data, ToCenterList);
        break;
    }
    case PF_CCUParamSet_CCU:
    {
        ParseCCUQueryArgResult(pCanFrame->data, ToCenterList);
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
                break;
            }
            else
            {
                ParseFrameDataTran(pCanFrame->data, pModule);

            }
        }

        break;
    }
        //smm add
        //解析机柜
    case PF_JIGUI:
    {
      ParseJiGui(pCanFrame->data,ToCenterList,strFrameHead.ucSa);
      break;
    }
        //解析枪头
    case PF_GunTemperature_DC:
    {
        ParseQiangTou(pCanFrame->data,ToCenterList);
        break;
    }
        //smm end
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
void cDCCanProtocol::ParseActiveProtect(InfoMap CenterMap, unsigned char ucTermID)
{
    SendFrameActiveProtect(CenterMap, ucTermID);
}

//解析柔性充电设置<----控制中心
void cDCCanProtocol::ParseFlexibleCharge(InfoMap CenterMap, unsigned char ucTermID)
{
    SendFrameFlexibleCharge(CenterMap, ucTermID);
}

//解析通用静态参数设置<----控制中心
void cDCCanProtocol::ParseGeneralStaticArg(InfoMap CenterMap, unsigned char ucTermID)
{

    if(ucProVer == ProVer_Old_DC)   //设置辅源类型
    {
        can_frame * pCanFrame = new can_frame;
        FrameSetDispatchInfo_DC * pFrame = (FrameSetDispatchInfo_DC *)pCanFrame->data;
        memset((unsigned char *)pCanFrame, 0x00, sizeof(can_frame));
        pFrame->ucParamType = 0x06;
        pFrame->Data.stCmd06.ucAuxPowerType = CenterMap[Addr_AuxPowerType_GSA].at(0);
        SetFrameHead(pCanFrame, ID_DefaultControlCenterCanID, PF_SetDispatchInfo_DC, ucTermID, PriorityDefault, DL_SetDispatchInfo_DC);
        pSendListMutex->lock();
        pTerminalSendList->append(pCanFrame);
        pSendListMutex->unlock();
    }
    else if(ucProVer == ProVer_2_0_19_DC)   //静态参数设置
    {
        SendFrameStaticArg(CenterMap, ucTermID);
    }
}

//解析通用动态参数设置<----控制中心
void cDCCanProtocol::ParseGeneralDynamicArg(InfoMap CenterMap, unsigned char ucTermID)
{
    //    can_frame * pCanFrameOld[5];
    //    can_frame * pCanFrameNew[3];
    FrameSetDispatchInfo_DC * pFrame_Old[5];
    FrameGeneralDynamicParamSet * pFrame_New[3];

    QList <can_frame * > oldFrameList, newFrameList;
    can_frame * pFrame;
    if(CenterMap.contains(Addr_ArgData))
    {
        pFrame = new(can_frame);
        FrameGeneralDynamicParamSet * pData = (FrameGeneralDynamicParamSet * )pFrame->data;
        memcpy(pData, CenterMap[Addr_ArgData].data(), CenterMap[Addr_ArgData].length());
        SetFrameHead(pFrame, ID_DefaultControlCenterCanID, PF_GeneralDynamicParamSet_DC, ucTermID, PriorityDefault, DL_GeneralDynamicParamSet_DC);
        pSendListMutex->lock();
        pTerminalSendList->append(pFrame);
        pSendListMutex->unlock();
    }
    if(ucProVer == ProVer_Old_DC)
    {
        for(unsigned char i = 0; i < 5; i++)
        {
            //            pCanFrameOld[i] = new(can_frame);
            pFrame = new(can_frame);
            oldFrameList.append(pFrame);
            pFrame_Old[i] = (FrameSetDispatchInfo_DC *) (oldFrameList.at(i)->data);
            memset(oldFrameList.at(i), 0x00, sizeof(can_frame));
        }
        if(CenterMap.contains(Addr_WorkMode_GDA))//充电终端工作模式
        {
            pFrame_Old[2]->ucParamType = 0x03;
            pFrame_Old[2]->Data.stCmd03.ucChargeMode = CenterMap[Addr_WorkMode_GDA].at(0);
            memset(pFrame_Old[2]->Data.stCmd03.ucReserved, 0x00, sizeof(pFrame_Old[2]->Data.stCmd03.ucReserved));
        }
        if(CenterMap.contains(Addr_WorkState_GDA))//充电终端工作状态
        {
            pFrame_Old[2]->ucParamType = 0x03;
            pFrame_Old[2]->Data.stCmd03.ucChargeMode = CenterMap[Addr_WorkState_GDA].at(0) + 5;
            memset(pFrame_Old[2]->Data.stCmd03.ucReserved, 0x00, sizeof(pFrame_Old[2]->Data.stCmd03.ucReserved));
        }
        if(CenterMap.contains(Addr_GroupStrategy_GDA))//群充策略
        {
            pFrame_Old[4]->ucParamType = 0x05;
            pFrame_Old[4]->Data.stCmd05.ucGroupChargeStrategy = CenterMap[Addr_GroupStrategy_GDA].at(0);
            memset(pFrame_Old[4]->Data.stCmd05.ucReserved, 0x00, sizeof(pFrame_Old[4]->Data.stCmd05.ucReserved));
        }
        if(CenterMap.contains(Addr_PriorityLevel_GDA))//优先级
        {
            pFrame_Old[0]->ucParamType = 0x01;
            pFrame_Old[0]->Data.stCmd01.ucPriority = CenterMap[Addr_PriorityLevel_GDA].at(0);
            memset(pFrame_Old[0]->Data.stCmd01.ucReserved, 0x00, sizeof(pFrame_Old[0]->Data.stCmd01.ucReserved));
        }
        if(CenterMap.contains(Addr_ChargeEndTime_GDA))//完成时间 (该设置项未使用)
        {
            ;
            //            pFrame_Old[0]->ucParamType = 0x02;
        }
        for(unsigned char i = 0; i < 5; i++)
        {
            if(oldFrameList.at(i)->data[0] == 0x00)
            {
                delete oldFrameList.at(i);
            }
            else
            {
                SetFrameHead(oldFrameList.at(i), ID_DefaultControlCenterCanID, PF_SetDispatchInfo_DC, ucTermID, PriorityDefault, DL_SetDispatchInfo_DC);
                pSendListMutex->lock();
                pTerminalSendList->append(oldFrameList.at(i));
                pSendListMutex->unlock();
            }
        }
    }
    //采用新协议(V2.0.15之后版本)
    else if(ucProVer == ProVer_2_0_19_DC)
    {
        for(unsigned char i = 0; i < 3; i++)
        {
            pFrame = new(can_frame);
            newFrameList.append(pFrame);
            pFrame_New[i] = (FrameGeneralDynamicParamSet *)(newFrameList.at(i)->data);
            memset(newFrameList.at(i), 0x00, sizeof(can_frame));
        }
        if(CenterMap.contains(Addr_WorkMode_GDA))//充电终端工作模式
        {
            pFrame_New[2]->ucParamType = 0x03;
            pFrame_New[2]->Data.stCmd03.ucWorkModle = CenterMap[Addr_WorkMode_GDA].at(0);
            memset(pFrame_New[2]->Data.stCmd03.ucReserved, 0x00, sizeof(pFrame_New[2]->Data.stCmd03.ucReserved));
        }
        if(CenterMap.contains(Addr_WorkState_GDA))//充电终端工作状态
        {
            pFrame_New[2]->ucParamType = 0x03;
            pFrame_New[2]->Data.stCmd03.ucGroupType = CenterMap[Addr_WorkState_GDA].at(0);
            memset(pFrame_New[2]->Data.stCmd03.ucReserved, 0x00, sizeof(pFrame_New[2]->Data.stCmd03.ucReserved));
        }
        if(CenterMap.contains(Addr_GroupStrategy_GDA))//群充策略
        {
            pFrame_New[2]->ucParamType = 0x03;
            pFrame_New[2]->Data.stCmd03.ucGroupStrategy = CenterMap[Addr_GroupStrategy_GDA].at(0);
            memset(pFrame_New[2]->Data.stCmd03.ucReserved, 0x00, sizeof(pFrame_New[2]->Data.stCmd03.ucReserved));
        }
        if(CenterMap.contains(Addr_PriorityLevel_GDA))//充电优先等级
        {
            pFrame_New[0]->ucParamType = 0x01;
            pFrame_New[0]->Data.stCmd01.ucPriority = CenterMap[Addr_PriorityLevel_GDA].at(0);
            memset(pFrame_New[0]->Data.stCmd01.ucReserved, 0x00, sizeof(pFrame_New[0]->Data.stCmd01.ucReserved));
        }
        if(CenterMap.contains(Addr_ChargeEndTime_GDA))//充电完成时间
        {
            ;//pFrame_New[0]->ucParamType = 0x02;
        }

        for(unsigned char i = 0; i < 3; i++)
        {
            if(newFrameList.at(i)->data[0] == 0x00)
            {
                delete newFrameList.at(i);
            }
            else
            {
                SetFrameHead(newFrameList.at(i), ID_DefaultControlCenterCanID, PF_GeneralDynamicParamSet_DC, ucTermID, PriorityDefault, DL_GeneralDynamicParamSet_DC);
                pSendListMutex->lock();
                pTerminalSendList->append(newFrameList.at(i));
                pSendListMutex->unlock();
            }
        }
    }
}
//smm add
//解析机柜温度湿度
void cDCCanProtocol::ParseJiGui(unsigned char * pData,QList<CanMapNode> & ToCenterList,unsigned char canID)
{
         CanMapNode newNode;
         //定义相关数据
         JiGuiwenDuShiDu  jWenShi;
         //解帧

         if(canID == 181)
         {
             memcpy((unsigned char *)&jWenShi,pData,sizeof(jWenShi));
             //设置机柜温度湿度参数
             short temp = jWenShi.temp/10.0;
             //newNode.stCanMap.insert(Addr_StationEnvTemp,QByteArray((char*)&temp,2));
             InsertCanInfoMapLine((char *)& temp, sizeof(temp), Addr_StationEnvTemp ,newNode.stCanMap);
              //设置机柜湿度参数
             temp = jWenShi.humidty/10.0;
             //newNode.stCanMap.insert(Addr_StationEnvHumi,QByteArray((char*)&temp,2));
             InsertCanInfoMapLine((char *)& temp, sizeof(temp), Addr_StationEnvHumi ,newNode.stCanMap);
             newNode.enType = AddrType_TempHumi;
         }
         else
         {
             return ; // 暂时不考虑
         }
         //添加到List
         ToCenterList.append(newNode);
}
//解析枪头A和B
void cDCCanProtocol::ParseQiangTou(unsigned char * pData,QList<CanMapNode> & ToCenterList){

         CanMapNode newNode;
         Qiang    Qia;
         memcpy((unsigned char *)&Qia,pData,sizeof(Qia));

         float Atemp = (Qia.QiAtemp-500)/10.0;  //温度偏移-50,分辨率0.1

         InsertCanInfoMapLine((char *)& Atemp, sizeof(Atemp), Addr_Qiang_Temp ,newNode.stCanMap);

         newNode.enType = AddrType_TermMeasure;

         //添加到List
         ToCenterList.append(newNode);
}

//smm end



//解析查询主动防护设置<----控制中心
void cDCCanProtocol::ParseQueryActiveProtect(InfoMap CenterMap, unsigned char ucTermID)
{
    if(CenterMap.isEmpty())
    {
        ;
    }
    SendFrameAppyArg(ucTermID, ucTermID, 0, PF_ActiveProtectionSet_DC);
}

//解析查询柔性充电设置<----控制中心
void cDCCanProtocol::ParseQueryFlexibleCharge(InfoMap CenterMap, unsigned char ucTermID)
{
    if(CenterMap.isEmpty())
    {
        ;
    }
    SendFrameAppyArg(ucTermID, ucTermID, 0, PF_FlexibleChargeSet_DC);
}

//解析查询通用静态参数设置<----控制中心
void cDCCanProtocol::ParseQueryGeneralStaticArg(InfoMap CenterMap, unsigned char ucTermID)
{
    if(CenterMap.isEmpty())
    {
        ;
    }
    SendFrameAppyArg(ucTermID, ucTermID, 0, PF_GeneralStaticParamSet_DC);
}

//解析查询通用动态参数设置<----控制中心
void cDCCanProtocol::ParseQueryGeneralDynamicArg(InfoMap CenterMap, unsigned char ucTermID)
{
    if(!CenterMap.contains(Addr_ArgNo))
    {
        return;
    }
    unsigned char ucArgNo = CenterMap[Addr_ArgNo].at(0);
    SendFrameAppyArg(ucTermID, ucTermID, ucArgNo, PF_GeneralDynamicParamSet_DC);
}

//解析CCU参数设置<----控制中心
void cDCCanProtocol::ParseCCUArg(InfoMap CenterMap, unsigned char ucTermID)
{
    SendFrameCCUArg(CenterMap, ucTermID);
}

//解析CCU参数查询<----控制中心
void cDCCanProtocol::ParseCCUArgQuery(InfoMap CenterMap, unsigned char ucTermID)
{
    if(CenterMap.isEmpty())
    {
        ;
    }
    SendFrameAppyArg(ucTermID, ucTermID, 0, PF_CCUParamSet_CCU);
}

//解析能效系统模块控制<----控制中心
void cDCCanProtocol::ParseCtrlModule_EP(InfoMap CenterMap, unsigned char ucTermID)
{
    if((!CenterMap.isEmpty())||(ucTermID!=0))
    {
        ;
    }
}
//解析能效系统功率控制<----控制中心
void cDCCanProtocol::ParseSetPower_EP(InfoMap CenterMap, unsigned char ucTermID)
{
    if((!CenterMap.isEmpty())||(ucTermID!=0))
    {
        ;
    }
}

void cDCCanProtocol::CheckBroadCastState()
{
    //定时广播
    if(uiBroadCastCount%ucBroadCastInterval == 0)
    {
        SendFrameAppyPGN(ID_BroadCastCanID,PF_ChargerState1_DC);
        SendFrameAppyPGN(ID_BroadCastCanID,PF_ChargerState2_DC);
        SendFrameAppyPGN(ID_BroadCastCanID,PF_ChargerState3_DC);
    }

    if(uiBroadCastCount%(ucBroadCastInterval * 60) == 0)
    {
        for(int i = 0;i < 10;i++)
        {
            if(ucCCUNum[i] == 0)
                continue;
            else if(ucCCUNum[i] >= 231 && ucCCUNum[i] <= 240)
                SendFrameAppyPGN(ucCCUNum[i],PF_SpecificInfo);
        }
    }

    //定时对时
    if(uiBroadCastCount%usSetTimeInterval == 0)
    {
        SendFrameTimeSync(ID_BroadCastCanID);
    }
    uiBroadCastCount++;
}

//获取故障代码----直流充电状态1
//说明:
//1.若故障代码有意义则上传故障代码,
//2.若故障代码无意义则依次判断故障并上传
unsigned char cDCCanProtocol::CheckFaultCode_State1(State1Fault_DC stFault, unsigned char ucFaultCodeIn)
{
    //故障代码不为空, 返回故障代码
    if(ucFaultCodeIn != 0)
    {
        return ucFaultCodeIn;
    }
    else
    {
        if(stFault.ucModuleError != 0)
        {
            return 0x30;//充电模块故障
        }
        else if(stFault.ucMainContactorError != 0)
        {
            return 0x31;//主接触器故障
        }
        else if(stFault.ucAuxContactorError != 0)
        {
            return 0x32;//辅助接触器故障
        }
        else if(stFault.ucOutputOverVoltage != 0)
        {
            return 0x39;//直流输出过压
        }
        else if(stFault.ucOutputOverCurrent != 0)
        {
            return 0x3B;//直流输出过流
        }
        else
        {
            return 0x00;//无故障
        }
    }
}

//获取故障代码----BMS充电阶段3
unsigned char cDCCanProtocol::CheckFaultCode_BMSCharge3(BMSCharge3Fault_DC &stFault)
{
    if(stFault.stByte1.ucSingleBatteryVoltageError == 0x01)  //单体动力蓄电池电压过高
    {
        return 0x70;
    }
    else if(stFault.stByte1.ucSingleBatteryVoltageError == 0x02) //单体动力蓄电池电压过低
    {
        return 0x69;
    }

    if(stFault.stByte1.ucSOCError == 0x01)  //整车动力蓄电池SOC过高
    {
        return 0x71;
    }
    else if(stFault.stByte1.ucSOCError == 0x02) //整车动力蓄电池SOC过低
    {
        return 0x72;
    }

    if(stFault.stByte1.ucOverCurrent == 0x01) //过流
    {
        return 0x73;
    }

    if(stFault.stByte1.ucTempTooHigh == 0x01) //过温
    {
        return 0x74;
    }

    if(stFault.stByte2.ucInsulationState == 0x01) //绝缘故障
    {
        return 0x75;
    }

    if(stFault.stByte2.ucOutputConnectorState == 0x01) //输出连接器故障
    {
        return 0x76;
    }

    return 0;   //无故障
}

//获取故障代码----直流充电状态4
//说明:
//1.若故障码中为不正常,则上传故障
unsigned char cDCCanProtocol::CheckFaultCode_State4(State4Fault1_DC stFault1, State4Fault2_DC stFault2)
{
    if((*((unsigned char *)&stFault1) == 0)
            &&(stFault2.ucBatteryInsulationState == 0)
            &&(stFault2.ucBatteryConnecterState == 0))
    {
        return 0;//无故障
    }

    if(stFault1.ucBatteryVoltage == 1)
    {
        return 0x70;//电池电压过高
    }
    if(stFault1.ucBatterySOC == 1)
    {
        return 0x71;//电池SOC过高
    }
    if(stFault1.ucBatterySOC == 2)
    {
        return 0x72;//电池SOC过低
    }
    if(stFault1.ucBatteryCurrent == 1)
    {
        return 0x73;//电池充电过流
    }
    if(stFault1.ucBatteryTemperature == 1)
    {
        return 0x74;//电池温度过高
    }

    if(stFault2.ucBatteryInsulationState == 1)
    {
        return 0x75;//电池绝缘故障
    }
    if(stFault2.ucBatteryConnecterState == 1)
    {
        return 0x76;//电池输出连接器异常
    }
    //充电允许另外定义
    return 0;//无故障
}

//获取充电机工作状态----直流充电状态1
//说明:若工作状态不存在则返回未定义充电机状态
unsigned char cDCCanProtocol::CheckWorkState(unsigned char ucWorkState, unsigned char ucChildWorkState)
{
    switch(ucWorkState)
    {
    case 0:
    {
        switch(ucChildWorkState)
        {
        case 0://待机
        {
            return CHARGE_STATUS_REALTIME_STANDBY;
            break;
        }
        case 1://暂停
        {
            return CHARGE_STATUS_REALTIME_PAUSE;
            break;
        }
        case 2://限制
        {
            return CHARGE_STATUS_REALTIME_LIMIT;
            break;
        }
        case 3://切换中
        {
            return CHARGE_STATUS_REALTIME_SWITCH;
            break;
        }
        case 4://放电
        {
            return CHARGE_STATUS_REALTIME_DISCHARGING;
            break;
        }
        case 6://车辆暂停
        {
            return CHARGE_STATUS_REALTIME_CARPAUSE;
            break;
        }
        case 7://充电设备暂停
        {
            return CHARGE_STATUS_REALTIME_DEVPAUSE;
            break;
        }
        default://未定义充电机状态
        {
            return CHARGE_STATUS_REALTIME_UNKNOWN;
            break;
        }
        }
        break;
    }
    case 1://充电中
    {
        switch(ucChildWorkState)
        {
        case 4://放电
        {
            return CHARGE_STATUS_REALTIME_DISCHARGING;
            break;
        }
        default:
        {
            return CHARGE_STATUS_REALTIME_CHARGING;
            break;
        }
        }
    }
    case 2://故障
    {
        return CHARGE_STATUS_REALTIME_FAULT;
        break;
    }
    case 3://启动中
    {
        return CHARGE_STATUS_REALTIME_STARTING;
        break;
    }
    default:
    {
        return CHARGE_STATUS_REALTIME_UNKNOWN;
        break;
    }
    }
}

//生成  主动防护帧
void cDCCanProtocol::MakeFrameActiveProtect(InfoMap &CenterMap, cLongPackageModule * pModule)
{
    if(CenterMap.contains(Addr_ArgData))
    {
        memcpy(pModule->pLongPackage->pData, CenterMap[Addr_ArgData].data(), pModule->pLongPackage->uiDataLength);
        return;
    }
}

//生成终端柔性充电帧
void cDCCanProtocol::MakeFrameFlexibleCharge(InfoMap &CenterMap, cLongPackageModule * pModule)
{
    FrameFlexibleParamSet * pFrame = (FrameFlexibleParamSet *)(pModule->pLongPackage->pData);
    if(CenterMap.contains(Addr_ArgNo))
    {
        pModule->pLongPackage->pData[0] = CenterMap[Addr_ArgNo].at(0);
        memcpy(pModule->pLongPackage->pData+1, CenterMap[Addr_ArgData].data(), pModule->pLongPackage->uiDataLength);
        return;
    }

    if(CenterMap.contains(Addr_SOCCoefficient_FC))
    {
        pFrame->ucType = 1;
        memcpy((unsigned char *)pFrame->Data.stType1, CenterMap[Addr_SOCCoefficient_FC].data(), 20 *sizeof(pFrame->Data.stType1));
    }
    else if(CenterMap.contains(Addr_TempCoefficient_FC))
    {
        pFrame->ucType = 2;
        memcpy((unsigned char *)pFrame->Data.stType2, CenterMap[Addr_TempCoefficient_FC].data(), 20 *sizeof(pFrame->Data.stType2));
    }
    else if(CenterMap.contains(Addr_TimeCoefficient_FC))
    {
        pFrame->ucType = 3;
        memcpy((unsigned char *)pFrame->Data.stType3, CenterMap[Addr_TimeCoefficient_FC].data(), 20 *sizeof(pFrame->Data.stType3));
    }
    else
    {
        return;
    }
}

//发送终端主动防护帧
void cDCCanProtocol::SendFrameActiveProtect(InfoMap &CenterMap, unsigned char ucCanID)
{
    cLongPackageModule * pModule = CheckModule(ucCanID);
    MakeFrameLongPackage(pModule->pLongPackage, ucCanID, PF_ActiveProtectionSet_DC, DL_ActiveProtectionSet_DC);
    MakeFrameActiveProtect(CenterMap, pModule);
    pModule->bFreeFlag = FALSE;
    pModule->bUsedFlag = FALSE;
    pModule->bValidFlag = FALSE;
    pModuleSendMapMutex->lock();
    pModuleMapToSend->insert(ucCanID, pModule);
    pModuleSendMapMutex->unlock();
//    SendFrameApplySend(pModule);
}

//发送终端柔性充电帧
void cDCCanProtocol::SendFrameFlexibleCharge(InfoMap &CenterMap, unsigned char ucCanID)
{
    cLongPackageModule * pModule = new cLongPackageModule(ucCanID);//CheckModule(ucCanID);
    MakeFrameLongPackage(pModule->pLongPackage, ucCanID, PF_FlexibleChargeSet_DC, DL_FlexibleChargeSet_DC);
    MakeFrameFlexibleCharge(CenterMap, pModule);
    pModule->bFreeFlag = FALSE;
    pModule->bUsedFlag = FALSE;
    pModule->bValidFlag = FALSE;
    pModuleSendMapMutex->lock();
    pModuleMapToSend->insert(ucCanID, pModule);
    pModuleSendMapMutex->unlock();
//    SendFrameApplySend(pModule);
}

//发送通用静态参数设置帧
void cDCCanProtocol::SendFrameStaticArg(InfoMap &CenterMap, unsigned char ucCanID)
{
    can_frame *pCanFrame = new(can_frame);
    MakeFrameStaticArg(CenterMap, pCanFrame, ucCanID);
    pSendListMutex->lock();
    pTerminalSendList->append(pCanFrame);
    pSendListMutex->unlock();
}

//发送CCU参数设置帧
void cDCCanProtocol::SendFrameCCUArg(InfoMap &CenterMap, unsigned char ucCanID)
{
    can_frame *pCanFrame = new(can_frame);
    QByteArray argData = CenterMap[Addr_ArgData];
    MakeFrameCCUParamSet(argData, pCanFrame, ucCanID);
    pSendListMutex->lock();
    pTerminalSendList->append(pCanFrame);
    pSendListMutex->unlock();
}

//生成CCU设置帧----0xB5  旧接口, 具体设置项赋值
void cDCCanProtocol::MakeFrameCCUParamSet(unsigned int uiKey, QByteArray CenterData, can_frame * &pFrame, unsigned char ucCanID)
{
    FrameCCUParamSet_CCU * pTarget =  (FrameCCUParamSet_CCU *)(pFrame->data);
    switch(uiKey)
    {
    case Addr_CabMaxPower_Adj:
        pTarget->usCabMaxPower = (unsigned short)(*(int *)(CenterData.data()));
        break;
    default:
        break;
    }
    SetFrameHead(pFrame, ID_DefaultControlCenterCanID, PF_CCUParamSet_CCU, ucCanID, PriorityDefault, DL_CCUParamSet_CCU);
}

//生成CCU设置帧----0xB5  新接口, 直接内存拷贝
void cDCCanProtocol::MakeFrameCCUParamSet(QByteArray argArray, can_frame * &pFrame, unsigned char ucCanID)
{
    memset(pFrame->data, 0x00, DL_CCUParamSet_CCU);
    memcpy(pFrame->data, argArray.data(), DL_CCUParamSet_CCU);
    SetFrameHead(pFrame, ID_DefaultControlCenterCanID, PF_CCUParamSet_CCU, ucCanID, PriorityDefault, DL_CCUParamSet_CCU);
}

//生成通用静态参数设置帧
void cDCCanProtocol::MakeFrameStaticArg(InfoMap &CenterMap, can_frame *pFrame, unsigned char ucCanID)
{
    if(CenterMap.contains(Addr_ArgData))
    {
        memcpy((char *)pFrame->data, CenterMap[Addr_ArgData].data(), sizeof(FrameGeneralStaticParamSet));
    }
    else
    {
        FrameGeneralStaticParamSet stFrame;
        memset((unsigned char *)&stFrame, 0x00, sizeof(stFrame));
        stFrame.stByte1.ucAuxPowerSet = CenterMap[Addr_AuxPowerType_GSA].at(0);
        stFrame.stByte1.ucElecLockType = CenterMap[Addr_ElecLockType_GSA].at(0);
        stFrame.stByte1.ucVINEnableFlag = CenterMap[Addr_VINEnable_GSA].at(0);
        stFrame.stByte1.ucElecLockEnableFlag = CenterMap[Addr_ElecLockEnable_GSA].at(0);
        stFrame.stByte2.ucBMSProType = CenterMap[Addr_BMSProTypeSet_GSA].at(0);
        stFrame.stByte2.ucResrved = 0x0F;
        stFrame.usGunMaxCurrent = *((float *)CenterMap[Addr_MaxGunCur_GSA].data())*10;
        stFrame.ucTermID = CenterMap[Addr_TermIDSet_GSA].at(0);
        memset(stFrame.ucReserved, 0x00, sizeof(stFrame.ucReserved));
        memcpy(pFrame->data, (unsigned char *)&stFrame, sizeof(stFrame));
    }
    SetFrameHead(pFrame, ID_DefaultControlCenterCanID, PF_GeneralStaticParamSet_DC, ucCanID, PriorityDefault, DL_GeneralStaticParamSet_DC);
}

//发送通用动态参数设置帧
//void cDCCanProtocol::MakeFrameDynamicArg(InfoMap &CenterMap, can_frame *pCanFrame, unsigned char ucCanID)
//{
//    ;
//}

//解析充电机状态1
//说明: 充电机状态1中的群充轮充标识不上传平台
void cDCCanProtocol::ParseFrameState1(unsigned char * pData, QList<CanMapNode>  &ToCenterList)
{
    CanMapNode newNode;

    //定义相关数据
    State1Fault_DC stFault;//故障结构体

    char chLinkState;//连接确认开关状态
    char chRelyState;///输出继电器状态
    char chGroupQueueFlag;//群充轮充标识 --不上传
    unsigned char ucFaultCode;//故障代码
    unsigned char ucWorkState;//充电机工作状态
    float fChargeVoltage;//充电机充电电压
    float fChargeCurrent;//充电机充电电流
    //char cElectronicLockStatus;//电子锁状态

    //解帧
    FrameChargerState1_DC strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    stFault = strFrame.stFault;
    chLinkState = CheckLinkState(strFrame.stStatus.ucLinkState);
    chRelyState = strFrame.stStatus.ucRelyState;
    chGroupQueueFlag = strFrame.ucGroupQueueFlag;
    fChargeVoltage = (float)strFrame.usChargeVoltage/10.0;
    fChargeCurrent = (float)(strFrame.sChargeCurrent  - 4000 )/10.0;

    Q_UNUSED(chGroupQueueFlag);
    //处理故障代码
    ucFaultCode = CheckFaultCode_State1(stFault, strFrame.ucFaultCode);
    //处理充电机工作状态
    ucWorkState = CheckWorkState(strFrame.stStatus.ucWorkState, strFrame.stStatus.ucWorkStateChild);

    //添加连接确认开关状态
    InsertCanInfoMapLine(&chLinkState, sizeof(chLinkState), Addr_LinkState_Term, newNode.stCanMap);
    //添加输出继电器状态
    InsertCanInfoMapLine(&chRelyState, sizeof(chRelyState), Addr_RelyState_Term, newNode.stCanMap);
    //添加故障代码
    InsertCanInfoMapLine((char *)&ucFaultCode, sizeof(ucFaultCode), Addr_FaultCode_Term, newNode.stCanMap);
    //添加充电机工作状态
    InsertCanInfoMapLine((char *)&ucWorkState, sizeof(ucWorkState), Addr_WorkState_Term, newNode.stCanMap);

    newNode.enType = AddrType_TermSignal;
    ToCenterList.append(newNode);

    newNode.stCanMap.clear();
    //添加直流电压
    InsertCanInfoMapLine((char *)&fChargeVoltage, sizeof(fChargeVoltage), Addr_DCChargeVoltage_Term, newNode.stCanMap);
    //添加直流电流
    InsertCanInfoMapLine((char *)&fChargeCurrent, sizeof(fChargeCurrent), Addr_DCChargeCurrent_Term, newNode.stCanMap);

    newNode.enType = AddrType_TermMeasure;
    ToCenterList.append(newNode);
}

//解析充电机状态2
void cDCCanProtocol::ParseFrameState2(unsigned char * pData, QList<CanMapNode>  &ToCenterList)
{
    CanMapNode newNode;

    //定义相关数据
    float fBMSNeedVoltage;//BMS 电压需求
    float fBMSNeedCurrent;//BMS 电流需求
    unsigned char ucBaterySOC;//电池当前SOC
    short sMaxBateryTemperature;//最高电池温度
    float fMaxBateryVoltage;//最高电池电压

    //解帧
    FrameChargerState2_DC strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    fBMSNeedVoltage = (float)(strFrame.usBMSNeedVoltage)/10.0;
    fBMSNeedCurrent = (float)(strFrame.sBMSNeedCurrent - 4000)/10.0;
    ucBaterySOC = strFrame.ucBaterySOC;
    sMaxBateryTemperature = strFrame.ucMaxBateryTemperature - 50;
    fMaxBateryVoltage = (float)strFrame.sMaxBateryVoltage/100.0;

    //添加BMS 电压需求
    InsertCanInfoMapLine((char *)&fBMSNeedVoltage, sizeof(fBMSNeedVoltage), Addr_NeedVoltage_BMS, newNode.stCanMap);

    //添加BMS 电流需求
    InsertCanInfoMapLine((char *)&fBMSNeedCurrent, sizeof(fBMSNeedCurrent), Addr_NeedCurrent_BMS, newNode.stCanMap);

    //添加电池当前SOC
    InsertCanInfoMapLine((char *)&ucBaterySOC, sizeof(ucBaterySOC), Addr_NowSOC_BMS, newNode.stCanMap);
    //添加最高电池温度
    InsertCanInfoMapLine((char *)&sMaxBateryTemperature, sizeof(sMaxBateryTemperature), Addr_MaxBatteryTemp_BMS, newNode.stCanMap);
    //添加最高单体电池电压
    InsertCanInfoMapLine((char *)&fMaxBateryVoltage, sizeof(fMaxBateryVoltage), Addr_MaxSingleBatteryVoltage_BMS, newNode.stCanMap);

    newNode.enType = AddrType_TermBMS;
    ToCenterList.append(newNode);
}

//解析充电机状态3
void cDCCanProtocol::ParseFrameState3(unsigned char * pData, QList<CanMapNode>  &ToCenterList)
{
    CanMapNode newNode;
    //定义相关数据
    unsigned short usChargeTime;//本次累计充电时间
    uint uiTotalChargeEnergy;//累计充电电量
    unsigned char ucChargeEndCode;//充电中止代码

    //解帧
    FrameChargerState3_DC strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    usChargeTime = strFrame.usChargeTime;
    uiTotalChargeEnergy = strFrame.uiTotalChargeEnergy;
    ucChargeEndCode = strFrame.ucChargeEndCode;

    //添加充电时间
    InsertCanInfoMapLine((char *)&usChargeTime, sizeof(usChargeTime), Addr_ChargeTime_Term, newNode.stCanMap);
    newNode.enType = AddrType_TermBMS;
    ToCenterList.append(newNode);
    newNode.stCanMap.clear();

    //添加累计充电电量
    InsertCanInfoMapLine((char *)&uiTotalChargeEnergy, sizeof(uiTotalChargeEnergy), Addr_TotalActiveEnergy_Term, newNode.stCanMap);
    newNode.enType = AddrType_TermMeasure;
    ToCenterList.append(newNode);
    newNode.stCanMap.clear();

    //添加充电中止代码
    InsertCanInfoMapLine((char *)&ucChargeEndCode, sizeof(ucChargeEndCode), Addr_ChargeEndCode_Term, newNode.stCanMap);
    newNode.enType = AddrType_TermSignal;
    ToCenterList.append(newNode);
}

//解析充电机状态4
void cDCCanProtocol::ParseFrameState4(unsigned char * pData, QList<CanMapNode>  &ToCenterList)
{
    CanMapNode newNode;
    //定义相关数据
    short sLowestBatteryTemperature;//最低电池温度
    float fLowestChargeVoltage;//最低电池电压数(单体电池电压)
    unsigned char ucChargeFaultCode;//充电故障代码
    float fActivePower;//充电有功功率
    char chChargePermitFlag;//BMS充电允许标志

    //解帧
    FrameChargerState4_DC strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    sLowestBatteryTemperature = strFrame.ucLowestBatteryTemperature - 50;
    fLowestChargeVoltage = (float)(strFrame.usLowestChargeVoltage)/100.0;
    fActivePower = (float)(strFrame.usActivePower)/100.0; //单位kw, 精度0.01kw
    chChargePermitFlag = strFrame.stFaultFlag2.ucChargePermit;
    ucChargeFaultCode = CheckFaultCode_State4(strFrame.stFaultFlag1, strFrame.stFaultFlag2);

    //添加最低电池温度
    InsertCanInfoMapLine((char *)&sLowestBatteryTemperature, sizeof(sLowestBatteryTemperature), Addr_MinBatteryTemp_BMS, newNode.stCanMap);
    //添加最低电池电压数
    InsertCanInfoMapLine((char *)&fLowestChargeVoltage, sizeof(fLowestChargeVoltage), Addr_MinSingleVoltage_BMS, newNode.stCanMap);
    //添加BMS充电允许标志
    InsertCanInfoMapLine(&chChargePermitFlag, sizeof(chChargePermitFlag), Addr_ChargePermitFlag_BMS, newNode.stCanMap);
    newNode.enType = AddrType_TermBMS;
    ToCenterList.append(newNode);

    //添加BMS故障代码
    InsertCanInfoMapLine((char *)&ucChargeFaultCode, sizeof(ucChargeFaultCode), Addr_BMSFaultCode_Term, newNode.stCanMap);
    newNode.enType = AddrType_TermSignal;
    ToCenterList.append(newNode);

    //添加充电有功功率
    InsertCanInfoMapLine((char *)&fActivePower, sizeof(fActivePower), Addr_TotalActivePower_Term, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&strFrame.usActivePower,sizeof(strFrame.usActivePower), Addr_TotalActivePower_original_Term, newNode.stCanMap);
    newNode.enType = AddrType_TermMeasure;
    ToCenterList.append(newNode);
}

//解析充电机状态5
void cDCCanProtocol::ParseFrameState5(unsigned char * pData, QList<CanMapNode>  &ToCenterList)
{
    CanMapNode newNode;
    //定义相关数据
    float fAPhaseVoltage;		//A相充电电压
    float fBPhaseVoltage;		//B相充电电压
    float fCPhaseVoltage;		//C相充电电压
    float fAPhaseCurrent;		//A相充电电流

    //解帧
    FrameChargerState5_DC strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    fAPhaseVoltage = strFrame.usAPhaseVoltage/10.0;  //单位V, 精度0.1V
    fBPhaseVoltage = strFrame.usBPhaseVoltage/10.0;  //单位V, 精度0.1V
    fCPhaseVoltage = strFrame.usCPhaseVoltage/10.0;  //单位V, 精度0.1V
    fAPhaseCurrent = (strFrame.sAPhaseCurrent - 4000)/10.0;  //单位A, 精度0.1A

    //添加A相充电电压
    InsertCanInfoMapLine((char *)&fAPhaseVoltage, sizeof(fAPhaseVoltage), Addr_AVoltage_Term, newNode.stCanMap);
    //添加B相充电电压
    InsertCanInfoMapLine((char *)&fBPhaseVoltage, sizeof(fBPhaseVoltage), Addr_BVoltage_Term, newNode.stCanMap);
    //添加C相充电电压
    InsertCanInfoMapLine((char *)&fCPhaseVoltage, sizeof(fCPhaseVoltage), Addr_CVoltage_Term, newNode.stCanMap);
    //添加A相充电电流
    InsertCanInfoMapLine((char *)&fAPhaseCurrent, sizeof(fAPhaseCurrent), Addr_ACurrent_Term, newNode.stCanMap);

    newNode.enType = AddrType_TermMeasure;
    ToCenterList.append(newNode);

}

//解析充电机状态6----旧协议
void cDCCanProtocol::ParseFrameState6_old(unsigned char * pData, QList<CanMapNode>  &ToCenterList)
{
    CanMapNode newNode;
    //定义相关数据
    float fBPhaseCurrent;		//B相充电电流
    float fCPhaseCurrent;		//C相充电电流
    float fZeroLineCurrent; //零线电流
    unsigned char ucCanID;  //配对ＣＡＮ　ＩＤ
    unsigned char ucWorkMode;  //工作模式

    //解帧
    FrameChargerState6_old_DC strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    fBPhaseCurrent = (strFrame.sBPhaseCurrent - 4000)/10.0;  //单位V, 精度0.1V
    fCPhaseCurrent = (strFrame.sCPhaseCurrent - 4000)/10.0;  //单位V, 精度0.1V
    fZeroLineCurrent = (strFrame.sZeroLineCurrent - 4000)/10.0;  //单位V, 精度0.1V
    ucCanID = strFrame.ucReserved[0];
    ucWorkMode = strFrame.ucReserved[1];

    //添加B相充电电流
    InsertCanInfoMapLine((char *)&fBPhaseCurrent, sizeof(fBPhaseCurrent), Addr_BCurrent_Term, newNode.stCanMap);
    //添加C相充电电流
    InsertCanInfoMapLine((char *)&fCPhaseCurrent, sizeof(fCPhaseCurrent), Addr_CCurrent_Term, newNode.stCanMap);
    //添加零线电流
    InsertCanInfoMapLine((char *)&fZeroLineCurrent, sizeof(fZeroLineCurrent), Addr_ZeroLineCurrent_Term, newNode.stCanMap);
    //添加配对终端ＣＡＮＩＤ
    InsertCanInfoMapLine((char *)&ucCanID, sizeof(ucCanID),Addr_300kw_PairCanID, newNode.stCanMap);
    //工作模式
    InsertCanInfoMapLine((char *)&ucWorkMode, sizeof(ucWorkMode),Addr_300kw_WorkMode, newNode.stCanMap);

    newNode.enType = AddrType_TermMeasure;
    ToCenterList.append(newNode);
}

//解析充电机状态6----新协议
void cDCCanProtocol::ParseFrameState6_new(unsigned char * pData, QList<CanMapNode>  &ToCenterList)
{
    CanMapNode newNode;
    //定义相关数据
    float fBPhaseCurrent;		//B相充电电流
    float fCPhaseCurrent;		//C相充电电流
    float fZeroLineCurrent; //零线电流
    float fActivePower; //有功功率
    //解帧
    FrameChargerState6_new_DC strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    fBPhaseCurrent = (strFrame.sBPhaseCurrent - 4000)/10.0;  //单位V, 精度0.1V
    fCPhaseCurrent = (strFrame.sCPhaseCurrent - 4000)/10.0;  //单位V, 精度0.1V
    fZeroLineCurrent = (strFrame.sZeroLineCurrent - 4000)/10.0;  //单位V, 精度0.1V
    fActivePower = strFrame.usActivePower/100.0;  //单位kw, 精度0.01kw

    //添加B相充电电流
    InsertCanInfoMapLine((char *)&fBPhaseCurrent, sizeof(fBPhaseCurrent), Addr_BCurrent_Term, newNode.stCanMap);
    //添加C相充电电流
    InsertCanInfoMapLine((char *)&fCPhaseCurrent, sizeof(fCPhaseCurrent), Addr_CCurrent_Term, newNode.stCanMap);
    //添加零线电流
    InsertCanInfoMapLine((char *)&fZeroLineCurrent, sizeof(fZeroLineCurrent), Addr_ZeroLineCurrent_Term, newNode.stCanMap);
    //添加有功功率
    InsertCanInfoMapLine((char *)&fActivePower, sizeof(fActivePower), Addr_TotalActivePower_Term, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&strFrame.usActivePower,sizeof(strFrame.usActivePower), Addr_TotalActivePower_original_Term, newNode.stCanMap);

    newNode.enType = AddrType_TermMeasure;
    ToCenterList.append(newNode);
}

//解析充电机状态7
void cDCCanProtocol::ParseFrameState7(unsigned char * pData, QList<CanMapNode>  &ToCenterList)
{
    CanMapNode newNode;
    //定义相关数据
    float fReactivePower;		//总无功功率
    float fPowerFactor;		//总功率因数
    float fVoltageUnbalanceRate; //电压不平衡率
    float fCurrentUnbalanceRate; //电流不平衡率
    //解帧
    FrameChargerState7_DC strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    fReactivePower = (strFrame.usReactivePower - 32000)/100.0;  //单位kvar, 精度0.1kvar
    fPowerFactor = (strFrame.usPowerFactor - 1000)/1000.0;  //单位V, 精度0.1V
    fVoltageUnbalanceRate = strFrame.sVoltageUnbalanceRate/100.0;  //单位1%, 精度0.01
    fCurrentUnbalanceRate = strFrame.sCurrentUnbalanceRate/100.0;  //单位1%, 精度0.01

    //添加总无功功率
    InsertCanInfoMapLine((char *)&fReactivePower, sizeof(fReactivePower), Addr_TotalReactivePower_Term, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&strFrame.usReactivePower,sizeof(strFrame.usReactivePower), Addr_TotalReactivePower_original_Term, newNode.stCanMap);
    //添加总功率因数
    InsertCanInfoMapLine((char *)&fPowerFactor, sizeof(fPowerFactor), Addr_TotalPowerFactor_Term, newNode.stCanMap);
    //添加电压不平衡率
    InsertCanInfoMapLine((char *)&fVoltageUnbalanceRate, sizeof(fVoltageUnbalanceRate), Addr_VoltageUnbalanceRate_Term, newNode.stCanMap);
    //添加电流不平衡率
    InsertCanInfoMapLine((char *)&fCurrentUnbalanceRate, sizeof(fCurrentUnbalanceRate), Addr_CurrentUnbalanceRate_Term, newNode.stCanMap);

    newNode.enType = AddrType_TermMeasure;
    ToCenterList.append(newNode);
}

//解析充电机状态8
void cDCCanProtocol::ParseFrameState8(unsigned char * pData, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;
    //定义相关数据
    uint uiReactiveElectricEnergy;		//总无功电能

    //解帧
    FrameChargerState8_DC strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    uiReactiveElectricEnergy = strFrame.uiReactiveElectricEnergy;  //单位kwh, 精度0.1kwh

    //添加总无功电能
    InsertCanInfoMapLine((char *)&uiReactiveElectricEnergy, sizeof(uiReactiveElectricEnergy), Addr_TotalReactiveEnergy_Term, newNode.stCanMap);

    newNode.enType = AddrType_TermMeasure;
    ToCenterList.append(newNode);

}

//解析充电机状态9
void cDCCanProtocol::ParseFrameState9(unsigned char * pData, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;

    //定义相关数据
    uint uiReverseActiveEnergy;		  //反向总有功电能
    uint uiReverseReactiveEnergy;     //反向总无功电能

    //解帧
    FrameChargerState9_DC strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    uiReverseActiveEnergy = strFrame.uiReverseActiveEnergy;  //单位kwh, 精度0.1kwh
    uiReverseReactiveEnergy = strFrame.uiReverseReactiveEnergy;  //单位kwh, 精度0.1kwh

    //添加反向总有功电能
    InsertCanInfoMapLine((char *)&uiReverseActiveEnergy, sizeof(uiReverseActiveEnergy), Addr_TotalReverseActiveEnergy_Term, newNode.stCanMap);
    //添加反向总无功电能
    InsertCanInfoMapLine((char *)&uiReverseReactiveEnergy, sizeof(uiReverseReactiveEnergy), Addr_TotalReverseReactiveEnergy_Term, newNode.stCanMap);

    newNode.enType = AddrType_TermMeasure;
    ToCenterList.append(newNode);
}

////校验单双枪充电信息
//bool cDCCanProtocol::CheckChargeMannerInfo(FrameChargerMannerInfo_DC strFrame, unsigned char canID)
//{
//    if(strFrame.chargeManner == SINGLE_CHARGE)//单枪充电
//    {//校验主枪与源地址是否相同
//        if(strFrame.canID_master == canID)
//        {
//            return true;
//        }
//    }
//    else
//    {
//        return (CheckChargeGunGroupInfo(strFrame, canID));
//    }

//    return false;
//}
///*
// *校验多枪组
//*/
//bool cDCCanProtocol::CheckChargeGunGroupInfo(FrameChargerMannerInfo_DC strFrame, unsigned char canID)
//{
//    struct db_result_st result;
//    QString strSql;
//    int iCount=0;
//    unsigned char temp[7];

//    memset(temp,0x00,7);
//    memcpy(temp,&strFrame.canID_master,7);

//    //CAN地址是否处于多枪分组中，如果存在则获取组编号
//    strSql.sprintf("SELECT * FROM chargegun_group_table WHERE gun1=%d OR gun2=%d OR gun3=%d OR gun4=%d OR gun5=%d;",
//            canID, canID, canID, canID, canID);
//    if(pDBOperate->DBSqlQuery(strSql.toAscii().data(), &result, DB_PARAM) != 0)
//        return;

//    if(result.row > 0)
//    {
//        //数据库中有
//        iCount=0;
//        for(int num=0;num<7;num++)
//        {
//            if(atoi(result.result[2+num]))
//                iCount ++;
//             for(int cmp =0;cmp<7;cmp++)
//             {
//                 if(temp[cmp] == atoi(result.result[2+num]))
//                 {
//                     break;
//                 }
//             }
//             if(cmp==7)
//             {
//                 //cuowu
//                 //break;
//                   pDBOperate->DBQueryFree(&result);
//                 return false;
//             }
//        }
//          pDBOperate->DBQueryFree(&result);
//        if(iCount == strFrame.chargeManner)
//             return true;
//        else
//            return false;
//    }
//    else
//    {
//        //数据库中没有
//        if(strFrame.chargeManner == COUPLE_CHARGE)
//        {
//            if(qAbs(strFrame.canID_master - strFrame.canID_slave1) == 1)
//                 return true;
//            else
//                 return false;
//        }
//        else
//        {
//            for (int count =0;count < strFrame.chargeManner;count ++)
//            {
//                for(int y =0 ;y<strFrame.chargeManner;y++)
//                {
//                    if(qAbs(temp[count] - temp[y]) == 1)
//                    {
//                        break;
//                    }
//                }
//                if(y ==strFrame.chargeManner )
//                {
//                    return false;
//                }
//            }
//            return true;
//        }
//    }
//    return false;
//}

//解析充电方式(单枪充电／双枪充电)信息帧
void cDCCanProtocol::ParseFrameChargeManner(can_frame *srcFrame, QList<CanMapNode>  &ToCenterList)
{
    CanMapNode newNode;
   // CtrlCmdAck result = Ack_CmdAck;

    //解帧
    FrameChargerMannerInfo_DC strFrame;
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

//解析模块故障信息
void cDCCanProtocol::ParseFrameModuleFaultInfo(unsigned char * pData, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;
    //    unsigned char ucFaultState = 0;

    //解帧
    FrameModuleFaultInfo_DC strFrame;
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

//解析负荷调度上传相关状态----0x76(旧协议)
void cDCCanProtocol::ParseFrameLoadDispatch(unsigned char * pData, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;
    //解帧
    FrameLoadDispatch_DC strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));
    //添加群充轮充控制模式
    InsertCanInfoMapLine((char *)&strFrame.ucGroupType, sizeof(strFrame.ucGroupType), Addr_CtrlModeFlag_Term, newNode.stCanMap);
    //群充策略
    InsertCanInfoMapLine((char *)&strFrame.ucGroupStrategy, sizeof(strFrame.ucGroupStrategy), Addr_GroupModeFlag_Term, newNode.stCanMap);
    //辅助电源类型
    InsertCanInfoMapLine((char *)&strFrame.ucAuxPowerType, sizeof(strFrame.ucAuxPowerType), Addr_AuxPowerType_Term, newNode.stCanMap);
    //是否有空闲模块
    //    InsertCanInfoMapLine((char *)&strFrame.ucFreeModuleFlag, sizeof(strFrame.ucFreeModuleFlag), Addr_BMSProtocolVer_BMS, newNode.stCanMap);
    //车辆排队信息
    //    InsertCanInfoMapLine((char *)&strFrame.ucQueueMsg, sizeof(strFrame.ucQueueMsg), Addr_BMSProtocolVer_BMS, newNode.stCanMap);

    newNode.enType = AddrType_TermSignal;
    ToCenterList.append(newNode);
}
//解析BMS信息----握手阶段1
void cDCCanProtocol::ParseFrameBMSHandShake1(unsigned char * pData, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;
    //解帧
    FrameBMSHandShake1 strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));
    //    for(int i = 0 ; i < 3; i++)
    //    {
    //    }
    QString tempStr;
    tempStr.sprintf("%d.%d",strFrame.ucBMSProtocolVer1, strFrame.usBMSProtocolVer2);
    //添加BMS协议版本号
//    InsertCanInfoMapLine((char *)&strFrame.ucBMSProtocolVer, sizeof(strFrame.ucBMSProtocolVer), Addr_BMSProtocolVer_BMS, newNode.stCanMap);
    newNode.stCanMap.insert(Addr_BMSProtocolVer_BMS, tempStr.toAscii());
    newNode.enType = AddrType_TermBMS;
    ToCenterList.append(newNode);
}

//解析BMS信息----握手阶段2
void cDCCanProtocol::ParseFrameBMSHandShake2(unsigned char * pData, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;
    float fMaxAllowedVoltage = 0;   //最高允许充电总电压
    //解帧
    FrameBMSHandShake2 strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));
    fMaxAllowedVoltage = strFrame.usMaxAllowedVoltage*0.1;
    //最高允许充电总电压
    InsertCanInfoMapLine((char *)&fMaxAllowedVoltage, sizeof(fMaxAllowedVoltage), Addr_MaxAllowedVoltage_BMS, newNode.stCanMap);

    newNode.enType = AddrType_TermBMS;
    ToCenterList.append(newNode);
}

//解析BMS信息----握手阶段3
void cDCCanProtocol::ParseFrameBMSHandShake3(FrameLongPackage *pLongPackage, QList<CanMapNode> &ToCenterList)
{
    QString tempStr;
    CanMapNode newNode;
    float fBatteryRatedCapacity;//电池额定容量Ah
    float fBatteryRatedVoltage;//电池额定电压V
    unsigned int uiChargeTimeCount = 0; //充电次数
    FrameBMSHandShake3 strFrame;
    memcpy((unsigned char *)&strFrame, pLongPackage->pData, sizeof(strFrame));
    //赋值
    fBatteryRatedCapacity = strFrame.usBatteryRatedCapacity*0.1;
    fBatteryRatedVoltage = strFrame.usBatteryRatedVoltage*0.1;
    uiChargeTimeCount = strFrame.ucChargeTimeCount[2]*65536+strFrame.ucChargeTimeCount[1]* 256 + strFrame.ucChargeTimeCount[0];

    //BMS协议版本号

    tempStr.clear();
    tempStr.sprintf("%d.%d",strFrame.ucBMSProtocolVer1, strFrame.usBMSProtocolVer2);
    newNode.stCanMap.insert(Addr_BMSProtocolVer_BMS, tempStr.toAscii());

//    InsertCanInfoMapLine((char *)&strFrame.ucBMSProtocolVer, sizeof(strFrame.ucBMSProtocolVer), Addr_BMSProtocolVer_BMS, newNode.stCanMap);

    //电池类型
    InsertCanInfoMapLine((char *)&strFrame.ucBatteryType, sizeof(strFrame.ucBatteryType), Addr_BatteryType_BMS, newNode.stCanMap);

    //电池额定容量
    InsertCanInfoMapLine((char *)&fBatteryRatedCapacity, sizeof(fBatteryRatedCapacity), Addr_BatteryRatedCapacity_BMS, newNode.stCanMap);
    //电池额定电压
    InsertCanInfoMapLine((char *)&fBatteryRatedVoltage, sizeof(fBatteryRatedVoltage), Addr_BatteryRatedVoltage_BMS, newNode.stCanMap);
    //ASCII,生产厂商
    InsertCanInfoMapLine((char *)&strFrame.cManufacturer, 4, Addr_BatteryManufacturer_BMS, newNode.stCanMap);

    //电池组序号
    InsertCanInfoMapLine((char *)&strFrame.uiBatterySerialNum, sizeof(strFrame.uiBatterySerialNum), Addr_BatterySerialNum_BMS, newNode.stCanMap);

    //生产日期:年
//    InsertCanInfoMapLine((char *)&strFrame.ucProduceYear, 3, Addr_BatteryProduceDate_BMS, newNode.stCanMap);
    tempStr.clear();
    tempStr.sprintf("%d:%d:%d",strFrame.ucProduceYear + 1985, strFrame.ucProduceMonth, strFrame.ucProduceDay);
    newNode.stCanMap.insert(Addr_BatteryProduceDate_BMS, tempStr.toAscii());


    //电池组充电次数
    InsertCanInfoMapLine((char *)&uiChargeTimeCount, sizeof(uiChargeTimeCount), Addr_BatteryChargeTime_BMS, newNode.stCanMap);

    //电池组产权标识
    InsertCanInfoMapLine((char *)&strFrame.ucOwnerFlag, sizeof(strFrame.ucOwnerFlag), Addr_BatteryOwnerFlag_BMS, newNode.stCanMap);

    //ASCII, VIN号
    InsertCanInfoMapLine((char *)&strFrame.chVIN, 17, Addr_BatteryVIN_BMS, newNode.stCanMap);

    //BMS软件版本号
    tempStr.clear();
    tempStr.sprintf("%d:%d:%d-%d",((strFrame.ucBMSSoftwareVer[3] << 8) + strFrame.ucBMSSoftwareVer[4]), int(strFrame.ucBMSSoftwareVer[2]), int(strFrame.ucBMSSoftwareVer[1]), int(strFrame.ucBMSSoftwareVer[0]));
    newNode.stCanMap.insert(Addr_BMSSoftwareVer_BMS, tempStr.toAscii());

//    InsertCanInfoMapLine((char *)&strFrame.ucBMSSoftwareVer, sizeof(strFrame.ucBMSSoftwareVer), Addr_BMSSoftwareVer_BMS, newNode.stCanMap);

//    for(int i = 0 ; i < 8; i++)
//    {
//    }
    newNode.enType = AddrType_TermBMS;
    ToCenterList.append(newNode);
}

//解析BMS信息----BMS参数配置阶段1
void cDCCanProtocol::ParseFrameBMSParamSet1(FrameLongPackage * pLongPackage, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;
    float fTemp = 0;
    float fSingleBatteryMaxAllowedVlotage;//单体电池最大允许电压
    float fMaxAllowedCurrent;//最大允许电流
    float fBatteryTotalEnergy;//电池总容量,kwh
    float fMaxParamAllowedVoltage;//最大允许电压
    //short sMaxAllowedTemperature;//最高允许温度
    float fParamSOC;//参数配置阶段SOC
    float fBatteryVoltage;//电池电压
    FrameBMSParamSet1 strFrame;
    memcpy((unsigned char *)&strFrame, pLongPackage->pData, sizeof(strFrame));
    //赋值
    fSingleBatteryMaxAllowedVlotage = strFrame.usSingleBatteryMaxAllowedVlotage * 0.01;
    fMaxAllowedCurrent = strFrame.usMaxAllowedCurrent * 0.1 - 2000;
    fBatteryTotalEnergy = strFrame.usBatteryTotalEnergy * 0.1;
    fMaxParamAllowedVoltage = strFrame.usMaxParamAllowedVoltage * 0.1;

    fParamSOC = strFrame.usParamSOC*0.1;
    fBatteryVoltage = strFrame.usBatteryVoltage*0.1;

    //单体电池最大允许电压
    InsertCanInfoMapLine((char *)&fSingleBatteryMaxAllowedVlotage, sizeof(fSingleBatteryMaxAllowedVlotage), Addr_SingleBatteryMaxAllowedVoltage_BMS, newNode.stCanMap);

    //最大允许电流
    InsertCanInfoMapLine((char *)&fMaxAllowedCurrent, sizeof(fMaxAllowedCurrent), Addr_MaxAllowedCurrent_BMS, newNode.stCanMap);
    //电池总容量,kwh
    InsertCanInfoMapLine((char *)&fBatteryTotalEnergy, sizeof(fBatteryTotalEnergy), Addr_BatteryTotalEnergy_BMS, newNode.stCanMap);

    //最大允许电压
    InsertCanInfoMapLine((char *)&fMaxParamAllowedVoltage, sizeof(fMaxParamAllowedVoltage), Addr_MaxParamAllowedVoltage_BMS, newNode.stCanMap);

    //最高允许温度
    fTemp = strFrame.ucMaxAllowedTemperature - 50;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_MaxtAllowedTemp_BMS, newNode.stCanMap);

    //参数配置阶段SOC
    InsertCanInfoMapLine((char *)&fParamSOC, sizeof(fParamSOC), Addr_ParamSOC_BMS, newNode.stCanMap);

    //电池电压
    InsertCanInfoMapLine((char *)&fBatteryVoltage, sizeof(fBatteryVoltage), Addr_BatteryVoltage_BMS, newNode.stCanMap);

    newNode.enType = AddrType_TermBMS;
    ToCenterList.append(newNode);
}

//解析BMS信息----BMS参数配置阶段2
void cDCCanProtocol::ParseFrameBMSParamSet2(unsigned char * pData, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;
    float fMaxOutputVoltage;//最大输出电压
    float fMinOutputVoltage;//最小输出电压
    float fMaxOutputCurrent;//最大输出电流
    float fMinOutputCurrent;//最小输出电流
    //解帧
    FrameBMSParamSet2 strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));
    fMaxOutputVoltage = strFrame.usMaxOutputVoltage*0.1;
    fMinOutputVoltage = strFrame.usMinOutputVoltage*0.1;
    fMaxOutputCurrent = strFrame.usMaxOutputCurrent*0.1 - 2000;
    fMinOutputCurrent = strFrame.usMinOutputCurrent*0.1 - 2000;
    //最高允许充电总电压
    InsertCanInfoMapLine((char *)&fMaxOutputVoltage, sizeof(fMaxOutputVoltage), Addr_MaxOutputVoltage_BMS, newNode.stCanMap);

    //最高允许充电总电压
    InsertCanInfoMapLine((char *)&fMinOutputVoltage, sizeof(fMinOutputVoltage), Addr_MinOutputVoltage_BMS, newNode.stCanMap);

    //最高允许充电总电压
    InsertCanInfoMapLine((char *)&fMaxOutputCurrent, sizeof(fMaxOutputCurrent), Addr_MaxOutputCurrent_BMS, newNode.stCanMap);

    //最高允许充电总电压
    InsertCanInfoMapLine((char *)&fMinOutputCurrent, sizeof(fMinOutputCurrent), Addr_MinOutputCurrent_BMS, newNode.stCanMap);


    newNode.enType = AddrType_TermBMS;
    ToCenterList.append(newNode);
}

//解析BMS信息----BMS充电阶段1
void cDCCanProtocol::ParseFrameBMSCharging1(unsigned char * pData, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;
    float fNeedVoltage;//需求电压
    float fNeedCurrent;//需求电流
    float fRemainChargeTime;//剩余充电时间
    //解帧
    FrameBMSCharging1 strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));
    fNeedVoltage = strFrame.usNeedVoltage*0.1;
    fNeedCurrent = strFrame.usNeedCurrent*0.1 - 2000;
    fRemainChargeTime = strFrame.usRemainChargeTime;
    //需求电压
    InsertCanInfoMapLine((char *)&fNeedVoltage, sizeof(fNeedVoltage), Addr_NeedVoltage_BMS, newNode.stCanMap);

    //需求电流
    InsertCanInfoMapLine((char *)&fNeedCurrent, sizeof(fNeedCurrent), Addr_NeedCurrent_BMS, newNode.stCanMap);
    //充电类型
    InsertCanInfoMapLine((char *)&strFrame.ucChargeType, sizeof(strFrame.ucChargeType), Addr_ChargeType_BMS, newNode.stCanMap);

    //剩余充电时间
    InsertCanInfoMapLine((char *)&fRemainChargeTime, sizeof(fRemainChargeTime), Addr_LeftTime, newNode.stCanMap);

    newNode.enType = AddrType_TermBMS;
    ToCenterList.append(newNode);
}

//解析BMS信息----BMS充电阶段2
void cDCCanProtocol::ParseFrameBMSCharging2(unsigned char * pData, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;
    float fChargeVoltageMeasured;//充电电压测量值
    float fChargeCurrentMeasured;//充电电流测量值
    float fHighestSingleBatteryVoltage;//最高单体电池电压
    //解帧
    FrameBMSCharging2 strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));
    fChargeVoltageMeasured = strFrame.usChargeVoltageMeasured*0.1;
    fChargeCurrentMeasured = strFrame.usChargeCurrentMeasured*0.1 - 2000;
    fHighestSingleBatteryVoltage = strFrame.usHighestSingleBatteryVoltage*0.01;

    //充电电压测量值
    InsertCanInfoMapLine((char *)&fChargeVoltageMeasured, sizeof(fChargeVoltageMeasured), Addr_ChargeVoltageMeasured_BMS, newNode.stCanMap);

    //充电电流测量值
    InsertCanInfoMapLine((char *)&fChargeCurrentMeasured, sizeof(fChargeCurrentMeasured), Addr_ChargeCurrentMeasured_BMS, newNode.stCanMap);

    //最高单体电池电压
    InsertCanInfoMapLine((char *)&fHighestSingleBatteryVoltage, sizeof(fHighestSingleBatteryVoltage), Addr_MaxSingleBatteryVoltage_BMS, newNode.stCanMap);

    //最高单体电池组号
    //InsertCanInfoMapLine((char *)&strFrame.ucHighestSingleBatterySerialNum, sizeof(strFrame.ucHighestSingleBatterySerialNum), Addr_MaxSingleBatteryVoltageSerial_BMS, newNode.stCanMap);  //nihai modify 20170523
    InsertCanInfoMapLine((char *)&strFrame.ucHighestSingleBatterySerialNum, sizeof(strFrame.ucHighestSingleBatterySerialNum), Addr_MaxSingleBatterySerialNum_BMS, newNode.stCanMap);

    //当前SOC
   // InsertCanInfoMapLine((char *)&strFrame.ucNowSOC, sizeof(strFrame.ucNowSOC), Addr_MaxSingleBatterySerialNum_BMS, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&strFrame.ucNowSOC, sizeof(strFrame.ucNowSOC), Addr_NowSOC_BMS, newNode.stCanMap);



    //ocean modify 20170523
    if((strFrame.ucNowSOC>=0) && (strFrame.ucNowSOC <=100))
    {
        newNode.enType = AddrType_TermBMS;
        ToCenterList.append(newNode);
    }
}

//解析BMS信息----BMS充电阶段3
void cDCCanProtocol::ParseFrameBMSCharging3(unsigned char * pData, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;
    short sHighestBatteryTemp;//最高电池温度
    short sLowestBatteryTemp;//最低电池温度
    unsigned char ucFaultCode = 0;//BMS故障代码
    unsigned char ucChargePermit = 0;//充电允许标志
    //解帧
    FrameBMSCharging3 strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));
    sHighestBatteryTemp = strFrame.ucHighestBatteryTemp - 50;
    sLowestBatteryTemp = strFrame.ucLowestBatteryTemp - 50;
    ucChargePermit = strFrame.stFault.stByte2.ucChargePermitFlag;
    ucFaultCode = CheckFaultCode_BMSCharge3(strFrame.stFault);
    //单体电池编号
    InsertCanInfoMapLine((char *)&strFrame.ucSingleBatteryNum, sizeof(strFrame.ucSingleBatteryNum), Addr_SingleBatteryNum_BMS, newNode.stCanMap);

    //最高电池温度
    InsertCanInfoMapLine((char *)&sHighestBatteryTemp, sizeof(sHighestBatteryTemp), Addr_MaxBatteryTemp_BMS, newNode.stCanMap);

    //最高温度检测点号
    InsertCanInfoMapLine((char *)&strFrame.ucHighestTempPointNum, sizeof(strFrame.ucHighestTempPointNum), Addr_MaxTempPointNum_BMS, newNode.stCanMap);

    //最低电池温度
    InsertCanInfoMapLine((char *)&sLowestBatteryTemp, sizeof(sLowestBatteryTemp), Addr_MinBatteryTemp_BMS, newNode.stCanMap);

    //最低温度检测点号
    InsertCanInfoMapLine((char *)&strFrame.ucLowestTempPointNum, sizeof(strFrame.ucLowestTempPointNum), Addr_MinTempPointNum_BMS, newNode.stCanMap);

    //BMS充电允许标识
    InsertCanInfoMapLine((char *)&ucChargePermit, sizeof(ucChargePermit), Addr_ChargePermitFlag_BMS, newNode.stCanMap);

    newNode.enType = AddrType_TermBMS;
    ToCenterList.append(newNode);
    newNode.stCanMap.clear();

    //BMS故障代码
    InsertCanInfoMapLine((char *)&ucFaultCode, sizeof(ucFaultCode), Addr_BMSFaultCode_Term, newNode.stCanMap);

    newNode.enType = AddrType_TermSignal;
    ToCenterList.append(newNode);
}

//解析BMS信息----BMS充电结束阶段1
void cDCCanProtocol::ParseFrameBMSChargeEnd1(unsigned char * pData, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;

    //解帧
    FrameBMSChargeEnd1 strFrame;
    int iTemp = 0;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    //BMS中止充电原因
    iTemp = *(unsigned char *)&strFrame.stBMSStopReason;
    InsertCanInfoMapLine((char *)&iTemp, sizeof(iTemp), Addr_BMSStopReason_BMS, newNode.stCanMap);
    //BMS中止充电故障原因
    iTemp = *(unsigned short *)&strFrame.stBMSFaultReason;
    InsertCanInfoMapLine((char *)&iTemp, sizeof(iTemp), Addr_BMSFaultReason_BMS, newNode.stCanMap);
    //BMS中止充电错误原因
    iTemp = *(unsigned char *)&strFrame.stBMSErrorReason;
    InsertCanInfoMapLine((char *)&iTemp, sizeof(iTemp), Addr_BMSErrorReason_BMS, newNode.stCanMap);

    newNode.enType = AddrType_TermBMS;
    ToCenterList.append(newNode);
}

//解析BMS信息----BMS充电结束阶段2
void cDCCanProtocol::ParseFrameBMSChargeEnd2(unsigned char * pData, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;

    //解帧
    FrameBMSChargeEnd2 strFrame;
    int iTemp = 0;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    //充电机中止充电原因
    iTemp = *(unsigned char *)&(strFrame.stChargerStopReason);
    InsertCanInfoMapLine((char *)&iTemp, sizeof(iTemp), Addr_ChargerStopReason_BMS, newNode.stCanMap);

    //充电机中止充电故障原因
    iTemp = *(unsigned short *)&(strFrame.stChargerFaultReason);
    InsertCanInfoMapLine((char *)&iTemp, sizeof(iTemp), Addr_ChargerFaultReason_BMS, newNode.stCanMap);

    //充电机中止充电错误原因
    iTemp = *(unsigned char *)&(strFrame.stChargerErrorReason);
    InsertCanInfoMapLine((char *)&iTemp, sizeof(iTemp), Addr_ChargerErrorReason_BMS, newNode.stCanMap);

    newNode.enType = AddrType_TermBMS;
    ToCenterList.append(newNode);
}

//解析BMS信息----BMS充电结束阶段3
void cDCCanProtocol::ParseFrameBMSChargeEnd3(unsigned char * pData, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;

    //    unsigned char ucChargeEndSOC;//中止SOC状态
    float fLowestSingleVoltage;//最低单体电池电压
    float fHighestSingleVoltage;//最高单体电池电压
    float fTemp = 0;
    //解帧
    FrameBMSChargeEnd3 strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    fLowestSingleVoltage = strFrame.usLowestSingleVoltage*0.01;//最低单体电池电压
    fHighestSingleVoltage = strFrame.usHighestSingleVoltage*0.01;//最高单体电池电压

    //中止SOC状态
    fTemp = strFrame.ucChargeEndSOC;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_ChargeEndSOC_BMS, newNode.stCanMap);

    //最低单体电池电压
    InsertCanInfoMapLine((char *)&fLowestSingleVoltage, sizeof(fLowestSingleVoltage), Addr_MinSingleVoltage_BMS_END, newNode.stCanMap);

    //最高单体电池电压
    InsertCanInfoMapLine((char *)&fHighestSingleVoltage, sizeof(fHighestSingleVoltage), Addr_MaxSingleVoltage_BMS, newNode.stCanMap);

    //最低温度
    fTemp = strFrame.ucLowestTemperature - 50;//最低温度
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_MinTemp_BMS, newNode.stCanMap);

    //最高温度
    fTemp = strFrame.ucHighestTemperature - 50;//最高温度
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_MaxTemp_BMS, newNode.stCanMap);

    newNode.enType = AddrType_TermBMS;
    ToCenterList.append(newNode);
}

//解析BMS信息----BMS充电结束阶段4
void cDCCanProtocol::ParseFrameBMSChargeEnd4(unsigned char * pData, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;
    QString s1;
    //解帧
    FrameBMSChargeEnd4 strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    //BMS错误报文
    InsertCanInfoMapLine((char *)&strFrame, sizeof(strFrame), Addr_BMSErrorFrame_BMS, newNode.stCanMap);

    newNode.enType = AddrType_TermBMS;
    ToCenterList.append(newNode);
}

//解析BMS信息----BMS充电结束阶段5
void cDCCanProtocol::ParseFrameBMSChargeEnd5(unsigned char * pData, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;
    //解帧
    FrameBMSChargeEnd5 strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    //充电机错误报文
    InsertCanInfoMapLine((char *)&strFrame, sizeof(strFrame), Addr_ChargerErrorFrame_BMS, newNode.stCanMap);

    newNode.enType = AddrType_TermBMS;
    ToCenterList.append(newNode);
}

//解析直流模块信息1
void cDCCanProtocol::ParseFrameModuleInfo1(unsigned char * pData, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;
    unsigned char ucWorkState = 0;//工作状态
    float fOutVoltage;//输出电压
    float fOutCurrent;//输出电流
    short sM1Temperature;//M1温度
    //解帧
    FrameModuleInfo1_CCU strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));
    ucWorkState = strFrame.stByte2.ucWorkState;
    fOutVoltage = strFrame.usOutVoltage * 0.1;
    fOutCurrent = (strFrame.usOutCurrent - 4000) * 0.1;
//    fOutCurrent = -15.5;
    sM1Temperature = (strFrame.ucM1Temperature) - 50;

    //工作状态
    InsertCanInfoMapLine((char *)&ucWorkState, sizeof(ucWorkState), Addr_DCModuleWorkState_DCcab, newNode.stCanMap);
    //内部ID
    InsertCanInfoMapLine((char *)&strFrame.ucID, sizeof(strFrame.ucID), Addr_DevID_DC_Comm, newNode.stCanMap);
    newNode.enType = AddrType_MODSignal_DCCab;
    ToCenterList.append(newNode);
    newNode.stCanMap.clear();

    //输出电压
    InsertCanInfoMapLine((char *)&fOutVoltage, sizeof(fOutVoltage), Addr_DCModuleOutVoltage_DCcab, newNode.stCanMap);

    //输出电流
    InsertCanInfoMapLine((char *)&fOutCurrent, sizeof(fOutCurrent), Addr_DCModuleOutCurrent_DCcab, newNode.stCanMap);

    //M1温度
    InsertCanInfoMapLine((char *)&sM1Temperature, sizeof(sM1Temperature), Addr_DCModuleM1Temp_DCcab, newNode.stCanMap);

    //内部ID
    InsertCanInfoMapLine((char *)&strFrame.ucID, sizeof(strFrame.ucID), Addr_DevID_DC_Comm, newNode.stCanMap);

    newNode.enType = AddrType_MODMeasure_DCCab;
    ToCenterList.append(newNode);
}

//解析直流模块信息2
void cDCCanProtocol::ParseFrameModuleInfo2(unsigned char * pData, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;
    float fInAVoltage;//A相输入电压
    float fInBVoltage;//B相输入电压
    float fInCVoltage;//C相输入电压
    short sEnvTemperature;//环境温度

    //解帧
    FrameModuleInfo2_CCU strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    fInAVoltage = strFrame.usInAVoltage * 0.1;
    fInBVoltage = strFrame.usInBVoltage * 0.1;
    fInCVoltage = strFrame.usInCVoltage * 0.1;
    sEnvTemperature = (strFrame.ucEnvTemperature) - 50;

    //A相输入电压
    InsertCanInfoMapLine((char *)&fInAVoltage, sizeof(fInAVoltage), Addr_DCModuleInAVoltage_DCcab, newNode.stCanMap);

    //B相输入电压
    InsertCanInfoMapLine((char *)&fInBVoltage, sizeof(fInBVoltage), Addr_DCModuleInBVoltage_DCcab, newNode.stCanMap);

    //C相输入电压
    InsertCanInfoMapLine((char *)&fInCVoltage, sizeof(fInCVoltage), Addr_DCModuleInCVoltage_DCcab, newNode.stCanMap);

    //环境温度
    InsertCanInfoMapLine((char *)&sEnvTemperature, sizeof(sEnvTemperature), Addr_DCModuleEnvTemp_DCcab, newNode.stCanMap);

    //内部ID
    InsertCanInfoMapLine((char *)&strFrame.ucID, sizeof(strFrame.ucID), Addr_DevID_DC_Comm, newNode.stCanMap);

    newNode.enType = AddrType_MODMeasure_DCCab;
    ToCenterList.append(newNode);
}

//解析直流模块信息3
void cDCCanProtocol::ParseFrameModuleInfo3(unsigned char * pData, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;

    float fInCurrent;//输入电流
    unsigned int uiTotalRunTime;//总运行时间(min)
    unsigned int uiSwitchTime;//切换次数(次)
    //解帧
    FrameModuleInfo3_CCU strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    fInCurrent = strFrame.usInCurrent * 0.1 - 400;
    uiTotalRunTime = strFrame.usTotalRunTime ;
    uiSwitchTime = strFrame.usSwitchTime * 10;

    //输入电流
    InsertCanInfoMapLine((char *)&fInCurrent, sizeof(fInCurrent), Addr_DCModuleInCurrent_DCcab, newNode.stCanMap);

    //总运行时间
    InsertCanInfoMapLine((char *)&uiTotalRunTime, sizeof(uiTotalRunTime), Addr_DCModuleTotalRunTime_DCcab, newNode.stCanMap);

    //切换次数
    InsertCanInfoMapLine((char *)&uiSwitchTime, sizeof(uiSwitchTime), Addr_DCModuleSwitchTime_DCcab, newNode.stCanMap);

    //内部ID
    InsertCanInfoMapLine((char *)&strFrame.ucID, sizeof(strFrame.ucID), Addr_DevID_DC_Comm, newNode.stCanMap);

    newNode.enType = AddrType_MODMeasure_DCCab;
    ToCenterList.append(newNode);
    newNode.stCanMap.clear();

    //切换次数
    InsertCanInfoMapLine((char *)&strFrame.ucGroupNum, sizeof(strFrame.ucGroupNum), Addr_DCModuleGroupNum_DCcab, newNode.stCanMap);

    //内部ID
    InsertCanInfoMapLine((char *)&strFrame.ucID, sizeof(strFrame.ucID), Addr_DevID_DC_Comm, newNode.stCanMap);

    newNode.enType = AddrType_MODSignal_DCCab;
    ToCenterList.append(newNode);
}

//解析PDU信息1
void cDCCanProtocol::ParseFramePDUInfo1(unsigned char * pData, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;
    unsigned char ucWorkState = 0;//工作状态
    float fOutVoltage;//输出电压
    float fOutCurrent;//输出电流
    short sEnvTemperature;//环境温度
    unsigned char ucTemp = 0;
    //解帧
    FramePDUInfo1_CCU strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));
    ucWorkState = strFrame.stByte2.ucWorkState;
    fOutVoltage = strFrame.usOutVoltage * 0.1;
    fOutCurrent = (strFrame.usOutCurrent - 20000) * 0.1;
//    fOutCurrent = -25.5;

    sEnvTemperature = strFrame.ucEnvTemperature - 50;

    //工作状态
    InsertCanInfoMapLine((char *)&ucWorkState, sizeof(ucWorkState), Addr_PDUWorkState_DCcab, newNode.stCanMap);

    ucTemp = strFrame.stByte2.ucGreenLight;
    InsertCanInfoMapLine((char *)&ucTemp, sizeof(ucTemp), Addr_PDUGreenLight_DCcab, newNode.stCanMap);
    ucTemp = strFrame.stByte2.ucYellowLight;
    InsertCanInfoMapLine((char *)&ucTemp, sizeof(ucTemp), Addr_PDUYellowLight_DCcab, newNode.stCanMap);
    ucTemp = strFrame.stByte2.ucRedLight;
    InsertCanInfoMapLine((char *)&ucTemp, sizeof(ucTemp), Addr_PDURedLight_DCcab, newNode.stCanMap);

    //内部ID
    InsertCanInfoMapLine((char *)&strFrame.ucID, sizeof(strFrame.ucID), Addr_DevID_DC_Comm, newNode.stCanMap);

    newNode.enType = AddrType_PDUSignal_DCCab;
    ToCenterList.append(newNode);
    newNode.stCanMap.clear();

    //输出电压
    InsertCanInfoMapLine((char *)&fOutVoltage, sizeof(fOutVoltage), Addr_PDUOutVoltage_DCcab, newNode.stCanMap);

    //输出电流
    InsertCanInfoMapLine((char *)&fOutCurrent, sizeof(fOutCurrent), Addr_PDUOutCurrent_DCcab, newNode.stCanMap);



    //环境温度
    InsertCanInfoMapLine((char *)&sEnvTemperature, sizeof(sEnvTemperature), Addr_PDUEnvTemp_DCcab, newNode.stCanMap);

    //内部ID
    InsertCanInfoMapLine((char *)&strFrame.ucID, sizeof(strFrame.ucID), Addr_DevID_DC_Comm, newNode.stCanMap);

    newNode.enType = AddrType_PDUMeasure_DCCab;
    ToCenterList.append(newNode);
}

//解析PDU信息2
void cDCCanProtocol::ParseFramePDUInfo2(unsigned char * pData, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;
    float fAmmeterEnergy;//电表度数
    unsigned int uiTotalRunTime;//总运行时间(min)
    unsigned int uiSwitchTime;//切换次数(次)
    short sRadiatorTemperature;//散热器温度
    //解帧
    FramePDUInfo2_CCU strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    fAmmeterEnergy = strFrame.usAmmeterEnergy * 20;
    uiTotalRunTime = strFrame.usTotalRunTime;
    uiSwitchTime = strFrame.usSwitchTime * 10;
    sRadiatorTemperature = strFrame.ucRadiatorTemperature - 50;

    //电表度数
    InsertCanInfoMapLine((char *)&fAmmeterEnergy, sizeof(fAmmeterEnergy), Addr_PDUEnergy_DCcab, newNode.stCanMap);

    //总运行时间
    InsertCanInfoMapLine((char *)&uiTotalRunTime, sizeof(uiTotalRunTime), Addr_PDUTotalRunTime_DCcab, newNode.stCanMap);

    //切换次数
    InsertCanInfoMapLine((char *)&uiSwitchTime, sizeof(uiSwitchTime), Addr_PDUSwitchTime_DCcab, newNode.stCanMap);

    //散热器温度
    InsertCanInfoMapLine((char *)&sRadiatorTemperature, sizeof(sRadiatorTemperature), Addr_PDURadTemp_DCcab, newNode.stCanMap);

    //内部ID
    InsertCanInfoMapLine((char *)&strFrame.ucID, sizeof(strFrame.ucID), Addr_DevID_DC_Comm, newNode.stCanMap);

    newNode.enType = AddrType_PDUMeasure_DCCab;
    ToCenterList.append(newNode);
}

//解析PDU信息3
void cDCCanProtocol::ParseFramePDUInfo3(unsigned char * pData, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;
    //解帧
    FramePDUInfo3_CCU strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    //负对地阻值
    InsertCanInfoMapLine((char *)&(strFrame.usResistanceMinus), sizeof(strFrame.usResistanceMinus), Addr_PDUResistanceMinus_DCcab, newNode.stCanMap);

    //正对地阻值
    InsertCanInfoMapLine((char *)&(strFrame.usResistancePlus), sizeof(strFrame.usResistancePlus), Addr_PDUResistancePlus_DCcab, newNode.stCanMap);

    //内部ID
    InsertCanInfoMapLine((char *)&strFrame.ucID, sizeof(strFrame.ucID), Addr_DevID_DC_Comm, newNode.stCanMap);

    newNode.enType = AddrType_PDUMeasure_DCCab;
    ToCenterList.append(newNode);
}

//解析PDU信息4
void cDCCanProtocol::ParseFramePDUInfo4(unsigned char * pData, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;
    //解帧
    FramePDUInfo4_CCU strFrame;
    float fSetVol;  //PDU设模块输出电压
    float fSetCur;  //PDU设模块限流点

    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    fSetVol = strFrame.sSetVol * 0.1;
    fSetCur = strFrame.sSetCur * 0.01;


    //PDU设模块输出电压
    InsertCanInfoMapLine((char *)&fSetVol, sizeof(fSetVol), Addr_PDUSetVol_DCcab, newNode.stCanMap);

    //PDU设模块限流点
    InsertCanInfoMapLine((char *)&fSetCur, sizeof(fSetCur), Addr_PDUSetCur_DCcab, newNode.stCanMap);


    //内部ID
    InsertCanInfoMapLine((char *)&strFrame.ucID, sizeof(strFrame.ucID), Addr_DevID_DC_Comm, newNode.stCanMap);

    newNode.enType = AddrType_PDUMeasure_DCCab;
    ToCenterList.append(newNode);
}

//解析分支箱信息1
void cDCCanProtocol::ParseFrameBOXInfo1(unsigned char * pData, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;
    unsigned char ucWorkState = 0;//工作状态
    float fOutVoltage;//输出电压
    float fOutCurrent;//输出电流
    short sEnvTemperature;//环境温度
    //解帧
    FrameBranchInfo1_CCU strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));
    ucWorkState = strFrame.stByte2.ucWorkState;
    fOutVoltage = strFrame.usOutVoltage * 0.1;
    fOutCurrent = (strFrame.usOutVoltage - 20000) * 0.1;
    sEnvTemperature = strFrame.ucEnvTemperature - 50;

    //工作状态
    InsertCanInfoMapLine((char *)&ucWorkState, sizeof(ucWorkState), Addr_BOXWorkState_DCcab, newNode.stCanMap);

    //内部ID
    InsertCanInfoMapLine((char *)&strFrame.ucID, sizeof(strFrame.ucID), Addr_DevID_DC_Comm, newNode.stCanMap);

    newNode.enType = AddrType_BOXSignal_DCCab;
    ToCenterList.append(newNode);
    newNode.stCanMap.clear();

    //输出电压
    InsertCanInfoMapLine((char *)&fOutVoltage, sizeof(fOutVoltage), Addr_BOXOutVoltage_DCcab, newNode.stCanMap);

    //输出电流
    InsertCanInfoMapLine((char *)&fOutCurrent, sizeof(fOutCurrent), Addr_BOXOutCurrent_DCcab, newNode.stCanMap);

    //环境温度
    InsertCanInfoMapLine((char *)&sEnvTemperature, sizeof(sEnvTemperature), Addr_BOXEnvTemp_DCcab, newNode.stCanMap);

    //内部ID
    InsertCanInfoMapLine((char *)&strFrame.ucID, sizeof(strFrame.ucID), Addr_DevID_DC_Comm, newNode.stCanMap);

    newNode.enType = AddrType_BOXMeasure_DCCab;
    ToCenterList.append(newNode);
}

//解析分支箱信息2
void cDCCanProtocol::ParseFrameBOXInfo2(unsigned char * pData, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;
    unsigned int uiTotalRunTime;//总运行时间(min)
    unsigned int uiSwitchTime;//切换次数(次)
    short sM1Temperature;//M1板温度
    //解帧
    FrameBranchInfo2_CCU strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    uiTotalRunTime = strFrame.usTotalRunTime;
    uiSwitchTime = strFrame.usSwitchTime * 10;
    sM1Temperature = strFrame.ucM1Temperature - 50;

    //总运行时间
    InsertCanInfoMapLine((char *)&uiTotalRunTime, sizeof(uiTotalRunTime), Addr_BOXTotalRunTime_DCcab, newNode.stCanMap);

    //切换次数
    InsertCanInfoMapLine((char *)&uiSwitchTime, sizeof(uiSwitchTime), Addr_BOXSwitchTime_DCcab, newNode.stCanMap);
    //M1板温度
    InsertCanInfoMapLine((char *)&sM1Temperature, sizeof(sM1Temperature), Addr_BOXM1Temp_DCcab, newNode.stCanMap);

    //内部ID
    InsertCanInfoMapLine((char *)&strFrame.ucID, sizeof(strFrame.ucID), Addr_DevID_DC_Comm, newNode.stCanMap);


    newNode.enType = AddrType_BOXMeasure_DCCab;
    ToCenterList.append(newNode);
}

//解析CCU信息1
void cDCCanProtocol::ParseFrameCCUInfo1(unsigned char * pData, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;
    //定义相关数据
    unsigned char ucWorkState ; //工作状态
    unsigned char ucInputContactor; //输入接触器状态
    unsigned char ucLinkageContactor;//联动接触器状态
    short sEnvTemperature; //环境温度
    //    unsigned char ucModleType;//设备类型

    //解帧
    FrameCCUInfo1_CCU strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    ucWorkState = strFrame.stByte2.ucWorkState;
    ucInputContactor = strFrame.stByte4.ucInputContactor;
    ucLinkageContactor = strFrame.stByte4.ucLinkageContactor;
    sEnvTemperature = strFrame.ucEnvTemperature - 50;

    //工作状态
    InsertCanInfoMapLine((char *)&ucWorkState, sizeof(ucWorkState), Addr_CCUWorkState_DCcab, newNode.stCanMap);
    //输入接触器状态
    InsertCanInfoMapLine((char *)&ucInputContactor, sizeof(ucInputContactor), Addr_CCUInputContactor_DCcab, newNode.stCanMap);
    //联动接触器状态
    InsertCanInfoMapLine((char *)&ucLinkageContactor, sizeof(ucLinkageContactor), Addr_CCULinkageContactor_DCcab, newNode.stCanMap);
    //设备类型
    InsertCanInfoMapLine((char *)&strFrame.ucModleType, sizeof(strFrame.ucModleType), Addr_CCUSystemType_DCcab, newNode.stCanMap);
    newNode.enType = AddrType_CCUSignal_DCCab;
    ToCenterList.append(newNode);
    newNode.stCanMap.clear();

    //环境温度
    InsertCanInfoMapLine((char *)&sEnvTemperature, sizeof(sEnvTemperature), Addr_CCUEnvTemp_DCcab, newNode.stCanMap);

    newNode.enType = AddrType_CCUMeasure_DCCab;
    ToCenterList.append(newNode);
}

//解析CCU信息2
void cDCCanProtocol::ParseFrameCCUInfo2(unsigned char * pData, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;
    //定义相关数据
    float fRetedPower;		  //额定功率
    float fNowPower;    //当前需求功率
    float fOutPower;    //当前输出功率
    unsigned int uiTotalRunTime;    //总运行时间
    //解帧
    FrameCCUInfo2_CCU strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    fRetedPower = strFrame.usRetedPower * 0.1;  //单位0.1kw, 精度0.1kw
    fNowPower = strFrame.usNowPower * 0.1;  //单位0.1kw, 精度0.1kw
    uiTotalRunTime = strFrame.uiTotalRunTime;   //单位小时, 精度1h
    fOutPower = strFrame.usOutPower * 0.1;

    //添加额定功率
    InsertCanInfoMapLine((char *)&fRetedPower, sizeof(fRetedPower), Addr_CabRatedPower_DCCab, newNode.stCanMap);

    //添加当前功率
    InsertCanInfoMapLine((char *)&fNowPower, sizeof(fNowPower), Addr_CabNowPower_DCCab, newNode.stCanMap);

    //添加运行时间
    InsertCanInfoMapLine((char *)&uiTotalRunTime, sizeof(uiTotalRunTime), Addr_CCUTotalRunTime_DCcab, newNode.stCanMap);

    //添加out功率
    InsertCanInfoMapLine((char *)&fOutPower, sizeof(fOutPower), Addr_CabOutPower_DCCab, newNode.stCanMap);

    newNode.enType = AddrType_CCUMeasure_DCCab;
    ToCenterList.append(newNode);
}

//解析VIN 0x74
void cDCCanProtocol::ParseFrameVIN(FrameLongPackage * pLongPackage, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;
    FrameVIN_DC stFrame;
    if(pLongPackage->pData == NULL)
    {
        return;
    }
    memset(stFrame.chData, 0x00, sizeof(stFrame.chData));
    memcpy((unsigned char *)&stFrame, pLongPackage->pData, sizeof(stFrame));
    //    QByteArray test;
    //    test.append(stFrame.chData, sizeof(stFrame.chData));
    //    //添加VIN号
    InsertCanInfoMapLine(stFrame.chData, sizeof(stFrame.chData), Addr_BatteryVIN_BMS, newNode.stCanMap);

    newNode.enType = AddrType_TermBMS;
    ToCenterList.append(newNode);

}

//解析故障点记录
void cDCCanProtocol::ParseAlarmPointRecord(FrameLongPackage * pLongPackage, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;
    InsertCanInfoMapLine((char *)pLongPackage->pData, pLongPackage->uiDataLength, Addr_DCAlarmPoint, newNode.stCanMap);
    newNode.enType = AddrType_FaultRecord_DCcab;
    ToCenterList.append(newNode);
}

//解析故障录波数据
void cDCCanProtocol::ParseFaultRecord(FrameLongPackage * pLongPackage, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;
    QString sFileName = "/mnt/nandflash/" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QFile * pfRec = new QFile(sFileName);
    pfRec->open(QIODevice::WriteOnly);
    pfRec->write((char *)pLongPackage->pData, pLongPackage->uiDataLength);
    pfRec->close();

    InsertCanInfoMapLine(sFileName.toAscii().data(), sFileName.length(), Addr_DCFaultRecord, newNode.stCanMap);
    newNode.enType = AddrType_FaultDetail_DCcab;
    ToCenterList.append(newNode);
    delete pfRec;
}

//解析终端主动防护参数 0xB0
void cDCCanProtocol::ParseTermActiveArg(FrameLongPackage * pLongPackage, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;
    FrameActiveProtectionSet stFrame;
    //通用变量定义
    float fTemp = 0;
    int iTemp = 0;
    unsigned char ucTemp = 0;
    if(pLongPackage->pData == NULL)
    {
        return;
    }
    //插入原始数据
    InsertCanInfoMapLine((char *)pLongPackage->pData, pLongPackage->uiDataLength, Addr_ArgData, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&ucTemp, sizeof(ucTemp), Addr_ArgNo, newNode.stCanMap);

    //帧解析, 转换为通用格式, 解帧
    memset(&stFrame, 0x00, sizeof(stFrame));
    memcpy((unsigned char *)&stFrame, pLongPackage->pData, sizeof(stFrame));
    InsertCanInfoMapLine((char *)pLongPackage->pData, pLongPackage->uiDataLength, Addr_ArgData, newNode.stCanMap);

    fTemp = stFrame.usBalanceCurrentCoefficient*0.01;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_BalanceCurCoe_AP, newNode.stCanMap);
    iTemp = stFrame.ucBalanceTime;
    InsertCanInfoMapLine((char *)&iTemp, sizeof(iTemp), Addr_BalanceTime_AP, newNode.stCanMap);

    iTemp = stFrame.ucTempThreshold - 50;
    InsertCanInfoMapLine((char *)&iTemp, sizeof(iTemp), Addr_TempTh_AP, newNode.stCanMap);
    iTemp = stFrame.ucTempRiseEnsureTime;
    InsertCanInfoMapLine((char *)&iTemp, sizeof(iTemp), Addr_TempRiseEnsureTime_AP, newNode.stCanMap);
    iTemp = stFrame.ucNoTempProtect;
    InsertCanInfoMapLine((char *)&iTemp, sizeof(iTemp), Addr_NoTempProtect_AP, newNode.stCanMap);

    fTemp = stFrame.usSingleOverVoltageThreshold*0.01;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_SingleOVTh_AP, newNode.stCanMap);
    iTemp = stFrame.ucSingleOverVoltageEnsureTime;
    InsertCanInfoMapLine((char *)&iTemp, sizeof(iTemp), Addr_SingleOVEnsureTime_AP, newNode.stCanMap);
    iTemp = stFrame.ucNoSingleOverVoltageProtect;
    InsertCanInfoMapLine((char *)&iTemp, sizeof(iTemp), Addr_NoSingleOVP_AP, newNode.stCanMap);

    fTemp = stFrame.usTotalOverVoltageThreshold*0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_TotalOVTh_AP, newNode.stCanMap);
    iTemp = stFrame.ucOverVoltageEnsureTime;
    InsertCanInfoMapLine((char *)&iTemp, sizeof(iTemp), Addr_TotalOVEnsureTime_AP, newNode.stCanMap);
    iTemp = stFrame.ucNoOverVoltageProtect;
    InsertCanInfoMapLine((char *)&iTemp, sizeof(iTemp), Addr_NoTotalOVP_AP, newNode.stCanMap);

    fTemp = stFrame.usOverCurrentThreshold*0.1 - 2000;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_OCTh_AP, newNode.stCanMap);
    iTemp = stFrame.ucOverCurrentEnsureTime;
    InsertCanInfoMapLine((char *)&iTemp, sizeof(iTemp), Addr_OCEnsureTime_AP, newNode.stCanMap);
    iTemp = stFrame.ucNoOverCurrentProtect;
    InsertCanInfoMapLine((char *)&iTemp, sizeof(iTemp), Addr_NoOCP_AP, newNode.stCanMap);

    iTemp = stFrame.ucOverTempThreshold - 50;
    InsertCanInfoMapLine((char *)&iTemp, sizeof(iTemp), Addr_OTTh_AP, newNode.stCanMap);
    iTemp = stFrame.ucOverTempEnsureTime;
    InsertCanInfoMapLine((char *)&iTemp, sizeof(iTemp), Addr_OTEnsureTime_AP, newNode.stCanMap);
    iTemp = stFrame.ucNoOverTempProtect;
    InsertCanInfoMapLine((char *)&iTemp, sizeof(iTemp), Addr_NoOTP_AP, newNode.stCanMap);

    iTemp = stFrame.ucBelowTempThreshold - 50;
    InsertCanInfoMapLine((char *)&iTemp, sizeof(iTemp), Addr_BTTh_AP, newNode.stCanMap);
    iTemp = stFrame.ucBelowTempEnsureTime;
    InsertCanInfoMapLine((char *)&iTemp, sizeof(iTemp), Addr_BTEnsureTime_AP, newNode.stCanMap);
    iTemp = stFrame.ucNoBelowTempProtect;
    InsertCanInfoMapLine((char *)&iTemp, sizeof(iTemp), Addr_NoBTP_AP, newNode.stCanMap);

    fTemp = stFrame.usBMSRelayAdhesionVoltageThreshold*0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_BMSRAVTh_AP, newNode.stCanMap);
    iTemp = stFrame.ucBMSRelayAdhesionEnsureTime;
    InsertCanInfoMapLine((char *)&iTemp, sizeof(iTemp), Addr_BMSRAVEnsureTime_AP, newNode.stCanMap);
    iTemp = stFrame.ucNoBMSRelayAdhesion;
    InsertCanInfoMapLine((char *)&iTemp, sizeof(iTemp), Addr_NoBMSRAVP_AP, newNode.stCanMap);

    fTemp = stFrame.usBMSRelayBreakOffVoltageThreshold*0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_BMSRBOVTh_AP, newNode.stCanMap);
    iTemp = stFrame.usBMSRelayBreakOffCurrentThreshold*0.1 - 2000;
    InsertCanInfoMapLine((char *)&iTemp, sizeof(iTemp), Addr_BMSRBOCTh_AP, newNode.stCanMap);
    iTemp = stFrame.ucBMSRelayBreakOffEnsureTime;
    InsertCanInfoMapLine((char *)&iTemp, sizeof(iTemp), Addr_BMSRBOEnsureTime_AP, newNode.stCanMap);
    iTemp = stFrame.ucNoBMSRelayBreakOff;
    InsertCanInfoMapLine((char *)&iTemp, sizeof(iTemp), Addr_NoBMSRBOP_AP, newNode.stCanMap);

    fTemp = stFrame.ucOverChargeJudgemetCoefficient*0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_OCJC_AP, newNode.stCanMap);
    fTemp = stFrame.ucOverChargeEnergyReferanceValue*0.1;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_OCERefValue_AP, newNode.stCanMap);
    iTemp = stFrame.ucOverChargeEnergyEnsureTime;
    InsertCanInfoMapLine((char *)&iTemp, sizeof(iTemp), Addr_OCEEnsureTime_AP, newNode.stCanMap);
    iTemp = stFrame.ucNoOverCharge;
    InsertCanInfoMapLine((char *)&iTemp, sizeof(iTemp), Addr_NoOverChargeP_AP, newNode.stCanMap);

    iTemp = stFrame.ucBMSDataRepetTime;
    InsertCanInfoMapLine((char *)&iTemp, sizeof(iTemp), Addr_BMSDataRepetTime_AP, newNode.stCanMap);
    iTemp = stFrame.ucNoBMSDataRepet;
    InsertCanInfoMapLine((char *)&iTemp, sizeof(iTemp), Addr_NoBMSDataRepetProtect_AP, newNode.stCanMap);
    iTemp = stFrame.ucNoBMSCheck;
    InsertCanInfoMapLine((char *)&iTemp, sizeof(iTemp), Addr_NoBMSCheck_AP, newNode.stCanMap);

    fTemp = stFrame.usOVTh_LNCM*0.01;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_OVTh_LNCM_AP, newNode.stCanMap);
    fTemp = stFrame.usOVTh_LTO*0.01;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_OVTh_LTO_AP, newNode.stCanMap);
    fTemp = stFrame.usOVTh_LMO*0.01;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_OVTh_LMO_AP, newNode.stCanMap);

    iTemp = stFrame.ucOTTh_LNCM - 50;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_OTTh_LNCM_AP, newNode.stCanMap);
    iTemp = stFrame.ucOTTh_LTO - 50;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_OTTh_LTO_AP, newNode.stCanMap);
    iTemp = stFrame.ucOTTh_LMO - 50;
    InsertCanInfoMapLine((char *)&fTemp, sizeof(fTemp), Addr_OTTh_LMO_AP, newNode.stCanMap);

    newNode.enType = AddrType_ActiveProtectQueryResult;
    ToCenterList.append(newNode);
}

//解析终端柔性充电参数 0xB1
void cDCCanProtocol::ParseTermFlexArg(FrameLongPackage * pLongPackage, QList<CanMapNode> &ToCenterList)
{
    CanMapNode newNode;
    FrameFlexibleParamSet stFrame;
    unsigned char ucType = 0;

    if(pLongPackage->pData == NULL)
    {
        return;
    }
    //帧解析, 转换为通用格式
    memset(&stFrame, 0x00, sizeof(stFrame));
    memcpy((unsigned char *)&stFrame, pLongPackage->pData, sizeof(stFrame));
    ucType = pLongPackage->pData[0];

    InsertCanInfoMapLine((char *)&ucType, sizeof(ucType), Addr_ArgNo, newNode.stCanMap);
    InsertCanInfoMapLine((char *)pLongPackage->pData, pLongPackage->uiDataLength, Addr_ArgData, newNode.stCanMap);

    switch(ucType)
    {
    case 1:
        InsertCanInfoMapLine((char *)(pLongPackage->pData +1), pLongPackage->uiDataLength - 1, Addr_SOCCoefficient_FC, newNode.stCanMap);
        break;
    case 2:
        InsertCanInfoMapLine((char *)(pLongPackage->pData +1), pLongPackage->uiDataLength - 1, Addr_TempCoefficient_FC, newNode.stCanMap);
        break;
    case 3:
        InsertCanInfoMapLine((char *)(pLongPackage->pData +1), pLongPackage->uiDataLength - 1, Addr_TimeCoefficient_FC, newNode.stCanMap);
        break;
    default:
        ucType = 0;
        break;
    }
    if(ucType == 0)
    {
        return;
    }
    newNode.enType = AddrType_ActiveProtectQueryResult;
    ToCenterList.append(newNode);
}

//解析通用静态参数设置 0xB2
void cDCCanProtocol::ParseTermGeneralStaticArg(unsigned char * pData, QList<CanMapNode>  &ToCenterList)
{

    CanMapNode newNode;
    FrameGeneralStaticParamSet stFrame;
    unsigned char ucTemp = 0;
    float fMaxGunCurrent = 0;
    memcpy((unsigned char *)&stFrame, pData, sizeof(stFrame));

    InsertCanInfoMapLine((char *)&ucTemp, sizeof(ucTemp), Addr_ArgNo, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&stFrame, sizeof(stFrame), Addr_ArgData, newNode.stCanMap);

    //添加VIN使能标识
    ucTemp = stFrame.stByte1.ucVINEnableFlag;
    InsertCanInfoMapLine((char *)&ucTemp, sizeof(ucTemp), Addr_VINEnable_GSA, newNode.stCanMap);
    //添加电子锁类型
    ucTemp = stFrame.stByte1.ucElecLockType;
    InsertCanInfoMapLine((char *)&ucTemp, sizeof(ucTemp), Addr_ElecLockType_GSA, newNode.stCanMap);
    //添加电子锁使能
    ucTemp = stFrame.stByte1.ucElecLockEnableFlag;
    InsertCanInfoMapLine((char *)&ucTemp, sizeof(ucTemp), Addr_ElecLockEnable_GSA, newNode.stCanMap);
    //添加辅源类型
    ucTemp = stFrame.stByte1.ucAuxPowerSet;
    InsertCanInfoMapLine((char *)&ucTemp, sizeof(ucTemp), Addr_AuxPowerType_GSA, newNode.stCanMap);
    //添加最大枪头电流
    fMaxGunCurrent = stFrame.usGunMaxCurrent*0.1 -2000;
    InsertCanInfoMapLine((char *)&fMaxGunCurrent, sizeof(fMaxGunCurrent), Addr_MaxGunCur_GSA, newNode.stCanMap);
    //添加设置BMS新老国标协议
    ucTemp = stFrame.stByte2.ucBMSProType;
    InsertCanInfoMapLine((char *)&ucTemp, sizeof(ucTemp), Addr_BMSProTypeSet_GSA, newNode.stCanMap);
    //添加设置终端ID
    ucTemp = stFrame.ucTermID;
    InsertCanInfoMapLine((char *)&ucTemp, sizeof(ucTemp), Addr_TermIDSet_GSA, newNode.stCanMap);

    newNode.enType = AddrType_GeneralStaticArgQueryResult;
    ToCenterList.append(newNode);

}

//解析通用动态参数设置 0xB3
void cDCCanProtocol::ParseTermGeneralDynamicArg(unsigned char * pData, QList<CanMapNode>  &ToCenterList)
{

    CanMapNode newNode;
    FrameGeneralDynamicParamSet stFrame;
    unsigned char ucTemp;
    unsigned char ucType = pData[0];

    InsertCanInfoMapLine((char *)&ucType, sizeof(ucType), Addr_ArgNo, newNode.stCanMap);
    InsertCanInfoMapLine((char *)pData, 8, Addr_ArgData, newNode.stCanMap);

    switch(pData[0])    //指令类型
    {
    case 1:
    {
        memcpy((unsigned char *)&stFrame.Data.stCmd01, pData +1, sizeof(stFrame) - 1);
        ucTemp = stFrame.Data.stCmd01.ucPriority;
        InsertCanInfoMapLine((char *)&ucTemp, sizeof(ucTemp), Addr_PriorityLevel_GDA, newNode.stCanMap);
        newNode.enType = AddrType_GeneralDynamicArgQueryResult;
        ToCenterList.append(newNode);
        break;
    }
    case 2:
    {
        memcpy((unsigned char *)&stFrame.Data.stCmd02, pData +1, sizeof(stFrame) - 1);
        InsertCanInfoMapLine((char *)&stFrame.Data.stCmd02, sizeof(stFrame.Data.stCmd02), Addr_ChargeEndTime_GDA, newNode.stCanMap);
        newNode.enType = AddrType_GeneralDynamicArgQueryResult;
        ToCenterList.append(newNode);
        break;
    }
    case 3:
    {
        memcpy((unsigned char *)&stFrame.Data.stCmd03, pData +1, sizeof(stFrame) - 1);
        InsertCanInfoMapLine((char *)&stFrame.Data.stCmd03.ucWorkModle, sizeof(stFrame.Data.stCmd03.ucWorkModle), Addr_WorkMode_GDA, newNode.stCanMap);
        InsertCanInfoMapLine((char *)&stFrame.Data.stCmd03.ucGroupType, sizeof(stFrame.Data.stCmd03.ucGroupType), Addr_WorkState_GDA, newNode.stCanMap);
        InsertCanInfoMapLine((char *)&stFrame.Data.stCmd03.ucGroupStrategy, sizeof(stFrame.Data.stCmd03.ucGroupStrategy), Addr_GroupStrategy_GDA, newNode.stCanMap);
        newNode.enType = AddrType_GeneralDynamicArgQueryResult;
        ToCenterList.append(newNode);
        break;
    }
    default:
        break;
    }
}

//解析CCU参数设置 0xB5(CAN)
void cDCCanProtocol::ParseCCUQueryArgResult(unsigned char * pData, QList<CanMapNode>  &ToCenterList)
{
    CanMapNode newNode;
    FrameCCUParamSet_CCU stFrame;
    unsigned char ucTemp = 0;
    float ftemp = 0;
    memcpy((unsigned char *)&stFrame, pData, sizeof(stFrame));

    InsertCanInfoMapLine((char *)&ucTemp, sizeof(ucTemp), Addr_ArgNo, newNode.stCanMap);
    InsertCanInfoMapLine((char *)&stFrame, sizeof(stFrame), Addr_ArgData, newNode.stCanMap);

    //设置CCU的ID
    ucTemp = stFrame.ucCCUID;
    InsertCanInfoMapLine((char *)&ucTemp, sizeof(ucTemp), Addr_CCUidSet_CCUArg, newNode.stCanMap);
    //设置直流柜终端的起始地址
    ucTemp = stFrame.ucTermStartID;
    InsertCanInfoMapLine((char *)&ucTemp, sizeof(ucTemp), Addr_TermStartID_CCUArg, newNode.stCanMap);
    //直流柜最大输出功率
    ftemp = stFrame.usCabMaxPower*0.1;
    InsertCanInfoMapLine((char *)&ftemp, sizeof(ftemp), Addr_CabMaxPower_CCUArg, newNode.stCanMap);

    newNode.enType = AddrType_CCUQueryResult;
    ToCenterList.append(newNode);

}

//解析长帧
void cDCCanProtocol::ParseLongFrame(FrameLongPackage * pLongPackage)
{
    QList <CanMapNode> ToCenterList;
    QByteArray canAddrArray;
    canAddrArray.append(pLongPackage->ucTermID);
    switch(pLongPackage->ucPGN[1])
    {
    case PF_BMS_HandShake3_DC://解析BMS信息----握手阶段3
        ParseFrameBMSHandShake3(pLongPackage, ToCenterList);
        break;
    case PF_BMS_ParamSet1_DC://解析BMS信息----BMS参数配置阶段1
        ParseFrameBMSParamSet1(pLongPackage, ToCenterList);
        break;
    case PF_SpecificInfo://解析设备模块信息
        ParseSpecificInfo(pLongPackage, ToCenterList);
        break;
    case PF_VIN_DC://解析VIN
        ParseFrameVIN(pLongPackage, ToCenterList);
        break;
    case PF_AlarmPointRecord_CCU:   //解析故障点记录
        ParseAlarmPointRecord(pLongPackage, ToCenterList);
        break;
    case PF_FaultRecord_CCU:    //解析故障录波数据
        ParseFaultRecord(pLongPackage, ToCenterList);
        break;
    case PF_ActiveProtectionSet_DC: //解析主动防护设置
        ParseTermActiveArg(pLongPackage, ToCenterList);
        break;
    case PF_FlexibleChargeSet_DC:   //解析柔性充电设置
        ParseTermFlexArg(pLongPackage, ToCenterList);
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


//多枪分组信息下发
void cDCCanProtocol::ParseChargeGunGroupInfo(InfoMap CenterMap, unsigned char ucTermID)
{


    MakeFrameChargeGunGroup(CenterMap, ucTermID);

}

void cDCCanProtocol::ParseDoubleSys300kwSetting(InfoMap CenterMap, unsigned char ucTermID)
{
    MakeFrameChargeGunWorkMode(CenterMap, ucTermID);
}


//生成通用静态参数设置帧
void cDCCanProtocol::MakeFrameChargeGunGroup(InfoMap &CenterMap, unsigned char ucCanID)
{
    FrameChargerMannerInfo_DC stFrame;
    QByteArray ar;
    can_frame *pCanFrame = new(can_frame);
    int iGroup = 0;
    unsigned char temp[7];

    if(CenterMap.count() > 0)
    {
        ar = CenterMap[Addr_Group_ChargeGun];
        for(int i = 0; i < ar.length(); i = i + 10)
        {
            memset((unsigned char *)&stFrame, 0x00, sizeof(stFrame));
            memset(temp,0x00,7);
            pCanFrame = new(can_frame);
            iGroup=0;
            for(int y =0;y <7;y++)
            {
                if(ar.at(i+y))
                {
                    temp[y] = ar.at(i+y);
                    iGroup++;
                }
            }
            stFrame.chargeManner = iGroup;
            memcpy(&stFrame.canID_master,temp,7);
            memcpy((unsigned char *)pCanFrame->data, (unsigned char *)&stFrame, sizeof(stFrame));
            SetFrameHead(pCanFrame, ID_DefaultControlCenterCanID, PF_ChargeGunGroup_DC, ucCanID, PriorityDefault, DL_GeneralStaticParamSet_DC);
            pSendListMutex->lock();
            pTerminalSendList->append(pCanFrame);
            pSendListMutex->unlock();
        }
    }
}

//统计CCU数量
void cDCCanProtocol::ParseCCUNum(unsigned char	ucSa)
{
    if(ucSa <231 && ucSa > 240)
        return;

    for(int i = 0;i < 10;i++)
    {
        if(ucCCUNum[i] < 231 && ucCCUNum[i] > 240)
        {
            ucCCUNum[i] = 0;
            continue;
        }
        else if(ucSa == ucCCUNum[i])
            return;
        else
        {
            if(ucCCUNum[i] >= 231 && ucCCUNum[i] <= 240)
                continue;

            ucCCUNum[i] = ucSa;
            return;
        }
    }
}

//生成直流双枪充电工作模式下发信息
void cDCCanProtocol::MakeFrameChargeGunWorkMode(InfoMap &CenterMap, unsigned char ucCanID)
{
    int workmode = 2;
    unsigned char ucCanIDMaster;
    unsigned char ucCanIDSlave1;

    if(CenterMap.contains(Addr_ChargeGun_Master))
        ucCanIDMaster = CenterMap[Addr_ChargeGun_Master].at(0);
    if(CenterMap.contains(Addr_ChargeGun_Slave1))
        ucCanIDSlave1 = CenterMap[Addr_ChargeGun_Slave1].at(0);
    if(CenterMap.contains(Addr_300kw_WorkMode)){
       workmode =  CenterMap[Addr_300kw_WorkMode].at(0);
    }

    FrameWorkMode_DC stFrame;
    can_frame *pCanFrame = new(can_frame);
    can_frame *pCanFrame2 = new(can_frame);

    stFrame.cCanID = ucCanIDSlave1;
    stFrame.cWorkMode = workmode;
    memcpy((unsigned char *)pCanFrame->data, (unsigned char *)&stFrame, sizeof(stFrame));
    SetFrameHead(pCanFrame, ID_DefaultControlCenterCanID, PF_TwoGunSet_Old_DC, ucCanIDMaster, PriorityDefault, DL_300kwWorkMode_DC);

    pSendListMutex->lock();
    pTerminalSendList->append(pCanFrame);
    pSendListMutex->unlock();

    stFrame.cCanID = ucCanIDMaster;
    memcpy((unsigned char *)pCanFrame2->data, (unsigned char *)&stFrame, sizeof(stFrame));
    SetFrameHead(pCanFrame2, ID_DefaultControlCenterCanID, PF_TwoGunSet_Old_DC, ucCanIDSlave1, PriorityDefault, DL_300kwWorkMode_DC);

    pSendListMutex->lock();
    pTerminalSendList->append(pCanFrame2);
    pSendListMutex->unlock();
}
