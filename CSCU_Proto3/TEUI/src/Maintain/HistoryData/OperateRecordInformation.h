#ifndef OPERATERECORDINFORMATION_H
#define OPERATERECORDINFORMATION_H

#include <QWidget>
#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"
#include "InfoData.h"

namespace Ui {
class OperateRecordInformation;
}

class OperateRecordInformation : public QWidget, public SwapBase
{
    Q_OBJECT

public:
    explicit OperateRecordInformation(QWidget *parent = 0, CBus *bus = 0, ProtocolBase *protocol = 0);
    ~OperateRecordInformation();
    void receiveFromBus(InfoMap, InfoAddrType);

signals:
   void sigFromBus(InfoMap, InfoAddrType);

private slots:
    void slotBusToOwn(InfoMap, InfoAddrType);

    void on_pushButtonUp_clicked();

    void on_pushButtonNext_clicked();

private:
    Ui::OperateRecordInformation *ui;
    int totalNum;
    int totalPage;
    int currentPage;

    CBus *bus;
    ProtocolBase *protocol;

    void createTableItem();
    void queryCurrentPage( int page);
    void showCurrentPageRecord(stAllHistoryOperate record);
};

#endif // OPERATERECORDINFORMATION_H
