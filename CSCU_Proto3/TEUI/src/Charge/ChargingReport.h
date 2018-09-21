#ifndef CHARGINGREPORT_H
#define CHARGINGREPORT_H

#include <QWidget>
#include <QTimer>

#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"
#include "ProtocolBase.h"
#include "InfoData.h"

namespace Ui {
class ChargingReport;
}

class ChargingReport : public QWidget, public SwapBase
{
    Q_OBJECT

public:
	explicit ChargingReport(QWidget *parent = 0, CBus * bus = 0, ProtocolBase *protocol = 0);
	~ChargingReport();

	void receiveFromBus(InfoMap Map, InfoAddrType Type);
	void queryChargingReport(unsigned char canAddr);
	void switchToCharging(unsigned char canAddr);
    void queryChargeSpecialFeatureSet();
signals:
	void sigFromBus(InfoMap, InfoAddrType);
	void sigBackToMain(int oldStatus, int newStatus, QVariant var);

private slots:
	void slotBusToOwn(InfoMap, InfoAddrType);
	void slotTimerQueryCharging();
	void on_buttonSave_clicked();
	void on_buttonStopCharging_clicked();
    void timeoutchange();

private:
    Ui::ChargingReport *ui;
    CBus *bus;
    ProtocolBase *protocol;
	QTimer *timerQuery;
    unsigned char canAddr;

    int password;
	
	void queryCardInfo();
	void sendStopChargeCmd();
    void clearChargeReport();
    void buttonStopCharge();

    stSpecialFunc par;

    QWidget *funcWidget;
};

#endif // CHARGINGREPORT_H
