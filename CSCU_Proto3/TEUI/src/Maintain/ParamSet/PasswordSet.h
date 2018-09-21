#ifndef PASSWORDSET_H
#define PASSWORDSET_H

#include "Common.h"
#include <QWidget>
#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"
#include "InfoData.h"

namespace Ui {
class PasswordSet;
}

class PasswordSet : public QWidget, public SwapBase
{
    Q_OBJECT

public:
    explicit PasswordSet(QWidget *parent = 0, CBus *bus = 0, ProtocolBase *protocol = 0);
    ~PasswordSet();
    void receiveFromBus(InfoMap Map, InfoAddrType Type);

signals:
   void sigFromBus(InfoMap, InfoAddrType);
   void sigBackToMain(int type);

private slots:
    void on_pushButtonSave_clicked();
	void slotBusToOwn(InfoMap Map, InfoAddrType Type);

private:
    Ui::PasswordSet *ui;
    CBus *bus;
    ProtocolBase *protocol;
	unsigned int oldPassword;
	unsigned int inputNewPassword;

};

#endif // PASSWORDSET_H
