#ifndef VERSIONINFORMATION_H
#define VERSIONINFORMATION_H

#include <QWidget>

#include "Common.h"
#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"
#include "InfoData.h"


namespace Ui {
class VersionInformation;
}

class VersionInformation : public QWidget, public SwapBase
{
    Q_OBJECT

public:
    explicit VersionInformation(QWidget *parent = 0, CBus *bus = 0, ProtocolBase *protocol = 0);
    ~VersionInformation();
     void receiveFromBus(InfoMap Map, InfoAddrType Type);
signals:
   void sigFromBus(InfoMap, InfoAddrType);

private slots:
   void slotBusToOwn(InfoMap, InfoAddrType);

private:
    Ui::VersionInformation *ui;
    CBus *bus;
    ProtocolBase *protocol;
    void showVersionInfo(stVersionInformation data);
};

#endif // VERSIONINFORMATION_H
