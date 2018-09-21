#include <QDebug>

#include "ChargeTerm.h"
#include "TerminalInformation.h"
#include "ui_TerminalInformation.h"

TerminalInformation::TerminalInformation(QWidget *parent, CBus *bus, ProtocolBase *protocol) :
    QWidget(parent),
    ui(new Ui::TerminalInformation)
{
    ui->setupUi(this);
    ui->lineEditTerminalNum->setInputMask("999");
    this->bus = bus;
    this->protocol = protocol;

    /*注册数据*/
    QList<InfoAddrType> list;
    list.append(InfoAddrConfig);
    list.append(InfoAddrReal);
    this->bus->registDev(this, list);

    QObject::connect(this,SIGNAL(sigFromBus(InfoMap, InfoAddrType)), this, SLOT(slotBusToOwn(InfoMap, InfoAddrType)));
    QObject::connect(ui->lineEditTerminalNum, SIGNAL(lineEnterEnd(QString)), this, SLOT(queryEnterEnd(QString)));

    /*发送查询集控设置命令*/
    InfoProtocol infoPro;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoConfigCSCU, QByteArray());
    this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
    currentPosition = 1;

    timerQuery = new QTimer();
    timerQuery->setInterval(2000);
    QObject::connect(timerQuery, SIGNAL(timeout()), this, SLOT(timerQueryTerminal()));
    timerQuery->start();

    clearTerminalInfo();
}

void TerminalInformation::timerQueryTerminal()
{
    queryTermInfo(canAddrMap.value(currentPosition));
}

TerminalInformation::~TerminalInformation()
{
    canAddrMap.clear();
    this->bus->cancelRegistDev(this);
    delete ui;
}

void TerminalInformation::queryEnterEnd(QString str)
{
    bool ok;
    int value = 0;
    unsigned char canAddr = str.toLatin1().toInt(&ok, 10);

    //qDebug() << "TerminalInformation enter canaddr=" << canAddr;
     QMap< int, unsigned char>::iterator it;
     for(it = canAddrMap.begin(); it != canAddrMap.end(); ++it)
     {
           unsigned char canValue = it.value();
           value ++;
           if(canValue == canAddr)
           {
               currentPosition = value;
               queryTermInfo(canAddr);
               return;
           }
     }

    //qDebug() << "TerminalInformation enter canaddr is vaild.";
}

//清空终端信息
void TerminalInformation::clearTerminalInfo()
{
    ui->showAPhaseVoltage->setText("");
    ui->showAPhaseElectricity->setText("");
    ui->showBPhaseVoltage->setText("");
    ui->showBPhaseElectricity->setText("");
    ui->showCPhaseVoltage->setText("");
    ui->showCPhaseElectricity->setText("");
    ui->showTotalActivePower->setText("");
    ui->showTotalReactivePower->setText("");
    ui->showTotalPowerFactor->setText("");
    ui->showVoltageUnbalanceRate->setText("");
    ui->showElectricityUnbalanceRate->setText("");
    ui->showNeutralLineElectricity->setText("");
    ui->showDCSideVoltage->setText("");
    ui->showDCSideElectricity->setText("");
    ui->showTotalActiveEnergy->setText("");
    ui->showTotalReactiveEnergy->setText("");
    ui->showGunTemperature->setText("");
}

void TerminalInformation::showTermInfo(stTerminalReal data)
{
	ui->lineEditTerminalNum->setText(QString::number(data.canAddr, 10));
	
	if(data.status == CHARGE_STATUS_DISCONNECT)
	{
		QString value = QString("--");

		ui->showAPhaseVoltage->setText(value);
		ui->showAPhaseElectricity->setText(value);
		ui->showBPhaseVoltage->setText(value);
		ui->showBPhaseElectricity->setText(value);
		ui->showCPhaseVoltage->setText(value);
		ui->showCPhaseElectricity->setText(value);
		ui->showTotalActivePower->setText(value);
		ui->showTotalReactivePower->setText(value);
		ui->showTotalPowerFactor->setText(value);
		ui->showVoltageUnbalanceRate->setText(value);
		ui->showElectricityUnbalanceRate->setText(value);
		ui->showNeutralLineElectricity->setText(value);
		ui->showDCSideVoltage->setText(value);
		ui->showDCSideElectricity->setText(value);
		ui->showTotalActiveEnergy->setText(value);
		ui->showTotalReactiveEnergy->setText(value);
        ui->showGunTemperature->setText(value);
		return;
	}

	float tmp;
//	ui->lineEditTerminalNum->setText(QString::number(data.canAddr, 10));
	tmp = ((float )data.voltageA)/10;
    ui->showAPhaseVoltage->setText(QString("%1").arg(tmp)+" V");
    tmp = ((float )(qAbs(data.currentA)))/10;
    ui->showAPhaseElectricity->setText(QString("%1").arg(tmp)+" A");
	tmp = ((float )data.voltageB)/10;
    ui->showBPhaseVoltage->setText(QString("%1").arg(tmp)+" V");
    tmp = ((float )(qAbs(data.currentB)))/10;
    ui->showBPhaseElectricity->setText(QString("%1").arg(tmp)+" A");
    tmp = ((float )data.voltageC)/10;
    ui->showCPhaseVoltage->setText(QString("%1").arg(tmp)+" V");
    tmp = ((float )(qAbs(data.currentC)))/10;
    ui->showCPhaseElectricity->setText(QString("%1").arg(tmp)+" A");

    tmp = ((float )data.totalActivePower)/100;
    ui->showTotalActivePower->setText(QString("%1").arg(tmp)+" kW");
    tmp = ((float )(qAbs(data.totalNactivePower)))/100;
    ui->showTotalReactivePower->setText(QString("%1").arg(tmp)+" kW");

    tmp = ((float )(qAbs(data.powerFactor)))/1000;
    ui->showTotalPowerFactor->setText(QString("%1").arg(tmp));

    tmp = ((float )data.volUnbalance)/100;
    ui->showVoltageUnbalanceRate->setText(QString("%1").arg(tmp));
    tmp = ((float )data.curUnbalance)/100;
    ui->showElectricityUnbalanceRate->setText(QString("%1").arg(tmp));
    tmp = ((float )data.zeroLineCurrent)/10;
    ui->showNeutralLineElectricity->setText(QString("%1").arg(tmp)+" A");
    tmp = ((float )data.DCVolatge)/10;
    ui->showDCSideVoltage->setText(QString("%1").arg(tmp)+" V");
    tmp = ((float )(qAbs(data.DCCurrent)))/10;
    ui->showDCSideElectricity->setText(QString("%1").arg(tmp)+" A");

    tmp = ((float )data.totalActiveEnergy)/100;
    ui->showTotalActiveEnergy->setText(QString("%1").arg(tmp)+" kWh");
    tmp = ((float )(qAbs(data.totalNactiveEnergy)))/100;
    ui->showTotalReactiveEnergy->setText(QString("%1").arg(tmp)+" kWh");
    tmp = ((float )data.guntemp)/10;	//merge by yanwei 20171011
    ui->showGunTemperature->setText(QString::number(tmp, 'f', 1)+QObject::tr(" ℃"));	//merge by yanwei 20171011
}
unsigned char TerminalInformation::calculateCanAddr(int type)
{
    if(type == 1)  //下一个
        currentPosition ++;
    else if(type == 2)  //上一个
         currentPosition--;
   if(currentPosition > canAddrMap.size())
      currentPosition = 1;
   else if(currentPosition == 0)
      currentPosition = canAddrMap.size();

    return canAddrMap.value(currentPosition);
}
void TerminalInformation::queryTermInfo(unsigned char canAddr)
{
    InfoProtocol infoPro;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoRealStatus, QByteArray((char *)&canAddr, 1));
    this->protocol->sendProtocolData(infoPro, InfoAddrReal);
}

void TerminalInformation::receiveFromBus(InfoMap Map, InfoAddrType Type)
{
    emit sigFromBus(Map, Type);
}

void TerminalInformation::slotBusToOwn(InfoMap Map, InfoAddrType Type)
{
    switch(Type)
    {
        case InfoAddrReal:
        {
			if(false == Map.contains(InfoRealStatus))
				break;
			QVariant var = Map.value(InfoRealStatus);
			stTerminalReal data = var.value<stTerminalReal>();
			showTermInfo(data);
		}break;
		case InfoAddrConfig:
		{
			if(false == Map.contains(InfoConfigCSCU))
				break;
			
			QVariant var = Map.value(InfoConfigCSCU);
			stCSCUParam param = var.value<stCSCUParam>();

            currentPosition = 0;
            int count = 1;

            for(int i=0; i< param.singlePhaseNum; i++)
               canAddrMap.insert(count++, 1+i);
            for(int i=0; i< param.threePhaseNum; i++)
                canAddrMap.insert(count++, 151+i);
            for(int i=0; i< param.dcPhaseNUm; i++)
                canAddrMap.insert(count++, 181+i);

            unsigned char canAddr = calculateCanAddr(1);
            queryTermInfo(canAddr);
        }break;
        default: break;
    }
}
#if 0
void TerminalInformation::on_buttonFind_clicked()
{
    bool ok;
    unsigned char canAddr = ui->lineEditTerminalNum->text().toLatin1().toInt(&ok, 10);

     QMap< int, unsigned char>::iterator it;
     for(it = canAddrMap.begin(); it != canAddrMap.end(); ++it)
     {
           unsigned char canValue = it.value();
           if(canValue == canAddr)
           {
               queryTermInfo(canAddr);
               return;
           }
     }

    qDebug() << "TerminalInformation::on_buttonFind_clicked enter canaddr is vaild.";
}
#endif

void TerminalInformation::on_buttonUp_clicked()
{
    unsigned char canAddr = calculateCanAddr(2);
    queryTermInfo(canAddr);
}

void TerminalInformation::on_buttonDown_clicked()
{
    unsigned char canAddr = calculateCanAddr(1);
    queryTermInfo(canAddr);
}
