#include <string.h>
#include <QDebug>

#include "VINStartCharging.h"
#include "ui_VINStartCharging.h"
#include "Common.h"
#include "ChargeTerm.h"

VINStartCharging::VINStartCharging(QWidget *parent, CBus *bus, ProtocolBase *protocol) :
    QWidget(parent),
    ui(new Ui::VINStartCharging)
{
    ui->setupUi(this);

    this->canAddr = 0;
    this->chargeType = 0;
    this->bus = bus;
    this->protocol = protocol;

//    /*注册数据*/
//    QList<InfoAddrType> list;
//    list.append(InfoAddrExchange);
//    //this->bus->registDev(this, list);



    QObject::connect(this,SIGNAL(sigFromBus(InfoMap, InfoAddrType)), this, SLOT(slotBusToOwn(InfoMap, InfoAddrType)), Qt::QueuedConnection);
    QObject::connect(ui->timeLimit, SIGNAL(timeout()), this, SLOT(timeoutchange()));

//    /*发送查询车辆VIN命令*/
//    InfoProtocol infoPro;
//    infoPro.insert(InfoDataType, QByteArray(1, 0));
//    infoPro.insert(InfoConfigVINNumber, QByteArray());
//    this->protocol->sendProtocolData(infoPro, InfoAddrConfig);

}

VINStartCharging::~VINStartCharging()
{
    //this->bus->cancelRegistDev(this);
    delete ui;
}

/**
 * @brief 接收到总线过来的数据
 * @param Map
 * @param type
 */
void VINStartCharging::receiveFromBus(InfoMap Map, InfoAddrType type)
{
    //emit sigFromBus(Map, type);
}

void VINStartCharging::slotBusToOwn(InfoMap Map, InfoAddrType Type)
{


}
/**
 *切换到VIN充电界面
 */
void VINStartCharging::switchToVINApplayCharge(unsigned char canAddr, unsigned char type)
{
     ui->timeLimit->startDownCout(30);
     this->canAddr = canAddr;
     this->chargeType = type;
     if(chargeType == 1)      //VIN申请充电
         ui->stackedWidget->setCurrentIndex(0);
     else if(chargeType == 2)
         ui->stackedWidget->setCurrentIndex(1);
}

void VINStartCharging::sendToCSCU(char chargeType, unsigned short chargeEnergy)
{
    InfoProtocol infoPro;
    stChargeCmd chargeCmd;

    memset(&chargeCmd , 0, sizeof(stChargeCmd));
    chargeCmd.canAddr = this->canAddr;
    chargeCmd.cmd = 1;
    chargeCmd.status = CHARGE_STATUS_GUN_STANDBY;
    chargeCmd.chargeEnergy = chargeEnergy;
    chargeCmd.chargeType = chargeType;

    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoExchangeChargeCmd, QByteArray( (char *)&chargeCmd, sizeof(stChargeCmd)));
    this->protocol->sendProtocolData(infoPro, InfoAddrExchange);
}
/**
 *超时时间到了
 */
void VINStartCharging::timeoutchange()
{
    ui->timeLimit->stopDownCout();
    //this->bus->cancelRegistDev(this);

    QVariant varParam;
    emit sigBackToMain(PAGE_CHARGEMANAGE_VINSTART, PAGE_CHARGEMANAGE_MAIN, varParam);
}

/**
 *用户点击确定
 */
void VINStartCharging::on_buttonStartCharge_clicked()
{
    ui->timeLimit->stopDownCout();
    //this->bus->cancelRegistDev(this);
    sendToCSCU(3,0);
    QVariant varParam;
    varParam.setValue(QString(QObject::tr("正在申请充电中，请稍后。。。")));
    emit sigBackToMain(PAGE_CHARGEMANAGE_VINSTART, PAGE_CHARGEMANAGE_FLASHWAIT, varParam);
}
/**
 *用户点击返回
 */
void VINStartCharging::on_buttonBack_clicked()
{
    ui->timeLimit->stopDownCout();
    //this->bus->cancelRegistDev(this);

    QVariant varParam;
    emit sigBackToMain(PAGE_CHARGEMANAGE_VINSTART, PAGE_CHARGEMANAGE_MAIN, varParam);
}

void VINStartCharging::on_buttonStartCardCharge_clicked()
{
    ui->timeLimit->stopDownCout();
    QVariant varParam;
    emit sigBackToMain(PAGE_CHARGEMANAGE_VINSTART, PAGE_CHARGEMANAGE_STANDBY, varParam);
}

void VINStartCharging::on_buttonStartVINCharge_2_clicked()
{
    ui->timeLimit->stopDownCout();
    //this->bus->cancelRegistDev(this);
    sendToCSCU(3,0);
    QVariant varParam;
    varParam.setValue(QString(QObject::tr("正在申请充电中，请稍后。。。")));
    emit sigBackToMain(PAGE_CHARGEMANAGE_VINSTART, PAGE_CHARGEMANAGE_FLASHWAIT, varParam);
}
