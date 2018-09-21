#include <QDebug>
#include <QList>
#include <QMovie>

#include "ApplayCharge.h"
#include "ChargeTerm.h"
#include "ui_ApplayCharge.h"
#include "PasswordCharge.h"
#include "StatusRemindWindow.h"

ApplayCharge::ApplayCharge(QWidget *parent, CBus *bus, ProtocolBase *protocol) :
    QWidget(parent),
    ui(new Ui::ApplayCharge)
{
    ui->setupUi(this);

	this->canAddr = 0;
	this->bus = bus;
    this->protocol = protocol;

    QMovie *movie = new QMovie(":/card_action.gif");
   // movie->setSpeed(10);
    ui->labelCard->setMovie(movie);
    movie->start();
    
    QObject::connect(this,SIGNAL(sigFromBus(InfoMap, InfoAddrType)), this, SLOT(slotBusToOwn(InfoMap, InfoAddrType)), Qt::QueuedConnection); 
    QObject::connect(ui->timeLimit, SIGNAL(timeout()), this, SLOT(timeoutchange()));
    timerQuery.setInterval(200);
    QObject::connect(&timerQuery, SIGNAL(timeout()), this, SLOT(timerQueryResult()));
}

ApplayCharge::~ApplayCharge()
{
    this->bus->cancelRegistDev(this);
    delete ui;
}

/**
 * @brief 接收到总线过来的数据
 * @param Map
 * @param type
 */
void ApplayCharge::receiveFromBus(InfoMap Map, InfoAddrType type)
{
	emit sigFromBus(Map, type);
}

void ApplayCharge::slotBusToOwn(InfoMap Map, InfoAddrType Type)
{
    //qDebug() << " ApplayCharge::slotBusToOwn1111111111111111111";
	switch(Type)
	{
		case InfoAddrExchange:
			{
                //qDebug() << " ApplayCharge::slotBusToOwn2222222222211111";
				if(false == Map.contains(InfoExchangeApplayChargeResult))
					break;
				QVariant var = Map.value(InfoExchangeApplayChargeResult);
				stApplayChargeResult data = var.value<stApplayChargeResult>();	//申请读卡结果
				
                //qDebug() << " ApplayCharge::........result=" << data.result;
                if(data.result == 0 || data.result == 1)
				{
					ui->timeLimit->stopDownCout();
					timerQuery.stop();
					this->bus->cancelRegistDev(this);
					
					QVariant varParam;
                    varParam.setValue(QString(QObject::tr("正在获取卡片信息，请稍后。。。")));
					emit sigBackToMain(PAGE_CHARGEMANAGE_STANDBY, PAGE_CHARGEMANAGE_FLASHWAIT, varParam);
				}
			}break;
         case InfoAddrConfig:
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
                     localparam = param;
                     if(localparam.chargeMode == 0)
                         ui->buttonStartCharge->hide();
                     else
                         ui->buttonStartCharge->show();
                 }

             }break;
		default: break;
	}
}
/**
 *超时时间到了
 */
void ApplayCharge::timeoutchange()
{
	ui->timeLimit->stopDownCout();
	timerQuery.stop();
    this->bus->cancelRegistDev(this);
	
	QVariant varParam;
	emit sigBackToMain(PAGE_CHARGEMANAGE_STANDBY, PAGE_CHARGEMANAGE_MAIN, varParam);
    //qDebug() << "ApplayCharge::send stop card info.........2222222222222222222222222";
    InfoProtocol infoPro;                        //add by songqb 增加停止读卡指令 2017-5-23
    stApplayCardCmd applayCmd;

    applayCmd.canAddr = this->canAddr;
    applayCmd.cardcmd = 2;
    applayCmd.cmd = 1;
    applayCmd.status = CHARGE_STATUS_GUN_STANDBY;

    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoExchangeApplayChargeCmd, QByteArray( (char *)&applayCmd, sizeof(stApplayCardCmd)));
    this->protocol->sendProtocolData(infoPro, InfoAddrExchange);
}
/**
 *用户点击了返回按钮
 */
void ApplayCharge::on_buttonBack_clicked()
{
    ui->timeLimit->stopDownCout();
	timerQuery.stop();
    this->bus->cancelRegistDev(this);
	
	QVariant var;
	emit sigBackToMain(PAGE_CHARGEMANAGE_STANDBY, PAGE_CHARGEMANAGE_MAIN, var);
    //qDebug() << "ApplayCharge::send stop card info.........11111111111111111111";

    InfoProtocol infoPro;                        //add by songqb 增加停止读卡指令 2017-5-23
    stApplayCardCmd applayCmd;

    applayCmd.canAddr = this->canAddr;
    applayCmd.cardcmd = 2;
    applayCmd.cmd = 1;
    applayCmd.status = CHARGE_STATUS_GUN_STANDBY;

    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoExchangeApplayChargeCmd, QByteArray( (char *)&applayCmd, sizeof(stApplayCardCmd)));
    this->protocol->sendProtocolData(infoPro, InfoAddrExchange);
}
/**
 *向cscu下发命令
 */
void ApplayCharge::sendToCSCU(InfoBodyType type)
{
	InfoProtocol infoPro;
	stApplayCmd applayCmd;

	applayCmd.canAddr = this->canAddr;
	applayCmd.cmd = 1;
	applayCmd.status = CHARGE_STATUS_GUN_STANDBY; 

	infoPro.insert(InfoDataType, QByteArray(1, 0));
	infoPro.insert(type, QByteArray( (char *)&applayCmd, sizeof(stApplayCmd)));
	this->protocol->sendProtocolData(infoPro, InfoAddrExchange);
}  
/**
 *切换到扫码界面
 */
void ApplayCharge::switchToApplayCharge(unsigned char canAddr)
{

    //qDebug() << "now the chargeMode 1111111111111111111" << localparam.chargeMode;
    queryConfigSpecialFeatureSet();
	/*注册数据*/
    QList<InfoAddrType> list;
    list.append(InfoAddrExchange);
    list.append(InfoAddrConfig);
    this->bus->registDev(this, list);

	this->canAddr = canAddr;
	
	QString qrPath("/tmp/");
	qrPath.append(QString::number(canAddr, 10));
	qrPath.append(".png");
	
	ui->labelQrcode->setPixmap(QPixmap(qrPath));
	ui->labelQrcode->show();

    ui->timeLimit->startDownCout(30);
    //qDebug() << "ApplayCharge::send need read card info.........";

    InfoProtocol infoPro;           //发送读卡信息指令
    stApplayCardCmd applayCmd;

    applayCmd.canAddr = this->canAddr;
    applayCmd.cardcmd = 1;
    applayCmd.cmd = 1;
    applayCmd.status = CHARGE_STATUS_GUN_STANDBY;

    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoExchangeApplayChargeCmd, QByteArray( (char *)&applayCmd, sizeof(stApplayCardCmd)));
    this->protocol->sendProtocolData(infoPro, InfoAddrExchange);
    timerQuery.start();


//    infoPro.insert(InfoDataType, QByteArray(1, 0));
//    //infoPro.insert(InfoConfigChangeChargePassword, QByteArray());
//    infoPro.insert(InfoConfigPassword, QByteArray());
//    this->protocol->sendProtocolData(infoPro, InfoAddrConfig);

}
/**
 *定时发送识别到卡号指令
 */
void ApplayCharge::timerQueryResult()
{
    //qDebug() << "ApplayCharge::send applay card info.........";
	sendToCSCU(InfoExchangeApplayChargeResult);
}

void ApplayCharge::queryConfigSpecialFeatureSet()    //重新查询特殊功能设置 add by songqb  2017-6-14
{
    InfoProtocol infoPro;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoConfigSpecialFunc, QByteArray());
    this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
}

void ApplayCharge::on_buttonStartCharge_clicked()   //本地充电　　add by songqb  2017-6-16
{
    if(localparam.localType == 0)  //本地密码充电
    {
        password = 0;
        //qDebug() << "ApplayCharge::send passwprd //////////////////////*******************.........";
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
            //qDebug() << "The charge passwperd  is .........//////"  << QString::number(password, 10);
            if(((QString::number(password, 10) == inputPassword) && ( inputPassword.length() == 6 )))      //密码正确
            {
                ui->timeLimit->stopDownCout();
                timerQuery.stop();
                this->bus->cancelRegistDev(this);

                QVariant var;
                emit sigBackToMain(PAGE_CHARGEMANAGE_STANDBY, PAGE_CHARGEMANAGE_MAIN, var);

                InfoProtocol infoPro;
                stLocalApplayCharge applayCmd;

                applayCmd.canAddr = this->canAddr;
                applayCmd.status = CHARGE_STATUS_GUN_STANDBY;
                applayCmd.type = 1;
                applayCmd.chargecmd = 1;

                infoPro.insert(InfoDataType, QByteArray(1, 0));
                infoPro.insert(InfoExchangePasswordCharge, QByteArray( (char *)&applayCmd, sizeof(stLocalApplayCharge)));
                this->protocol->sendProtocolData(infoPro, InfoAddrExchange);
                StatusRemindWindow  *statusDialog= new StatusRemindWindow(this, 0, 5, QObject::tr("正在启动充电中。。。"));
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
    if(localparam.localType == 1)  //本地按钮充电
    {

        ui->timeLimit->stopDownCout();
        timerQuery.stop();
        this->bus->cancelRegistDev(this);

        QVariant var;
        emit sigBackToMain(PAGE_CHARGEMANAGE_STANDBY, PAGE_CHARGEMANAGE_MAIN, var);

        InfoProtocol infoPro;
        stLocalApplayCharge applayCmd;

        applayCmd.canAddr = this->canAddr;
        applayCmd.status = CHARGE_STATUS_GUN_STANDBY;
        applayCmd.type = 2;
        applayCmd.chargecmd = 1;

        infoPro.insert(InfoDataType, QByteArray(1, 0));
        infoPro.insert(InfoExchangePasswordCharge, QByteArray( (char *)&applayCmd, sizeof(stLocalApplayCharge)));
        this->protocol->sendProtocolData(infoPro, InfoAddrExchange);
        StatusRemindWindow  *statusDialog= new StatusRemindWindow(this, 0, 5, QObject::tr("正在启动充电中。。。"));
        statusDialog->exec();
        delete statusDialog;
    }
}
