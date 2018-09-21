#include "io.h"

//需要设置为输入的GPIO口
//GPIO16：DIN1
//GPIO17：DIN2
//GPIO18：DIN3
//GPIO19：DIN4
//GPIO20：DIN5
//GPIO21：DIN6
//GPIO24：DIN7
//GPIO25：DIN8

const unsigned char     GPIO_Input[8] ={17,16,19,18,21,20,25,24};//输入GPIO配置

GPIO::GPIO()
{
    QString strSql;
    struct db_result_st result;
    stCSCUSysConfig stTempCCUsysConfig;
    memset(Status_GPIO_Old,0x0,8) ;//存放上一次的状态
    memset(Status_GPIO_Alarm,0x0,8) ;//GPIO报警状态
    Firt_Read_GPIO = 1;//是否为首次读取IO口
    fd_gpio = -1;

    alarmInfo1 = alarmInfo2 = 0;//报警信息
    alarmInfo1_old = alarmInfo2_old = 0;
    ucRelayCmd = 0;
    ucRelayIndex = 0;

    pParamSet = ParamSet::GetInstance();
    pRealDataFilter = RealDataFilter::GetInstance();
    gpDevCache = DevCache::GetInstance(); 
    pDBOperate = DBOperate::GetInstance();

    pParamSet->querySetting(&stTempCCUsysConfig, PARAM_CSCU_SYS);
    ucNumTerminal[0] = stTempCCUsysConfig.singlePhase;
    ucNumTerminal[1] = stTempCCUsysConfig.threePhase;
    ucNumTerminal[2] = stTempCCUsysConfig.directCurrent;

    //初始化qmap，默认io设备-继电器对应关系为：1灯带-K1, 2跳闸-K2, 3绿灯-K3, 4黄灯-K4
    qmap.insert(LIGHT_BAR_RELAY_INDEX, QByteArray(1, (char)K1));
    qmap.insert(SWITCH_RELAY_INDEX, QByteArray(1, (char)K2));
    qmap.insert(GREEN_LIGHT_RELAY_INDEX, QByteArray(1, (char)K3));
    qmap.insert(YELLOW_LIGHT_RELAY_INDEX, QByteArray(1, (char)K4));

    strSql = "SELECT device_type, relay_type FROM relay_set_table;";
    if(pDBOperate->DBSqlQuery(strSql.toAscii().data(), &result, DB_PARAM) != 0)
        return ;

    for(int i = 0; i < result.row; i++)
    {
        qmap.insert(atoi(result.result[i * result.column]),QString(atoi(result.result[i * result.column + 1])).toAscii());
    }
    pDBOperate->DBQueryFree(&result);
}
GPIO::~GPIO()
{
    close(fd_gpio);
}

///
/// \brief GPIO::getGPIOConfig
/// \param pPara
/// \return
///获取输入常开常闭配置到　Status_GPIO_Normal
int GPIO::getGPIOConfig(ParamSet * &pPara)
{

    if(pPara)
    {
        unParamConfig *paramConfig = new unParamConfig;
        pPara->querySetting(paramConfig,PARAM_IO);
        memcpy(Status_GPIO_Normal,paramConfig->ioConfig.inOpenColse,8);
    }
    else
    {
    }

    return 0;
}

int GPIO::getMAGNETICSWITCHConfig(ParamSet * &pPara)
{

    if(pPara)
    {
        unParamConfig *paramConfig = new unParamConfig;
        pPara->querySetting(paramConfig,PARAM_MAGNETIC);
        configswitch=paramConfig->magneticConfig.bOpenDoorPowerOutages;
    }
    else
    {
    }

    return 0;
}

//GPIO设备初始化
int  GPIO::GPIO_Init()
{
    int rc;

    getGPIOConfig(pParamSet);
    getMAGNETICSWITCHConfig(pParamSet);

    if( (fd_gpio = open("/dev/em9280_gpio", O_RDWR) ) > 0){

        //将设定好的GPIO设置为输入状态
        for(int j = 0; j<8; j++){
            unsigned int i = 1;
            i = i << GPIO_Input[j];
            rc = GPIO_OutDisable(fd_gpio, i);
            if(rc < 0)
            {
            }
        }
    }
}

void GPIO::Check_GPIO_Config()
{
    for(int i = 0; i<8; i++){
        if(Status_GPIO_Normal[i] != 0x01 && Status_GPIO_Normal[i] != 0x02){//既不是常闭，也不是常开
            Status_GPIO_Normal[i] = UNKNOWNIO;//设置成未知
        }
    }
}

//将数组形式的报警信息转换成bit形式
void GPIO::Translate_Status_Char2BIT(unsigned char * src_buf, unsigned short * BIT)
{
    for(int i=0; i<8; i++){
        unsigned short k = 1;
        k = k << i;
        if(src_buf[i] == 1){
            *BIT = *BIT | k;
        }
        else{
            *BIT = *BIT & ~k;
        }
    }
}


//处理读取到的IO口状态
//输入：src_status读取到的IO口状态
void GPIO::Process_GPIO(unsigned char * src_status)
{
    if(Firt_Read_GPIO == 1){//第一次读取IO口，读取初始状态
        Firt_Read_GPIO = 2;
        memcpy(Status_GPIO_Old, src_status, 8);
        return;
    }
    else if(Firt_Read_GPIO == 2)//第二次读取IO口
    {
        Firt_Read_GPIO = 3;
        for(int i = 0; i<8; i++){
            QString TempStr;
            TempStr.sprintf("%d",i+1);

            if(src_status[i] == Status_GPIO_Normal[i]){
                Status_GPIO_Alarm[i] = DISALARM;
                //m_logger_fault->warn("GPIO SCAN input " + TempStr + " end alarm!") ;
            }
            else{
                Status_GPIO_Alarm[i] = ALARM;
            }
        }
        memcpy(Status_GPIO_Old, src_status, 8);
        return;
    }
    else{
        for(int i = 0; i<8; i++){
            QString TempStr;
            TempStr.sprintf("%d",i+1);
            if(Status_GPIO_Old[i] != src_status[i]){//状态有变化时处理
                if(src_status[i] == Status_GPIO_Normal[i]){//结束报警
                    Status_GPIO_Alarm[i] = DISALARM;

                    //m_logger_fault->warn("GPIO SCAN input " + TempStr + " end alarm!") ;
                }
                else{//开始报警
                    Status_GPIO_Alarm[i] = ALARM;
                }
            }
            else{//无变化时不触发信号
                if(src_status[i] == Status_GPIO_Normal[i]){
                    Status_GPIO_Alarm[i] = DISALARM;
                }
                else{
                    Status_GPIO_Alarm[i] = ALARM;
                }
            }
        }
        memcpy(Status_GPIO_Old, src_status, 8);
    }
}

//输出继电器控制函数 add by XX 2016-08-30
//ucRelayIndex : 输出继电器编号 , 详见io.h中 enum GPIORelayOutIndex
//ucRelayCmd : 输出继电器指令 , 详见io.h中   enum GPIORelayOutCtrl
//ucRelayCmd : 输出继电器持续时间, 为0 则表示持续输出
unsigned char GPIO::SetRelayOutPut(unsigned char ucRelayIndexIn, unsigned char ucRelayCmdIn)
{
    int iTempRelayIndex;

    //GPIO 28 29 30 31 设置为输出端口
    GPIO_OutEnable(fd_gpio,0xf0000000);

    switch (qmap[ucRelayIndexIn].at(0))
    {
    case K1:
        iTempRelayIndex = GPIO28_SPI_SCLK;
        break;
    case K2:
        iTempRelayIndex = GPIO29_SPI_CS0N;
        break;
    case K3:
        iTempRelayIndex = GPIO31_SPI_MOSI;
        break;
    case K4:
        iTempRelayIndex = GPIO30_SPI_MISO;
        break;
    default:
        return 0;
        break;
    }

    if(ucRelayCmdIn == IOCtrl_RelayClose)
    {
        GPIO_OutClear(fd_gpio, (1 << iTempRelayIndex)); //继电器闭合
    }
    else if(ucRelayCmdIn == IOCtrl_RelayOpen)
    {
        GPIO_OutSet(fd_gpio, (1 << iTempRelayIndex)); //继电器断开
    }
    else
    {
        return 0;
    }

    return 0xFF;
}
///
/// \brief GPIO::Get_PinState
/// \param dwPinState
/// \param Status_GPIO_Input
/// \return
///将8路输入开关状态从物理位转化到八位数组Status_GPIO_Input
bool GPIO::Get_PinState(unsigned int dwPinState, unsigned char * Status_GPIO_Input)
{
    unsigned int temp;
    for(int j = 0; j<8; j++)
    {
        unsigned int i = 1;
        i = i << GPIO_Input[j];
        temp = dwPinState & i;
        Status_GPIO_Input[j] = temp >> GPIO_Input[j];
    }
    return true;

}


int GPIO::GPIO_OutEnable(int fd, unsigned int dwEnBits)
{
    int 				rc;
    struct double_pars	dpars;

    dpars.par1 = EM9280_GPIO_OUTPUT_ENABLE;		// 0
    dpars.par2 = dwEnBits;

    rc = write(fd, &dpars, sizeof(struct double_pars));
    return rc;
}

int GPIO::GPIO_OutDisable(int fd, unsigned int dwDisBits)
{
    int 				rc;
    struct double_pars	dpars;

    dpars.par1 = EM9280_GPIO_OUTPUT_DISABLE;	// 1
    dpars.par2 = dwDisBits;

    rc = write(fd, &dpars, sizeof(struct double_pars));
    return rc;
}

int GPIO::GPIO_OutSet(int fd, unsigned int dwSetBits)
{
    int 				rc;
    struct double_pars	dpars;

    dpars.par1 = EM9280_GPIO_OUTPUT_SET;	// 2
    dpars.par2 = dwSetBits;

    rc = write(fd, &dpars, sizeof(struct double_pars));
    return rc;
}

int GPIO::GPIO_OutClear(int fd, unsigned int dwClearBits)
{
    int 				rc;
    struct double_pars	dpars;

    dpars.par1 = EM9280_GPIO_OUTPUT_CLEAR;	// 3
    dpars.par2 = dwClearBits;

    rc = write(fd, &dpars, sizeof(struct double_pars));
    return rc;
}

int GPIO::GPIO_PinState(int fd, unsigned int* pPinState)
{
    int 				rc;
    struct double_pars	dpars;

    dpars.par1 = EM9280_GPIO_INPUT_STATE;	// 5
    dpars.par2 = *pPinState;

    rc = read(fd, &dpars, sizeof(struct double_pars));
    if(!rc)
    {
        *pPinState = dpars.par2;
    }
    return rc;
}

void GPIO::relayTimeout_Switch()
{
    relayWatchTimer_Switch->stop();

    if(ucRelayCmd == IOCtrl_RelayClose)
        SetRelayOutPut(ucRelayIndex, IOCtrl_RelayOpen);
    else if(ucRelayCmd == IOCtrl_RelayOpen)
        SetRelayOutPut(ucRelayIndex, IOCtrl_RelayClose);

    ucRelayIndex = 0;
    ucRelayCmd   = 0;
}

void GPIO::refreshRelayControl(InfoMap &RecvBusDataMap)
{
    //解析Map内容
	if(!RecvBusDataMap.contains(Addr_Relay_ID))
    {
        return;
    }
    if(!RecvBusDataMap.contains(Addr_Relay_CmdType))
    {
        return;
    }
    if(!RecvBusDataMap.contains(Addr_Relay_HoldType))
    {
        return;
    }

    ucRelayIndex = (unsigned char)RecvBusDataMap.value(Addr_Relay_ID).at(0);

    ucRelayCmd = RecvBusDataMap.value(Addr_Relay_CmdType).at(0) == 1 ? IOCtrl_RelayClose : IOCtrl_RelayOpen;

    unsigned char ucHoldTime = (unsigned char)RecvBusDataMap.value(Addr_Relay_HoldType).at(0);

    unsigned char ucRet = SetRelayOutPut(ucRelayIndex, ucRelayCmd);

    relayWatchTimer_Switch->start(ucHoldTime*1000);

    InfoMap AckRelayCtrlMap;
    AckRelayCtrlMap.insert(Addr_Relay_ID, QByteArray(1, (char)ucRelayIndex));
    AckRelayCtrlMap.insert(Addr_Relay_Result, QByteArray(1, (char)ucRet));
    emit sigToBus(AckRelayCtrlMap, AddrType_RelayControl_Result);
}

void GPIO::reLoadPara(InfoMap &RecvBusDataMap)
{
    InfoMap::iterator itTarget;
    for(itTarget = RecvBusDataMap.begin(); itTarget != RecvBusDataMap.end(); itTarget++)
    {
        if (itTarget.key() == Addr_Param_Change)
        {
            if(itTarget.value().at(0) == PARAM_IO)
            {
                getGPIOConfig(pParamSet);
                break;
            }
        }
    }
}

void GPIO::slotFromBus(InfoMap RecvBusDataMap, InfoAddrType enAddrType)
{
    switch(enAddrType)
    {
    case AddrType_RelayControl_Publish:
        refreshRelayControl(RecvBusDataMap);
        break;
    case AddrType_ParamChange:
        reLoadPara(RecvBusDataMap);
        break;
    default:
        break;
    }
}

void GPIO::reportAlarmUrgency(int alarmInfo1,int alarmInfo2)
{
    InfoMap alarmMap;
//    alarmMap.insert(Addr_Alarm_Type1,QByteArray(alarmInfo1,2));
    alarmMap.insert(Addr_Alarm_Type1,QByteArray(2,alarmInfo1));
    alarmMap.insert(Addr_Alarm_Type2,QByteArray(2,alarmInfo2));
    emit sigToBus(alarmMap, AddrType_Alarm_Report);//突发到总线
    pRealDataFilter->realDataUpdate(alarmMap, AddrType_Alarm_Report);//实时数据放到缓存
}

//根据配置选项初始化
int GPIO::InitModule( QThread* pThread)
{
    m_pWorkThread = pThread;
    this->moveToThread(m_pWorkThread);
    QObject::connect(m_pWorkThread, SIGNAL(started()), this, SLOT(ProcStartWork()));

    GPIO_Init();

    return 0;
}

void GPIO::ProcTimeOut()
{
    unsigned int         dwPinState;
    unsigned char      Status_GPIO_Input[8] = {0};//存放输入的GPIO状态
    dwPinState = 0xffffffff;
    GPIO_PinState(fd_gpio, &dwPinState);
    Get_PinState(dwPinState, Status_GPIO_Input);
    Process_GPIO(Status_GPIO_Input);
    Translate_Status_Char2BIT(Status_GPIO_Alarm, &alarmInfo1);

    if((alarmInfo1_old != alarmInfo1) || (alarmInfo2_old != alarmInfo2))
    {
        reportOpenDoorPowerOutages(configswitch);
        alarmInfo1_old = alarmInfo1;
        alarmInfo2_old = alarmInfo2;
        reportAlarmUrgency(alarmInfo1,alarmInfo2);
    }
}
void GPIO::reportOpenDoorPowerOutages(bool &configswitch)
{
   if(configswitch)    //add by weiwb
   {
       if((alarmInfo1_old&0x0001)!=(alarmInfo1&0x0001))
       {
           unsigned short magneticswitchstatus;
           InfoMap magneticMap;
           magneticswitchstatus=alarmInfo1&0x0001;
           magneticMap.insert(Addr_MagneticSwitch_Status,QByteArray(2,magneticswitchstatus));
           emit sigToBus(magneticMap, AddrType_MagneticSwitch_State);
           magneticMap.clear();
       }
   }
}

bool GPIO::RunStateLightGreen()
{
    int      ucRunState;
    unsigned char      ucCanAddr = 0;
    TerminalStatus stTerminalStatus;

    for(int i = 0;i <ucNumTerminal[0] + ucNumTerminal[1] + ucNumTerminal[2];i++)
    {
        if(i < ucNumTerminal[0])
            ucCanAddr = i + 1;
        else if(i >= ucNumTerminal[0] && i < (ucNumTerminal[0] + ucNumTerminal[1]))
                ucCanAddr = i + 151 - ucNumTerminal[0];
        else if(i >= (ucNumTerminal[0] + ucNumTerminal[1]) && i < (ucNumTerminal[0] + ucNumTerminal[1] + ucNumTerminal[2]))
            ucCanAddr = i + 181 - ucNumTerminal[0] - ucNumTerminal[1];

        if(gpDevCache->QueryTerminalStatus(ucCanAddr, stTerminalStatus) == 0)
        {
            return false;
        }
        if(stTerminalStatus.stFrameRemoteSingle.charge_status == 1)
        {
            ucRunState = GreenLightOn;
            break;
        }
        else if(stTerminalStatus.stFrameRemoteSingle.charge_status !=1 &&  stTerminalStatus.stFrameRemoteSingle.charge_status != 2)
            ucRunState = GreenLightOff;
    }
    if(ucRunState == GreenLightOn)
        SetRelayOutPut(GREEN_LIGHT_RELAY_INDEX, IOCtrl_RelayClose);
    else if(ucRunState == GreenLightOff)
        SetRelayOutPut(GREEN_LIGHT_RELAY_INDEX, IOCtrl_RelayOpen);

    return true;
}

bool GPIO::RunStateLightYellow()
{
    int      ucRunState;
    unsigned char      ucCanAddr = 0;
    TerminalStatus stTerminalStatus;

    for(int i = 0;i < ucNumTerminal[0] + ucNumTerminal[1] + ucNumTerminal[2];i++)
    {
        if(i < ucNumTerminal[0])
            ucCanAddr = i + 1;
        else if(i >= ucNumTerminal[0] && i < (ucNumTerminal[0] + ucNumTerminal[1]))
                ucCanAddr = i + 151 - ucNumTerminal[0];
        else if(i >= (ucNumTerminal[0] + ucNumTerminal[1]) && i < (ucNumTerminal[0] + ucNumTerminal[1] + ucNumTerminal[2]))
            ucCanAddr = i + 181 - ucNumTerminal[0] - ucNumTerminal[1];

        if(gpDevCache->QueryTerminalStatus(ucCanAddr, stTerminalStatus) == 0)
        {
            return false;
        }
        if(stTerminalStatus.stFrameRemoteSingle.charge_status == 2)
        {
            ucRunState = YellowLightOn;
            break;
        }
        else if(stTerminalStatus.stFrameRemoteSingle.charge_status != 2 && stTerminalStatus.stFrameRemoteSingle.charge_status != 1)
            ucRunState = YellowLightOff;
    }

    if(ucRunState == YellowLightOn)
        SetRelayOutPut(YELLOW_LIGHT_RELAY_INDEX, IOCtrl_RelayClose);
   else if(ucRunState == YellowLightOff)
        SetRelayOutPut(YELLOW_LIGHT_RELAY_INDEX, IOCtrl_RelayOpen);

    return true;
}

void GPIO::ProcRunStateTimeOut()
{    
    RunStateLightYellow();
    RunStateLightGreen();
}

void GPIO::ProcStartWork()
{
    actionTimer = new QTimer();
    connect(actionTimer,SIGNAL(timeout()), SLOT(ProcTimeOut()));
    actionTimer->start(100);
	
    RunStateTimer = new QTimer();
    connect(RunStateTimer,SIGNAL(timeout()), SLOT(ProcRunStateTimeOut()));
    RunStateTimer->start(1000);

    relayWatchTimer_Switch = new QTimer();
    connect(relayWatchTimer_Switch,SIGNAL(timeout()),SLOT(relayTimeout_Switch()));

    pTimerLightBar = new QTimer();
    connect(pTimerLightBar, SIGNAL(timeout()), this, SLOT(slotLightBarTimeout()));
    pTimerLightBar->start(20000);
}

//注册模块到总线
int GPIO::RegistModule()
{
	QList<int> list;

	list.append(AddrType_ParamChange);
	list.append(AddrType_RelayControl_Publish);

	CBus::GetInstance()->RegistDev(this, list);

    return 0;
}
//启动模块
int GPIO::StartModule()
{
    m_pWorkThread->start();
    return 0;
}
int GPIO::StopModule()
{
    return 0;
}
//模块工作状态
int GPIO::ModuleStatus()
{
    return 0;
}

//SO库调用实现函数, 创建新实例返回
CModuleIO* CreateDevInstance()
{
    return new GPIO();
}

//实例销毁
void DestroyDevInstance(CModuleIO* pModule)
{
    if(pModule){
        delete pModule;
    }
}

void GPIO:: relay1_contrl_function(unsigned char flag)
{
    int fd_light;
    if((fd_light = open("/dev/em9280_gpio", O_RDWR)) == -1)
    {
        return;
    }
    //GPIO 28 29 30 31 设置为输出端口
    GPIO_OutEnable(fd_light,0xf0000000);

    if(flag)
        GPIO_OutClear(fd_light, (1 << 28)); //k1 开灯
    else
        GPIO_OutSet(fd_light, (1 << 28)); //k1 关灯

    close(fd_light);
}
void GPIO:: relay2_contrl_function(unsigned char flag)
{
    int fd_light;
    if((fd_light = open("/dev/em9280_gpio", O_RDWR)) == -1)
    {
        return;
    }
    //GPIO 28 29 30 31 设置为输出端口
    GPIO_OutEnable(fd_light,0xf0000000);

    if(flag)
        GPIO_OutClear(fd_light, (1 << 29)); //k1 开灯
    else
        GPIO_OutSet(fd_light, (1 << 29)); //k1 关灯

    close(fd_light);
}

//4路继电器控制     flag=闭合／断开　num=k1/k2/k3/k4
void GPIO:: relay_contrl_function(unsigned char flag,unsigned char num)
{
    int fd_relay;
    if((fd_relay = open("/dev/em9280_gpio", O_RDWR)) == -1)
    {
        return;
    }
    //GPIO 28 29 30 31 设置为输出端口
    GPIO_OutEnable(fd_relay,0xf0000000);
    switch(num)
    {
    case 1:
        if(flag)
            GPIO_OutClear(fd_relay, (1 << 28)); //k1 闭合
        else
            GPIO_OutSet(fd_relay, (1 << 28)); //k1 断开
        break;
    case 2:
        if(flag)
            GPIO_OutClear(fd_relay, (1 << 29)); //k2 闭合
        else
            GPIO_OutSet(fd_relay, (1 << 29)); //k2 断开
        break;
    case 3:
        if(flag)
            GPIO_OutClear(fd_relay, (1 << 30)); //k3 闭合
        else
            GPIO_OutSet(fd_relay, (1 << 30)); //k3 断开
        break;
    case 4:
        if(flag)
            GPIO_OutClear(fd_relay, (1 << 31)); //k4 闭合
        else
            GPIO_OutSet(fd_relay, (1 << 31)); //k4  断开
        break;
defalut:
        break;
    }
    close(fd_relay);
}

void GPIO::slotLightBarTimeout()
{
    QSettings *pSettings = new QSettings(MAIN_CONFIG, QSettings::IniFormat);
    bool lightEnable = false;
    int lightOpenHour = 0;
    int lightOpenMinute = 0;
    int lightCloseHour = 0;
    int lightCloseMinute = 0;
    if(pSettings != NULL)
    {
        pSettings->beginGroup("LIGHTSETTING");
        lightEnable = pSettings->value("lightcontrol_enable").toBool();
        lightOpenHour = pSettings->value("lightopentime_hour").toInt();
        lightOpenMinute = pSettings->value("lightopentime_minute").toInt();
        lightCloseHour = pSettings->value("lightclosetime_hour").toInt();
        lightCloseMinute = pSettings->value("lightclosetime_minute").toInt();
        pSettings->endGroup();
        delete pSettings;
    }
    //判断灯带使能
    if(!lightEnable)
        return;
    //判断时间是否正确
    if(!QTime::isValid(lightOpenHour, lightOpenMinute, 0) || !QTime::isValid(lightCloseHour, lightCloseMinute, 0))
        return;

    QTime openTime = QTime(lightOpenHour, lightOpenMinute, 0);
    QTime closeTime = QTime(lightCloseHour, lightCloseMinute, 0);
    QTime nowTime = QTime::currentTime();

    if(openTime < closeTime)
    {
        if(nowTime >= openTime && nowTime < closeTime)
            SetRelayOutPut(LIGHT_BAR_RELAY_INDEX, IOCtrl_RelayClose);//开
        else
            SetRelayOutPut(LIGHT_BAR_RELAY_INDEX, IOCtrl_RelayOpen);//关
    }
    else if(openTime > closeTime)
    {
        if(nowTime >= closeTime && nowTime < openTime)
            SetRelayOutPut(LIGHT_BAR_RELAY_INDEX, IOCtrl_RelayOpen);//关
        else
            SetRelayOutPut(LIGHT_BAR_RELAY_INDEX, IOCtrl_RelayClose);//开
    }
}
