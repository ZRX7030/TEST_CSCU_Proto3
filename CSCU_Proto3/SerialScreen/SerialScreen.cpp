#include "SerialScreen.h"
#include "RealDataFilter/RealDataFilter.h"

cSerialRead::cSerialRead(cSerialPort *pSerialPortIn)
{
    uiRecvBufSize = 200;
    pSerialPort = pSerialPortIn;
    bEndWork = FALSE;
    pSerialData = new unsigned char[uiRecvBufSize];
}
cSerialRead::~cSerialRead()
{
    delete pSerialData;
}

//读取串口数据
void cSerialRead::ReadSerialData()
{
    int iDataNum = 0;
    while(!bEndWork)
    {
        //每次读取数据时间间隔100ms
        iDataNum = pSerialPort->Read(pSerialData,uiRecvBufSize, 100);

        if(!bEndWork && iDataNum >= 0)
        {
            emit sigRecvSerialData(pSerialData,iDataNum);
            pSerialData = new unsigned char[uiRecvBufSize];
        }
   	}
}

void cSerialRead::ProcStartWork()
{
    ReadSerialData();
}

cSerialWrite::cSerialWrite(cSerialPort *pSerialPortIn)
{
    pSerialPort = pSerialPortIn;
}

void cSerialWrite::ProcSendSerialData(unsigned char *pSerialData, int iLen)
{
    pSerialPort->Write(pSerialData, iLen);
    if(pSerialData)
    {
        delete pSerialData;
    }
}

cSerialScreen::cSerialScreen()
{
	_strLogName = "screen";
    bWorkStartFlag = FALSE;
    bCardResultFlag = FALSE;
    bVinResultFlag = FALSE;
    ucVINResultStep = 0;
    bPageWaitFlag = FALSE;
    bBanlanceFlag = FALSE;
    bScreenRebootFlag = FALSE;
    pDevCache = NULL;
    iPageWaitCount = 0;
    ucDCSpecAckStep = 0;//直流特殊功能回复设置
    iPageLoadDispatchChoose = 0; // //当前负荷调度界面标志位
    //bTicketPrint[250] = {false};  //小票机申请一次标志位

    //错峰设置数据接收
    memset((unsigned char *)&ServerConfig, 0x00, sizeof(ServerConfig));
    memset((unsigned char *)&NetConfig, 0x00, sizeof(NetConfig));
    memset((unsigned char *)&cscuSysConfig, 0x00, sizeof(cscuSysConfig));
    memset((unsigned char *)&ChargeConfig, 0x00, sizeof(ChargeConfig));
    memset((unsigned char *)&IOConfig, 0x00, sizeof(IOConfig));
    memset((unsigned char *)&PowerLimitConfig, 0x00, sizeof(PowerLimitConfig));
    memset((unsigned char *)&PeakSet, 0x00, sizeof(PeakSet));

    //刷卡卡号
    CardNum.clear();

    pDevCache = DevCache::GetInstance();
    pDBOperate = DBOperate::GetInstance();
    pParamSet = ParamSet::GetInstance();
    pLog = Log::GetInstance();

    QueryParamInfo();
}

cSerialScreen::~cSerialScreen()
{
	if(pSerialReadThread){
		pSerialRead->bEndWork = TRUE;
		pSerialReadThread->quit();
		pSerialReadThread->wait();
		delete pSerialReadThread ;//串口读线程指针
		pSerialReadThread = NULL;
	}
	if(pSerialRead)
		delete pSerialRead;//读取串口操作类
	if(pSerialWrite)
		delete pSerialWrite;//写入串口操作类
	if(pSerialPort)
		delete pSerialPort;//对应串口类
	if(pSecTimer)
		delete pSecTimer;//1秒计数器
	if(pMSecTimer)
		delete pMSecTimer;	//50ms计数器
	if(pProtocol)
		delete pProtocol;	//串口屏协议指针
}

//根据配置选项初始化
int cSerialScreen::InitModule( QThread* pThread)
{
    m_pWorkThread = pThread;

    this->moveToThread(m_pWorkThread);
    QObject::connect(m_pWorkThread, SIGNAL(started()), this, SLOT(ProcStartWork()));

    return 0;
}

//注册设备到总线
int cSerialScreen::RegistModule()
{
	QList<int> List;//
	List.append(AddrType_TermIndex_QueryFinish);//结束始获取子站名称，CAN地址和终端编号对应关系。服务器发布，显示屏订阅
	List.append(AddrType_Udisk_Insert);//检测到U盘插入
	List.append(AddrType_UpdateResult);//升级结果+日志上传结果
	List.append(AddrType_CenterReadCard);//主题二：集中读卡卡号, 刷卡主界面, 刷卡终端详情界面
	List.append(AddrType_ApplyAccountInfoResult_ToScreen);//主题六：充电服务返回账户信息给显示屏
	List.append(AddrType_InApplyStartChargeResult_ToScreen);//主题十：内部申请开始充电结果至显示屏
	List.append(AddrType_OutApplyStartChargeResult_ToScreen);//主题十四：远程申请开始充电结果至显示屏
	List.append(AddrType_InApplyStopChargeResult_ToScreen);//主题十八：内部申请结束充电结果至显示屏
	List.append(AddrType_OutApplyStopChargeResult_ToScreen);//主题二十二：远程申请结束充电结果至显示屏
	List.append(AddrType_SmartChargeSet_Result);//本地设置错峰充电参数设置结果
	List.append(AddrType_VINViaScreenApplyCharge_Result);//主题二：VIN后6位申请开始充电返回结果
	List.append(AddrType_GeneralDynamicArgRenewAck_DB);//通用动态参数更新数据库回复
	List.append(AddrType_GeneralStaticArgRenewAck_DB);//通用静态参数更新数据库回复
	List.append(AddrType_MakePrintTicketResult);   //执行打印小票结果, 信息体：CAN地址 ,打印结果 发出方：小票机, 订阅方：显示屏
	List.append(AddrType_ApplyPrintTicket);  //申请打印小票一次
	List.append(AddrType_VinApplyStartCharge_Result);//单双枪使能，单枪点屏vin充电
	List.append(AddrType_Change_ChargeGunGroup_Info);//多枪分组信息改变，屏幕分组信息调整
	List.append(AddrType_InVinApplyStartCharge_Result);   //VIN申请开始充电结果

	CBus::GetInstance()->RegistDev(this, List);
	return 0;
}

int cSerialScreen::StartModule()
{
    m_pWorkThread->start();

    return 0;
}

//停止模块
int cSerialScreen::StopModule()
{
	if(bWorkStartFlag){
		bWorkStartFlag = FALSE;
		pSerialRead->bEndWork = TRUE;
	}

    return 0;
}

//模块工作状态
int cSerialScreen::ModuleStatus()
{
    return 0;
}

//初始化
void cSerialScreen::Init()
{
    bool bSerialOK = TRUE;
    pSerialPort = new cSerialPort(); //对应串口类
    pSerialReadThread = new QThread();//串口读线程指针
    pSecTimer = new QTimer();//1秒计数器
    pMSecTimer = new QTimer();//50ms计数器
    pSerialRead = new cSerialRead(pSerialPort);//读取串口操作类
    pSerialWrite = new cSerialWrite(pSerialPort);//写入串口操作类
    pProtocol = new cSerialScreenProtocol(pDevCache, pDBOperate, pParamSet, pLog);//串口屏协议指针
    //校验不通过
//    if(!CheckDBTermName())
//    {
//        InitTermNameDB();
//    }
    //初始化终端名称图
    InitTermNameMap();
    InitTermNameMapShow();
    InitTermNameMapMulti();
    //初始化为主界面
    ScreenState.us_page_num = MENU_PAGE_MAIN;
    ScreenState.st_page_value.uc_page_num_return = MENU_PAGE_MAIN;
    ScreenState.st_page_value.uc_hold_time = 0;
    ScreenState.st_page_value.uiPageTimeOut = 0xFFFFFF;
    iPageSwitchCount = 0; //页面切换计数器初始化为0
    iSetTimeCount = 0; //对时计数器
    ucCardStep = 0;//刷卡阶段初始化为0

    QueryParamInfo();
    //串口打开并初始化
    switch(cscuSysConfig.boardType)
    {
//    case 1:
//        pSerialPort->Open(SCREEN_SERIAL_NUM_1);
//        break;
    case 2:
        pSerialPort->Open(SCREEN_SERIAL_NUM_2);
        break;
    case 3:
        bSerialOK = pSerialPort->Open(SCREEN_SERIAL_NUM_3);
        break;
    default:
        pSerialPort->Open(SCREEN_SERIAL_NUM_2);
        break;
    }
    //串口打开失败, 不执行串口屏模块
    if(bSerialOK == FALSE)
    {
        StopModule();
        return;
    }
    pSerialPort->SetParity(8, 'N', 1);
    pSerialPort->SetSpeed(115200);
    //接收线程初始化
    pSerialRead->moveToThread(pSerialReadThread);
    connect(pSerialReadThread, SIGNAL(started()), pSerialRead, SLOT(ProcStartWork()));
    //关联定时器 ---- 1s
    connect(pSecTimer, SIGNAL(timeout()), this, SLOT(ProcOneSecTimeOut()));
    //关联协议和串口发送类
    connect(pProtocol, SIGNAL(sigSendSerialData(unsigned char*,int)), pSerialWrite, SLOT(ProcSendSerialData(unsigned char*,int)));
    //关联自身和串口接收类
    connect(pSerialRead, SIGNAL(sigRecvSerialData(unsigned char*,int)), this, SLOT(ProcRecvSerialData(unsigned char*,int)));
    //定时器启动,1s
    pSecTimer->start(1000);
    pSerialReadThread->start();
}

//初始化终端名称图(写TermNameMap)
void cSerialScreen::InitTermNameMap()
{
    //数据库操作----查询终端名称表,并赋值到TermNameMap
    db_result_st dbst;
    int iTermID = 0;
    char * pName = NULL;
    QByteArray tmpArray;
    QString todo = "SELECT canaddr, name FROM terminal_name_table";
    if(0 != pDBOperate->DBSqlQuery(todo.toAscii().data(), &dbst, DB_PARAM))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! InitTermNameMap Query Error");
        return;
    }
    //终端名称赋值
    for(int i = 0; i < dbst.row; i++)
    {
        iTermID = atoi(dbst.result[i * dbst.column]);
        pName = dbst.result[i * dbst.column + 1];
        tmpArray.clear();
        tmpArray.append(pName);
        NameMap.insert((unsigned char )iTermID, tmpArray);
    }
    //NameMapShow = NameMap;
    pDBOperate->DBQueryFree(&dbst);
}
//初始化终端名称图(写TermNameMap)
void cSerialScreen::InitTermNameMapShow()
{
    //数据库操作----查询终端名称表,并赋值到TermNameMap
    db_result_st dbst;
    int iTermID = 0;
    char * pName = NULL;
    QByteArray tmpArray;
    QString todo = "SELECT canaddr, name FROM terminal_name_show_table";
    if(0 != pDBOperate->DBSqlQuery(todo.toAscii().data(), &dbst, DB_PARAM))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! InitTermNameMap Query Error");
        NameMapShow = NameMap;
        return;
    }
    if(dbst.row<1)
    {
        NameMapShow = NameMap;
    }else
    {
        NameMapShow.clear();
        //终端名称赋值
        for(int i = 0; i < dbst.row; i++)
        {
            iTermID = atoi(dbst.result[i * dbst.column]);
            pName = dbst.result[i * dbst.column + 1];
            tmpArray.clear();
            tmpArray.append(pName);
            NameMapShow.insert((unsigned char )iTermID, tmpArray);
        }
    }
    pDBOperate->DBQueryFree(&dbst);
}

//初始化终端名称图(写TermNameMap)
void cSerialScreen::InitTermNameMapMulti()
{
    //数据库操作----查询终端名称表,并赋值到TermNameMap
    db_result_st dbst;
    int iTermID = 0;
    char * pName = NULL;
    QByteArray tmpArray;
    QString todo = "SELECT canaddr, name FROM terminal_name_multi_table";
    if(0 != pDBOperate->DBSqlQuery(todo.toAscii().data(), &dbst, DB_PARAM))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! InitTermNameMap Query Error");
        NameMapShow = NameMap;
        return;
    }
    if(dbst.row<1)
    {
        NameMapShow = NameMap;
    }else
    {
        NameMapShow.clear();
        //终端名称赋值
        for(int i = 0; i < dbst.row; i++)
        {
            iTermID = atoi(dbst.result[i * dbst.column]);
            pName = dbst.result[i * dbst.column + 1];
            tmpArray.clear();
            tmpArray.append(pName);
            NameMapShow.insert((unsigned char )iTermID, tmpArray);
        }
    }
    pDBOperate->DBQueryFree(&dbst);
}

//校验帧头
bool cSerialScreen::CheckFrameHead(unsigned char * pData, int iLength)
{
    if((pData[0] != 0x5A)||(pData[1] != 0xA5))
    {
        pLog->getLogPoint(_strLogName)->info("Screen Recv Serial Data Format ERROR");
        return FALSE;
    }
    if(iLength != pData[2] + 3)
    {
        pLog->getLogPoint(_strLogName)->info("Screen Recv Serial Data Length ERROR");
        return FALSE;
    }
    return TRUE;
}

//检查串口屏工作状态
void cSerialScreen::CheckWorkState()
{
    switch(ScreenState.us_page_num)
    {
    case MENU_PAGE_MAIN: //主界面
        pProtocol->SendMainPageData(NameMap, cscuSysConfig, cscuSysConfig.version,NameMapShow);
        iPageSwitchCount = 0;
        break;
    case MENU_PAGE_TERM_INFO_NORMAL: //终端-详细信息
        pProtocol->SendTermPageData(ScreenState.st_page_value.uc_can_addr, 1, ThreePhaseTypeConfig);
        break;
    case MENU_PAGE_TERM_INFO_CARD: //终端-充电中信息
        pProtocol->SendTermPageChargingData(ScreenState.st_page_value.uc_can_addr, (unsigned char)cscuSysConfig.normalCardType, ThreePhaseTypeConfig);
        break;
    case MENU_PAGE_BMS: //页面09H，终端BMS信息
        pProtocol->SendTermBMSInfo(ScreenState.st_page_value.uc_can_addr);
        break;
    case MENU_PAGE_SWIPE_CARD_MAIN: //刷卡主页面 "请刷卡,获取余额信息"
    {
        TerminalStatus stStatus;
        pDevCache->QueryTerminalStatus(ScreenState.st_page_value.uc_can_addr, stStatus);
        if(stStatus.cStatus == CHARGE_STATUS_STARTING) //启动中
        {
            if(bCardResultFlag == FALSE)
            {
                unsigned char ucResult = 252;
                QByteArray retArray;
                //将结果显示到屏幕
                retArray = GetCardResult(ucResult, 2);  //约定该状态及显示
                pProtocol->SendSwitchPage(MENU_PAGE_CARD_RESULT);
                pProtocol->SendFrameCardResult(retArray);
                pProtocol->SendPageCount(TO_CardStartStopResult_Screen);
                //超时时间置为3s
                ScreenState.st_page_value.uiPageTimeOut = TO_CardStartStopResult_Screen;
                iPageSwitchCount = 0;
                bCardResultFlag = TRUE;
            }
        }
        break;
    }
    default:
        break;
    }
}

//检查串口屏页面等待切换
void cSerialScreen::CheckPageWaitTime()
{
    if(bPageWaitFlag == TRUE)
    {
        switch(ScreenState.us_page_num)
        {
        case MENU_PAGE_SET_SPEC_FUNC:   //特殊功能设置页面, 提示后返回
        case MENU_PAGE_IN_LINE_INFO:    //进线侧信息页面, 提示后返回
//        case MENU_PAGE_PASSWD_INPUT:    //密码输入界面, 提示后返回     2017-01-19 br zrx
        case MENU_PAGE_SET_SYSTEM: //系统参数设置页面
        case MENU_PAGE_LOAD_DISPATCH_CHOOSE:    //负荷调度选择界面
            if(iPageSwitchCount >= iPageWaitTime)
            {
                Ctrl_SwitchParamMain(0);
                pProtocol->SendSwitchPage(MENU_PAGE_SET_MAIN);

                pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
                iPageSwitchCount = 0;
                bPageWaitFlag = FALSE;
            }
            break;
        case MENU_PAGE_VIN_START_CHARGE: //输入VIN后6位, 开始充电界面 ----普通版专有
            if(iPageSwitchCount >= iPageWaitTime)
            {
                Ctrl_SwitchMain(0);
                pProtocol->SendSwitchPage(MENU_PAGE_MAIN);
                pProtocol->SendMainPageData(NameMap, cscuSysConfig, cscuSysConfig.version,NameMapShow);
                bPageWaitFlag = FALSE;
            }
            break;
        default:
            bPageWaitFlag = FALSE;
            break;
        }//end switch
    }//end if
}

//检查串口屏页面超时
void cSerialScreen::CheckPageTimeOut()
{
    //超时返回
    if((unsigned int)iPageSwitchCount >= ScreenState.st_page_value.uiPageTimeOut)
    {
        iPageSwitchCount = 0;
        QByteArray tempArray = "平台无响应, 超时返回";
        //界面切换
        switch(ScreenState.us_page_num)
        {
        case MENU_PAGE_MAIN: //主界面
            break;
        case MENU_PAGE_VIN_START_CHARGE: //输入VIN后6位, 开始充电界面 ----普通版专有
            if(ucVINResultStep == 0)
            {
                ucVINResultStep = 2;
                pProtocol->SendSwitchPage(MENU_PAGE_VIN_APPLY_START_RESULT);
                pProtocol->SendVINResult(ICON_VIN_CHECK_ERROR);
                pProtocol->SendPageCount(TO_VINResult_Screen);
                ScreenState.st_page_value.uiPageTimeOut = TO_VINResult_Screen;
            }
            if(ucVINResultStep == 1)
            {
                ucVINResultStep = 2;
                pProtocol->SendSwitchPage(MENU_PAGE_VIN_APPLY_START_RESULT);
                pProtocol->SendVINResult(ICON_VIN_CHARGE_FAILED);
                pProtocol->SendPageCount(TO_VINResult_Screen);
                ScreenState.st_page_value.uiPageTimeOut = TO_VINResult_Screen;
            }
            else
            {
                Ctrl_SwitchMain(0x00);
                pProtocol->SendSwitchPage(MENU_PAGE_MAIN);
                pProtocol->SendMainPageData(NameMap, cscuSysConfig, cscuSysConfig.version,NameMapShow);
            }
            break;
        case MENU_PAGE_SWIPE_CARD_MAIN: //页面6EH, 刷卡主页面
        case MENU_PAGE_TERM_INFO_CARD:// 终端详情, 刷卡版
            if(bCardResultFlag == FALSE)
            {
                pProtocol->SendSwitchPage(MENU_PAGE_CARD_RESULT);
                pProtocol->SendFrameCardResult(tempArray);
                pProtocol->SendPageCount(TO_CardResult_Screen);
                ScreenState.st_page_value.uiPageTimeOut = TO_CardResult_Screen;
                bCardResultFlag = TRUE;
            }
            else
            {
                Ctrl_SwitchMain(0x00);
                pProtocol->SendSwitchPage(MENU_PAGE_MAIN);
                pProtocol->SendMainPageData(NameMap, cscuSysConfig, cscuSysConfig.version,NameMapShow);
            }
            break;
        case MENU_PAGE_VIN_CARD_CHARGE_NORMAL:
        case MENU_PAGE_VIN_MANUAL_CHARGE:
            if(bVinResultFlag == FALSE)
            {
                pProtocol->SendSwitchPage(MENU_PAGE_CARD_RESULT);
                pProtocol->SendFrameCardResult(tempArray);
                pProtocol->SendPageCount(TO_CardResult_Screen);
                ScreenState.st_page_value.uiPageTimeOut = TO_CardResult_Screen;
                bVinResultFlag = TRUE;
            }
            else
            {
                Ctrl_SwitchMain(0x00);
                pProtocol->SendSwitchPage(MENU_PAGE_MAIN);
                pProtocol->SendMainPageData(NameMap, cscuSysConfig, cscuSysConfig.version,NameMapShow);
            }
            break;
        case MENU_PAGE_SET_MAIN: //页面04H，设置主页面
        case MENU_PAGE_SET_PRO: //工程选择主页面
        case MENU_PAGE_SET_SYSTEM:  //系统参数设置界面
        case MENU_PAGE_SET_PHASE:   //交流相别设置页面
        //case MENU_PAGE_PASSWD_INPUT:    //密码输入页面
        case MENU_PAGE_SET_SPEC_FUNC:   //特殊功能设置页面
        case MENU_PAGE_IN_LINE_INFO:    //进线侧信息页面
        case MENU_PAGE_ENV_INFO:    //子站环境信息页面
        case MENU_PAGE_DC_SPEC:     //直流特殊功能界面
        case MENU_PAGE_LOAD_DISPATCH_CHOOSE:    //负荷调度选择界面
        case MENU_PAGE_FAULT_INFO:  //故障信息列表页面
        case MENU_PAGE_TERM_INFO_NORMAL: //终端详情, 普通版
        case MENU_PAGE_UDISK: //页面0x2AH, U盘处理
        case MENU_PAGE_BMS: //页面09H, BMS详情页面
        case MENU_PAGE_CHARGE_CARD_REPORT:  //充电报告界面----刷卡版
        case MENU_PAGE_CHARGE_NORMAL_REPORT:  //充电报告界面----普通版
        case MENU_PAGE_PEAK_STAGGER: // 错峰充电显示页面
        case MENU_PAGE_LOAD_LIMIT: // 负荷约束电设置页面
        case MENU_PAGE_TICKET_PRINT_MAIN: //小票机打印提示主界面
        case MENU_PAGE_TICKET_NOPAPER:   //缺纸提示
            Ctrl_SwitchMain(0x00);
            pProtocol->SendSwitchPage(MENU_PAGE_MAIN);
            pProtocol->SendMainPageData(NameMap, cscuSysConfig, cscuSysConfig.version,NameMapShow);
            break;
		case MENU_PAGE_SPEC_GENERAL:
		case MENU_PAGE_SPEC_EMERGENCY:
		case MENU_PAGE_SPEC_CFCD:
		case MENU_PAGE_SPEC_POLICY:
		case MENU_PAGE_SPEC_FGPJ:
		case MENU_PAGE_SPEC_DOUBLE_GUN:
			if(ScreenState.st_page_value.uc_page_num_return > 0)
				Ctrl_SwitchPage(ScreenState.st_page_value.uc_page_num_return, TO_SpecFuncSet_Screen);
			else
				Ctrl_SwitchPage(MENU_PAGE_SET_MAIN, TO_ParamMain_Screen);
			break;
		default:
			Ctrl_SwitchMain(0x00);
            pProtocol->SendSwitchPage(MENU_PAGE_MAIN);
            pProtocol->SendMainPageData(NameMap, cscuSysConfig, cscuSysConfig.version,NameMapShow);
            break;
        }
    }
}

//系统参数比较, 返回FALSE, 无变化；返回TRUE, 有变化
bool cSerialScreen::SysParamCmp(ParamSetPage_Screen & stNew)
{
    unParamConfig unParm;
    bool bRet = FALSE;
//    int iMinSize = 0;
    QByteArray TempArr;
    QString TempStr;
    ParamSetPage_Screen stOld = GetSysParamSet();
    //校验Sever配置 ---- 站地址修改(重启)
    if(memcmp(stOld.chStationAddr, stNew.chStationAddr, sizeof(stOld.chStationAddr)))
    {
        memset(ServerConfig.stationNo, 0x00, sizeof(ServerConfig.stationNo));
        strcpy(ServerConfig.stationNo, stNew.chStationAddr);
        unParm.server0Config = ServerConfig;
        pParamSet->updateSetting(&unParm, PARAM_SERVER0);
        bRet = TRUE;
        bScreenRebootFlag = TRUE;
    }

    //校验cscu配置(重启)
    if( (stOld.usACSinNum != stNew.usACSinNum)
            ||(stOld.usACThrNum != stNew.usACThrNum)
            ||(stOld.usDCNum != stNew.usDCNum)
            ||memcmp(stOld.usDNSServer, stNew.usDNSServer, sizeof(stNew.usDNSServer)) )
    {
        cscuSysConfig.singlePhase = stNew.usACSinNum;
        cscuSysConfig.threePhase = stNew.usACThrNum;
        cscuSysConfig.directCurrent = stNew.usDCNum;
        memset(cscuSysConfig.dns,0x0,sizeof(cscuSysConfig.dns));
        for(int i = 0; i < 4; i++)
        {
            TempStr = QString::number(stNew.usDNSServer[i]) + ".";
            TempArr.append(TempStr);
        }
        TempArr.remove(TempArr.length() -1 , 1);
        strcpy(cscuSysConfig.dns, TempArr.data());
        bRet = TRUE;
        bScreenRebootFlag = TRUE;
        unParm.cscuSysConfig = cscuSysConfig;
        pParamSet->updateSetting(&unParm, PARAM_CSCU_SYS);
    }
    //校验Net0配置(重启)
    if( (memcmp(stOld.usLocolIp, stNew.usLocolIp, sizeof(stNew.usLocolIp)))
            ||(memcmp(stOld.usGateWay, stNew.usGateWay, sizeof(stNew.usGateWay)))
            )
    {
        TempArr.clear();
        for(int i = 0; i < 4; i++)
        {
            TempStr = QString::number(stNew.usLocolIp[i]) + ".";
            TempArr.append(TempStr);
        }
        TempArr.remove(TempArr.length() -1 , 1);
        strcpy(NetConfig.localIp, TempArr.data());

        TempArr.clear();
        for(int i = 0; i < 4; i++)
        {
            TempStr = QString::number(stNew.usGateWay[i]) + ".";
            TempArr.append(TempStr);
        }
        TempArr.remove(TempArr.length() -1 , 1);
        strcpy(NetConfig.gateway, TempArr.data());
        unParm.net0Config = NetConfig;
        pParamSet->updateSetting(&unParm, PARAM_NET0);
        bRet = TRUE;
        bScreenRebootFlag = TRUE;
    }
    return bRet;
}

//获取配置信息
void cSerialScreen::QueryParamInfo()
{
    //类型为QList的配置需要先清空list，否则会重复添加
    AllAmmeterConfig.ammeterConfig.clear();
    ThreePhaseTypeConfig.phaseTypeConfig.clear();
    AllTPFVConfig.tpfvConfig.clear();
    if( (!pParamSet->querySetting(&cscuSysConfig, PARAM_CSCU_SYS))
            ||(!pParamSet->querySetting(&NetConfig,PARAM_NET0))
            ||(!pParamSet->querySetting(&ServerConfig,PARAM_SERVER0))
            ||(!pParamSet->querySetting(&ChargeConfig,PARAM_CHARGE))
            ||(!pParamSet->querySetting(&AllAmmeterConfig,PARAM_AMMETER))
            ||(!pParamSet->querySetting(&IOConfig, PARAM_IO))
            ||(!pParamSet->querySetting(&ThreePhaseTypeConfig, PARAM_PHASE_TYPE))
            ||(!pParamSet->querySetting(&SmartChargeConfig, PARAM_SMARTCHARGE))
            ||(!pParamSet->querySetting(&PowerLimitConfig, PARAM_POWERLIMIT))
            ||(!pParamSet->querySetting(&AllTPFVConfig, PARAM_TPFV))
            )
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! Screen Load Param ERROR!");
    }
}

void cSerialScreen::SendCenterData(InfoMap ToCenterMap, InfoAddrType enType)
{
    emit sigToBus(ToCenterMap, enType);
}

//发送 相控制总线发送升级请求
void cSerialScreen::SendUpdateApply()
{
    InfoMap ToCenterMap;
    QByteArray tempArray;
    tempArray.append(0x02);//来源: U盘
    ToCenterMap.insert(Addr_Cmd_Source, tempArray);
    tempArray.clear();
    tempArray.append(0x01);//命令: 升级
    ToCenterMap.insert(Addr_Cmd_Master, tempArray);

    //发送
    SendCenterData(ToCenterMap, AddrType_ExecUpdate);
}

//发送 日志导出请求
void cSerialScreen::SendLogOutApply()
{
    InfoMap ToCenterMap;
    QByteArray tempArray;
    tempArray.append(0x02);//来源: U盘
    ToCenterMap.insert(Addr_Cmd_Source, tempArray);
    tempArray.clear();
    tempArray.append(0x02);//命令: 上传
    ToCenterMap.insert(Addr_Cmd_Master, tempArray);
    tempArray.clear();
    tempArray.append(0x01);//类型: 日志
    ToCenterMap.insert(Addr_Cmd_Slave, tempArray);

    //发送
    SendCenterData(ToCenterMap, AddrType_ExecUpdate);
}

//发送 刷卡请求
void cSerialScreen::SendCardNumApply()
{
    InfoMap ToCenterMap;
    QByteArray ArrayTemp;
    //插入CAN地址
    ArrayTemp.append((char)ScreenState.st_page_value.uc_can_addr);
    ToCenterMap.insert(Addr_CanID_Comm, ArrayTemp);
    //插入卡类型
    ArrayTemp.clear();
    ArrayTemp.append((char)ChargeConfig.cardType);
    ToCenterMap.insert(Addr_CardType, ArrayTemp);
    //发送
    SendCenterData(ToCenterMap, AddrType_ScreenApplyReadCard);
    pLog->getLogPoint(_strLogName)->info(QString("SendCardNumApply canaddr=%1 cardtype=%2").arg(ScreenState.st_page_value.uc_can_addr).arg(ChargeConfig.cardType));
}

//发送 刷卡结束请求, 让读卡器停止读卡
void cSerialScreen::SendCardApplyStop()
{
    InfoMap ToCenterMap;
    //发送
    SendCenterData(ToCenterMap, AddrType_ScreenApplyStopCard);
    pLog->getLogPoint(_strLogName)->info("SendCardApplyStop");
}

//发送 刷卡申请账户信息, 发送到总线
void cSerialScreen::SendCardApplyAccountInfo()
{
    InfoMap ToCenterMap;
    QByteArray CanID;
    CanID.append((char)ScreenState.st_page_value.uc_can_addr);
    if(!CardNum.isEmpty())
        ToCenterMap.insert(Addr_CardAccount, CardNum);//卡号
    if(!ScanCodeIDNum.isEmpty())
        ToCenterMap.insert(Addr_ScanCode_customerID, ScanCodeIDNum);//扫码客户ID
    ToCenterMap.insert(Addr_CanID_Comm, CanID);//can地址
     pLog->getLogPoint(_strLogName)->info(QString("SendCardApplyAccountInfo canaddr=%1").arg(ScreenState.st_page_value.uc_can_addr));
    //发送
    SendCenterData(ToCenterMap, AddrType_CenterCardApplyAccountInfo);
}

//发送 刷卡开始充电请求, 发送到总线
void cSerialScreen::SendCardStartCharge()
{
    char chtemp = 0x04;
    unsigned int uiValue = 0xffffffff;
    InfoMap ToCenterMap;
    QByteArray ChargeType;
    QByteArray CanID;
    QByteArray Value;

    ChargeType.append(chtemp);//4: 充满为止
    CanID.append((char)ScreenState.st_page_value.uc_can_addr);
    Value.append((char *)&uiValue, 4);
    if(CardNum.length() >= 8)
    {
        ToCenterMap.insert(Addr_CardAccount, CardNum);//卡号
        ToCenterMap.insert(Addr_CardApplyChargeType, ChargeType);//充电类型
    }
    if(ScanCodeIDNum.length() >= 16)
    {
        ToCenterMap.insert(Addr_ScanCode_customerID, ScanCodeIDNum);//扫码customerID
        ToCenterMap.insert(Addr_ScanCode_Charge_Type, ChargeType);//扫码充电类型
    }
    ToCenterMap.insert(Addr_CardChargeTypeValue, Value);//值
    ToCenterMap.insert(Addr_CanID_Comm, CanID);//can地址

    //发送
    SendCenterData(ToCenterMap, AddrType_InApplyStartChargeByScreen);
}

//发送 刷卡结束充电请求, 发送到总线
void cSerialScreen::SendCardStopCharge()
{
    InfoMap ToCenterMap;
    QByteArray CanID;
    CanID.append((char)ScreenState.st_page_value.uc_can_addr);
    if(!CardNum.isEmpty())
        ToCenterMap.insert(Addr_CardAccount, CardNum);//卡号
    if(!ScanCodeIDNum.isEmpty())
        ToCenterMap.insert(Addr_ScanCode_customerID, ScanCodeIDNum);//扫码customerID
    ToCenterMap.insert(Addr_CanID_Comm, CanID);//can地址

    //发送
    SendCenterData(ToCenterMap, AddrType_InApplyStopChargeByScreen);
}

//发送 错峰充电设置, 发送到总线
void cSerialScreen::SendPeakSet()
{
    InfoMap ToCenterMap;
    QByteArray SetData;
    for(unsigned char i = 0; i < 20; i++)
    {
        if( (PeakSet[i].start_hour == 0) && (PeakSet[i].start_minute == 0)
                && (PeakSet[i].stop_hour == 0) && (PeakSet[i].stop_minute == 0) )
        {
            continue;
        }
        SetData.clear();
        SetData.append((char *)&(PeakSet[i]), sizeof(stTPFVConfig));
        ToCenterMap.insert(i, SetData);
    }
    //发送
    //注:若列表为空, 由负荷调度模块返回失败结果
    SendCenterData(ToCenterMap, AddrType_SmartChargeSet);
}

//发送 VIN后6位申请充电, 发送到总线
void cSerialScreen::SendVINApplyCharge()
{
    InfoMap ToCenterMap;
    QByteArray SendData;
    SendData.append((char)ScreenState.st_page_value.uc_can_addr);
    ToCenterMap.insert(Addr_CanID_Comm,SendData);
    SendData.clear();
    SendData.append(chVINCach, sizeof(chVINCach));
    ToCenterMap.insert(Addr_BatteryVIN_BMS, SendData);

    //发送
    SendCenterData(ToCenterMap, AddrType_VINViaScreenApplyCharge);
}

//发送 屏幕申请结束充电, 发送到总线
void cSerialScreen::SendApplyStopCharge()
{
    InfoMap ToCenterMap;
    QByteArray SendData;
    SendData.append((char)ScreenState.st_page_value.uc_can_addr);
    ToCenterMap.insert(Addr_CanID_Comm,SendData);
    //发送
    SendCenterData(ToCenterMap, AddrType_ScreenApplyStopCharge);
}

void cSerialScreen::SendDCSpecApply()
{
    InfoMap ToCenterMap;
    //发送
    SendCenterData(ToCenterMap, AddrType_GeneralStaticArgRenew_DB);
    SendCenterData(ToCenterMap, AddrType_GeneralDynamicArgRenew_DB);
}

//发送 终端名称申请, 发送到总线
void cSerialScreen::SendTermNameApply()
{
    InfoMap ToCenterMap;
    QByteArray array;
    array.append(0xFF);
    ToCenterMap.insert(Addr_TermName_Adj, array);
    //发送
    SendCenterData(ToCenterMap, AddrType_TermIndex_Query);
}

//解析 主界面接收指令
void cSerialScreen::ParseFrameMainPage(unsigned char * pData, int iLength)
{
    unsigned char ucCmdType = pData[3];
    switch(ucCmdType)
    {
    //接收终端状态指令
    case Type_TermState_Screen:
        ParseFrameTermState(pData, iLength);
        break;
        //接收按钮指令
    case Type_Button_Screen:
        ParseFrameButtonMain(pData, iLength);
        break;
        //接收TEUI版本信息
    case Type_ReadMem_Screen:
        ParseFrameTEUIVer(pData);
        break;
    default:
        break;
    }
}

//解析 VIN开始充电主页面
void cSerialScreen::ParseFrameVINStartMain(unsigned char * pData, int iLength)
{
    unsigned char ucCmdType = pData[3];
    switch(ucCmdType)
    {
    //接收开始充电指令
    case Type_VINEndInput_Screen:
        if(ParseFrameVINEndInput(pData, iLength))   //VIN后6位校验成功
        {
            SendVINApplyCharge();
            pProtocol->SendSwitchPage(MENU_PAGE_VIN_APPLY_START);
            ScreenState.st_page_value.uiPageTimeOut = TO_ApplyStart_Screen;
            pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
            ucVINResultStep = 1;
            iPageSwitchCount = 0;
        }
        else    //VIN后6位校验失败
        {
            pProtocol->SendSwitchPage(MENU_PAGE_VIN_APPLY_START_RESULT);
            ScreenState.st_page_value.uiPageTimeOut = TO_VINResult_Screen;
            pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
            pProtocol->SendVINResult(ICON_VIN_CHECK_ERROR);
            ucVINResultStep = 2;
            iPageSwitchCount = 0;
        }
        break;
        //接收按钮指令
    case Type_Button_Screen:
        ParseFrameCardMainButton(pData, iLength);
        break;
    default:
        break;
    }
}

//解析 VIN后6位
bool cSerialScreen::ParseFrameVINEndInput(unsigned char * pData, int iLength)
{
    FrameVINEnd_Screen stFrame;
    memcpy((unsigned char * )& stFrame, pData, iLength);
    //VIN后6位校验成功
    if(memcmp(stFrame.chVINEnd, (unsigned char *)&chVINCach[11], 6) == 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}

//解析 VIN开始充电主页面--按钮按下
void cSerialScreen::ParseFrameVINStartMainButton(unsigned char * pData, int iLength)
{
    FrameButtonData_Screen stFrame;
    memcpy((unsigned char *)&stFrame, pData, iLength);
    switch(htons(stFrame.usButNum))
    {
    case 1://切换至主页面----返回按钮
        if(stFrame.usPageNum == 0xFFFF)
        {
            Ctrl_SwitchMain(0);
            pProtocol->SendSwitchPage(MENU_PAGE_MAIN);
        }
        break;
    default:
        break;
    }
}

//解析 刷卡主界面
void cSerialScreen::ParseFrameCardMain(unsigned char * pData, int iLength)
{
    unsigned char ucCmdType = pData[3];
    switch(ucCmdType)
    {
    //接收开始充电指令
    case Type_CardStartCharge_Screen:
        ucCardStep = 2;
        pProtocol->SendSwitchPage(MENU_PAGE_APPLAY_ACCOUNT_INFO);
        pProtocol->SendFrameCardWait(ucCardStep);
        SendCardStartCharge();
        iPageSwitchCount = 0;
        bCardResultFlag = FALSE;
        pProtocol->SendPageCount(TO_CardStartChargeWait_Screen);
        ScreenState.st_page_value.uiPageTimeOut = TO_CardStartChargeWait_Screen;
        break;
        //接收按钮指令
    case Type_Button_Screen:
        ParseFrameCardMainButton(pData, iLength);
        break;
    default:
        break;
    }
}

//解析 刷卡主界面--按钮按下
void cSerialScreen::ParseFrameCardMainButton(unsigned char * pData, int iLength)
{
    FrameButtonData_Screen stFrame;
    memcpy((unsigned char *)&stFrame, pData, iLength);
    switch(htons(stFrame.usButNum))
    {
    case 1://切换至主页面----返回按钮
        if(stFrame.usPageNum == 0xFFFF)
        {
            Ctrl_SwitchMain(0);
            pProtocol->SendSwitchPage(MENU_PAGE_MAIN);
            SendCardApplyStop();
        }
        break;
    default:
        break;
    }
}

//解析 终端详情界面接收指令
void cSerialScreen::ParseFrameTermDatil_Normal(unsigned char * pData, int iLength)
{
    unsigned char ucCanID = 0;
    unsigned char ucCmdType = pData[3];
    switch(ucCmdType)
    {
    //接收按钮指令
    case Type_Button_Screen:
        ParseFrameButtonTermDatil_Normal(pData, iLength);
        break;
        //点击终端BMS信息
    case Type_TermBMS_Screen:
        ucCanID = ParseFrameBMSApply(pData, iLength);
        Ctrl_SwitchTermBMS(ucCanID);
        pProtocol->SendTermBMSInfo(ucCanID);
        pProtocol->SendPageCount(30); //给TEUI发送30秒倒计时 add by zrx
        break;
    }
}

//解析 主界面下按钮
void cSerialScreen::ParseFrameButtonMain(unsigned char * pData, int iLength)
{
    FrameButtonData_Screen stFrame;
    memcpy((unsigned char *)&stFrame, pData, iLength);
    switch(htons(stFrame.usButNum))
    {
    case 1:
        if(stFrame.usPageNum == 0xFFFF)//主界面点返回, 立即显示主界面
        {
            Ctrl_SwitchMain(0);
            pProtocol->SendSwitchPage(MENU_PAGE_MAIN);
        }
        else //切换至参数设置主界面
        {
               Ctrl_SwitchPasswdInput(0);
               pProtocol->SendSwitchPage(MENU_PAGE_PASSWD_INPUT);        //add by zrx
//            Ctrl_SwitchParamMain(0);
//            pProtocol->SendSwitchPage(MENU_PAGE_SET_MAIN);

            pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);

        }
        break;
    case 2:     //余额查询指令  add by zrx
        bBanlanceFlag = TRUE;
        ucCardStep = 1;
        Ctrl_SwitchCardMain(0);
        SendCardNumApply();
        pProtocol->SendSwitchPage(MENU_PAGE_SWIPE_CARD_MAIN);
        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        break;
    default:
        break;
    }
}

//解析 终端详情界面按钮
void cSerialScreen::ParseFrameButtonTermDatil_Normal(unsigned char * pData, int iLength)
{
    FrameButtonData_Screen stFrame;
    unsigned char ucCanID = 0;
    TerminalStatus stTerm;

    memcpy((unsigned char *)&stFrame, pData, iLength);
    switch(htons(stFrame.usButNum))
    {
    case BUTTON_TYPE_RETURN://切换至设置主页面----返回按钮
        if(stFrame.usPageNum == 0xFFFF)
        {
            Ctrl_SwitchMain(0);
            pProtocol->SendSwitchPage(MENU_PAGE_MAIN);
            pProtocol->SendMainPageData(NameMap, cscuSysConfig, cscuSysConfig.version,NameMapShow);
            SendCardApplyStop();
        }
        break;
    case BUTTON_TYPE_STOP_CHARGE: //if(stFrame.usPageNum == htons(MENU_PAGE_TERM_INFO_NORMAL_LOCAL_STOP))
        SendApplyStopCharge();
        iPageSwitchCount = 0;
        pProtocol->SendSwitchPage(MENU_PAGE_LOCAL_STOP_WAIT);
        ScreenState.st_page_value.uiPageTimeOut = TO_LocalStop_Screen;
        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        SendCardApplyStop();
        break;
    case BUTTON_TYPE_BMS_INFO://BMS详情按钮
        //状态机
        ucCanID = ScreenState.st_page_value.uc_can_addr;
        Ctrl_SwitchTermBMS(ucCanID);
        //切界面
        if(ChargeConfig.localStop == 0) {//不支持本地结束
            pProtocol->SendSwitchPage(MENU_PAGE_BMS);
        }
        else{
            pProtocol->SendSwitchPage(MENU_PAGE_BMS_STOP_BUTTON);
        }
        ucCardStep = 3;
        //给页面送显
        pProtocol->SendTermBMSInfo(ucCanID);
        //倒计时
        ScreenState.st_page_value.uiPageTimeOut = TO_TermBMS_Screen;
        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut); //给TEUI发送30秒倒计时 add by zrx
        break;
    case BUTTON_TYPE_CHARGING_INFO://充电信息页面
        ucCanID = ScreenState.st_page_value.uc_can_addr;
        if(! pDevCache->QueryTerminalStatus(ucCanID,stTerm))
        {
            pLog->getLogPoint(_strLogName)->info("Screen pDevCache Query FALSE");
        }
        if(ChargeConfig.localStop == 0) {//不支持本地结束
            pProtocol->SendSwitchPage(MENU_PAGE_TERM_INFO_CARD);
        }
        else{
            pProtocol->SendSwitchPage(MENU_PAGE_TERM_INFO_CARD_STOP_BUTTON);
        }
        ucCardStep = 3;
        Ctrl_SwitchTermDetail_Card(stTerm.cCanAddr);
        pProtocol->SendTermDetailData_Card(stTerm, ThreePhaseTypeConfig);
        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        SendCardNumApply();
    default:
        break;
    }
}

//解析-充电报告界面按钮
void cSerialScreen::ParseFrameButtonChargeReport(unsigned char * pData, int iLength)
{
    FrameButtonData_Screen stFrame;
    unsigned char ucCanID = 0;

    memcpy((unsigned char *)&stFrame, pData, iLength);
    switch(htons(stFrame.usButNum))
    {
    case BUTTON_TYPE_RETURN://切换至设置主页面----返回按钮
        if(stFrame.usPageNum == 0xFFFF)
        {
            Ctrl_SwitchMain(0);
            pProtocol->SendSwitchPage(MENU_PAGE_MAIN);
            pProtocol->SendMainPageData(NameMap, cscuSysConfig, cscuSysConfig.version,NameMapShow);
        }
        else if(htons(stFrame.usPageNum) == 0x002D)
        {
            if(ChargeConfig.ticketEnable == 1)  //小票机使能
            {
                ucCanID = ScreenState.st_page_value.uc_can_addr;
                TerminalStatus stTerm;
                if(! pDevCache->QueryTerminalStatus(ucCanID,stTerm))
                    {
                        pLog->getLogPoint(_strLogName)->info("ERROR! Screen pDevCache Query  Error!");
                        return;
                    }
                if(stTerm.bTicketPrint == 1)
                {
                    Ctrl_SwitchTicketPrintMain(ucCanID);
                    pProtocol->SendSwitchPage(MENU_PAGE_TICKET_PRINT_MAIN);
                    pProtocol->SendPageCount(30);
                }
                else if(stTerm.bTicketPrint == 2) //您已打印一次，不允许重复打印
                {
                    QByteArray retArray;
                    pProtocol->SendSwitchPage(MENU_PAGE_CARD_RESULT);
                    retArray = "您已打印一次,不允许重复打印!";
                    pProtocol->SendFrameCardResult(retArray);
                    pProtocol->SendPageCount(TO_CardStartStopResult_Screen);
                    //超时时间置为5s
                    ScreenState.st_page_value.uiPageTimeOut = TO_CardStartStopResult_Screen;
                    iPageSwitchCount = 0;
                    bCardResultFlag = TRUE;
                }
            }
            else //小票机功能未使能，请先进行配置！
            {
                QByteArray retArray;
                pProtocol->SendSwitchPage(MENU_PAGE_CARD_RESULT);
                retArray = "小票机功能未使能,请先进行配置!";
                pProtocol->SendFrameCardResult(retArray);
                pProtocol->SendPageCount(TO_CardStartStopResult_Screen);
                //超时时间置为5s
                ScreenState.st_page_value.uiPageTimeOut = TO_CardStartStopResult_Screen;
                iPageSwitchCount = 0;
                bCardResultFlag = TRUE;
            }
        }
        break;
    case BUTTON_TYPE_CHARGE_REPORT: //切换到界面45-充电报告
        ucCanID = ScreenState.st_page_value.uc_can_addr;
        Ctrl_SwitchChargeReport_Card(ucCanID);//状态机
        pProtocol->SendSwitchPage(MENU_PAGE_CHARGE_CARD_REPORT);//发送切换页面指令
        pProtocol->SendTermChargeReport(ucCanID);//页面送显
        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);//倒计时
        break;
    case BUTTON_TYPE_CHARGE_REPORT_DETAIL://切换到界面145-充电明细
        ucCanID = ScreenState.st_page_value.uc_can_addr;
        Ctrl_SwitchChargeReport_Card(ucCanID);//状态机
        pProtocol->SendSwitchPage(MENU_PAGE_CHARGE_CARD_REPORT_DETAIL);//发送切换页面指令
        pProtocol->SendTermChargeReportDetail1(ucCanID);//页面送显
        pProtocol->SendTermChargeReportDetail2(ucCanID);//页面送显
        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);//倒计时
        break;
    default:
        break;
    }
}

//解析 BMS信息申请
unsigned char cSerialScreen::ParseFrameBMSApply(unsigned char * pData, int iLength)
{
    unsigned char ucCanID;
    FrameGetTermBMS_Screen stFrame;
    memcpy((unsigned char *)&stFrame, pData, iLength);
    ucCanID = htons(stFrame.usCanID);
    return ucCanID;
}

//解析 BMS页面下按钮
void cSerialScreen::ParseFrameButtonBMSInfo(unsigned char * pData, int iLength)
{
    FrameButtonData_Screen stFrame;
    unsigned char ucCanID = 0;

    memcpy((unsigned char *)&stFrame, pData, iLength);
    switch(htons(stFrame.usButNum))
    {
    case BUTTON_TYPE_RETURN://返回按钮
        if(stFrame.usPageNum == 0xFFFF)
        {
            Ctrl_SwitchMain(0);
            pProtocol->SendSwitchPage(MENU_PAGE_MAIN);
            pProtocol->SendMainPageData(NameMap, cscuSysConfig, cscuSysConfig.version,NameMapShow);
            SendCardApplyStop();
        }
        break;
    case BUTTON_TYPE_STOP_CHARGE: //结束充电按钮
        SendApplyStopCharge();
        iPageSwitchCount = 0;
        pProtocol->SendSwitchPage(MENU_PAGE_LOCAL_STOP_WAIT);
        ScreenState.st_page_value.uiPageTimeOut = TO_LocalStop_Screen;
        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        SendCardApplyStop();
        break;
    case BUTTON_TYPE_CHARGING_INFO://充电信息页面
        ucCanID = ScreenState.st_page_value.uc_can_addr;
        if(ChargeConfig.localStop == 0) {//不支持本地结束
            pProtocol->SendSwitchPage(MENU_PAGE_TERM_INFO_CARD);
        }
        else{
            pProtocol->SendSwitchPage(MENU_PAGE_TERM_INFO_CARD_STOP_BUTTON);
        }
        ucCardStep = 3;
        Ctrl_SwitchTermDetail_Card(ucCanID);
        pProtocol->SendTermPageChargingData(ucCanID, (unsigned char)cscuSysConfig.normalCardType, ThreePhaseTypeConfig);
        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        SendCardNumApply();
        break;
    case BUTTON_TYPE_CHARGING_INFO_DETAIL://终端-详细详情-按钮
        //状态机
        ucCanID = ScreenState.st_page_value.uc_can_addr;
        Ctrl_SwitchTermDetail_Normal(ucCanID);
        //切界面
        if(ChargeConfig.localStop == 0) {//不支持本地结束
            pProtocol->SendSwitchPage(MENU_PAGE_TERM_INFO_NORMAL);
        }
        else{
            pProtocol->SendSwitchPage(MENU_PAGE_TERM_INFO_NORMAL_LOCAL_STOP);
        }
        ucCardStep = 3;
        //给页面送显        
        pProtocol->SendTermPageData(ucCanID, (unsigned char)cscuSysConfig.normalCardType, ThreePhaseTypeConfig);
        //倒计时
        ScreenState.st_page_value.uiPageTimeOut = TO_TermDetail_Normal_Screen;
        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        SendCardNumApply();//通知读卡
        break;
    default:
        break;
    }
}

//解析 终端详情界面接收指令----刷卡版
void cSerialScreen::ParseFrameTermDatil_Card(unsigned char * pData, int iLength)
{
//    unsigned char ucCanID = 0;
    unsigned char ucCmdType = pData[3];
    switch(ucCmdType)
    {
    //接收按钮指令
    case Type_Button_Screen:
        ParseFrameButtonTermDatil_Card(pData, iLength);
        break;
    }
}

//解析 充电信息界面-按钮处理
void cSerialScreen::ParseFrameButtonTermDatil_Card(unsigned char * pData, int iLength)
{
    FrameButtonData_Screen stFrame;
    unsigned char ucCanID = 0;
    memcpy((unsigned char *)&stFrame, pData, iLength);
    switch(htons(stFrame.usButNum))
    {
    case BUTTON_TYPE_RETURN://返回按钮
        if(stFrame.usPageNum == 0xFFFF)
        {
            ucCanID = ScreenState.st_page_value.uc_can_addr;
            Ctrl_SwitchMain(0);
            SendCardApplyStop();
            pProtocol->SendSwitchPage(MENU_PAGE_MAIN);
        }
        break;
        //add by YCZ 2017-04-06 充电中新增三个按钮处理.
    case BUTTON_TYPE_STOP_CHARGE://结束充电按钮
        SendApplyStopCharge();
        iPageSwitchCount = 0;
        pProtocol->SendSwitchPage(MENU_PAGE_LOCAL_STOP_WAIT);
        ScreenState.st_page_value.uiPageTimeOut = TO_LocalStop_Screen;
        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        SendCardApplyStop();
        break;
    case BUTTON_TYPE_BMS_INFO://BMS详情按钮
        //状态机
        ucCanID = ScreenState.st_page_value.uc_can_addr;
        Ctrl_SwitchTermBMS(ucCanID);
        //切界面
        if(ChargeConfig.localStop == 0) {//不支持本地结束
            pProtocol->SendSwitchPage(MENU_PAGE_BMS);
        }
        else{
            pProtocol->SendSwitchPage(MENU_PAGE_BMS_STOP_BUTTON);
        }
        ucCardStep = 3;
        //给页面送显
        pProtocol->SendTermBMSInfo(ucCanID);
        //倒计时
        ScreenState.st_page_value.uiPageTimeOut = TO_TermBMS_Screen;
        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut); //给TEUI发送30秒倒计时 add by zrx
        SendCardNumApply();//通知读卡
        break;
    case BUTTON_TYPE_CHARGING_INFO_DETAIL://终端详情按钮
        //状态机
        ucCanID = ScreenState.st_page_value.uc_can_addr;
        Ctrl_SwitchTermDetail_Normal(ucCanID);
        //切界面
        if(ChargeConfig.localStop == 0) {//不支持本地结束
            pProtocol->SendSwitchPage(MENU_PAGE_TERM_INFO_NORMAL);
        }
        else{
            pProtocol->SendSwitchPage(MENU_PAGE_TERM_INFO_NORMAL_LOCAL_STOP);
        }
        ucCardStep = 3;
        //给页面送显
        pProtocol->SendTermPageData(ucCanID, (unsigned char)cscuSysConfig.normalCardType, ThreePhaseTypeConfig);
        //倒计时
        ScreenState.st_page_value.uiPageTimeOut = TO_TermDetail_Normal_Screen;
        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        break;
    default:
        break;
    }
}

//解析 充电报告界面接收指令----普通版
void cSerialScreen::ParseFrameChargeReport_Normal(unsigned char * pData, int iLength)
{
    iLength = 0;
    unsigned char ucCmdType = pData[3];
    switch(ucCmdType)
    {
    //接收按钮指令(返回)
    case Type_Button_Screen:
        Ctrl_SwitchMain(0);
        pProtocol->SendSwitchPage(MENU_PAGE_MAIN);
        break;
    }
}

//解析 充电报告界面接收指令----刷卡版
void cSerialScreen::ParseFrameChargeReport_Card(unsigned char * pData, int iLength)
{
    unsigned char ucCmdType = pData[3];
    switch(ucCmdType)
    {
    //开始充电指令
    case Type_TermState_Screen:
        bBanlanceFlag = FALSE;
        ucCardStep = 1;
        Ctrl_SwitchCardMain(ScreenState.st_page_value.uc_can_addr);
        pProtocol->SendSwitchPage(MENU_PAGE_SWIPE_CARD_MAIN);
        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        SendCardNumApply();
        break;
        //接收按钮指令(返回)
    case Type_Button_Screen:
        ParseFrameButtonChargeReport(pData, iLength);
        break;
    }
}

//解析 设置主页面下主函数
void cSerialScreen::ParseFrameSetMain(unsigned char * pData, int iLength)
{
    switch(pData[3])
    {
    case Type_Button_Screen:
        ParseFrameButtonParamMain(pData, iLength);
        break;
    case Type_TermName_Screen:
        SendTermNameApply();
        break;
    default:
        break;
    }
}

//解析 设置主页面下按钮
void cSerialScreen::ParseFrameButtonParamMain(unsigned char * pData, int iLength)
{
    FrameButtonData_Screen stFrame;
    ParamSetPage_Screen stNow;
    QByteArray AmmeterAddr;
    //SpecFuncSet_Screen stSet;
    if(pData[3]!= Type_Button_Screen)
    {
        return;
    }
    memcpy((unsigned char *)&stFrame, pData, iLength);
    switch(htons(stFrame.usButNum))
    {
    case 1://返回按钮
        if(stFrame.usPageNum == 0xFFFF)
        {
            Ctrl_SwitchMain(0);
            pProtocol->SendSwitchPage(MENU_PAGE_MAIN);
        }
        else if(stFrame.usPageNum == htons(MENU_PAGE_SET_MAIN))//切换至工程选择页面
        {
            QueryParamInfo();
            Ctrl_SwitchProChose(0);
            pProtocol->SendSwitchPage(MENU_PAGE_SET_PRO);
            pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
            pProtocol->SendFrameProType(cscuSysConfig.normalCardType);
        }
        break;
    case 2://切换至系统参数设置页面
        QueryParamInfo();
        Ctrl_SwitchSysParamSet(0);
        pProtocol->SendSwitchPage(MENU_PAGE_SET_SYSTEM);
        stNow = GetSysParamSet();
        pProtocol->SendSysParamData(stNow);
        pProtocol->SendPageCountStop(MENU_PAGE_SET_SYSTEM);
        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        break;
    case 3://切换至交流相别设置界面
        QueryParamInfo();
        Ctrl_SwitchACPhaseSet(0);
        pProtocol->SendSwitchPage(MENU_PAGE_SET_PHASE);
        pProtocol->SendFrameACPhaseSet(ThreePhaseTypeConfig);
        pProtocol->SendPageCountStop(MENU_PAGE_SET_PHASE);
        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        break;
    case 4://特殊功能设置显示页面
		Ctrl_SwitchPage(MENU_PAGE_SPEC_GENERAL, TO_SpecFuncSet_Screen);
		pProtocol->SendSpecGeneral();
		pProtocol->SendSpecGeneral1();
        break;
    case 5://切换至进线侧信息界面
        Ctrl_SwitchInLineInfo(0);
        if(!AllAmmeterConfig.ammeterConfig.isEmpty())
        {
            pProtocol->SendSwitchPage(MENU_PAGE_IN_LINE_INFO);
            pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
            AmmeterAddr.append((char *)AllAmmeterConfig.ammeterConfig.at(0).addr, 6);
            pProtocol->SendAmmeterAddr(AllAmmeterConfig.ammeterConfig, 1);
            pProtocol->SendInLineInfo(AmmeterAddr);
        }
        else
        {
            pProtocol->SendSwitchPage(MENU_PAGE_SET_SPEC_RESULT);
            pProtocol->SendShowIcon(ICON_INLINE_PARAM_FAILED);
            ScreenState.st_page_value.uiPageTimeOut = TO_AmmeterParamFault_Screen;
            pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
            bPageWaitFlag = TRUE;
            iPageWaitTime = TO_AmmeterParamFault_Screen;
        }
        break;
    case 6://切换至子站环境信息页面
        QueryParamInfo();
        Ctrl_SwitchEnvInfo(0);
        pProtocol->SendEnvInfo();
        pProtocol->SendAlarmSet(IOConfig);
        pProtocol->SendPageCountStop(MENU_PAGE_ENV_INFO);
        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        break;
    case 7://切换至直流特殊功能设置页面
        QueryParamInfo();
        ucDCSpecAckStep = 0;
        SendDCSpecApply();
//        pProtocol->SendDCSpec(cscuSysConfig.directCurrent);
        break;
    case 9://切换至负荷调度选择界面
        QueryParamInfo();
        Ctrl_SwitchLoadDispatchChoose(0);
        pProtocol->SendSwitchPage(MENU_PAGE_LOAD_DISPATCH_CHOOSE);
        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        pProtocol->SendLoadDispatchEnable(SmartChargeConfig.sSmartCharge_Enable, PowerLimitConfig.sPowerLimit_Enable);
        break;
    default:
        break;
    }
}

//解析 工程选择界面下按钮
void cSerialScreen::ParseFrameProSetButton(unsigned char * pData, int iLength)
{
    FrameButtonData_Screen stFrame;
    unParamConfig unParm;
    memcpy((unsigned char *)&stFrame, pData, iLength);
    unParm.cscuSysConfig = cscuSysConfig;
    switch(htons(stFrame.usButNum))
    {
    case 1://返回按钮
        if(stFrame.usPageNum == 0xFFFF)
        {
            Ctrl_SwitchParamMain(0);
            pProtocol->SendSwitchPage(MENU_PAGE_SET_MAIN);
            pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        }
        else if (stFrame.usPageNum == htons(MENU_PAGE_SET_PRO))// 普通版
        {
            unParm.cscuSysConfig.normalCardType = 1;
            cscuSysConfig.normalCardType = 1;
            pParamSet->updateSetting(&unParm, PARAM_CSCU_SYS);
            Ctrl_SwitchParamMain(0);
            pProtocol->SendSwitchPage(MENU_PAGE_SET_MAIN);
            pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        }
        break;
    case 2: // 刷卡版
        unParm.cscuSysConfig.normalCardType = 2;
        cscuSysConfig.normalCardType = 2;
        pParamSet->updateSetting(&unParm, PARAM_CSCU_SYS);
        Ctrl_SwitchParamMain(0);
        pProtocol->SendSwitchPage(MENU_PAGE_SET_MAIN);
        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        break;
    default:
        break;
    }
}

//解析 系统设置界面命令
void cSerialScreen::ParseFrameSysSetMain(unsigned char * pData, int iLength)
{
    ParamSetPage_Screen stSet;
    unsigned char ucCmdType = pData[3];

    switch(ucCmdType)
    {
    case Type_ParamSet_Screen:
        stSet = ParseFrameSysSet(pData, iLength);
        if(SysParamCmp(stSet))
        {
            if(bScreenRebootFlag)//重启等待界面
            {
                pProtocol->SendSwitchPage(MENU_PAGE_PARAM_CHANGE);
                pProtocol->SendPageCount(TO_SysSetResultWait_Screen__CHANGE);
            }
            else    //返回设置主界面
            {
                iPageSwitchCount = 0;
                pProtocol->SendSwitchPage(MENU_PAGE_PARAM_SET_SUCCESS);
//                ScreenState.st_page_value.uiPageTimeOut = TO_SysSetResultWait_Screen__SUCCESS;
                pProtocol->SendPageCount(TO_SysSetResultWait_Screen__SUCCESS);
                iPageWaitTime = TO_SysSetResultWait_Screen__SUCCESS;
//                Ctrl_SwitchParamMain(0);
//                pProtocol->SendSwitchPage(MENU_PAGE_SET_MAIN);
//                pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
//                iPageSwitchCount = 0;
                bPageWaitFlag = TRUE;
            }
        }
        else
        {
            pProtocol->SendSwitchPage(MENU_PAGE_PARAM_NOCHANGE);
            pProtocol->SendPageCount(TO_SysSetResultWait_Screen__NOCHANGE);
            iPageSwitchCount = 0;
            bPageWaitFlag = TRUE;
            iPageWaitTime = TO_SysSetResultWait_Screen__NOCHANGE;
        }
        break;
    case Type_Button_Screen:
        ParseFrameButtonSysSet(pData, iLength);
        break;
    default:
        break;
    }
}

//解析 系统设置
ParamSetPage_Screen cSerialScreen::ParseFrameSysSet(unsigned char * pData, int iLength)
{
    FrameRecvSysParam_Screen stFrame;
    ParamSetPage_Screen stRet;
    memcpy((unsigned char *)&stFrame, pData, iLength);
    stRet.usACSinNum = htons(stFrame.usACSinNum);
    stRet.usACThrNum = htons(stFrame.usACThrNum);
    stRet.usDCNum = htons(stFrame.usDCNum);
    for(int i = 0; i < 4; i++)
    {
        stRet.usLocolIp[i] = htons(stFrame.usLocolIp[i]);
        stRet.usGateWay[i] = htons(stFrame.usGateWay[i]);
        stRet.usDNSServer[i] = htons(stFrame.usDNSServer[i]);
    }
    stRet.usServerPort = htons(stFrame.usServerPort);
    stRet.usZigBeeID = htons(stFrame.usZigBeeID);
    memset(stRet.chStationAddr ,0x00,  sizeof(stRet.chStationAddr));
    memcpy(stRet.chStationAddr,  stFrame.chStationAddr, sizeof(stFrame.chStationAddr));
    memset(stRet.chDomainName ,0x00,  sizeof(stRet.chDomainName));
    memcpy(stRet.chDomainName,  stFrame.chDomainName, sizeof(stFrame.chDomainName));
    //将串中的0xFF转换为0
    for(unsigned char i = 0; i < sizeof(stRet.chStationAddr); i++)
    {
        if(stRet.chStationAddr[i] == 0xFF)
        {
            stRet.chStationAddr[i] = 0;
        }
    }
    for(unsigned char i = 0; i < sizeof(stRet.chDomainName); i++)
    {
        if(stRet.chDomainName[i] == 0xFF)
        {
            stRet.chDomainName[i] = 0;
        }
    }
    return stRet;
}

//解析 系统设置按钮设置
void cSerialScreen::ParseFrameButtonSysSet(unsigned char * pData, int iLength)
{
    FrameButtonData_Screen stFrame;
    memcpy((unsigned char *)&stFrame, pData, iLength);
    switch(htons(stFrame.usButNum))
    {
    case 1://切换至设置主页面----返回按钮
        if(stFrame.usPageNum == 0xFFFF)
        {
            bPageWaitFlag = FALSE;
            Ctrl_SwitchParamMain(0);
            pProtocol->SendSwitchPage(MENU_PAGE_SET_MAIN);
            pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        }
        break;
    default:
        break;
    }
}

//解析 交流相别设置界面命令
void cSerialScreen::ParseFrameACPhaseMain(unsigned char * pData, int iLength)
{
    unsigned char ucCmdType = pData[3];
    switch(ucCmdType)
    {
    //接收相别设置指令
    case Type_ACPhaseSet_Screen:
        ParseFrameACPhaseSet(pData, iLength);
        Ctrl_SwitchParamMain(0);
        pProtocol->SendSwitchPage(MENU_PAGE_SET_MAIN);
        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        break;
        //接收按钮指令
    case Type_Button_Screen:
        ParseFrameButtonACPhase(pData, iLength);
        break;
    default:
        break;
    }
}

//解析 交流相别设置界面命令--按钮按下
void cSerialScreen::ParseFrameButtonACPhase(unsigned char * pData, int iLength)
{
    FrameButtonData_Screen stFrame;
    memcpy((unsigned char *)&stFrame, pData, iLength);
    switch(htons(stFrame.usButNum))
    {
    case 1://切换至设置主页面----返回按钮
        if(stFrame.usPageNum == 0xFFFF)
        {
            Ctrl_SwitchParamMain(0);
            pProtocol->SendSwitchPage(MENU_PAGE_SET_MAIN);
            pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        }
        break;
    default:
        break;
    }
}

//解析 交流相别设置界面命令
void cSerialScreen::ParseFrameACPhaseSet(unsigned char * pData, int iLength)
{
    FrameACPhaseSet_Screen stFrame;
    memcpy((unsigned char *)&stFrame, pData, iLength);
    stThreePhaseTypeConfig tempConfig;//三相相别全部参数, 接收屏幕设置
    stPhaseTypeConfig tempNode; //三相相别记录节点

    //写入数据库
    for(int i = 0; i < 50; i++)
    {
        tempNode.canaddr = ID_MinACSinCanID + i;
        tempNode.type = (unsigned char)htons(stFrame.usPhase[i]);
        tempConfig.phaseTypeConfig.append(tempNode);
    }
    pParamSet->updateSetting(&tempConfig, PARAM_PHASE_TYPE);
}

//解析 密码登陆设置主界面
void cSerialScreen::ParsePasswdMain(unsigned char * pData, int iLength)
{
    unsigned char ucCmdType = pData[3];
//    SpecFuncSet_Screen stSet;
    switch(ucCmdType)
    {
    case Type_SpecPasswd_Screen:
        //解析密码正确
        if(ParseSpecPasswd(pData, iLength))
        {
            Ctrl_SwitchParamMain(0);
            pProtocol->SendSwitchPage(MENU_PAGE_SET_MAIN);     //by zrx
            pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
//            stSet = GetSpecFunSet();
//            pProtocol->SendSwitchPage(MENU_PAGE_SET_SPEC_FUNC);
//            pProtocol->SendSpecFuncSet(stSet);
        }
        //解析密码错误
        else
        {
            pProtocol->SendSwitchPage(MENU_PAGE_PASSWD_WRONG);
            pProtocol->SendPageCount(TO_PasswdWrong_Screen);
//            bPageWaitFlag = TRUE;
            iPageWaitTime = TO_PasswdWrong_Screen;

            ScreenState.us_page_num = MENU_PAGE_PASSWD_WRONG;
            ScreenState.st_page_value.uc_hold_time = TO_PasswdWrong_Screen;
            ScreenState.st_page_value.uc_page_num_return = MENU_PAGE_MAIN;
            ScreenState.st_page_value.uiPageTimeOut = TO_ChangePwdResult;//超时时间5s
            iPageSwitchCount = 0;//计数器清零
            bPageWaitFlag = FALSE;// 状态机切换, 取消等待处理
        }
        break;
    case Type_SpecResetPasswd_Screen:
        if(ParseResetPasswd(pData, iLength))    //校验成功
        {
            Ctrl_SwitchChangePwdSuccess(0);
            pProtocol->SendSwitchPage(MENU_PAGE_RESET_PASSWD_SUCCESS);
        }
        else    //校验失败
        {
            Ctrl_SwitchChangePwdFail(0);
            pProtocol->SendSwitchPage(MENU_PAGE_RESET_SPEC_FAILED);
        }
        pProtocol->SendPageCount(TO_SpecResetPasswdResult_Screen);
        bPageWaitFlag = TRUE;
        iPageWaitTime = TO_SpecResetPasswdResult_Screen;
        iPageSwitchCount = 0;
        break;
    case Type_Button_Screen:
        ParsePasswdMainButton(pData, iLength);
        break;
    default:
        break;
    }
}

//解析 特殊功能密码设置主界面--按钮按下
void cSerialScreen::ParsePasswdMainButton(unsigned char * pData, int iLength)
{
    FrameButtonData_Screen stFrame;
    memcpy((unsigned char *)&stFrame, pData, iLength);
    switch(htons(stFrame.usButNum))
    {
    case 1://切换至主页面----返回按钮
        if(stFrame.usPageNum == 0xFFFF)
        {
//            Ctrl_SwitchParamMain(0);
            Ctrl_SwitchMain(0);     //2017-01-19 by zrx
            pProtocol->SendSwitchPage(MENU_PAGE_MAIN);
//            pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        }
        break;
    case 2://密码修改按钮
    {
        iPageSwitchCount = 0;
        pProtocol->SendSwitchPage(MENU_PAGE_SHOW_PASSWD);
//        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        pProtocol->SendPageCount(60);
    }
        break;
    default:
        break;
    }
}

//解析 特殊功能设置密码
bool cSerialScreen::ParseSpecPasswd(unsigned char * pData, int iLength)
{
    unsigned int uiPasswd = 0;
    FrameSpecPasswd_Screen stFrame;
    memcpy((unsigned char *)&stFrame, pData, iLength);
    //密码转换为unsigned int
    for(int i = 0; i < 4; i++)
    {
        uiPasswd +=stFrame.ucPasswd[i] << 8*(3 - i);
    }
    if(uiPasswd == (unsigned int)cscuSysConfig.password)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//解析 重新设置密码
bool cSerialScreen::ParseResetPasswd(unsigned char * pData, int iLength)
{
    unParamConfig unParm;
    unsigned int uiPasswd = 0;
    FrameResetPasswd_Screen stFrame;
    stCSCUSysConfig ConfigNew;
    memcpy((unsigned char *)&stFrame, pData, iLength);
    for(int i = 0; i < 4; i++)
    {
        uiPasswd +=stFrame.ucPasswdOld[i] << 8*(3 - i);
    }
    if(uiPasswd != (unsigned int)cscuSysConfig.password)
    {
        return FALSE;
    }

    for(unsigned char i = 0; i < 4; i++)
    {
        if(stFrame.ucPasswdEnsure[i] != stFrame.ucPasswdNew[i])
        {
            return FALSE;
        }
    }
    ConfigNew = cscuSysConfig;
    ConfigNew.password = 0;
    for(unsigned char i = 0; i < 4; i++)
    {
        ConfigNew.password += stFrame.ucPasswdNew[i] << 8*(3 - i);
    }
    unParm.cscuSysConfig = ConfigNew;
    pParamSet->updateSetting(&unParm, PARAM_CSCU_SYS);
    QueryParamInfo();
    return TRUE;
}


//解析 密码修改成功主界面
void cSerialScreen::ParsePasswdChangeResult(unsigned char * pData, int iLength)
{
    FrameButtonData_Screen stFrame;
    memcpy((unsigned char *)&stFrame, pData, iLength);
    unsigned char ucCmdType = pData[3];
    switch(ucCmdType)
    {
        case Type_Button_Screen:
            if((htons(stFrame.usButNum)) == 1)
            {
                if(stFrame.usPageNum == 0xFFFF)
                {
                    Ctrl_SwitchMain(0);
                    pProtocol->SendSwitchPage(MENU_PAGE_MAIN);
    //                pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
                }
            }
            break;
        default:
            break;
    }
}
//  hd
//双枪充电使能，按“确定”按钮充电
void cSerialScreen::ParseCardChargeRequest_Normal(unsigned char * pData, int iLength)
{
    FrameButtonData_Screen stFrame;
    memcpy((unsigned char *)&stFrame, pData, iLength);

    //返回
    if((htons(stFrame.usButNum)) == 1 && stFrame.usPageNum == 0xFFFF)
    {
        Ctrl_SwitchMain(0);
        pProtocol->SendSwitchPage(MENU_PAGE_MAIN);
    }

    unsigned char ucCmdType = pData[7];
    unsigned char ucPagNum = pData[5];
    //刷卡充电
    if((ucPagNum == 0x5F || ucPagNum == 0x61) && ucCmdType == 0x02)
    {
        ucCardStep = 1;
        Ctrl_SwitchCardMain(ScreenState.st_page_value.uc_can_addr);
        SendCardNumApply();
        pProtocol->SendSwitchPage(MENU_PAGE_SWIPE_CARD_MAIN);
        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
    }//vin/车牌号充电
    else if((ucPagNum == 0x5F && ucCmdType == 0x03)|| (ucPagNum == 0x60 && ucCmdType == 0x02) )
    {
        InfoMap qInfoMap;
        CHARGE_STEP stChargeStep;//存放从缓存中读出的状态机

        if(pDevCache->QueryChargeStep(ScreenState.st_page_value.uc_can_addr, stChargeStep) != false)
        {
            ucCardStep = 2;
            pProtocol->SendSwitchPage(MENU_PAGE_APPLAY_ACCOUNT_INFO);
            pProtocol->SendFrameCardWait(ucCardStep);
            iPageSwitchCount = 0;
            bVinResultFlag = FALSE;
            pProtocol->SendPageCount(TO_CardStartChargeWait_Screen);
            ScreenState.st_page_value.uiPageTimeOut = TO_CardStartChargeWait_Screen;

        qInfoMap.insert(Addr_CanID_Comm, QByteArray(1,ScreenState.st_page_value.uc_can_addr));
        qInfoMap.insert(Addr_BatteryVIN_BMS,QByteArray::fromRawData(stChargeStep.sVIN, LENGTH_VIN_NO));
        qInfoMap.insert(Addr_VINApplyStartChargeType,QByteArray(1,1));//默认充满为止
        //emit sigToBus(qInfoMap,AddrType_VinApplyStartCharge);
        emit sigToBus(qInfoMap,AddrType_VinApplyStartChargeImmed);
        }
        else
        {
               pLog->getLogPoint(_strLogName)->info("ERROR! Screen VIN Apply NO ChargeStep !!");
               ShowVINApplyChargeResult(247);
        }
        //返回主界面
        //Ctrl_SwitchMain(0);
        //pProtocol->SendSwitchPage(MENU_PAGE_MAIN);
    }

}


//解析 小票机打印主界面
void cSerialScreen::ParseTicketPrint(unsigned char * pData, int iLength)   //add by zrx
{
    FrameButtonData_Screen stFrame;
    memcpy((unsigned char *)&stFrame, pData, iLength);
    unsigned char ucCmdType = pData[3];
    switch(ucCmdType)
    {
        case Type_Button_Screen:
            if((htons(stFrame.usButNum)) == 1)
            {
                if(stFrame.usPageNum == 0xFFFF)
                {
                    Ctrl_SwitchMain(0);
                    pProtocol->SendSwitchPage(MENU_PAGE_MAIN);
                }
                else if(htons(stFrame.usPageNum) == 0x0021)
                {
                    InfoMap ToCenterMap;
                    QByteArray CanID;
                    CanID.append((char)ScreenState.st_page_value.uc_can_addr);
                    ToCenterMap.insert(Addr_CanID_Comm, CanID);//can地址

                    //发送
                    SendCenterData(ToCenterMap, AddrType_MakePrintTicket);
                }
            }
            break;
        default:
            break;
    }
}

//解析 进线侧数据主界面
void cSerialScreen::ParseInLineInfoMain(unsigned char * pData, int iLength)
{
    unsigned char ucType = pData[3];
    switch(ucType)
    {
    case Type_Button_Screen:
        ParseInLineButton(pData, iLength);
        break;
    default:
        break;
    }
}

//解析 进线侧数据主界面--按钮按下
void cSerialScreen::ParseInLineButton(unsigned char * pData, int iLength)
{
    QByteArray AmmeterAddr;
    FrameButtonData_Screen stFrame;
    memcpy((unsigned char *)&stFrame, pData, iLength);
    int iAmmeterNum = 0;
    switch(htons(stFrame.usButNum))
    {
    case 1://切换至设置主页面----返回按钮
        if(stFrame.usPageNum == 0xFFFF)
        {
            Ctrl_SwitchParamMain(0);
            pProtocol->SendSwitchPage(MENU_PAGE_SET_MAIN);
            pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        }
        break;
    case 5://发送对应的电表编号1-10
        if(stFrame.usPageNum != htons(MENU_PAGE_IN_LINE_INFO))
        {
            break;
        }
        iAmmeterNum = htons(stFrame.usData1);
        AmmeterAddr.append((char *)AllAmmeterConfig.ammeterConfig.at(iAmmeterNum - 1).addr, 6);
        pProtocol->SendAmmeterAddr(AllAmmeterConfig.ammeterConfig, iAmmeterNum);
        pProtocol->SendInLineInfo(AmmeterAddr);
        break;
    default:
        break;
    }
}

//解析 环境检测主界面
void cSerialScreen::ParseEnvInfo(unsigned char * pData, int iLength)
{
    unsigned char ucType = pData[3];
    switch(ucType)
    {
    case Type_Button_Screen:
        ParseEnvInfoButton(pData, iLength);
        break;
    case Type_AlarmParamSet_Screen:
        ParseAlarmSet(pData, iLength);
        Ctrl_SwitchParamMain(0);
        pProtocol->SendSwitchPage(MENU_PAGE_SET_MAIN);
        iPageSwitchCount = 0;
        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        break;
    default:
        break;
    }
}

//解析 报警器设备设置信息----环境检测主界面
void cSerialScreen::ParseAlarmSet(unsigned char * pData, int iLength)
{
    FrameAlarmSet_Screen stFrame;
    memcpy((unsigned char *)&stFrame, pData, iLength);
    //数据赋值
    for(unsigned char i = 0; i < 10; i++)
    {
        if(stFrame.usAlarm[i]==0x0200)  //add by weiwb
        {
           stFrame.usAlarm[i]=0x0000;
        }
        IOConfig.inOpenColse[i] = (char)htons(stFrame.usAlarm[i]);
    }
    pParamSet->updateSetting(&IOConfig, PARAM_IO);
}

//解析 环境检测主界面--按钮按下
void cSerialScreen::ParseEnvInfoButton(unsigned char * pData, int iLength)
{
    FrameButtonData_Screen stFrame;
    memcpy((unsigned char *)&stFrame, pData, iLength);
    switch(htons(stFrame.usButNum))
    {
    case 1://切换至设置主页面----返回按钮
        if(stFrame.usPageNum == 0xFFFF)
        {
            Ctrl_SwitchParamMain(0);
            pProtocol->SendSwitchPage(MENU_PAGE_SET_MAIN);
            pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        }
        break;
    default:
        break;
    }
}

//解析 直流特殊功能设置主界面
void cSerialScreen::ParseDCSpecMain(unsigned char * pData, int iLength)
{
    unsigned char ucType = pData[3];
    switch(ucType)
    {
    case Type_Button_Screen:
        ParseDCSpecButton(pData, iLength);
        break;
    case Type_DCSpecSet_Screen:
        ParseDCSpecSet(pData, iLength);
        break;
    default:
        break;
    }
}

//解析 直流特殊功能设置主界面--按钮按下
void cSerialScreen::ParseDCSpecButton(unsigned char * pData, int iLength)
{
    FrameButtonData_Screen stFrame;
    memcpy((unsigned char *)&stFrame, pData, iLength);
    switch(htons(stFrame.usButNum))
    {
    case 1://切换至设置主页面----返回按钮
        if(stFrame.usPageNum == 0xFFFF)
        {
            Ctrl_SwitchParamMain(0);
            pProtocol->SendSwitchPage(MENU_PAGE_SET_MAIN);
            pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        }
        break;
    default:
        break;
    }
}
//解析 直流特殊功能设置
void cSerialScreen::ParseDCSpecSet(unsigned char * pData, int iLength)
{
    iLength = 0;
    InfoMap ToCenterMap;
    QByteArray tempArray;
    FrameDCSpecSet_Screen stFrame;
//    if(iLength != sizeof(stFrame))
//    {
//        return;
//    }
    memcpy((unsigned char * )&stFrame, pData, sizeof(stFrame));
    tempArray.append((char)(htons(stFrame.sGroupStrategy)));
    ToCenterMap.insert(Addr_GroupStrategy_GDA, tempArray);
    tempArray.clear();
    tempArray.append((char)(htons(stFrame.sTermWorkState)));
    ToCenterMap.insert(Addr_WorkState_GDA, tempArray);
    tempArray.clear();
    tempArray.append((char)(htons(stFrame.usCanID)));
    ToCenterMap.insert(Addr_CanID_Comm, tempArray);
    tempArray.clear();
    SendCenterData(ToCenterMap, AddrType_GeneralDynamicArgApply);

    ToCenterMap.clear();
    tempArray.clear();
    tempArray.append((char)(htons(stFrame.sAuxType)));
    ToCenterMap.insert(Addr_AuxPowerType_GSA, tempArray);
    tempArray.clear();
    tempArray.append((char)(htons(stFrame.usCanID)));
    ToCenterMap.insert(Addr_CanID_Comm, tempArray);
    SendCenterData(ToCenterMap, AddrType_GeneralStaticArgApply);
}

//解析 负荷调度选择界面
void cSerialScreen::ParseFrameLoadDispatchMain(unsigned char * pData, int iLength)
{
    unsigned char ucType = pData[3];
    switch(ucType)
    {
    case Type_Button_Screen:    //接收按钮指令
        ParseLoadDispatchButton(pData, iLength);
        break;
    case Type_LoadDispatchEnable_Screen:   //接收负荷调度,错峰充电使能信息指令
        ParseLoadDispatchEnable(pData, iLength);
        break;
    case Type_PeakSet_Screen:   //接收错峰充电设置指令
        ParseFramePeakSet(pData, iLength);
        iPageSwitchCount = 0;
        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        break;
    case Type_LoadLimit_Screen://接收负荷限制功能设置指令
        ParseFrameLoadLimit(pData, iLength);
        Ctrl_SwitchParamMain(0);
        pProtocol->SendSwitchPage(MENU_PAGE_SET_MAIN);
        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        break;
    default:
        break;
    }
}

//解析 负荷调度选择界面--按钮按下
void cSerialScreen::ParseLoadDispatchButton(unsigned char * pData, int iLength)
{
    FrameButtonData_Screen stFrame;
    memcpy((unsigned char *)&stFrame, pData, iLength);
    switch(htons(stFrame.usButNum))
    {
    case 1:
        //切换至设置主页面----返回按钮
        if(stFrame.usPageNum == 0xFFFF)
        {
            switch (iPageLoadDispatchChoose) {
            case 3://返回 页面04H，设置主页面
                iPageLoadDispatchChoose = 0;
                ScreenState.us_page_num = MENU_PAGE_SET_MAIN;
                ScreenState.st_page_value.uc_can_addr = 0;
                ScreenState.st_page_value.uc_hold_time = 30;
                ScreenState.st_page_value.uc_page_num_return = MENU_PAGE_SET_MAIN;
                ScreenState.st_page_value.uiPageTimeOut = TO_ParamMain_Screen;//超时时间30s
                iPageSwitchCount = 0;//计数器清零
                bPageWaitFlag = FALSE;// 状态机切换, 取消等待处理

                //切换至错峰充电设置界面
                pProtocol->SendSwitchPage(MENU_PAGE_SET_MAIN);
                pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
//                //发送错峰充电页面数据
//                pProtocol->SendPeakSetPage(AllTPFVConfig, 1);
//                pProtocol->SendPeakSetPage(AllTPFVConfig, 2);
                break;
            default:
                Ctrl_SwitchParamMain(0);
                pProtocol->SendSwitchPage(MENU_PAGE_SET_MAIN);
//                pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
                pProtocol->SendMainPageData(NameMap, cscuSysConfig, cscuSysConfig.version,NameMapShow);
                pProtocol->SendPageCount(30); //给TEUI发送30秒倒计时 add by zrx
                break;
            }

        }
        //进入错峰充电设置界面
        else if(stFrame.usPageNum == htons(MENU_PAGE_PEAK_STAGGER))
        {
            QueryParamInfo();
            //发送 错峰充电详细设置数据----负荷调度界面
            iPageSwitchCount = 0;
            ScreenState.st_page_value.uiPageTimeOut = TO_PeakSet_Screen;
            pProtocol->SendSwitchPage(MENU_PAGE_PEAK_SET);
            iPageLoadDispatchChoose = 3;
            pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
            for(unsigned char i = 1; i < 5; i++)
            {
                pProtocol->SendPeakSetDetail(AllTPFVConfig, i);
            }
        }
        //点击设置保存,发送数据到总线
        else if(stFrame.usPageNum == htons(MENU_PAGE_PEAK_SET_END))
        {
            SendPeakSet();
        }
        break;
    default:
        break;
    }
}

//解析 负荷调度,错峰充电使能信息指令
void cSerialScreen::ParseLoadDispatchEnable(unsigned char * pData, int iLength)
{
    iLength = 0;
    FrameLoadEnable_Screen stFrame;
    memcpy((unsigned char *)&stFrame, pData, sizeof(stFrame));
    QueryParamInfo();
    if(stFrame.usOptFlag == htons(0x01))//错峰功能
    {
        if(stFrame.usEnableFlag == htons(0x00)) //关闭
        {
            SmartChargeConfig.sSmartCharge_Enable = FALSE;
            pParamSet->updateSetting(&SmartChargeConfig, PARAM_SMARTCHARGE);
            Ctrl_SwitchParamMain(0);
            pProtocol->SendSwitchPage(MENU_PAGE_SET_MAIN);
            pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        }
        else if(stFrame.usEnableFlag == htons(0x01)) //开启
        {
            SmartChargeConfig.sSmartCharge_Enable = TRUE;
            pParamSet->updateSetting(&SmartChargeConfig, PARAM_SMARTCHARGE);
            //切换至错峰充电设置界面
            pProtocol->SendSwitchPage(MENU_PAGE_PEAK_STAGGER);
            //发送错峰充电页面数据
            pProtocol->SendPeakSetPage(AllTPFVConfig, 1);
            pProtocol->SendPeakSetPage(AllTPFVConfig, 2);
            //发送 错峰充电状态提示图标
            pProtocol->SendPeakAutoIcon(ChargeConfig.vinAuto);
            //超时时间1min
            iPageSwitchCount = 0;
            ScreenState.st_page_value.uiPageTimeOut = 60;
            pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        }
    }
    else if(stFrame.usOptFlag == htons(0x02))//负荷功能
    {
        if(stFrame.usEnableFlag == htons(0x00)) //关闭
        {
            PowerLimitConfig.sPowerLimit_Enable = FALSE;
            pParamSet->updateSetting(&PowerLimitConfig, PARAM_POWERLIMIT);
            Ctrl_SwitchParamMain(0);
            pProtocol->SendSwitchPage(MENU_PAGE_SET_MAIN);
            pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        }
        else if(stFrame.usEnableFlag == htons(0x01)) //开启
        {
            PowerLimitConfig.sPowerLimit_Enable = TRUE;
            pParamSet->updateSetting(&PowerLimitConfig, PARAM_POWERLIMIT);
            //切换至负荷约束设置界面
            pProtocol->SendSwitchPage(MENU_PAGE_LOAD_LIMIT);
            pProtocol->SendLoadLimit(PowerLimitConfig);
            //超时时间1min
            iPageSwitchCount = 0;
            ScreenState.st_page_value.uiPageTimeOut = 90;
            ScreenState.st_page_value.uiPageTimeOut = 60;
            pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        }
    }
}

//解析 错峰充电设置界面
void cSerialScreen::ParseFramePeakSet(unsigned char * pData, int iLength)
{
    unsigned short usType;
    FramePeakShowDetail_Screen stFrame;
    memcpy((unsigned char *)&stFrame, pData, iLength);
    usType = htons(stFrame.usAddr);
    //屏的峰平谷尖枚举和总线枚举转换
    switch(htons(stFrame.usAddr))
    {
    case 1:
        usType = 1;
        break;
    case 2:
        usType = 2;
        break;
    case 3:
        usType = 3;
        break;
    case 4:
        usType = 4;
        break;
    default:
        usType = 1;
        break;
    }

    for(unsigned char i = 0; i < 5; i++)
    {
        PeakSet[i + 5*(usType - 1)].time_seg = usType;
        PeakSet[i + 5*(usType - 1)].start_hour = htons(stFrame.stRecord[i].usStartH);
        PeakSet[i + 5*(usType - 1)].start_minute = htons(stFrame.stRecord[i].usStartM);
        PeakSet[i + 5*(usType - 1)].stop_hour = htons(stFrame.stRecord[i].usStopH);
        PeakSet[i + 5*(usType - 1)].stop_minute = htons(stFrame.stRecord[i].usStopM);
        PeakSet[i + 5*(usType - 1)].limit_soc = htons(stFrame.stRecord[i].usSOC);
        PeakSet[i + 5*(usType - 1)].limit_current = htons(stFrame.stRecord[i].usCurrent);
    }
}

//解析 负荷限制功能设置指令
void cSerialScreen::ParseFrameLoadLimit(unsigned char * pData, int iLength)
{
    FrameLoadLimit_Screen stFrame;
    memcpy((unsigned char *)&stFrame, pData, iLength);
    PowerLimitConfig.sCCUcount  = htons(stFrame.usCCUNum);
    PowerLimitConfig.STATION_LIMT_POWER = htons(stFrame.usTotalPower);
    PowerLimitConfig.SAFE_CHARGE_POWER = htons(stFrame.usSecurePower);
    PowerLimitConfig.sSUMPower_Manual = htons(stFrame.usLimitPower);
    PowerLimitConfig.sSUMPower_Ammeter_Enable = htons(stFrame.usDynamicEnable);
    PowerLimitConfig.sSUMPower_Manual_Enable = htons(stFrame.usLocalEnable);
    PowerLimitConfig.sSUMPower_Server_Enable = htons(stFrame.usRemoteEnable);
    //更新配置文件
    pParamSet->updateSetting(&PowerLimitConfig, PARAM_POWERLIMIT);
}

//解析 故障列表主界面
void cSerialScreen::ParseFaultInfo(unsigned char * pData, int iLength)
{
    unsigned char ucType = pData[3];
    switch(ucType)
    {
    case Type_Button_Screen:
        ParseFaultInfoButton(pData, iLength);
        break;
    default:
        break;
    }
}

//解析 故障列表界面--按钮按下
void cSerialScreen::ParseFaultInfoButton(unsigned char * pData, int iLength)
{
    FrameButtonData_Screen stFrame;
    memcpy((unsigned char *)&stFrame, pData, iLength);
    switch(htons(stFrame.usButNum))
    {
    case 1://切换至设置主页面----返回按钮
        if(stFrame.usPageNum == 0xFFFF)
        {
            Ctrl_SwitchMain(0);
            pProtocol->SendSwitchPage(MENU_PAGE_MAIN);
            pProtocol->SendMainPageData(NameMap, cscuSysConfig, cscuSysConfig.version,NameMapShow);
        }
        break;
    default:
        break;
    }
}

//解析 U盘操作页面--按钮按下
void cSerialScreen::ParseUDiskButton(unsigned char * pData, int iLength)
{
    FrameButtonData_Screen stFrame;
    memcpy((unsigned char *)&stFrame, pData, iLength);
    switch(htons(stFrame.usButNum))
    {
    case 1://返回按钮, 导出日志
        if(stFrame.usPageNum == 0xFFFF)//切换至主页面----返回按钮
        {
            Ctrl_SwitchMain(0);
            pProtocol->SendSwitchPage(MENU_PAGE_MAIN);
            pProtocol->SendMainPageData(NameMap, cscuSysConfig, cscuSysConfig.version,NameMapShow);
        }
        else//切换至日志导出中
        {
            iPageSwitchCount = 0;
            pProtocol->SendSwitchPage(MENU_PAGE_LOG_OUT);
            ScreenState.st_page_value.uiPageTimeOut = TO_LogOut_Screen;
            pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
            SendLogOutApply();
        }
        break;
    case 2://升级
        iPageSwitchCount = 0;
        pProtocol->SendSwitchPage(MENU_UPDATE_PROGRAM);
        ScreenState.st_page_value.uiPageTimeOut = TO_UpdateProgram_Screen;
        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        SendUpdateApply();
        break;
    default:
        break;
    }
}

//解析 刷卡主页面
void cSerialScreen::ParseCardMain(unsigned char * pData, int iLength)
{
    iLength = 0;
    unsigned char ucType = pData[3];
    switch(ucType)
    {
    case Type_CardStartCharge_Screen://刷卡开始充电
        iPageSwitchCount = 0;
        pProtocol->SendSwitchPage(MENU_UPDATE_PROGRAM);
        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        break;
    default:
        break;
    }
}

//解析总线 接收到U盘处理结果
bool cSerialScreen::ParseCenterUdiskResult(InfoMap CenterMap)
{
    unsigned char ucSource, ucType, ucResult;
    if(CenterMap.isEmpty())
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! CenterMap Empty");

        return FALSE;
    }
    ucSource = CenterMap[Addr_Cmd_Source].data()[0];
    ucType = CenterMap[Addr_Cmd_Master].data()[0];
    ucResult = CenterMap[Addr_Back_Result].data()[0];
    if(ucSource == 2)//U盘
    {
        if(ucType == 1)//升级
        {
            if(ucResult == 1)//成功
            {
                pProtocol->SendSwitchPage(MENU_UPDATE_SUCCESS);
            }
            else
            {
                pProtocol->SendSwitchPage(MENU_UPDATE_FAILED);
            }
        }
        else if(ucType == 2)//日志
        {
            if(ucResult == 1)//成功
            {
                pProtocol->SendSwitchPage(MENU_PAGE_LOG_OUT_SUCCSESS);
            }
            else
            {
                pProtocol->SendSwitchPage(MENU_PAGE_LOG_OUT_FAILED);
            }
        }
        iPageSwitchCount = 0;
        ScreenState.st_page_value.uiPageTimeOut = TO_UDiskResult_Screen;
        pProtocol->SendPageCount(TO_UDiskResult_Screen);
        return TRUE;
    }
    return FALSE;
}

//本地设置错峰充电参数设置结果
bool cSerialScreen::ParseCenterPeakSetResult(InfoMap CenterMap)
{
    unsigned char ucResult = 0;
    if(CenterMap.isEmpty()){
        pLog->getLogPoint(_strLogName)->info("ERROR! CenterMap Empty");
        return FALSE;
    }

    ucResult = CenterMap[Addr_SmartChargeSet_Result].at(0);
	Ctrl_SwitchPage(MENU_PAGE_SET_SPEC_RESULT, TO_SpecFuncSetResult_Screen, false, ScreenState.us_page_num);
    if(ucResult == 0xFF){
    	pProtocol->SendShowIcon(ICON_SET_SPEC_SUCCESS);
	}else{
       	pProtocol->SendShowIcon(ICON_SET_SPEC_FAILED);
	}

	GetSpecCFCD();
	pProtocol->SendSpecCFCD(AllTPFVConfig, 1);

    return TRUE;
}

//解析 总线接收刷卡卡号
bool cSerialScreen::ParseCenterCardNumber(InfoMap CenterMap)
{
    pLog->getLogPoint(_strLogName)->info("ParseCenterCardNumber");
    if(CenterMap.isEmpty())
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! CenterMap Empty");
        return FALSE;
    }
    if(CenterMap.contains(Addr_CardAccount))
    {
        CardNum = CenterMap[Addr_CardAccount];
        ScanCodeIDNum.clear();
    }
    else if(CenterMap.contains(Addr_ScanCode_customerID))
    {
        ScanCodeIDNum = CenterMap[Addr_ScanCode_customerID];
        CardNum.clear();
    }
    pProtocol->SendSwitchPage(MENU_PAGE_APPLAY_ACCOUNT_INFO);
    pProtocol->SendFrameCardWait(ucCardStep);
    pProtocol->SendPageCount(TO_ApplyAccountInfo_Screen);
    ScreenState.st_page_value.uiPageTimeOut = TO_ApplyAccountInfo_Screen;
    iPageSwitchCount = 0;
    return TRUE;
}

//解析 总线接收账户信息
bool cSerialScreen::ParseCenterAccountInfo(InfoMap CenterMap)
{
    if(CenterMap.isEmpty())
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! CenterMap Empty");
        return FALSE;
    }
    else//账户信息列表显示
    {
		//南京3.0协议余额显示
		if(CenterMap.contains(Addr_Account_Balance)){
			uint iBalance;
			pProtocol->SendSwitchPage(MENU_PAGE_NORMAL_ACCOUNT);
			pProtocol->SendPageCount(TO_ShowAccountInfo_Screen);
//			iBalance = *(float*)CenterMap[Addr_Account_Balance].data() * 100;
            iBalance = *((uint *)CenterMap[Addr_Account_Balance].data());
            pProtocol->SendFrameAccountBalance(iBalance);
			ScreenState.st_page_value.uiPageTimeOut = TO_ShowAccountInfo_Screen;
		}

        if(CenterMap.contains(Addr_Account_Detail)){    //3.0协议账户信息
            pProtocol->SendSwitchPage(MENU_PAGE_NORMAL_ACCOUNT);
            pProtocol->SendPageCount(TO_ShowAccountInfo_Screen);
            pProtocol->SendFrameAccountBalance(*(float*)CenterMap[Addr_Account_Detail].data());
            ScreenState.st_page_value.uiPageTimeOut = TO_ShowAccountInfo_Screen;
		}

        if(CenterMap.contains(Addr_CardAccountList))    //账户信息
        {
            AccountInfo stInfo;
            memcpy((char *)&stInfo, CenterMap[Addr_CardAccountList].data(), CenterMap[Addr_CardAccountList].length());
            QString strlog = QString("stInfo.stAccount[0].uiValue is : %1").arg(stInfo.stAccount[0].uiValue);
            pLog->getLogPoint(_strLogName)->info(strlog);
            iPageSwitchCount = 0;

            //目前仅处理现金账户余额
            if(stInfo.stAccount[0].uiValue == 0x7FFFFFFF)
            {
                if(bBanlanceFlag)//余额查询
                {
                    bBanlanceFlag = FALSE;
                    pProtocol->SendSwitchPage(MENU_PAGE_POST_PAID_ACCOUNT_BALANCE_ENQUIRY);
                }
                else //主界面点击“连接就绪”
                {
                    //                pProtocol->SendSwitchPage(MENU_PAGE_APPLAY_ACCOUNT_INFO);
                    pProtocol->SendSwitchPage(MENU_PAGE_POST_PAID_ACCOUNT);
                }
                pProtocol->SendPageCount(TO_ShowAccountInfo_Screen);
                ScreenState.st_page_value.uiPageTimeOut = TO_ShowAccountInfo_Screen;
            }
            else
            {
                if(bBanlanceFlag)//余额查询
                {
                    bBanlanceFlag = FALSE;
                    pProtocol->SendSwitchPage(MENU_PAGE_NORMAL_ACCOUNT__BALANCE_ENQUIRY);
                }
                else //主界面点击“连接就绪”
                {
                    pProtocol->SendSwitchPage(MENU_PAGE_NORMAL_ACCOUNT);
                }

                pProtocol->SendPageCount(TO_ShowAccountInfo_Screen);
                pProtocol->SendFrameAccountBalance(stInfo.stAccount[0].uiValue);
                ScreenState.st_page_value.uiPageTimeOut = TO_ShowAccountInfo_Screen;
            }
        }
    }
    return TRUE;
}

//解析 总线接收刷卡开始充电/结束充电结果----外部结果(充电服务模块返回), ucType: 1, 开始充电; 2, 结束充电
bool cSerialScreen::ParseCenterCardOutResult(InfoMap CenterMap, unsigned char ucType)
{
    if(CenterMap.isEmpty())
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! CenterMap Empty");
        return FALSE;
    }
    else//显示结果
    {
        unsigned char ucCanID = 0;
        unsigned char ucResult = 0;
        QByteArray tempName;
        QByteArray tempScanCodeName;
        QByteArray retArray;

        ucCanID = (unsigned char)CenterMap[Addr_CanID_Comm].at(0);//CAN地址

        if(ucType == 1)
        {
            if(CenterMap.contains(Addr_CardApplyCharge_Result))
                ucResult = (unsigned char)CenterMap[Addr_CardApplyCharge_Result].at(0);//开始充电结果
            else if(CenterMap.contains(Addr_ScanCode_StartCharge_Result))
                ucResult = (unsigned char)CenterMap[Addr_ScanCode_StartCharge_Result].at(0);//开始充电结果(扫码)
        }
        else if(ucType == 2)
        {
            if(CenterMap.contains(Addr_CardStopCharge_Result))
                ucResult = (unsigned char)CenterMap[Addr_CardStopCharge_Result].at(0);//结束充电结果
            else if(CenterMap.contains(Addr_ScanCode_StopCharge_Result))
                ucResult = (unsigned char)CenterMap[Addr_ScanCode_StopCharge_Result].at(0);//结束充电结果(扫码)
        }
        tempName = CenterMap[Addr_CardAccount];//卡号
        tempScanCodeName = CenterMap[Addr_ScanCode_customerID];//扫码customerID
        //CAN地址校验
        if((ucCanID == ScreenState.st_page_value.uc_can_addr)||(ScreenState.st_page_value.uc_can_addr == 0))
        {
			//在用另一张卡结束充电时，需给用户进行提示卡号校验失败，因此删除卡号校验，屏幕只提供显示功能即可
            //if(tempName == CardNum)
            {
                //将结果显示到屏幕
                retArray = GetCardResult(ucResult, ucType);
                pProtocol->SendSwitchPage(MENU_PAGE_CARD_RESULT);
                pProtocol->SendFrameCardResult(retArray);
                pProtocol->SendPageCount(TO_CardStartStopResult_Screen);
                //超时时间置为3s
                ScreenState.st_page_value.uiPageTimeOut = TO_CardStartStopResult_Screen;
                iPageSwitchCount = 0;
                bCardResultFlag = TRUE;
            }
        }
    }
    return TRUE;
}

//解析 总线接收刷卡内部申请, 开始充电, 结束充电结果 ---- 内部结果(充电模块返回), ucType: 1, 开始充电; 2, 结束充电
bool cSerialScreen::ParseCenterCardInResult(InfoMap CenterMap, unsigned char ucType)
{
    if(CenterMap.isEmpty())
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! CenterMap Empty");
        return FALSE;
    }
    else//显示结果
    {
        unsigned char ucCanID = 0;
        unsigned char ucResult = 0;
        QByteArray tempCardNum;
        QByteArray tempScanCodeNum;
        QByteArray retArray;

        ucCanID = (unsigned char)CenterMap[Addr_CanID_Comm].at(0);//CAN地址

        if(ucType == 1)//开始充电
        {
            ucResult = (unsigned char)CenterMap[Addr_InApplyStartCharge_Result].at(0);//内部申请结果
        }
        else if(ucType == 2)//结束充电
        {
            ucResult = (unsigned char)CenterMap[Addr_InApplyStopCharge_Result].at(0);//内部结束结果
        }
        tempCardNum = CenterMap[Addr_CardAccount];//卡号
        tempScanCodeNum = CenterMap[Addr_ScanCode_customerID];//扫码customerID
        //CAN地址校验
        if(ucCanID == ScreenState.st_page_value.uc_can_addr)
        {
            //卡号校验
            if(tempCardNum == CardNum)
            {
                //将结果显示到屏幕, 0x01: 失败, 0xFF, 成功
                if(ucResult == 0x01)//失败
                {
                    pProtocol->SendSwitchPage(MENU_PAGE_CARD_RESULT);
                    retArray = "内部刷卡不允许";
                    pProtocol->SendFrameCardResult(retArray);
                    pProtocol->SendPageCount(TO_CardStartStopResult_Screen);
                    //超时时间置为5s
                    ScreenState.st_page_value.uiPageTimeOut = TO_CardStartStopResult_Screen;
                    iPageSwitchCount = 0;
                    bCardResultFlag = TRUE;
                }
                else if(ucResult == 0xFF)//成功
                {
                    iPageSwitchCount = 0;
                    ScreenState.st_page_value.uiPageTimeOut = TO_CardStartChargeWait_Screen;
                }else
                {
                    pProtocol->SendSwitchPage(MENU_PAGE_CARD_RESULT);
                    //retArray = "内部刷卡不允许";
                     retArray = GetCardResult(ucResult, ucType);
                    pProtocol->SendFrameCardResult(retArray);
                    pProtocol->SendPageCount(TO_CardStartStopResult_Screen);
                    //超时时间置为5s
                    ScreenState.st_page_value.uiPageTimeOut = TO_CardStartStopResult_Screen;
                    iPageSwitchCount = 0;
                    bCardResultFlag = TRUE;
                }
            }
            else if(tempScanCodeNum == ScanCodeIDNum) //扫码customerID 验证
            {
                //将结果显示到屏幕, 0x01: 失败, 0xFF, 成功
                if(ucResult == 0x01)//失败
                {
                    pProtocol->SendSwitchPage(MENU_PAGE_CARD_RESULT);
                    retArray = "内部刷卡不允许";
                    pProtocol->SendFrameCardResult(retArray);
                    pProtocol->SendPageCount(TO_CardStartStopResult_Screen);
                    //超时时间置为5s
                    ScreenState.st_page_value.uiPageTimeOut = TO_CardStartStopResult_Screen;
                    iPageSwitchCount = 0;
                    bCardResultFlag = TRUE;
                }
                else if(ucResult == 0xFF)//成功
                {
                    iPageSwitchCount = 0;
                    ScreenState.st_page_value.uiPageTimeOut = TO_CardStartChargeWait_Screen;
                }
            }
        }
    }

    return TRUE;
}

//解析 总线接收VIN内部申请开始充电的结果
bool cSerialScreen::ParseCenterVINResult(InfoMap CenterMap)
{
    unsigned char ucResult = CenterMap[Addr_VINViaApplyStartCharge_Result].at(0);
    if(ucResult == 0xFF)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//解析 总线接收终端名称
bool cSerialScreen::ParseCenterTermName(InfoMap CenterMap)
{
    unsigned char ucCanID = 0;
    unsigned char ucTermNum = 0;
    unsigned char ucSinNum = 0;
    unsigned char ucThrNum = 0;
    unsigned char ucDCNum = 0;
    QByteArray nameArray, totalArray, strName;
    TermNameMap newNameMap;
    QString DBExec;
    TermNameMap::iterator it;
    //写入站名称----GBK 转换为 UTF-8
    QByteArray TempArray;
    QString TempString;
    QTextCodec * pGBK = QTextCodec::codecForName("GBK");

    QueryParamInfo();

    //获取子站名称
    if(CenterMap.contains(Addr_TermName_Adj))
    {
        TempArray = CenterMap[Addr_TermName_Adj];
        TempString = pGBK->toUnicode(TempArray.data(), TempArray.length());
        strName = TempString.toAscii();
        strncpy(cscuSysConfig.stationName, strName.data(), strName.length());
        pParamSet->updateSetting(&cscuSysConfig, PARAM_CSCU_SYS);
    }
    //CAN地址和终端编号对应关系
    if(CenterMap.contains(Addr_TermIndex_Adj))
    {
        ucTermNum = CenterMap[Addr_TermIndex_Adj].length()/11;
        totalArray = CenterMap[Addr_TermIndex_Adj];
        for(unsigned char i = 0 ; i < ucTermNum; i++)
        {
            nameArray.clear();
            ucCanID = *(totalArray.data() + i*11);
            nameArray.append(totalArray.data() + 1 + i*11, 10);
            TempString = pGBK->toUnicode(nameArray.data(), nameArray.length());
            nameArray = TempString.toAscii();

            newNameMap.insert(ucCanID, nameArray);
            if((ucCanID >= ID_MinACSinCanID) && (ucCanID <= ID_MaxACSinCanID))
            {
                ucSinNum++;
            }
            else if((ucCanID >= ID_MinACThrCanID) && (ucCanID <= ID_MaxACThrCanID))
            {
                ucThrNum++;
            }
            else if((ucCanID >= ID_MinDCCanID) && (ucCanID <= ID_MaxDCCanID))
            {
                ucDCNum++;
            }
        }
        //校验各类型终端数量
        if(     (ucSinNum != cscuSysConfig.singlePhase)
            ||(ucThrNum != cscuSysConfig.threePhase)
            ||(ucDCNum != cscuSysConfig.directCurrent)    )
        {
            return FALSE;
        }
        //删除当前表中所有内容
        QString todo = "DELETE FROM terminal_name_table";
        pDBOperate->DBSqlExec(todo.toAscii().data(), DB_PARAM);
        NameMap = newNameMap;
        //插入数据库
        for(it = newNameMap.begin(); it!= newNameMap.end(); ++it)
        {
            DBExec = "INSERT INTO terminal_name_table ( canaddr, name)  VALUES( " + QString::number(it.key(), 10) + " , " + " '" + it.value() + "' " + " ) ";
            pDBOperate->DBSqlExec(DBExec.toAscii().data(), DB_PARAM);

           // ParseCenterTermMultiName();
            DBExec = QString("UPDATE  terminal_name_multi_table  SET name='%1' WHERE canaddr ='%2' AND multitype !=2").arg(it.value().data() ).arg(it.key() );
            pDBOperate->DBSqlExec(DBExec.toAscii().data(), DB_PARAM);
        }
    }
    return TRUE;
}

//终端状态从缓存中获取, 和屏幕打印的状态枚举值不同
void cSerialScreen::ParseFrameTermState(unsigned char * pData, int iLength)
{
    FrameGetTermState_Screen stTemp;
    unsigned char ucCanID;
    memcpy((unsigned char *)&stTemp, pData, iLength);
    ucCanID = (unsigned char) htons(stTemp.usCanID);

    //判断是否为终端故障状态, 切换至故障列表界面
    if(Term_Fault_Screen == htons(stTemp.usState) || (Term_Bow_Fault_Screen == htons(stTemp.usState)))  //hd 2018-3-28 充电弓图标的状态码
    {
        Ctrl_SwitchFaultInfo(0);
        pProtocol->SendSwitchPage(MENU_PAGE_FAULT_INFO);
        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        pProtocol->SendFaultInfoList(NameMap);
        iPageSwitchCount = 0;
    }
    else
    {
        //终端状态判断
        GetTermState(ucCanID);
    }
}

//解析  TEUI版本号
void cSerialScreen::ParseFrameTEUIVer(unsigned char * pData)
{
    Frame_TEUIVer_Screen stFrame;
    stScreenConfig screenConfig;
    memcpy(&stFrame, pData, sizeof(Frame_TEUIVer_Screen));
    if(pParamSet->querySetting(&screenConfig, PARAM_SCREEN))
    {
        strncpy(screenConfig.version, stFrame.Ver, sizeof(screenConfig.version) -1);
        pParamSet->updateSetting(&screenConfig, PARAM_SCREEN);
//        for(int i = 0 ; i < sizeof(Frame_TEUIVer_Screen); i++)
//        {
//        }
    }
}

//点击终端状态逻辑----刷卡版
void cSerialScreen::CheckTermState_Card(TerminalStatus &stTerm)
{
    stChargeConfig ChargeConfig_temp;
    pParamSet->querySetting(&ChargeConfig_temp,PARAM_CHARGE);
    if((stTerm.gunType == SLAVE_GUN || stTerm.gunType == COUPLE_ERR) && (ChargeConfig_temp.coupleGun !=0))//配对错误／副枪不响应点击
        return;
    bCardResultFlag = TRUE;
    bVinResultFlag = TRUE;
    switch(stTerm.cStatus)
    {
    case CHARGE_STATUS_DISCONNECT: //离线-未通信
    {
        bBanlanceFlag = FALSE;
        Ctrl_SwitchCardMain(stTerm.cCanAddr);
        ScreenState.st_page_value.uiPageTimeOut = TO_TermAbnormalStatus_Screen;
        pProtocol->SendSwitchPage(MENU_PAGE_PROMPT_OFFLINE);
        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        break;
    }
    case CHARGE_STATUS_FREE: //待机-空闲
    {
        bBanlanceFlag = FALSE;
        Ctrl_SwitchCardMain(stTerm.cCanAddr);
        ScreenState.st_page_value.uiPageTimeOut = TO_TermAbnormalStatus_Screen;
        pProtocol->SendSwitchPage(MENU_PAGE_PROMPT_OFFLINK);
        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        break;
    }
    case CHARGE_STATUS_GUN_STANDBY : //待机-枪已连接
    {
        bBanlanceFlag = FALSE;
        ucCardStep = 1;
//        stChargeConfig ChargeConfig_temp;
//        pParamSet->querySetting(&ChargeConfig_temp,PARAM_CHARGE);
        //仅针对直流设备，交流设备不处理
        if((ChargeConfig_temp.coupleGun != 0) && (stTerm.cCanAddr > ID_MaxACThrCanID) && (stTerm.cCanAddr < ID_MinCCUCanID))//双枪充电功能使能
        {
             pLog->getLogPoint(_strLogName)->info(QString("gun_standby couplegun enable  couplegun=%1").arg(ChargeConfig_temp.coupleGun));
//            if(ChargeConfig.coupleGun == 1)//单双枪刷卡充电
//            {
//                Ctrl_SwitchCardCharge_Normal(stTerm.cCanAddr);
//                pProtocol->SendSwitchPage(MENU_PAGE_CARD_CHARGE_NORMAL);
//            }
//            else
            if(ChargeConfig_temp.coupleGun == 2 )//&& stTerm.chargeManner == SINGLE_CHARGE)//单枪vin充电
            {
                Ctrl_SwitchVINManualCharge_Normal(stTerm.cCanAddr);
                pProtocol->SendSwitchPage(MENU_PAGE_VIN_MANUAL_CHARGE);
            }
            else if(ChargeConfig_temp.coupleGun == 3)//单双枪刷卡／单枪vin充电
            {
                Ctrl_SwitchVINCardCharge_Normal(stTerm.cCanAddr);
                pProtocol->SendSwitchPage(MENU_PAGE_VIN_CARD_CHARGE_NORMAL);
            }
            else
            {
                Ctrl_SwitchCardMain(stTerm.cCanAddr);
                SendCardNumApply();;
                pProtocol->SendSwitchPage(MENU_PAGE_SWIPE_CARD_MAIN);
              //  pProtocol->SendTermPageData(stTerm.cCanAddr, (unsigned char)cscuSysConfig.normalCardType, ThreePhaseTypeConfig);
            }
        }
        else//双枪充电功能未开启
        {
            Ctrl_SwitchCardMain(stTerm.cCanAddr);
            SendCardNumApply();
            pProtocol->SendSwitchPage(MENU_PAGE_SWIPE_CARD_MAIN);
           // pProtocol->SendTermPageData(stTerm.cCanAddr, (unsigned char)cscuSysConfig.normalCardType, ThreePhaseTypeConfig);
        }
        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        break;
    }
    case CHARGE_STATUS_CHARGING: //充电-充电中, 切换充电详情界面
    {
        //add by YCZ 2017-04-06 增加对结束按钮的判断
        if(ChargeConfig.localStop == 0) {//不支持本地结束
            pProtocol->SendSwitchPage(MENU_PAGE_TERM_INFO_CARD);
        }
        else{
            pProtocol->SendSwitchPage(MENU_PAGE_TERM_INFO_CARD_STOP_BUTTON);
        }
        ucCardStep = 3;
        Ctrl_SwitchTermDetail_Card(stTerm.cCanAddr);
//        pProtocol->SendSwitchPage(MENU_PAGE_TERM_INFO_CARD);
        pProtocol->SendTermDetailData_Card(stTerm, ThreePhaseTypeConfig);
        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        SendCardNumApply();

        break;
    }
    case CHARGE_STATUS_FINISH: //待机-已完成
    case CHARGE_STATUS_FULL:    //待机-车已充满
    {
        bCardResultFlag = TRUE;
        bVinResultFlag = TRUE;
        Ctrl_SwitchChargeReport_Card(stTerm.cCanAddr);
        pProtocol->SendSwitchPage(MENU_PAGE_CHARGE_CARD_REPORT);
        pProtocol->SendTermChargeReport(stTerm.cCanAddr);
        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        break;
    }
    case CHARGE_STATUS_DISCHARGING: //放电
        break;
    case CHARGE_STATUS_STARTING : //启动中
    case CHARGE_STATUS_LIMIT : //充电-限制
    case CHARGE_STATUS_PAUSH : //充电-暂停
    case CHARGE_STATUS_SWITCH: //待机-切换中
        break;
    case CHARGE_STATUS_FAULT: //故障
        break;
    default:
        break;
    }
}

//点击终端状态逻辑----普通版
void cSerialScreen::CheckTermState_Normal(TerminalStatus &stTerm)
{
    switch(stTerm.cStatus)
    {
    case CHARGE_STATUS_FINISH: //待机-已完成
    case CHARGE_STATUS_FULL://13待机-车已充满 A
        Ctrl_SwitchChargeReport_Normal(stTerm.cCanAddr);
        pProtocol->SendSwitchPage(MENU_PAGE_CHARGE_CARD_REPORT);
        pProtocol->SendTermChargeReport(stTerm.cCanAddr);
        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        break;
    case CHARGE_STATUS_GUN_STANDBY : //待机-枪已连接
        if(ChargeConfig.vinOffline == 1)    //允许离线充电
        {
            ucVINResultStep = 0;
            strncpy(chVINCach, (char *)stTerm.stFrameBmsInfo.BMS_car_VIN, sizeof(chVINCach));
            Ctrl_SwitchVINStart(stTerm.cCanAddr);
            pProtocol->SendSwitchPage(MENU_PAGE_VIN_START_CHARGE);
            pProtocol->SendTermVINInfo(stTerm);
            pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        }
        else    //不允许离线充电
        {
            Ctrl_SwitchTermDetail_Normal(stTerm.cCanAddr);
            pProtocol->SendSwitchPage(MENU_PAGE_TERM_INFO_NORMAL);
            pProtocol->SendTermPageData(stTerm.cCanAddr, (unsigned char)cscuSysConfig.normalCardType, ThreePhaseTypeConfig);
            pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        }
        break;
    case CHARGE_STATUS_DISCONNECT: //离线-未通信
        Ctrl_SwitchTermDetail_Normal(stTerm.cCanAddr);
        pProtocol->SendSwitchPage(MENU_PAGE_PROMPT_OFFLINE);
        ScreenState.st_page_value.uiPageTimeOut = TO_TermOfflineState_Screen;
        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        break;
    case CHARGE_STATUS_DISCHARGING: //放电
        break;
    case CHARGE_STATUS_STARTING : //启动中
    case CHARGE_STATUS_LIMIT : //充电-限制
    case CHARGE_STATUS_PAUSH : //充电-暂停
    case CHARGE_STATUS_CARPAUSH : //车辆-暂停
    case CHARGE_STATUS_DEVPAUSH : //充电设备-暂停
    case CHARGE_STATUS_SWITCH: //待机-切换中
    case CHARGE_STATUS_FREE: //待机-空闲 切换充电详情界面
        Ctrl_SwitchTermDetail_Normal(stTerm.cCanAddr);
        pProtocol->SendSwitchPage(MENU_PAGE_TERM_INFO_NORMAL);
        pProtocol->SendTermPageData(stTerm.cCanAddr, (unsigned char)cscuSysConfig.normalCardType, ThreePhaseTypeConfig);
        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        break;
    case CHARGE_STATUS_CHARGING: //充电-充电中, 切换充电详情界面
        if(ChargeConfig.localStop == 0) //不支持本地结束
        {
            pProtocol->SendSwitchPage(MENU_PAGE_TERM_INFO_NORMAL);
        }
        else
        {
            pProtocol->SendSwitchPage(MENU_PAGE_TERM_INFO_NORMAL_LOCAL_STOP);
        }
        Ctrl_SwitchTermDetail_Normal(stTerm.cCanAddr);
        pProtocol->SendTermPageData(stTerm.cCanAddr, (unsigned char)cscuSysConfig.normalCardType, ThreePhaseTypeConfig);
        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        break;
    case CHARGE_STATUS_FAULT: //故障
        break;
    default:
        break;
    }
}

//状态机控制----切换至主界面
void cSerialScreen::Ctrl_SwitchMain(unsigned char ucCanID)
{
    ScreenState.us_page_num = MENU_PAGE_MAIN;
    ScreenState.st_page_value.uc_can_addr = ucCanID;
    ScreenState.st_page_value.uc_hold_time = 0xFF;
    ScreenState.st_page_value.uc_page_num_return = MENU_PAGE_MAIN;
    ScreenState.st_page_value.uiPageTimeOut = TO_Main_Screen;//超时时间
    iPageSwitchCount = 0;//计数器清零
    bPageWaitFlag = FALSE;// 状态机切换, 取消等待处理

}

//状态机控制----切换至终端详情界面----普通版
void cSerialScreen::Ctrl_SwitchTermDetail_Normal(unsigned char ucCanID)
{
    ScreenState.us_page_num = MENU_PAGE_TERM_INFO_NORMAL;
    ScreenState.st_page_value.uc_can_addr = ucCanID;
    ScreenState.st_page_value.uc_hold_time = 30;
    ScreenState.st_page_value.uc_page_num_return = MENU_PAGE_MAIN;
    ScreenState.st_page_value.uiPageTimeOut = TO_TermDetail_Normal_Screen;//超时时间30s
    iPageSwitchCount = 0;//计数器清零
    bPageWaitFlag = FALSE;// 状态机切换, 取消等待处理
}

//状态机控制----切换至单枪刷卡和vin/车牌号充电界面----普通版
void cSerialScreen::Ctrl_SwitchVINCardCharge_Normal(unsigned char ucCanID)
{
    ScreenState.us_page_num = MENU_PAGE_VIN_CARD_CHARGE_NORMAL;
    ScreenState.st_page_value.uc_can_addr = ucCanID;
    ScreenState.st_page_value.uc_hold_time = 30;
    ScreenState.st_page_value.uc_page_num_return = MENU_PAGE_MAIN;
    ScreenState.st_page_value.uiPageTimeOut = TO_TermDetail_Normal_Screen;//超时时间30s
    iPageSwitchCount = 0;//计数器清零
    bPageWaitFlag = FALSE;// 状态机切换, 取消等待处理
}

//状态机控制----切换至单枪vin/车牌号充电界面----普通版
void cSerialScreen::Ctrl_SwitchVINManualCharge_Normal(unsigned char ucCanID)
{
    ScreenState.us_page_num = MENU_PAGE_VIN_MANUAL_CHARGE;
    ScreenState.st_page_value.uc_can_addr = ucCanID;
    ScreenState.st_page_value.uc_hold_time = 30;
    ScreenState.st_page_value.uc_page_num_return = MENU_PAGE_MAIN;
    ScreenState.st_page_value.uiPageTimeOut = TO_TermDetail_Normal_Screen;//超时时间30s
    iPageSwitchCount = 0;//计数器清零
    bPageWaitFlag = FALSE;// 状态机切换, 取消等待处理
}

//状态机控制----切换至单枪刷卡充电界面----普通版
void cSerialScreen::Ctrl_SwitchCardCharge_Normal(unsigned char ucCanID)
{
    ScreenState.us_page_num = MENU_PAGE_CARD_CHARGE_NORMAL;
    ScreenState.st_page_value.uc_can_addr = ucCanID;
    ScreenState.st_page_value.uc_hold_time = 30;
    ScreenState.st_page_value.uc_page_num_return = MENU_PAGE_MAIN;
    ScreenState.st_page_value.uiPageTimeOut = TO_TermDetail_Normal_Screen;//超时时间30s
    iPageSwitchCount = 0;//计数器清零
    bPageWaitFlag = FALSE;// 状态机切换, 取消等待处理
}

//状态机控制----切换至终端详情界面----刷卡版
void cSerialScreen::Ctrl_SwitchTermDetail_Card(unsigned char ucCanID)
{
    ScreenState.us_page_num = MENU_PAGE_TERM_INFO_CARD;
    ScreenState.st_page_value.uc_can_addr = ucCanID;
    ScreenState.st_page_value.uc_hold_time = 30;
    ScreenState.st_page_value.uc_page_num_return = MENU_PAGE_MAIN;
    ScreenState.st_page_value.uiPageTimeOut = TO_TermDetail_Card_Screen;//超时时间30s
    iPageSwitchCount = 0;//计数器清零
    bPageWaitFlag = FALSE;// 状态机切换, 取消等待处理
}

//状态机控制----切换至终端BMS界面
void cSerialScreen::Ctrl_SwitchTermBMS(unsigned char ucCanID)
{
    ScreenState.us_page_num = MENU_PAGE_BMS;
    ScreenState.st_page_value.uc_can_addr = ucCanID;
    ScreenState.st_page_value.uc_hold_time = 30;
    ScreenState.st_page_value.uc_page_num_return = MENU_PAGE_TERM_INFO_NORMAL;
    ScreenState.st_page_value.uiPageTimeOut = TO_TermBMS_Screen;//超时时间30s
    iPageSwitchCount = 0;//计数器清零
    bPageWaitFlag = FALSE;// 状态机切换, 取消等待处理
}

//状态机控制----切换至输入VIN充电界面
void cSerialScreen::Ctrl_SwitchVINStart(unsigned char ucCanID)
{
    ScreenState.us_page_num = MENU_PAGE_VIN_START_CHARGE;
    ScreenState.st_page_value.uc_can_addr = ucCanID;
    ScreenState.st_page_value.uc_hold_time = 30;
    ScreenState.st_page_value.uc_page_num_return = MENU_PAGE_MAIN;
    ScreenState.st_page_value.uiPageTimeOut = TO_VINStart_Screen;//超时时间60s
    iPageSwitchCount = 0;//计数器清零
    bPageWaitFlag = FALSE;// 状态机切换, 取消等待处理
}

//状态机控制----切换至单双枪充电选择单枪充电方式页面
void cSerialScreen::Ctrl_SwitchCoupleChargeManner(unsigned char ucCanID)
{
    ScreenState.us_page_num = MENU_PAGE_COUPLECHARGE_MANNER;
    ScreenState.st_page_value.uc_can_addr = ucCanID;
    ScreenState.st_page_value.uc_hold_time = 30;
    ScreenState.st_page_value.uc_page_num_return = MENU_PAGE_SET_SPEC_FUNC;
    ScreenState.st_page_value.uiPageTimeOut = TO_CoupleChargeMannerSet;//超时时间30s
    iPageSwitchCount = 0;//计数器清零
    bPageWaitFlag = FALSE;// 状态机切换, 取消等待处理
}

//状态机控制----切换至设置主页面
void cSerialScreen::Ctrl_SwitchParamMain(unsigned char ucCanID)
{
    ScreenState.us_page_num = MENU_PAGE_SET_MAIN;
    ScreenState.st_page_value.uc_can_addr = ucCanID;
    ScreenState.st_page_value.uc_hold_time = 30;
    ScreenState.st_page_value.uc_page_num_return = MENU_PAGE_MAIN;
    ScreenState.st_page_value.uiPageTimeOut = TO_ParamMain_Screen;//超时时间30s
    iPageSwitchCount = 0;//计数器清零
    bPageWaitFlag = FALSE;// 状态机切换, 取消等待处理
}

//状态机控制----切换至修改密码成功主页面
void cSerialScreen::Ctrl_SwitchChangePwdSuccess(unsigned char ucCanID)
{
    ScreenState.us_page_num = MENU_PAGE_RESET_PASSWD_SUCCESS;
    ScreenState.st_page_value.uc_can_addr = ucCanID;
    ScreenState.st_page_value.uc_hold_time = 5;
    ScreenState.st_page_value.uc_page_num_return = MENU_PAGE_MAIN;
    ScreenState.st_page_value.uiPageTimeOut = TO_ChangePwdResult;//超时时间5s
    iPageSwitchCount = 0;//计数器清零
    bPageWaitFlag = FALSE;// 状态机切换, 取消等待处理
}

//状态机控制----切换至修改密码失败主页面
void cSerialScreen::Ctrl_SwitchChangePwdFail(unsigned char ucCanID)
{
    ScreenState.us_page_num = MENU_PAGE_RESET_SPEC_FAILED;
    ScreenState.st_page_value.uc_can_addr = ucCanID;
    ScreenState.st_page_value.uc_hold_time = 5;
    ScreenState.st_page_value.uc_page_num_return = MENU_PAGE_MAIN;
    ScreenState.st_page_value.uiPageTimeOut = TO_ChangePwdResult;//超时时间5s
    iPageSwitchCount = 0;//计数器清零
    bPageWaitFlag = FALSE;// 状态机切换, 取消等待处理
}

//状态机控制----切换至工程选择主页面
void cSerialScreen::Ctrl_SwitchProChose(unsigned char ucCanID)
{
    ScreenState.us_page_num = MENU_PAGE_SET_PRO;
    ScreenState.st_page_value.uc_can_addr = ucCanID;
    ScreenState.st_page_value.uc_hold_time = 30;
    ScreenState.st_page_value.uc_page_num_return = MENU_PAGE_MAIN;
    ScreenState.st_page_value.uiPageTimeOut = TO_ProChose_Screen;//超时时间30s
    iPageSwitchCount = 0;//计数器清零
    bPageWaitFlag = FALSE;// 状态机切换, 取消等待处理
}

//状态机控制----切换至系统参数设置页面
void cSerialScreen::Ctrl_SwitchSysParamSet(unsigned char ucCanID)
{
    ScreenState.us_page_num = MENU_PAGE_SET_SYSTEM;
    ScreenState.st_page_value.uc_can_addr = ucCanID;
    ScreenState.st_page_value.uc_hold_time = 120;
    ScreenState.st_page_value.uc_page_num_return = MENU_PAGE_SET_MAIN;
    ScreenState.st_page_value.uiPageTimeOut = TO_SysParamSet_Screen;//超时时间30s
    iPageSwitchCount = 0;//计数器清零
    bPageWaitFlag = FALSE;// 状态机切换, 取消等待处理
}

//状态机控制----切换至交流相别设置页面
void cSerialScreen::Ctrl_SwitchACPhaseSet(unsigned char ucCanID)
{
    ScreenState.us_page_num = MENU_PAGE_SET_PHASE;
    ScreenState.st_page_value.uc_can_addr = ucCanID;
    ScreenState.st_page_value.uc_hold_time = 30;
    ScreenState.st_page_value.uc_page_num_return = MENU_PAGE_SET_MAIN;
    ScreenState.st_page_value.uiPageTimeOut = TO_ACPhaseSet_Screen;//超时时间30s
    iPageSwitchCount = 0;//计数器清零
    bPageWaitFlag = FALSE;// 状态机切换, 取消等待处理
}

//状态机控制----切换至密码输入页面
void cSerialScreen::Ctrl_SwitchPasswdInput(unsigned char ucCanID)
{
    ScreenState.us_page_num = MENU_PAGE_PASSWD_INPUT;
    ScreenState.st_page_value.uc_can_addr = ucCanID;
    ScreenState.st_page_value.uc_hold_time = 30;
    ScreenState.st_page_value.uc_page_num_return = MENU_PAGE_MAIN;
    ScreenState.st_page_value.uiPageTimeOut = TO_PasswdInput_Screen;//超时时间30s
    iPageSwitchCount = 0;//计数器清零
    bPageWaitFlag = FALSE;// 状态机切换, 取消等待处理
}

//状态机控制----切换至特殊功能设置页面
void cSerialScreen::Ctrl_SwitchSpecFuncSet(unsigned char ucCanID)
{
//    ScreenState.us_page_num = MENU_PAGE_SET_MAIN;//by zrx
    ScreenState.us_page_num = MENU_PAGE_SET_SPEC_FUNC;
    ScreenState.st_page_value.uc_can_addr = ucCanID;
    ScreenState.st_page_value.uc_hold_time = 30;
//    ScreenState.st_page_value.uc_page_num_return = MENU_PAGE_SET_MAIN;
    ScreenState.st_page_value.uc_page_num_return = MENU_PAGE_MAIN;
    ScreenState.st_page_value.uiPageTimeOut = TO_SpecFuncSet_Screen;//超时时间30s
    iPageSwitchCount = 0;//计数器清零
    bPageWaitFlag = FALSE;// 状态机切换, 取消等待处理
}

//状态机控制----切换至进线侧信息页面
void cSerialScreen::Ctrl_SwitchInLineInfo(unsigned char ucCanID)
{
    ScreenState.us_page_num = MENU_PAGE_IN_LINE_INFO;
    ScreenState.st_page_value.uc_can_addr = ucCanID;
    ScreenState.st_page_value.uc_hold_time = 30;
    ScreenState.st_page_value.uc_page_num_return = MENU_PAGE_SET_MAIN;
    ScreenState.st_page_value.uiPageTimeOut = TO_InLineInfo_Screen;//超时时间30s
    iPageSwitchCount = 0;//计数器清零
    bPageWaitFlag = FALSE;// 状态机切换, 取消等待处理
}

//状态机控制----切换至子站环境信息页面
void cSerialScreen::Ctrl_SwitchEnvInfo(unsigned char ucCanID)
{
    ScreenState.us_page_num = MENU_PAGE_ENV_INFO;
    ScreenState.st_page_value.uc_can_addr = ucCanID;
    ScreenState.st_page_value.uc_hold_time = 30;
    ScreenState.st_page_value.uc_page_num_return = MENU_PAGE_SET_MAIN;
    ScreenState.st_page_value.uiPageTimeOut = TO_EnvInfo_Screen;//超时时间90s
    iPageSwitchCount = 0;//计数器清零
    bPageWaitFlag = FALSE;// 状态机切换, 取消等待处理
}

//状态机控制----切换至直流特殊功能界面
void cSerialScreen::Ctrl_SwitchDCSpec(unsigned char ucCanID)
{
    ScreenState.us_page_num = MENU_PAGE_DC_SPEC;
    ScreenState.st_page_value.uc_can_addr = ucCanID;
    ScreenState.st_page_value.uc_hold_time = 30;
    ScreenState.st_page_value.uc_page_num_return = MENU_PAGE_SET_MAIN;
    ScreenState.st_page_value.uiPageTimeOut = TO_DCSpec_Screen;//超时时间30s
    iPageSwitchCount = 0;//计数器清零
    bPageWaitFlag = FALSE;// 状态机切换, 取消等待处理
}

//状态机控制----切换至负荷调度选择界面
void cSerialScreen::Ctrl_SwitchLoadDispatchChoose(unsigned char ucCanID)
{
    ScreenState.us_page_num = MENU_PAGE_LOAD_DISPATCH_CHOOSE;
    ScreenState.st_page_value.uc_can_addr = ucCanID;
    ScreenState.st_page_value.uc_hold_time = 30;
    ScreenState.st_page_value.uc_page_num_return = MENU_PAGE_SET_MAIN;
    ScreenState.st_page_value.uiPageTimeOut = TO_LoadDispatchChoose_Screen;//超时时间30s
    iPageSwitchCount = 0;//计数器清零
    bPageWaitFlag = FALSE;// 状态机切换, 取消等待处理
}

//状态机控制----切换至故障信息列表页面
void cSerialScreen::Ctrl_SwitchFaultInfo(unsigned char ucCanID)
{
    ScreenState.us_page_num = MENU_PAGE_FAULT_INFO;
    ScreenState.st_page_value.uc_can_addr = ucCanID;
    ScreenState.st_page_value.uc_hold_time = 30;
    ScreenState.st_page_value.uc_page_num_return = MENU_PAGE_MAIN;
    ScreenState.st_page_value.uiPageTimeOut = TO_FaultInfo_Screen;//超时时间30s
    iPageSwitchCount = 0;//计数器清零
    bPageWaitFlag = FALSE;// 状态机切换, 取消等待处理
}

//状态机控制----切换至U盘操作页面
void cSerialScreen::Ctrl_SwitchUDisk(unsigned char ucCanID)
{
    ScreenState.us_page_num = MENU_PAGE_UDISK;
    ScreenState.st_page_value.uc_can_addr = ucCanID;
    ScreenState.st_page_value.uc_hold_time = 30;
    ScreenState.st_page_value.uc_page_num_return = MENU_PAGE_MAIN;
    ScreenState.st_page_value.uiPageTimeOut = TO_SwitchUDisk_Screen;//超时时间30s
    iPageSwitchCount = 0;//计数器清零
    bPageWaitFlag = FALSE;// 状态机切换, 取消等待处理
}

//状态机控制----切换至刷卡主界面
void cSerialScreen::Ctrl_SwitchCardMain(unsigned char ucCanID)
{
    ScreenState.us_page_num = MENU_PAGE_SWIPE_CARD_MAIN;
    ScreenState.st_page_value.uc_can_addr = ucCanID;
    ScreenState.st_page_value.uc_hold_time = 30;
    ScreenState.st_page_value.uc_page_num_return = MENU_PAGE_MAIN;
    ScreenState.st_page_value.uiPageTimeOut = TO_CardMain_Screen;//超时时间30s
    iPageSwitchCount = 0;//计数器清零
    bPageWaitFlag = FALSE;// 状态机切换, 取消等待处理
}

//状态机控制----切换至充电报告界面----刷卡版
void cSerialScreen::Ctrl_SwitchChargeReport_Card(unsigned char ucCanID)
{
    ScreenState.us_page_num = MENU_PAGE_CHARGE_CARD_REPORT;
    ScreenState.st_page_value.uc_can_addr = ucCanID;
    ScreenState.st_page_value.uc_hold_time = 30;
    ScreenState.st_page_value.uc_page_num_return = MENU_PAGE_MAIN;
    ScreenState.st_page_value.uiPageTimeOut = TO_ChargeReport_Card_Screen;//超时时间30s
    iPageSwitchCount = 0;//计数器清零
    bPageWaitFlag = FALSE;// 状态机切换, 取消等待处理
}
//状态机控制----切换至充电报告界面----普通版
void cSerialScreen::Ctrl_SwitchChargeReport_Normal(unsigned char ucCanID)
{
    ScreenState.us_page_num = MENU_PAGE_CHARGE_NORMAL_REPORT;
    ScreenState.st_page_value.uc_can_addr = ucCanID;
    ScreenState.st_page_value.uc_hold_time = 30;
    ScreenState.st_page_value.uc_page_num_return = MENU_PAGE_MAIN;
    ScreenState.st_page_value.uiPageTimeOut = TO_ChargeReport_Normal_Screen;//超时时间30s
    iPageSwitchCount = 0;//计数器清零
    bPageWaitFlag = FALSE;// 状态机切换, 取消等待处理
}

//状态机控制----切换至小票机是否打印主界面
void cSerialScreen::Ctrl_SwitchTicketPrintMain(unsigned char ucCanID)
{
    ScreenState.us_page_num = MENU_PAGE_TICKET_PRINT_MAIN;
    ScreenState.st_page_value.uc_can_addr = ucCanID;
    ScreenState.st_page_value.uc_hold_time = 30;
    ScreenState.st_page_value.uc_page_num_return = MENU_PAGE_MAIN;
    ScreenState.st_page_value.uiPageTimeOut = TO_ChargeReport_Normal_Screen;//超时时间30s
    iPageSwitchCount = 0;//计数器清零
    bPageWaitFlag = FALSE;// 状态机切换, 取消等待处理
}

//状态机控制----切换至小票机缺纸提示界面
void cSerialScreen::Ctrl_SwitchNoPaperMain(unsigned char ucCanID)
{
    ScreenState.us_page_num = MENU_PAGE_TICKET_NOPAPER;
    ScreenState.st_page_value.uc_can_addr = ucCanID;
    ScreenState.st_page_value.uc_hold_time = 5;
    ScreenState.st_page_value.uc_page_num_return = MENU_PAGE_MAIN;
    ScreenState.st_page_value.uiPageTimeOut = 5;//超时时间30s
    iPageSwitchCount = 0;//计数器清零
    bPageWaitFlag = FALSE;// 状态机切换, 取消等待处理
}


//处理 点击终端状态接收指令----主页面
void cSerialScreen::GetTermState(unsigned char ucCanID)
{
    TerminalStatus stTerm;
    if(! pDevCache->QueryTerminalStatus(ucCanID,stTerm))
    {
        pLog->getLogPoint(_strLogName)->info("Screen pDevCache Query FALSE");
    }
//    if(cscuSysConfig.normalCardType == 1)//普通版
//    {
//        CheckTermState_Normal(stTerm);
//    }
//    else if(cscuSysConfig.normalCardType == 2)//刷卡版
//    {
//        CheckTermState_Card(stTerm);
//    }
    CheckTermState_Card(stTerm);//modify by ycz 2017-04-06 整合刷卡和普通
}

//处理 获取系统设置----系统设置界面
ParamSetPage_Screen cSerialScreen::GetSysParamSet()
{
    ParamSetPage_Screen retSt;
    QByteArray IpArray;
    QList <QByteArray> IpList;
    //域名
    memset(retSt.chDomainName, 0x00, sizeof(retSt.chDomainName));
    strncpy(retSt.chDomainName, ServerConfig.serverIp1, sizeof(retSt.chDomainName));
    //站地址
    memset(retSt.chStationAddr, 0x00, sizeof(retSt.chStationAddr));
    strncpy(retSt.chStationAddr, ServerConfig.stationNo, sizeof(retSt.chStationAddr));

    //DNS服务器
    IpArray = QByteArray(cscuSysConfig.dns);
    IpList = IpArray.split('.');
    for(int i = 0; i < 4; i++)
    {
        if(IpList.count() != 4)
        {
            retSt.usDNSServer[i] = 0xFF;
        }
        else
        {
            retSt.usDNSServer[i] = IpList.at(i).toInt();
        }
    }
    //网关
    IpList.clear();
    IpArray = QByteArray(NetConfig.gateway);
    IpList = IpArray.split('.');
    for(int i = 0; i < 4; i++)
    {
        if(IpList.count() != 4)
        {
            retSt.usGateWay[i] = 0xFF;
        }
        else
        {
            retSt.usGateWay[i] = IpList.at(i).toInt();
        }
    }
    //本地IP
    IpList.clear();
    IpArray = QByteArray(NetConfig.localIp);
    IpList = IpArray.split('.');
    for(int i = 0; i < 4; i++)
    {
        if(IpList.count() != 4)
        {
            retSt.usLocolIp[i] = 0xFF;
        }
        else
        {
            retSt.usLocolIp[i] = IpList.at(i).toInt();
        }
    }
    //终端个数
    retSt.usACSinNum = cscuSysConfig.singlePhase;
    retSt.usACThrNum = cscuSysConfig.threePhase;
    retSt.usDCNum = cscuSysConfig.directCurrent;
    retSt.usServerPort = ServerConfig.serverPort1;
    retSt.usZigBeeID = 0;
    return retSt;
}

//处理 获取特殊功能设置----特殊功能设置界面
SpecFuncSet_Screen cSerialScreen::GetSpecFunSet()
{
    SpecFuncSet_Screen stRet;
    QueryParamInfo();
    stRet.usCoupleGun = ChargeConfig.coupleGun;       //双枪充电设置
    stRet.usVINOffline = ChargeConfig.vinOffline;          //断网后VIN启动充电
    stRet.usLocalStop = ChargeConfig.localStop;          //本地结束 ---- (“普通版”终端信息结束按钮)
    stRet.usCardType = ChargeConfig.cardType;          //刷卡类型
    stRet.usVINAuto = ChargeConfig.vinAuto;          //VIN自动申请充电
    stRet.usCardAuto = ChargeConfig.cardAuto;          //刷卡自动申请充电
    stRet.usVINType = ChargeConfig.vinType;          //VIN数据类型
    stRet.usBoardType = cscuSysConfig.boardType;    //底板型号 值1/v1.0  2/v2.0
    stRet.usEnergyFilter = ChargeConfig.energyFilter;		//异常电度数过滤
    return stRet;
}

//解析内部总线数据
void cSerialScreen::slotFromBus(InfoMap RecvCenterMap, InfoAddrType enType)
{
    if(bWorkStartFlag == FALSE)
    {
        return;
    }
    switch(enType)
    {
    case AddrType_Udisk_Insert: //U盘插入
        if(ScreenState.us_page_num != MENU_PAGE_UDISK)
        {
            Ctrl_SwitchUDisk(0);
            pProtocol->SendSwitchPage(MENU_PAGE_UDISK);
            pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
        }
        break;
    case AddrType_UpdateResult://升级或日志导出结果
        ParseCenterUdiskResult(RecvCenterMap);
        break;
    case AddrType_SmartChargeSet_Result://本地设置错峰充电参数设置结果
        ParseCenterPeakSetResult(RecvCenterMap);
        break;
    case AddrType_CenterReadCard://主题二：集中读卡卡号, 刷卡主界面, 刷卡终端详情界面
        pLog->getLogPoint(_strLogName)->info(QString("AddrType_CenterReadCard us_page_num=%1").arg(ScreenState.us_page_num));

        switch (ScreenState.us_page_num) {
        case MENU_PAGE_SWIPE_CARD_MAIN:
        case MENU_PAGE_TERM_INFO_NORMAL:
        case MENU_PAGE_TERM_INFO_NORMAL_LOCAL_STOP:
        case MENU_PAGE_BMS:
        case MENU_PAGE_BMS_STOP_BUTTON:
        case MENU_PAGE_TERM_INFO_CARD:
        case MENU_PAGE_TERM_INFO_CARD_STOP_BUTTON:
            ParseCenterCardNumber(RecvCenterMap);
            bCardResultFlag = FALSE;
            if(ucCardStep == 1)
            {
                SendCardApplyAccountInfo();
            }
            if(ucCardStep == 3)
            {
                SendCardStopCharge();
            }
            break;
        default:
            break;
        }
//        if((ScreenState.us_page_num == MENU_PAGE_SWIPE_CARD_MAIN)
//                ||(ScreenState.us_page_num == MENU_PAGE_TERM_INFO_CARD) )
//        {
//            ParseCenterCardNumber(RecvCenterMap);
//            bCardResultFlag = FALSE;
//            if(ucCardStep == 1)
//            {
//                SendCardApplyAccountInfo();
//            }
//            if(ucCardStep == 3)
//            {
//                SendCardStopCharge();
//            }
//        }
        break;
    case AddrType_ApplyAccountInfoResult_ToScreen://主题六：充电服务返回账户信息给显示屏
        if(ScreenState.us_page_num == MENU_PAGE_SWIPE_CARD_MAIN)
        {
            bCardResultFlag = TRUE;
            ParseCenterAccountInfo(RecvCenterMap);
        }
        break;
    case AddrType_InApplyStartChargeResult_ToScreen://主题十：内部申请开始充电结果至显示屏
        if(ScreenState.us_page_num == MENU_PAGE_SWIPE_CARD_MAIN)
        {
            ParseCenterCardInResult(RecvCenterMap, 1);
        }
        break;
    case AddrType_OutApplyStartChargeResult_ToScreen://主题十四：远程申请开始充电结果至显示屏
        if(ScreenState.us_page_num == MENU_PAGE_SWIPE_CARD_MAIN)
        {
            ParseCenterCardOutResult(RecvCenterMap, 1);
        }
        break;
    case AddrType_InApplyStopChargeResult_ToScreen://主题十八：内部申请结束充电结果至显示屏
        switch (ScreenState.us_page_num) {
        case MENU_PAGE_SWIPE_CARD_MAIN:
        case MENU_PAGE_TERM_INFO_NORMAL:
        case MENU_PAGE_TERM_INFO_NORMAL_LOCAL_STOP:
        case MENU_PAGE_BMS:
        case MENU_PAGE_BMS_STOP_BUTTON:
        case MENU_PAGE_TERM_INFO_CARD:
        case MENU_PAGE_TERM_INFO_CARD_STOP_BUTTON:
            ParseCenterCardInResult(RecvCenterMap, 2);
            break;
        default:
            break;
        }

//        if(ScreenState.us_page_num == MENU_PAGE_SWIPE_CARD_MAIN)
//        {
//            ParseCenterCardInResult(RecvCenterMap, 2);
//        }
        break;
    case AddrType_OutApplyStopChargeResult_ToScreen://主题二十二：远程申请结束充电结果至显示屏
        switch (ScreenState.us_page_num) {
        case MENU_PAGE_SWIPE_CARD_MAIN:
        case MENU_PAGE_TERM_INFO_NORMAL:
        case MENU_PAGE_TERM_INFO_NORMAL_LOCAL_STOP:
        case MENU_PAGE_BMS:
        case MENU_PAGE_BMS_STOP_BUTTON:
        case MENU_PAGE_TERM_INFO_CARD:
        case MENU_PAGE_TERM_INFO_CARD_STOP_BUTTON:
            ParseCenterCardOutResult(RecvCenterMap, 2);
            break;
        default:
            break;
        }

//        if((ScreenState.us_page_num == MENU_PAGE_SWIPE_CARD_MAIN)
//                ||(ScreenState.us_page_num == MENU_PAGE_TERM_INFO_CARD))
//        {
//            ParseCenterCardOutResult(RecvCenterMap, 2);
//        }
        break;
    case AddrType_VINViaScreenApplyCharge_Result://主题二：VIN后6位申请开始充电返回结果
        if(ScreenState.us_page_num == MENU_PAGE_VIN_START_CHARGE)
        {
            pProtocol->SendSwitchPage(MENU_PAGE_VIN_APPLY_START_RESULT);
            ScreenState.st_page_value.uiPageTimeOut = TO_VINResult_Screen;
            if(ParseCenterVINResult(RecvCenterMap)) //成功
            {
                pProtocol->SendVINResult(ICON_VIN_CHARGE_SUCCESS);
            }
            else    //失败
            {
                pProtocol->SendVINResult(ICON_VIN_CHARGE_FAILED);
            }
            pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
            ucVINResultStep = 2;
//            pProtocol->SendVINResult(ICON_VIN_CHECK_ERROR);
//            bPageWaitFlag = TRUE;
//            iPageWaitTime = TO_VINResult_Screen;
            iPageSwitchCount = 0;
        }
        break;
    case AddrType_GeneralDynamicArgRenewAck_DB: //通用动态参数更新数据库回复
    case AddrType_GeneralStaticArgRenewAck_DB:  //通用静态参数更新数据库回复
        if(ScreenState.us_page_num != MENU_PAGE_SET_MAIN)
        {
            break;
        }
        ucDCSpecAckStep++;
        if(ucDCSpecAckStep >= 2)
        {
            Ctrl_SwitchDCSpec(0);
            pProtocol->SendSwitchPage(MENU_PAGE_DC_SPEC);
            pProtocol->SendDCSpec(cscuSysConfig.directCurrent);
            pProtocol->SendPageCountStop(MENU_PAGE_DC_SPEC);
            pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
            ucDCSpecAckStep = 0;
        }
        break;
    case AddrType_TermIndex_QueryFinish:    //结束始获取子站名称，CAN地址和终端编号对应关系。服务器发布，显示屏订阅
        ParseCenterTermName(RecvCenterMap);
        InitTermNameMapMulti();
        break;
    case AddrType_MakePrintTicketResult:   //执行打印小票结果
    {
        ParseTicketPrintResult(RecvCenterMap);
    }
        break;
    case AddrType_ApplyPrintTicket:  //申请打印小票一次
    {
        unsigned char ucCanID;
        ucCanID = RecvCenterMap[Addr_CanID_Comm].data()[0];
        TerminalStatus & stTempTerminalStatus = pDevCache->GetUpdateTerminalStatus(ucCanID);
        stTempTerminalStatus.bTicketPrint = 1;
        pDevCache->FreeUpdateTerminalStatus();
        pDevCache->SaveTerminalStatus(ucCanID);
    }
    case AddrType_Change_ChargeGunGroup_Info:
        //初始化终端名称图
        InitTermNameMapShow();
        InitTermNameMapMulti();
        break;
    case AddrType_InVinApplyStartCharge_Result:
        if(ScreenState.us_page_num == MENU_PAGE_VIN_CARD_CHARGE_NORMAL ||
                ScreenState.us_page_num == MENU_PAGE_VIN_MANUAL_CHARGE)
        {
            ParseCenterVinInResult(RecvCenterMap, 1);
        }
        break;
    default:
        break;
    }
}

//解析串口数据
void cSerialScreen::ProcRecvSerialData(unsigned char * pData, int iLength)
{
    if(bWorkStartFlag == FALSE)
    {
        return;
    }
    QString strRecvData = "Recv ";
    for(unsigned char i = 0; i < iLength; i++)
    {
        strRecvData+= QString::number(pData[i], 16) + " ";
    }
    strRecvData += " | State : " + QString::number(ScreenState.us_page_num, 16);
    pLog->getLogPoint(_strLogName)->info(strRecvData);
//    for(int i = 0; i < iLength; i++)
//    {
//    }
    //若帧头校验不通过,则返回
    if(!CheckFrameHead(pData, iLength))
    {
        delete pData;
        return;
    }
    //页面命令解析
    switch(ScreenState.us_page_num)
    {
    case MENU_PAGE_MAIN:
        ParseFrameMainPage(pData, iLength);
        break;
    case MENU_PAGE_SWIPE_CARD_MAIN:
        ParseFrameCardMain(pData, iLength);
        break;
    case MENU_PAGE_TERM_INFO_NORMAL:
        ParseFrameTermDatil_Normal(pData, iLength);
        break;
    case MENU_PAGE_TERM_INFO_CARD:
        ParseFrameTermDatil_Card(pData, iLength);
        break;
    case MENU_PAGE_BMS:
        ParseFrameButtonBMSInfo(pData, iLength);
        break;
    case MENU_PAGE_VIN_START_CHARGE:
        ParseFrameVINStartMain(pData, iLength);
        break;
    case MENU_PAGE_SET_MAIN:
        ParseFrameSetMain(pData, iLength);
        break;
    case MENU_PAGE_SET_PRO:
        ParseFrameProSetButton(pData, iLength);
        break;
    case MENU_PAGE_SET_SYSTEM:
        ParseFrameSysSetMain(pData, iLength);
        break;
    case MENU_PAGE_SET_PHASE:
        ParseFrameACPhaseMain(pData, iLength);
        break;
    case MENU_PAGE_PASSWD_INPUT:
        ParsePasswdMain(pData, iLength);
        break;
    case MENU_PAGE_SPEC_GENERAL:
	case MENU_PAGE_SPEC_EMERGENCY:
	case MENU_PAGE_SPEC_CFCD:
	case MENU_PAGE_SPEC_POLICY:
	case MENU_PAGE_SPEC_FGPJ:
	case MENU_PAGE_SPEC_DOUBLE_GUN:
        ParseSpecFuncMain(pData);
        break;
    case MENU_PAGE_COUPLECHARGE_MANNER:
        ParseCoupleChargeMannerSet(pData, iLength);
        break;
    case MENU_PAGE_IN_LINE_INFO:
        ParseInLineInfoMain(pData, iLength);
        break;
    case MENU_PAGE_ENV_INFO:
        ParseEnvInfo(pData, iLength);
        break;
    case MENU_PAGE_DC_SPEC:
        ParseDCSpecMain(pData, iLength);
        break;
    case MENU_PAGE_PEAK_STAGGER:
    case MENU_PAGE_LOAD_DISPATCH_CHOOSE:
        ParseFrameLoadDispatchMain(pData, iLength);
        break;
    case MENU_PAGE_FAULT_INFO:
        ParseFaultInfo(pData, iLength);
        break;
    case MENU_PAGE_UDISK:
        ParseUDiskButton(pData, iLength);
        break;
    case MENU_PAGE_CHARGE_CARD_REPORT_DETAIL:
    case MENU_PAGE_CHARGE_CARD_REPORT:
        ParseFrameChargeReport_Card(pData, iLength);
        break;
    case MENU_PAGE_CHARGE_NORMAL_REPORT:
        ParseFrameChargeReport_Normal(pData, iLength);
        break;
    case MENU_PAGE_PASSWD_WRONG:
    case MENU_PAGE_RESET_PASSWD_SUCCESS:     //add by zrx
        ParsePasswdChangeResult(pData, iLength);
        break;
    case MENU_PAGE_RESET_SPEC_FAILED:     //add by zrx
        ParsePasswdChangeResult(pData, iLength);
        break;
    case MENU_PAGE_TICKET_PRINT_MAIN:     //add by zrx
    case MENU_PAGE_TICKET_NOPAPER:
        ParseTicketPrint(pData, iLength);
        break;
        
    case MENU_PAGE_CARD_CHARGE_NORMAL:
    case MENU_PAGE_VIN_MANUAL_CHARGE:
    case MENU_PAGE_VIN_CARD_CHARGE_NORMAL:
        ParseCardChargeRequest_Normal(pData,iLength);
        break;

    default:
        break;
    }
    delete pData;
}

//启动模块
void cSerialScreen::ProcStartWork()
{
    Init();
    //模块开始工作
    bWorkStartFlag = TRUE;
    Ctrl_SwitchMain(0);
    //页面刷新
    ParamSetPage_Screen stParamSetPage = GetSysParamSet();
    pProtocol->SendFrameTermNum(stParamSetPage);
    sleep(2);
    pProtocol->SendFrameScreenReboot();
    pProtocol->SendFrameSetTime();
//    pProtocol->SendFrameSetSensitivity();
    //切换至主界面
    pProtocol->SendMainPageData(NameMap, cscuSysConfig, cscuSysConfig.version,NameMapShow);
    pProtocol->SendFrameQueryTEUIVer();
}

void cSerialScreen::ProcOneSecTimeOut()
{
    iPageSwitchCount++;
    CheckWorkState();
    CheckPageWaitTime();
    CheckPageTimeOut();
    iSetTimeCount++;
    if(iSetTimeCount > TI_SetTime_Screen)
    {
        iSetTimeCount = 0;
        pProtocol->SendFrameSetTime();
    }
    pProtocol->SendFrameSerialState();
}

//SO库调用实现函数, 创建新实例返回
CModuleIO* CreateDevInstance()
{
    return new cSerialScreen();
}

//实例销毁
void DestroyDevInstance(CModuleIO* pModule)
{
    if(pModule){
        delete pModule;
    }
}

/*
 * 页面切换
 *
 * iPageTo：	输入 目标页
 * iHoldTime：	输入 目标页持续时间
 * bAsParent：	输入 目标页是否作为父页面
 * iPageReturn：输入 返回页
 * 返回值：无
 */
void cSerialScreen::Ctrl_SwitchPage(int iPageTo, int iHoldTime, bool bAsParent, int iPageReturn)
{
	if(iPageTo < 0 || iPageReturn < 0)
		return;

    iPageSwitchCount = 0;

	ScreenState.us_page_num = bAsParent ? iPageTo : ScreenState.us_page_num;
    ScreenState.st_page_value.uc_page_num_return = iPageReturn;
    ScreenState.st_page_value.uiPageTimeOut = iHoldTime;

	pProtocol->SendSwitchPage(iPageTo);
	pProtocol->SendPageCount(iHoldTime);
}

void cSerialScreen::ParseSpecFuncMain(unsigned char * pData)
{
    unsigned char ucCmdType = pData[3];
    switch(ucCmdType)
    {
    //接收特殊功能设置指令
    case Type_SpecFuncSet_Screen:
		Ctrl_SwitchPage(MENU_PAGE_SET_SPEC_RESULT, TO_SpecFuncSetResult_Screen, false, ScreenState.us_page_num);
        if(ParseSpecFunc(pData, ScreenState.us_page_num))
            pProtocol->SendShowIcon(ICON_SET_SPEC_SUCCESS);
    	else
            pProtocol->SendShowIcon(ICON_SET_SPEC_FAILED);
        break;
        //接收按钮指令
    case Type_Button_Screen:
        ParseFrameButtonSpecFunc(pData);
        break;
	case Type_SpecFGPJ_Screen:
		ParseSpecFGPJ(pData);
		break;
	case Type_SpecCFCD_Screen:
		ParseSpecCFCD(pData);
		break;
	case Type_SpecPolicy_Screen:
		ParseSpecPolicy(pData);
		break;
    default:
        break;
    }
}

void cSerialScreen::ParseFrameButtonSpecFunc(unsigned char * pData)
{
    FrameButtonData_Screen *frame;

    frame = (FrameButtonData_Screen *)pData;
	switch(htons(frame->usPageNum)){
		case 0xFFFF:
			switch(htons(frame->usButNum))
			{
				case 1://切换至设置主页面----返回按钮
					if(ScreenState.st_page_value.uc_page_num_return > 0)
						Ctrl_SwitchPage(ScreenState.st_page_value.uc_page_num_return, TO_SpecFuncSet_Screen, false);
					else
						Ctrl_SwitchPage(MENU_PAGE_SET_MAIN, TO_ParamMain_Screen);
					break;
				case 2://切换至常用功能设置
					Ctrl_SwitchPage(MENU_PAGE_SPEC_GENERAL, TO_SpecFuncSet_Screen);
					pProtocol->SendSpecGeneral();
					pProtocol->SendSpecGeneral1();
					break;
				case 3://切换至应急充电设置
					break;
					Ctrl_SwitchPage(MENU_PAGE_SPEC_EMERGENCY, TO_SpecFuncSet_Screen);
					pProtocol->SendSpecEmergency();
					break;
				case 4://切换至错峰充电设置
					Ctrl_SwitchPage(MENU_PAGE_SPEC_CFCD, TO_SpecFuncSet_Screen);
					GetSpecCFCD();
					pProtocol->SendSpecCFCD(AllTPFVConfig, 1);
					break;
				case 5://切换至计费策略设置
					Ctrl_SwitchPage(MENU_PAGE_SPEC_POLICY, TO_SpecFuncSet_Screen);
					GetSpecPolicy();
					pProtocol->SendSpecPolicy(m_stAllPolicy, 1);
					break;
				case 6://切换至峰谷平尖设置
					Ctrl_SwitchPage(MENU_PAGE_SPEC_FGPJ, TO_SpecFuncSet_Screen);
					GetSpecFGPJ();
					pProtocol->SendSpecFGPJ(m_stAllFGPJ, 1);
					break;
				case 7://切换至双枪设置
                    //break;
					Ctrl_SwitchPage(MENU_PAGE_SPEC_DOUBLE_GUN, TO_SpecFuncSet_Screen);
					pProtocol->SendSpecDoubleGun();
					break;
				default:
					break;
			}
			break;
		case 0x7777:
			switch(htons(frame->usButNum)){
				case 0x55:
					if(ntohs(frame->usData1) == 0){
						GetSpecFGPJ();
						pProtocol->SendSpecFGPJSet(m_stFGPJCache, 1);
					}else{
						pProtocol->SendSpecFGPJSet(m_stFGPJCache, ntohs(frame->usData1));
					}
					break;
				case 0x58:
					SaveSpecFGPJ();
					break;
				case 0x60:
					if(ntohs(frame->usData1) == 0){
						GetSpecCFCD();
						pProtocol->SendSpecCFCDSet(AllTPFVConfig, 1);
					}else{
						pProtocol->SendSpecCFCDSwitch(PeakSet, ntohs(frame->usData1));
					}
					break;
				case 0x62:
					if(ntohs(frame->usData1) == 0){
						GetSpecPolicy();
						pProtocol->SendSpecPolicySet(m_stAllPolicy, 1);
					}else{
						pProtocol->SendSpecPolicySwitch(m_stPolicyCache, ntohs(frame->usData1));
					}
					break;
				case 0x61:
					SendPeakSet();
					break;
				case 0x63:
					SaveSpecPolicy();
					break;
			}
			break;
		case MENU_PAGE_SPEC_CFCD:
			switch(htons(frame->usButNum)){
				case 2:
					Ctrl_SwitchPage(MENU_PAGE_SPEC_CFCD_SET, TO_SpecFuncSet_Screen, false, MENU_PAGE_SPEC_CFCD);
					break;
				case 3:
					pProtocol->SendSpecCFCD(AllTPFVConfig, 1);
					break;
				case 4:	
					pProtocol->SendSpecCFCD(AllTPFVConfig, 2);
					break;
			}
			break;
		case MENU_PAGE_SPEC_POLICY:
			switch(htons(frame->usButNum)){
				case 2:
					Ctrl_SwitchPage(MENU_PAGE_SPEC_POLICY_SET, TO_SpecFuncSet_Screen, false, MENU_PAGE_SPEC_POLICY);
					break;
				case 3:
					pProtocol->SendSpecPolicy(m_stAllPolicy, 1);
					break;
				case 4:	
					pProtocol->SendSpecPolicy(m_stAllPolicy, 2);
					break;
			}
			break;
		case MENU_PAGE_SPEC_FGPJ:
			switch(htons(frame->usButNum)){
				case 2:
					Ctrl_SwitchPage(MENU_PAGE_SPEC_FGPJ_SET, TO_SpecFuncSet_Screen, false, MENU_PAGE_SPEC_FGPJ);
					break;
				case 3:
					pProtocol->SendSpecFGPJ(m_stAllFGPJ, 1);
					break;
				case 4:
					pProtocol->SendSpecFGPJ(m_stAllFGPJ, 2);
					break;
			}
			break;
		default:
			return;
	}
}

//解析 特殊功能设置
bool cSerialScreen::ParseSpecFunc(uchar * pData, int iPage)
{
	switch(iPage){
		case MENU_PAGE_SPEC_GENERAL:
			return ParseSpecGeneral(pData);
		case MENU_PAGE_SPEC_EMERGENCY:
			return ParseSpecEmergency(pData);
		case MENU_PAGE_SPEC_DOUBLE_GUN:
            return ParseSpecDoubleGun(pData);
			return false;
		default:
			return false;
	}
}

bool cSerialScreen::ParseSpecGeneral(uchar * pData)
{
	Frame_SpecGeneral_Screen *frame;
	Frame_SpecGeneral1_Screen *frame1;
    stCSCUSysConfig cscu;
    stChargeConfig charge;
	stSmartCarConfig smart;
	stSmartChargeConfig smart1;
	
	frame = (Frame_SpecGeneral_Screen *)pData;

	pParamSet->querySetting(&charge, PARAM_CHARGE);
	charge.vinOffline = ntohs(frame->sVINOffline);
	charge.localStop = ntohs(frame->sLocalStop);
	charge.vinAuto = ntohs(frame->sVINAuto);
	charge.cardAuto = ntohs(frame->sCardAuto);
	charge.energyFilter = ntohs(frame->sEnergyFilter);
	charge.localPolicy = ntohs(frame->sLocalPolicy);
	charge.cardType = ntohs(frame->sCardType);
	charge.vinType = ntohs(frame->sVINType);
	if(!pParamSet->updateSetting(&charge, PARAM_CHARGE))
		return false;

	pParamSet->querySetting(&smart, PARAM_SMARTCAR);
	if(smart.sSmartCar_Enable != ntohs(frame->sCarPrioty)){
		smart.sSmartCar_Enable = ntohs(frame->sCarPrioty);
		if(!pParamSet->updateSetting(&smart, PARAM_SMARTCAR))
			return false;
	}

	pParamSet->querySetting(&cscu, PARAM_CSCU_SYS);
	if((ushort)cscu.boardType != ntohs(frame->sBoardType)){
		cscu.boardType = ntohs(frame->sBoardType);
		if(!pParamSet->updateSetting(&cscu, PARAM_CSCU_SYS))
			return false;
	}

	frame1 = (Frame_SpecGeneral1_Screen *)(pData + sizeof(Frame_SpecGeneral_Screen));
	pParamSet->querySetting(&charge, PARAM_CHARGE);
	if((ushort)charge.fgpjEnable != ntohs(frame1->sFGPJ)){
		charge.fgpjEnable = ntohs(frame1->sFGPJ);
		if(!pParamSet->updateSetting(&charge, PARAM_CHARGE))
			return false;
	}

	pParamSet->querySetting(&smart1, PARAM_SMARTCHARGE);
	if((ushort)smart1.sSmartCharge_Enable != ntohs(frame1->sCuoFeng)){
		smart1.sSmartCharge_Enable = ntohs(frame1->sCuoFeng);
		if(!pParamSet->updateSetting(&smart1, PARAM_SMARTCHARGE))
			return false;
	}   

	return true;
}

bool cSerialScreen::ParseSpecEmergency(uchar * pData)
{
	Frame_SpecEmergency_Screen *frame;
	EmergencyConfig config;

	memset(&config, 0, sizeof(EmergencyConfig));

	if(!pParamSet->querySetting(&config, PARAM_EMERGENCY))
		return false;

	frame = (Frame_SpecEmergency_Screen *)pData;
	config.vin_authenticate = ntohs(frame->sVinAuth);
	config.queue_gun = ntohs(frame->sQueueForGun);
	config.card_authenticate = ntohs(frame->sCardAuth);
	config.queue_card = ntohs(frame->sQueueForCard);
	config.car_authenticate = ntohs(frame->sCarNoAuth);
	config.queue_car = ntohs(frame->sQueueForCar);

	return pParamSet->updateSetting(&config, PARAM_EMERGENCY);
}

bool cSerialScreen::GetSpecCFCD()
{
	if(!pParamSet->querySetting(&AllTPFVConfig, PARAM_TPFV))
		return false;

	memset(PeakSet, 0 , sizeof(PeakSet));
	for(int i = 0; i < AllTPFVConfig.tpfvConfig.count(); i++){
		memcpy(&PeakSet[i], &AllTPFVConfig.tpfvConfig.at(i), sizeof(stTPFVConfig));
	}
	return true;
}

bool cSerialScreen::ParseSpecCFCD(uchar * pData)
{
	Frame_SpecCFCDSet_Screen *frame;
	ushort sType = 1;

	frame = (Frame_SpecCFCDSet_Screen *)pData;
	sType = htons(frame->sPage);

    for(int i = 0; i < 5; i++)
    {
        PeakSet[i + 5*(sType - 1)].time_seg = sType;
        PeakSet[i + 5*(sType - 1)].start_hour = ntohs(frame->stRecord[i].sStartH);
        PeakSet[i + 5*(sType - 1)].start_minute = ntohs(frame->stRecord[i].sStartM);
        PeakSet[i + 5*(sType - 1)].stop_hour = ntohs(frame->stRecord[i].sStopH);
        PeakSet[i + 5*(sType - 1)].stop_minute = ntohs(frame->stRecord[i].sStopM);
        PeakSet[i + 5*(sType - 1)].limit_soc = ntohs(frame->stRecord[i].sSOC);
        PeakSet[i + 5*(sType - 1)].limit_current = ntohs(frame->stRecord[i].sCurrent);
    }

	return true;
}

void cSerialScreen::ParseSpecPolicy(uchar * pData)
{
	Frame_SpecPolicySet_Screen *frame;
	ushort sType = 1;

	frame = (Frame_SpecPolicySet_Screen *)pData;
	sType = htons(frame->sPage);

    for(int i = 0; i < 5; i++)
    {
        m_stPolicyCache[i + 5*(sType - 1)].start_hour = ntohs(frame->stPolicy[i].sStartH);
        m_stPolicyCache[i + 5*(sType - 1)].start_minute = ntohs(frame->stPolicy[i].sStartM);
        m_stPolicyCache[i + 5*(sType - 1)].stop_hour = ntohs(frame->stPolicy[i].sStopH);
        m_stPolicyCache[i + 5*(sType - 1)].stop_minute = ntohs(frame->stPolicy[i].sStopM);
        m_stPolicyCache[i + 5*(sType - 1)].electric_fee= ntohs(frame->stPolicy[i].sElectricFee);
        m_stPolicyCache[i + 5*(sType - 1)].service_fee = ntohs(frame->stPolicy[i].sServiceFee);
    }
}

bool cSerialScreen::GetSpecPolicy()
{
	if(!pParamSet->querySetting(&m_stAllPolicy, PARAM_LOCALPOLICY))
		return false;

	memset(m_stPolicyCache, 0 , sizeof(m_stPolicyCache));
	for(int i = 0; i < m_stAllPolicy.policyConfig.count(); i++){
		memcpy(&m_stPolicyCache[i], &m_stAllPolicy.policyConfig.at(i), sizeof(LocalPolicyConfig));
	}
	return true;
}

bool cSerialScreen::SaveSpecPolicy()
{
	int iIndex = 0;
	m_stAllPolicy.policyConfig.clear();
	for(int i = 0; i < 20; i++){
		if((m_stPolicyCache[i].start_hour == 0) && 
			(m_stPolicyCache[i].start_minute == 0) &&
           	(m_stPolicyCache[i].stop_hour == 0) && 
			(m_stPolicyCache[i].stop_minute == 0))
            continue;

		iIndex++;
		m_stPolicyCache[i].policy_index = iIndex;
		m_stAllPolicy.policyConfig.append(m_stPolicyCache[i]);	
	}
	Ctrl_SwitchPage(MENU_PAGE_SET_SPEC_RESULT, TO_SpecFuncSetResult_Screen, false, ScreenState.us_page_num);
	if(pParamSet->updateSetting(&m_stAllPolicy, PARAM_LOCALPOLICY)){
    	pProtocol->SendShowIcon(ICON_SET_SPEC_SUCCESS);
		pProtocol->SendSpecPolicy(m_stAllPolicy, 1);
		return true;
	}else{
       	pProtocol->SendShowIcon(ICON_SET_SPEC_FAILED);
		return false;
	}
}

bool cSerialScreen::ParseSpecFGPJ(uchar * pData)
{
	Frame_SpecFGPJ8_Screen *frame;

	ushort sType = 1;

	frame = (Frame_SpecFGPJ8_Screen *)pData;
	sType = htons(frame->sPage);

    for(int i = 0; i < 8; i++)
    {
        m_stFGPJCache[i + 8*(sType - 1)].time_seg = sType;
        m_stFGPJCache[i + 8*(sType - 1)].start_hour = ntohs(frame->stSegment[i].sStartH);
        m_stFGPJCache[i + 8*(sType - 1)].start_minute = ntohs(frame->stSegment[i].sStartM);
        m_stFGPJCache[i + 8*(sType - 1)].stop_hour = ntohs(frame->stSegment[i].sStopH);
        m_stFGPJCache[i + 8*(sType - 1)].stop_minute = ntohs(frame->stSegment[i].sStopM);
    }

	return false;
}

bool cSerialScreen::GetSpecFGPJ()
{
	if(!pParamSet->querySetting(&m_stAllFGPJ, PARAM_FGPJ))
		return false;

	memset(m_stFGPJCache, 0 , sizeof(m_stFGPJCache));

	int j = 0, iSeg = 1, iSegment, iIndex;
	for(int i = 0; i < m_stAllFGPJ.fgpjConfig.count(); i++){
		iSegment = m_stAllFGPJ.fgpjConfig.at(i).time_seg;
		if(iSegment > 4 || iSegment <= 0){
			memset(m_stFGPJCache, 0 , sizeof(m_stFGPJCache));
			return false;
		}
		if(iSeg != iSegment){
			iSeg = iSegment;	
			j = 0;
		}

		iIndex = (iSegment - 1) * 8;
		memcpy(&m_stFGPJCache[iIndex + j], &m_stAllFGPJ.fgpjConfig.at(i), sizeof(FGPJConfig));
		j++;
	}

	return true;
}

bool cSerialScreen::SaveSpecFGPJ()
{
	m_stAllFGPJ.fgpjConfig.clear();

	for(int i = 0; i < 32; i++){
		if((m_stFGPJCache[i].start_hour == 0) && 
			(m_stFGPJCache[i].start_minute == 0) &&
           	(m_stFGPJCache[i].stop_hour == 0) && 
			(m_stFGPJCache[i].stop_minute == 0))
            continue;

		m_stAllFGPJ.fgpjConfig.append(m_stFGPJCache[i]);	
	}

	Ctrl_SwitchPage(MENU_PAGE_SET_SPEC_RESULT, TO_SpecFuncSetResult_Screen, false, ScreenState.us_page_num);
	if(pParamSet->updateSetting(&m_stAllFGPJ, PARAM_FGPJ)){
    	pProtocol->SendShowIcon(ICON_SET_SPEC_SUCCESS);
		pProtocol->SendSpecFGPJ(m_stAllFGPJ, 1);
		return true;
	}else{
       	pProtocol->SendShowIcon(ICON_SET_SPEC_FAILED);
	}
	return false;
}


bool cSerialScreen::ParseSpecDoubleGun(uchar * pData)
{
    stChargeConfig charge;
    Frame_CoupleGun_Screen *couplegun;

    couplegun = (Frame_CoupleGun_Screen *)pData;

    pParamSet->querySetting(&charge, PARAM_CHARGE);
    if(ntohs(couplegun->coupleGun))
    {
        charge.coupleGun = ntohs(couplegun->chargemoudle);
    }
    else
        charge.coupleGun = ntohs(couplegun->coupleGun);
    if(!pParamSet->updateSetting(&charge, PARAM_CHARGE))
            return false;
    return true;
}


void cSerialScreen::ShowVINApplyChargeResult(unsigned char ucResult)
{
    //unsigned char ucResult = 252;
    QByteArray retArray;
    //将结果显示到屏幕  250
    retArray = GetCardResult(ucResult, 1);  //约定该状态及显示
    pProtocol->SendSwitchPage(MENU_PAGE_CARD_RESULT);
    pProtocol->SendFrameCardResult(retArray);
    pProtocol->SendPageCount(TO_CardStartStopResult_Screen);
    //超时时间置为3s
    ScreenState.st_page_value.uiPageTimeOut = TO_CardStartStopResult_Screen;
    iPageSwitchCount = 0;
    bVinResultFlag = TRUE;
}

//解析 总线接收刷卡内部申请, 开始充电, 结束充电结果 ---- 内部结果(充电模块返回), ucType: 1, 开始充电; 2, 结束充电
bool cSerialScreen::ParseCenterVinInResult(InfoMap CenterMap, unsigned char ucType)
{
    if(CenterMap.isEmpty())
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! CenterMap Empty");
        return FALSE;
    }
    else//显示结果
    {
        unsigned char ucCanID = 0;
        unsigned char ucResult = 0;
        QByteArray tempCardNum;
        //QByteArray retArray;

        ucCanID = (unsigned char)CenterMap[Addr_CanID_Comm].at(0);//CAN地址

        if(ucType == 1)//开始充电
        {
            ucResult = (unsigned char)CenterMap[Addr_InVINApplyStartChargeType_Result].at(0);//内部申请结果
        }
        else if(ucType == 2)//结束充电
        {
            ucResult = (unsigned char)CenterMap[Addr_InVINApplyStopChargeType_Result].at(0);//内部结束结果
        }
        tempCardNum = CenterMap[Addr_BatteryVIN_BMS];//卡号
        //CAN地址校验
        //if(ucCanID == ScreenState.st_page_value.uc_can_addr)
       // {
            //卡号校验
            //if(tempCardNum == CardNum)
           // {
                //将结果显示到屏幕, 0x01: 失败, 0xFF, 成功
                if(ucResult == 0xFF)//成功
                {
//                    iPageSwitchCount = 0;
//                    ScreenState.st_page_value.uiPageTimeOut = TO_CardStartChargeWait_Screen;
                    Ctrl_SwitchMain(0);
                    pProtocol->SendSwitchPage(MENU_PAGE_MAIN);
                    pProtocol->SendMainPageData(NameMap, cscuSysConfig, cscuSysConfig.version,NameMapShow);
                }
                else
                {
                    ShowVINApplyChargeResult(ucResult);
                }
                bVinResultFlag = TRUE;
           // }
       // }
    }

    return TRUE;
}

//*/
 //解析 小票机打印结果1-打印 2-缺纸
bool cSerialScreen::ParseTicketPrintResult(InfoMap CenterMap)
{
    unsigned char  ucResult,ucCanID;
    if(CenterMap.isEmpty())
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! CenterMap Empty");

        return FALSE;
    }
    ucResult = CenterMap[Addr_TicketPrint_Result].data()[0];
    ucCanID = CenterMap[Addr_CanID_Comm].data()[0];
    if(ucResult == 1){
        pProtocol->SendSwitchPage(MENU_PAGE_APPLAY_ACCOUNT_INFO);
        pProtocol->SendFrameCardWait(4);
        pProtocol->SendPageCount(5);
        ScreenState.st_page_value.uiPageTimeOut = 5;//超时时间5s

        TerminalStatus & stTempTerminalStatus = pDevCache->GetUpdateTerminalStatus(ucCanID);
        stTempTerminalStatus.bTicketPrint = 2;
        pDevCache->FreeUpdateTerminalStatus();
        pDevCache->SaveTerminalStatus(ucCanID);
    }
    else if(ucResult == 2){  //缺纸提示
        Ctrl_SwitchNoPaperMain(0);
        pProtocol->SendSwitchPage(MENU_PAGE_TICKET_NOPAPER);
        pProtocol->SendPageCount(ScreenState.st_page_value.uiPageTimeOut);
    }
    else if(ucResult == 3){
        QByteArray retArray;
        pProtocol->SendSwitchPage(MENU_PAGE_CARD_RESULT);
        retArray = "请检查小票机设备连接是否正常!";
        pProtocol->SendFrameCardResult(retArray);
        pProtocol->SendPageCount(TO_CardStartStopResult_Screen);
        //超时时间置为5s
        ScreenState.st_page_value.uiPageTimeOut = TO_CardStartStopResult_Screen;
        iPageSwitchCount = 0;
    }
}

