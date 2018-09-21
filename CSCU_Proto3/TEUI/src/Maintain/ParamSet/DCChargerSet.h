#ifndef DCCHARGERSET_H
#define DCCHARGERSET_H
#include <QTimer>
#include <QTableWidget>
#include <QWidget>
#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"
#include <QVariant>
#include "InfoData.h"



namespace Ui {
class DCChargerSet;
}

class DCChargerSet : public QWidget, public SwapBase
{
    Q_OBJECT

public:
    explicit DCChargerSet(QWidget *parent = 0, CBus *bus = 0, ProtocolBase *protocol = 0);
    ~DCChargerSet();
	void receiveFromBus(InfoMap, InfoAddrType);

signals:
   void sigFromBus(InfoMap, InfoAddrType);

private slots:
	void timerOver();
    void slotBusToOwn(InfoMap, InfoAddrType);
    void on_buttonSave_clicked();
    void on_buttonUp_clicked();
    void on_buttonDown_clicked();
    void on_tabWidgetChargerSet_currentChanged(int index);


private:
    Ui::DCChargerSet *ui;
    CBus *bus;
    ProtocolBase *protocol;

    int totalNum;
    int totalPage;
    int currentPage;
	int leftNum;

	int itemCount;
	int currentPageItemCount;
	
	stDCChargerDeviceNum deviceAddrList;			//地址列表list
    QMap<unsigned char, stDCChargerTermParam>	terminalDataMap;		//存放终端数据
    QMap<unsigned char, stDCChargerMonitorParam> monitorDataMap;		//存放监控数据
    QTimer *timerInit;

	void terminalTableInit();
    void createTableItems();
    void tablseItemInit(QTableWidget *table);
	void currentPageInit(int page);

    void showTermData(stDCChargerTermParam data);
    void showMonitorData(stDCChargerMonitorParam data);
    
	void saveTermData();
    void saveMonitorData();
    void clearTerminalInfor();
    void clearMonitorInfor();
    void setMonitorcolumn();
};

#endif // DCCHARGERSET_H
