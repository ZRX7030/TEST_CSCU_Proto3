#include <QDebug>

#include "PeakChargingInformationView.h"
#include "StatusRemindWindow.h"
#include "ui_PeakChargingInformationView.h"

PeakChargingInformationView::PeakChargingInformationView(QWidget *parent, CBus *bus, ProtocolBase *protocol) :
    QWidget(parent),
    ui(new Ui::PeakChargingInformationView)
{
    ui->setupUi(this);

	this->bus = bus;
	this->protocol = protocol;
	/*注册数据*/
	QList<InfoAddrType> list;
	list.append(InfoAddrConfig);
    list.append(InfoAddrExchange);
	this->bus->registDev(this, list);

    createTableItems();

	QObject::connect(this,SIGNAL(sigFromBus(InfoMap, InfoAddrType)), this, SLOT(slotBusToOwn(InfoMap, InfoAddrType)), Qt::QueuedConnection);
    /*发送查询集控设置命令*/
    InfoProtocol infoPro;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoConfigTPFV, QByteArray());
    this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
}

PeakChargingInformationView::~PeakChargingInformationView()
{
    this->bus->cancelRegistDev(this);
    delete ui;
}
/**
 * @brief 单元格初始化
 * @param table
 */
void PeakChargingInformationView::tablseItemInit(QTableWidget *table)
{
    for(int i=0; i<table->rowCount(); i++)
    {
        // table->insertRow(i);
        for(int k=0; k<table->columnCount(); k++)
        {
            QTableWidgetItem *tableItem = new QTableWidgetItem();
            tableItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter );
            table->setItem(i, k, tableItem);
            table->item(i, k)->setText(QString::number(0,10));
        }
    }
}
/**
 * @brief 表格创建
 */
void PeakChargingInformationView::createTableItems()
{
    tablseItemInit(ui->tableWidgetTip);
    tablseItemInit(ui->tableWidgetPeak);
    tablseItemInit(ui->tableWidgetFlat);
    tablseItemInit(ui->tableWidgetValley);
}

/**
 * @brief PeakChargingInformationView::receiveFromBus
 * @param Map
 * @param Type
 */
void PeakChargingInformationView::receiveFromBus(InfoMap Map, InfoAddrType Type)
{
	emit sigFromBus(Map, Type);
}

/**
 * @brief PeakChargingInformationView::showTPFVTables
 * @param seg
 */
void PeakChargingInformationView::showTPFVTables(int seg)
{
    QTableWidget *table;
    QList<stTPFVParam> *tpfvParam;

    if(seg == 1)
    {
        tpfvParam = &tipList;
        table = ui->tableWidgetTip;
    }
    else if( seg ==2 )
    {
        tpfvParam = &peakList;
         table = ui->tableWidgetPeak;
    }
    else if( seg == 3 )
    {
        tpfvParam = &flatList;
        table = ui->tableWidgetFlat;
    }
    else if( seg == 4 )
    {
        tpfvParam = &valleyList;
        table = ui->tableWidgetValley;
    }
    else
        return;
//qDebug() << " PeakChargingInformationView receive data 3";
    int count = tpfvParam->size();
    if(tpfvParam->size() >  table->rowCount())
        count = table->rowCount();

    for(int row = 0; row < count; row++)
    {
        stTPFVParam param = tpfvParam->at(row);

        table->item(row, 0)->setText(QString::number(param.start_hour, 10));
        table->item(row, 1)->setText(QString::number(param.start_minute, 10));
        table->item(row, 2)->setText(QString::number(param.stop_hour, 10));
        table->item(row, 3)->setText(QString::number(param.stop_minute, 10));
        table->item(row, 4)->setText(QString::number(param.limit_soc, 10));
        table->item(row, 5)->setText(QString::number(param.limit_current, 10));
    }
}

void PeakChargingInformationView::slotBusToOwn(InfoMap Map, InfoAddrType Type)
{
   // qDebug() << " PeakChargingInformationView receive data11111111111111111\n";
	if(Type == InfoAddrConfig)
	{
		if(false == Map.contains(InfoConfigTPFV))
			return;
		
		QVariant var = Map.value(InfoConfigTPFV);
        stAllTPFVParam param = var.value<stAllTPFVParam>();

        QList<stTPFVParam> allList = param.tpfvList;
  //  qDebug() << " PeakChargingInformationView receive data\\\\\\\\\\\\\\\\\\\\" << param.peakCharegeEnable;
        if(param.peakCharegeEnable)
            ui->comboBoxPeakChargingOpenOrClose->setCurrentIndex(1);
        else
            ui->comboBoxPeakChargingOpenOrClose->setCurrentIndex(0);

        for(int i=0; i<allList.size(); i++)
        {
            stTPFVParam param = allList.at(i);
            if(param.time_seg == 1)
                tipList.append(param);
            else if(param.time_seg == 2)
                peakList.append(param);
            else if(param.time_seg == 3)
                flatList.append(param);
            else if(param.time_seg == 4)
                valleyList.append(param);
        }
//qDebug() << " PeakChargingInformationView receive data 2";
        showTPFVTables(1);
        showTPFVTables(2);
        showTPFVTables(3);
        showTPFVTables(4);
	}
	else if(Type == InfoAddrExchange)
    {
        if(false == Map.contains(InfoExchangeParamResult))
            return;
        QVariant var = Map.value(InfoExchangeParamResult);
        stExchangeResult result = var.value<stExchangeResult>();

        if(result.type == InfoConfigTPFV)  //back cscu config reuslt
        {
            StatusRemindWindow  *statusDialog = NULL;
            if(result.result == 0)
                statusDialog = new StatusRemindWindow(this, 1, 5, QObject::tr("错峰参数设置失败"));
            else
                statusDialog= new StatusRemindWindow(this, 0, 5, QObject::tr("错峰参数设置成功"));

            statusDialog->exec();
            delete statusDialog;
        }
    }
}

/**
 *点击了保存按钮
 */
void PeakChargingInformationView::on_buttonSave_clicked()
{
    //qDebug() << "PeakChargingInformationView  1";
    unsigned char tmp_buff[1024];

	InfoProtocol infoPro;
    stTPFVParam param;
    unsigned char seg=0;
    QList<stTPFVParam> allList;

    QTableWidget *table;
    for(int i=0; i<4; i++)
    {
        if(i == 0)
        {
            table = ui->tableWidgetTip;
            seg = 1;
        }
        else if(i == 1)
        {
            table = ui->tableWidgetPeak;
            seg = 2;
        }
        else if(i == 2)
        {
            table = ui->tableWidgetFlat;
            seg = 3;
        }
        else if(i == 3)
        {
            table = ui->tableWidgetValley;
            seg = 4;
        }

        for(int row=0; row < table->rowCount(); row ++)
        {    //qDebug() << "PeakChargingInformationView  2 i, row" << i << row;
            param.time_seg = seg;
            param.start_hour = table->item(row, 0)->text().toInt();
            param.start_minute = table->item(row, 1)->text().toInt();
            param.stop_hour = table->item(row, 2)->text().toInt();
            param.stop_minute = table->item(row, 3)->text().toInt();
            param.limit_soc = table->item(row, 4)->text().toInt();
            param.limit_current = table->item(row, 5)->text().toInt();

            allList.append(param);
            if(param.limit_soc > 100)      //add by songqb 2017-6-1
            {
                StatusRemindWindow  *statusDialog = NULL;
                statusDialog = new StatusRemindWindow(this, 1, 5, QObject::tr("SOC设置超过允许设置的最大值！"));
                statusDialog->exec();
                delete statusDialog;
                return;
            }
        }
    }

    unsigned char *point = tmp_buff +2;
    if(ui->comboBoxPeakChargingOpenOrClose->currentIndex() == 0)
        tmp_buff[0] = 0;
    else
         tmp_buff[0] = 1;
    tmp_buff[1] = allList.size();
    for(int i=0; i<allList.size(); i++)
    {
        param = allList.at(i);
        memcpy(point, (unsigned char *)&param, sizeof(stTPFVParam));
        point += sizeof(stTPFVParam);
    }
  //  qDebug() << "PeakChargingInformationView 33333333333333333333333333 3";
	infoPro.insert(InfoDataType, QByteArray(1, 1));
    infoPro.insert(InfoConfigTPFV, QByteArray((char *)&tmp_buff, point -tmp_buff));
	this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
}

/**
 *tab发生改变
 */
void PeakChargingInformationView::on_tabWidget_currentChanged(int index)
{

}
