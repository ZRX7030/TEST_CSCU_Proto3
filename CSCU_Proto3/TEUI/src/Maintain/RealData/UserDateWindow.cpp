#include "UserDateWindow.h"
#include "ui_UserDateWindow.h"

UserDateWindow::UserDateWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UserDateWindow)
{
    ui->setupUi(this);
}

UserDateWindow::~UserDateWindow()
{
    delete ui;
}
