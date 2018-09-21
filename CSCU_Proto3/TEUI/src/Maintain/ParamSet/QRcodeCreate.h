#ifndef QRCODECREATE_H
#define QRCODECREATE_H

#include <QWidget>
#include <QTableWidget>

#include <QTimer>
#include <QVariant>

#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"
#include "InfoData.h"

namespace Ui {
class QRcodeCreate;
}

class QRcodeCreate : public QWidget, public SwapBase
{
    Q_OBJECT

public:
    explicit QRcodeCreate(QWidget *parent = 0, CBus *bus = 0, ProtocolBase *protocol = 0);
    ~QRcodeCreate();

    void receiveFromBus(InfoMap Map, InfoAddrType Type);

signals:
   void sigFromBus(InfoMap, InfoAddrType);

private slots:
   void slotBusToOwn(InfoMap, InfoAddrType);
   void on_buttonUp_clicked();
   void on_buttonDown_clicked();
   void on_buttonSave_clicked();

private:
    Ui::QRcodeCreate *ui;

    int totalNum;
    int totalPage;
    int currentPage;

    QList<stQRcodeCreate> codeCreatelist;

    CBus *bus;
    ProtocolBase *protocol;

    void showCurrnetPage(int startPos, int count);
    void saveCurrnetPage(int startPos, int count);
    int findListPosition(int page,int &startPos, int &count);
    void  createTableItems();
    void tablseItemInit(QTableWidget *table);
};

#endif // QRCODECREATE_H
