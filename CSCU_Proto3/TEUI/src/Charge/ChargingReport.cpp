#include <QDebug>
#include <QDateTime>
#include "ChargingReport.h"
#include "InfoData.h"
#include "ui_ChargingReport.h"
#include "ChargeTerm.h"
#include "Common.h"
#include "PasswordCharge.h"
#include "StatusRemindWindow.h"

ChargingReport::ChargingReport(QWidget *parent, CBus *bus, ProtocolBase *protocol) :
    QWidget(parent),
    ui(new Ui::ChargingReport)
{
    ui->setupUi(this);

    this->bus = bus;
    this->protocol = protocol;
	this->canAddr = 0;

    QObject::connect(this,SIGNAL(sigFromBus(InfoMap, InfoAddrType)), this, SLOT(slotBusToOwn(InfoMap, InfoAddrType)), Qt::QueuedConnection);

	timerQuery = new QTimer();
    timerQuery->setInterval(1000);
    QObject::connect(timerQuery, SIGNAL(timeout()), this, SLOT(slotTimerQueryCharging()));

    QObject::connect(ui->timeLimit, SIGNAL(timeout()), this, SLOT(timeoutchange()));

	ui->buttonStopCharging->hide();
}

ChargingReport::~ChargingReport()
{
    this->bus->cancelRegistDev(this);
    delete ui;
}
/**
 *切换到充电中
 */
void ChargingReport::switchToCharging(unsigned char canAddr)
{
    queryChargeSpecialFeatureSet();

    /*注册数据*/
    QList<InfoAddrType> list;
    list.append(InfoAddrReal);
    list.append(InfoAddrExchange);
    list.append(InfoAddrConfig);
    this->bus->registDev(this, list);

    ui->timeLimit->startDownCout(30);
    this->canAddr = canAddr;
    timerQuery->start();
    clearChargeReport();   //清空充电报告
	queryChargingReport(this->canAddr);
    //qDebug() << "send card **********************************" << canAddr;
	InfoProtocol infoPro;
    stApplayCardCmd applayCmd;

	applayCmd.canAddr = this->canAddr;
    applayCmd.cardcmd =1;
	applayCmd.cmd = 2;
	applayCmd.status = CHARGE_STATUS_CHARGING; 

	infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoExchangeApplayChargeCmd, QByteArray( (char *)&applayCmd, sizeof(stApplayCardCmd)));
	this->protocol->sendProtocolData(infoPro, InfoAddrExchange);

    //qDebug() << "queryCharginghReport canAddr=////////////////////////////////" << canAddr;
    infoPro.clear();
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoRealCharge, QByteArray((char *)&canAddr, 1));
    this->protocol->sendProtocolData(infoPro, InfoAddrReal);

}

void ChargingReport::receiveFromBus(InfoMap Map, InfoAddrType type)
{
     emit sigFromBus(Map, type);
}
/**
 *下发读取卡信息指令
 */
void ChargingReport::queryCardInfo()
{
    //qDebug() << "send card //////////////////////////" << canAddr;
	InfoProtocol infoPro;
	stApplayCmd applayCmd;

	applayCmd.canAddr = this->canAddr;
	applayCmd.cmd = 2;
	applayCmd.status = CHARGE_STATUS_CHARGING; 

	infoPro.insert(InfoDataType, QByteArray(1, 0));
	infoPro.insert(InfoExchangeApplayChargeResult, QByteArray( (char *)&applayCmd, sizeof(stApplayCmd)));
	this->protocol->sendProtocolData(infoPro, InfoAddrExchange);
}
/**
 *下发停止充电指令
 */

void ChargingReport::sendStopChargeCmd()
{
	InfoProtocol infoPro;
	stChargeCmd chargeCmd;

	memset(&chargeCmd , 0, sizeof(stChargeCmd));
	chargeCmd.canAddr = this->canAddr;
	chargeCmd.cmd = 2;
	chargeCmd.status = CHARGE_STATUS_CHARGING; 
	chargeCmd.chargeEnergy = 0;
	chargeCmd.chargeType = 0;

	infoPro.insert(InfoDataType, QByteArray(1, 0));
	infoPro.insert(InfoExchangeChargeCmd, QByteArray( (char *)&chargeCmd, sizeof(stChargeCmd)));
	this->protocol->sendProtocolData(infoPro, InfoAddrExchange);
} 
/**
 *读取充电报告指令
 */
void ChargingReport::queryChargingReport(unsigned char canAddr)
{
    //qDebug() << "queryCharginghReport canAddr=" << canAddr;
    InfoProtocol infoPro;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoRealCharge, QByteArray((char *)&canAddr, 1));
    this->protocol->sendProtocolData(infoPro, InfoAddrReal);
}
void ChargingReport::clearChargeReport()    //清空充电报告 add by songqb 2017-6-6
{
    ui->labelVoltage->setText("");
    //ui->labelElectricity->setText("");
    ui->labelSumEnergy->setText("");
    ui->labelChargedTime->setText("");
    ui->labelDcVoltage->setText("");
    ui->labelPower->setText("");
    ui->labelChargingEnergy->setText("");
    ui->labelStartTime->setText("");
    ui->labelDcElectricity->setText("");
}
void ChargingReport::slotBusToOwn(InfoMap Map, InfoAddrType Type)
{
    if(Type == InfoAddrReal)
    {
       // qDebug() << "receive charging report....................";
        if(false == Map.contains(InfoRealCharge))
            return;

		QVariant var = Map.value(InfoRealCharge);
		stChargeReal data = var.value<stChargeReal>();
		short voltageSum = data.voltageA + data.voltageB + data.voltageC;
		short currentSum = data.currentA + data.currentB + data.currentC;

		//qDebug() << "current................./..............." << data.currentA <<data.currentB << data.currentC;
		/*输入测电压、电流*/
		if(data.canAddr > 0  && data.canAddr < 151 )
		{
			ui->labelVoltage->setText(QString::number((double)voltageSum/10, 'f',  1));
            //ui->labelElectricity->setText(QString::number((double)(qAbs(currentSum))/100, 'f',  2));
		}
		else
        {
			ui->labelVoltage->setText(QString::number((double)data.voltageA/10, 'f',  1)+"/"+QString::number((double)data.voltageB/10, 'f',  1)+"/" +QString::number((double)data.voltageC/10, 'f',  1));
           // if(currentSum == 0)
           //     ui->labelElectricity->setText(QString("--")+"/"+QString("--")+"/"+QString("--"));
           // else
           //     ui->labelElectricity->setText(QString::number((double)(qAbs(data.currentA))/100, 'f',  2)+"/" +QString::number((double)(qAbs(data.currentB))/100, 'f',  2)+"/" +QString::number((double)(qAbs(data.currentC))/100, 'f',  2));
        }

        ui->labelPower->setText(QString::number((double)data.power/10, 'f',  1));
		ui->labelSumEnergy->setText(QString::number((double)data.currentEnergy/100, 'f',  2));
		ui->labelChargingEnergy->setText(QString::number((double)data.chargeEnergy/100, 'f',  2));
        //ui->labelStartTime->setText(QString(data.startTime));
        QDateTime time = QDateTime::fromString(QString(data.startTime),"yyyy-MM-dd hh:mm:ss");
        ui->labelStartTime->setText(QString(time.toString("hh:mm:ss")));
		ui->labelChargedTime->setText(QString::number(data.chargeTime, 10));

		/*输出测电压、电流*/
		if(data.canAddr > 180)
		{
			/*soc*/
			ui->progressBarSoc->setValue(data.currentSoc);
			ui->progressBarSoc->setTextVisible(true);

			/*直流侧电压、电流*/
			ui->labelDcVoltage->setText(QString::number((double)data.DCvoltage/10, 'f',  1));
            ui->labelDcElectricity->setText(QString::number((double)(qAbs(data.DCcurrent))/10, 'f',  1));
		}
		else if(data.canAddr > 0  && data.canAddr < 151 )
		{
			ui->labelDcVoltage->setText(QString::number((double)voltageSum/10, 'f',  1));
            ui->labelDcElectricity->setText(QString::number((double)(qAbs(currentSum))/10, 'f',  1));

			ui->progressBarSoc->setValue(0);
			ui->progressBarSoc->setTextVisible(false);
		}
		else
		{
			ui->labelDcVoltage->setText(QString::number((double)data.voltageA/10, 'f',  1)+"/"+QString::number((double)data.voltageB/10, 'f',  1)+"/" +QString::number((double)data.voltageC/10, 'f',  1));
            ui->labelDcElectricity->setText(QString::number((double)(qAbs(data.currentA))/10, 'f',  1)+"/" +QString::number((double)(qAbs(data.currentB))/10, 'f',  1)+"/" +QString::number((double)(qAbs(data.currentC))/10, 'f',  1));

			ui->progressBarSoc->setValue(0);
			ui->progressBarSoc->setTextVisible(false);
		}
	}

	else if(Type == InfoAddrExchange)
	{
		//qDebug() << "chargeing  reporti receive card info::slotBusToOwn//////////////////////";
		if(false == Map.contains(InfoExchangeApplayChargeResult))
			return;
		QVariant var = Map.value(InfoExchangeApplayChargeResult);

		stApplayChargeResult data = var.value<stApplayChargeResult>();	//申请读卡结果
        //qDebug() << "now the result is -------------------------" << data.result;
		if(data.result == 0)
		{
			QVariant varParam;
			varParam.setValue(QObject::tr("结束充电中，请等待。。。"));

			this->bus->cancelRegistDev(this);
			emit sigBackToMain(PAGE_CHARGEMANAGE_CHARGING, PAGE_CHARGEMANAGE_FLASHWAIT, varParam);
			ui->timeLimit->stopDownCout();
			timerQuery->stop();

			sendStopChargeCmd();
		}
	}
    else if(Type == InfoAddrConfig)
    {
        if(Map.contains(InfoConfigChangeChargePassword))
           {
               //qDebug() << " recevive password ..................................../.....................";
               QVariant var = Map.value(InfoConfigChangeChargePassword);
               password  = var.value<int>();
           }
        else if(Map.contains(InfoConfigSpecialFunc))
        {
            QVariant var = Map.value(InfoConfigSpecialFunc);
            stSpecialFunc param = var.value<stSpecialFunc>();
            par = param;
            if(par.chargeMode == 0 && par.localStop == 0)
                ui->buttonStopCharging->hide();
            else
                ui->buttonStopCharging->show();
        }
    }
}
/**
 *充电中定时数据查询
 */
void ChargingReport::slotTimerQueryCharging()
{
	queryChargingReport(this->canAddr);
	queryCardInfo();
}

/**
 *界面超时
 */
void ChargingReport::timeoutchange()
{
	QVariant var;
	ui->timeLimit->stopDownCout();
    timerQuery->stop();
	this->bus->cancelRegistDev(this);
	emit sigBackToMain(PAGE_CHARGEMANAGE_CHARGING, PAGE_CHARGEMANAGE_MAIN, var);

    //Debug() << "send card //////////////////////////2222222222222" << canAddr;
	InfoProtocol infoPro;               //add by songqb 增加停止读卡指令 2017-5-23
	stApplayCardCmd applayCmd;
	applayCmd.canAddr = this->canAddr;
	applayCmd.cardcmd =2;
	applayCmd.cmd = 2;
	applayCmd.status = CHARGE_STATUS_CHARGING;

	infoPro.insert(InfoDataType, QByteArray(1, 0));
	infoPro.insert(InfoExchangeApplayChargeCmd, QByteArray( (char *)&applayCmd, sizeof(stApplayCardCmd)));
	this->protocol->sendProtocolData(infoPro, InfoAddrExchange);
}
/**
 *点击返回
 */
void ChargingReport::on_buttonSave_clicked()
{
	QVariant var;
	timerQuery->stop();
	ui->timeLimit->stopDownCout();
	this->bus->cancelRegistDev(this);
	emit sigBackToMain(PAGE_CHARGEMANAGE_CHARGING, PAGE_CHARGEMANAGE_MAIN, var);

    //qDebug() << "send card //////////////////////////1111111111111" << canAddr;
	InfoProtocol infoPro;                 //add by songqb 增加停止读卡指令 2017-5-23
	stApplayCardCmd applayCmd;
	applayCmd.canAddr = this->canAddr;
	applayCmd.cardcmd =2;
	applayCmd.cmd = 2;
	applayCmd.status = CHARGE_STATUS_CHARGING;

	infoPro.insert(InfoDataType, QByteArray(1, 0));
	infoPro.insert(InfoExchangeApplayChargeCmd, QByteArray( (char *)&applayCmd, sizeof(stApplayCardCmd)));
	this->protocol->sendProtocolData(infoPro, InfoAddrExchange);
}

void ChargingReport::buttonStopCharge()   //按钮结束充电
{
    InfoProtocol infoPro;
    stChargeCmd chargeCmd;

    memset(&chargeCmd , 0, sizeof(stChargeCmd));
    chargeCmd.canAddr = this->canAddr;
    chargeCmd.cmd = 2;
    chargeCmd.status = CHARGE_STATUS_CHARGING;
    chargeCmd.chargeEnergy = 0;
    chargeCmd.chargeType = 0;

    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoExchangeButtonStopCharge, QByteArray( (char *)&chargeCmd, sizeof(stChargeCmd)));
    this->protocol->sendProtocolData(infoPro, InfoAddrExchange);
}
void ChargingReport::on_buttonStopCharging_clicked()
{
    if(par.chargeMode == 0)
    {
        QVariant varParam;
        varParam.setValue(QObject::tr("结束充电中，请等待。。。"));

        this->bus->cancelRegistDev(this);
        emit sigBackToMain(PAGE_CHARGEMANAGE_CHARGING, PAGE_CHARGEMANAGE_FLASHWAIT, varParam);
        ui->timeLimit->stopDownCout();
        timerQuery->stop();
        buttonStopCharge();
    }

    if(par.chargeMode == 1)
    {
        if(par.localType == 0)  //本地密码充电
        {
            password = 0;
            InfoProtocol infoPro;
            infoPro.insert(InfoDataType, QByteArray(1, 0));
            infoPro.insert(InfoConfigChangeChargePassword, QByteArray());
            this->protocol->sendProtocolData(infoPro, InfoAddrConfig);

            PasswordCharge *chargeDialog = new PasswordCharge(this);
            chargeDialog->show();
            int retValue = chargeDialog->exec();
            if(retValue == 1)
            {
                QString inputPassword = chargeDialog->getInputPassword();
                if(((QString::number(password, 10) == inputPassword) && ( inputPassword.length() == 6 )))      //密码正确
                {
                    QVariant var;
                    timerQuery->stop();
                    ui->timeLimit->stopDownCout();
                    this->bus->cancelRegistDev(this);
                    emit sigBackToMain(PAGE_CHARGEMANAGE_CHARGING, PAGE_CHARGEMANAGE_MAIN, var);

                    InfoProtocol infoPro;
                    stLocalApplayCharge applayCmd;

                    applayCmd.canAddr = this->canAddr;
                    applayCmd.status = CHARGE_STATUS_CHARGING;
                    applayCmd.type = 1;
                    applayCmd.chargecmd = 2;

                    infoPro.insert(InfoDataType, QByteArray(1, 0));
                    infoPro.insert(InfoExchangePasswordCharge, QByteArray( (char *)&applayCmd, sizeof(stLocalApplayCharge)));
                    this->protocol->sendProtocolData(infoPro, InfoAddrExchange);
                    StatusRemindWindow  *statusDialog= new StatusRemindWindow(this, 0, 5, QObject::tr("正在结束充电中。。。"));
                    statusDialog->exec();
                    delete statusDialog;
                }
                else
                {
                    StatusRemindWindow  *statusDialog= new StatusRemindWindow(this, 1, 5, QObject::tr("本地充电密码输入错误"));
                    statusDialog->exec();
                    delete statusDialog;
                }
            }
            delete chargeDialog;
        }
        if(par.localType == 1)  //本地按钮充电
        {
            QVariant var;
            timerQuery->stop();
            ui->timeLimit->stopDownCout();
            this->bus->cancelRegistDev(this);
            emit sigBackToMain(PAGE_CHARGEMANAGE_CHARGING, PAGE_CHARGEMANAGE_MAIN, var);

            InfoProtocol infoPro;
            stLocalApplayCharge applayCmd;

            applayCmd.canAddr = this->canAddr;
            applayCmd.status = CHARGE_STATUS_CHARGING;
            applayCmd.type = 2;
            applayCmd.chargecmd = 2;

            infoPro.insert(InfoDataType, QByteArray(1, 0));
            infoPro.insert(InfoExchangePasswordCharge, QByteArray( (char *)&applayCmd, sizeof(stLocalApplayCharge)));
            this->protocol->sendProtocolData(infoPro, InfoAddrExchange);
            StatusRemindWindow  *statusDialog= new StatusRemindWindow(this, 0, 5, QObject::tr("正在结束充电中。。。"));
            statusDialog->exec();
            delete statusDialog;
        }
    }
}
void ChargingReport::queryChargeSpecialFeatureSet()  //重新查询特殊功能设置 add by songqb  2017-6-14
{
    InfoProtocol infoPro;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoConfigSpecialFunc, QByteArray());
    this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
}
