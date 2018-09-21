#ifndef IO_H
#define IO_H

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <QTimer>
#include <QDebug>
#include "RealDataFilter.h"
#include "GeneralData/ModuleIO.h"
#include "em9280_drivers.h"
#include "Bus/Bus.h"
#include "ParamSet/ParamSet.h"
#include "CommonFunc/commfunc.h"

#define UNKNOWNIO 0x00 //未知
#define ALARM       1//报警
#define DISALARM    0//非报警

#define GreenLightOn    1 //绿灯亮
#define GreenLightOff   2 //绿灯灭
#define YellowLightOn   3 //黄灯亮
#define YellowLightOff  4 //黄灯灭

typedef enum __RELAY_INDEX
{
    LIGHT_BAR_RELAY_INDEX = 1,  //灯条继电器
    SWITCH_RELAY_INDEX,         //跳闸继电器
    GREEN_LIGHT_RELAY_INDEX,    //运行指示灯(绿灯)
    YELLOW_LIGHT_RELAY_INDEX    //运行指示灯(黄灯)
}RELAY_INDEX;

typedef enum __RELAY
{
    K1 = 1,
    K2,
    K3,
    K4
}RELAY;

typedef enum _GPIORelayOutIndex_9280
{
    GPIO28_SPI_SCLK = 28,//K1
    GPIO29_SPI_CS0N = 29,//K2
    GPIO30_SPI_MISO = 30,//K4
    GPIO31_SPI_MOSI = 31//K3
}GPIORelayOutCtrlIndex_9280;
typedef enum _GPIORelayOutCtrl_9280
{
    IOCtrl_RelayClose = 1,
    IOCtrl_RelayOpen = 2
}GPIORelayOutCtrl_9280;


class GPIO : public CModuleIO
{
    Q_OBJECT
public:
    GPIO();
    ~GPIO();

    //根据配置选项初始化
    int InitModule( QThread* pThread);
    //注册设备到总线
    int RegistModule();
    //启动模块
    int StartModule();
    //停止模块
    int StopModule();
    //模块工作状态
    int ModuleStatus();

private:
    unsigned char               Status_GPIO_Normal[8]; //常开，常闭配置
    unsigned char               Status_GPIO_Old[8] ;//存放上一次的状态
    unsigned char               Status_GPIO_Alarm[8];//GPIO报警状态
    char                                Firt_Read_GPIO;//是否为首次读取IO口
    bool                                configswitch;

    int fd_gpio;
    unsigned short alarmInfo1,alarmInfo2;
    unsigned short alarmInfo1_old,alarmInfo2_old;

    InfoMap qmap;//io设备与继电器对应map，跳闸-K2, 灯带-K1, 绿灯-K3, 黄灯-K4
    __u8 ucNumTerminal[3];//充电桩数量

    QTimer * actionTimer;
    QTimer * RunStateTimer;
    QTimer * relayWatchTimer_Switch;
    QTimer * pTimerLightBar;//检测灯带开关灯
    unsigned char ucRelayCmd;//继电器控制命令
    unsigned char ucRelayIndex;//继电器序号

//    Log * pAmmeterLog;
    ParamSet * pParamSet;
    //实时数据过滤模块调用指针
    RealDataFilter * pRealDataFilter;
    DevCache * gpDevCache;
    DBOperate * pDBOperate;

    void init();

    int GPIO_OutEnable(int fd, unsigned int dwEnBits);
    int GPIO_OutDisable(int fd, unsigned int dwDisBits);
    int GPIO_OpenDrainEnable(int fd, unsigned int dwODBits);
    int GPIO_OutSet(int fd, unsigned int dwSetBits);
    int GPIO_OutClear(int fd, unsigned int dwClearBits);
    int GPIO_PinState(int fd, unsigned int * pPinState);
    bool Get_PinState(unsigned int dwPinState, unsigned char * status_gpio_input);
    void Process_GPIO(unsigned char * src_status);
    void Translate_Status_Char2BIT(unsigned char * src_buf, unsigned short * BIT);
    int  GPIO_Init();
    void Check_GPIO_Config();

    //add by XX 2016-08-29
    unsigned char SetRelayOutPut(unsigned char ucRelayIndexIn, unsigned char ucRelayCmdIn);

    void refreshRelayControl(InfoMap &RecvBusDataMap);
    int getGPIOConfig(ParamSet * &pPara);
    int getMAGNETICSWITCHConfig(ParamSet * &pPara);
    void reLoadPara(InfoMap &RecvBusDataMap);
    void reportAlarmUrgency(int,int);

    void relay1_contrl_function(unsigned char flag);//继电器１控制   1-闭合　０-断开
    void relay2_contrl_function(unsigned char flag);//继电器２控制   1-闭合　０-断开
    void relay_contrl_function(unsigned char flag,unsigned char num);//4路继电器控制     flag=闭合／断开　num=k1/k2/k3/k4

    void reportOpenDoorPowerOutages(bool &configswitch);
    bool RunStateLightYellow();
    bool RunStateLightGreen();

signals:
    void sigToBus(InfoMap, InfoAddrType);

public slots:
    void slotFromBus(InfoMap, InfoAddrType);
    //开始工作启动槽函数
    void ProcStartWork();
    void ProcTimeOut();
    void ProcRunStateTimeOut();
    void relayTimeout_Light();
    void relayTimeout_Switch();
    void slotLightBarTimeout();
};

#endif
