#include "SerialScreenProtocol.h"
#define RESENDSCREEN 200000  //200ms重发切换画面


cSerialScreenProtocol::cSerialScreenProtocol(DevCache *pDevCacheIn, DBOperate * pDBOperateIn, ParamSet *pSetting, Log * pLogIn)
{
    pDevCache = pDevCacheIn;
    pDBOperate = pDBOperateIn;
	pParamSet = pSetting;
    pLog = pLogIn;
    uiPageCount = 0;
	_strLogName = "screen";
}

//转换  (缓存)终端状态 -> (屏幕显示)终端状态
unsigned char cSerialScreenProtocol::CheckScreenTermState(unsigned char ucTermState, bool &CartoonFlag,int multitype)
{
    switch(ucTermState)
    {
    case CHARGE_STATUS_STARTING://启动中
        CartoonFlag = FALSE;
        if(multitype==2)//弓类型
            return Term_Bow_Starting_Screen;
        else
            return Term_Starting_Screen;
        break;
    case CHARGE_STATUS_GUN_STANDBY://待机-枪已连接
        CartoonFlag = FALSE;
        if(multitype==2)//弓类型
            return Term_Bow_Linking_Screen;
        else
            return Term_Linking_Screen;
        break;
    case CHARGE_STATUS_LIMIT://充电-限制
        CartoonFlag = FALSE;
        return Term_Limiting_Screen;
        break;
    case CHARGE_STATUS_PAUSH://充电-暂停
    case CHARGE_STATUS_CARPAUSH://充电-暂停
    case CHARGE_STATUS_DEVPAUSH://充电-暂停
        CartoonFlag = FALSE;
        return Term_Pausing_Screen;
        break;
    case CHARGE_STATUS_CHARGING://充电-充电中(有动画)

        if(multitype==2)//弓类型
        {
            CartoonFlag = FALSE;
            return Term_Bow_Charging_Screen;
        }
        else
        {
            CartoonFlag = TRUE;
            return Term_Charging_Screen;
        }
        break;
    case CHARGE_STATUS_SWITCH://待机-切换中
        CartoonFlag = FALSE;
        return Term_Swithing_Screen;
        break;
    case CHARGE_STATUS_FREE://待机-空闲(有动画)

        if(multitype==2)//弓类型
        {
            CartoonFlag = FALSE;
            return Term_Bow_Standby_Screen;
        }
        else
        {
            CartoonFlag = TRUE;
            return Term_Standby_Screen;
        }
        break;
    case CHARGE_STATUS_DISCONNECT://离线-未通信
        CartoonFlag = FALSE;
        if(multitype==2)//弓类型
            return Term_Bow_Offline_Screen;
        else
            return Term_Offline_Screen;
        break;
    case CHARGE_STATUS_FINISH://待机-已完成
        CartoonFlag = FALSE;
        if(multitype==2)//弓类型
            return Term_Bow_Finish_Screen;
        else
            return Term_Finish_Screen;
        break;
    case CHARGE_STATUS_FAULT://故障(有动画)

        if(multitype==2)//弓类型
        {
            CartoonFlag = FALSE;
            return Term_Bow_Fault_Screen;
        }
        else
        {
            CartoonFlag = TRUE;
            return Term_Fault_Screen;
        }
        break;
    case CHARGE_STATUS_DISCHARGING://放电
        CartoonFlag = TRUE;
        return Term_Discharge_Screen;
        break;
    case CHARGE_STATUS_WAITING: //等待中
        CartoonFlag = FALSE;
        return Term_Waiting_Screen;
        break;
    case CHARGE_STATUS_FULL://待机-车已充满

        if(multitype==2)//弓类型
        {
            CartoonFlag = FALSE;
            return Term_Bow_Full_Screen;
        }
        else
        {
            CartoonFlag = TRUE;
            return Term_Full_Screen;
        }
        break;
    case CHARGE_STATUS_QUEUE1://充电-排队1
        CartoonFlag = TRUE;
        return Term_Queue1_Screen;
        break;
    case CHARGE_STATUS_QUEUE2://充电-排队2
        CartoonFlag = TRUE;
        return Term_Queue2_Screen;
        break;
    case CHARGE_STATUS_QUEUE3://充电-排队3
        CartoonFlag = TRUE;
        return Term_Queue3_Screen;
        break;
    case CHARGE_STATUS_QUEUE4://充电-排队4
        CartoonFlag = TRUE;
        return Term_Queue4_Screen;
        break;
    case CHARGE_STATUS_QUEUE5://充电-排队5
        CartoonFlag = TRUE;
        return Term_Queue5_Screen;
        break;
    case CHARGE_STATUS_QUEUE6://充电-排队6
        CartoonFlag = TRUE;
        return Term_Queue6_Screen;
        break;
    case CHARGE_STATUS_QUEUE7://充电-排队7
        CartoonFlag = TRUE;
        return Term_Queue7_Screen;
        break;
    case CHARGE_STATUS_SLAVEGUN://副枪
        CartoonFlag = FALSE;
        return Term_Slave_Screen;
        break;
    case CHARGE_STATUS_COUPLE_ERR://配对错误
        CartoonFlag = FALSE;
        return Term_Couple_Error;
        break;
    case 0xff:    //取消图标显示
        CartoonFlag = FALSE;
        return 0xff;
        break;
    default://不属于以上状态
        CartoonFlag = FALSE;
        return Term_Offline_Screen;
        break;
    };
}

//发送主界面数据
void cSerialScreenProtocol::SendMainPageData(TermNameMap  &NameMap, stCSCUSysConfig &cscuConfig, char * pCSCUVer,TermNameMap  &NameMapShow)
{
    unsigned char ucNetState = 0;
    unsigned char ucCSCUWorkState = 0;
    QVariant Var, Param;
    if(!pDevCache->QueryRealStatusMeter(Var, CACHE_STATUS, Param))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! SendMainPageData  QueryRealStatusMeter Error!");
    }
    ucNetState = Var.value<RealStatusData>().connectStatus;
    ucCSCUWorkState = Var.value<RealStatusData>().emergencyStatus;
    //发送 各终端名称,ID,状态 , 故障信息, 1s一次
    SendFrameTermInfo(NameMap,NameMapShow);
    //发送网络图标
//    SendFrameNetState(ucNetState);
    if(uiPageCount%2 == 0)//2s一次
    {
        //发送 网络状态
        SendFrameNetState(ucNetState);
        SendFrameCSCUWorkState(ucCSCUWorkState);//发送工作模式状态
    }
    if(uiPageCount%60 == 0)//60s一次
    {
        //发送场站名称
        SendFrameStationName(cscuConfig.stationName);
        SendFrameCSCUProVer(pCSCUVer);
    }
}

//系统参数设置界面----发送设置信息
void cSerialScreenProtocol::SendSysParamData(ParamSetPage_Screen &stParamSetPage)
{
    SendFrameTermNum(stParamSetPage);
    SendFrameIPAndPort(stParamSetPage);
    SendFrameStationAddr(stParamSetPage);
    SendFrameDomainName(stParamSetPage);
    SendFrameGateWay(stParamSetPage);
}

//终端-详细信息----发送相关数据
void cSerialScreenProtocol::SendTermPageData(unsigned char ucCanID, unsigned char ucType, stThreePhaseTypeConfig &ThreePhaseTypeConfig)
{
    TerminalStatus stTerm;
    if(! pDevCache->QueryTerminalStatus(ucCanID,stTerm))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! Screen pDevCache Query  Error!");
    }
    //发送终端信息
    if(ucType == 1)
    {
        SendTermDetailData_Normal(stTerm);
    }
    else
    {
        SendTermDetailData_Normal(stTerm);
    }
}

//终端-充电中信息
void cSerialScreenProtocol::SendTermPageChargingData(unsigned char ucCanID, unsigned char ucType, stThreePhaseTypeConfig &ThreePhaseTypeConfig)
{
    TerminalStatus stTerm;
    if(! pDevCache->QueryTerminalStatus(ucCanID,stTerm))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! Screen pDevCache Query  Error!");
    }
    //发送终端信息
    if(ucType == 1)
    {
        SendTermDetailData_Card(stTerm, ThreePhaseTypeConfig);//不再区分普通版
    }
    else
    {
        SendTermDetailData_Card(stTerm, ThreePhaseTypeConfig);
    }
}

//特殊功能设置界面----发送设置信息
void cSerialScreenProtocol::SendSpecFuncSet(SpecFuncSet_Screen &stSpecFunc)
{
    FrameSpecFuncSet_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FrameSpecFuncSet_Screen)];
    pScreen = (FrameSpecFuncSet_Screen*)pData;
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameSpecFuncSet_Screen) - 3);
    //指令类型: 写控制寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //切换界面 寄存器地址
    pScreen->usAddr = htons(Reg_SpecFuncSet_Screen);
    //数据赋值
    pScreen->usCoupleGun = htons(stSpecFunc.usCoupleGun);
    pScreen->usVINOffline = htons(stSpecFunc.usVINOffline);
    pScreen->usLocalStop = htons(stSpecFunc.usLocalStop);
    pScreen->usCardType = htons(stSpecFunc.usCardType);
    pScreen->usVINAuto = htons(stSpecFunc.usVINAuto);
    pScreen->usCardAuto = htons(stSpecFunc.usCardAuto);
    pScreen->usVINType = htons(stSpecFunc.usVINType);
    pScreen->usBoardType = htons(stSpecFunc.usBoardType);
    pScreen->usEnergyFilter = htons(stSpecFunc.usEnergyFilter);
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
}

//进线侧数据显示界面----发送进线侧信息
void cSerialScreenProtocol::SendInLineInfo(QByteArray ammeterAddr)
{
    stAmmeterData ammeterData;
    QVariant Var;
    QVariant Param;
    unsigned int uiTempEnergy = 0;
    Param.setValue(ammeterAddr);
    if(!pDevCache->QueryRealStatusMeter(Var, CACHE_INLINE_AMMETER, Param))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! SendMainPageData  QueryRealStatusMeter Error!");
    }
    ammeterData = Var.value<stAmmeterData>();

    FrameInLineInfo_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FrameInLineInfo_Screen)];
    pScreen = (FrameInLineInfo_Screen*)pData;

    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameInLineInfo_Screen) - 3);
    //寄存器地址
    pScreen->usAddr = htons(Reg_InLineInfo_Screen);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //数据赋值
    pScreen->A_voltage = htons((unsigned short)(ammeterData.Vol_A));
    pScreen->A_current = htons((unsigned short)(ammeterData.Cur_A*100));
    pScreen->B_voltage = htons((unsigned short)(ammeterData.Vol_B));
    pScreen->B_current = htons((unsigned short)(ammeterData.Cur_B*100));
    pScreen->C_voltage = htons((unsigned short)(ammeterData.Vol_C));
    pScreen->C_current = htons((unsigned short)(ammeterData.Cur_C*100));
    pScreen->active_power = htons((unsigned short)(ammeterData.TotalPower*100));
    pScreen->reactive_power = htons((unsigned short)(ammeterData.TotalRePower*100));
    pScreen->power_factor = htons((unsigned short)(ammeterData.PowerFactor*1000));
    pScreen->neutralLine_current = htons((unsigned short)(ammeterData.Cur_0 *10));

    uiTempEnergy = (unsigned int)(ammeterData.ActiveAbsorbEnergy *100);
    pScreen->ActiveAbsorbEnergy_h = htons((unsigned short)(uiTempEnergy/0x10000));
    pScreen->ActiveAbsorbEnergy_l = htons((unsigned short)(uiTempEnergy%0x10000));

    uiTempEnergy = (unsigned int)(ammeterData.ActiveLiberateEnergy *100);
    pScreen->ActiveLiberateEnergy_h = htons((unsigned short)(uiTempEnergy/0x10000));
    pScreen->ActiveLiberateEnergy_l = htons((unsigned short)(uiTempEnergy%0x10000));

    uiTempEnergy = (unsigned int)(ammeterData.ReactiveSensibilityEnergy *100);
    pScreen->ReactiveSensibilityEnergy_h = htons((unsigned short)(uiTempEnergy/0x10000));
    pScreen->ReactiveSensibilityEnergy_l = htons((unsigned short)(uiTempEnergy%0x10000));

    uiTempEnergy = (unsigned int)(ammeterData.ReactiveCapacityEnergy *100);
    pScreen->ReactiveCapacityEnergy_h = htons(uiTempEnergy/0x10000);
    pScreen->ReactiveCapacityEnergy_l = htons(uiTempEnergy%0x10000);

    //发送帧
    SendFrame(pData, sizeof(* pScreen));
    //页面计数器清零
    uiPageCount = 0;
}

//进线侧信息界面----发送电表地址
//usNum: 第几块电表
//ammeterConfig: 电表配置信息
void cSerialScreenProtocol::SendAmmeterAddr(QList<stAmmeterConfig> &ammeterConfig, unsigned short usNum)
{
    FrameAmmeterAddr_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FrameAmmeterAddr_Screen)];
    pScreen = (FrameAmmeterAddr_Screen*)pData;
    QString strAddr;
    QByteArray arryAddr;
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameAmmeterAddr_Screen) - 3);
    //指令类型: 写控制寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //切换界面 寄存器地址
    pScreen->usAddr = htons(Reg_AmmeterAddr_Screen);
    //电表总个数
    pScreen->usTotalNum = htons((short)ammeterConfig.count());
    //第几个电表
    pScreen->usCountNum = htons(usNum);
    //电表地址赋值
    strAddr = ConvertHex2Qstr((unsigned char *)ammeterConfig.at(usNum - 1).addr, 6);

    for(unsigned char i = 0 ; i < strAddr.length(); i++)
    {
        if(strAddr.at(i) == ' ')
        {
            strAddr.remove(i, 1);
            i = 0;
        }
    }
    memset(pScreen->chAmmeterAddr, 0x00, sizeof(pScreen->chAmmeterAddr));
    memcpy(pScreen->chAmmeterAddr, strAddr.toAscii().data(), strAddr.toAscii().length());

    //发送帧
    SendFrame(pData, sizeof(* pScreen));
    //页面计数器清零
    uiPageCount = 0;
}

//子站环境信息界面----发送环境信息
//IOConfig , 1异常, 0正常;  显示屏: 0-无；1-正常；2-异常
void cSerialScreenProtocol::SendEnvInfo()
{
    unsigned short alarm1;          //告警数据1
    unsigned short alarm2;          //告警数据2
    unsigned short usTemp = 0;  //单独一位告警器状态
    unsigned short usState = 0;     //屏幕显示状态代码
//    unsigned short usCmp = 0;
    FrameEnvInfo_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FrameEnvInfo_Screen)];
    memset(pData, 0x00, sizeof( * pScreen));
    pScreen = (FrameEnvInfo_Screen*)pData;
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameEnvInfo_Screen) - 3);
    //寄存器地址
    pScreen->usAddr = htons(Reg_EnvInfo_Screen);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //数据赋值
    QVariant Var, Param;
    if(!pDevCache->QueryRealStatusMeter(Var, CACHE_STATUS, Param))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! SendMainPageData  QueryRealStatusMeter Error!");
    }
    alarm1 = Var.value<RealStatusData>().alarm1;
    alarm2 = Var.value<RealStatusData>().alarm2;
    pScreen->sHum = htons(Var.value<RealStatusData>().humidity);
    pScreen->sTemp = htons(Var.value<RealStatusData>().temperature);
    //告警状态到屏幕显示代码转换----报警器1
    for(unsigned char uc = 0; uc < 8; uc++)
    {
        usTemp = (alarm1 >> uc)&(0x0001);
        (usTemp == 0)?(usState = 1):(usState = 2);
        pScreen->usAlarm[uc] = htons(usState);
    }
    //告警状态到屏幕显示代码转换----报警器2
    for(unsigned char uc = 0; uc < 2; uc++)
    {
        usTemp = (alarm2 >> uc)&(0x0001);
        (usTemp == 0)?(usState = 1):(usState = 2);
        pScreen->usAlarm[uc + 8] = htons(usState);
    }
//    for(int i = 0; i < sizeof(* pScreen); i++)
//    {
//    }
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
    //页面计数器清零
    uiPageCount = 0;
}

//子站环境信息界面----发送IO配置信息
void cSerialScreenProtocol::SendAlarmSet(stIOConfig & IOConfig)
{
    FrameAlarmSet_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FrameAlarmSet_Screen)];
    pScreen = (FrameAlarmSet_Screen*)pData;
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameAlarmSet_Screen) - 3);
    //寄存器地址
    pScreen->usAddr = htons(Reg_AlarmSetInfo_Screen);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //数据赋值
    char tmpOpenColse[8];
    for(unsigned char i = 0; i < 8; i++)
    {
        tmpOpenColse[i]=IOConfig.inOpenColse[i];    //add by weiwb
        if(tmpOpenColse[i]==0)
           tmpOpenColse[i]=2;
        pScreen->usAlarm[i] = htons((unsigned short)(tmpOpenColse[i]));
    }
    pScreen->usAlarm[8] = 0x00;
    pScreen->usAlarm[9] = 0x00;
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
    //页面计数器清零
    uiPageCount = 0;
}

//直流特殊功能信息界面----发送现在直流信息
void cSerialScreenProtocol::SendDCSpec(unsigned char ucDCNum)
{
    for(unsigned char i = 0 ; i < ucDCNum; i++)
    {
        SendDCSpecLine(i + ID_MinDCCanID);
    }
}

//直流特殊功能信息界面----发送直流特殊功能设置一条信息
void cSerialScreenProtocol::SendDCSpecLine(unsigned char ucCanID)
{
    unsigned short  usTermWorkState  = 0;
    unsigned short  usGroupStrategy = 0;
    unsigned short  usAuxType = 0;
    unsigned short usAddr = 0;
    db_result_st dbst;
    QString todo;
    FrameDCSpec_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FrameDCSpec_Screen)];
    pScreen = (FrameDCSpec_Screen*)pData;
    //数据库操作----查询静态参数
    todo = "SELECT auxpower_type  FROM dcstatic_param_table WHERE canaddr = "+ QString::number(ucCanID, 10);
    if(0 != pDBOperate->DBSqlQuery(todo.toAscii().data(), &dbst, DB_PARAM))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! ParseTermStaticArg Query Error");
    }
    if(dbst.row != 0)
    {
        usAuxType = (unsigned short) atoi(dbst.result[0]);
    }
    pDBOperate->DBQueryFree(&dbst);
    //数据库操作----查询动态参数
    todo = "SELECT work_status, strategy FROM dcdynamic_param_table WHERE canaddr = "+ QString::number(ucCanID, 10);
    if(0 != pDBOperate->DBSqlQuery(todo.toAscii().data(), &dbst, DB_PARAM))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! ParseTermDynamicArg Query Error");
    }
    if(dbst.row != 0)
    {
        usTermWorkState = (unsigned short) atoi(dbst.result[0]);
        usGroupStrategy = (unsigned short) atoi(dbst.result[1]);
        //转换终端工作状态到串口屏协议
        switch(usTermWorkState)
        {
        case 6:
            usTermWorkState = 1;
            break;
        case 7:
            usTermWorkState = 2;
            break;
        default:
            break;
        }
    }
    pDBOperate->DBQueryFree(&dbst);

    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameDCSpec_Screen) - 3);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //寄存器地址
    usAddr = Reg_DCSpecSet_Screen + (ucCanID - ID_MinDCCanID) * Reg_DCSpecInterval_Screen;
    pScreen->usAddr = htons(usAddr);
    //终端工作状态
    pScreen->usTermWorkState = htons(usTermWorkState);
    //终端群充策略
    pScreen->usGroupStrategy = htons(usGroupStrategy);
    //终端辅助电源类型
    pScreen->usAuxType = htons(usAuxType);

    //发送帧
    SendFrame(pData, sizeof(* pScreen));

}

//发送 负荷调度使能设置----负荷调度界面
void cSerialScreenProtocol::SendLoadDispatchEnable(bool bPeakFlag, bool bLoadFlag)
{
    FrameLoadDispatch_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FrameLoadDispatch_Screen)];
    pScreen = (FrameLoadDispatch_Screen*)pData;
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameLoadDispatch_Screen) - 3);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //寄存器地址
    pScreen->usAddr = htons(Reg_PeakEnable_Screen);
    //错峰使能开关赋值
    pScreen->usPeakFlag = htons((unsigned short)(bPeakFlag));
    //负荷使能开关赋值
    pScreen->usLoadFlag = htons((unsigned short)(bLoadFlag));
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
}

//发送 错峰充电状态提示图标
void cSerialScreenProtocol::SendPeakAutoIcon(bool bPeakFlag)
{
    FramePeakIcon_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FramePeakIcon_Screen)];
    pScreen = (FramePeakIcon_Screen*)pData;
    memset(pData, 0x00, sizeof(*pScreen));
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FramePeakIcon_Screen) - 3);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //寄存器地址
    pScreen->usAddr = htons(Reg_SmartChargeIcon_Screen);
    if( bPeakFlag == 0)
    {
        pScreen->usPeakFlag = htons((unsigned short)(0001));
    }
    else
    {
        pScreen->usPeakFlag = htons((unsigned short)(0002));
    }
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
}

//发送 错峰充电页面数据----负荷调度界面
void cSerialScreenProtocol::SendPeakSetPage(stAllTPFVConfig &AllTPFVConfig, unsigned char ucPageNum)
{
    unsigned char ucCount = 0;
    FramePeakShowPage_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FramePeakShowPage_Screen)];
    pScreen = (FramePeakShowPage_Screen*)pData;
    memset(pData, 0x00, sizeof(*pScreen));
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FramePeakShowPage_Screen) - 3);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //第一页
    if(ucPageNum == 1)
    {
        //寄存器地址
        pScreen->usAddr = htons(Reg_PeakPageOne_Screen);
        ucCount = ( (10 <= AllTPFVConfig.tpfvConfig.count()) ? 10 : AllTPFVConfig.tpfvConfig.count() );
        for(unsigned char i = 0; i < (ucCount); i++)
        {
            pScreen->stRecord[i].usType = htons((unsigned short) AllTPFVConfig.tpfvConfig.at(i).time_seg);
            pScreen->stRecord[i].usStartH = htons((unsigned short) AllTPFVConfig.tpfvConfig.at(i).start_hour);
            pScreen->stRecord[i].usStartM = htons((unsigned short) AllTPFVConfig.tpfvConfig.at(i).start_minute);
            pScreen->stRecord[i].usStopH = htons((unsigned short) AllTPFVConfig.tpfvConfig.at(i).stop_hour);
            pScreen->stRecord[i].usStopM = htons((unsigned short) AllTPFVConfig.tpfvConfig.at(i).stop_minute);
            pScreen->stRecord[i].usSOC = htons((unsigned short) AllTPFVConfig.tpfvConfig.at(i).limit_soc);
            pScreen->stRecord[i].usCurrent = htons((unsigned short) AllTPFVConfig.tpfvConfig.at(i).limit_current);
        }
    }
    //第二页
    else if(ucPageNum == 2)
    {
        //寄存器地址
        pScreen->usAddr = htons(Reg_PeakPageTwo_Screen);
        ucCount = ( (10 < AllTPFVConfig.tpfvConfig.count()) ?  (AllTPFVConfig.tpfvConfig.count() - 10) : 0);
        for(unsigned char i = 0; i < (ucCount); i++)
        {
            pScreen->stRecord[i].usType = htons((unsigned short) AllTPFVConfig.tpfvConfig.at(i+10).time_seg);
            pScreen->stRecord[i].usStartH = htons((unsigned short) AllTPFVConfig.tpfvConfig.at(i+10).start_hour);
            pScreen->stRecord[i].usStartM = htons((unsigned short) AllTPFVConfig.tpfvConfig.at(i+10).start_minute);
            pScreen->stRecord[i].usStopH = htons((unsigned short) AllTPFVConfig.tpfvConfig.at(i+10).stop_hour);
            pScreen->stRecord[i].usStopM = htons((unsigned short) AllTPFVConfig.tpfvConfig.at(i+10).stop_minute);
            pScreen->stRecord[i].usSOC = htons((unsigned short) AllTPFVConfig.tpfvConfig.at(i+10).limit_soc);
            pScreen->stRecord[i].usCurrent = htons((unsigned short) AllTPFVConfig.tpfvConfig.at(i+10).limit_current);
        }
    }
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
}

//发送 错峰充电详细设置数据----负荷调度界面
//峰:1   谷:2   平:3   尖:4
void cSerialScreenProtocol::SendPeakSetDetail(stAllTPFVConfig &AllTPFVConfig, unsigned char ucType)
{
    FramePeakShowDetail_Screen * pScreen;
    unsigned char ucOffset = 0;
    unsigned char * pData = new unsigned char[sizeof(FramePeakShowDetail_Screen)];
    pScreen = (FramePeakShowDetail_Screen*)pData;
    //计算偏移量
    switch(ucType)
    {
    case 1:
        ucOffset = 0;
        break;
    case 2:
        ucOffset = 1;
        break;
    case 3:
        ucOffset = 2;
        break;
    case 4:
        ucOffset = 3;
        break;
    default:
        ucOffset = 0;
        break;
    }
    memset(pData, 0x00, sizeof(* pScreen));
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FramePeakShowDetail_Screen) - 3);
    //寄存器地址
    pScreen->usAddr = htons(Reg_PeakDetailBegin_Screen + Reg_PeakDetailInterval_Screen * (ucOffset));
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;

    //数据赋值
    for(unsigned char i = 0, j = 0; i < AllTPFVConfig.tpfvConfig.count(); i++)
    {
        if(AllTPFVConfig.tpfvConfig.at(i).time_seg == ucType)
        {
            pScreen->stRecord[j].usType = htons((unsigned short) AllTPFVConfig.tpfvConfig.at(i).time_seg);
            pScreen->stRecord[j].usStartH = htons((unsigned short) AllTPFVConfig.tpfvConfig.at(i).start_hour);
            pScreen->stRecord[j].usStartM = htons((unsigned short) AllTPFVConfig.tpfvConfig.at(i).start_minute);
            pScreen->stRecord[j].usStopH = htons((unsigned short) AllTPFVConfig.tpfvConfig.at(i).stop_hour);
            pScreen->stRecord[j].usStopM = htons((unsigned short) AllTPFVConfig.tpfvConfig.at(i).stop_minute);
            pScreen->stRecord[j].usSOC = htons((unsigned short) AllTPFVConfig.tpfvConfig.at(i).limit_soc);
            pScreen->stRecord[j].usCurrent = htons((unsigned short) AllTPFVConfig.tpfvConfig.at(i).limit_current);
            j++;
        }
    }
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
}

//发送负荷约束设置数据----负荷调度界面
void cSerialScreenProtocol::SendLoadLimit(stPowerLimitConfig &PowerLimitConfig)
{
    FrameLoadLimit_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FrameLoadLimit_Screen)];
    pScreen = (FrameLoadLimit_Screen*)pData;
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameLoadLimit_Screen) - 3);
    //寄存器地址
    pScreen->usAddr = htons(Reg_LoadLimit_Screen);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //数据赋值
    pScreen->usCCUNum = htons((unsigned short)(PowerLimitConfig.sCCUcount));
    pScreen->usTotalPower = htons((unsigned short)(PowerLimitConfig.STATION_LIMT_POWER));
    pScreen->usSecurePower = htons((unsigned short)(PowerLimitConfig.SAFE_CHARGE_POWER));
    pScreen->usLimitPower = htons((unsigned short)(PowerLimitConfig.sSUMPower_Manual));
    pScreen->usDynamicEnable = htons((unsigned short)(PowerLimitConfig.sSUMPower_Ammeter_Enable));
    pScreen->usLocalEnable = htons((unsigned short)(PowerLimitConfig.sSUMPower_Manual_Enable));
    pScreen->usRemoteEnable = htons((unsigned short)(PowerLimitConfig.sSUMPower_Server_Enable));

    //发送帧
    SendFrame(pData, sizeof(* pScreen));
}

//发送 终端故障列表界面
void cSerialScreenProtocol::SendFaultInfoList(TermNameMap  NameMap)
{
    //串口屏地址寄存器数组偏移
    unsigned char ucAddrOffset = 0;
    int i = 0;
    //终端状态缓存
    TerminalStatus stTemp;
    TermNameMap::iterator it;
    //向串口屏发送发送故障信息
    for(it = NameMap.begin(); it!= NameMap.end(); ++it, i++)
    {
        if(pDevCache->QueryTerminalStatus(it.key(), stTemp) == FALSE)
        {
            pLog->getLogPoint(_strLogName)->info("ERROR! SendFaultInfoList QueryTerminalStatus FALSE!");
            break;
        }
        //判断故障, 发送故障信息, 最多发送10条
        if( (stTemp.cStatus == CHARGE_STATUS_FAULT) || (stTemp.stFrameRemoteSingle.status_fault != 0) )
        {
            //发送 终端故障列表界面----一条记录
            SendFaultInfoLine(ucAddrOffset, stTemp);
            ucAddrOffset++;
            if(ucAddrOffset >= 10)
            {
                pLog->getLogPoint(_strLogName)->info("WARN! Not Enough place to show fault info!");
                break;
            }
        }
    }
}

//发送 切换界面指令, usPageNum : 目的页面编号
void cSerialScreenProtocol::SendSwitchPage(unsigned short usPageNum)
{
    FrameSwitchPage_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FrameSwitchPage_Screen)];
    pScreen = (FrameSwitchPage_Screen*)pData;
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameSwitchPage_Screen) - 3);
    //切换界面 寄存器地址
    pScreen->ucAddr = Reg_Spec_SwitchPage_Screen;
    //指令类型: 写控制寄存器
    pScreen->ucCmdType = Type_WriteCtrl_Screen;
    //切换界面编号
    pScreen->usPageNum = htons(usPageNum);

    //发送帧
    SendFrame(pData, sizeof(* pScreen));

    //200ms重发
    usleep(RESENDSCREEN);
    FrameSwitchPage_Screen * pResendScreen;
    unsigned char * pResendData = new unsigned char[sizeof(FrameSwitchPage_Screen)];
    pResendScreen = (FrameSwitchPage_Screen*)pResendData;
    //生成帧头
    MakeFrameHead(pResendScreen->strHead, sizeof(FrameSwitchPage_Screen) - 3);
    //切换界面 寄存器地址
    pResendScreen->ucAddr = Reg_Spec_SwitchPage_Screen;
    //指令类型: 写控制寄存器
    pResendScreen->ucCmdType = Type_WriteCtrl_Screen;
    //切换界面编号
    pResendScreen->usPageNum = htons(usPageNum);

    //发送帧
    SendFrame(pResendData, sizeof(* pResendScreen));
    //页面计数器清零
    uiPageCount = 0;
}

//发送 图标显示指令, usIconNum: 图标编号
void cSerialScreenProtocol::SendShowIcon(unsigned short usIconNum)
{
    FrameShowIcon_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FrameShowIcon_Screen)];
    pScreen = (FrameShowIcon_Screen*)pData;
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameShowIcon_Screen) - 3);
    //切换界面 寄存器地址
    pScreen->usAddr = htons(Reg_IconPage_Screen);
    //指令类型: 写控制寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //图标编号
    pScreen->usIconNum = htons(usIconNum);
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
    //页面计数器清零
    uiPageCount = 0;
}

//发送 VIN申请充电结果, usIconNum: 图标编号
//usIconNum, 详见枚举enum VINResult_Screen
void cSerialScreenProtocol::SendVINResult(unsigned short usIconNum)
{
    FrameShowIcon_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FrameShowIcon_Screen)];
    pScreen = (FrameShowIcon_Screen*)pData;
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameShowIcon_Screen) - 3);
    //切换界面 寄存器地址
    pScreen->usAddr = htons(Reg_VINApplyCharge_Screen);
    //指令类型: 写控制寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //图标编号
    pScreen->usIconNum = htons(usIconNum);
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
    //页面计数器清零
    uiPageCount = 0;
}

//发送 页面倒计时
void cSerialScreenProtocol::SendPageCount(unsigned short usCountTime)
{
    FramePageCount_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FramePageCount_Screen)];
    pScreen = (FramePageCount_Screen*)pData;
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FramePageCount_Screen) - 3);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //页面倒计时 寄存器地址
    pScreen->usAddr = htons(Reg_PageCount_Screen);
    //倒计时时间
    pScreen->usCountTime = htons(usCountTime);
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
}

//发送 中止页面倒计时
void cSerialScreenProtocol::SendPageCountStop(unsigned short usPageNum)
{
    FramePageCountStop_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FramePageCountStop_Screen)];
    pScreen = (FramePageCountStop_Screen*)pData;
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FramePageCountStop_Screen) - 3);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteCtrl_Screen;
    //寄存器地址( = 切换页面寄存器地址)
    pScreen->ucAddr = Reg_Spec_SwitchPage_Screen;
    //页面编号
    pScreen->usPageNum = htons(usPageNum);
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
}

//终端详情界面----发送终端详细信息(充电中)
void cSerialScreenProtocol::SendTermDetailData_Normal(TerminalStatus &stTerm)
{
    FrameTermDetail_Normal_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FrameTermDetail_Normal_Screen)];
    pScreen = (FrameTermDetail_Normal_Screen*)pData;
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameTermDetail_Normal_Screen) - 3);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //终端详情 寄存器地址
    pScreen->usAddr = htons(Reg_TermDetail_Normal_Screen);

    pScreen->A_voltage = htons((short)(stTerm.stFrameRemoteMeSurement1.A_voltage/0.1));
    pScreen->B_voltage = htons((short)(stTerm.stFrameRemoteMeSurement1.B_voltage/0.1));
    pScreen->C_voltage = htons((short)(stTerm.stFrameRemoteMeSurement1.C_voltage/0.1));
    pScreen->A_current = htons((short)(stTerm.stFrameRemoteMeSurement1.A_current/0.1 * (-1)));
    pScreen->B_current = htons((short)(stTerm.stFrameRemoteMeSurement1.B_current/0.1 * (-1)));
    pScreen->C_current = htons((short)(stTerm.stFrameRemoteMeSurement1.C_current/0.1* (-1)));
    //有功功率,无功功率 功率因数
    pScreen->active_power = htons((short)(stTerm.stFrameRemoteMeSurement1.active_power/0.01));
    pScreen->reactive_power = htons((short)(stTerm.stFrameRemoteMeSurement1.reactive_power/0.01));
    pScreen->power_factor = htons((short)(stTerm.stFrameRemoteMeSurement1.power_factor/0.001 * (-1)));
    //零线电流
    pScreen->neutralLine_current = htons((short)(stTerm.stFrameRemoteMeSurement1.neutralLine_current/0.1));
    //直流侧电压,电流
    pScreen->voltage_of_dc = htons((short)(stTerm.stFrameRemoteMeSurement1.voltage_of_dc/0.1));
    pScreen->current_of_dc = htons((short)(stTerm.stFrameRemoteMeSurement1.current_of_dc/0.1 * (-1)));
    //总有功电能,总无功电能
    pScreen->active_electric_energy_h = htons((short)((int)(stTerm.stFrameRemoteMeSurement2.active_electric_energy)/65536));
    pScreen->active_electric_energy_l = htons((short)((int)(stTerm.stFrameRemoteMeSurement2.active_electric_energy + 0.5)%65536));
    pScreen->reactive_electric_energy_h = htons((short)((int)(stTerm.stFrameRemoteMeSurement2.reactive_electric_energy)/65536));
    pScreen->reactive_electric_energy_l = htons((short)((int)(stTerm.stFrameRemoteMeSurement2.reactive_electric_energy + 0.5)%65536));
    //电压不平衡率, 电流不平衡率
    pScreen->voltage_unbalance_rate = htons((short)(stTerm.stFrameRemoteMeSurement1.voltage_unbalance_rate/0.01));
    pScreen->current_unbalance_rate = htons((short)(stTerm.stFrameRemoteMeSurement1.current_unbalance_rate/0.01));
    //充电中止原因
    pScreen->stop_reason = htons((short)(stTerm.stFrameRemoteSingle.Stop_Result));
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
}

//终端详情界面----发送终端详细信息----刷卡版
void cSerialScreenProtocol::SendTermDetailData_Card(TerminalStatus &stTerm, stThreePhaseTypeConfig &ThreePhaseTypeConfig)
{
    int seconds = 0;
    unsigned short usH, usM, usS;
    //字符编码转换
    QTextCodec * pGBK = QTextCodec::codecForName("GBK");
    QString tempString;
    QByteArray tempArray;

    CHARGE_STEP temStep;
    QDateTime StartTime, NowTime;
    QTime tmpTime;

    FrameTermDetail_Card_Screen * pScreen;

    unsigned char * pData = new unsigned char[sizeof(FrameTermDetail_Card_Screen)];
    pScreen = (FrameTermDetail_Card_Screen*)pData;
    unsigned char ucPhaseType = 0;
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameTermDetail_Card_Screen) - 3);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //终端详情 寄存器地址
    pScreen->usAddr = htons(Reg_TermDetail_Card_Screen);

    //相关充电状态查询
    pDevCache->QueryChargeStep(stTerm.cCanAddr, temStep);
    //单相电流显示
    if((stTerm.cCanAddr >= ID_MinACSinCanID)&&(stTerm.cCanAddr <= ID_MaxACSinCanID))
    {
        //相别判断
        for(unsigned char i = 0; i < ThreePhaseTypeConfig.phaseTypeConfig.count(); i++)
        {
            if(ThreePhaseTypeConfig.phaseTypeConfig.at(i).canaddr == stTerm.cCanAddr)
            {
                ucPhaseType = (unsigned char)ThreePhaseTypeConfig.phaseTypeConfig.at(i).type;
                break;
            }
        }
        //电压电流赋值
        switch(ucPhaseType)
        {
        case 2://B相
            pScreen->sVoltage = htons((short)(stTerm.stFrameRemoteMeSurement1.B_voltage/0.1));
            pScreen->sCurrent = htons((short)(stTerm.stFrameRemoteMeSurement1.B_current/0.1 * (-1)));
            break;
        case 3://C相
            pScreen->sVoltage = htons((short)(stTerm.stFrameRemoteMeSurement1.C_voltage/0.1));
            pScreen->sCurrent = htons((short)(stTerm.stFrameRemoteMeSurement1.C_current/0.1 * (-1)));
            break;
        case 0: //未有相别设置
        case 1://A相
        default://默认, 取A相
            pScreen->sVoltage = htons((short)(stTerm.stFrameRemoteMeSurement1.A_voltage/0.1));
            pScreen->sCurrent = htons((short)(stTerm.stFrameRemoteMeSurement1.A_current/0.1 * (-1)));
            break;
        }
    }
    //三相电流显示
    else if((stTerm.cCanAddr >= ID_MinACThrCanID)&&(stTerm.cCanAddr <= ID_MaxACThrCanID))
    {
        pScreen->sVoltage = htons((short)(stTerm.stFrameRemoteMeSurement1.A_voltage/0.1));
        pScreen->sCurrent = htons((short)(stTerm.stFrameRemoteMeSurement1.A_current/0.1 * (-1)));
    }
    //直流电流显示
    else if((stTerm.cCanAddr >= ID_MinDCCanID)&&(stTerm.cCanAddr <= ID_MaxDCCanID))
    {
        pScreen->sVoltage = htons((short)(stTerm.stFrameRemoteMeSurement1.voltage_of_dc/0.1));
        pScreen->sCurrent = htons((short)(stTerm.stFrameRemoteMeSurement1.current_of_dc/0.1 * (-1)));
    }
    //其他, 电压电流置零
    else
    {
        pScreen->sVoltage = 0;
        pScreen->sCurrent = 0;
    }

    //开始时间
    StartTime = QDateTime::fromString(QString(temStep.sStartTime),"yyyy-MM-dd HH:mm:ss");
    tmpTime = StartTime.time();
    pScreen->sStartHour = htons((short)tmpTime.hour());
    pScreen->sStartMin = htons((short)tmpTime.minute());
    pScreen->sStartSec = htons((short)(tmpTime.second()));
    //充电时间
    NowTime = QDateTime::currentDateTime();
    seconds = StartTime.secsTo(NowTime);
    //时间错误,则充电时间显示0
    if(seconds < 0)
    {
        seconds = 0;
    }
    usH = seconds/3600;
    usM = seconds/60 -usH*60;
    usS = seconds%60;
    pScreen->sChargeHour = htons(usH);
    pScreen->sChargeMin = htons(usM);
    pScreen->sChargeSec = htons(usS);

    //功率, 电能
    pScreen->sActivePower = htons((short)(stTerm.stFrameRemoteMeSurement1.active_power/0.01));    
    pScreen->uiTotalEnergy = htonl((unsigned int)(stTerm.stFrameRemoteMeSurement2.active_electric_energy));//浮点之取小数点前两位

    //SOC
    if((stTerm.cCanAddr <= ID_MaxDCCanID) &&(stTerm.cCanAddr >= ID_MinDCCanID))
    {
        tempString = QString::number(stTerm.stFrameBmsInfo.batery_SOC, 10) + "%";
    }
    else
    {
        tempString = "无";
    }
    tempArray = pGBK->fromUnicode(tempString);
    memset(pScreen->chSOC, 0x00, sizeof(pScreen->chSOC));
    memcpy(pScreen->chSOC, tempArray.data(), tempArray.length());

    int iChargeEnergy = 0;
    int iTempChargeEnergy = 0;

    iTempChargeEnergy = stTerm.stFrameRemoteMeSurement2.active_electric_energy - temStep.u32EnergyStartCharge;
    if(iTempChargeEnergy >= 0){
        iChargeEnergy = iTempChargeEnergy;
    }
    else{
		double dAmmeterRange = pParamSet->getAmmeterRange(stTerm.cCanAddr);
        if(( temStep.u32EnergyStartCharge > ((int)(dAmmeterRange * 100) - 60000) ) \
                && ( stTerm.stFrameRemoteMeSurement2.active_electric_energy <  60000) ){
            iChargeEnergy = (int)(dAmmeterRange * 100.0) - temStep.u32EnergyStartCharge + stTerm.stFrameRemoteMeSurement2.active_electric_energy;
        }
    }

    pScreen->usChargeEnergy = htons((unsigned short)(iChargeEnergy));
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
}

//终端详情界面----发送终端BMS信息
void cSerialScreenProtocol::SendTermBMSInfo(unsigned char ucCanID)
{
    TerminalStatus stTerm;
    FrameTermBMS_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FrameTermBMS_Screen)];

    QByteArray TempArray;
    QString TempString;
    QTextCodec * pGBK = QTextCodec::codecForName("GBK");

    pScreen = (FrameTermBMS_Screen*)pData;
    if(!pDevCache->QueryTerminalStatus(ucCanID, stTerm))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! SendTermBMSInfo QueryTerminalStatus Error!");
    }
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameTermBMS_Screen) - 3);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //终端详情 寄存器地址
    pScreen->usAddr = htons(Reg_TermBMS_Screen);
    //数据赋值
    pScreen->usBMSNeedVoltage = htons((short)(stTerm.stFrameBmsInfo.BMS_need_voltage/0.1));
    pScreen->sBMSNeedCurrent = htons((short)(stTerm.stFrameBmsInfo.BMS_need_current/0.1));
    pScreen->usBaterySOC = htons((unsigned short)stTerm.stFrameBmsInfo.batery_SOC);
    pScreen->usMaxBateryTemperature = htons((short)stTerm.stFrameBmsInfo.max_batery_temperature);
    pScreen->sMaxBateryVoltage = htons((short)(stTerm.stFrameBmsInfo.max_batery_voltage/0.01));
    pScreen->usLowestBatteryTemperature = htons((unsigned short)stTerm.stFrameBmsInfo.lowest_battery_temperature);
    pScreen->usLowestChargeVoltage = htons((unsigned short)(stTerm.stFrameBmsInfo.lowest_charge_voltage/0.01));

    //电池组充电时间
    pScreen->usBatteryChargeTime = htons((short)(0));
    //电池组额定电压
    pScreen->usBatteryRatedVoltage = htons((short)(stTerm.stFrameBmsHand.BatteryRatedVoltage / 0.1));
    //电池组额定容量
    pScreen->usBatteryRatedCapacity = htons((short)(stTerm.stFrameBmsHand.BatteryRatedCapacity / 0.1));
    //BMS工作模式
    switch (stTerm.stFrameBmsInfo.ChargeType) {
    case 1://恒压充电
        TempString = "恒压充电";
        break;
    case 2://恒流充电
        TempString = "恒流充电";
        break;
    default://未知
        TempString = "未知";
        break;
    }
    TempArray = pGBK->fromUnicode(TempString);
    memset(pScreen->sBMSWorkMode, 0x00, sizeof(pScreen->sBMSWorkMode));
    memcpy(pScreen->sBMSWorkMode, TempArray.data(), TempArray.length());
    //电池类型
    switch (stTerm.stFrameBmsHand.BatteryType) {
    case 1://铅酸电池
        TempString = "铅酸电池";
        break;
    case 2://镍氢电池
        TempString = "镍氢电池";
        break;
    case 3://磷酸铁锂电池
        TempString = "磷酸铁锂电池";
        break;
    case 4://锰酸锂电池
        TempString = "锰酸锂电池";
        break;
    case 5://钴酸锂电池
        TempString = "钴酸锂电池";
        break;
    case 6://三元材料电池
        TempString = "三元材料电池";
        break;
    case 7://聚合物锂电池
        TempString = "聚合物锂电池";
        break;
    case 8://钛酸锂电池
        TempString = "钛酸锂电池";
        break;
    case 0xFF://其他电池
        TempString = "其他电池";
        break;
    default://未知
        TempString = "其他电池";
        break;
    }
    TempArray = pGBK->fromUnicode(TempString);
    memset(pScreen->sBatteryType, 0x00, sizeof(pScreen->sBatteryType));
    memcpy(pScreen->sBatteryType, TempArray.data(), TempArray.length());

    //发送帧
    SendFrame(pData, sizeof(* pScreen));

}

//VIN----发送终端VIN信息----普通版独有
void cSerialScreenProtocol::SendTermVINInfo(TerminalStatus & stTerm)
{
    FrameVINInfo_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FrameVINInfo_Screen)];
    pScreen = (FrameVINInfo_Screen*)pData;

    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameVINInfo_Screen) - 3);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //终端详情 寄存器地址
    pScreen->usAddr = htons(Reg_TermVIN_Screen);
    //数据赋值
    strncpy(pScreen->chVIN, (char *)stTerm.stFrameBmsInfo.BMS_car_VIN, sizeof(pScreen->chVIN));
    memset(pScreen->chVIN + 11, '*', 6);
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
}

inline void cSerialScreenProtocol::MakeFrameHead(FrameHead_Screen & strHead, unsigned char ucLength)
{
    strHead.ucHead1 = 0x5A;
    strHead.ucHead2 = 0xA5;
    strHead.ucDataLength = ucLength;
}

void cSerialScreenProtocol::SendFrame(unsigned char * pData, int iLength)
{
//    for(int i = 0; i < iLength; i++)
//    {
//    }
    emit sigSendSerialData(pData, iLength);
}

//发送场站名称
void cSerialScreenProtocol::SendFrameStationName(char * pName)
{
    FrameStationName_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FrameStationName_Screen)];
    pScreen = (FrameStationName_Screen*)pData;
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameStationName_Screen) - 3);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //场站名称 寄存器地址
    pScreen->usAddr = htons(Reg_StationName_Screen);
    //写入站名称----UTF-8转换为GBK
    QByteArray TempArray;
    QString TempString;
    QTextCodec * pGBK = QTextCodec::codecForName("GBK");
    TempArray.append(pName);
    TempString = TempArray;
    TempArray = pGBK->fromUnicode(TempString);
    if(TempArray.length() > 40)  //站名称大于40，截取前40
        TempArray = TempArray.left(40);
    memset(pScreen->ucStationName, 0x00, sizeof(pScreen->ucStationName));
    memcpy(pScreen->ucStationName, TempArray.data(), TempArray.length());
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
}

//发送 CSCU版本号----主界面
void cSerialScreenProtocol::SendFrameCSCUProVer(char * pName)
{
    FrameCSCUProVer_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FrameCSCUProVer_Screen)];
    pScreen = (FrameCSCUProVer_Screen*)pData;
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameCSCUProVer_Screen) - 3);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //场站名称 寄存器地址
    pScreen->usAddr = htons(Reg_CSCUProVer_Screen);
    //写入CSCU版本号
    memset(pScreen->ucProVer, 0x00, sizeof(pScreen->ucProVer));
    memcpy(pScreen->ucProVer, pName, sizeof(pScreen->ucProVer));
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
}

//发送屏幕重启指令(返回按钮结构体)
void cSerialScreenProtocol::SendFrameScreenReboot()
{
    FrameRebootCmd_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FrameRebootCmd_Screen)];
    pScreen = (FrameRebootCmd_Screen*)pData;
    unsigned short Temp_Addr;
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameRebootCmd_Screen) - 3);
    //屏幕重启指令 寄存器地址
    Temp_Addr = Reg_RebootCmd_Screen;
    pScreen->usAddr = htons(Temp_Addr);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //屏幕重启指令 数据固定值: 0x0002
    pScreen->usREnsureFlag = htons(0x0002);
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
}

//发送 对时指令
void cSerialScreenProtocol::SendFrameSetTime()
{
    FrameSetTime_Screen * pScreen;
    QDateTime NowDateTime = QDateTime::currentDateTime();
    QString stDateTime;

    unsigned char * pData = new unsigned char[sizeof(FrameSetTime_Screen)];
    pScreen = (FrameSetTime_Screen*)pData;
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameSetTime_Screen) - 3);
    //TEUI屏对时 寄存器地址
    pScreen->ucAddr = Reg_Spec_SetTime_Screen;
    //指令类型: 写控制寄存器
    pScreen->ucCmdType = Type_WriteCtrl_Screen;
    //对时标识 数据固定值: 0x5A
    pScreen->ucCheckFlag = 0x5A;
    //数据赋值
    stDateTime = NowDateTime.toString("yyMMdd24hhmmss");
    CharToBCD((unsigned char*)stDateTime.toAscii().data(),14,(unsigned char*)&(pScreen->ucYear));
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
}

//发送 屏幕灵敏度选择
void cSerialScreenProtocol::SendFrameSetSensitivity()
{
    FrameSetSensitivity_Screen * pScreen;

    unsigned char * pData = new unsigned char[sizeof(FrameSetSensitivity_Screen)];
    pScreen = (FrameSetSensitivity_Screen*)pData;
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameSetSensitivity_Screen) - 3);
    //TEUI屏对时 寄存器地址
    pScreen->ucAddr = Reg_Spec_SetSensitivity_Screen;
    //指令类型: 写控制寄存器
    pScreen->ucCmdType = Type_WriteCtrl_Screen;
    //数据固定值:
    pScreen->ucFlagOne = 0x5A;
    pScreen->ucFlagTwo = 0x5A;
    pScreen->ucFlagThree = 0x1E;
    pScreen->ucFlagFour = 0x08;
    pScreen->ucFlagFive = 0x0A;
    //灵敏度设置
    pScreen->ucSetValue = 0x1F;
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
}

//发送 各终端名称,ID,状态, 故障图标状态----主界面
void cSerialScreenProtocol::SendFrameTermInfo(TermNameMap &NameMap,TermNameMap &NameMapShow)
{
    //串口屏地址寄存器数组偏移
    unsigned char ucAddrOffset = 0;
//    unsigned char TotalNumber=0;
    unsigned short usACSinNum = 0;
    unsigned short usACThrNum = 0;
    unsigned short usDCNum = 0;
    //终端状态缓存
    TerminalStatus stTemp;
    //字符编码转换
    QTextCodec * pGBK = QTextCodec::codecForName("GBK");
    QString TempString;
    QByteArray TempArray;
    TermNameMap NameMapTemp;
    int clearnum=0;
    //故障图标状态
    bool bFaultFlag = FALSE;
    stChargeConfig charge;

    pParamSet->querySetting(&charge, PARAM_CHARGE);

    if(NameMap.isEmpty())
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! NameMap Empty");
    }
    if(NameMapShow.isEmpty() || (NameMap.size() <NameMapShow.size()) || (charge.coupleGun == 0))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! NameMapShow Empty");
        NameMapTemp = NameMap;
    }
    else
    {
         NameMapTemp = NameMapShow;
         clearnum = NameMap.size() -NameMapShow.size();
    }

    TermNameMap::iterator it;
    //向串口屏发送对应的终端名称,地址
    for(it = NameMapTemp.begin(); it!= NameMapTemp.end(); ++it)
    {
        if(pDevCache->QueryTerminalStatus(it.key(), stTemp) == FALSE)
        {
            pLog->getLogPoint(_strLogName)->info("ERROR! SendFrameTermInfo QueryTerminalStatus ERROR");
            break;
        }
        TempString = it.value();
        TempArray = pGBK->fromUnicode(TempString);
        //有故障, 则故障图标显示
        if((stTemp.cStatus == CHARGE_STATUS_FAULT) || (stTemp.stFrameRemoteSingle.status_fault != 0))
        {
            bFaultFlag = TRUE;
        }
        //发送名称, CAN地址, 终端状态, 故障图标状态
        SendFrameTermName(ucAddrOffset, TempArray.data(), it.value().length());
        SendFrameTermID(ucAddrOffset, it.key());

        if(charge.coupleGun !=0 && stTemp.cStatus !=CHARGE_STATUS_DISCONNECT)
        {
            if(stTemp.gunType == SLAVE_GUN)//副枪
            {
                stTemp.cStatus = CHARGE_STATUS_SLAVEGUN;
            }
            else if(stTemp.gunType == COUPLE_ERR)//配对错误
            {
                stTemp.cStatus = CHARGE_STATUS_COUPLE_ERR;
            }
        }

        //查数据库获取终端类型
        //是主枪则判断终端类型
        int result=0;
        //if(stTemp.gunType == MASTER_GUN)
        if(charge.coupleGun !=0)
        {
            //
            result = GetMultiType(it.key());
        }
        SendFrameTermState(ucAddrOffset, stTemp.cStatus,result);
        SendFrameFaultIcon(bFaultFlag);
        ucAddrOffset++;
//        TotalNumber++;
        if(it.key() >= 1 && it.key() <= 110)
            usACSinNum++;
        else if(it.key() >= 151 && it.key() <= 180)
            usACThrNum++;
        else if(it.key() >= 181 && it.key() <= 230)
            usDCNum++;
    }
    //根据实际个数屏幕显示 hd 2018-4-2
    ParamSetPage_Screen stParamSetPage;
    memset(&stParamSetPage,0,sizeof(ParamSetPage_Screen));
    stParamSetPage.usACSinNum = usACSinNum;
    stParamSetPage.usACThrNum = usACThrNum;
    stParamSetPage.usDCNum = usDCNum;
    SendFrameTermNum(stParamSetPage);
    for(int i=0;i<clearnum;i++ )
    {
        TempString = '\0';
        TempArray = pGBK->fromUnicode(TempString);
        //bFaultFlag = FALSE;

        //发送名称, CAN地址, 终端状态, 故障图标状态
        SendFrameTermName(ucAddrOffset, TempArray.data(), 1);
        SendFrameTermID(ucAddrOffset, 0);

        SendFrameTermState(ucAddrOffset, 0xff);
        //SendFrameFaultIcon(bFaultFlag);
        ucAddrOffset++;
    }
}

int cSerialScreenProtocol::GetMultiType(unsigned char ucCanID)
{
    db_result_st dbst;
    QString todo;
    int result=0;
    //数据库操作----查询静态参数
    todo = "SELECT multitype  FROM terminal_name_multi_table WHERE canaddr = "+ QString::number(ucCanID, 10);
    if(0 != pDBOperate->DBSqlQuery(todo.toAscii().data(), &dbst, DB_PARAM))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! ParseTermStaticArg Query Error");
    }
    if(dbst.row != 0)
    {
        result = atoi(dbst.result[0]);
    }
    pDBOperate->DBQueryFree(&dbst);
    return result;
}
//发送 终端名称帧,----主界面
//ucOffset: 距终端名称首地址的偏移量 0-159
//pName: 终端名称
//ucNameLength: 终端名称串长度
void cSerialScreenProtocol::SendFrameTermName(unsigned char ucOffset,  char *pName, int iNameLength)
{
    FrameSendTermName_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FrameSendTermName_Screen)];
    pScreen = (FrameSendTermName_Screen*)pData;
    unsigned short Temp_Addr;

    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameSendTermName_Screen) - 3);
    //CAN终端名称 寄存器地址: 起始地址 + 间隔 * 偏移数量
    Temp_Addr = Reg_TermNameBegin_Screen + (RegIn_TermNameInterval_Screen * ucOffset);
    pScreen->usAddr = htons(Temp_Addr);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //终端名称赋值
    memset(pScreen->ucTermName, 0x00, sizeof(pScreen->ucTermName));
    if(strlen(pName)>9)   //多枪删除多余汉字  2018-3-13 hd
        iNameLength = strlen(pName)-6;
    if(iNameLength>9)   //不能超过9个字节   2018-3-13 hd
        iNameLength =9;
    memcpy(pScreen->ucTermName, (unsigned char *)pName, iNameLength);
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
}

//发送 终端ID帧,----主界面
//ucOffset: 距终端名称首地址的偏移量 0-159
//ucTermID: 终端ID
void cSerialScreenProtocol::SendFrameTermID(unsigned char ucOffset,  unsigned char ucTermID)
{
    FrameSendTermID_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FrameSendTermID_Screen)];
    pScreen = (FrameSendTermID_Screen*)pData;
    unsigned short Temp_Addr;

    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameSendTermID_Screen) - 3);
    //CAN终端名称 寄存器地址: 起始地址 + 间隔 * 偏移数量
    Temp_Addr = Reg_TermCanIDBegin_Screen + (RegIn_TermIDInterval_Screen * ucOffset);
    pScreen->usAddr = htons(Temp_Addr);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //终端ID赋值
    pScreen->usCanID = htons((short)ucTermID);
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
}

//发送 终端状态----主界面
void cSerialScreenProtocol::SendFrameTermState(unsigned char ucOffset,  unsigned char ucTermState,int multitype)
{
    bool bCartoonFlag = FALSE;//动画标识
    FrameSendTermState_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FrameSendTermState_Screen)];
    pScreen = (FrameSendTermState_Screen*)pData;
    unsigned short Temp_short;

    memset(pData, 0x00, sizeof( * pScreen));
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameSendTermState_Screen) - 3);
    //终端状态 寄存器地址: 起始地址 + 间隔 * 偏移数量
    Temp_short = Reg_TermStateBegin_Screen + (RegIn_TermStateInterval_Screen * ucOffset);
    pScreen->usAddr = htons(Temp_short);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //终端状态赋值
    Temp_short = CheckScreenTermState(ucTermState,bCartoonFlag,multitype);
    pScreen->usState = htons(Temp_short);
    //动画状态赋值
    if(bCartoonFlag)
    {
        pScreen->usCartoonType = pScreen->usState;
    }
    //预留字节清零
    memset(pScreen->ucReserved, 0x00, sizeof(pScreen->ucReserved));
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
}

//发送 故障图标----主界面
//bFaultFlag: FALSE: 无故障, TRUE: 有故障
void cSerialScreenProtocol::SendFrameFaultIcon(bool bFaultFlag)
{
    FrameFaultInfoIcon_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FrameFaultInfoIcon_Screen)];
    pScreen = (FrameFaultInfoIcon_Screen*)pData;
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameFaultInfoIcon_Screen) - 3);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //故障图标 寄存器地址
    pScreen->usAddr = htons(Reg_FaultInfoIcon_Screen);
    //故障图标赋值
    if(bFaultFlag)
    {
        pScreen->usFaultFlag = htons(0x0001);
    }
    else
    {
        pScreen->usFaultFlag = htons(0x0000);
    }
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
}

//发送 终端充电报告----充电报告界面
void cSerialScreenProtocol::SendTermChargeReport(unsigned char ucCanID)
{
    FrameTermChargeReport_Screen * pScreen;
    CHARGE_STEP stChargeStep;
    //字符编码转换
    QTextCodec * pGBK = QTextCodec::codecForName("GBK");
    QString tempString;
    QByteArray tempArray;
    //充电时间转换
    QDateTime startTime, stopTime;
    QTime tempTime;
    int seconds = 0;
    unsigned short usH, usM, usS;

    unsigned char * pData = new unsigned char[sizeof(FrameTermChargeReport_Screen)];
    pScreen = (FrameTermChargeReport_Screen*)pData;
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameTermChargeReport_Screen) - 3);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //充电报告 寄存器地址
    pScreen->usAddr = htons(Reg_ChargeReport_Screen);
    //数据赋值
    pDevCache->QueryChargeStep(ucCanID, stChargeStep);
    //起始SOC
    if((ucCanID <= ID_MaxDCCanID) &&(ucCanID >= ID_MinDCCanID))
    {
        tempString = QString::number(stChargeStep.ucStartSOC, 10) + "%";
    }
    else
    {
        tempString = "无";
    }
    tempArray = pGBK->fromUnicode(tempString);
    memset(pScreen->cStartSOC, 0x00, sizeof(pScreen->cStartSOC));
    memcpy(pScreen->cStartSOC, tempArray.data(), tempArray.length());
    //中止SOC
    if((ucCanID <= ID_MaxDCCanID) &&(ucCanID >= ID_MinDCCanID))
    {
        tempString = QString::number(stChargeStep.ucEndSOC, 10) + "%";
    }
    else
    {
        tempString = "无";
    }
    tempArray = pGBK->fromUnicode(tempString);
    memset((unsigned char *)&pScreen->cStopSOC, 0x00, sizeof(pScreen->cStopSOC));
    memcpy((unsigned char *)&pScreen->cStopSOC, tempArray.data(), tempArray.length());
    //充电电能
    pScreen->usEnergy = htons((unsigned short)(stChargeStep.u32TotalChargeEnergy));
    //开始充电时间
    startTime = QDateTime::fromString(QString(stChargeStep.sStartTime),"yyyy-MM-dd HH:mm:ss");
    tempTime = startTime.time();
    pScreen->sStartHour = htons((short)tempTime.hour());
    pScreen->sStartMin = htons((short)tempTime.minute());
    pScreen->sStartSec = htons((short)tempTime.second());
    //结束充电时间
    stopTime = QDateTime::fromString(QString(stChargeStep.sEndTime),"yyyy-MM-dd HH:mm:ss");
    tempTime = stopTime.time();

    pScreen->sStopHour = htons((short)tempTime.hour());
    pScreen->sStopMin = htons((short)tempTime.minute());
    pScreen->sStopSec = htons((short)tempTime.second());
    //充电时间
    seconds = startTime.secsTo(stopTime);
    //时间错误,则充电时间显示0
    if(seconds < 0)
    {
        seconds = 0;
    }

    usH = seconds/3600;
    usM = seconds/60 -usH*60;
    usS = seconds%60;

    pScreen->sChargeHour = htons(usH);
    pScreen->sChargeMin = htons(usM);
    pScreen->sChargeSec = htons(usS);

    //设备结束原因
    QString qstrDevStopReasonName;
    if(QueryDevStopReasonName(stChargeStep.ucStopReasonDev, qstrDevStopReasonName) == false){
        qstrDevStopReasonName = "未知";
    }

    tempArray.clear();
    tempArray = pGBK->fromUnicode(qstrDevStopReasonName);
    memset((unsigned char *)&pScreen->strStopReasonDev, 0x00, sizeof(pScreen->strStopReasonDev));
    memcpy((unsigned char *)&pScreen->strStopReasonDev, tempArray.data(), tempArray.length());

    //订单流水号
    if(stChargeStep.sEventNo[0] == 0 && stChargeStep.sEventNo[2] == 0 && stChargeStep.sEventNo[6] == 0){
        tempString = "无";
        tempArray.clear();
        tempArray = pGBK->fromUnicode(tempString);
        memset((unsigned char *)&pScreen->strEventNo, 0x00, sizeof(pScreen->strEventNo));
        memcpy((unsigned char *)&pScreen->strEventNo, tempArray.data(), tempArray.length());
    }
    else{
        memcpy(pScreen->strEventNo, QByteArray::fromRawData(stChargeStep.sEventNo,  sizeof(stChargeStep.sEventNo)).toHex().data(), sizeof(pScreen->strEventNo) );
    }
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
}


//发送 充电明细1
void cSerialScreenProtocol::SendTermChargeReportDetail1(unsigned char ucCanID)
{
    FrameTermChargeReportDetail1_Screen * pScreen;
    CHARGE_STEP stChargeStep;
    //字符编码转换
    QTextCodec * pGBK = QTextCodec::codecForName("GBK");
    QString tempString;
    QByteArray tempArray;

    //充电时间转换
    QDateTime startTime, stopTime;
    QTime tempTime;
    int seconds = 0;
    unsigned short usH, usM, usS;

    unsigned char * pData = new unsigned char[sizeof(FrameTermChargeReportDetail1_Screen)];
    pScreen = (FrameTermChargeReportDetail1_Screen*)pData;
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameTermChargeReportDetail1_Screen) - 3);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //充电明细1 寄存器地址
    pScreen->usAddr = htons(Reg_ChargeReportDetail1_Screen);
    //数据赋值
    if(pDevCache->QueryChargeStep(ucCanID, stChargeStep) == false){
        return;
    }

    //内部产生的uuid
    memcpy(pScreen->strOrderUUID, QByteArray::fromRawData(stChargeStep.sOrderUUID,  sizeof(stChargeStep.sOrderUUID)).toHex().data(), sizeof(pScreen->strOrderUUID) );
   //指令来源
    if(QueryCmdSrcName(stChargeStep.ucCmdSrc, tempString) == false){
        tempString = "未知";
    }
    tempArray.clear();
    tempArray = pGBK->fromUnicode(tempString);
    memset((unsigned char *)&pScreen->strCmdSrc, 0x00, sizeof(pScreen->strCmdSrc));
    memcpy((unsigned char *)&pScreen->strCmdSrc, tempArray.data(), tempArray.length());
    //开始原因
    unsigned char ucStartReason;//由于开始充电原因,结算后被清空,需要从数据库中获取
    if(QueryStartReasonCode(stChargeStep, ucStartReason) == false){
        tempString = "未知";
    }
    else{
        if(QueryStartReasonName(ucStartReason, tempString) == false){
            tempString = "未知";
        }
    }

    tempArray.clear();
    tempArray = pGBK->fromUnicode(tempString);
    memset((unsigned char *)&pScreen->strStartReason, 0x00, sizeof(pScreen->strStartReason));
    memcpy((unsigned char *)&pScreen->strStartReason, tempArray.data(), tempArray.length());
    //开始充电时间
    startTime = QDateTime::fromString(QString(stChargeStep.sStartTime),"yyyy-MM-dd HH:mm:ss");
    tempTime = startTime.time();
    pScreen->sStartHour = htons((short)tempTime.hour());
    pScreen->sStartMin = htons((short)tempTime.minute());
    pScreen->sStartSec = htons((short)tempTime.second());
    //结束充电时间
    stopTime = QDateTime::fromString(QString(stChargeStep.sEndTime),"yyyy-MM-dd HH:mm:ss");
    tempTime = stopTime.time();

    pScreen->sStopHour = htons((short)tempTime.hour());
    pScreen->sStopMin = htons((short)tempTime.minute());
    pScreen->sStopSec = htons((short)tempTime.second());
    //充电时间
    seconds = startTime.secsTo(stopTime);
    //时间错误,则充电时间显示0
    if(seconds < 0)
    {
        seconds = 0;
    }

    usH = seconds/3600;
    usM = seconds/60 -usH*60;
    usS = seconds%60;

    pScreen->sChargeHour = htons(usH);
    pScreen->sChargeMin = htons(usM);
    pScreen->sChargeSec = htons(usS);

    //开始充电电能
    pScreen->u32StartEnergy = htonl((unsigned int)(stChargeStep.u32EnergyStartCharge));
    //结束充电电能
    pScreen->u32StopsEnergy = htonl((unsigned int)(stChargeStep.u32EnergyEndCharge));
    //总用电电能
    pScreen->usEnergy = htons((unsigned short)(stChargeStep.u32TotalChargeEnergy));

    //订单流水号
    if(stChargeStep.sEventNo[0] == 0 && stChargeStep.sEventNo[2] == 0 && stChargeStep.sEventNo[6] == 0){
        tempString = "无";
        tempArray.clear();
        tempArray = pGBK->fromUnicode(tempString);
        memset((unsigned char *)&pScreen->strEventNo, 0x00, sizeof(pScreen->strEventNo));
        memcpy((unsigned char *)&pScreen->strEventNo, tempArray.data(), tempArray.length());
    }
    else{
        memcpy(pScreen->strEventNo, QByteArray::fromRawData(stChargeStep.sEventNo,  sizeof(stChargeStep.sEventNo)).toHex().data(), sizeof(pScreen->strEventNo) );
    }
    //卡号
    if(stChargeStep.sCardNo[0] == 0){
         tempString = "无";
         tempArray.clear();
         tempArray = pGBK->fromUnicode(tempString);
         memset((unsigned char *)&pScreen->strCardNo, 0x00, sizeof(pScreen->strCardNo));
         memcpy((unsigned char *)&pScreen->strCardNo, tempArray.data(), tempArray.length());
    }
    else{
        memcpy(pScreen->strCardNo, stChargeStep.sCardNo, sizeof(pScreen->strCardNo) );
    }
    //VIN号
    if(stChargeStep.sVIN[0]  == 0){
         tempString = "无";
         tempArray.clear();
         tempArray = pGBK->fromUnicode(tempString);
         memset((unsigned char *)&pScreen->strVIN, 0x00, sizeof(pScreen->strVIN));
         memcpy((unsigned char *)&pScreen->strVIN, tempArray.data(), tempArray.length());
    }
    else{
        memcpy(pScreen->strVIN, stChargeStep.sVIN, sizeof(stChargeStep.sVIN) );
    }
    //车牌号
    if(stChargeStep.sCarLisence[0] == 0){
        tempString = "无";
    }
    else{
        if(QueryCarLisenceName(stChargeStep.sCarLisence, tempString) == false){
            tempString = "无";
        }
    }
     tempArray.clear();
     tempArray = pGBK->fromUnicode(tempString);
     memset((unsigned char *)&pScreen->strCarLisence, 0x00, sizeof(pScreen->strCarLisence));
     memcpy((unsigned char *)&pScreen->strCarLisence, tempArray.data(), tempArray.length());

     //发送帧
     SendFrame(pData, sizeof(* pScreen));
}

//发送 充电明细2
void cSerialScreenProtocol::SendTermChargeReportDetail2(unsigned char ucCanID)
{
    FrameTermChargeReportDetail2_Screen * pScreen;
    CHARGE_STEP stChargeStep;
    //字符编码转换
    QTextCodec * pGBK = QTextCodec::codecForName("GBK");
    QString tempString;
    QByteArray tempArray;

    unsigned char * pData = new unsigned char[sizeof(FrameTermChargeReportDetail2_Screen)];
    pScreen = (FrameTermChargeReportDetail2_Screen*)pData;
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameTermChargeReportDetail2_Screen) - 3);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //充电明细2 寄存器地址
    pScreen->usAddr = htons(Reg_ChargeReportDetail2_Screen);
    //数据赋值
    if(pDevCache->QueryChargeStep(ucCanID, stChargeStep) == false){
        return;
    }
    //CSCU中止充电原因
    if(QueryCSCUStopReasonName(stChargeStep.ucStopReasonCSCU, tempString) == false){
        tempString = "未知";
    }
    tempArray.clear();
    tempArray = pGBK->fromUnicode(tempString);
    memset((unsigned char *)&pScreen->strStopReasonCSCU, 0x00, sizeof(pScreen->strStopReasonCSCU));
    memcpy((unsigned char *)&pScreen->strStopReasonCSCU, tempArray.data(), tempArray.length());

    //服务器中止充电原因
    if(QueryCloudStopReasonName(stChargeStep.ucStopReasonCloud, tempString) == false){
        tempString = "未知";
    }
    tempArray.clear();
    tempArray = pGBK->fromUnicode(tempString);
    memset((unsigned char *)&pScreen->strStopReasonCloud, 0x00, sizeof(pScreen->strStopReasonCloud));
    memcpy((unsigned char *)&pScreen->strStopReasonCloud, tempArray.data(), tempArray.length());

    //设备结束原因
    if(QueryDevStopReasonName(stChargeStep.ucStopReasonDev, tempString) == false){
        tempString = "未知";
    }
    tempArray.clear();
    tempArray = pGBK->fromUnicode(tempString);
    memset((unsigned char *)&pScreen->strStopReasonDev, 0x00, sizeof(pScreen->strStopReasonDev));
    memcpy((unsigned char *)&pScreen->strStopReasonDev, tempArray.data(), tempArray.length());

    //订单状态
    if(QueryOrderStatusName(stChargeStep.enOrderStatus, tempString) == false){
        tempString = "未知";
    }
    tempArray.clear();
    tempArray = pGBK->fromUnicode(tempString);
    memset((unsigned char *)&pScreen->strOrderStatus, 0x00, sizeof(pScreen->strOrderStatus));
    memcpy((unsigned char *)&pScreen->strOrderStatus, tempArray.data(), tempArray.length());

    //发送帧
    SendFrame(pData, sizeof(* pScreen));

}

//查询数据字典-设备中止原因对照表
bool cSerialScreenProtocol::QueryDevStopReasonName(unsigned char ucCode, QString &StringName)
{
    struct db_result_st result;
    QString qstrTableName = "charge_stop_name_table";
    QString todo = QString("select name from '%1' where code = '%2';")\
            .arg(qstrTableName)\
            .arg(QString::number(ucCode, 10));
    int ret = pDBOperate->DBSqlQuery(todo.toAscii().data(), &result, DB_PARAM);
    if(ret == 0)//查询成功
    {
        if(result.column == 1)
        {
            if(result.row >= 1){//至少有一个代码名称
                StringName = QString::fromUtf8(result.result[0]);
            }//end of 判断几行
        }//end of 判断几列
        else{
            StringName = "未知";
        }
        pDBOperate->DBQueryFree(&result);
    }
    else{
        return false;
    }
    return true;
}

//查询数据字典-CSCU中止原因对照表
bool cSerialScreenProtocol::QueryCSCUStopReasonName(unsigned char ucCode, QString &StringName)
{
    struct db_result_st result;
    QString qstrTableName = "order_stop_name_table";
    QString todo = QString("select name from '%1' where code = '%2';")\
            .arg(qstrTableName)\
            .arg(QString::number(ucCode, 10));
    int ret = pDBOperate->DBSqlQuery(todo.toAscii().data(), &result, DB_PARAM);
    if(ret == 0)//查询成功
    {
        if(result.column == 1)
        {
            if(result.row >= 1){//至少有一个代码名称
                StringName = QString::fromUtf8(result.result[0]);
            }//end of 判断几行
        }//end of 判断几列
        else{
            StringName = "未知";
        }
        pDBOperate->DBQueryFree(&result);
    }
    else{
        return false;
    }
    return true;
}

//查询数据字典-云平台中止原因对照表
bool cSerialScreenProtocol::QueryCloudStopReasonName(unsigned char ucCode, QString &StringName)
{
    unsigned char temp;

    temp = ucCode;
    StringName = "未启用";
    return true;
}

//查询数据字典-订单状态对照表
bool cSerialScreenProtocol::QueryOrderStatusName(int OrderStatus, QString &StringName)
{
    switch (OrderStatus) {
    case ORDER_STATUS_NON:
        StringName = "未知";
        break;
    case ORDER_STATUS_QUEUE:
        StringName = "排队";
        break;
    case ORDER_STATUS_FAIL:
        StringName = "失败";
        break;
    case ORDER_STATUS_OK:
        StringName = "成功";
        break;
    case ORDER_STATUS_ING:
        StringName = "进行";
        break;
    default:
        StringName = "未知";
        break;
    }
    return true;
}

//查询数据字典-指令来源对照表
bool cSerialScreenProtocol::QueryCmdSrcName(unsigned char ucCode, QString &StringName)
{
    struct db_result_st result;
    QString qstrTableName = "order_cmdsrc_name_table";
    QString todo = QString("select name from '%1' where code = '%2';")\
            .arg(qstrTableName)\
            .arg(QString::number(ucCode, 10));
    int ret = pDBOperate->DBSqlQuery(todo.toAscii().data(), &result, DB_PARAM);
    if(ret == 0)//查询成功
    {
        if(result.column == 1)
        {
            if(result.row >= 1){//至少有一个代码名称
                StringName = QString::fromUtf8(result.result[0]);
            }//end of 判断几行
        }//end of 判断几列
        else{
            StringName = "未知";
        }
        pDBOperate->DBQueryFree(&result);
    }
    else{
        return false;
    }
    return true;
}

//查询数据字典-开始原因对照表
bool cSerialScreenProtocol::QueryStartReasonName(unsigned char ucCode, QString &StringName)
{
    struct db_result_st result;
    QString qstrTableName = "order_start_name_table";
    QString todo = QString("select name from '%1' where code = '%2';")\
            .arg(qstrTableName)\
            .arg(QString::number(ucCode, 10));
    int ret = pDBOperate->DBSqlQuery(todo.toAscii().data(), &result, DB_PARAM);
    if(ret == 0)//查询成功
    {
        if(result.column == 1)
        {
            if(result.row >= 1){//至少有一个代码名称
                StringName = QString::fromUtf8(result.result[0]);
            }//end of 判断几行
        }//end of 判断几列
        else{
            StringName = "未知";
        }
        pDBOperate->DBQueryFree(&result);
    }
    else{
        return false;
    }
    return true;
}



//查询数据字典-开始原因对照表
bool cSerialScreenProtocol::QueryStartReasonCode(CHARGE_STEP &stChargeStep, unsigned char  &code)
{
    struct db_result_st result;
    QString todo = QString("select StartReason from charge_order where UUIDOwn = '%2';")\
            .arg(QByteArray::fromRawData(stChargeStep.sOrderUUID, LENGTH_GUID_NO).toHex().data());
    int ret = pDBOperate->DBSqlQuery(todo.toAscii().data(), &result, DB_PROCESS_RECORD);
    if(ret == 0)//查询成功
    {
        if(result.column == 1)
        {
            if(result.row >= 1){//至少有一个代码名称
                code = (unsigned char )atoi(result.result[0]);
            }//end of 判断几行
        }//end of 判断几列
        else{
            return false;
        }
        pDBOperate->DBQueryFree(&result);
    }
    else{
        return false;
    }
    return true;
}

//发送 查询TEUI版本号
void cSerialScreenProtocol::SendFrameQueryTEUIVer()
{
    Frame_TEUIQueryVer_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(Frame_TEUIQueryVer_Screen)];
    pScreen = (Frame_TEUIQueryVer_Screen*)pData;
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(Frame_TEUIQueryVer_Screen) - 3);
    //指令类型: 读数据寄存器
    pScreen->ucCmdType = Type_ReadMem_Screen;
    //场站名称 寄存器地址
    pScreen->usAddr = htons(Reg_TEUIProVer_Screen);
    //读取10个地址
    pScreen->cNo = 10;
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
}

//发送 终端故障列表界面----一条记录
void cSerialScreenProtocol::SendFaultInfoLine(unsigned char ucAddrOffset, TerminalStatus &Status)
{
    unsigned short usRegAddr = 0;
    FrameFaultInfoLine_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FrameFaultInfoLine_Screen)];
    pScreen = (FrameFaultInfoLine_Screen*)pData;
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameFaultInfoLine_Screen) - 3);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //故障列表记录 寄存器地址
    usRegAddr = Reg_FaultInfoLineBeagin_Screen + Reg_FaultInfoLineInterval_Screen * ucAddrOffset;
    pScreen->usAddr = htons(usRegAddr);
    //数据赋值
    pScreen->usFaultNum = htons((unsigned short)(ucAddrOffset + 1));
    pScreen->usCanID = htons((unsigned short)Status.cCanAddr);
    pScreen->usFaultCode = htons((unsigned short)Status.stFrameRemoteSingle.status_fault);
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
}

//发送 串口通信状态
void cSerialScreenProtocol::SendFrameSerialState()
{
    FrameSerialState_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FrameSerialState_Screen)];
    pScreen = (FrameSerialState_Screen*)pData;
    unsigned short Temp_Addr;
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameSerialState_Screen) - 3);
    //串口通信状态 寄存器地址
    Temp_Addr = Reg_SerialState_Screen;
    pScreen->usAddr = htons(Temp_Addr);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //串口通信状态 数据固定值: 0x0001
    pScreen->usSerialState = htons(0x0001);
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
}

//发送 网络通信状态
void cSerialScreenProtocol::SendFrameNetState(unsigned char ucState)
{
    FrameNetState_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FrameNetState_Screen)];
    pScreen = (FrameNetState_Screen*)pData;
    unsigned short Temp_Addr;
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameNetState_Screen) - 3);
    //串口通信状态 寄存器地址
    Temp_Addr = Reg_NetState_Screen;
    pScreen->usAddr = htons(Temp_Addr);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //网络通信状态
    if(ucState == 0)
    {
        pScreen->usNetState = htons(0x0001);
    }
    else if(ucState == 1)
    {
        pScreen->usNetState = htons(0x0005);
    }
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
}

//发送工作模式状态(本地应急模式, 其他模式)
void cSerialScreenProtocol::SendFrameCSCUWorkState(unsigned char ucState)
{
    _FrameCSCUWorkState_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(_FrameCSCUWorkState_Screen)];
    pScreen = (_FrameCSCUWorkState_Screen*)pData;
    unsigned short Temp_Addr;
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(_FrameCSCUWorkState_Screen) - 3);
    //串口通信状态 寄存器地址
    Temp_Addr = Reg_CSCUWorkState_Screen;
    pScreen->usAddr = htons(Temp_Addr);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //网络通信状态
    if(ucState == 0)
    {
        pScreen->usCSCUWorkState = htons(0x0000);
    }
    else if(ucState == 1)
    {
        pScreen->usCSCUWorkState = htons(0x0001);
    }
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
}

//发送 交直流终端数量----(系统参数设置界面, 重启后发送)
void cSerialScreenProtocol::SendFrameTermNum(ParamSetPage_Screen &stParamSetPage)
{
    FrameTermNum_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FrameTermNum_Screen)];
    pScreen = (FrameTermNum_Screen*)pData;
    unsigned short Temp_Addr;
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameTermNum_Screen) - 3);
    //交直流终端数量 寄存器地址
    Temp_Addr = Reg_TermNum_Screen;
    pScreen->usAddr = htons(Temp_Addr);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //终端个数
    pScreen->usACSinNum  = htons(stParamSetPage.usACSinNum);
    pScreen->usACThrNum  = htons(stParamSetPage.usACThrNum);
    pScreen->usDCNum  = htons(stParamSetPage.usDCNum);
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
}

//发送 本地IP和服务器端口号----系统参数设置界面
void cSerialScreenProtocol::SendFrameIPAndPort(ParamSetPage_Screen &stParamSetPage)
{
    FrameIPAndPort_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FrameIPAndPort_Screen)];
    pScreen = (FrameIPAndPort_Screen*)pData;
    unsigned short Temp_Addr;
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameIPAndPort_Screen) - 3);
    //本地IP和服务器端口号 寄存器地址
    Temp_Addr = Reg_IPAndPort_Screen;
    pScreen->usAddr = htons(Temp_Addr);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //本地IP和服务器端口号
    for(int i = 0; i < 4; i++)
    {
        pScreen->usLocolIp[i] = htons(stParamSetPage.usLocolIp[i]);
    }
    pScreen->usServerPort  = htons(stParamSetPage.usServerPort);
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
}

//发送 CSCU站地址----系统参数设置界面
void cSerialScreenProtocol::SendFrameStationAddr(ParamSetPage_Screen &stParamSetPage)
{
    FrameStationAddr_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FrameStationAddr_Screen)];
    pScreen = (FrameStationAddr_Screen*)pData;
    unsigned short Temp_Addr;
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameStationAddr_Screen) - 3);
    //CSCU站地址 寄存器地址
    Temp_Addr = Reg_StationAddr_Screen;
    pScreen->usAddr = htons(Temp_Addr);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //CSCU站地址
    memcpy(pScreen->chStationAddr, stParamSetPage.chStationAddr, sizeof(pScreen->chStationAddr));
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
}

//发送 域名----系统参数设置界面
void cSerialScreenProtocol::SendFrameDomainName(ParamSetPage_Screen &stParamSetPage)
{
    FrameDomainName_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FrameDomainName_Screen)];
    pScreen = (FrameDomainName_Screen*)pData;
    unsigned short Temp_Addr;
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameDomainName_Screen) - 3);
    //域名 寄存器地址
    Temp_Addr = Reg_DomainName_Screen;
    pScreen->usAddr = htons(Temp_Addr);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //域名
    memcpy(pScreen->chDomainName, stParamSetPage.chDomainName, sizeof(pScreen->chDomainName));
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
}

//发送 网关,DNS服务器,ZIGBEE地址----系统参数设置界面
void cSerialScreenProtocol::SendFrameGateWay(ParamSetPage_Screen &stParamSetPage)
{
    FrameGateWay_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FrameGateWay_Screen)];
    pScreen = (FrameGateWay_Screen*)pData;
    unsigned short Temp_Addr;
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameGateWay_Screen) - 3);
    //网关,DNS服务器,ZIGBEE地址  寄存器地址
    Temp_Addr = Reg_GateWay_Screen;
    pScreen->usAddr = htons(Temp_Addr);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //网关,DNS服务器,ZIGBEE地址
    for(int i = 0; i < 4; i++)
    {
        pScreen->usGateWay[i] = htons(stParamSetPage.usGateWay[i]);
        pScreen->usDNSServer[i] = htons(stParamSetPage.usDNSServer[i]);
    }
    pScreen->usZigBeeID = htons(stParamSetPage.usZigBeeID);
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
}

//发送 账户余额信息
void cSerialScreenProtocol::SendFrameAccountBalance(unsigned int uiAccountNum)
{
    FrameAccountBalance_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FrameAccountBalance_Screen)];
    pScreen = (FrameAccountBalance_Screen*)pData;
    unsigned short Temp_Addr;

    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameAccountBalance_Screen) - 3);
    //刷卡余额信息 寄存器地址
    Temp_Addr = Reg_AccountBalance_Screen;
    pScreen->usAddr = htons(Temp_Addr);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //刷卡余额赋值
    pScreen->uiBalance = htonl(uiAccountNum);

//    for(int i = 0; i < sizeof(* pScreen); i++)
//    {
//    }
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
}

//发送 刷卡等待信息----刷卡界面
//ucIconType: 1: 申请卡片信息; 2: 申请充电中; 3: 结束充电中
void cSerialScreenProtocol::SendFrameCardWait(unsigned char ucIconType)
{
    FrameCardWait_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FrameCardWait_Screen)];
    pScreen = (FrameCardWait_Screen*)pData;
    unsigned short Temp_Addr;

    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameCardWait_Screen) - 3);
    //刷卡余额信息 寄存器地址
    Temp_Addr = Reg_CardWaitIcon_Screen;
    pScreen->usAddr = htons(Temp_Addr);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //小图标赋值
    pScreen->usIcon = htons(ucIconType);

    //发送帧
    SendFrame(pData, sizeof(* pScreen));
}

//发送 刷卡结果----刷卡界面
void cSerialScreenProtocol::SendFrameCardResult(QByteArray arrayResult)
{
    //字符编码转换
    QTextCodec * pGBK = QTextCodec::codecForName("GBK");
    QString TempString;
    QByteArray retArray, retSendArray;
    //解帧
    unsigned short Temp_Addr;
    FrameCardResult_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FrameCardResult_Screen)];
    pScreen = (FrameCardResult_Screen*)pData;
    memset(pScreen->chResult, 0x00, sizeof(pScreen->chResult));
    //转换至GBK编码
    TempString = arrayResult;
    retArray = pGBK->fromUnicode(TempString);

    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameCardResult_Screen) - 3);
    //刷卡申请充电, 结束充电, 返回结果 寄存器地址
    Temp_Addr = Reg_CardResult_Screen;
    pScreen->usAddr = htons(Temp_Addr);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //刷卡申请充电/结束充电结果赋值
    GetFormatCardResult(retArray, pScreen->chResult, 64);
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
}

//发送 工程选择界面----当前工程类型
void cSerialScreenProtocol::SendFrameProType(unsigned char ucProType)
{
    FrameProType_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FrameProType_Screen)];
    pScreen = (FrameProType_Screen*)pData;
    unsigned short Temp_Addr;
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameProType_Screen) - 3);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //图标 寄存器地址
    Temp_Addr = Reg_ProSetIcon_Screen;
    pScreen->usAddr = htons(Temp_Addr);
    //发送类型
    switch(ucProType)
    {
    case 1: //普通版
        pScreen->usIconType = htons(0x0005);
        break;
    case 2: //刷卡版
        pScreen->usIconType = htons(0x0004);
        break;
    default:
        break;
    }
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
}

//发送 交流相别设置
void cSerialScreenProtocol::SendFrameACPhaseSet(stThreePhaseTypeConfig &ThreePhaseTypeConfig)
{

    FrameACPhaseSet_Screen * pScreen;
    unsigned char * pData = new unsigned char[sizeof(FrameACPhaseSet_Screen)];
    pScreen = (FrameACPhaseSet_Screen*)pData;
    unsigned short Temp_Addr;
    //生成帧头
    MakeFrameHead(pScreen->strHead, sizeof(FrameACPhaseSet_Screen) - 3);
    //交流相别 寄存器地址
    Temp_Addr = Reg_ACPhase_Screen;
    pScreen->usAddr = htons(Temp_Addr);
    //指令类型: 写数据寄存器
    pScreen->ucCmdType = Type_WriteMem_Screen;
    //交流相别
    memset(pScreen->usPhase, 0x00, sizeof(pScreen->usPhase));
    for(int i = 0; ((i < ThreePhaseTypeConfig.phaseTypeConfig.count()) && (i < 50)); i++)
    {
        pScreen->usPhase[i] = htons((unsigned short)(ThreePhaseTypeConfig.phaseTypeConfig.at(i).type));
    }
    //发送帧
    SendFrame(pData, sizeof(* pScreen));
}

//将刷卡结果打印规范化
void cSerialScreenProtocol::GetFormatCardResult(QByteArray &retArray, char * pDest, unsigned char ucLenth)
{
    unsigned char ucBlank = (ucLenth/2 - retArray.length())/2;
    memset(pDest, ' ', ucLenth);
    if(retArray.length() < ucLenth/2)
    {
        memcpy(pDest + ucBlank, retArray.data(), retArray.length());
    }
    else
    {
        memcpy(pDest, retArray.data(), retArray.length());
    }
}

void cSerialScreenProtocol::SendSpecGeneral()
{
	Frame_SpecGeneral_Screen *frame;
	stChargeConfig charge;
	stCSCUSysConfig	cscu;
	stSmartCarConfig car;

	frame = new Frame_SpecGeneral_Screen;
	memset(frame, 0, sizeof(Frame_SpecGeneral_Screen));

    MakeFrameHead(frame->strHead, sizeof(Frame_SpecGeneral_Screen) - 3);
    frame->cCmdType = Type_WriteMem_Screen;
	frame->sAddr = htons(Reg_SpecGeneral_Screen);

	pParamSet->querySetting(&charge, PARAM_CHARGE);
	frame->sVINOffline = htons(charge.vinOffline);
	frame->sLocalStop = htons(charge.localStop);
	frame->sVINAuto = htons(charge.vinAuto);
	frame->sCardAuto = htons(charge.cardAuto);
	frame->sEnergyFilter = htons(charge.energyFilter);
	frame->sEnergyFilter = htons(charge.energyFilter);
	frame->sLocalPolicy = htons(charge.localPolicy);
	frame->sCardType = htons(charge.cardType);
	frame->sVINType = htons(charge.vinType);

	pParamSet->querySetting(&car, PARAM_SMARTCAR);
	frame->sCarPrioty = htons(car.sSmartCar_Enable);

	pParamSet->querySetting(&cscu, PARAM_CSCU_SYS);
	frame->sBoardType = htons(cscu.boardType);

    SendFrame((uchar *)frame, sizeof(Frame_SpecGeneral_Screen));
}

void cSerialScreenProtocol::SendSpecGeneral1()
{
	Frame_SpecGeneral1_Screen *frame;
	stChargeConfig charge;
	stSmartChargeConfig smart;

	pParamSet->querySetting(&charge, PARAM_CHARGE);
	pParamSet->querySetting(&smart, PARAM_SMARTCHARGE);

	frame = new Frame_SpecGeneral1_Screen;
	memset(frame, 0, sizeof(Frame_SpecGeneral1_Screen));

    MakeFrameHead(frame->strHead, sizeof(Frame_SpecGeneral1_Screen) - 3);
    frame->cCmdType = Type_WriteMem_Screen;
	frame->sAddr = htons(Reg_SpecGeneral1_Screen);

	frame->sCuoFeng = htons(smart.sSmartCharge_Enable);
	frame->sFGPJ = htons(charge.fgpjEnable);

    SendFrame((uchar *)frame, sizeof(Frame_SpecGeneral1_Screen));
}

void cSerialScreenProtocol::SendSpecEmergency()
{
	Frame_SpecEmergency_Screen *frame;
	EmergencyConfig config;

	frame = new Frame_SpecEmergency_Screen; 
	memset(frame, 0, sizeof(Frame_SpecEmergency_Screen));

    MakeFrameHead(frame->strHead, sizeof(Frame_SpecEmergency_Screen) - 3);
    frame->cCmdType = Type_WriteMem_Screen;
	frame->sAddr = htons(Reg_SpecEmergency_Screen);

	pParamSet->querySetting(&config, PARAM_EMERGENCY);
	frame->sVinAuth = htons(config.vin_authenticate);
	frame->sQueueForGun = htons(config.queue_gun);
	frame->sCardAuth = htons(config.card_authenticate);
	frame->sQueueForCard = htons(config.queue_card);
	frame->sCarNoAuth = htons(config.car_authenticate);
	frame->sQueueForCar = htons(config.queue_car);

    SendFrame((uchar *)frame, sizeof(Frame_SpecEmergency_Screen));
}

void cSerialScreenProtocol::SendSpecCFCD(stAllTPFVConfig &config, uchar cPage)
{
	Frame_SpecCFCD_Screen *frame1, *frame2;
	int iFirst, iSecond;

	frame1 = new Frame_SpecCFCD_Screen;
	memset(frame1, 0, sizeof(Frame_SpecCFCD_Screen));
	frame2 = new Frame_SpecCFCD_Screen;
	memset(frame2, 0, sizeof(Frame_SpecCFCD_Screen));

    MakeFrameHead(frame1->strHead, sizeof(Frame_SpecCFCD_Screen) - 3);
    frame1->cCmdType = Type_WriteMem_Screen;
    MakeFrameHead(frame2->strHead, sizeof(Frame_SpecCFCD_Screen) - 3);
    frame2->cCmdType = Type_WriteMem_Screen;

    frame1->sAddr = htons(Reg_SpecCFCDFirst_Screen);
    frame2->sAddr = htons(Reg_SpecCFCDSecond_Screen);

	iFirst = (cPage - 1) * 10;
	iSecond = iFirst + 5; 

	for(int i = 0; i < 5 && (iFirst + i) < config.tpfvConfig.count(); i++){
        frame1->stRecord[i].sIndex = htons(iFirst + i + 1);
		frame1->stRecord[i].sStartH = htons((ushort)config.tpfvConfig.at(iFirst + i).start_hour);
		frame1->stRecord[i].sStartM = htons((ushort)config.tpfvConfig.at(iFirst + i).start_minute);
		frame1->stRecord[i].sStopH = htons((ushort)config.tpfvConfig.at(iFirst + i).stop_hour);
		frame1->stRecord[i].sStopM = htons((ushort)config.tpfvConfig.at(iFirst + i).stop_minute);
		frame1->stRecord[i].sSOC = htons((ushort)config.tpfvConfig.at(iFirst + i).limit_soc);
		frame1->stRecord[i].sCurrent = htons((ushort)config.tpfvConfig.at(iFirst + i).limit_current);
	}
    SendFrame((uchar *)frame1, sizeof(Frame_SpecCFCD_Screen));

	for(int i = 0; i < 5 && (iSecond + i) < config.tpfvConfig.count(); i++){
        frame2->stRecord[i].sIndex = htons(iSecond + i + 1);
		frame2->stRecord[i].sStartH = htons((ushort)config.tpfvConfig.at(iSecond + i).start_hour);
		frame2->stRecord[i].sStartM = htons((ushort)config.tpfvConfig.at(iSecond + i).start_minute);
		frame2->stRecord[i].sStopH = htons((ushort)config.tpfvConfig.at(iSecond + i).stop_hour);
		frame2->stRecord[i].sStopM = htons((ushort)config.tpfvConfig.at(iSecond + i).stop_minute);
		frame2->stRecord[i].sSOC = htons((ushort)config.tpfvConfig.at(iSecond + i).limit_soc);
		frame2->stRecord[i].sCurrent = htons((ushort)config.tpfvConfig.at(iSecond + i).limit_current);
	}
    SendFrame((uchar *)frame2, sizeof(Frame_SpecCFCD_Screen));
}

void cSerialScreenProtocol::SendSpecCFCDSet(stAllTPFVConfig &config, uchar cPage)
{
	Frame_SpecCFCDSet_Screen *frame;
	int iFirst;

	frame = new Frame_SpecCFCDSet_Screen;
	memset(frame, 0, sizeof(Frame_SpecCFCDSet_Screen));

    MakeFrameHead(frame->strHead, sizeof(Frame_SpecCFCDSet_Screen) - 3);
    frame->cCmdType = Type_WriteMem_Screen;
    frame->sAddr = htons(Reg_SpecCFCDSet_Screen);

	iFirst = (cPage - 1) * 5;

	for(int i = 0; i < 5 && (iFirst + i) < config.tpfvConfig.count(); i++){
		if(config.tpfvConfig.at(iFirst + i).start_hour == 0 &&
			config.tpfvConfig.at(iFirst + i).start_minute ==0 &&
			config.tpfvConfig.at(iFirst + i).stop_hour ==0 &&
			config.tpfvConfig.at(iFirst + i).stop_minute ==0)
			continue;
		frame->stRecord[i].sIndex = htons(iFirst + i + 1);
		frame->stRecord[i].sStartH = htons((ushort)config.tpfvConfig.at(iFirst + i).start_hour);
		frame->stRecord[i].sStartM = htons((ushort)config.tpfvConfig.at(iFirst + i).start_minute);
		frame->stRecord[i].sStopH = htons((ushort)config.tpfvConfig.at(iFirst + i).stop_hour);
		frame->stRecord[i].sStopM = htons((ushort)config.tpfvConfig.at(iFirst + i).stop_minute);
		frame->stRecord[i].sSOC = htons((ushort)config.tpfvConfig.at(iFirst + i).limit_soc);
		frame->stRecord[i].sCurrent = htons((ushort)config.tpfvConfig.at(iFirst + i).limit_current);
	}

    SendFrame((uchar *)frame, sizeof(Frame_SpecCFCDSet_Screen));
}

void cSerialScreenProtocol::SendSpecCFCDSwitch(stTPFVConfig *config, uchar cPage)
{
	Frame_SpecCFCDSet_Screen *frame;
	int iIndex;

	frame = new Frame_SpecCFCDSet_Screen;
	memset(frame, 0, sizeof(Frame_SpecCFCDSet_Screen));

    MakeFrameHead(frame->strHead, sizeof(Frame_SpecCFCDSet_Screen) - 3);
    frame->cCmdType = Type_WriteMem_Screen;
    frame->sAddr = htons(Reg_SpecCFCDSet_Screen);

	iIndex = (cPage - 1) * 5;

	for(int i = 0; i < 5; i++){
		if(config[iIndex + i].start_hour == 0 &&
			config[iIndex + i].start_minute ==0 &&
			config[iIndex + i].stop_hour ==0 &&
			config[iIndex + i].stop_minute ==0)
			continue;
		frame->stRecord[i].sIndex = htons(i + 1);
		frame->stRecord[i].sStartH = htons((ushort)config[iIndex + i].start_hour);
		frame->stRecord[i].sStartM = htons((ushort)config[iIndex + i].start_minute);
		frame->stRecord[i].sStopH = htons((ushort)config[iIndex + i].stop_hour);
		frame->stRecord[i].sStopM = htons((ushort)config[iIndex + i].stop_minute);
		frame->stRecord[i].sSOC = htons((ushort)config[iIndex + i].limit_soc);
		frame->stRecord[i].sCurrent = htons((ushort)config[iIndex + i].limit_current);
	}

    SendFrame((uchar *)frame, sizeof(Frame_SpecCFCDSet_Screen));
}

void cSerialScreenProtocol::SendSpecPolicy(AllLocalPolicyConfig &config, uchar cPage)
{
	Frame_SpecPolicy_Screen *frame1, *frame2;
	int iFirst, iSecond;

	if(cPage <= 0 || cPage > 2)
		return;

	frame1 = new Frame_SpecPolicy_Screen;
	memset(frame1, 0, sizeof(Frame_SpecPolicy_Screen));
    MakeFrameHead(frame1->strHead, sizeof(Frame_SpecPolicy_Screen) - 3);
    frame1->cCmdType = Type_WriteMem_Screen;
    frame1->sAddr = htons(Reg_SpecPolicyFirst_Screen);

	frame2 = new Frame_SpecPolicy_Screen;
	memset(frame2, 0, sizeof(Frame_SpecPolicy_Screen));
    MakeFrameHead(frame2->strHead, sizeof(Frame_SpecPolicy_Screen) - 3);
    frame2->cCmdType = Type_WriteMem_Screen;
    frame2->sAddr = htons(Reg_SpecPolicySecond_Screen);

	iFirst = (cPage - 1) * 10;
	iSecond = iFirst + 5; 

	for(int i = 0; i < 5 && (iFirst + i) < config.policyConfig.count(); i++){
        frame1->stPolicy[i].sIndex = htons(iFirst + i + 1);
		frame1->stPolicy[i].sStartH = htons(config.policyConfig[iFirst + i].start_hour);
		frame1->stPolicy[i].sStartM = htons(config.policyConfig[iFirst + i].start_minute);
		frame1->stPolicy[i].sStopH = htons(config.policyConfig[iFirst + i].stop_hour);
		frame1->stPolicy[i].sStopM = htons(config.policyConfig[iFirst + i].stop_minute);
		frame1->stPolicy[i].sElectricFee = htons(config.policyConfig[iFirst + i].electric_fee);
		frame1->stPolicy[i].sServiceFee = htons(config.policyConfig[iFirst + i].service_fee);
	}
    SendFrame((uchar *)frame1, sizeof(Frame_SpecPolicy_Screen));

	for(int i = 0; i < 5 && (iSecond + i) < config.policyConfig.count(); i++){
        frame2->stPolicy[i].sIndex = htons(iSecond + i + 1);
		frame2->stPolicy[i].sStartH = htons(config.policyConfig[iSecond + i].start_hour);
		frame2->stPolicy[i].sStartM = htons(config.policyConfig[iSecond + i].start_minute);
		frame2->stPolicy[i].sStopH = htons(config.policyConfig[iSecond + i].stop_hour);
		frame2->stPolicy[i].sStopM = htons(config.policyConfig[iSecond + i].stop_minute);
		frame2->stPolicy[i].sElectricFee = htons(config.policyConfig[iSecond + i].electric_fee);
		frame2->stPolicy[i].sServiceFee = htons(config.policyConfig[iSecond + i].service_fee);
	}
    SendFrame((uchar *)frame2, sizeof(Frame_SpecPolicy_Screen));
}

void cSerialScreenProtocol::SendSpecPolicySet(AllLocalPolicyConfig &config, uchar cPage)
{
	Frame_SpecPolicySet_Screen *frame;
	int iFirst;

	frame = new Frame_SpecPolicySet_Screen;
	memset(frame, 0, sizeof(Frame_SpecPolicySet_Screen));
    MakeFrameHead(frame->strHead, sizeof(Frame_SpecPolicySet_Screen) - 3);
    frame->cCmdType = Type_WriteMem_Screen;
    frame->sAddr = htons(Reg_SpecPolicySet_Screen);

	iFirst = (cPage - 1) * 5;

	for(int i = iFirst; i < (iFirst + 5) && i < config.policyConfig.count(); i++){
        frame->stPolicy[i].sIndex = htons(i + 1);
		frame->stPolicy[i].sStartH = htons(config.policyConfig[i].start_hour);
		frame->stPolicy[i].sStartM = htons(config.policyConfig[i].start_minute);
		frame->stPolicy[i].sStopH = htons(config.policyConfig[i].stop_hour);
		frame->stPolicy[i].sStopM = htons(config.policyConfig[i].stop_minute);
		frame->stPolicy[i].sElectricFee = htons(config.policyConfig[i].electric_fee);
		frame->stPolicy[i].sServiceFee = htons(config.policyConfig[i].service_fee);
	}

    SendFrame((uchar *)frame, sizeof(Frame_SpecPolicySet_Screen));
}

void cSerialScreenProtocol::SendSpecPolicySwitch(LocalPolicyConfig *config, uchar cPage)
{
	Frame_SpecPolicySet_Screen *frame;
	int iIndex;

	if(cPage <= 0 || cPage > 4)
		return;

	frame = new Frame_SpecPolicySet_Screen;

	memset(frame, 0, sizeof(Frame_SpecPolicySet_Screen));
    MakeFrameHead(frame->strHead, sizeof(Frame_SpecPolicySet_Screen) - 3);
    frame->cCmdType = Type_WriteMem_Screen;
    frame->sAddr = htons(Reg_SpecPolicySet_Screen);

	iIndex = (cPage - 1) * 5;

	for(int i = 0; i < 5; i++){
        frame->stPolicy[i].sIndex = htons((ushort)config[iIndex + i].policy_index);
		frame->stPolicy[i].sStartH = htons((ushort)config[iIndex + i].start_hour);
		frame->stPolicy[i].sStartM = htons((ushort)config[iIndex + i].start_minute);
		frame->stPolicy[i].sStopH = htons((ushort)config[iIndex + i].stop_hour);
		frame->stPolicy[i].sStopM = htons((ushort)config[iIndex + i].stop_minute);
		frame->stPolicy[i].sElectricFee = htons((ushort)config[iIndex + i].electric_fee);
		frame->stPolicy[i].sServiceFee = htons((ushort)config[iIndex + i].service_fee);
	}

    SendFrame((uchar *)frame, sizeof(Frame_SpecPolicySet_Screen));
}

void cSerialScreenProtocol::SendSpecFGPJ(AllFGPJConfig &config, uchar cPage)
{
	Frame_SpecFGPJ5_Screen *frame1, *frame2;
	int iFirst, iSecond;

	if(cPage <= 0 || cPage > 4)
		return;

	frame1 = new Frame_SpecFGPJ5_Screen;
	memset(frame1, 0, sizeof(Frame_SpecFGPJ5_Screen));
    MakeFrameHead(frame1->strHead, sizeof(Frame_SpecFGPJ5_Screen) - 3);
    frame1->cCmdType = Type_WriteMem_Screen;
    frame1->sAddr = htons(Reg_SpecFGPJFirst_Screen);

	frame2 = new Frame_SpecFGPJ5_Screen;
	memset(frame2, 0, sizeof(Frame_SpecFGPJ5_Screen));
    MakeFrameHead(frame2->strHead, sizeof(Frame_SpecFGPJ5_Screen) - 3);
    frame2->cCmdType = Type_WriteMem_Screen;
    frame2->sAddr = htons(Reg_SpecFGPJSecond_Screen);

	iFirst = (cPage - 1) * 10;
	iSecond = iFirst + 5; 

	for(int i = 0; i < 5 && (iFirst + i) < config.fgpjConfig.count(); i++){
        frame1->stSegment[i].sIndex = htons(iFirst + i + 1);
		frame1->stSegment[i].sSegment = htons(config.fgpjConfig.at(iFirst + i).time_seg);
		frame1->stSegment[i].sStartH = htons(config.fgpjConfig.at(iFirst + i).start_hour);
		frame1->stSegment[i].sStartM = htons(config.fgpjConfig.at(iFirst + i).start_minute);
		frame1->stSegment[i].sStopH = htons(config.fgpjConfig.at(iFirst + i).stop_hour);
		frame1->stSegment[i].sStopM = htons(config.fgpjConfig.at(iFirst + i).stop_minute);
	}
	SendFrame((uchar *)frame1, sizeof(Frame_SpecFGPJ5_Screen));

	for(int i = 0; i < 5 && (iSecond + i) < config.fgpjConfig.count(); i++){
        frame2->stSegment[i].sIndex = htons(iSecond + i + 1);
		frame2->stSegment[i].sSegment = htons(config.fgpjConfig.at(iSecond + i).time_seg);
		frame2->stSegment[i].sStartH = htons(config.fgpjConfig.at(iSecond + i).start_hour);
		frame2->stSegment[i].sStartM = htons(config.fgpjConfig.at(iSecond + i).start_minute);
		frame2->stSegment[i].sStopH = htons(config.fgpjConfig.at(iSecond + i).stop_hour);
		frame2->stSegment[i].sStopM = htons(config.fgpjConfig.at(iSecond + i).stop_minute);
	}
    SendFrame((uchar *)frame2, sizeof(Frame_SpecFGPJ5_Screen));
}

void cSerialScreenProtocol::SendSpecFGPJSet(FGPJConfig *config, uchar cPage)
{
	Frame_SpecFGPJ8_Screen *frame;
	int iIndex = 0;

	frame = new Frame_SpecFGPJ8_Screen;
	memset(frame, 0, sizeof(Frame_SpecFGPJ8_Screen));
    MakeFrameHead(frame->strHead, sizeof(Frame_SpecFGPJ8_Screen) - 3);
    frame->cCmdType = Type_WriteMem_Screen;
    frame->sAddr = htons(Reg_SpecFGPJSet_Screen);

	for(int i = 0; i < 32; i++){
		if(cPage != config[i].time_seg)
			continue;
		if(config[i].start_hour == 0 &&
			config[i].start_minute ==0 &&
			config[i].stop_hour ==0 &&
			config[i].stop_minute ==0)
			continue;
		frame->stSegment[iIndex].sIndex = htons(iIndex + 1);
		frame->stSegment[iIndex].sStartH = htons(config[i].start_hour);
		frame->stSegment[iIndex].sStartM = htons(config[i].start_minute);
		frame->stSegment[iIndex].sStopH = htons(config[i].stop_hour);
		frame->stSegment[iIndex].sStopM = htons(config[i].stop_minute);
		iIndex++;
	}

    SendFrame((uchar *)frame, sizeof(Frame_SpecFGPJ8_Screen));
}

void cSerialScreenProtocol::SendSpecDoubleGun()
{
    Frame_CoupleGun_Screen *couplegun;
    stChargeConfig charge;

    couplegun = new Frame_CoupleGun_Screen;
    memset(couplegun, 0, sizeof(Frame_CoupleGun_Screen));

    MakeFrameHead(couplegun->strHead, sizeof(Frame_CoupleGun_Screen) - 3);
    couplegun->cCmdType = Type_WriteMem_Screen;
    couplegun->sAddr = htons(Reg_SpecCoupleGun_Screen);

    pParamSet->querySetting(&charge, PARAM_CHARGE);
    if( htons(charge.coupleGun))
    {
        couplegun->coupleGun =htons(1);
    }else
    {
         couplegun->coupleGun = htons(charge.coupleGun);
    }
    couplegun->chargemoudle = htons(charge.coupleGun);

    SendFrame((uchar *)couplegun, sizeof(Frame_CoupleGun_Screen));

}
