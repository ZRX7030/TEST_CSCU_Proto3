#ifndef MULTIGUNCHARGESERVICE_H
#define MULTIGUNCHARGESERVICE_H

#include <QObject>
#include "ChargeService.h"

class MultigunChargeService : public QObject
{
    Q_OBJECT
public:
    MultigunChargeService();
    ~MultigunChargeService();
    
signals:
     void sigToInBus(InfoMap qInfoMap, InfoAddrType InfoType);// 向内部BUS发送数据
     void sigInVinApplyStartCharge(InfoMap qInfoMap);
    
public slots:

 private:
      Log * mgpLog;
     DevCache * mgpDevCache;
     ParamSet * mgpParamSet;
     DBOperate * mpDBOperate;
	 QString _strLogName;

    void PackageChargetypeReault2ChargeEquipment(unsigned char CANID, unsigned char RESULT);
    bool CheckChargeMannerInfo(InfoMap &qInfoMap);
   // bool CheckChargeGunGroupInfo(unsigned char chargeManner, unsigned char* group, unsigned char canID);
    void SetTermNameDB(unsigned char flag,unsigned char *group);
    unsigned char  GetCoupleGunNamedis(unsigned char canid);
    void UpdateTerminalChargeManner(unsigned char canID,unsigned char chargeManner,unsigned char gunType);
    unsigned char  GetAutochargeSet(unsigned char *temp,unsigned char chargeManner);
    unsigned char  GetAutochargeSetAlone(unsigned char temp,unsigned char chargeManner);
    void ProcChargeMannerResponseResult_offline(InfoMap &qInfoMap);
    inline void  LogOut(QString str,int Level);
    bool CheckChargeGunGroupInfo(unsigned char chargeManner, unsigned char* group, unsigned char canID);

public:
    void MultigunProcTerminalChargeMannerInfo(InfoMap &qInfoMap, bool m_bServiceOffline ,bool m_bNetOffline);

    void MultigunProcChargeMannerResponseResult(InfoMap &qInfoMap);
    void MultigunProcChargeGunGroupInfo(InfoMap qInfoMap);

    bool  MultigunGetMultType(stChargeConfig &charge,TerminalStatus &stTerminalStatus,unsigned char flag);
    void  MultigunClearChargeManner(TerminalStatus &st_TempStatusNow,TerminalStatus &st_TempStatusOld);
    bool MultigunVinEmergencyCouple(InfoMap qInfoMap,unsigned char flag);
    void MultigunSaveChargeStep(unsigned char canaddr);
    void MultigunSaveChargeVIN(InfoMap qInfoMap, InfoAddrType InfoType,CHARGE_STEP stChargeStep);
};

#endif // MULTIGUNCHARGESERVICE_H
