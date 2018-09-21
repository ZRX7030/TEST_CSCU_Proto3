#include "AlertDialog.h"
#include "ui_AlertDialog.h"

AlertDialog::AlertDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AlertDialog)
{
    ui->setupUi(this);
}

AlertDialog::~AlertDialog()
{
    delete ui;
}
