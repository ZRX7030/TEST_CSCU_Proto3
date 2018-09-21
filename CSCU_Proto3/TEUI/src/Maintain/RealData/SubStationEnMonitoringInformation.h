#ifndef SUBSTATIONENMONITORINGINFORMATION_H
#define SUBSTATIONENMONITORINGINFORMATION_H

#include <QWidget>
#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"

namespace Ui {
class SubStationEnMonitoringInformation;
}

class SubStationEnMonitoringInformation : public QWidget , public SwapBase
{
    Q_OBJECT

public:
    explicit SubStationEnMonitoringInformation(QWidget *parent = 0, CBus *bus = 0, ProtocolBase *protocol = 0);
    ~SubStationEnMonitoringInformation();
    void receiveFromBus(InfoMap Map, InfoAddrType type);

signals:
    void sigFromBus(InfoMap, InfoAddrType);
private slots:
    void on_pushButtonOK_clicked();
    void slotBusToOwn(InfoMap Map, InfoAddrType Type);
    void timerQuerySubStation();

private:
    Ui::SubStationEnMonitoringInformation *ui;
    CBus *bus;
    ProtocolBase *protocol;
    QTimer *timerQuery;

    void queryStationConfig();
    void queryRealStation();
};

#endif // SUBSTATIONENMONITORINGINFORMATION_H
