#include "ExportData.h"
#include "ui_ExportData.h"
#include "StatusRemindWindow.h"
#include "TimeLimit.h"


ExportData::ExportData(QWidget *parent, CBus *bus, ProtocolBase *protocol, void *param) :
    QWidget(parent),
    ui(new Ui::ExportData)
{
    ui->setupUi(this);

	ui->pushButtonExportDataBase->hide();

    this->teuiParam = (stTeuiParam *)param;
    this->bus = bus;
    this->protocol = protocol;
#if 0
    /*注册数据*/
    QList<InfoAddrType> list;
    list.append(InfoAddrReal);
    list.append(InfoAddrConfig);
    this->bus->registDev(this, list);
#endif
    QObject::connect(this,SIGNAL(sigFromBus(InfoMap, InfoAddrType)), this, SLOT(slotBusToOwn(InfoMap, InfoAddrType)), Qt::QueuedConnection);
    QObject::connect(ui->timeLimit, SIGNAL(timeout()), this, SLOT(timeoutchange()));
}

ExportData::~ExportData()
{
    //this->bus->cancelRegistDev(this);
    delete ui;
}


void ExportData::receiveFromBus(InfoMap Map, InfoAddrType Type)
{
//    emit sigFromBus(Map, Type);
}

void ExportData::slotBusToOwn(InfoMap Map, InfoAddrType Type)
{
	
}

/**
 *
 */
void ExportData::switchMainExportUpdate()
{
    ui->timeLimit->startDownCout(60);
}
/**
 *超时界面
 */
void ExportData::timeoutchange()
{
    ui->timeLimit->stopDownCout();
	emit sigClickLogo();
}
/**
 *点击返回按钮
 */
void ExportData::on_pushButtonReturn_clicked()
{
	ui->timeLimit->stopDownCout();
	emit sigClickLogo();
}
/**
 *点击升级按钮
 */
void ExportData::on_pushButtonUpgradeVersion_clicked()
{
    ui->timeLimit->stopDownCout();
	QVariant var;
	unsigned char cmdType = 1;
    InfoProtocol infoPro;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoExchangeUpdateExportCmd, QByteArray((char *)&cmdType, 1));
    this->protocol->sendProtocolData(infoPro, InfoAddrExchange);
   
    /*
	StatusRemindWindow  *statusDialog = new StatusRemindWindow(this, 0 , 10, QObject::tr("系统正在重启升级"));
    teuiParam->showTermWin = false;
	statusDialog->exec();
    delete statusDialog;*/

	var.setValue(cmdType);
    //emit sigBackToMain(PAGE_UPDATEEXPORT_MAIN, PAGE_CHARGEMANAGE_MAIN, var);
    emit sigBackToMain(PAGE_UPDATEEXPORT_MAIN, PAGE_UPDATEEXPORT_FLASH, var);
}
/**
 *点击导出日志
 */
void ExportData::on_pushButtonExportLog_clicked()
{
    ui->timeLimit->stopDownCout();
    QVariant var;
	unsigned char cmdType = 2;
    InfoProtocol infoPro;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoExchangeUpdateExportCmd, QByteArray((char *)&cmdType, 1));
    this->protocol->sendProtocolData(infoPro, InfoAddrExchange);
    
	var.setValue(cmdType);
    emit sigBackToMain(PAGE_UPDATEEXPORT_MAIN, PAGE_UPDATEEXPORT_FLASH, var);
}

/**
 *点击数据库导出按钮
 */
void ExportData::on_pushButtonExportDataBase_clicked()
{	
    ui->timeLimit->stopDownCout();
    QVariant var;
	unsigned char cmdType = 3;
    InfoProtocol infoPro;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoExchangeUpdateExportCmd, QByteArray((char *)&cmdType, 1));
    this->protocol->sendProtocolData(infoPro, InfoAddrExchange);
   
	var.setValue(cmdType);
    emit sigBackToMain(PAGE_UPDATEEXPORT_MAIN, PAGE_UPDATEEXPORT_FLASH, var);
}
