#include <QDebug>

#include "OperateRecordInformation.h"
#include "ui_OperateRecordInformation.h"
#include "Common.h"
#include "SwapBase.h"
#include "InfoData.h"

OperateRecordInformation::OperateRecordInformation(QWidget *parent, CBus *bus, ProtocolBase *protocol) :
    QWidget(parent),
    ui(new Ui::OperateRecordInformation)
{
    ui->setupUi(this);
    ui->tableWidget->setColumnWidth(0,100);
    ui->tableWidget->setColumnWidth(1,210);
    ui->tableWidget->setColumnWidth(2,210);

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
    unsigned short tmp = InfoHistoryOperate;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoHistoryTotal, QByteArray( (char *)&tmp, 2));
    this->protocol->sendProtocolData(infoPro, InfoAddrHistory);

    createTableItem();
}

OperateRecordInformation::~OperateRecordInformation()
{
    this->bus->cancelRegistDev(this);
    delete ui;
}
/**
 *查询当前页数据
 */
void OperateRecordInformation::queryCurrentPage(int page)
{
    unsigned char currentPage = page;

    InfoProtocol infoPro;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoHistoryOperate, QByteArray( (char *)&currentPage, 1));
    this->protocol->sendProtocolData(infoPro, InfoAddrHistory);
}

void OperateRecordInformation::createTableItem()
{
    QTableWidget * table = ui->tableWidget;
    for(int i=0; i<HISTNUM_PER_PAGE; i++)
    {
        for(int k=0; k<3; k++)
        {
            QTableWidgetItem *tableItem = new QTableWidgetItem();
            tableItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            tableItem->setFlags(Qt::NoItemFlags);			//设置表格不能触摸
            table->setItem(i, k, tableItem);
        }
    }
}

/**
 *展现当前页记录
 */

void OperateRecordInformation::showCurrentPageRecord(stAllHistoryOperate record)
{
    QTableWidget * table= ui->tableWidget;
    QList<stHistoryOperate> *operateList =&record.operateList;

    int count = operateList->size();

/*    totalPage = totalNum/8;
    if(totalNum%8)
        totalPage += 1;
*/
    if(table->rowCount() < operateList->size())
        count = table->rowCount();

    QString showLable = QObject::tr("总条数：") + QString::number(totalNum, 10) + QObject::tr(", ") + QObject::tr("共") + QString::number(totalPage, 10) + QObject::tr("页 ") + QObject::tr("当前第") + QString::number(record.currentPage,10) + QObject::tr("页");

    ui->labelInfoShow->setText(showLable);

    int id = HISTNUM_PER_PAGE * (record.currentPage-1) + 1;
    for(int row=0; row <count; row++)
    {
        stHistoryOperate operateRecord = operateList->at(row);
        table->item(row, 0)->setText(QString::number(id++, 10));		//序号
        table->item(row, 1)->setText(QString(QObject::tr(operateRecord.operateTime)));//操作时间
        table->item(row, 2)->setText(QString(QObject::tr(operateRecord.operateType)));//操作类型
    }

    if(table->rowCount() > count)
    {
        for(int rowNum=count; rowNum < table->rowCount(); rowNum++)
        {
            for(int column=0; column <3; column++)
                table->item(rowNum, column)->setText(QString(""));
        }
    }
}
/**
 *bus 总线调用接口
 */
void OperateRecordInformation::receiveFromBus(InfoMap Map, InfoAddrType Type)
{
    emit sigFromBus(Map, Type);
}

void OperateRecordInformation::slotBusToOwn(InfoMap Map, InfoAddrType Type)
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
        else if (Map.contains(InfoHistoryOperate))  //当前页详细的数据
        {
            //qDebug() << "receive history operate...............";
            QVariant var = Map.value(InfoHistoryOperate);
            stAllHistoryOperate record = var.value<stAllHistoryOperate>();

            showCurrentPageRecord(record);
        }
    }
}
/**
 * @brief 点击下一页
 */
void OperateRecordInformation::on_pushButtonNext_clicked()
{
    currentPage++;
    if(currentPage > totalPage)
        currentPage = 1;
    queryCurrentPage(currentPage);
}
void OperateRecordInformation::on_pushButtonUp_clicked()
{
    currentPage--;
    if(currentPage == 0)
        currentPage = totalPage;

    queryCurrentPage(currentPage);
}
