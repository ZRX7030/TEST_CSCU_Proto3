#ifndef CHARGESERVICESON_H
#define CHARGESERVICESON_H

#include "ChargeService.h"
#include "multigunchargeservice.h"
#include <QObject>
class ChargeServiceSon:  public  ChargeService
{
     Q_OBJECT
public:
    ChargeServiceSon();
    ~ChargeServiceSon();
public slots:
    void slotProcRecvInVinApplyStartCharge(InfoMap qInfoMap);
    void slotsigToBus(InfoMap, InfoAddrType);
private:
    MultigunChargeService * multigunchargeservice;

    virtual void ProcTerminalChargeMannerInfo(InfoMap &qInfoMap);
    virtual void ProcChargeMannerResponseResult(InfoMap &qInfoMap);
    virtual void ProcChargeGunGroupInfo(InfoMap qInfoMap);
    virtual unsigned char  GetMultType(stChargeConfig &charge,TerminalStatus &stTerminalStatus,unsigned char flag);
    virtual void  ClearChargeManner(TerminalStatus &st_TempStatusNow,TerminalStatus &st_TempStatusOld);
    virtual  void ProcVinChargeSubImmed(InfoMap qInfoMap) ;
    virtual  void  VinEmergencyCouple(InfoMap qInfoMap, InfoAddrType InfoType);
    virtual bool CardGetCheck(unsigned char canid);

   // static carrysigtobus();
};

#endif // CHARGESERVICESON_H
