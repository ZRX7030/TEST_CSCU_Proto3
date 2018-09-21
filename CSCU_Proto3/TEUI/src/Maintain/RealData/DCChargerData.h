#ifndef DCCHARGERDATA_H
#define DCCHARGERDATA_H

#include <QTableWidget>
#include <QWidget>
#include <QTimer>

#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"
#include "ProtocolBase.h"
#include "InfoData.h"



namespace Ui {
class DCChargerData;
}

class DCChargerData : public QWidget, public SwapBase
{
    Q_OBJECT

public:
    explicit DCChargerData(QWidget *parent = 0, CBus *bus = 0, ProtocolBase *protocol = 0);
    ~DCChargerData();
	void receiveFromBus(InfoMap, InfoAddrType);

signals:
   void sigFromBus(InfoMap, InfoAddrType);

private slots:
	void slotQueryTerminal();
    void slotBusToOwn(InfoMap, InfoAddrType);
    void on_buttonUp_clicked();
    void on_buttonDown_clicked();
    void on_tabWidgetDCInformation_currentChanged(int index);

private:
    Ui::DCChargerData *ui;
	
	CBus *bus;
    ProtocolBase *protocol;

    int itemCount;         //用于记录更新哪一行数据
    int queryPositon;

	int currentPage;	  //用于翻页记录
    int totalNum;
    int totalPage;
    int leftNum;
	int currentPageItemCount;

	stAllDCChargerTypeNum addrParam;		//pdu ccu module  地址参数
	QList<stDCChargerTypeNum> queryCanList;
    QTimer *timerQuery; 

    void createTableItems();
	void currentPageInit(int page);
	void tablseItemInit(QTableWidget *table);
    void settablecolumncount();
	void clearCurrentTable(QTableWidget *table);

	void showModuleInfo(stDCModuleRealData module);
	void showPDUInfo(stDCPduRealData pdu);
	void showCCUInfo(stDCCcuRealData ccu);
};

#endif // DCCHARGERDATA_H
