#ifndef LINESIDEINFORMATION_H
#define LINESIDEINFORMATION_H
#include <QTimer>

#include <QWidget>
#include <QTimer>
#include <QMap>
#include <QByteArray>

#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"
#include "InfoData.h"

namespace Ui {
class LineSideInformation;
}

class LineSideInformation : public QWidget, public SwapBase
{
    Q_OBJECT

public:
    explicit LineSideInformation(QWidget *parent = 0, CBus *bus = 0, ProtocolBase *protocol = 0);
    ~LineSideInformation();
    void receiveFromBus(InfoMap, InfoAddrType);

signals:
   void sigFromBus(InfoMap, InfoAddrType);

private slots:
    void on_buttonUp_clicked();
    void slotBusToOwn(InfoMap, InfoAddrType);
    void on_buttonDown_clicked();
    void timerQueryAmmeter();

private:
    Ui::LineSideInformation *ui;

    CBus *bus;
    ProtocolBase *protocol;
    QMap<int ,QByteArray> addrMap;
    QTimer *timerQuery;

    int totalNum;
    int currentNum;

    void queryAmmeterData(int position);
    void showAmmeterData(stAmmeterData data);
    bool BCDToChar( unsigned char* pBCD, int iLen, unsigned char* pChar );
};

#endif // LINESIDEINFORMATION_H
