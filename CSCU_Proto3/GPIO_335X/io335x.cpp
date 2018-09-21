#include "io335x.h"

#define OPEN  0x02  //断开
#define CLOSE 0x01  //闭合
#define UNKNOWN 0x0 //未知

#define Light_RelayIndex_335X    0x01//灯条继电器
#define Switch_RelayIndex_335X 0x02//跳闸继电器

//需要设置为输入的GPIO口
//GPIO16：DIN1
//GPIO17：DIN2
//GPIO18：DIN3
//GPIO19：DIN4
//GPIO20：DIN5
//GPIO21：DIN6
//GPIO24：DIN7
//GPIO25：DIN8

const unsigned char     GPIO_Input[8] ={GPIO_DI_1,GPIO_DI_2,GPIO_DI_3,GPIO_DI_4, GPIO_DI_5,GPIO_DI_6,GPIO_DI_7,GPIO_DI_8};//输入GPIO配置

GPIO_335X::GPIO_335X()
{
    memset(Status_GPIO_Old,0x0,8) ;//存放上一次的状态
    memset(Status_GPIO_Alarm,0x0,8) ;//GPIO报警状态
    Firt_Read_GPIO = 1;//是否为首次读取IO口
    fd_gpio = -1;

    alarmInfo1 = alarmInfo2 = 0;//报警信息
    alarmInfo1_old = alarmInfo2_old = 0;

    ucRelayCmd1 = 0;
    ucRelayCmd2 = 0;

    pParamSet = ParamSet::GetInstance();
    pRealDataFilter = RealDataFilter::GetInstance();
}
GPIO_335X::~GPIO_335X()
{
    ;
}
///
/// \brief GPIO::getGPIOConfig
/// \param pPara
/// \return
///获取输入常开常闭配置到　Status_GPIO_Normal
int GPIO_335X::getGPIOConfig(ParamSet * &pPara)
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

//GPIO设备初始化
int  GPIO_335X::GPIO_Init()
{
    getGPIOConfig(pParamSet);
    return 0;
}

void GPIO_335X::Check_GPIO_Config()
{
    for(int i = 0; i<8; i++){
        if(Status_GPIO_Normal[i] != 0x01 && Status_GPIO_Normal[i] != 0x02){//既不是常闭，也不是常开
            Status_GPIO_Normal[i] = UNKNOWN;//设置成未知
        }
    }
}

//将数组形式的报警信息转换成bit形式
void GPIO_335X::Translate_Status_Char2BIT(unsigned char * src_buf, unsigned short * BIT)
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
void GPIO_335X::Process_GPIO(unsigned char * src_status)
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
unsigned char GPIO_335X::SetRelayOutPut(unsigned char ucRelayIndexIn, unsigned char ucRelayCmdIn)
{
    int iRelayIndex;
    int iValue = 0;
    switch (ucRelayIndexIn)
    {
    case 01:    //灯光控制
        iRelayIndex = IOID_LightCtrl_335X;
        break;
    case 02:    //跳闸控制
        iRelayIndex = IOID_TripCtrl_335X;
        break;
    default:
        return 0;
        break;
    }

    if(ucRelayCmdIn == IOCtrl_RelayClose_335X)//继电器闭合
    {
        iValue = 1;
        GPIO_Write(iRelayIndex, iValue);
    }
    else if(ucRelayCmdIn == IOCtrl_RelayOpen_335X)//继电器断开
    {
        iValue = 0;
        GPIO_Write(iRelayIndex, iValue);
    }
    else
    {
        return 0;
    }
    return 0xFF;
}

//掉电检测
//工作时为1, 高电平; 掉电时为0, 低电平
void GPIO_335X::CheckPowerDown()
{
    int iValue = 1;
    GPIO_Read(GPIO_PWRDN_DET, &iValue);
    if(iValue == 0)//掉电状态
    {
        InfoMap ToCenterMap;
        emit sigToBus(ToCenterMap,AddrType_PowerDown_Sys);
    }
}

int GPIO_335X::GPIO_Read(int id, int *p_value)
{
    {
        int fd;
        int ret;
        int cmd;

        fd = open(GPIO_DEV, O_RDONLY);
        if(fd < 0) {
            perror("Error: open(\"/dev/gpio\") ");
            return -1;
        }

        *p_value = 0;
        cmd = _IOR(GPIO_DEV_TYPE, id, *p_value);
        ret = ioctl(fd, cmd, p_value);
        if(ret < 0) {
            perror("Error: ioctl() ");
            close(fd);
            return -1;
        }

        close(fd);
        return 0;
    }
}

int GPIO_335X::GPIO_Write(int id, int value)
{
    int fd;
    int ret;
    int cmd;

    fd = open(GPIO_DEV, O_WRONLY);
    if(fd < 0) {
        perror("Error: open(\"/dev/gpio\") ");
        return -1;
    }

    cmd = _IOW(GPIO_DEV_TYPE, id, value);
    ret = ioctl(fd, cmd, &value);
    if(ret < 0) {
        perror("Error: ioctl() ");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

///
/// \brief GPIO::Get_PinState
/// \param dwPinState
/// \param Status_GPIO_Input
/// \return
///将8路输入开关状态从物理位转化到八位数组Status_GPIO_Input
bool GPIO_335X::Get_PinState(unsigned char * Status_GPIO_Input)
{
    int iValue = 0;
    for(int j = 0; j<8; j++)
    {
        unsigned int i = 1;
        i = i << GPIO_Input[j];
        GPIO_Read(GPIO_Input[j], &iValue);
        Status_GPIO_Input[j] = iValue;
    }
    return true;
}

void GPIO_335X::relayTimeout_Light()
{
    relayWatchTimer_Light->stop();

    if(ucRelayCmd1 == CLOSE)
    {
        SetRelayOutPut(Light_RelayIndex_335X, OPEN);
    }
    else if(ucRelayCmd1 == OPEN)
    {
        SetRelayOutPut(Light_RelayIndex_335X, CLOSE);
    }

    ucRelayCmd1 = 0x0;
}

void GPIO_335X::relayTimeout_Switch()
{
    relayWatchTimer_Switch->stop();

    if(ucRelayCmd2 == CLOSE)
    {
        SetRelayOutPut(Switch_RelayIndex_335X, OPEN);
    }
    else if(ucRelayCmd2 == OPEN)
    {
        SetRelayOutPut(Switch_RelayIndex_335X, CLOSE);
    }

    ucRelayCmd1 = 0x0;
}

void GPIO_335X::refreshRelayControl(InfoMap &RecvBusDataMap)
{
    InfoMap::iterator itTarget;
    InfoMap AckRelayCtrlMap;
    unsigned char ucRelayIndex,ucRelayCmd,ucHoldTime;
    unsigned char ucRet = 0;

    //解析Map内容
    for(itTarget = RecvBusDataMap.begin(); itTarget != RecvBusDataMap.end(); itTarget++)
    {
        switch (itTarget.key())
        {
        case Addr_Relay_ID:
            ucRelayIndex = itTarget.value().at(0);
            break;
        case Addr_Relay_CmdType:
            ucRelayCmd = itTarget.value().at(0);
            break;
        case Addr_Relay_HoldType:
            ucHoldTime = itTarget.value().at(0);
            break;
        default:
            break;
        }
    }

    if(ucRelayIndex == Light_RelayIndex_335X)
    {
        ucRelayCmd1 = ucRelayCmd;
        if(ucHoldTime !=0x0)
        {
           relayWatchTimer_Light->start(ucHoldTime*1000);
        }
    }
    else if(ucRelayIndex == Switch_RelayIndex_335X)
    {
        ucRelayCmd2 = ucRelayCmd;
        if(ucHoldTime !=0x0)
        {
           relayWatchTimer_Switch->start(ucHoldTime*1000);
        }
    }

    ucRet = SetRelayOutPut(ucRelayIndex, ucRelayCmd);

    AckRelayCtrlMap.insert(Addr_Relay_ID,QByteArray((char*)&ucRelayIndex,1));
    AckRelayCtrlMap.insert(Addr_Relay_Result,QByteArray((char*)&ucRet,1));
    emit sigToBus(AckRelayCtrlMap,AddrType_RelayControl_Result);

}

void GPIO_335X::reLoadPara(InfoMap &RecvBusDataMap)
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

void GPIO_335X::slotFromBus(InfoMap RecvBusDataMap, InfoAddrType enAddrType)
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

void GPIO_335X::reportAlarmUrgency(int alarmInfo1,int alarmInfo2)
{
    InfoMap alarmMap;
//    alarmMap.insert(Addr_Alarm_Type1,QByteArray(alarmInfo1,2));
    alarmMap.insert(Addr_Alarm_Type1,QByteArray(2,alarmInfo1));
    alarmMap.insert(Addr_Alarm_Type2,QByteArray(2,alarmInfo2));
    emit sigToBus(alarmMap, AddrType_Alarm_Report);//突发到总线
    pRealDataFilter->realDataUpdate(alarmMap, AddrType_Alarm_Report);//实时数据放到缓存
}

//根据配置选项初始化
int GPIO_335X::InitModule( QThread* pThread)
{
    m_pWorkThread = pThread;
    this->moveToThread(m_pWorkThread);
    QObject::connect(m_pWorkThread, SIGNAL(started()), this, SLOT(ProcStartWork()));

    GPIO_Init();

    return 0;
}

void GPIO_335X::ProcTimeOut()
{
    unsigned char      Status_GPIO_Input[8] = {0};//存放输入的GPIO状态
    CheckPowerDown();
    Get_PinState(Status_GPIO_Input);
    Process_GPIO(Status_GPIO_Input);
    Translate_Status_Char2BIT(Status_GPIO_Alarm, &alarmInfo1);
    if((alarmInfo1_old != alarmInfo1) || (alarmInfo2_old != alarmInfo2))
    {
        alarmInfo1_old = alarmInfo1;
        alarmInfo2_old = alarmInfo2;
         reportAlarmUrgency(alarmInfo1,alarmInfo2);
    }

}

void GPIO_335X::ProcStartWork()
{
    actionTimer = new QTimer();
    actionTimer->start(100);//
    connect(actionTimer,SIGNAL(timeout()), SLOT(ProcTimeOut()));

    relayWatchTimer_Light = new QTimer();
    connect(relayWatchTimer_Light,SIGNAL(timeout()),SLOT(relayTimeout_Light()));
    relayWatchTimer_Switch = new QTimer();
    connect(relayWatchTimer_Switch,SIGNAL(timeout()),SLOT(relayTimeout_Switch()));
}

//注册模块到总线
int GPIO_335X::RegistModule()
{
	QList<int> list;

	list.append(AddrType_ParamChange);
	list.append(AddrType_RelayControl_Publish);

	CBus::GetInstance()->RegistDev(this, list);

    return 0;
}
//启动模块
int GPIO_335X::StartModule()
{
    m_pWorkThread->start();
    return 0;
}
int GPIO_335X::StopModule()
{
    return 0;
}
//模块工作状态
int GPIO_335X::ModuleStatus()
{
    return 0;
}

//SO库调用实现函数, 创建新实例返回
CModuleIO* CreateDevInstance()
{
    return new GPIO_335X();
}

//实例销毁
void DestroyDevInstance(CModuleIO* pModule)
{
    if(pModule){
        delete pModule;
    }
}

