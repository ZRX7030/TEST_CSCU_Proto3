#include <stdlib.h>
#include <QDebug>

#include "ChargeTerm.h"
#include "InfoData.h"
#include "FlashWaitWidget.h"
#include "ui_FlashWaitWidget.h"
#include "TimeLimit.h"

FlashWaitWidget::FlashWaitWidget(QWidget *parent, CBus *bus, ProtocolBase *protocol) :
    QWidget(parent),
    ui(new Ui::FlashWaitWidget)
{
    ui->setupUi(this);
    this->bus = bus;
    this->protocol = protocol;
	this->startStopFlag = 0;
     this->chargeType = 0;
	this->Type = InfoBodyNone;
    timerCount = 1;
  
	QObject::connect(this,SIGNAL(sigFromBus(InfoMap, InfoAddrType)), this, SLOT(slotBusToOwn(InfoMap, InfoAddrType)),Qt::QueuedConnection);
    QObject::connect(ui->timeLimit, SIGNAL(timeout()), this, SLOT(timeoutchange()));
//	QObject::connect(this,SIGNAL(sigBackToMainIntel(int, int, QVariant)), this, SLOT(slotBackToMainIntel(int, int, QVariant)));
    
	flashTimer = new QTimer();
    flashTimer->setInterval(500);
    QObject::connect(flashTimer,SIGNAL(timeout()), this, SLOT(flashOverTimer()));
}

FlashWaitWidget::~FlashWaitWidget()
{
    this->bus->cancelRegistDev(this);
	delete flashTimer;
    delete ui;
}

void FlashWaitWidget::slotBackToMainIntel(int oldStatus, int newStatus, QVariant var)
{
	//emit sigBackToMain(oldStatus, newStatus, var);
}
/**
 *切换到flash等待界面
 */
void FlashWaitWidget::switchToFlashWait(unsigned char canAddr,unsigned char chargeType, QString text, int time, int flag, InfoBodyType Type )
{
    timerCount = 1;
	
	/*注册数据*/
    QList<InfoAddrType> list;
    list.append(InfoAddrExchange);
    this->bus->registDev(this, list);
  
	this->canAddr = canAddr;
    this->chargeType = chargeType;
	this->Type = Type;				//指令下发数据标示
	startStopFlag = flag;			//充电、结束充电标示

	ui->labelStartOrEndCharging->setText(text);
	ui->timeLimit->startDownCout(time);
    flashTimer->start();
}

void FlashWaitWidget::receiveFromBus(InfoMap Map, InfoAddrType type)
{
    emit sigFromBus(Map, type);
}

void FlashWaitWidget::slotBusToOwn(InfoMap Map, InfoAddrType Type)
{
    if(Type == InfoAddrExchange)
    {
        if(true == Map.contains(InfoExchangeApplayChargeResult))    //申请卡片信息指令应答
		{
			QVariant var = Map.value(InfoExchangeApplayChargeResult);
			stApplayChargeResult data = var.value<stApplayChargeResult>();

			QVariant varParam;
            //qDebug() << "FlashWaitWidgert::datatesult="  << data.result;
			if(data.result == 1)  //卡片信息申请成功
			{
				if(data.cmd == 1)
				{
                    //qDebug() << "flashwait left money is" <<data.leftMoney;
					ui->timeLimit->stopDownCout();
					flashTimer->stop();
					this->bus->cancelRegistDev(this);
                    //qDebug()<<"...................../.................."<<chargeType;
					varParam.setValue(data);
                    if(chargeType == 1)       //应急模式下刷卡充电
                    {
                        QVariant varParam;
                        QString str;
                        str = QString(QObject::tr("开始充电申请成功!"));

                        char cmdBuff[100];
                        snprintf(cmdBuff, sizeof(cmdBuff), "aplay %s &", START_CHARGE_SUCCESS);
                        system(cmdBuff);

                        varParam.setValue(str);
                        emit sigBackToMain(PAGE_CHARGEMANAGE_FLASHWAIT, PAGE_CHARGEMANAGE_BASE, varParam);
                    }
                    else
                    {
                        emit sigBackToMain(PAGE_CHARGEMANAGE_FLASHWAIT, PAGE_CHARGEMANAGE_SELECTCHARGE, varParam);
                    }
				}
			}
			else if(data.result == 2 ) //申请充电时卡片信息申请失败
			{
				ui->timeLimit->stopDownCout();
				flashTimer->stop();
				this->bus->cancelRegistDev(this);
				
                //qDebug() << "cancel register...............2";
				varParam.setValue(QString(QObject::tr(data.failReson)));
				emit sigBackToMain(PAGE_CHARGEMANAGE_FLASHWAIT, PAGE_CHARGEMANAGE_BASE, varParam);
			}
		}
		else if(true == Map.contains(InfoExchangeChargeResult))  //启动充电结束充电指令应答
		{
            //qDebug() <<"receive statr respond.......";
			QVariant var = Map.value(InfoExchangeChargeResult);
			stChargeResult data = var.value<stChargeResult>();	//申请充电结果

			QVariant varParam;
			QString str;
			if(data.result == 1)  //申请成功
			{
				if(data.cmd == 1) 
				{
					str = QString(QObject::tr("开始充电申请成功。。。。"));
					
					char cmdBuff[100];
					snprintf(cmdBuff, sizeof(cmdBuff), "aplay %s &", START_CHARGE_SUCCESS);
					system(cmdBuff);
				}
				else
				{
					str = QString(QObject::tr("结束充电申请成功。。。。"));
				
					char cmdBuff[100];
                    snprintf(cmdBuff, sizeof(cmdBuff), "aplay %s &", STOP_CHARGE_SUCCESS	);
					system(cmdBuff);
				}
				
				ui->timeLimit->stopDownCout();
				flashTimer->stop();
                //qDebug() << "cancel register...............1";
				this->bus->cancelRegistDev(this);
				
				varParam.setValue(str);
				emit sigBackToMain(PAGE_CHARGEMANAGE_FLASHWAIT, PAGE_CHARGEMANAGE_BASE, varParam);
			}
			else if(data.result == 2) //申请失败
			{
				str = QString(QObject::tr(data.failReson));
				
				ui->timeLimit->stopDownCout();
				flashTimer->stop();
				this->bus->cancelRegistDev(this);
				
				varParam.setValue(str);
				emit sigBackToMain(PAGE_CHARGEMANAGE_FLASHWAIT, PAGE_CHARGEMANAGE_BASE, varParam);
			}
		}
        else if(true == Map.contains(InfoExchangeButtonStopCharge))      //按钮结束充电
        {
            QVariant var = Map.value(InfoExchangeButtonStopCharge);
            stButtonStopResult data = var.value<stButtonStopResult>();	//申请按钮结束充电结果
            QVariant varParam;
            QString str;
            if(data.result == 1)
            {
                str = QString(QObject::tr("结束充电申请成功!"));

                char cmdBuff[100];
                snprintf(cmdBuff, sizeof(cmdBuff), "aplay %s &", STOP_CHARGE_SUCCESS	);
                system(cmdBuff);

                ui->timeLimit->stopDownCout();
                flashTimer->stop();
                this->bus->cancelRegistDev(this);

                varParam.setValue(str);
                emit sigBackToMain(PAGE_CHARGEMANAGE_FLASHWAIT, PAGE_CHARGEMANAGE_BASE, varParam);
            }
        }
        else if(true == Map.contains(InfoExchangePrintPaper))
        {
            QVariant var = Map.value(InfoExchangePrintPaper);
            stPrintPaperResult data = var.value<stPrintPaperResult>();

            QVariant varParam;
            QString str;
            if(data.result == 1)
            {
                str = QString(QObject::tr("小票打印成功!"));

            }
            else if(data.result == 0)
            {
                str = QString(QObject::tr(data.failReson));
            }
            varParam.setValue(str);
            ui->timeLimit->stopDownCout();
            flashTimer->stop();
            this->bus->cancelRegistDev(this);
            emit sigBackToMain(PAGE_CHARGEMANAGE_FLASHWAIT, PAGE_CHARGEMANAGE_BASE, varParam);
        }
	}
}
/**
 *向cscu下发命令
 */
void FlashWaitWidget::sendToCSCU()
{
	InfoProtocol infoPro;
	stApplayCmd applayCmd;

	applayCmd.canAddr = this->canAddr;
	applayCmd.cmd = this->startStopFlag;
	if(startStopFlag == 1)
		applayCmd.status = CHARGE_STATUS_GUN_STANDBY; 
	else if(startStopFlag == 2)
		applayCmd.status = CHARGE_STATUS_CHARGING; 

	infoPro.insert(InfoDataType, QByteArray(1, 0));
	infoPro.insert(this->Type, QByteArray( (char *)&applayCmd, sizeof(stApplayCmd)));
	this->protocol->sendProtocolData(infoPro, InfoAddrExchange);
} 
/**
 *动画定时器
 */
void FlashWaitWidget::flashOverTimer()
{
	timerCount++;
	if(timerCount > 16)
		timerCount = 1;
	ui->stackedWidget->setCurrentIndex(timerCount - 1);
//	qDebug() << "falsh  timer send to cscu............" << timerCount;

	if(startStopFlag)
	{
		sendToCSCU();
	}
}
/**
 *界面超时
 */
void FlashWaitWidget::timeoutchange()
{
	//qDebug() << "FlashWaitWidget   timeoutchange....................]]]]]]]]]]]]]]]]]";

	ui->timeLimit->stopDownCout();
    flashTimer->stop();
	this->bus->cancelRegistDev(this);
    
	QVariant var;
    emit sigBackToMain(PAGE_CHARGEMANAGE_FLASHWAIT, PAGE_CHARGEMANAGE_MAIN, var);
}
