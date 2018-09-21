#ifndef VINSTARTCHARGING_H
#define VINSTARTCHARGING_H

#include <QWidget>
#include <QTimer>
#include <QVariant>

#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"
#include "InfoData.h"

namespace Ui {
class VINStartCharging;
}

class VINStartCharging : public QWidget,public SwapBase
{
    Q_OBJECT

public:
    explicit VINStartCharging(QWidget *parent = 0,CBus *bus = 0, ProtocolBase *protocol = 0);
    ~VINStartCharging();
    void receiveFromBus(InfoMap, InfoAddrType);
    void switchToVINApplayCharge(unsigned char canAddr,unsigned char type);

signals:
    void sigFromBus(InfoMap, InfoAddrType);
    void sigBackToMain(int oldStatus, int newStatus, QVariant var);

private:
    Ui::VINStartCharging *ui;
    unsigned char canAddr;
    unsigned char chargeType;
    CBus *bus;
    ProtocolBase *protocol;

    void sendToCSCU(char chargeType, unsigned short chargeEnergy);

private slots:
    void slotBusToOwn(InfoMap, InfoAddrType);
    void timeoutchange();
    void on_buttonStartCharge_clicked();
    void on_buttonBack_clicked();
    void on_buttonStartCardCharge_clicked();
    void on_buttonStartVINCharge_2_clicked();
};

#endif // VINSTARTCHARGING_H
