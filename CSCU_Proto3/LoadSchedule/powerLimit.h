#ifndef POWERLIMIT_H
#define POWERLIMIT_H

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <QTimer>
#include <QDebug>
#include "RealDataFilter.h"
#include "GeneralData/ModuleIO.h"
#include "Bus/Bus.h"
#include "ParamSet/ParamSet.h"
#include "CommonFunc/commfunc.h"
#include "DevCache/DevCache.h"



class PowerLimit : public QObject
{
    Q_OBJECT
public:
    PowerLimit(DevCache*,Log *,ParamSet *);
    ~PowerLimit();

private:

    QTimer * actionTimer;

	QString _strLogName;
    Log * pPowerLimitLog;
    ParamSet * pParamSet;
    DevCache* m_pDevCache;	//数据缓存
    //实时数据过滤模块调用指针
    RealDataFilter * pRealDataFilter;
    stPowerLimitConfig config;

    float sumPower;

    unsigned int limitPowerGroup[10];//已下发CCU限制功率
    unsigned int fullPowerGroup[10];//收到CCU额定功率
    unsigned int requirePowerGroup[10];//CCU需求功率
    unsigned int sumPower_Ammeter;//电表计算限制功率
    unsigned int SumLimitPower;//集控限制功率

    int powerNoChangeMonitor;//电表平缓变化时间
    int readAmmeterFailCount;
    QList<float> powerDownMonitor;//电表功率变动记录
    QList<float> powerUpMonitor;
    bool downFlag,upFlag;//电表测量值上升下降标志位
    bool getParamSetResult;//设置CCU反馈
    int waitParamSetResultCount;//设置CCU后等待响应时间
    unsigned int getResponse[10];//0:收到CCU设置到响应   1:未收到

    QVariant vKey, vValue;
    stAmmeterData stAmmeter;
    stAllAmmeterConfig conf_allAmmeter;
    stAmmeterConfig conf_ammeter;
    float oldPower;
    CCUAllRealData ccuDataMap;

    void init();
    void getSumLimitPower();
    void setLimitPower(unsigned int TempGridPower,unsigned short cCanAddr);
    bool checkChange(unsigned int requirePower);
    void reAssignPower();
    void getPowerLimit_ammeter(int newPower);
    void dynamicSetLimitPower_up(float newPower,float tmpPower);
    void dynamicSetLimitPower_down(float newPower,float tmpPower);
    bool queryMonitorPower(float &);
    void savePower2Log();
    void saveKeyPoint2File(QString str);
    bool checkReassinPower();
    void getPowerInfoFromCCU();
    void checkSetPowerLimitResult();
    void savePower2File(float sumPower, float chargePower);


signals:
    void sig_setLimitPower(InfoMap, InfoAddrType);
    void sig_stopCharge(InfoMap, InfoAddrType);
public slots:
    //开始工作启动槽函数
    void ProcStartWork();
    void ProcTimeOut();
    void slot_paraChange(stPowerLimitConfig);
    void slot_readAmmeterFail();
    void slot_readAmmeterSuccess();
    void slot_setCCUResult(InfoMap);

};

#endif
