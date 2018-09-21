#include "TerminalChargingInformation.h"
#include "ui_TerminalChargingInformation.h"

TerminalChargingInformation::TerminalChargingInformation(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TerminalChargingInformation)
{
    ui->setupUi(this);
}

TerminalChargingInformation::~TerminalChargingInformation()
{
    delete ui;
}
