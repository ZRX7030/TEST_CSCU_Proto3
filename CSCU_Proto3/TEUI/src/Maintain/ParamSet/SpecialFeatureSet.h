#ifndef SPECIALFEATURESET_H
#define SPECIALFEATURESET_H

#include <QWidget>
#include "Common.h"
#include "InfoData.h"
#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"

namespace Ui {
class SpecialFeatureSet;
}

class SpecialFeatureSet : public QWidget, public SwapBase
{
    Q_OBJECT

public:
    explicit SpecialFeatureSet(QWidget *parent = 0, CBus *bus = 0, ProtocolBase *protocol = 0, void *param = 0);
	~SpecialFeatureSet();

	void receiveFromBus(InfoMap, InfoAddrType);

signals:
	void sigFromBus(InfoMap, InfoAddrType);

private slots:
	void slotBusToOwn(InfoMap, InfoAddrType);

    void on_buttonSave_clicked();

private:
   Ui::SpecialFeatureSet *ui;
   
   CBus *bus;
   ProtocolBase *protocol;
   stTeuiParam *teuiParam;
   stSpecialFunc special_set;	//merge by yanwei 20171011
};

#endif // SPECIALFEATURESET_H
