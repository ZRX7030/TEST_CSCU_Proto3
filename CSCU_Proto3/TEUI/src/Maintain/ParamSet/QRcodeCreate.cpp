#include <QDebug>
#include <QList>
#include <string.h>

#include "QRcodeCreate.h"
#include "ui_QRcodeCreate.h"
#include "StatusRemindWindow.h"
QRcodeCreate::QRcodeCreate(QWidget *parent, CBus * bus, ProtocolBase *protocol) :
    QWidget(parent),
    ui(new Ui::QRcodeCreate)
{
    ui->setupUi(this);
    ui->tableWidgetQRcode->setColumnWidth(0,150);
    ui->tableWidgetQRcode->setColumnWidth(1,320);

    createTableItems();

    this->bus = bus;
    this->protocol = protocol;

    this->currentPage = 0;
    this->totalNum = 0;
    this->totalPage = 0;

    /*注册数据*/
    QList<InfoAddrType> list;
    list.append(InfoAddrConfig);
    list.append(InfoAddrExchange);
    this->bus->registDev(this, list);

    QObject::connect(this,SIGNAL(sigFromBus(InfoMap, InfoAddrType)), this, SLOT(slotBusToOwn(InfoMap, InfoAddrType)), Qt::QueuedConnection);

    /*发送查询二维码生成命令*/
    InfoProtocol infoPro;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoConfigTerminalQRCode, QByteArray());
    this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
}

QRcodeCreate::~QRcodeCreate()
{
    this->bus->cancelRegistDev(this);
    delete ui;
}

/**
 * @brief 表格创建
 */
void QRcodeCreate:: createTableItems()
{
    tablseItemInit(ui->tableWidgetQRcode);
}
/**
 * @brief 单元格初始化
 * @param table
 */
void QRcodeCreate::tablseItemInit(QTableWidget *table)
{
    for(int i=0; i<table->rowCount(); i++)
    {
        for(int k=0; k<table->columnCount(); k++)
        {
            QTableWidgetItem *tableItem = new QTableWidgetItem();
            tableItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter );
            table->setItem(i, k, tableItem);
            table->item(i, k)->setText(QString(""));
            table->item(i,0)->setFlags(Qt::NoItemFlags);   //设置第一列不能触摸
        }
    }
}
void QRcodeCreate::receiveFromBus(InfoMap Map, InfoAddrType Type)
{
    emit sigFromBus(Map, Type);
}

void QRcodeCreate::slotBusToOwn(InfoMap Map, InfoAddrType Type)
{
    //qDebug() << "QRcodeCreate receive data";
    if(Type == InfoAddrConfig)     //收到配置数据
    {
        if(false == Map.contains(InfoConfigTerminalQRCode))
            return;

        QVariant var = Map.value(InfoConfigTerminalQRCode);
        stAllQRcodeCreate param = var.value<stAllQRcodeCreate>();
        codeCreatelist.clear();
        codeCreatelist = param.codeList;

        totalNum = param.codeList.size();
        totalPage = totalNum/10;
        if(totalNum%10)
            totalPage += 1;

       // qDebug() << "total num, totalpage " << totalNum << " " << totalPage;
        currentPage = 1;

        int startPos = 0, count = 0;
        if(findListPosition(currentPage, startPos, count))
        {
            //qDebug() << "QRcodeCreate receive data1111111111" << startPos << "    " << count;
            showCurrnetPage(startPos, count);
        }
    }
    else if(Type == InfoAddrExchange)       //设置结果返回
    {
        if(false == Map.contains(InfoExchangeParamResult))
           return;
        QVariant var = Map[InfoExchangeParamResult];
        stExchangeResult result = var.value<stExchangeResult>();

        if(result.type == InfoConfigTerminalQRCode)   //
        {
           StatusRemindWindow *statusDialog = NULL;
           if(result.result == 0)
               statusDialog = new StatusRemindWindow(this, 1, 5, QObject::tr("二维码生成失败"));
           else
		   {
				system("create_qrcode.sh &");
               statusDialog= new StatusRemindWindow(this, 0, 5, QObject::tr("二维码生成成功"));
		   }
           statusDialog->exec();
           delete statusDialog;
        }
    }
}
/**
 * @brief 根据当前页，计算在list中的开始位置及显示的数量
 */
int QRcodeCreate::findListPosition(int page, int &startPos, int &count)
{
    int currentShowNum = 0;
    if(totalNum == 0)
        return 0;
    if( (totalNum/10) >= page)
        currentShowNum = 10;
    else
        currentShowNum = totalNum%10;

    startPos = (page -1)*10;
    count = currentShowNum;

    return 1;
}
/**
 *table显示当前页配置数据
 */
void QRcodeCreate::showCurrnetPage(int startPos, int count)
{
    QTableWidget *table = ui->tableWidgetQRcode;
    for(int i=0; i<count; i++)
    {
        stQRcodeCreate terminalCode = codeCreatelist.at(startPos+i);
		//qDebug() << "(terminalCode.canAddr=" << terminalCode.canAddr << "i="  << startPos+i;
        table->item(i,0)->setText(QString::number(terminalCode.canAddr, 10));
		terminalCode.terminalCode[sizeof(terminalCode.terminalCode)-1] = 0;
		table->item(i,1)->setText(QString(terminalCode.terminalCode));
	}
    /*清空剩下行*/
    for(int i = count; i < table->rowCount(); i++)
    {
        table->item(i, 0)->setText(QObject::tr(""));
        table->item(i, 1)->setText(QObject::tr(""));
    }

	QString showLable = QObject::tr("总条数：") + QString::number(totalNum, 10) + QObject::tr(", ") + QObject::tr("共") + QString::number(totalPage, 10) + QObject::tr("页 ") + QObject::tr("当前第") + QString::number(currentPage, 10) + QObject::tr("页");
	ui->labelTotal->setText(showLable);
}
/**
 *保存当前table中的数据
 */
void QRcodeCreate::saveCurrnetPage(int startPos, int count)
{
    for(int i = 0; i < count; i++)
    {
        stQRcodeCreate &terminalCode = codeCreatelist.operator[](startPos+i);
        snprintf(terminalCode.terminalCode, sizeof(terminalCode.terminalCode), ui->tableWidgetQRcode->item(i, 1)->text().toLatin1().data());
    }
}
/**
 *上一页
 */
void QRcodeCreate::on_buttonUp_clicked()
{
    if(currentPage == 0)
        return;

    int startPos=0, count=0;
    if(findListPosition(currentPage, startPos, count))
        saveCurrnetPage(startPos,count);

    currentPage--;
    if(currentPage == 0)
        currentPage = totalPage;
    if(findListPosition(currentPage, startPos, count))
        showCurrnetPage(startPos, count);
}
/**
 *下一页
 */
void QRcodeCreate::on_buttonDown_clicked()
{
    int startPos=0, count=0;
    if(findListPosition(currentPage, startPos, count))
        saveCurrnetPage(startPos, count);

    currentPage++;
    if(currentPage > totalPage)
        currentPage = 1;
    if(findListPosition(currentPage, startPos, count))
        showCurrnetPage(startPos, count);
}
/**
 *保存
 */
void QRcodeCreate::on_buttonSave_clicked()
{
    if(currentPage == 0)
        return;
    int startPos=0, count=0;
    if(findListPosition(currentPage, startPos, count))
        saveCurrnetPage(startPos, count);

    stQRcodeCreate param;

	InfoProtocol infoPro;
    unsigned char tmpBuff[1024];
    unsigned char *point = tmpBuff + 1;
   
	tmpBuff[0] = codeCreatelist.size();
    for(int i=0; i<codeCreatelist.size();i++)
    {
        param = codeCreatelist.at(i);
        memcpy(point, (unsigned char *)&param, sizeof(stQRcodeCreate));
        point += sizeof(stQRcodeCreate);
    }
        //qDebug() << "QRcodeTerminal View  3 set size=" << codeCreatelist.size();
    infoPro.insert(InfoDataType,QByteArray(1,1));
    infoPro.insert(InfoConfigTerminalQRCode,QByteArray((char *)tmpBuff, point - tmpBuff));
    this->protocol->sendProtocolData(infoPro,InfoAddrConfig);
}
