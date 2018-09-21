#include <QDebug>

#include "InfoData.h"
#include "ChagingFailureInformation.h"
#include "ui_ChagingFailureInformation.h"
#include "TimeLimit.h"
#include <qscrollbar.h>


ChagingFailureInformation::ChagingFailureInformation(QWidget *parent, CBus *bus, ProtocolBase *protocol) :
    QWidget(parent),
    ui(new Ui::ChagingFailureInformation)
{
    ui->setupUi(this);
    createTableItems();
    this->bus = bus;
    this->protocol = protocol;
    this->canAddr = 0;

    QObject::connect(this,SIGNAL(sigFromBus(InfoMap, InfoAddrType)), this, SLOT(slotBusToOwn(InfoMap, InfoAddrType)), Qt::QueuedConnection);
    QObject::connect(ui->timeLimit, SIGNAL(timeout()), this, SLOT(timeoutchange()));
}

ChagingFailureInformation::~ChagingFailureInformation()
{
    this->bus->cancelRegistDev(this);
    delete ui;
}

void ChagingFailureInformation::switchToChargeFalut(unsigned char canAddr)
{
	/*注册数据*/
    QList<InfoAddrType> list;
    list.append(InfoAddrReal);
    this->bus->registDev(this, list);

    this->canAddr = canAddr;
    clearChargeFailureInfor();
    queryChargeFaultReport(this->canAddr);
    ui->timeLimit->startDownCout(30);
   // qDebug() << "///////////////////////22222222222222222";
}
void ChagingFailureInformation::settablecolumncount()
{
    QTableWidget *table = ui->tableWidgetFailureinfor;
    for(int i=0; i<table->rowCount(); i++)
    {
        for(int k=0; k<table->columnCount(); k++)
        {
             ui->tableWidgetFailureinfor->setColumnWidth(0,120);
             ui->tableWidgetFailureinfor->setColumnWidth(1,120);
             ui->tableWidgetFailureinfor->setColumnWidth(2,120);
             ui->tableWidgetFailureinfor->setColumnWidth(3,310);
        }
    }
}
void ChagingFailureInformation::clearChargeFailureInfor()
{
    QTableWidget *table = ui->tableWidgetFailureinfor;
    for(int i=0; i<table->rowCount(); i++)
    {
        for(int k=0; k<table->columnCount(); k++)
            table->item(i, k)->setText(QString(""));
    }
    ui->labelSOC->setText("");
    ui->labelStartTime->setText("");
    ui->labelStopTime->setText("");
    ui->labelSuspendReason->setText("");
}
void ChagingFailureInformation::receiveFromBus(InfoMap Map, InfoAddrType type)
{
     emit sigFromBus(Map, type);
}

void ChagingFailureInformation::queryChargeFaultReport(unsigned char canAddr)
{
    InfoProtocol infoPro;
    infoPro.insert(InfoDataType,QByteArray(1,0));
    infoPro.insert(InfoRealFault,QByteArray((char *)&canAddr, 1));
    this->protocol->sendProtocolData(infoPro,InfoAddrReal);
}
void ChagingFailureInformation::createTableItems()
{
    tablseItemInit(ui->tableWidgetFailureinfor);
    settablecolumncount();
}
void ChagingFailureInformation::tablseItemInit(QTableWidget *table)
{
    for(int i=0; i<table->rowCount(); i++)
    {
        for(int k=0; k<table->columnCount(); k++)
        {
            QTableWidgetItem *tableItem = new QTableWidgetItem();
            tableItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter );
            table->setItem(i, k, tableItem);
            table->item(i, k)->setText(QString(""));
            table->item(i,k)->setFlags(Qt::NoItemFlags);   //设置第一列不能触摸
        }
    }
}
void ChagingFailureInformation::slotBusToOwn(InfoMap Map, InfoAddrType Type)
{
    //qDebug() << "///////////////////////3333333333333333333333";
    if(Type == InfoAddrReal)
    {
        if(false == Map.contains(InfoRealFault))
            return;

        QVariant var = Map.value(InfoRealFault);
        stAllTerminalFault data = var.value<stAllTerminalFault>();
        //qDebug() << "///////////////////////444444444444444444444444";

        for(int i=0; i<data.faultList.size();i++)
        {
            //qDebug() << "///////////////////////555555555555555555" << this->canAddr;
            stTerminalFault fault = data.faultList.at(i);
            QTableWidget *table = ui->tableWidgetFailureinfor;
            table->item(i,0)->setText(QString::number(i+1, 10));
            table->item(i,1)->setText(QString::number(fault.canAddr, 10));
            table->item(i,2)->setText(QString::number(fault.numID, 10));
            table->item(i,3)->setText(QObject::tr(fault.faultInfo));
        }

        ui->labelStartTime->setText(QString(data.chargestartTime));
        ui->labelStopTime->setText(QString(data.chargestopTime));
        ui->labelSuspendReason->setText(QObject::tr(data.stopReson));
        //ui->labelSOC->setText(QString::number(data.currentSOC, 10)+"%");

        if(QString(data.chargestartTime) == "")
        {
            ui->labelSOC->setText(QString(""));
        }
        else
        {
            if(this->canAddr > 180)
                ui->labelSOC->setText(QString::number(data.currentSOC, 10)+"%");
            else
                ui->labelSOC->setText(QString(QObject::tr("无")));
        }
    }
}

/**
 *充电中定时数据查询
 */
void ChagingFailureInformation::slotTimerQueryChargingFault()
{
    queryChargeFaultReport(this->canAddr);
}
#if 0
void ChagingFailureInformation::showFailureInformation()
{
    QTableWidget *table = ui->tableWidgetFailureinfor;
    for(int i=0; i )
}
#endif
void ChagingFailureInformation::timeoutchange()
{
    this->bus->cancelRegistDev(this);

	QVariant var;
	emit sigBackToMain(PAGE_CHARGEMANAGE_FAULT, PAGE_CHARGEMANAGE_MAIN, var);
    ui->timeLimit->stopDownCout();
}
void ChagingFailureInformation::on_buttonReturn_clicked()
{
    this->bus->cancelRegistDev(this);

	QVariant var;
	emit sigBackToMain(PAGE_CHARGEMANAGE_FAULT, PAGE_CHARGEMANAGE_MAIN, var);
    ui->timeLimit->stopDownCout();
}

void ChagingFailureInformation::on_tabWidget_currentChanged(int index)
{
//    InfoProtocol infoPro;
//    infoPro.insert(InfoDataType,QByteArray(1,0));
//    infoPro.insert(InfoRealFault,QByteArray((char *)&canAddr, 1));
//    this->protocol->sendProtocolData(infoPro,InfoAddrReal);
}
