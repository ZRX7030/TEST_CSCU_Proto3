#include "src/Charge/PrintPaper.h"
#include "ui_PrintPaper.h"

PrintPaper::PrintPaper(QWidget *parent, CBus *bus, ProtocolBase *protocol) :
    QWidget(parent),
    ui(new Ui::PrintPaper)
{
    ui->setupUi(this);
    this->bus = bus;
    this->protocol = protocol;
    this->canAddr = 0;
    QObject::connect(ui->timeLimit, SIGNAL(timeout()), this, SLOT(timeoutchange()));
}

PrintPaper::~PrintPaper()
{
    delete ui;
}

void PrintPaper::switchToPrintPaper(unsigned char canAddr)
{
    this->canAddr = canAddr;
    ui->timeLimit->startDownCout(30);
    //ui->labelCANID->setText(QString::number(canAddr, 10)+ QObject::tr("号"));
}
void PrintPaper::receiveFromBus(InfoMap, InfoAddrType)
{

}
void PrintPaper::sendPrintPaper()
{
    InfoProtocol infoPro;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoExchangePrintPaper , QByteArray((char *)&canAddr, 1));
    this->protocol->sendProtocolData(infoPro, InfoAddrExchange);
}
void PrintPaper::timeoutchange()
{
    QVariant var;
    emit sigBackToMain(PAGE_CHARGEMANAGE_PRINTPAPER, PAGE_CHARGEMANAGE_MAIN, var);
    ui->timeLimit->stopDownCout();
}
void PrintPaper::on_buttonOK_clicked()
{
    sendPrintPaper();
    QVariant varParam;
    varParam.setValue(QString(QObject::tr("正在打印小票，请稍后。。。")));
    emit sigBackToMain(PAGE_CHARGEMANAGE_PRINTPAPER, PAGE_CHARGEMANAGE_FLASHWAIT, varParam);
    ui->timeLimit->stopDownCout();
}

void PrintPaper::on_buttonBack_clicked()
{
    QVariant var;
    emit sigBackToMain(PAGE_CHARGEMANAGE_PRINTPAPER, PAGE_CHARGEMANAGE_MAIN, var);
    ui->timeLimit->stopDownCout();
}
