#ifndef CHARGEMODESELECT_H
#define CHARGEMODESELECT_H

#include <QWidget>
#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"
#include "InfoData.h"

namespace Ui {
class ChargeModeSelect;
}

class ChargeModeSelect : public QWidget, public SwapBase
{
    Q_OBJECT
    
public:
    explicit ChargeModeSelect(QWidget *parent = 0, CBus *bus = 0, ProtocolBase *protocol = 0);
    ~ChargeModeSelect();
    void receiveFromBus(InfoMap Map, InfoAddrType Type);

signals:
   void sigFromBus(InfoMap, InfoAddrType);
    
private:
    Ui::ChargeModeSelect *ui;
    CBus *bus;
    ProtocolBase *protocol;
    stSpecialFunc special_set;  //add by songqb 2018-1-15

    unsigned int inputPassword;
    unsigned int randomPassword;
    void switchtochargemode();
    void switchtolocalcharge();

private slots:
    void slotBusToOwn(InfoMap Map, InfoAddrType Type);

    void on_buttonokpassword_clicked();
    void on_buttonoklocalcharge_clicked();
    void on_buttonokchargemode_clicked();
};

#endif // CHARGEMODESELECT_H
