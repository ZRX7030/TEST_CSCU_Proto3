#ifndef ACTRIPHASESET_H
#define ACTRIPHASESET_H

#include <QWidget>
#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"
#include "ProtocolBase.h"
#include "InfoData.h"

namespace Ui {
class ActriphaseSet;
}

class ActriphaseSet : public QWidget, public SwapBase
{
    Q_OBJECT

public:
    explicit ActriphaseSet(QWidget *parent = 0, CBus *bus = 0, ProtocolBase *protocol = 0);
    ~ActriphaseSet();

    void receiveFromBus(InfoMap Map, InfoAddrType Type);

signals:
   void sigFromBus(InfoMap, InfoAddrType);

private slots:
    void slotBusToOwn(InfoMap, InfoAddrType);
    void on_buttonDown_clicked();
    void on_buttonSave_clicked();
    void on_buttonUp_clicked();

private:
    Ui::ActriphaseSet *ui;

    int totalNum;
    int totalPage;
    int currentPage;

    QList<stPhaseType> phaseTypeList;

    CBus *bus;
    ProtocolBase *protocol;

    void showCurrnetPage(int startPos, int count);
    void saveCurrnetPage(int startPos, int count);
	int findListPosition(int page, int &startPos, int &count);
    void createTableItems();
};

#endif // ACTRIPHASESET_H
