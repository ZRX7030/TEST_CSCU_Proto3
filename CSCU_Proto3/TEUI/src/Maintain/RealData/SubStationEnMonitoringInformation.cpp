#include <QDebug>

#include "SubStationEnMonitoringInformation.h"
#include "ui_SubStationEnMonitoringInformation.h"
#include "StatusRemindWindow.h"
#include "Common.h"
#include "SwapBase.h"
#include "InfoData.h"

SubStationEnMonitoringInformation::SubStationEnMonitoringInformation(QWidget *parent, CBus *bus, ProtocolBase *protocol) :
    QWidget(parent),
    ui(new Ui::SubStationEnMonitoringInformation)
{
    ui->setupUi(this);
	
	this->bus = bus;
    this->protocol = protocol;

    ui->comboBoxAlertor9->hide();              //隐藏报警器9和10,模块化集控只有8个IO口
    ui->comboBoxAlertor10->hide();
    ui->labelAlertor9->hide();
    ui->labelAlertor10->hide();
    ui->lineEditAlertor9->hide();
    ui->lineEditAlertor10->hide();

    /*注册数据*/
    QList<InfoAddrType> list;
    list.append(InfoAddrConfig);
    list.append(InfoAddrReal);
    list.append(InfoAddrExchange);
    this->bus->registDev(this, list);

    QObject::connect(this,SIGNAL(sigFromBus(InfoMap, InfoAddrType)), this, SLOT(slotBusToOwn(InfoMap, InfoAddrType)), Qt::QueuedConnection);
	
	queryRealStation();
	queryStationConfig();

    timerQuery = new QTimer();           // by songqb 2017-5-24
    timerQuery->setInterval(3000);
    QObject::connect(timerQuery, SIGNAL(timeout()), this, SLOT(timerQuerySubStation()));
    timerQuery->start();
}

SubStationEnMonitoringInformation::~SubStationEnMonitoringInformation()
{
    if(timerQuery)
        delete timerQuery;

    this->bus->cancelRegistDev(this);
    delete ui;
}
void SubStationEnMonitoringInformation::timerQuerySubStation()  // by songqb 2017-5-24
{
    queryRealStation();
}
void SubStationEnMonitoringInformation::receiveFromBus(InfoMap Map, InfoAddrType type)
{
     emit sigFromBus(Map, type);
}
/**
 *查询环境数据
 */
void SubStationEnMonitoringInformation::queryRealStation()
{
    InfoProtocol infoPro;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoRealStation, QByteArray());
    this->protocol->sendProtocolData(infoPro, InfoAddrReal);
}
/**
 *查询配置
 */
void SubStationEnMonitoringInformation::queryStationConfig()
{
    InfoProtocol infoPro;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoConfigIO, QByteArray());
    this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
}

/**
 *收到cscu数据
 */
void SubStationEnMonitoringInformation::slotBusToOwn(InfoMap Map, InfoAddrType Type)
{
    if(Type == InfoAddrConfig)			//io配置数据
    {
        if(false == Map.contains(InfoConfigIO))
            return;

        //qDebug() << "receive config param..........";
        QVariant var = Map.value(InfoConfigIO);
        stIOConfigParam data = var.value<stIOConfigParam>();
		
		ui->comboBoxAlertor1->setCurrentIndex(data.io[0]);
		ui->comboBoxAlertor2->setCurrentIndex(data.io[1]);
		ui->comboBoxAlertor3->setCurrentIndex(data.io[2]);
		ui->comboBoxAlertor4->setCurrentIndex(data.io[3]);
		ui->comboBoxAlertor5->setCurrentIndex(data.io[4]);
		ui->comboBoxAlertor6->setCurrentIndex(data.io[5]);
		ui->comboBoxAlertor7->setCurrentIndex(data.io[6]);
		ui->comboBoxAlertor8->setCurrentIndex(data.io[7]);
		ui->comboBoxAlertor9->setCurrentIndex(data.io[8]);
		ui->comboBoxAlertor10->setCurrentIndex(data.io[9]);
    }
	else if(Type == InfoAddrReal)			//实时环境数据
	{
		if(false == Map.contains(InfoRealStation))
			return;

        //qDebug() << "receive realstation data..........";
		float tmp;
        QVariant var = Map.value(InfoRealStation);
		stStationRealData data = var.value<stStationRealData>();

        tmp = ((float )data.tempeture)/100;
        ui->lineEditSubStationTemperature->setText(QString::number(tmp, 'f', 1)+ QObject::tr(" 度"));
        tmp = ((float )data.humidity)/100;
        ui->lineEditSubStationHumidity->setText(QString::number(tmp, 'f', 1)+" %");

        QString alarmDispAlarm = QString(QObject::tr("告警"));
        QString alarmDispNormal = QString(QObject::tr("正常"));

        if(data.alarm[0] == 0)
			ui->lineEditAlertor1->setText(alarmDispAlarm);
		else
			ui->lineEditAlertor1->setText(alarmDispNormal);
        if(data.alarm[1] == 0)
			ui->lineEditAlertor2->setText(alarmDispAlarm);
		else
			ui->lineEditAlertor2->setText(alarmDispNormal);
        if(data.alarm[2] == 0)
			ui->lineEditAlertor3->setText(alarmDispAlarm);
		else
			ui->lineEditAlertor3->setText(alarmDispNormal);
        if(data.alarm[3] == 0)
			ui->lineEditAlertor4->setText(alarmDispAlarm);
		else
			ui->lineEditAlertor4->setText(alarmDispNormal);
        if(data.alarm[4] == 0)
			ui->lineEditAlertor5->setText(alarmDispAlarm);
		else
			ui->lineEditAlertor5->setText(alarmDispNormal);
        if(data.alarm[5] == 0)
			ui->lineEditAlertor6->setText(alarmDispAlarm);
		else
			ui->lineEditAlertor6->setText(alarmDispNormal);
        if(data.alarm[6] == 0)
			ui->lineEditAlertor7->setText(alarmDispAlarm);
		else
			ui->lineEditAlertor7->setText(alarmDispNormal);
        if(data.alarm[7] == 0)
			ui->lineEditAlertor8->setText(alarmDispAlarm);
		else
			ui->lineEditAlertor8->setText(alarmDispNormal);
        if(data.alarm[8] == 0)
			ui->lineEditAlertor9->setText(alarmDispAlarm);
		else
			ui->lineEditAlertor9->setText(alarmDispNormal);
        if(data.alarm[9] == 0)
			ui->lineEditAlertor10->setText(alarmDispAlarm);
		else
			ui->lineEditAlertor10->setText(alarmDispNormal);
	}
	else if(Type == InfoAddrExchange)
    {
        if(false == Map.contains(InfoExchangeParamResult))
            return;
		
        QVariant var = Map.value(InfoExchangeParamResult);
        stExchangeResult result = var.value<stExchangeResult>();

        if(result.type == InfoConfigIO)
        {
            StatusRemindWindow  *statusDialog = NULL;
            if(result.result == 0)
                statusDialog = new StatusRemindWindow(this, 1, 5, QObject::tr("IO口配置失败!"));
            else
                statusDialog= new StatusRemindWindow(this, 0, 5, QObject::tr("IO口配置成功!"));

            statusDialog->exec();
            delete statusDialog;
        }
    }
}
/**
 *用户点击保存
 */
void SubStationEnMonitoringInformation::on_pushButtonOK_clicked()
{
	stIOConfigParam data;
	data.io[0] = ui->comboBoxAlertor1->currentIndex(); 
	data.io[1] = ui->comboBoxAlertor2->currentIndex(); 
	data.io[2] = ui->comboBoxAlertor3->currentIndex(); 
	data.io[3] = ui->comboBoxAlertor4->currentIndex(); 
	data.io[4] = ui->comboBoxAlertor5->currentIndex(); 
	data.io[5] = ui->comboBoxAlertor6->currentIndex(); 
	data.io[6] = ui->comboBoxAlertor7->currentIndex(); 
	data.io[7] = ui->comboBoxAlertor8->currentIndex(); 
	data.io[8] = ui->comboBoxAlertor9->currentIndex(); 
	data.io[9] = ui->comboBoxAlertor10->currentIndex(); 
	
	InfoProtocol infoPro;									//下发设置指令
    infoPro.insert(InfoDataType, QByteArray(1, 1));
    infoPro.insert(InfoConfigIO, QByteArray((char *)&data, sizeof(stIOConfigParam)));
    this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
}
