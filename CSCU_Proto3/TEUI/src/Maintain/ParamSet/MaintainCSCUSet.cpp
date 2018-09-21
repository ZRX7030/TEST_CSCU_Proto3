#include <QDebug>
#include <QRegExp>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "MaintainCSCUSet.h"
#include "ui_MaintainCSCUSet.h"
#include "Common.h"
#include "SwapBase.h"
#include "InfoData.h"
#include "StatusRemindWindow.h"
#include <qvalidator.h>

MaintainCSCUSet::MaintainCSCUSet(QWidget *parent, CBus *bus, ProtocolBase *protocol, void *param) :
    QWidget(parent),
    ui(new Ui::MaintainCSCUSet)
{
    ui->setupUi(this);
	teuiParam = (stTeuiParam *)param;
    //this->setAttribute(Qt::WA_DeleteOnClose);
	this->bus = bus;
	this->protocol = protocol;
    ui->labelZigbeeAddress->hide();
    ui->lineEditZigbeeAddress->hide();
	
	ui->lineEditIP->setMaxLength(15);
    ui->lineEditDefaultGateway->setMaxLength(15);
    ui->lineEditDNSServer->setMaxLength(15);
    //ui->lineEditIP->setInputMask("000.000.000.000;");
   // ui->lineEditDefaultGateway->setInputMask("000.000.000.000;");
    //ui->lineEditDNSServer->setInputMask("000.000.000.000;");
    //ui->lineEditServer1Address->setInputMask("000.000.000.000;");
    ui->lineEditServer1PortNumber->setInputMask("99999");
    ui->lineEditServer2PortNumber->setInputMask("99999");
    ui->lineEditServer3PortNumber->setInputMask("99999");
    ui->lineEditStationAddress->setMaxLength(15);
    ui->lineEditDCTerminalCount->setInputMask("99");
    ui->lineEditThreePhaseTerminalCount->setInputMask("99");
    ui->lineEditSingleNum->setInputMask("99");

	/*注册数据*/
	QList<InfoAddrType> list;
	list.append(InfoAddrConfig);
    list.append(InfoAddrExchange);
	this->bus->registDev(this, list);
    
	QObject::connect(this,SIGNAL(sigFromBus(InfoMap, InfoAddrType)), this, SLOT(slotBusToOwn(InfoMap, InfoAddrType)), Qt::QueuedConnection);
    /*发送查询集控设置命令*/
    InfoProtocol infoPro;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoConfigCSCU, QByteArray());
    this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
}

MaintainCSCUSet::~MaintainCSCUSet()
{
	this->bus->cancelRegistDev(this);
    delete ui;
}

void MaintainCSCUSet::receiveFromBus(InfoMap Map, InfoAddrType Type)
{
	emit sigFromBus(Map, Type);
}

void MaintainCSCUSet::slotBusToOwn(InfoMap Map, InfoAddrType Type)
{
    //qDebug() << " MaintainCSCUSet receive data\n";
	if(Type == InfoAddrConfig)
	{
		if(false == Map.contains(InfoConfigCSCU))
			return;
		
		char tmpBuff[50];
		QVariant var = Map.value(InfoConfigCSCU);

		stCSCUParam param = var.value<stCSCUParam>();
		ui->lineEditIP->setText(param.ipAddr);
		ui->lineEditDefaultGateway->setText(param.gateway);
		ui->lineEditDNSServer->setText(param.dns);

        ui->lineEditServer1Address->setText(param.server1Host);
        snprintf(tmpBuff, sizeof(tmpBuff), "%d", param.server1Port);
        ui->lineEditServer1PortNumber->setText(tmpBuff);
        ui->lineEditServer2Address->setText(param.server2Host);
        snprintf(tmpBuff, sizeof(tmpBuff), "%d", param.server2Port);
        ui->lineEditServer2PortNumber->setText(tmpBuff);
        ui->lineEditServer3Address->setText(param.server3Host);
        snprintf(tmpBuff, sizeof(tmpBuff), "%d", param.server3Port);
        ui->lineEditServer3PortNumber->setText(tmpBuff);

        ui->lineEditStationAddress->setText(param.stationAddr);
        snprintf(tmpBuff, sizeof(tmpBuff), "%d", param.canAddr);
        ui->lineEditZigbeeAddress->setText(tmpBuff);

        snprintf(tmpBuff, sizeof(tmpBuff), "%d", param.singlePhaseNum);
        ui->lineEditSingleNum->setText(tmpBuff);

        snprintf(tmpBuff, sizeof(tmpBuff), "%d", param.threePhaseNum);
        ui->lineEditThreePhaseTerminalCount->setText(tmpBuff);

        snprintf(tmpBuff, sizeof(tmpBuff), "%d", param.dcPhaseNUm);
        ui->lineEditDCTerminalCount->setText(tmpBuff);

	}
    else if(Type == InfoAddrExchange)
    {
        if(false == Map.contains(InfoExchangeParamResult))
            return;
        QVariant var = Map.value(InfoExchangeParamResult);
        stExchangeResult result = var.value<stExchangeResult>();

        if(result.type == InfoConfigCSCU)  //back cscu config reuslt
        {
			int flag=1;
			int time = 5;
			QString text;
    
			if(result.result == 0)
			{
				flag = 1;
				text = QObject::tr("集控设置失败");
			}
			else
			{
				flag = 0;
				text = QObject::tr("集控设置成功");
			}
			if(result.rebootFlag == 1)
			{
				time = 10;
                text.append(QObject::tr("系统正在重启，请稍等！"));
				teuiParam->showTermWin = false;
			}
			StatusRemindWindow  *statusDialog = new StatusRemindWindow(this, flag, time, text);
            statusDialog->exec();
            delete statusDialog;
        }
    }
}

bool MaintainCSCUSet::ipValidCheck(QString ip )
{
#if 0
    QRegExp rx2("^([1]?/d/d?|2[0-4]/d|25[0-5])/.([1]?/d/d?|2[0-4]/d|25[0-5])/.([1]?/d/d?|2[0-4]/d|25[0-5])/.([1]?/d/d?|2[0-4]/d|25[0-5])$");

    if( !rx2.exactMatch(ip) )
        return false;
    else
        return true;
#endif
	struct in_addr addr;
	int ret;

	ret = inet_pton(AF_INET, ip.toLatin1().data(), &addr);
	if (ret > 0)
		return true;
	else
		return false;
}

void MaintainCSCUSet::on_buttonSave_clicked()
{
	bool ok;
    InfoProtocol infoPro;
    stCSCUParam param;

    bool ipaddr = ipValidCheck(ui->lineEditIP->text().toLatin1().data());
    bool dns = ipValidCheck(ui->lineEditDNSServer->text().toLatin1().data());
    bool gateway = ipValidCheck(ui->lineEditDefaultGateway->text().toLatin1().data());

    if(ipaddr == false || dns == false || gateway == false)
    {
        StatusRemindWindow  *statusDialog = new StatusRemindWindow(this, 1, 5, QObject::tr("输入的IP格式错误"));
        statusDialog->exec();
        delete statusDialog;
        return ;
    }

	if((ui->lineEditSingleNum->text().toLatin1().toInt(&ok, 10) == 0 ) && 
			(ui->lineEditThreePhaseTerminalCount->text().toLatin1().toInt(&ok, 10) == 0 ) && 
			(ui->lineEditDCTerminalCount->text().toLatin1().toInt(&ok, 10) == 0) )
	{
		StatusRemindWindow  *statusDialog = new StatusRemindWindow(this, 1, 5, QObject::tr("输入的终端个数不能全为0"));
		statusDialog->exec();
		delete statusDialog;
		return ;
	}

	
	memset(&param, 0, sizeof(stCSCUParam));
	snprintf(param.netmask, sizeof(param.netmask),"%s", "255.255.255.0");

	snprintf(param.ipAddr, sizeof(param.ipAddr), "%s", ui->lineEditIP->text().toLatin1().data());
	snprintf(param.gateway, sizeof(param.gateway), "%s", ui->lineEditDefaultGateway->text().toLatin1().data());
	snprintf(param.dns, sizeof(param.dns), "%s", ui->lineEditDNSServer->text().toLatin1().data());
    snprintf(param.server1Host, sizeof(param.server1Host), "%s", ui->lineEditServer1Address->text().toLatin1().data());
    param.server1Port = ui->lineEditServer1PortNumber->text().toLatin1().toInt(&ok, 10);
    snprintf(param.server2Host, sizeof(param.server2Host), "%s", ui->lineEditServer2Address->text().toLatin1().data());
    param.server2Port = ui->lineEditServer2PortNumber->text().toLatin1().toInt(&ok, 10);
    snprintf(param.server3Host, sizeof(param.server3Host), "%s", ui->lineEditServer3Address->text().toLatin1().data());
    param.server3Port = ui->lineEditServer3PortNumber->text().toLatin1().toInt(&ok, 10);
    snprintf(param.stationAddr, sizeof(param.stationAddr), "%s", ui->lineEditStationAddress->text().toLatin1().data());
	param.canAddr = ui->lineEditZigbeeAddress->text().toLatin1().toInt(&ok, 10);
    param.singlePhaseNum = ui->lineEditSingleNum->text().toLatin1().toInt(&ok, 10);
    param.threePhaseNum = ui->lineEditThreePhaseTerminalCount->text().toLatin1().toInt(&ok, 10);
    param.dcPhaseNUm = ui->lineEditDCTerminalCount->text().toLatin1().toInt(&ok, 10);

    if(param.singlePhaseNum + param.threePhaseNum + param.dcPhaseNUm > 32)  //设置终端总个数限置
    {
        StatusRemindWindow  *statusDialog = new StatusRemindWindow(this, 1, 5, QObject::tr("输入的终端总个数之和不能大于32个!"));
        statusDialog->exec();
        delete statusDialog;
        return ;
    }

    infoPro.insert(InfoDataType, QByteArray(1, 1));
    infoPro.insert(InfoConfigCSCU, QByteArray((char *)&param, sizeof(stCSCUParam)));
    this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
}
