#ifndef FLASHEXPORTLOG_H
#define FLASHEXPORTLOG_H

#include <QWidget>
#include <QTimer>
#include <QVariant>

#include "Common.h"
#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"
#include "ProtocolBase.h"

namespace Ui {
class FlashExportLog;
}

class FlashExportLog : public QWidget, public SwapBase
{
    Q_OBJECT

public:
    explicit FlashExportLog(QWidget *parent = 0, CBus *bus = 0, ProtocolBase *protocol = 0, void *param = 0);
    ~FlashExportLog();
    void receiveFromBus(InfoMap Map, InfoAddrType Type);
    void switchToFlashExport(QString text, int time, int type);
signals:
   void sigFromBus(InfoMap, InfoAddrType);
   void sigBackToMain(int oldStatus, int newStatus, QVariant var);
   void sigClickLogo();

private slots:
    void slotBusToOwn(InfoMap, InfoAddrType);
    void flashOverTimer();
    void timeoutchange();
private:
    Ui::FlashExportLog *ui;
    QTimer *flashTimer;
    int timerCount;
    CBus *bus;
    ProtocolBase *protocol;
    stTeuiParam *teuiParam;

    unsigned char canAddr;
	int Type;
    void sendToCSCU();
};

#endif // FLASHEXPORTLOG_H
