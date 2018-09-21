#ifndef SMARTCHARGE_H
#define SMARTCHARGE_H

#include <QObject>
#include "ParamSet/ParamSet.h"
#include "DevCache/DevCache.h"
#include <QTimer>
#include <QMap>
#include "CommonFunc/commfunc.h"
#include "vechiclepriority.h"

#define INTERVAL_COUNT 48

typedef QMap<unsigned char, char>  MarkMap;

class SmartCharge : public QObject
{
    Q_OBJECT

public:
    SmartCharge(stAllTPFVConfig *,DevCache*,ParamSet * ,Log*,DBOperate*);
    ~SmartCharge();

private:
    unsigned char TimeFlag[INTERVAL_COUNT];//一天分48个时段，０：未设置；1:峰；2：谷；3：平；4：尖
    ParamSet * pParamSet;
    stAllTPFVConfig *config;
    QTimer * actionTimer;
    QDateTime dt;
    short minute,hour;
//    bool firstCheckFlag;
    QMutex configLock;

    Log *pLog_smartCharge;
	QString _strLogName;

    unsigned char old_linkStatus;

    DevCache *pDevCache;
    DBOperate* pDBOperate;
    vechiclepriority *vechiclePriority;

    MarkMap itemInterruptMap;//标记终端是否被干预(平台和设备) 1:被干预  0:未干预
    MarkMap itemStopMap;//标记集控错峰结束充电所在时段

    void InitItemStatus();
    void markItem(unsigned char CanID, bool flag);
    void SmartChargeAction(TerminalStatus &status, int index,unsigned char canID,CHARGE_STEP &stChargeStep);
    bool identifyTimeSection(stTPFVConfig config);
    int getStopTimeSection(QDateTime);


    
signals:
   void sig_stopCharge(InfoMap, InfoAddrType);
   void sig_adjChargeCur(InfoMap, InfoAddrType);

   void sig_timeout(void);     //给车辆优先级调度使用
   void sig_clearcurruntnum(unsigned int); //给车辆优先级调度使用
    
public slots:
    void slotFromBus();
    void ProcTimeOut();
    void slot_paraChange(stAllTPFVConfig *);
    void slot_smartChargeSwitch(bool flag);
    
};

#endif // SMARTCHARGE_H
