#include <QDebug>
#include <qscrollbar.h>
#include "ChargingRecord.h"
#include "ui_ChargingRecord.h"

ChargingRecord::ChargingRecord(QWidget *parent, CBus *bus, ProtocolBase *protocol) :
    QWidget(parent),
    ui(new Ui::ChargingRecord)
{
    ui->setupUi(this);
     //this->setAttribute(Qt::WA_DeleteOnClose);
    //ui->tableChargeRecord->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    ui->tableChargeRecord->setColumnWidth(0,55);
    ui->tableChargeRecord->setColumnWidth(1,110);
    ui->tableChargeRecord->setColumnWidth(2,165);
    ui->tableChargeRecord->setColumnWidth(3,165);
    ui->tableChargeRecord->setColumnWidth(4,90);
    ui->tableChargeRecord->setColumnWidth(5,300);

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
	unsigned short tmp = InfoHistoryCharge;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoHistoryTotal, QByteArray( (char *)&tmp, 2));
    this->protocol->sendProtocolData(infoPro, InfoAddrHistory);

	createTableItem();
}


ChargingRecord::~ChargingRecord()
{
	this->bus->cancelRegistDev(this);
    delete ui;
}

/**
 *查询当前页数据
 */
void ChargingRecord::queryCurrentPage( int page)
{
	unsigned char currentPage = page;

    InfoProtocol infoPro;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoHistoryCharge, QByteArray( (char *)&currentPage, 1));
    this->protocol->sendProtocolData(infoPro, InfoAddrHistory);
}

void ChargingRecord::createTableItem()
{
	QTableWidget * table= ui->tableChargeRecord;
    table->horizontalScrollBar()->setStyleSheet("QScrollBar{height:26px;}"); //滚动条加粗
    for(int i=0; i<HISTNUM_PER_PAGE; i++)
	{
		//table->insertRow(i);
        for(int k=0; k<6; k++)
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
void ChargingRecord::showCurrentPageRecord(stAllHistoryCharge record)
{
	QTableWidget * table= ui->tableChargeRecord;
	QList<stHistoryCharge> *chargeList = &record.chargeList;

	int count = chargeList->size();
	
	if(table->rowCount() < chargeList->size())
		count = table->rowCount();

    QString showLable = QObject::tr("总条数：") + QString::number(totalNum, 10) + QObject::tr(", ") + QObject::tr("共") + QString::number(totalPage, 10) + QObject::tr("页 ") + QObject::tr("当前第") + QString::number(record.currentPage, 10) + QObject::tr("页");

	ui->labelInfoShow->setText(showLable);

	int id = HISTNUM_PER_PAGE * (record.currentPage-1) + 1; 
	for(int row=0; row <count; row++)
	{
		stHistoryCharge chargeRecord = chargeList->at(row);

        table->item(row, 0)->setText(QString::number(id++, 10));		//序号
        table->item(row, 1)->setText(QString::number(chargeRecord.canAddr,10));//can地址
		table->item(row, 2)->setText(QString(chargeRecord.startTime));
		table->item(row, 3)->setText(QString(chargeRecord.stopTime));
        table->item(row, 4)->setText(QString::number((double)chargeRecord.chargeEnergy/100, 'f',  2));
		table->item(row, 5)->setText(QObject::tr(chargeRecord.stopReson));
	}

    if(table->rowCount() > count)
    {
        for(int rowNum=count; rowNum < table->rowCount(); rowNum++)
        {
            for(int column=0; column <6; column++)
                table->item(rowNum, column)->setText(QString(""));
        }
    }
}

void ChargingRecord::receiveFromBus(InfoMap Map, InfoAddrType Type)
{
	emit sigFromBus(Map, Type);
}

void ChargingRecord::slotBusToOwn(InfoMap Map, InfoAddrType Type)
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
            QString showLable = QObject::tr("总条数：") + QString::number(totalNum, 10) + QObject::tr(", ") + QObject::tr("共") + QString::number(totalPage, 10) + QObject::tr("页 ") + QObject::tr("当前第") + QString::number(0, 10) + QObject::tr("页");
			ui->labelInfoShow->setText(showLable);

			queryCurrentPage(currentPage);
		}
		else if (Map.contains(InfoHistoryCharge))  //当前页详细的数据
		{
			QVariant var = Map.value(InfoHistoryCharge);
			stAllHistoryCharge record = var.value<stAllHistoryCharge>();

			showCurrentPageRecord(record);
		}
	}
}
/**
 * @brief 点击上一页
 */
void ChargingRecord::on_buttonUp_clicked()
{
	currentPage--;
	if(currentPage == 0)
		currentPage = totalPage;

	queryCurrentPage(currentPage);
}
/**
 * @brief 点击下一页
 */
void ChargingRecord::on_buttonNext_clicked()
{
	currentPage++;
	if(currentPage > totalPage)
		currentPage = 1;
	queryCurrentPage(currentPage);
}
