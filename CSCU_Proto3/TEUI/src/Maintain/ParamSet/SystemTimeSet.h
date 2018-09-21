#ifndef SYSTEMTIMESET_H
#define SYSTEMTIMESET_H

#include "Common.h"
#include <QWidget>
#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"
#include "InfoData.h"

namespace Ui {
class SystemTimeSet;
}

class SystemTimeSet : public QWidget
{
    Q_OBJECT

public:
    explicit SystemTimeSet(QWidget *parent = 0, CBus *bus = 0, ProtocolBase *protocol = 0);
    ~SystemTimeSet();

private slots:
    void on_pushButtonSave_clicked();

private:
    Ui::SystemTimeSet *ui;
};

#endif // SYSTEMTIMESET_H
