#include "ChargeModeSelect.h"
#include "ui_ChargeModeSelect.h"
#include "StatusRemindWindow.h"
#include <QDebug>
#include <QList>

#include "InfoData.h"

ChargeModeSelect::ChargeModeSelect(QWidget *parent, CBus * bus, ProtocolBase *protocol) :
    QWidget(parent),
    ui(new Ui::ChargeModeSelect)
{
    ui->setupUi(this);
    this->bus = bus;
    this->protocol = protocol;
    randomPassword = 0;

    /*注册数据*/
    QList<InfoAddrType> list;
    list.append(InfoAddrConfig);
    list.append(InfoAddrExchange);
    this->bus->registDev(this, list);

    QObject::connect(this,SIGNAL(sigFromBus(InfoMap, InfoAddrType)), this, SLOT(slotBusToOwn(InfoMap, InfoAddrType)), Qt::QueuedConnection);

    //发送查询充电模式设置动态密码
    InfoProtocol infoPro;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoConfigChargeSelectPassword, QByteArray());
    this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
}

ChargeModeSelect::~ChargeModeSelect()
{
    this->bus->cancelRegistDev(this);
    delete ui;
}
void ChargeModeSelect::switchtochargemode()
{
     ui->stackedWidget->setCurrentIndex(0);
     /*发送特殊更能设置查询*/
     InfoProtocol infoPro;
     infoPro.insert(InfoDataType, QByteArray(1, 0));
     infoPro.insert(InfoConfigSpecialFunc, QByteArray());
     this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
}
void ChargeModeSelect::switchtolocalcharge()
{
    ui->stackedWidget->setCurrentIndex(2);
    //发送查询本地充电模式
    InfoProtocol infoPro;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoConfigSpecialFunc, QByteArray());
    this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
}
void ChargeModeSelect::slotBusToOwn(InfoMap Map, InfoAddrType Type)
{
    if(Type == InfoAddrConfig)
    {
        if( Map.contains(InfoConfigChargeSelectPassword))
        {
            QVariant var = Map.value(InfoConfigChargeSelectPassword);
            randomPassword = var.value<unsigned int>();
            ui->showpassword->setText(QString::number(randomPassword, 10));
        }
        else if(Map.contains(InfoConfigSpecialFunc))
        {

            QVariant var = Map.value(InfoConfigSpecialFunc);
            stSpecialFunc param = var.value<stSpecialFunc>();
            memcpy(&special_set,&param,sizeof(param));

            if(param.chargeMode == 0)
                ui->comboBoxchargemode->setCurrentIndex(0);
            else
                ui->comboBoxchargemode->setCurrentIndex(1);
            if(param.localType == 0)
                ui->comboBoxlocalcharge->setCurrentIndex(0);
            else
                ui->comboBoxlocalcharge->setCurrentIndex(1);
        }
    }

    else if(Type == InfoAddrExchange)
    {
        if(false == Map.contains(InfoExchangeParamResult))
            return;
        QVariant var = Map.value(InfoExchangeParamResult);
        stExchangeResult result = var.value<stExchangeResult>();

        if(result.type == InfoConfigChargeSelectPassword)
        {
            StatusRemindWindow  *statusDialog = NULL;
            if(result.result == 0)
                statusDialog = new StatusRemindWindow(this, 1, 5, QObject::tr("动态密码登录失败"));
            else
            {
                switchtochargemode();
                statusDialog= new StatusRemindWindow(this, 0, 5, QObject::tr("动态密码登录成功"));
            }

            statusDialog->exec();
            delete statusDialog;
        }
        if(result.type == InfoConfigSpecialFunc)
        {
            StatusRemindWindow  *statusDialog = NULL;
            if(result.result == 0)
                statusDialog = new StatusRemindWindow(this, 1, 5, QObject::tr("充电模式选择失败"));
            else
            {
                statusDialog= new StatusRemindWindow(this, 0, 5, QObject::tr("充电模式选择成功"));
                if(ui->comboBoxchargemode->currentIndex() == 1)
                    switchtolocalcharge();
            }

            statusDialog->exec();
            delete statusDialog;
        }
    }
}

void ChargeModeSelect::receiveFromBus(InfoMap Map, InfoAddrType type)
{
    emit sigFromBus(Map, type);
}

void ChargeModeSelect::on_buttonokpassword_clicked()
{
    QString inputpassword = ui->lineEditPassword->text();
    unsigned int randompassword = (unsigned int )inputpassword.toLong();
    InfoProtocol infoPro;
    infoPro.insert(InfoDataType, QByteArray(1, 1));
    infoPro.insert(InfoConfigChargeSelectPassword, QByteArray((char *)&randompassword, sizeof(unsigned int )));
    this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
}

void ChargeModeSelect::on_buttonokchargemode_clicked()
{
    InfoProtocol infoPro;
    stSpecialFunc param;

    param.cardAuto = special_set.cardAuto;
    param.cardType = special_set.cardType;
    param.energyFilter = special_set.energyFilter;
    param.localStop = special_set.localStop;
    param.vinAuto = special_set.vinAuto;
    param.vinOffline = special_set.vinOffline;
    param.vinType = special_set.vinType;
    param.localType = special_set.localType;
    param.pileType = special_set.pileType;
    param.coupleGun = special_set.coupleGun;
    param.printPaper = special_set.printPaper;
    param.languageSelect = special_set.languageSelect;
    if(ui->comboBoxchargemode->currentIndex() == 0)
        param.chargeMode = 0;
    else
        param.chargeMode = 1;

    infoPro.insert(InfoDataType, QByteArray(1, 1));
    infoPro.insert(InfoConfigSpecialFunc, QByteArray((char *)&param, sizeof(stSpecialFunc)));
    this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
}


void ChargeModeSelect::on_buttonoklocalcharge_clicked()
{
    InfoProtocol infoPro;
    stSpecialFunc param;
    param.cardAuto = special_set.cardAuto;
    param.cardType = special_set.cardType;
    param.energyFilter = special_set.energyFilter;
    param.localStop = special_set.localStop;
    param.vinAuto = special_set.vinAuto;
    param.vinOffline = special_set.vinOffline;
    param.vinType = special_set.vinType;
    param.pileType = special_set.pileType;
    param.coupleGun = special_set.coupleGun;
    param.printPaper = special_set.printPaper;
    param.languageSelect = special_set.languageSelect;
    param.chargeMode = 1;
    if(ui->comboBoxlocalcharge->currentIndex() == 0)
        param.localType = 0;
    else
        param.localType = 1;

    infoPro.insert(InfoDataType, QByteArray(1, 1));
    infoPro.insert(InfoConfigSpecialFunc, QByteArray((char *)&param, sizeof(stSpecialFunc)));
    this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
}


