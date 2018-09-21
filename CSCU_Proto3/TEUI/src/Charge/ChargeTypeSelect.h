#ifndef CHARGETYPESELECT_H
#define CHARGETYPESELECT_H

#include <QWidget>
#include <QVariant>

#include "Common.h"
#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"
#include "ProtocolBase.h"

namespace Ui {
class ChargeTypeSelect;
}

class ChargeTypeSelect : public QWidget, public SwapBase
{
    Q_OBJECT

public:
    explicit ChargeTypeSelect(QWidget *parent = 0, CBus *bus = 0, ProtocolBase *protocol = 0);
    ~ChargeTypeSelect();
	void switchToChargeTypeSelect(QVariant var);
    void receiveFromBus(InfoMap Map, InfoAddrType Type);
signals:
   //void sigFromBus(InfoMap, InfoAddrType);
   void sigBackToMain(int oldStatus, int newStatus, QVariant var);

private slots:
   //void slotBusToOwn(InfoMap, InfoAddrType);
   void on_buttonAutoCharge_clicked();
   //void on_buttonEnengyCharge_clicked();
   void on_buttonBack_clicked();
   void timeoutchange();

private:
    Ui::ChargeTypeSelect *ui;
    CBus *bus;
    ProtocolBase *protocol;
    unsigned char canAddr;
	void sendToCSCU(char chargeType, unsigned short chargeEnergy);
};

#endif // ChargeTypeSelect_H
