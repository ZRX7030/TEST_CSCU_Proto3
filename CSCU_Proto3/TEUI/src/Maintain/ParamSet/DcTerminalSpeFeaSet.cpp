#include "DcTerminalSpeFeaSet.h"
#include "ui_DcTerminalSpeFeaSet.h"

DcTerminalSpeFeaSet::DcTerminalSpeFeaSet(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DcTerminalSpeFeaSet)
{
    ui->setupUi(this);
}

DcTerminalSpeFeaSet::~DcTerminalSpeFeaSet()
{
    delete ui;
}
