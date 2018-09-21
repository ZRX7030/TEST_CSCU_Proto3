#include <QDebug>

#include "ChargeTerm.h"
#include "BMSInformation.h"
#include "Common.h"
#include "ui_BMSInformation.h"

BMSInformation::BMSInformation(QWidget *parent, CBus *bus, ProtocolBase *protocol) :
    QWidget(parent),
    ui(new Ui::BMSInformation)
{
    ui->setupUi(this);
    ui->lineEditTerminal->setInputMask("999");

    this->bus = bus;
    this->protocol = protocol;

    /*注册数据*/
    QList<InfoAddrType> list;
    list.append(InfoAddrConfig);
    list.append(InfoAddrReal);
    this->bus->registDev(this, list);

    QObject::connect(this,SIGNAL(sigFromBus(InfoMap, InfoAddrType)), this, SLOT(slotBusToOwn(InfoMap, InfoAddrType)));
    QObject::connect(ui->lineEditTerminal, SIGNAL(lineEnterEnd(QString)), this, SLOT(queryEnterEnd(QString)));

    /*发送查询集控设置命令*/
    InfoProtocol infoPro;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoConfigCSCU, QByteArray());
    this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
    ui->lineEditTerminal->setText(QString("0"));
    currentPosition = 1;

    timerQuery = new QTimer();
    timerQuery->setInterval(2000);
    QObject::connect(timerQuery, SIGNAL(timeout()), this, SLOT(timerQueryBMS()));
    timerQuery->start();
   // ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
}

BMSInformation::~BMSInformation()
{
    if(timerQuery)
        delete timerQuery;

    canAddrMap.clear();
    this->bus->cancelRegistDev(this);
    delete ui;
}

void BMSInformation::timerQueryBMS()
{
    queryBMSInfo(canAddrMap.value(currentPosition));
}

void BMSInformation::queryEnterEnd(QString str)
{
    bool ok;
    int value = 0;
    unsigned char canAddr = str.toLatin1().toInt(&ok, 10);

    //qDebug() << "BMSInformation enter canaddr=" << canAddr;
     QMap< int, unsigned char>::iterator it;
     for(it = canAddrMap.begin(); it != canAddrMap.end(); ++it)
     {
           unsigned char canValue = it.value();
           value ++;
           if(canValue == canAddr)
           {
               currentPosition = value;
               queryBMSInfo(canAddr);
               return;
           }
     }

    //qDebug() << "enter canaddr is vaild.";
}
/**
 * @brief 根据个数判断can地址
 * @param type
 */
unsigned char BMSInformation::calculateCanAddr(int type)
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

 void BMSInformation::slotBusToOwn(InfoMap Map, InfoAddrType Type)
 {
     switch(Type)
     {
         case InfoAddrReal:
         {
             //qDebug() << "BMSInformation::slotBusToOwn";
			 if(false == Map.contains(InfoRealBMS))
				 break;
             QVariant var = Map.value(InfoRealBMS);

             stTerminalBMS data = var.value<stTerminalBMS>();
             showBmsInfo(data);
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
             queryBMSInfo(canAddr);
         }break;
         default: break;
     }
 }
/**
 * @brief 接收到总线过来的数据
 * @param Map
 * @param type
 */
void BMSInformation::receiveFromBus(InfoMap Map, InfoAddrType type)
{
     emit sigFromBus(Map, type);
}

void BMSInformation::queryBMSInfo(unsigned char canAddr)
{
    InfoProtocol infoPro;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoRealBMS, QByteArray((char *)&canAddr, 1));
    this->protocol->sendProtocolData(infoPro, InfoAddrReal);
}
/**
 *显示Bms信息
 */
void BMSInformation::showBmsInfo(stTerminalBMS data)
{
	ui->lineEditTerminal->setText(QString::number(data.canAddr, 10));
	
	if(data.status == CHARGE_STATUS_DISCONNECT)  //离线
	{
		QString value = QString("--");
		ui->showBMSRequirementVoltage->setText(value);
		ui->showBMSRequirementElectricity->setText(value);
		ui->showPresentSOC->setText(value);
		ui->showMaxBatteryTemperature->setText(value);
		ui->showMinBatteryTemperature->setText(value);
		ui->showMaxBatteryVoltage->setText(value);
		ui->showMinBatteryVoltage->setText(value);
	
		return;
	}

    if(data.status == CHARGE_STATUS_FREE)  //待机-空闲
    {
        QString value = QString("--");
        ui->showBMSRequirementVoltage->setText(value);
        ui->showBMSRequirementElectricity->setText(value);
        ui->showPresentSOC->setText(value);
        ui->showMaxBatteryTemperature->setText(value);
        ui->showMinBatteryTemperature->setText(value);
        ui->showMaxBatteryVoltage->setText(value);
        ui->showMinBatteryVoltage->setText(value);

        return;
    }
	float tmp;
	//ui->lineEditTerminal->setText(QString::number(data.canAddr, 10));

	tmp = ((float )data.BMSNeedVoltage)/10;
    ui->showBMSRequirementVoltage->setText(QString("%1").arg(tmp)+" V");
    tmp = ((float )(qAbs(data.BMSNeedCurrent)))/10;
    ui->showBMSRequirementElectricity->setText(QString("%1").arg(tmp)+" A");
    ui->showPresentSOC->setText(QString::number(data.currentSoc, 10)+" %");
    ui->showMaxBatteryTemperature->setText(QString::number(data.maxBatteryTemp, 10)+ QObject::tr(" 度"));
    ui->showMinBatteryTemperature->setText(QString::number(data.minBatteryTemp, 10)+ QObject::tr(" 度"));
    tmp = ((float )data.maxBatteryVoltage)/100;
    ui->showMaxBatteryVoltage->setText(QString("%1").arg(tmp)+" V");
    tmp = ((float )data.minBatteryVoltage)/100;
    ui->showMinBatteryVoltage->setText(QString("%1").arg(tmp)+" V");
}
#if 0
/**
 * 点击查询按钮
 */
void BMSInformation::on_buttonFind_clicked()
{
    bool ok;
    unsigned char canAddr = ui->lineEditTerminal->text().toLatin1().toInt(&ok, 10);

    qDebug() << "BMSInformation enter canaddr=" << canAddr;
     QMap< int, unsigned char>::iterator it;
     for(it = canAddrMap.begin(); it != canAddrMap.end(); ++it)
     {
           unsigned char canValue = it.value();
           if(canValue == canAddr)
           {
               queryBMSInfo(canAddr);
               return;
           }
     }

    qDebug() << "enter canaddr is vaild.";
}
#endif
/**
 * 点击上一个按钮
 */
void BMSInformation::on_buttonUp_clicked()
{
    unsigned char canAddr = calculateCanAddr(2);
    queryBMSInfo(canAddr);
}
/**
 * 点击下一个按钮
 */
void BMSInformation::on_buttonDown_clicked()
{
    unsigned char canAddr = calculateCanAddr(1);
    queryBMSInfo(canAddr);
}
