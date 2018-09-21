#ifndef FAILUREINFORMATION_H
#define FAILUREINFORMATION_H

#include <QWidget>
#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"
#include "InfoData.h"

namespace Ui {
class FailureInformation;
}

class FailureInformation : public QWidget, public SwapBase 
{
    Q_OBJECT

public:
    explicit FailureInformation(QWidget *parent = 0, CBus *bus = 0, ProtocolBase *protocol = 0);
    ~FailureInformation();
	
	void receiveFromBus(InfoMap, InfoAddrType);

signals:
   void sigFromBus(InfoMap, InfoAddrType);

private slots:
    void slotBusToOwn(InfoMap, InfoAddrType);
    void on_pushButtonNextPage_clicked();
    void on_pushButtonPreviousPage_clicked();

private:
    Ui::FailureInformation *ui;
	
	int totalNum;
	int totalPage;
	int currentPage;

	CBus *bus;
	ProtocolBase *protocol;
	
	void createTableItem();
	void queryCurrentPage( int page);
	void showCurrentPageRecord(stAllHistoryFault record);
};

#endif // FAILUREINFORMATION_H
