#ifndef LOADSCHEDULE_H
#define LOADSCHEDULE_H

#include <QDebug>
#include <QTimer>
#include "RealDataFilter.h"
#include "GeneralData/ModuleIO.h"
#include "Bus/Bus.h"
#include "ParamSet/ParamSet.h"
#include "Log/Log.h"
#include "powerLimit.h"
#include "smartcharge.h"
#include "peakshaving.h"


typedef struct
{
    int type;
    int startHour;
    int startMinute;
    int endHour;
    int endMinute;
} chargeSet;
//typedef QMap<int, chargeSet> setMap;

class LoadSchedule : public CModuleIO
{
    Q_OBJECT
public:
    LoadSchedule();
    ~LoadSchedule();

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

    QTimer * actionTimer;

//    Log * pAmmeterLog;
    Log *pLog_loadSchedule;
    ParamSet * pParamSet;
    //实时数据过滤模块调用指针
    RealDataFilter * pRealDataFilter;
    DevCache *m_pDevCache;
    PowerLimit * powerLimit;
    SmartCharge * smartCharge;
    PeakShaving *peakShaving;

    stAllTPFVConfig *newConfig;
    DBOperate * m_pDatabase;


    void init();
    int getLoadScheduleConfig(ParamSet * &pPara);
    void procParamChange(InfoMap &);
    bool checkTimeSection(chargeSet item1,QList<chargeSet> itemList);
    bool paraCheck_smartCharge(InfoMap &);
    void procSmartChargeSet(InfoMap);
    void parseSmartChargeConfig(stAllTPFVConfig *newConfig,InfoMap &RecvBusDataMap);

signals:
    void sigToBus(InfoMap, InfoAddrType);
    void sig_paraChange_powerLimit(stPowerLimitConfig);
    void sig_paraChange_smartCharge(stAllTPFVConfig *);
    void sig_paraChange_smartChargeSwitch(bool);
    void sig_readAmmeterFail();
    void sig_readAmmeterSucess();
    void sig_setCCUResult(InfoMap);
    void sig_sendTermSignal(InfoMap qInfoMap);
    void sig_StartCharge(unsigned char);    //hd
    void sig_chargeFromEnergyPlan(unsigned char ,unsigned char);

public slots:
    void slotFromBus(InfoMap, InfoAddrType);
    //开始工作启动槽函数
    void ProcStartWork();
//    void ProcTimeOut();
    void slot_setPowerLimit(InfoMap, InfoAddrType);

};

#endif //LOADSCHEDULE_H
