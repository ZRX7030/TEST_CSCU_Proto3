#ifndef PEAKCHARGINGINFORMATIONVIEW_H
#define PEAKCHARGINGINFORMATIONVIEW_H

#include <QWidget>
#include <QTableWidget>

#include "Common.h"
#include "InfoData.h"
#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"


namespace Ui {
class PeakChargingInformationView;
}

class PeakChargingInformationView : public QWidget, public SwapBase
{
    Q_OBJECT

public:
    explicit PeakChargingInformationView(QWidget *parent = 0,CBus *bus = 0, ProtocolBase *protocol = 0);
    ~PeakChargingInformationView();

	void receiveFromBus(InfoMap, InfoAddrType);

signals:
	void sigFromBus(InfoMap, InfoAddrType);

private slots:
	void slotBusToOwn(InfoMap, InfoAddrType);
    void on_buttonSave_clicked();

    void on_tabWidget_currentChanged(int index);

private:
    Ui::PeakChargingInformationView *ui;
	
	CBus *bus;
	ProtocolBase *protocol;

    QList<stTPFVParam> tipList;
    QList<stTPFVParam> peakList;
    QList<stTPFVParam> flatList;
    QList<stTPFVParam> valleyList;

    void createTableItems();
    void showTPFVTables(int seg);
    void tablseItemInit(QTableWidget *table);
};

#endif // PEAKCHARGINGINFORMATIONVIEW_H
