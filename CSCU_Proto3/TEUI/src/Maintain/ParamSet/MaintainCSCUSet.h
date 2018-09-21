#ifndef MAINTAINCSCUSET_H
#define MAINTAINCSCUSET_H

#include <QWidget>

#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"
#include "Common.h"

namespace Ui {
class MaintainCSCUSet;
}


class MaintainCSCUSet : public QWidget, public SwapBase
{
    Q_OBJECT

public:
    explicit MaintainCSCUSet(QWidget *parent = 0, CBus *bus = 0, ProtocolBase *protocol = 0, void *param = 0);
    ~MaintainCSCUSet();
	void receiveFromBus(InfoMap, InfoAddrType);

private:
    Ui::MaintainCSCUSet *ui;
	CBus *bus;
	ProtocolBase *protocol;
	stTeuiParam *teuiParam;

    bool ipValidCheck(QString ip );
signals:
   void sigFromBus(InfoMap, InfoAddrType);

private slots:
   void slotBusToOwn(InfoMap, InfoAddrType);
   void on_buttonSave_clicked();
};

#endif // MAINTAINCSCUSET_H
