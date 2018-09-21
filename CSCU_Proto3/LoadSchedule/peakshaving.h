#ifndef PEAKSHAVING_H
#define PEAKSHAVING_H

#include <QObject>
#include "ParamSet/ParamSet.h"
#include "DevCache/DevCache.h"
#include <QTimer>
#include <QMap>
#include "CommonFunc/commfunc.h"

#define INTERVAL_COUNT 48

typedef QMap<unsigned char, char>  MarkMap;
class PeakShaving : public QObject
{
    Q_OBJECT
public:
    PeakShaving(stAllTPFVConfig *,DevCache*,ParamSet * ,Log*);
    ~PeakShaving();

private:
    unsigned char TimeFlag[INTERVAL_COUNT];//一天分48个时段，０：未设置；1:峰；2：谷；3：平；4：尖
    ParamSet * pParamSet;
    stAllTPFVConfig *config;
    QTimer * actionTimer;
    QMutex configLock;
     short minute,hour;

    Log *pLog_smartCharge;
	QString _strLogName;

    unsigned char old_linkStatus;

    DevCache *pDevCache;

      __u8 ucNumTerminal[3];//充电桩数量

    void SmartChargeAction(TerminalStatus &status, int index,unsigned char canID,unsigned char flag);
    bool identifyTimeSection(stTPFVConfig config);
    void LogOut(QString str,int Level);



signals:
   void sig_ChargeApply(InfoMap, InfoAddrType);    //申请开始或结束充电hd

public slots:
    void ProcTimeOut();
    void slot_paraChange(stAllTPFVConfig *);
    void slot_smartChargeSwitch(bool flag);
    void slot_StartChargingCMD(unsigned char canID);    //hd
    void slot_chargeFromEnergyPlan(unsigned char,unsigned char);
    
};

#endif // PEAKSHAVING_H
