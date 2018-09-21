#ifndef LOADCONSTRAINTSET_H
#define LOADCONSTRAINTSET_H

#include <QWidget>

#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"
#include "InfoData.h"
#include "Common.h"

namespace Ui {
class LoadConstraintSet;
}

class LoadConstraintSet : public QWidget, public SwapBase
{
    Q_OBJECT

public:
    explicit LoadConstraintSet(QWidget *parent = 0, CBus * bus = 0, ProtocolBase *protocol = 0);
    ~LoadConstraintSet();
	void receiveFromBus(InfoMap, InfoAddrType);

signals:
	void sigFromBus(InfoMap, InfoAddrType);

private slots:
    void on_pushButton_clicked();
	void slotBusToOwn(InfoMap, InfoAddrType);

private:
    Ui::LoadConstraintSet *ui;
	CBus *bus;
	ProtocolBase *protocol;
};

#endif // LOADCONSTRAINTSET_H
