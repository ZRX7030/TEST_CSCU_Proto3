#ifndef CHARGEMANAGE_H
#define CHARGEMANAGE_H

#include <QWidget>
#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"
#include "InfoData.h"



class ChargeManage : public QWidget
{
    Q_OBJECT
    
public:
    explicit ChargeManage(QWidget *parent = 0, CBus *bus = 0, ProtocolBase * portocol = 0);
    ~ChargeManage();

    void changeCharger(unsigned char canaddr, int status);
    
private:
   unsigned char canAddr;
   int chargeStatus;

   CBus *bus;
   ProtocolBase *protocol;

};

#endif // CHARGEMANAGE_H
