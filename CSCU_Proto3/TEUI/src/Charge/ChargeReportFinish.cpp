#include <QDebug>
#include <QDateTime>
#include "Common.h"
#include "InfoData.h"
#include "ChargeReportFinish.h"
#include "ui_ChargeReportFinish.h"
#include "TimeLimit.h"


ChargeReportFinish::ChargeReportFinish(QWidget *parent, CBus *bus, ProtocolBase *protocol) :
    QWidget(parent),
    ui(new Ui::ChargeReportFinish)
{
    ui->setupUi(this);
    this->bus = bus;
    this->protocol = protocol;
    this->canAddr = 0;

    /*注册数据*/
    QList<InfoAddrType> list;
    list.append(InfoAddrHistory);
    list.append(InfoAddrConfig);
    this->bus->registDev(this, list);

    QObject::connect(this,SIGNAL(sigFromBus(InfoMap, InfoAddrType)), this, SLOT(slotBusToOwn(InfoMap, InfoAddrType)), Qt::QueuedConnection);
    QObject::connect(ui->timeLimit, SIGNAL(timeout()), this, SLOT(timeoutchange()));

    ui->labelSuspendReason->setWordWrap(true);
    ui->buttonPrintTicket->hide();
    //setWindowFlags(Qt::FramelessWindowHint|Qt::Dialog);
//    this->setWindowOpacity(1); //窗口整体透明度，0-1 从全透明到不透明
  //  this->setAttribute(Qt::WA_TranslucentBackground); //设置背景透明，允许鼠标穿透

}

ChargeReportFinish::~ChargeReportFinish()
{
    this->bus->cancelRegistDev(this);
    delete ui;
}
/**
 *切换到充电完成
 */
void ChargeReportFinish::switchToChargeFinish(unsigned char canAddr)
{
    this->canAddr = canAddr;
    clearChargeReportFinish();
    queryChargeFinishReport(this->canAddr);
    ui->timeLimit->startDownCout(30);
}

void ChargeReportFinish::receiveFromBus(InfoMap Map, InfoAddrType type)
{
     emit sigFromBus(Map, type);
}
void ChargeReportFinish::clearChargeReportFinish()  //清除充电报告　add by songqb 2017-6-6
{
    ui->labelCANID->setText("");
    ui->labelChargingEnergy->setText("");
    ui->labelChargingTime->setText("");
    ui->labelEndTime->setText("");
    ui->labelStartSOC->setText("");
    ui->labelStartTime->setText("");
    ui->labelSuspendReason->setText("");
}
void ChargeReportFinish::slotBusToOwn(InfoMap Map, InfoAddrType Type)
{
    if(Type == InfoAddrHistory)
    {
        //qDebug() << "receive charge report...........................";
        if(false == Map.contains(InfoChargeReport))
            return;

        QVariant var = Map.value(InfoChargeReport);
        stChargeReport data = var.value<stChargeReport>();
        float tmp;
        //ui->labelCANID->setText(QString::number(data.canAddr, 10));

       // tmp = ((float )data.chargeEnergy)/10;
        ui->labelChargingEnergy->setText(QString::number((double)data.chargeEnergy/100, 'f',  2)+"kWh");
//        ui->labelStartTime->setText(QString(data.startTime));
//        ui->labelEndTime->setText(QString(data.stopTime));
        QDateTime startTime = QDateTime::fromString(QString(data.startTime),"yyyy-MM-dd hh:mm:ss");
        QDateTime stopTime = QDateTime::fromString(QString(data.stopTime),"yyyy-MM-dd hh:mm:ss");
        ui->labelStartTime->setText(QString(startTime.toString("hh:mm:ss")));
        ui->labelEndTime->setText(QString(stopTime.toString("hh:mm:ss")));

        ui->labelChargingTime->setText(QString::number(data.chargeTime, 10)+QString(QObject::tr("分")));
        ui->labelSuspendReason->setText(QObject::tr(data.stopReson));
        if(data.canAddr > 180)
        {
            ui->labelStartSOC->setText(QString::number(data.startSoc, 10)+"%");
            ui->progressBar->setTextVisible(true);
            ui->progressBar->setValue(data.stopSoc);
        }
        else
        {
            ui->labelStartSOC->setText(QString(QObject::tr("无")));
            ui->progressBar->setValue(0);
            ui->progressBar->setTextVisible(false);
        }
    }
    else if(Type == InfoAddrConfig)
    {
        if(Map.contains(InfoConfigSpecialFunc))
        {
            QVariant var = Map.value(InfoConfigSpecialFunc);
            stSpecialFunc param = var.value<stSpecialFunc>();
            par = param;
            qDebug() << "receive special..........................."<<par.printPaper;
            if(par.printPaper == 1)
                ui->buttonPrintTicket->show();
            else
                ui->buttonPrintTicket->hide();
        }
    }
}

void ChargeReportFinish::queryChargeFinishReport(unsigned char canAddr)
{
    //qDebug() << "queryChargeFinishReport canAddr=" << canAddr;
    InfoProtocol infoPro;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoChargeReport, QByteArray((char *)&canAddr, 1));
    this->protocol->sendProtocolData(infoPro, InfoAddrHistory);
}
void ChargeReportFinish::timeoutchange()
{
	QVariant var;
	emit sigBackToMain(PAGE_CHARGEMANAGE_FINISH, PAGE_CHARGEMANAGE_MAIN, var);
    ui->timeLimit->stopDownCout();
}
void ChargeReportFinish::on_buttonSave_clicked()
{
	QVariant var;
	emit sigBackToMain(PAGE_CHARGEMANAGE_FINISH, PAGE_CHARGEMANAGE_MAIN, var);
    ui->timeLimit->stopDownCout();
}

void ChargeReportFinish::on_buttonStartCharging_clicked()
{
    ui->timeLimit->stopDownCout();
    QVariant var;
    emit sigBackToMain(PAGE_CHARGEMANAGE_FINISH, PAGE_CHARGEMANAGE_STANDBY, var);
}

void ChargeReportFinish::on_buttonPrintTicket_clicked()
{
    ui->timeLimit->stopDownCout();
    QVariant var;
    emit sigBackToMain(PAGE_CHARGEMANAGE_FINISH, PAGE_CHARGEMANAGE_PRINTPAPER, var);
}

void ChargeReportFinish::queryChargeFinishSpecialFeatureSet()
{
    InfoProtocol infoPro;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoConfigSpecialFunc, QByteArray());
    this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
}
