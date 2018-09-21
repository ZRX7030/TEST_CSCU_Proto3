#include "ModuleInformation.h"
#include "ui_ModuleInformation.h"


ModuleInformation::ModuleInformation(QWidget *parent, CBus *bus, ProtocolBase *protocol) :
    QWidget(parent),
    ui(new Ui::ModuleInformation)
{
    ui->setupUi(this);
}

ModuleInformation::~ModuleInformation()
{
    delete ui;
}
