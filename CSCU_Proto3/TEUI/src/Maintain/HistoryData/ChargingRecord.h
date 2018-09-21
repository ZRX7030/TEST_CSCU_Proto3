#ifndef CHARGINGRECORD_H
#define CHARGINGRECORD_H

#include <QWidget>

#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"
#include "InfoData.h"

namespace Ui {
class ChargingRecord;
}

class ChargingRecord : public QWidget, public SwapBase
{
    Q_OBJECT
    
public:
    explicit ChargingRecord(QWidget *parent = 0, CBus *bus = 0, ProtocolBase *protocol = 0);
    ~ChargingRecord();
    void receiveFromBus(InfoMap, InfoAddrType);

signals:
   void sigFromBus(InfoMap, InfoAddrType);

private slots:
    void slotBusToOwn(InfoMap, InfoAddrType);
    void on_buttonUp_clicked();
    void on_buttonNext_clicked();

private:
    Ui::ChargingRecord *ui;
   
	int totalNum;
	int totalPage;
	int currentPage;

	CBus *bus;
	ProtocolBase *protocol;

	void createTableItem();
	void queryCurrentPage( int page);
	void showCurrentPageRecord(stAllHistoryCharge record);
};

#endif // CHARGINGRECORD_H
