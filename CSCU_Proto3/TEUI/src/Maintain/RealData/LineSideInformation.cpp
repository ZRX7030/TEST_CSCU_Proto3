#include <QVariant>
#include <QDebug>
#include "LineSideInformation.h"
#include "ui_LineSideInformation.h"

LineSideInformation::LineSideInformation(QWidget *parent, CBus *bus, ProtocolBase *protocol) :
    QWidget(parent),
    ui(new Ui::LineSideInformation)
{
    ui->setupUi(this);
    this->bus = bus;
    this->protocol = protocol;

    totalNum = 0;
    currentNum = 0;

    /*注册数据*/
    QList<InfoAddrType> list;
    list.append(InfoAddrReal);
    list.append(InfoAddrConfig);
    this->bus->registDev(this, list);

    QObject::connect(this,SIGNAL(sigFromBus(InfoMap, InfoAddrType)), this, SLOT(slotBusToOwn(InfoMap, InfoAddrType)));

    /*查询电表参数*/
    InfoProtocol infoPro;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoConfigAmmeterAddr, QByteArray());
    this->protocol->sendProtocolData(infoPro, InfoAddrConfig);

    timerQuery = new QTimer();
    timerQuery->setInterval(2000);
    QObject::connect(timerQuery, SIGNAL(timeout()), this, SLOT(timerQueryAmmeter()));
    timerQuery->start();
}

LineSideInformation::~LineSideInformation()
{
    this->bus->cancelRegistDev(this);
    delete ui;
}

void LineSideInformation::timerQueryAmmeter()
{
    queryAmmeterData(currentNum);
}
/**
 *查询电表数据
 */
void LineSideInformation::queryAmmeterData(int position)
{
    InfoProtocol infoPro;

    if(position > totalNum || position == 0)
        return;

    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoRealAmmeter, addrMap.value(position));
    this->protocol->sendProtocolData(infoPro, InfoAddrReal);
}

bool LineSideInformation::BCDToChar( unsigned char* pBCD, int iLen, unsigned char* pChar )
{
     int i = 0;
     unsigned char chHigh; // 保存BCD码中的高位
     unsigned char chLow; // 保存BCD码中的低位

     for (i = 0; i < iLen; i ++)
     {
        chLow = pBCD[i] & 0x0f;
         chHigh = (pBCD[i] & 0xf0) >> 4;

        if ((chHigh > 15) || (chLow > 15))
        {
            return FALSE;
         }
         // 转化得到第一个字符
         if (chHigh<=9)
         {
            pChar[i * 2] = chHigh + '0';
        }
        else
         {
             pChar[i * 2] = chHigh + 'A' - 10;
        }

         // 转化得到第二个字符
         if (chLow<=9)
        {
             pChar[i * 2+1] = chLow + '0';
         }
        else
         {
            pChar[i * 2+1] = chLow + 'A' - 10;
        }
     }
     return true;
}
/**
 * @brief 显示电表数据
 * @param data
 */
void LineSideInformation::showAmmeterData(stAmmeterData data)
{
    char dstStr[30];
	unsigned char tmp_bcd[6];
    float tmp=0;

    ui->lineEditAmmeter->setText(QString::number(currentNum, 10));

	/*电表地址倒序*/
	for(int i=0; i<6; i++)
		tmp_bcd[i] = data.addr[5-i];

    BCDToChar( tmp_bcd, 6, (unsigned char *)dstStr );
    dstStr[12] = 0;
    ui->lineEditAmmeterAddress->setText(QString(dstStr));

    tmp = ((float )data.currentA)/100;
    ui->lineEditAPhaseElectricity->setText(QString("%1").arg(tmp)+" A");
    tmp = ((float )data.voltageA)/10;
    //qDebug() << "A相电压111111111111111111111111=" << data.voltageA;
    ui->lineEditAPhaseVoltage->setText(QString("%1").arg(tmp)+" V");

    tmp = ((float )data.currentB)/100;
    ui->lineEditBPhaseElectricity->setText(QString("%1").arg(tmp)+" A");
    tmp = ((float )data.voltageB)/10;
    ui->lineEditBPhaseVoltage->setText(QString("%1").arg(tmp)+" V");
    //qDebug() << "B 相电流=" << data.currentB;

    tmp = ((float )data.currentC)/100;
    ui->lineEditCPhaseElectricity->setText(QString("%1").arg(tmp)+" A");
    tmp = ((float )data.voltageC)/10;
    ui->lineEditCPhaseVoltage->setText(QString("%1").arg(tmp)+" V");

    tmp = ((float )data.capNactiveEnergy)/100;
    ui->lineEditCapacitiveReactivePower->setText(QString("%1").arg(tmp)+" kWh");

    tmp = ((float )data.zeroLineCurrent)/10;
    ui->lineEditNeutralLineElectricity->setText(QString("%1").arg(tmp)+" A");

    tmp = ((float )data.sensNactiveEnenrgy)/100;
    ui->lineEditPerceptualReactivePower->setText(QString("%1").arg(tmp)+" kWh");

    tmp = ((float )data.totalACtivePower)/100;
    ui->lineEditTotalActivePower->setText(QString("%1").arg(tmp)+" kW");

    tmp = ((float )data.totalNactivePower)/100;
    ui->lineEditTotalActiveEnergy->setText(QString("%1").arg(tmp)+" kW");

    tmp = ((float )data.ActiveAbsorbEnergy)/100;
    ui->lineEditPositiveActivePower->setText(QString("%1").arg(tmp)+" kWh");
    //qDebug() << "正向有功电能=" << data.ActiveAbsorbEnergy;

    tmp = ((float )data.ActiveLiberateEnergy)/100;
    ui->lineEditReverseActivePower->setText(QString("%1").arg(tmp)+" kWh");

    tmp = ((float )data.powerFactor)/1000;
    //qDebug() << "因数=" << data.powerFactor;
    ui->lineEditTotalPowerFactor->setText(QString("%1").arg(tmp));
}

void LineSideInformation::on_buttonUp_clicked()
{
    if(currentNum == 0)
        return;
    currentNum --;
    if(currentNum == 0)
        currentNum = totalNum;

    queryAmmeterData(currentNum);
}

void LineSideInformation::on_buttonDown_clicked()
{
    if(currentNum == 0)
        return;
    currentNum ++;
    if(currentNum > totalNum)
        currentNum--;

    queryAmmeterData(currentNum);
}


void LineSideInformation::receiveFromBus(InfoMap Map, InfoAddrType Type)
{
    emit sigFromBus(Map, Type);
}

void LineSideInformation::slotBusToOwn(InfoMap Map, InfoAddrType Type)
{
    if(Type == InfoAddrReal)
    {
        if(Map.contains(InfoRealAmmeter))
        {
            QVariant var = Map.value(InfoRealAmmeter);
            stAmmeterData data = var.value<stAmmeterData>();

            showAmmeterData(data);
        }
    }
    else if(Type == InfoAddrConfig)         //电表地址
    {
        if (Map.contains(InfoConfigAmmeterAddr))
        {
            QVariant var = Map.value(InfoConfigAmmeterAddr);
            stAllAmmeterAddr ammeterAddr = var.value<stAllAmmeterAddr>();
            QList<stAmmeterAddr> ammeterList = ammeterAddr.ammeterList;

            totalNum = ammeterList.size();
            if(totalNum)
                currentNum = 1;

            ui->labelTotal->setText(QObject::tr("共") + QString::number(currentNum, 10) + QObject::tr("块电表"));
			
			/*讲电表地址放到Map里*/
            for(int i=0; i< ammeterList.size(); i++)
            {
                stAmmeterAddr addr = ammeterList.at(i);
                addrMap.insert(i+1, QByteArray((char *)addr.ammeterAddr, 6));
            }

            queryAmmeterData(currentNum);
        }
    }
}
