#ifndef IO335X_H
#define IO335X_H

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <QTimer>
#include <QDebug>
#include "fl335x_drivers.h"
#include "RealDataFilter.h"
#include "GeneralData/ModuleIO.h"
#include "Bus/Bus.h"
#include "ParamSet/ParamSet.h"
#include "CommonFunc/commfunc.h"
//#define OPEN         1 //断开
//#define CLOSE        0//闭合
#define ALARM       1//报警
#define DISALARM 0//非报警

typedef enum _GPIORelayOutIndex_335X
{
    IOID_LightCtrl_335X = GPIO_DO_1,//灯光控制
    IOID_TripCtrl_335X = GPIO_DO_6//跳闸控制 ,nihai modify 2018-01-22跳闸功能使用GPIO2N &GPIO2L
}GPIORelayOutCtrlIndex_335X;

typedef enum _GPIORelayOutCtrl_335X
{
    IOCtrl_RelayClose_335X = 1,
    IOCtrl_RelayOpen_335X = 2
}GPIORelayOutCtrl_335X;


class GPIO_335X : public CModuleIO
{
    Q_OBJECT
public:
    GPIO_335X();
    ~GPIO_335X();

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

    int fd_gpio;
    unsigned short alarmInfo1,alarmInfo2;
    unsigned short alarmInfo1_old,alarmInfo2_old;

    QTimer * actionTimer;
    QTimer * relayWatchTimer_Light;
    QTimer * relayWatchTimer_Switch;

    unsigned char ucRelayCmd1;//灯光控制命令
    unsigned char ucRelayCmd2;//跳闸控制命令

    Log * pAmmeterLog;
    ParamSet * pParamSet;
    //实时数据过滤模块调用指针
    RealDataFilter * pRealDataFilter;

//    void init();
    //掉电检测
    void CheckPowerDown();
    int GPIO_Read(int id, int *p_value);
    int GPIO_Write(int id, int value);
    bool Get_PinState(unsigned char * status_gpio_input);
    void Process_GPIO(unsigned char * src_status);
    void Translate_Status_Char2BIT(unsigned char * src_buf, unsigned short * BIT);
    int  GPIO_Init();
    void Check_GPIO_Config();

    //add by XX 2016-08-29
    unsigned char SetRelayOutPut(unsigned char ucRelayIndexIn, unsigned char ucRelayCmdIn);

    void refreshRelayControl(InfoMap &RecvBusDataMap);
    int getGPIOConfig(ParamSet * &pPara);
    void reLoadPara(InfoMap &RecvBusDataMap);
    void reportAlarmUrgency(int,int);

signals:
    void sigToBus(InfoMap, InfoAddrType);

public slots:
    void slotFromBus(InfoMap, InfoAddrType);
    //开始工作启动槽函数
    void ProcStartWork();
    void ProcTimeOut();
    void relayTimeout_Light();
    void relayTimeout_Switch();

};

#endif
