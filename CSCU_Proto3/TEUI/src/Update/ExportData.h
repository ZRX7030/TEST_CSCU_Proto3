#ifndef EXPORTDATA_H
#define EXPORTDATA_H

#include <QWidget>
#include "Common.h"
#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"
#include "InfoData.h"

namespace Ui {
class ExportData;
}

class ExportData : public QWidget, public SwapBase
{
    Q_OBJECT

public:
    explicit ExportData(QWidget *parent = 0, CBus *bus = 0, ProtocolBase *protocol = 0, void *param = 0);
    ~ExportData();

    void receiveFromBus(InfoMap, InfoAddrType);
	void switchMainExportUpdate();

signals:
   void sigFromBus(InfoMap, InfoAddrType);
   void sigBackToMain(int oldStatus, int newStatus, QVariant var);
   void sigClickLogo();

private slots:
    void slotBusToOwn(InfoMap, InfoAddrType);

    void on_pushButtonExportLog_clicked();
    void on_pushButtonReturn_clicked();
    void timeoutchange();

    void on_pushButtonUpgradeVersion_clicked();

    void on_pushButtonExportDataBase_clicked();

private:
    Ui::ExportData *ui;
    CBus *bus;
    ProtocolBase *protocol;
	stTeuiParam *teuiParam;
};

#endif // EXPORTDATA_H
