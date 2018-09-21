#include "ModuleTerm.h"
#include "ui_ModuleTerm.h"

ModuleTerm::ModuleTerm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ModuleTerm)
{
    ui->setupUi(this);
}

ModuleTerm::~ModuleTerm()
{
    delete ui;
}
