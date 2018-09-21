#ifndef BMSINFORMATION_H
#define BMSINFORMATION_H

#include <QWidget>
#include <QTimer>

#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"
#include "InfoData.h"

namespace Ui {
class BMSInformation;
}

class BMSInformation : public QWidget, public SwapBase
{
    Q_OBJECT
    
public:
    explicit BMSInformation(QWidget *parent = 0,CBus *bus = 0, ProtocolBase *protocol = 0);
    ~BMSInformation();
    
	void receiveFromBus(InfoMap, InfoAddrType);

signals:
   void sigFromBus(InfoMap, InfoAddrType);

private slots:
   //void on_buttonFind_clicked();
    void on_buttonUp_clicked();
    void on_buttonDown_clicked();

    void slotBusToOwn(InfoMap, InfoAddrType);
    void queryEnterEnd(QString);
    void timerQueryBMS();

private:
    Ui::BMSInformation *ui;
    CBus *bus;
    ProtocolBase *protocol;
    int currentPosition;

    QTimer *timerQuery;
    QMap<int, unsigned char> canAddrMap;

    void showBmsInfo(stTerminalBMS data);
    unsigned char calculateCanAddr(int type);
    void queryBMSInfo(unsigned char canAddr);
};

#endif // BMSINFORMATION_H
