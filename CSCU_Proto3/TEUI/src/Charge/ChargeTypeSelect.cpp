#include <string.h>
#include <stdlib.h>

#include <QDebug>

#include "InfoData.h"
#include "ChargeTypeSelect.h"
#include "ui_ChargeTypeSelect.h"
#include "InfoData.h"
#include "TimeLimit.h"
#include "Common.h"
#include "ChargeTerm.h"
#include "StatusRemindWindow.h"

ChargeTypeSelect::ChargeTypeSelect(QWidget *parent, CBus *bus, ProtocolBase *protocol) :
    QWidget(parent),
    ui(new Ui::ChargeTypeSelect)
{
    ui->setupUi(this);
    this->bus = bus;
    this->protocol = protocol;
	this->canAddr = 0;
    /*注册数据*/
//    QList<InfoAddrType> list;
 //   list.append(InfoAddrNone);
 //   this->bus->registDev(this, list);
  //  QObject::connect(this,SIGNAL(sigFromBus(InfoMap, InfoAddrType)), this, SLOT(slotBusToOwn(InfoMap, InfoAddrType)), Qt::QueuedConnection);
    QObject::connect(ui->timeLimit, SIGNAL(timeout()), this, SLOT(timeoutchange()));
}

ChargeTypeSelect::~ChargeTypeSelect()
{
   // this->bus->cancelRegistDev(this);
    delete ui;
}
/**
 *向cscu下发命令
 */
void ChargeTypeSelect::sendToCSCU(char chargeType, unsigned short chargeEnergy)
{
	InfoProtocol infoPro;
	stChargeCmd chargeCmd;

	memset(&chargeCmd , 0, sizeof(stChargeCmd));
	chargeCmd.canAddr = this->canAddr;
	chargeCmd.cmd = 1;
	chargeCmd.status = CHARGE_STATUS_GUN_STANDBY; 
	chargeCmd.chargeEnergy = chargeEnergy;
	chargeCmd.chargeType = chargeType;

	infoPro.insert(InfoDataType, QByteArray(1, 0));
	infoPro.insert(InfoExchangeChargeCmd, QByteArray( (char *)&chargeCmd, sizeof(stChargeCmd)));
	this->protocol->sendProtocolData(infoPro, InfoAddrExchange);
} 
/**
 *	传入的参数为上一个界面的请求结果
 */
void ChargeTypeSelect::switchToChargeTypeSelect(QVariant var)
{
    ui->timeLimit->startDownCout(30);
    //ui->lineEditEnergyValue->setText(QString(""));
	stApplayChargeResult result = var.value<stApplayChargeResult>();
	this->canAddr = result.canAddr;
    //qDebug() << "ChargeTypeSelect left money is" << result.leftMoney;
	//显示余额、卡号等信息
	if(result.leftMoney >= 0)
	{
		float tmp = ((float )result.leftMoney)/100;
        ui->labelBalances->setText((QString::number(tmp, 'f', 2))+QString(QObject::tr("元")));;
	}
	else
	{
		ui->labelBalances->setText(QString(""));
	}

	ui->timeLimit->startDownCout(30);
    //qDebug() << "chargeSelect......switch charge type............";
}
void ChargeTypeSelect::receiveFromBus(InfoMap Map, InfoAddrType type)
{
	//emit sigFromBus(Map, type);
}
#if 0
void ChargeTypeSelect::slotBusToOwn(InfoMap Map, InfoAddrType Type)
{

}
#endif
/**
 *点击自动充满
 */
void ChargeTypeSelect::on_buttonAutoCharge_clicked()
{
    //qDebug() << "send start charge cmd..........";
	sendToCSCU(1, 0);

	QVariant var;
    var.setValue(QObject::tr("启动充电中，请等待。。。"));
    emit sigBackToMain(PAGE_CHARGEMANAGE_SELECTCHARGE, PAGE_CHARGEMANAGE_FLASHWAIT, var);
    ui->timeLimit->stopDownCout();
}
#if 0
/**
 *点击按电量充值
 */
void ChargeTypeSelect::on_buttonEnengyCharge_clicked()
{
	unsigned short energy=0;
	bool ok=false;
	energy = (ui->lineEditEnergyValue->text().toFloat(&ok)) * 100;
	if(ok == false)
	{
		StatusRemindWindow  *statusDialog= new StatusRemindWindow(this, 1, 5, QObject::tr("电量格式输入不对，请重新输入"));
		statusDialog->exec();
		delete statusDialog;
		return;
	}

    sendToCSCU(2, energy);

	QVariant var;
    var.setValue(QObject::tr("启动充电中，请等待。。。"));
	emit sigBackToMain(PAGE_CHARGEMANAGE_SELECTCHARGE, PAGE_CHARGEMANAGE_FLASHWAIT, var);
	ui->timeLimit->stopDownCout();
}
#endif
void ChargeTypeSelect::timeoutchange()
{
    //qDebug() << "ChargeTypoeSelsect..................";
    QVariant var;
    emit sigBackToMain(PAGE_CHARGEMANAGE_SELECTCHARGE, PAGE_CHARGEMANAGE_MAIN, var);
    ui->timeLimit->stopDownCout();
}
void ChargeTypeSelect::on_buttonBack_clicked()
{
    QVariant var;
    emit sigBackToMain(PAGE_CHARGEMANAGE_SELECTCHARGE, PAGE_CHARGEMANAGE_MAIN, var);
    ui->timeLimit->stopDownCout();
}
