#include "src/Charge/PasswordCharge.h"
#include "ui_PasswordCharge.h"
#include "TimeLimit.h"
#include <qvalidator.h>

PasswordCharge::PasswordCharge(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PasswordCharge)
{
    ui->setupUi(this);
    ui->timeLimit->startDownCout(30);
    setWindowFlags(Qt::FramelessWindowHint|Qt::Dialog);

    ui->lineEditPassword->setInputMask("999999");  //设置输入最大字符数6
     QObject::connect(ui->timeLimit, SIGNAL(timeout()), this, SLOT(timeoutchange()));
}

PasswordCharge::~PasswordCharge()
{
    delete ui;
}

QString PasswordCharge::getInputPassword()
{
    return ui->lineEditPassword->text();
}
void PasswordCharge::timeoutchange()
{
    ui->timeLimit->stopDownCout();
    done(-1);
    close();
}
void PasswordCharge::on_pushButtonReturn_clicked()
{
    done(-1);
    close();
}


void PasswordCharge::on_pushButtonPasswordCharge_clicked()
{
    done(1);
    close();
}
