#include <stdlib.h>
#include <QDebug>
#include "ChargeTerm.h"
#include "InfoData.h"
#include "TimeLimit.h"
#include "FlashExportLog.h"
#include "ui_FlashExportLog.h"
#include "StatusRemindWindow.h"

FlashExportLog::FlashExportLog(QWidget *parent, CBus *bus, ProtocolBase *protocol, void *param) :
    QWidget(parent),
    ui(new Ui::FlashExportLog)
{
    ui->setupUi(this);

    this->teuiParam = (stTeuiParam *)param;
	this->bus = bus;
    this->protocol = protocol;
    this->Type = 0;
    timerCount = 1;
    
	QObject::connect(this,SIGNAL(sigFromBus(InfoMap, InfoAddrType)), this, SLOT(slotBusToOwn(InfoMap, InfoAddrType)), Qt::QueuedConnection);
    QObject::connect(ui->timeLimit, SIGNAL(timeout()), this, SLOT(timeoutchange()));

    flashTimer = new QTimer();
    flashTimer->setInterval(1000);
    QObject::connect(flashTimer,SIGNAL(timeout()), this, SLOT(flashOverTimer()));
}

FlashExportLog::~FlashExportLog()
{
    delete flashTimer;
    delete ui;
}

/**
 *切换到flash等待界面
 */
void FlashExportLog::switchToFlashExport(QString text, int time, int Type )
{
	/*注册数据*/
   // qDebug() <<"switchToFlashExport....................";
    QList<InfoAddrType> list;
    list.append(InfoAddrExchange);
    this->bus->registDev(this, list);
    this->Type = Type;
  
    if(this->Type == 1)
    {
        ui->labelExportDate->setText(QObject::tr("版本升级中，请稍后"));
    }
    else if(this->Type == 2)
    {
        ui->labelExportDate->setText(QObject::tr("日志导出中，请稍后"));
    }
    ui->timeLimit->startDownCout(time);
    flashTimer->start();
}
void FlashExportLog::receiveFromBus(InfoMap Map, InfoAddrType type)
{
    emit sigFromBus(Map, type);
}
void FlashExportLog::slotBusToOwn(InfoMap Map, InfoAddrType Type)
{
    //qDebug() << "FlashExportLog::slotBusToOwn(InfoMap Map, InfoAddrType Type......1";
    if(Type == InfoAddrExchange)
	{  
        //qDebug() << "FlashExportLog::slotBusToOwn(InfoMap Map, InfoAddrType Type......2";
		if(false == Map.contains(InfoExchangeUpdateExportResult))
			return;
 //qDebug() << "FlashExportLog::slotBusToOwn(InfoMap Map, InfoAddrType Type.......3";
		QVariant var = Map.value(InfoExchangeUpdateExportResult);
		stUpdateExportResult data = var.value<stUpdateExportResult>();
        //qDebug() << "receive  InfoExchangeUpdateExportResult  result...." << data.cmdType << "   "<< data.result;
		if( data.cmdType == 0 )
			return ;
		
		/*取消结果查询*/
		this->bus->cancelRegistDev(this);
		ui->timeLimit->stopDownCout();
		flashTimer->stop();

		QString Disp;
		int type;

		if(data.cmdType == 2)			//日志导出
			Disp.append(QObject::tr("日志导出"));		
		else if(data.cmdType == 3)	   //数据库导出
			Disp.append(QObject::tr("数据库导出"));		
        /*else if(data.cmdType == 1)	  //升级
            Disp.append(QObject::tr("升级"));*/

		if(data.result == 1)
		{
			type = 0;
            if(data.cmdType == 1)
            {
                Disp.append(QObject::tr("系统正在重启,请稍后!"));
                this->teuiParam->showTermWin = false;
            }
            else
                Disp.append(QObject::tr("成功"));
		}
        else if(data.result == 2)
        {
            type = 1;
            if(data.cmdType == 1)
            {
                Disp.append(QObject::tr("未找到升级程序!"));
            }
            else if(data.cmdType == 2)
            {
                Disp.append(QObject::tr("失败!"));
            }

        }
        else if(data.result == 3)
        {
            type = 1;
            Disp.append(QObject::tr("升级文件损坏!"));
        }
        else
		{
			type = 1;
			Disp.append(QObject::tr("失败！"));
		}

        StatusRemindWindow  *statusDialog = new StatusRemindWindow(this, type , 10, Disp);
		statusDialog->exec();
		delete statusDialog;
		emit sigClickLogo();
	}
}

/**
 *发送查询结果
 */
void FlashExportLog::sendToCSCU()
{
	QVariant var;
	unsigned char cmdType = this->Type;
    InfoProtocol infoPro;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoExchangeUpdateExportResult, QByteArray((char *)&cmdType, 1));
    this->protocol->sendProtocolData(infoPro, InfoAddrExchange);
}
/**
 *数据导出动画
 */
void FlashExportLog::flashOverTimer()
{
    timerCount++;
    if(timerCount > 4)
        timerCount = 1;
    //ui->labellogexport1->show();
    ui->stackedWidget->setCurrentIndex(timerCount - 1);

    sendToCSCU();
}
/**
 *超时
 */
void FlashExportLog::timeoutchange()
{
    this->bus->cancelRegistDev(this);
    ui->timeLimit->stopDownCout();
    flashTimer->stop();
	
	emit sigClickLogo();
}











