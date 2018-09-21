#include "ChargeServiceSon.h"

CModuleIO* CreateDevInstance()
{
    return new ChargeServiceSon();
}

void DestroyDevInstance(CModuleIO* pModule)
{
    if(pModule){
        delete pModule;
    }
}
ChargeServiceSon::ChargeServiceSon()
{
    multigunchargeservice = new MultigunChargeService() ;
    QObject::connect( multigunchargeservice, SIGNAL(sigToInBus(InfoMap, InfoAddrType)), this, SLOT(slotsigToBus(InfoMap, InfoAddrType)) );
    QObject::connect( multigunchargeservice, SIGNAL(sigInVinApplyStartCharge(InfoMap)), this, SLOT(slotProcRecvInVinApplyStartCharge(InfoMap)) );
}

ChargeServiceSon::~ChargeServiceSon()
{}

void ChargeServiceSon::slotProcRecvInVinApplyStartCharge(InfoMap qInfoMap)
{
    bool m_bServiceOffline = Getm_bServiceOffline();
    bool m_bNetOffline = Getm_bNetOffline();
    InfoAddrType InfoType;
    InfoType =AddrType_InVinApplyStartCharge;
    if(m_bServiceOffline || m_bNetOffline)
    {
        if(multigunchargeservice->MultigunVinEmergencyCouple(qInfoMap,1))
        {
			//多枪删除老的应急充电流程
             //VinEmergency(qInfoMap, InfoType);
             if(qInfoMap.contains(Addr_CanID_Comm))
                multigunchargeservice->MultigunSaveChargeStep(qInfoMap[Addr_CanID_Comm].at(0));
        }
        else
        {
            ////2
//            QByteArray qTempByteArray;
//            qTempByteArray.clear();
//            qTempByteArray.append(248);//
//            qInfoMap.insert(Addr_InVINApplyStartChargeType_Result, qTempByteArray);//VIN内部申请开始充电结果
//            emit sigToBus(qInfoMap, AddrType_InVinApplyStartCharge_Result);

        }
    }
    else
    {
        ProcRecvInVinApplyStartCharge(qInfoMap);
         if(qInfoMap.contains(Addr_CanID_Comm))
            multigunchargeservice->MultigunSaveChargeStep(qInfoMap[Addr_CanID_Comm].at(0));
    }
   // InfoAddrType InfoType;
    //ProcRecvInVinApplyStartCharge(qInfoMap);
    //InfoType =AddrType_InVinApplyStartCharge;  //取消消息传送方式使用直接调用处理函数方式，保证开始原因胡赋值在处理之后执行
   // emit sigToBus(qInfoMap, InfoType);    //信号排队中先执行了开始原因胡赋值 blok 阻塞执行
    //ProcRecvInVinApplyStartCharge(qInfoMap);
    //multigunchargeservice->MultigunSaveChargeStep(qInfoMap[Addr_CanID_Comm].at(0));
}

void ChargeServiceSon::slotsigToBus(InfoMap qInfoMap, InfoAddrType InfoType)
{
    emit sigToBus(qInfoMap, InfoType);
}
void ChargeServiceSon::ProcTerminalChargeMannerInfo(InfoMap &qInfoMap)
{
    bool m_bServiceOffline = Getm_bServiceOffline();
    bool m_bNetOffline = Getm_bNetOffline();
    multigunchargeservice->MultigunProcTerminalChargeMannerInfo(qInfoMap,m_bServiceOffline,m_bNetOffline);
}

void ChargeServiceSon::ProcChargeMannerResponseResult(InfoMap &qInfoMap)
{
    multigunchargeservice->MultigunProcChargeMannerResponseResult(qInfoMap);
}

void ChargeServiceSon::ProcChargeGunGroupInfo(InfoMap qInfoMap)
{
    multigunchargeservice->MultigunProcChargeGunGroupInfo(qInfoMap);
}

unsigned char  ChargeServiceSon::GetMultType(stChargeConfig &charge,TerminalStatus &stTerminalStatus,unsigned char flag)
{
    return (multigunchargeservice->MultigunGetMultType(charge,stTerminalStatus,flag));
}

void  ChargeServiceSon::ClearChargeManner(TerminalStatus &st_TempStatusNow,TerminalStatus &st_TempStatusOld)
{
    multigunchargeservice->MultigunClearChargeManner(st_TempStatusNow,st_TempStatusOld);
}

//屏幕VIN
void  ChargeServiceSon::ProcVinChargeSubImmed(InfoMap qInfoMap)
{
    bool m_bServiceOffline = Getm_bServiceOffline();
    bool m_bNetOffline = Getm_bNetOffline();
    InfoAddrType InfoType;
     if(m_bServiceOffline || m_bNetOffline)
    {
       if(multigunchargeservice->MultigunVinEmergencyCouple(qInfoMap,0))
       {
           InfoType = AddrType_InVinApplyStartCharge;
           //LogOut(QString().sprintf("[Emergency] VIN Account=%s Couple screen Apply Start Charge CAN=%d", qInfoMap[Addr_BatteryVIN_BMS].data(), qInfoMap[Addr_CanID_Comm]), 2);
		   //多枪取消老应急充电流程
           /*if(VinEmergency(qInfoMap, InfoType))
           {
                   //SendInVinApplyStartChargeResult(stChargeStep, 0xFF);
                   QByteArray qTempByteArray;
                   qTempByteArray.clear();
                   qTempByteArray.append(255);//
                   qInfoMap.insert(Addr_InVINApplyStartChargeType_Result, qTempByteArray);//VIN内部申请开始充电结果
                   emit sigToBus(qInfoMap, AddrType_InVinApplyStartCharge_Result);
                   if(qInfoMap.contains(Addr_CanID_Comm))
                   {
                        multigunchargeservice->MultigunSaveChargeStep(qInfoMap[Addr_CanID_Comm].at(0));
                   }
           }
           else
		   */
           {
               QByteArray qTempByteArray;
               qTempByteArray.clear();
               qTempByteArray.append(254);//
               qInfoMap.insert(Addr_InVINApplyStartChargeType_Result, qTempByteArray);//VIN内部申请开始充电结果
               emit sigToBus(qInfoMap, AddrType_InVinApplyStartCharge_Result);
           }
       }
       else
       {
           QByteArray qTempByteArray;
           qTempByteArray.clear();
           qTempByteArray.append(246);//
           qInfoMap.insert(Addr_InVINApplyStartChargeType_Result, qTempByteArray);//VIN内部申请开始充电结果
           emit sigToBus(qInfoMap, AddrType_InVinApplyStartCharge_Result);
       }
    }else
    {
         if(multigunchargeservice->MultigunVinEmergencyCouple(qInfoMap,0))
         {
          //LogOut(QString().sprintf(" VIN Account=%s Couple screen Apply Start Charge CAN=%d", qInfoMap[Addr_BatteryVIN_BMS].data(), qInfoMap[Addr_CanID_Comm]), 2);
           emit sigToBus(qInfoMap, AddrType_VinApplyStartCharge);
           if(qInfoMap.contains(Addr_CanID_Comm))
                multigunchargeservice->MultigunSaveChargeStep(qInfoMap[Addr_CanID_Comm].at(0));
         }
         else
         {
             QByteArray qTempByteArray;
             qTempByteArray.clear();
             qTempByteArray.append(246);//
             qInfoMap.insert(Addr_InVINApplyStartChargeType_Result, qTempByteArray);//VIN内部申请开始充电结果
             emit sigToBus(qInfoMap, AddrType_InVinApplyStartCharge_Result);
         }
     }
}

//设备VIN
void  ChargeServiceSon::VinEmergencyCouple(InfoMap qInfoMap, InfoAddrType InfoType)
{
    if(multigunchargeservice->MultigunVinEmergencyCouple(qInfoMap,1))
    {
		//取消老应急充电流程
         //VinEmergency(qInfoMap, InfoType);
         if(qInfoMap.contains(Addr_CanID_Comm))
             multigunchargeservice->MultigunSaveChargeStep(qInfoMap[Addr_CanID_Comm].at(0));
    }
    else
    {
          CHARGE_STEP stChargeStep;
          InitChargeStep(stChargeStep);//创建一个空的终端状态
        multigunchargeservice->MultigunSaveChargeVIN(qInfoMap, InfoType,stChargeStep);
        //////3
//        QByteArray qTempByteArray;
//        qTempByteArray.clear();
//        qTempByteArray.append(248);//
//        qInfoMap.insert(Addr_InVINApplyStartChargeType_Result, qTempByteArray);//VIN内部申请开始充电结果
//        emit sigToBus(qInfoMap, AddrType_InVinApplyStartCharge_Result);

    }
}


bool ChargeServiceSon::CardGetCheck(unsigned char canid)
{
    InfoMap qInfoMap;
    qInfoMap.clear();
    qInfoMap.insert(Addr_CanID_Comm,QByteArray(1,canid));
    return (multigunchargeservice->MultigunVinEmergencyCouple(qInfoMap,0));
}
