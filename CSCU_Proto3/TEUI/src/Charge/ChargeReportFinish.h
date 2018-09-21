#ifndef CHARGEREPORTFINISH_H
#define CHARGEREPORTFINISH_H

#include <QWidget>

#include "Common.h"
#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"
#include "InfoData.h"

namespace Ui {
class ChargeReportFinish;
}

class ChargeReportFinish : public QWidget, public SwapBase
{
    Q_OBJECT

public:
    explicit ChargeReportFinish(QWidget *parent, CBus * bus = 0, ProtocolBase *protocol = 0);
    ~ChargeReportFinish();
    void receiveFromBus(InfoMap Map, InfoAddrType Type);
    void queryChargeFinishReport(unsigned char canAddr);
	void switchToChargeFinish(unsigned char canAddr);
    void queryChargeFinishSpecialFeatureSet();

signals:
   void sigFromBus(InfoMap, InfoAddrType);
   void sigBackToMain(int oldStatus, int newStatus, QVariant var);

private slots:
    void slotBusToOwn(InfoMap, InfoAddrType);
    void on_buttonSave_clicked();
    void on_buttonStartCharging_clicked();
    void timeoutchange();

    void on_buttonPrintTicket_clicked();

private:
    Ui::ChargeReportFinish *ui;
    CBus *bus;
    ProtocolBase *protocol;
    unsigned char canAddr;
    void clearChargeReportFinish();
    stSpecialFunc par;
};

#endif // CHARGEREPORTFINISH_H
