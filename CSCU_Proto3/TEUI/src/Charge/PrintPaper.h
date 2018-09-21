#ifndef PRINTPAPER_H
#define PRINTPAPER_H

#include <QWidget>
#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"
#include "InfoData.h"

namespace Ui {
class PrintPaper;
}

class PrintPaper : public QWidget, public SwapBase
{
    Q_OBJECT
    
public:
    explicit PrintPaper(QWidget *parent = 0,CBus *bus = 0, ProtocolBase *protocol = 0);
    void receiveFromBus(InfoMap, InfoAddrType);
    void switchToPrintPaper(unsigned char canAddr);
    ~PrintPaper();

signals:
    void sigBackToMain(int oldStatus, int newStatus, QVariant var);
private slots:
    void timeoutchange();
    void on_buttonOK_clicked();
    void on_buttonBack_clicked();

private:
    Ui::PrintPaper *ui;
    CBus *bus;
    ProtocolBase *protocol;
    unsigned char canAddr;

    void sendPrintPaper();
};

#endif // PRINTPAPER_H
