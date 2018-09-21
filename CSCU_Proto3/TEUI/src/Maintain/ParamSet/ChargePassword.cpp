#include <QDebug>
#include <QList>

#include "InfoData.h"
#include "ChargePassword.h"
#include "ui_ChargePassword.h"
#include "StatusRemindWindow.h"

ChargePassword::ChargePassword(QWidget *parent, CBus *bus, ProtocolBase *protocol) :
    QWidget(parent),
    ui(new Ui::ChargePassword)
{
    ui->setupUi(this);
    this->bus = bus;
    this->protocol = protocol;
    oldPassword  = 0;
    inputNewPassword = 0;

    ui->lineEditConfirmNewPassword->setInputMask("999999");
    ui->lineEditNewPassword->setInputMask("999999");
    ui->lineEditOldPassword->setInputMask("999999");

    /*注册数据*/
    QList<InfoAddrType> list;
    list.append(InfoAddrConfig);
    list.append(InfoAddrExchange);
    this->bus->registDev(this, list);

    QObject::connect(this,SIGNAL(sigFromBus(InfoMap, InfoAddrType)), this, SLOT(slotBusToOwn(InfoMap, InfoAddrType)), Qt::QueuedConnection);

    //发送密码读取操作
    //qDebug() <<"send password /////////////////////////////////";
    InfoProtocol infoPro;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoConfigChangeChargePassword, QByteArray());
    this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
}
void ChargePassword::slotBusToOwn(InfoMap Map, InfoAddrType Type)
{
    if(Type == InfoAddrConfig)
    {
        if(false == Map.contains(InfoConfigChangeChargePassword))
            return;

        QVariant var = Map.value(InfoConfigChangeChargePassword);
        oldPassword = var.value<unsigned int>();
        //qDebug() << "receive old password is " << QString::number(oldPassword, 10) << "lllllllllll";
    }
    else if(Type == InfoAddrExchange)
    {
        if(false == Map.contains(InfoExchangeParamResult))
            return;

        QVariant var = Map.value(InfoExchangeParamResult);
        stExchangeResult result = var.value<stExchangeResult>();

        if(result.type == InfoConfigChangeChargePassword)
        {
            StatusRemindWindow  *statusDialog = NULL;
            if(result.result == 0)
                statusDialog = new StatusRemindWindow(this, 1, 5, QObject::tr("本地充电密码修改失败"));
            else
            {
                this->oldPassword = this->inputNewPassword;
                statusDialog= new StatusRemindWindow(this, 0, 5, QObject::tr("本地充电密码修改成功"));
            }

            statusDialog->exec();
            delete statusDialog;
        }
    }
}
ChargePassword::~ChargePassword()
{
    this->bus->cancelRegistDev(this);
    delete ui;
}
void ChargePassword::receiveFromBus(InfoMap Map, InfoAddrType type)
{
     emit sigFromBus(Map, type);
}


void ChargePassword::on_pushButtonSave_clicked()
{
    QString OldPassword = ui->lineEditOldPassword->text();
    QString NewPassword = ui->lineEditNewPassword->text();
    QString ConfirmPassword = ui->lineEditConfirmNewPassword->text();

    if(NewPassword.length() < 6 || ConfirmPassword.length() < 6)
    {
        StatusRemindWindow  *statusDialog= new StatusRemindWindow(this, 1, 5, QObject::tr("请输入6位密码"));
        statusDialog->exec();
        delete statusDialog;
    }
    else if(QString::number(this->oldPassword, 10) != OldPassword)
    {

        StatusRemindWindow  *statusDialog= new StatusRemindWindow(this, 1, 5, QObject::tr("旧的密码输入错误"));
        statusDialog->exec();
        delete statusDialog;
    }
    else if( NewPassword != ConfirmPassword )
    {
        StatusRemindWindow  *statusDialog= new StatusRemindWindow(this, 1, 5, QObject::tr("两次输入的新密码不一致"));
        statusDialog->exec();
        delete statusDialog;
    }
    else
    {
        unsigned int newPassword = (unsigned int )NewPassword.toLong();
        InfoProtocol infoPro;
        infoPro.insert(InfoDataType, QByteArray(1, 1));
        infoPro.insert(InfoConfigChangeChargePassword, QByteArray((char *)&newPassword, sizeof(unsigned int )));
        this->protocol->sendProtocolData(infoPro, InfoAddrConfig);

        inputNewPassword = (unsigned int)ui->lineEditConfirmNewPassword->text().toLong();
    }
}
