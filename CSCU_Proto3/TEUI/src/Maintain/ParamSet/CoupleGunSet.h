#ifndef COUPLEGUNSET_H
#define COUPLEGUNSET_H

#include <QWidget>
#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"
#include "InfoData.h"

namespace Ui {
class CoupleGunSet;
}

class CoupleGunSet : public QWidget, public SwapBase
{
    Q_OBJECT
    
public:
    explicit CoupleGunSet(QWidget *parent = 0, CBus *bus = 0, ProtocolBase *protocol = 0);
    ~CoupleGunSet();
    void receiveFromBus(InfoMap Map, InfoAddrType Type);

signals:
   void sigFromBus(InfoMap, InfoAddrType);
    
private:
    Ui::CoupleGunSet *ui;
    CBus *bus;
    ProtocolBase *protocol;
    stSpecialFunc special_set;  //add by songqb 2018-1-16

private slots:
    void slotBusToOwn(InfoMap Map, InfoAddrType Type);
    void on_pushButtonCoupleSet_clicked();
    void on_pushButtonCoupleMode_clicked();
};

#endif // COUPLEGUNSET_H
