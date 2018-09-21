#ifndef CHAGINGFAILUREINFORMATION_H
#define CHAGINGFAILUREINFORMATION_H

#include <QWidget>
#include <QTableWidget>
#include "Common.h"
#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"
#include "ProtocolBase.h"

namespace Ui {
class ChagingFailureInformation;
}

class ChagingFailureInformation : public QWidget, public SwapBase
{
    Q_OBJECT

public:
    explicit ChagingFailureInformation(QWidget *parent, CBus * bus = 0, ProtocolBase *protocol = 0);
    ~ChagingFailureInformation();
    void receiveFromBus(InfoMap Map, InfoAddrType Type);
    void queryChargeFaultReport(unsigned char canAddr);
    void switchToChargeFalut(unsigned char canAddr);

signals:
   void sigFromBus(InfoMap, InfoAddrType);
   void sigBackToMain(int oldStatus, int newStatus, QVariant var);

private slots:

    void slotBusToOwn(InfoMap, InfoAddrType);
    void slotTimerQueryChargingFault();
    void on_buttonReturn_clicked();
    void timeoutchange();

    void on_tabWidget_currentChanged(int index);

private:
    Ui::ChagingFailureInformation *ui;
    CBus *bus;
    ProtocolBase *protocol;
    unsigned char canAddr;
    void clearChargeFailureInfor();
    void showFailureInformation();
    void showChargeInformation();

    void createTableItems();
    void tablseItemInit(QTableWidget *table);
    void settablecolumncount();
};

#endif // CHAGINGFAILUREINFORMATION_H
