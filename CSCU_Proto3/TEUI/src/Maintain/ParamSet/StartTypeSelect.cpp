#include "StartTypeSelect.h"
#include "ui_StartTypeSelect.h"

StartTypeSelect::StartTypeSelect(QWidget *parent, CBus *bus, ProtocolBase *protocol) :
    QWidget(parent),
    ui(new Ui::StartTypeSelect)
{
    ui->setupUi(this);
}

StartTypeSelect::~StartTypeSelect()
{
    delete ui;
}
