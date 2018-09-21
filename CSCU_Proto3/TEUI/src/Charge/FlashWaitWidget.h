#ifndef FLASHWAITWIDGET_H
#define FLASHWAITWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QVariant>

#include "Common.h"
#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"
#include "ProtocolBase.h"


namespace Ui {
class FlashWaitWidget;
}

class FlashWaitWidget : public QWidget, public SwapBase
{
    Q_OBJECT

public:
    explicit FlashWaitWidget(QWidget *parent = 0, CBus *bus = 0, ProtocolBase *protocol = 0);
    ~FlashWaitWidget();
    void receiveFromBus(InfoMap Map, InfoAddrType Type);
    void switchToFlashWait(unsigned char canAddr,unsigned char chargeType, QString text, int time, int flag, InfoBodyType type);

signals:
   void sigFromBus(InfoMap, InfoAddrType);
   void sigBackToMain(int oldStatus, int newStatus, QVariant var);
   void sigBackToMainIntel(int oldStatus, int newStatus, QVariant var);


private slots:
	void slotBusToOwn(InfoMap, InfoAddrType);
    void slotBackToMainIntel(int oldStatus, int newStatus, QVariant var);
	void flashOverTimer();
    void timeoutchange();

private:
    Ui::FlashWaitWidget *ui;
    QTimer *flashTimer;
    int timerCount;
    CBus *bus;
    ProtocolBase *protocol;

	unsigned char canAddr;
    unsigned char chargeType;
	int startStopFlag;
	InfoBodyType Type;

	void sendToCSCU();
};

#endif // FLASHWAITWIDGET_H
