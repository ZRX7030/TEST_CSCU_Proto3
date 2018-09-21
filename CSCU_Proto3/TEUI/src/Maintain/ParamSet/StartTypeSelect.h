#ifndef STARTTYPESELECT_H
#define STARTTYPESELECT_H

#include <QWidget>
#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"
#include "ProtocolBase.h"
#include "InfoData.h"


namespace Ui {
class StartTypeSelect;
}

class StartTypeSelect : public QWidget
{
    Q_OBJECT

public:
    explicit StartTypeSelect(QWidget *parent = 0, CBus *bus = 0, ProtocolBase *protocol = 0);
    ~StartTypeSelect();

private:
    Ui::StartTypeSelect *ui;
};

#endif // STARTTYPESELECT_H
