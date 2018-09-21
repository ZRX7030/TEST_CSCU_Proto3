#ifndef TERMINALCHARGINGINFORMATION_H
#define TERMINALCHARGINGINFORMATION_H

#include <QWidget>

namespace Ui {
class TerminalChargingInformation;
}

class TerminalChargingInformation : public QWidget
{
    Q_OBJECT

public:
    explicit TerminalChargingInformation(QWidget *parent = 0);
    ~TerminalChargingInformation();

private:
    Ui::TerminalChargingInformation *ui;
};

#endif // TERMINALCHARGINGINFORMATION_H
