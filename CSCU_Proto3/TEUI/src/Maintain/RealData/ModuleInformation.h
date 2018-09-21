#ifndef MODULEINFORMATION_H
#define MODULEINFORMATION_H

#include <QWidget>

#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"


namespace Ui {
class ModuleInformation;
}

class ModuleInformation : public QWidget
{
    Q_OBJECT
    
public:
    explicit ModuleInformation(QWidget *parent = 0, CBus *bus = 0, ProtocolBase *protocol = 0);
    ~ModuleInformation();
    
private:
    Ui::ModuleInformation *ui;
};

#endif // MODULEINFORMATION_H
