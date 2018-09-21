#ifndef CHARGEPASSWORD_H
#define CHARGEPASSWORD_H

#include "Common.h"
#include <QWidget>
#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"
#include "InfoData.h"

namespace Ui {
class ChargePassword;
}

class ChargePassword : public QWidget, public SwapBase
{
    Q_OBJECT
    
public:
    explicit ChargePassword(QWidget *parent = 0, CBus *bus = 0, ProtocolBase *protocol = 0);
    ~ChargePassword();
    void receiveFromBus(InfoMap Map, InfoAddrType Type);

signals:
   void sigFromBus(InfoMap, InfoAddrType);
   void sigBackToMain(int type);

private slots:
void slotBusToOwn(InfoMap Map, InfoAddrType Type);

void on_pushButtonSave_clicked();

private:
    Ui::ChargePassword *ui;
    CBus *bus;
    ProtocolBase *protocol;
    unsigned int oldPassword;
    unsigned int inputNewPassword;
};

#endif // CHARGEPASSWORD_H
