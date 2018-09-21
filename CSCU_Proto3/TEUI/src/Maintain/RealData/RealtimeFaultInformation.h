#ifndef REALTIMEFAULTINFORMATION_H
#define REALTIMEFAULTINFORMATION_H

#include <QWidget>
#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"
#include "InfoData.h"

namespace Ui {
class RealtimeFaultInformation;
}

class RealtimeFaultInformation : public QWidget, public SwapBase
{
    Q_OBJECT
    
public:
    explicit RealtimeFaultInformation(QWidget *parent = 0, CBus *bus = 0, ProtocolBase *protocol = 0);
    ~RealtimeFaultInformation();

    void receiveFromBus(InfoMap, InfoAddrType);

signals:
   void sigFromBus(InfoMap, InfoAddrType);

private slots:
   void slotBusToOwn(InfoMap, InfoAddrType);
   void on_prevPageBtn_clicked();

   void on_nextPageBtn_clicked();

private:
    Ui::RealtimeFaultInformation *ui;

    int totalNum;
    int totalPage;
    int currentPage;

    CBus *bus;
    ProtocolBase *protocol;

    void createTableItem();
    void queryCurrentPage(int page);
    void showCurrentPageRecord(stAllRealtimeFault record);
};

#endif // REALTIMEFAULTINFORMATION_H
