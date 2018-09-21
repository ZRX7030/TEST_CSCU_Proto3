#include "CoupleGunSet.h"
#include "ui_CoupleGunSet.h"
#include "StatusRemindWindow.h"

CoupleGunSet::CoupleGunSet(QWidget *parent, CBus * bus, ProtocolBase *protocol) :
    QWidget(parent),
    ui(new Ui::CoupleGunSet)
{
    ui->setupUi(this);
    this->bus = bus;
    this->protocol = protocol;
    ui->stackedWidget->setCurrentIndex(0);
    /*注册数据*/
    QList<InfoAddrType> list;
    list.append(InfoAddrConfig);
    list.append(InfoAddrExchange);
    this->bus->registDev(this, list);

    QObject::connect(this,SIGNAL(sigFromBus(InfoMap, InfoAddrType)), this, SLOT(slotBusToOwn(InfoMap, InfoAddrType)), Qt::QueuedConnection);

    /*发送特殊更能设置查询*/
    InfoProtocol infoPro;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoConfigSpecialFunc, QByteArray());
    this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
}

CoupleGunSet::~CoupleGunSet()
{
    this->bus->cancelRegistDev(this);
    delete ui;
}

void CoupleGunSet::slotBusToOwn(InfoMap Map, InfoAddrType Type)
{
    if(Type == InfoAddrConfig)
    {
        if(Map.contains(InfoConfigSpecialFunc))
        {
            QVariant var = Map.value(InfoConfigSpecialFunc);
            stSpecialFunc param = var.value<stSpecialFunc>();
            memcpy(&special_set,&param,sizeof(param));

            if(param.coupleGun == 0)
                ui->comboBoxCoupleState->setCurrentIndex(0);
            else
            {
                ui->comboBoxCoupleState->setCurrentIndex(1);
                if(param.coupleGun == 1)
                    ui->comboBoxCoupleGunType->setCurrentIndex(1);
                else if(param.coupleGun == 2)
                    ui->comboBoxCoupleGunType->setCurrentIndex(2);
                else if(param.coupleGun == 3)
                    ui->comboBoxCoupleGunType->setCurrentIndex(3);
            }
        }
     }
    else if(Type == InfoAddrExchange)
    {
        if(false == Map.contains(InfoExchangeParamResult))
            return;
        QVariant var = Map.value(InfoExchangeParamResult);
        stExchangeResult result = var.value<stExchangeResult>();

        if(result.type == InfoConfigSpecialFunc)
        {
            StatusRemindWindow  *statusDialog = NULL;
            if(result.result == 0)
                statusDialog = new StatusRemindWindow(this, 1, 5, QObject::tr("多枪充电设置失败"));
            else
            {
                statusDialog= new StatusRemindWindow(this, 0, 5, QObject::tr("多枪充电设置成功"));
            }
            statusDialog->exec();
            delete statusDialog;
        }
    }
}

void CoupleGunSet::receiveFromBus(InfoMap Map, InfoAddrType Type)
{
    emit sigFromBus(Map, Type);
}

/**
 *多枪使能是否开启
 */
void CoupleGunSet::on_pushButtonCoupleSet_clicked()
{
    InfoProtocol infoPro;
    stSpecialFunc param;
    if(ui->comboBoxCoupleState->currentIndex() == 0)
    {
        param.cardAuto = special_set.cardAuto;
        param.cardType = special_set.cardType;
        param.energyFilter = special_set.energyFilter;
        param.localStop = special_set.localStop;
        param.vinAuto = special_set.vinAuto;
        param.vinOffline = special_set.vinOffline;
        param.vinType = special_set.vinType;
        param.localType = special_set.localType;
        param.pileType = special_set.pileType;
        param.chargeMode = special_set.chargeMode;
        param.printPaper = special_set.printPaper;
        param.languageSelect = special_set.languageSelect;
        param.coupleGun = 0;

        infoPro.insert(InfoDataType, QByteArray(1, 1));
        infoPro.insert(InfoConfigSpecialFunc, QByteArray((char *)&param, sizeof(stSpecialFunc)));
        this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
    }
    else
    {
        ui->stackedWidget->setCurrentIndex(1);
    }
}

/**
 *多枪开启后启动充电方式选择
 */
void CoupleGunSet::on_pushButtonCoupleMode_clicked()
{
    if(ui->comboBoxCoupleGunType->currentIndex() == 0)
    {
        StatusRemindWindow  *statusDialog = NULL;
        statusDialog = new StatusRemindWindow(this, 1, 5, QObject::tr("充电模式不能为空，请重新选择！"));
        statusDialog->exec();
        delete statusDialog;
        return;
    }

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
    param.chargeMode = special_set.chargeMode;
    param.printPaper = special_set.printPaper;
    param.languageSelect = special_set.languageSelect;
    if(ui->comboBoxCoupleGunType->currentIndex() == 1)
        param.coupleGun = 1;
    else if(ui->comboBoxCoupleGunType->currentIndex() == 2)
        param.coupleGun = 2;
    else if(ui->comboBoxCoupleGunType->currentIndex() == 3)
        param.coupleGun = 3;

    infoPro.insert(InfoDataType, QByteArray(1, 1));
    infoPro.insert(InfoConfigSpecialFunc, QByteArray((char *)&param, sizeof(stSpecialFunc)));
    this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
}
