#ifndef TERMINALINFORMATION_H
#define TERMINALINFORMATION_H

#include <QWidget>
#include <QTimer>

#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"
#include "InfoData.h"

namespace Ui {
class TerminalInformation;
}

class TerminalInformation : public QWidget, public SwapBase
{
    Q_OBJECT

public:
    explicit TerminalInformation(QWidget *parent = 0,CBus *bus = 0, ProtocolBase *protocol = 0);
    ~TerminalInformation();
    void receiveFromBus(InfoMap, InfoAddrType);

signals:
   void sigFromBus(InfoMap, InfoAddrType);

private:
    Ui::TerminalInformation *ui;

    CBus *bus;
    ProtocolBase *protocol;
    int currentPosition;
    QTimer *timerQuery;

    QMap<int, unsigned char> canAddrMap;

    void showTermInfo(stTerminalReal data);
    unsigned char calculateCanAddr(int type);
    void queryTermInfo(unsigned char canAddr);
    void clearTerminalInfo();

private slots:
    void queryEnterEnd(QString);
    void slotBusToOwn(InfoMap, InfoAddrType);
    //void on_buttonFind_clicked();
    void on_buttonUp_clicked();
    void on_buttonDown_clicked();
    void timerQueryTerminal();
};

#endif // TERMINALINFORMATION_H
