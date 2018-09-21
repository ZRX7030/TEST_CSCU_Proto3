#include <QDebug>
#include "LoadConstraintSet.h"
#include "StatusRemindWindow.h"
#include "ui_LoadConstraintSet.h"

LoadConstraintSet::LoadConstraintSet(QWidget *parent, CBus * bus, ProtocolBase *protocol) :
    QWidget(parent),
    ui(new Ui::LoadConstraintSet)
{
    ui->setupUi(this);
    ui->lineEditCCUNumber->setInputMask("99");
    ui->lineEditLocalLimitedPower->setInputMask("999");
    ui->lineEditSafetyChargingPower->setInputMask("999");
    ui->lineEditStandTotalLimitedPower->setInputMask("999");

	this->bus = bus;
	this->protocol = protocol;
	/*注册数据*/
	QList<InfoAddrType> list;
	list.append(InfoAddrConfig);
    list.append(InfoAddrExchange);
	this->bus->registDev(this, list);
    
	QObject::connect(this,SIGNAL(sigFromBus(InfoMap, InfoAddrType)), this, SLOT(slotBusToOwn(InfoMap, InfoAddrType)), Qt::QueuedConnection);
    /*发送查询负荷约束设置命令*/
    InfoProtocol infoPro;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoConfigLoad, QByteArray());
    this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
}

LoadConstraintSet::~LoadConstraintSet()
{
	this->bus->cancelRegistDev(this);
    delete ui;
}

void LoadConstraintSet::receiveFromBus(InfoMap Map, InfoAddrType Type)
{
	emit sigFromBus(Map, Type);
}

void LoadConstraintSet::slotBusToOwn(InfoMap Map, InfoAddrType Type)
{
    //qDebug() << " LoadConstraintSet receive data";
	if(Type == InfoAddrConfig)
	{
        if(false == Map.contains(InfoConfigLoad))
			return;

        float tmp;
        bool ok;

        QVariant var = Map.value(InfoConfigLoad);
		stPowerLimitParam param = var.value<stPowerLimitParam>();

        if(param.sPowerLimit_Enable)
            ui->comboBoxLoadConstraintOpenOrClose->setCurrentIndex(1);
        else
            ui->comboBoxLoadConstraintOpenOrClose->setCurrentIndex(0);

        if(param.sSUMPower_Server_Enable)		//服务器下发设置限制功率
            ui->comboBoxPowerEnablingPlatform->setCurrentIndex(1);
        else
            ui->comboBoxPowerEnablingPlatform->setCurrentIndex(0);

       if(param.sSUMPower_Ammeter_Enable)		//电表动态计算设置限制功率
           ui->comboBoxDynamicSettingLimitsPower->setCurrentIndex(1);
       else
           ui->comboBoxDynamicSettingLimitsPower->setCurrentIndex(0);

       if(param.sSUMPower_Manual_Enable)		//点屏设置限制功率
            ui->comboBoxLocalSettingLimitsPower->setCurrentIndex(1);
       else
            ui->comboBoxLocalSettingLimitsPower->setCurrentIndex(0);

        ui->lineEditCCUNumber->setText(QString::number(param.sCCUcount, 10));
        ui->lineEditStandTotalLimitedPower->setText(QString::number(param.STATION_LIMT_POWER, 10));//场站总限制功率
        ui->lineEditLocalLimitedPower->setText(QString::number(param.sSUMPower_Manual, 10));
        ui->lineEditSafetyChargingPower->setText(QString::number(param.SAFE_CHARGE_POWER, 10));//场站安全充电功率
	}
	else if(Type == InfoAddrExchange)
    {
        if(false == Map.contains(InfoExchangeParamResult))
            return;
        QVariant var = Map.value(InfoExchangeParamResult);
        stExchangeResult result = var.value<stExchangeResult>();

        if(result.type == InfoConfigLoad)  //back cscu config reuslt
        {
            StatusRemindWindow  *statusDialog = NULL;
            if(result.result == 0)
                statusDialog = new StatusRemindWindow(this, 1, 5, QObject::tr("负荷约束设置失败"));
            else
                statusDialog= new StatusRemindWindow(this, 0, 5, QObject::tr("负荷约束设置成功"));

            statusDialog->exec();
            delete statusDialog;
        }
    }
}

void LoadConstraintSet::on_pushButton_clicked()
{
    bool ok;
    InfoProtocol infoPro;
    stPowerLimitParam param;

    param.sSUMPower_Server_Enable = 1;
    param.sSUMPower_Ammeter_Enable = 1;
    param.sSUMPower_Manual_Enable = 1;
    param.sPowerLimit_Enable = 1;

    if(ui->comboBoxLoadConstraintOpenOrClose->currentIndex() == 0)
         param.sPowerLimit_Enable = 0;
    if(ui->comboBoxPowerEnablingPlatform->currentIndex() == 0)
        param.sSUMPower_Server_Enable = 0;
    if(ui->comboBoxDynamicSettingLimitsPower->currentIndex() == 0)		//电表动态计算设置限制功率
       param.sSUMPower_Ammeter_Enable = 0;
    if(ui->comboBoxLocalSettingLimitsPower->currentIndex() == 0)		//点屏设置限制功率
        param.sSUMPower_Manual_Enable = 0;

	param.sCCUcount = ui->lineEditCCUNumber->text().toInt(&ok, 10);
    param.STATION_LIMT_POWER = ui->lineEditStandTotalLimitedPower->text().toInt(&ok,10);
    param.sSUMPower_Manual =  ui->lineEditLocalLimitedPower->text().toInt(&ok,10);
    param.SAFE_CHARGE_POWER =  ui->lineEditSafetyChargingPower->text().toInt(&ok,10);

	infoPro.insert(InfoDataType, QByteArray(1, 1));
    infoPro.insert(InfoConfigLoad, QByteArray((char *)&param, sizeof(stPowerLimitParam)));
    this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
}
