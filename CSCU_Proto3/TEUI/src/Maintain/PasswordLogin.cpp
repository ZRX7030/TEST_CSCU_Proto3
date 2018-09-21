#include "PasswordLogin.h"
#include "ui_PasswordLogin.h"
#include "TimeLimit.h"
#include <qvalidator.h>

PasswordLogin::PasswordLogin(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PasswordLogin)
{
    ui->setupUi(this);
    ui->timeLimit->hide();
    //ui->lineEditPassword->setValidator(new QIntValidator(100000, 999999, this));
    //ui->timeLimit->startDownCout(20);
    setWindowFlags(Qt::FramelessWindowHint|Qt::Dialog);
	//this->setAttribute(Qt::WA_DeleteOnClose, true);
	//this->setWindowOpacity(1); //窗口整体透明度，0-1 从全透明到不透明
    //this->setAttribute(Qt::WA_TranslucentBackground); //设置背景透明，允许鼠标穿透
    ui->lineEditPassword->setInputMask("999999");  //设置输入最大字符数6
    //ui->lineEditPassword->setMaxLength(6);
    //ui->lineEditPassword->setEchoMode(QLineEdit::Password);

    //QObject::connect(ui->timeLimit, SIGNAL(timeout()), this, SLOT(timeoutchange()));
}

PasswordLogin::~PasswordLogin()
{
    delete ui;
}


QString PasswordLogin::getInputPassword()
{
    return ui->lineEditPassword->text();
}

void PasswordLogin::timeoutchange()
{
    ui->timeLimit->stopDownCout();
    done(-1);
    close();
}
void PasswordLogin::on_pushButtonLogin_clicked()
{
    done(1);
    close();
}

void PasswordLogin::on_pushButtonReturn_clicked()
{
    done(-1);
    close();
}
