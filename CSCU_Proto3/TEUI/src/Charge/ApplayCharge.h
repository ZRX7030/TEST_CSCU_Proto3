#ifndef APPLAYCHARGE_H
#define APPLAYCHARGE_H

#include <QWidget>
#include <QTimer>
#include <QVariant>

#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"
#include "InfoData.h"

namespace Ui {
class ApplayCharge;
}

class ApplayCharge : public QWidget, public SwapBase
{
    Q_OBJECT
    
public:
    explicit ApplayCharge(QWidget *parent = 0,CBus *bus = 0, ProtocolBase *protocol = 0);
    ~ApplayCharge();
	void receiveFromBus(InfoMap, InfoAddrType);

    void switchToApplayCharge(unsigned char canAddr);
	void sendToCSCU(InfoBodyType type);
    void queryConfigSpecialFeatureSet();

signals:
	void sigFromBus(InfoMap, InfoAddrType);
	void sigBackToMain(int oldStatus, int newStatus, QVariant var);

private slots:
    void slotBusToOwn(InfoMap, InfoAddrType);
	void timerQueryResult();
    void on_buttonBack_clicked();
    void timeoutchange();

    void on_buttonStartCharge_clicked();

private:
    Ui::ApplayCharge *ui;
    unsigned char canAddr;
	QTimer timerQuery;
	CBus *bus;
    ProtocolBase *protocol;
    int password;

    stSpecialFunc localparam;

};

#endif // APPLAYCHARGE_H
