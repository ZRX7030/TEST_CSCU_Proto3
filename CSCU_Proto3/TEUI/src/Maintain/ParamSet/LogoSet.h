#ifndef LOGOSET_H
#define LOGOSET_H

#include <QWidget>
#include "Common.h"
#include "InfoData.h"
#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"

namespace Ui {
class LogoSet;
}

class LogoSet : public QWidget
{
    Q_OBJECT

public:
    explicit LogoSet(QWidget *parent = 0, CBus *bus = 0, ProtocolBase *protocol = 0);
    ~LogoSet();

private slots:
    void on_buttonSave_clicked();

private:
    Ui::LogoSet *ui;
    void Readfile();
};

#endif // LOGOSET_H
