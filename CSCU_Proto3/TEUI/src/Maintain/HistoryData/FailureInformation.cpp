#include <QDebug>
#include "FailureInformation.h"
#include "ui_FailureInformation.h"
#include <qscrollbar.h>

FailureInformation::FailureInformation(QWidget *parent, CBus *bus, ProtocolBase *protocol) :
    QWidget(parent),
    ui(new Ui::FailureInformation)
{
    ui->setupUi(this);
    ui->tableWidget->setColumnWidth(0,60);
    ui->tableWidget->setColumnWidth(1,90);
    ui->tableWidget->setColumnWidth(2,130);
    ui->tableWidget->setColumnWidth(3,130);
    ui->tableWidget->setColumnWidth(4,140);
    ui->tableWidget->setColumnWidth(7,300);
	this->bus = bus;
	this->protocol = protocol;
	
	totalNum = 0;
	totalPage = 0;
	currentPage = 1;

    /*注册数据*/
    QList<InfoAddrType> list;
    list.append(InfoAddrHistory);
    this->bus->registDev(this, list);

    QObject::connect(this,SIGNAL(sigFromBus(InfoMap, InfoAddrType)), this, SLOT(slotBusToOwn(InfoMap, InfoAddrType)));

	/*查询一共多少页*/	
    InfoProtocol infoPro;
	unsigned short tmp = InfoHistoryFault;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoHistoryTotal, QByteArray( (char *)&tmp, 2));
    this->protocol->sendProtocolData(infoPro, InfoAddrHistory);

	createTableItem();
}

FailureInformation::~FailureInformation()
{
	this->bus->cancelRegistDev(this);
    delete ui;
}

/**
 *查询当前页数据
 */
void FailureInformation::queryCurrentPage( int page)
{
	unsigned char currentPage = page;

    InfoProtocol infoPro;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoHistoryFault, QByteArray( (char *)&currentPage, 1));
    this->protocol->sendProtocolData(infoPro, InfoAddrHistory);
}

void FailureInformation::createTableItem()
{
	QTableWidget * table= ui->tableWidget;
    table->horizontalScrollBar()->setStyleSheet("QScrollBar{height:26px;}"); //滚动条加粗
    for(int i=0; i<HISTNUM_PER_PAGE; i++)
	{
		//table->insertRow(i);
        for(int k=0; k<9; k++)
		{
			QTableWidgetItem *tableItem = new QTableWidgetItem();
			tableItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter );
            tableItem->setFlags(Qt::NoItemFlags);			//设置表格不能触摸
			table->setItem(i, k, tableItem);
		}
	}
}
/**
 *展现当前页记录
 */
void FailureInformation::showCurrentPageRecord(stAllHistoryFault record)
{
	QTableWidget * table= ui->tableWidget;
	QList<stHistoryFault> *faultList = &record.faultList;

	int count = faultList->size();

    /*totalPage = totalNum/8;
    if(totalNum%8)
        totalPage += 1;
		*/
    //qDebug() << "total num, totalpage  faultList " << totalNum << "  " << totalPage << faultList->size();

    if(table->rowCount() < faultList->size())
		count = table->rowCount();

    QString showLable = QObject::tr("总条数：") + QString::number(totalNum, 10) + QObject::tr(", ") + QObject::tr("共") + QString::number(totalPage, 10) + QObject::tr("页 ") + QObject::tr("当前第") + QString::number(record.currentPage, 10) + QObject::tr("页");

	ui->labelInfoShow->setText(showLable);

	int id = HISTNUM_PER_PAGE * (record.currentPage-1) + 1; 
	for(int row=0; row <count; row++)
	{
		stHistoryFault faultRecord = faultList->at(row);

        table->item(row, 0)->setText(QString::number(id++, 10));		//序号
        table->item(row, 1)->setText(QString::number(faultRecord.canAddr, 10));//can地址
        table->item(row, 2)->setText(QString::number(faultRecord.inID, 10));
        table->item(row, 3)->setText(QString::number(faultRecord.minPDUAddr, 10));
        table->item(row, 4)->setText(QString::number(faultRecord.maxPDUAddr, 10));
        table->item(row, 5)->setText(QString(faultRecord.startTime));
        table->item(row, 6)->setText(QString(faultRecord.stopTime));
        table->item(row, 7)->setText(QObject::tr(faultRecord.faultReson));
        if(faultRecord.property == 1)
            table->item(row, 8)->setText(QString(QObject::tr("终端故障")));
        else
            table->item(row, 8)->setText(QString(QObject::tr("设备故障")));
	}

    if(table->rowCount() > count)
    {
        for(int rowNum=count; rowNum < table->rowCount(); rowNum++)
        {
            for(int column=0; column <9; column++)
                table->item(rowNum, column)->setText(QString(""));
        }
    }
}
/**
 *bus 总线调用接口
 */
void FailureInformation::receiveFromBus(InfoMap Map, InfoAddrType Type)
{
	emit sigFromBus(Map, Type);
}

void FailureInformation::slotBusToOwn(InfoMap Map, InfoAddrType Type)
{
	if(Type == InfoAddrHistory)
	{
		if(Map.contains(InfoHistoryTotal))   //一共多少数据
		{
			QVariant var = Map.value(InfoHistoryTotal);
			stHistoryInfo info = var.value<stHistoryInfo>();
			totalNum = info.totalNum;
			totalPage = info.totalPage;

			currentPage = totalPage;
            QString showLable = QObject::tr("总条数：") + QString::number(totalNum, 10) + QObject::tr(", ") + QObject::tr("共") + QString::number(totalPage, 10) + QObject::tr("页 ") + QObject::tr("当前第") + QString::number(0, 0) + QObject::tr("页");
			
			ui->labelInfoShow->setText(showLable);

			queryCurrentPage(currentPage);
		}
		else if (Map.contains(InfoHistoryFault))  //当前页详细的数据
		{
			QVariant var = Map.value(InfoHistoryFault);
			stAllHistoryFault record = var.value<stAllHistoryFault>();

			showCurrentPageRecord(record);
		}
	}
}

/**
 * @brief 点击下一页
 */

void FailureInformation::on_pushButtonNextPage_clicked()
{
	currentPage++;
	if(currentPage > totalPage)
		currentPage = 1;
    queryCurrentPage(currentPage);
}
/**
 * @brief 点击上一页
 */

void FailureInformation::on_pushButtonPreviousPage_clicked()
{
	currentPage--;
	if(currentPage == 0)
		currentPage = totalPage;

	queryCurrentPage(currentPage);
}
