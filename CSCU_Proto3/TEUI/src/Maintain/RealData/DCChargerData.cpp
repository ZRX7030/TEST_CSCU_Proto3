#include <QDebug>

#include "DCChargerData.h"
#include "ui_DCChargerData.h"
#include <qscrollbar.h>

#define				PAGE_SIZE		8

DCChargerData::DCChargerData(QWidget *parent, CBus *bus, ProtocolBase *protocol) :
    QWidget(parent),
    ui(new Ui::DCChargerData)
{
    ui->setupUi(this);
    createTableItems();

    this->bus = bus;
    this->protocol = protocol;

	totalNum = 0;
	currentPage = 0;
    itemCount = 0;
	currentPageItemCount = 0 ;
    queryPositon = 0;

    /*注册数据*/
    QList<InfoAddrType> list;
    list.append(InfoAddrReal);
    list.append(InfoAddrConfig);
    this->bus->registDev(this, list);
    
	timerQuery = new QTimer();
    timerQuery->setInterval(1000);
    QObject::connect(timerQuery, SIGNAL(timeout()), this, SLOT(slotQueryTerminal()));
    
	QObject::connect(this,SIGNAL(sigFromBus(InfoMap, InfoAddrType)), this, SLOT(slotBusToOwn(InfoMap, InfoAddrType)));
    
    timerQuery->start();

    /*查询地址参数*/
    InfoProtocol infoPro;
    int index = ui->tabWidgetDCInformation->currentIndex();
    unsigned char param;
	if(index == 0)
		param = 3;
	else if(index == 1)
		param = 2;
	else if(index == 2)
		param = 1;

    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoConfigDCMPCNum, QByteArray((char *)&param, 1));
    this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
}

DCChargerData::~DCChargerData()
{
    timerQuery->stop();
	delete timerQuery;
    this->bus->cancelRegistDev(this);
	delete ui;
}
    
/**
 * @brief 表格创建
 */
void DCChargerData:: createTableItems()
{
    tablseItemInit(ui->tableWidgetDcModuleDate);
    tablseItemInit(ui->tableWidgetPDUDate);
    tablseItemInit(ui->tableWidgetCCUDate);
    settablecolumncount();
}
/**
 *数据清空
 */
void DCChargerData::clearCurrentTable(QTableWidget *table)
{
	for(int row = 0; row < table->rowCount(); row++)
		for(int column = 0; column < table->columnCount(); column++)
			table->item(row, column)->setText("");
}
/**
 * @brief 单元格初始化
 * @param table
 */
void DCChargerData::tablseItemInit(QTableWidget *table)   //by songqb
{
	table->horizontalScrollBar()->setStyleSheet("QScrollBar{height:26px;}"); //滚动条加粗
	for(int x=0; x<table->rowCount(); x++)
	{
		for(int y=0; y<table->columnCount(); y++)
		{
			QTableWidgetItem *tableItem = new QTableWidgetItem();
			tableItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter );
            tableItem->setFlags(Qt::NoItemFlags);			//设置表格不能触摸
			table->setItem(x, y, tableItem);
			//table->item(x, y)->setText(QString(""));
        }
    }
}
void DCChargerData::settablecolumncount()  //by songqb 2017-4-28
{
    //直流模块表格设置不同列宽
    QTableWidget *tableDcModule = ui->tableWidgetDcModuleDate;
    for(int i=0; i<tableDcModule->rowCount(); i++)
    {
        for(int m=0; m<tableDcModule->columnCount(); m++)
        {
            ui->tableWidgetDcModuleDate->setColumnWidth(0,90);
            ui->tableWidgetDcModuleDate->setColumnWidth(1,130);
            ui->tableWidgetDcModuleDate->setColumnWidth(2,130);
            ui->tableWidgetDcModuleDate->setColumnWidth(3,130);
            ui->tableWidgetDcModuleDate->setColumnWidth(4,130);
            ui->tableWidgetDcModuleDate->setColumnWidth(5,180);
            ui->tableWidgetDcModuleDate->setColumnWidth(6,180);
            ui->tableWidgetDcModuleDate->setColumnWidth(7,180);
            ui->tableWidgetDcModuleDate->setColumnWidth(8,150);
            ui->tableWidgetDcModuleDate->setColumnWidth(9,110);
            ui->tableWidgetDcModuleDate->setColumnWidth(10,260);
            ui->tableWidgetDcModuleDate->setColumnWidth(11,160);
            ui->tableWidgetDcModuleDate->setColumnWidth(12,160);
            ui->tableWidgetDcModuleDate->setColumnWidth(13,160);
            ui->tableWidgetDcModuleDate->setColumnWidth(14,160);
        }
    }
    //PDU模块表格设置不同列宽
    QTableWidget *tablePDU = ui->tableWidgetPDUDate;
    for(int j=0; j<tablePDU->rowCount(); j++)
    {
        for(int n=0; n<tablePDU->columnCount(); n++)
        {
            ui->tableWidgetPDUDate->setColumnWidth(0,90);
            ui->tableWidgetPDUDate->setColumnWidth(1,130);
            ui->tableWidgetPDUDate->setColumnWidth(2,130);
            ui->tableWidgetPDUDate->setColumnWidth(3,130);
            ui->tableWidgetPDUDate->setColumnWidth(4,170);
            ui->tableWidgetPDUDate->setColumnWidth(5,110);
            ui->tableWidgetPDUDate->setColumnWidth(6,260);
            ui->tableWidgetPDUDate->setColumnWidth(7,160);
            ui->tableWidgetPDUDate->setColumnWidth(8,160);
            ui->tableWidgetPDUDate->setColumnWidth(9,160);
            ui->tableWidgetPDUDate->setColumnWidth(10,160);
        }
    }
    //CCU模块表格设置不同列宽
    QTableWidget *tableCCU = ui->tableWidgetCCUDate;
    for(int k=0; k<tableCCU->rowCount(); k++)
    {
        for(int p=0; p<tableCCU->columnCount(); p++)
        {
            ui->tableWidgetCCUDate->setColumnWidth(0,70);
            ui->tableWidgetCCUDate->setColumnWidth(1,130);
            ui->tableWidgetCCUDate->setColumnWidth(2,150);
            ui->tableWidgetCCUDate->setColumnWidth(3,120);
            ui->tableWidgetCCUDate->setColumnWidth(4,110);
            ui->tableWidgetCCUDate->setColumnWidth(5,260);
            ui->tableWidgetCCUDate->setColumnWidth(6,160);
            ui->tableWidgetCCUDate->setColumnWidth(7,160);
            ui->tableWidgetCCUDate->setColumnWidth(8,160);
            ui->tableWidgetCCUDate->setColumnWidth(9,160);
        }
    }
}
/**
 *直流模块数据显示
 */

void DCChargerData::showModuleInfo(stDCModuleRealData module)
{
    if(ui->tableWidgetDcModuleDate->columnCount() != 15)
        return;
    //qDebug() << "showModule info item///////////..............///////////=" << itemCount << "currentPage item=" << currentPageItemCount<< "module id is ...."<<module.id;
    ui->tableWidgetDcModuleDate->item(itemCount, 0)->setText(QString::number(module.canAddr, 10)+"/"+QString::number(module.id, 10)); //nihai 增加CCU ID
    //qDebug() << "curent111111111111111111111111111 "<< module.out_current/100;
   // ui->tableWidgetDcModuleDate->item(itemCount, 1)->setText(QString::number(module.work_status, 10));
    ui->tableWidgetDcModuleDate->item(itemCount, 2)->setText(QString::number((double)module.out_volatge/10 , 'f',  1)+"V");
    ui->tableWidgetDcModuleDate->item(itemCount, 3)->setText(QString::number((double)(qAbs(module.out_current))/100 , 'f',  2)+"A");
   // ui->tableWidgetDcModuleDate->item(itemCount, 4)->setText(QString::number((double)module.m1_tempeture/10 , 'f',  1));
    ui->tableWidgetDcModuleDate->item(itemCount, 4)->setText(QString::number(module.m1_tempeture, 10)+QObject::tr("度"));
    ui->tableWidgetDcModuleDate->item(itemCount, 5)->setText(QString::number((double)module.in_a_volatge/10, 'f', 1)+"V");
    ui->tableWidgetDcModuleDate->item(itemCount, 6)->setText(QString::number((double)module.in_b_volatge/10, 'f', 1)+"V");
    ui->tableWidgetDcModuleDate->item(itemCount, 7)->setText(QString::number((double)module.in_c_volatge/10, 'f', 1)+"V");
    ui->tableWidgetDcModuleDate->item(itemCount, 8)->setText(QString::number(module.module_group, 10));
    ui->tableWidgetDcModuleDate->item(itemCount, 9)->setText(QString::number(module.warning_status, 10));
    ui->tableWidgetDcModuleDate->item(itemCount, 10)->setText(QString(module.seq));
    ui->tableWidgetDcModuleDate->item(itemCount, 11)->setText(QString(module.soft_version1));
    ui->tableWidgetDcModuleDate->item(itemCount, 12)->setText(QString(module.soft_version2));
    ui->tableWidgetDcModuleDate->item(itemCount, 13)->setText(QString(module.soft_version3));
    ui->tableWidgetDcModuleDate->item(itemCount, 14)->setText(QString(module.hard_version));
    if(module.work_status == 0)
        ui->tableWidgetDcModuleDate->item(itemCount, 1)->setText(QString(QObject::tr("待机")));
    else if(module.work_status == 1)
        ui->tableWidgetDcModuleDate->item(itemCount, 1)->setText(QString(QObject::tr("休眠")));
    else if(module.work_status == 2)
        ui->tableWidgetDcModuleDate->item(itemCount, 1)->setText(QString(QObject::tr("运行")));
    else if(module.work_status == 3)
        ui->tableWidgetDcModuleDate->item(itemCount, 1)->setText(QString(QObject::tr("离线")));

	itemCount ++;
    //qDebug() << "module info end item="<< itemCount;
    if(itemCount >= currentPageItemCount)
	{
		for(int row = itemCount; row<PAGE_SIZE; row++)
			for(int column = 0; column < ui->tableWidgetDcModuleDate->columnCount(); column ++)
				ui->tableWidgetDcModuleDate->item(row, column)->setText("");
        itemCount = 0;
	}
}

/**
 *pdu数据显示
 */
void DCChargerData::showPDUInfo(stDCPduRealData pdu)
{
    if(ui->tableWidgetPDUDate->columnCount() != 11)
        return;
    ui->tableWidgetPDUDate->item(itemCount, 0)->setText(QString::number(pdu.canAddr, 10)+"/"+QString::number(pdu.id, 10));
    //ui->tableWidgetPDUDate->item(itemCount, 1)->setText(QString::number(pdu.work_status, 10));
    ui->tableWidgetPDUDate->item(itemCount, 2)->setText(QString::number((double)pdu.out_volatge/10 , 'f',  1)+"V");
    ui->tableWidgetPDUDate->item(itemCount, 3)->setText(QString::number((double)(qAbs(pdu.out_current))/10 , 'f',  2)+"A");// mod by yanwei 20170912, 100=>10
    //ui->tableWidgetPDUDate->item(itemCount, 4)->setText(QString::number((double)pdu.tempeture/10, 'f', 1));
    ui->tableWidgetPDUDate->item(itemCount, 4)->setText(QString::number(pdu.tempeture, 10)+QObject::tr("度"));
    ui->tableWidgetPDUDate->item(itemCount, 5)->setText(QString::number(pdu.warning_status, 10));
    ui->tableWidgetPDUDate->item(itemCount, 6)->setText(QString(pdu.seq));
    ui->tableWidgetPDUDate->item(itemCount, 7)->setText(QString(pdu.soft_version1));
    ui->tableWidgetPDUDate->item(itemCount, 8)->setText(QString(pdu.soft_version2));
    ui->tableWidgetPDUDate->item(itemCount, 9)->setText(QString(pdu.soft_version3));
    ui->tableWidgetPDUDate->item(itemCount, 10)->setText(QString(pdu.hard_version));
    if(pdu.work_status == 0)
        ui->tableWidgetPDUDate->item(itemCount, 1)->setText(QString(QObject::tr("待机")));
    else if(pdu.work_status == 1)
        ui->tableWidgetPDUDate->item(itemCount, 1)->setText(QString(QObject::tr("休眠")));
    else if(pdu.work_status == 2)
        ui->tableWidgetPDUDate->item(itemCount, 1)->setText(QString(QObject::tr("离线")));
    else if(pdu.work_status == 3)
        ui->tableWidgetPDUDate->item(itemCount, 1)->setText(QString(QObject::tr("枪已连接")));
    else if(pdu.work_status == 4)
        ui->tableWidgetPDUDate->item(itemCount, 1)->setText(QString(QObject::tr("启动中")));
    else if(pdu.work_status == 5)
        ui->tableWidgetPDUDate->item(itemCount, 1)->setText(QString(QObject::tr("充电中")));
    else if(pdu.work_status == 6)
        ui->tableWidgetPDUDate->item(itemCount, 1)->setText(QString(QObject::tr("充电结束")));

    itemCount ++;
    if(itemCount >= currentPageItemCount)
	{
		for(int row = itemCount; row<PAGE_SIZE; row++)
			for(int column = 0; column < ui->tableWidgetPDUDate->columnCount(); column ++)
				ui->tableWidgetPDUDate->item(row, column)->setText("");

        itemCount = 0;
	}
}


/**
 *ccu数据显示
 */
void DCChargerData::showCCUInfo(stDCCcuRealData ccu)
{
    //qDebug() << "show ccu info step1....." << ui->tableWidgetCCUDate->columnCount();
    if(ui->tableWidgetCCUDate->columnCount() != 10)
        return;
    //qDebug() << "show ccu info step2.....ccu temp=" << ccu.tempeture;
    ui->tableWidgetCCUDate->item(itemCount, 0)->setText(QString::number(ccu.id, 10));
    //ui->tableWidgetCCUDate->item(itemCount, 1)->setText(QString::number(ccu.work_status, 10));
    ui->tableWidgetCCUDate->item(itemCount, 2)->setText(QString::number(ccu.tempeture, 10)+QObject::tr("度"));
    ui->tableWidgetCCUDate->item(itemCount, 3)->setText(QString::number((double)ccu.out_power/10 , 'f',  1)+"kW");
    ui->tableWidgetCCUDate->item(itemCount, 4)->setText(QString::number(ccu.warning_status, 10));
    ui->tableWidgetCCUDate->item(itemCount, 5)->setText(QString(ccu.seq));
    ui->tableWidgetCCUDate->item(itemCount, 6)->setText(QString(ccu.soft_version1));
    ui->tableWidgetCCUDate->item(itemCount, 7)->setText(QString(ccu.soft_version2));
    ui->tableWidgetCCUDate->item(itemCount, 8)->setText(QString(ccu.soft_version3));
    ui->tableWidgetCCUDate->item(itemCount, 9)->setText(QString(ccu.hard_version));
    if(ccu.work_status == 0)
        ui->tableWidgetCCUDate->item(itemCount, 1)->setText(QString(QObject::tr("待机")));
    else if(ccu.work_status == 1)
        ui->tableWidgetCCUDate->item(itemCount, 1)->setText(QString(QObject::tr("休眠")));
    else if(ccu.work_status == 2)
        ui->tableWidgetCCUDate->item(itemCount, 1)->setText(QString(QObject::tr("运行")));
    else if(ccu.work_status == 3)
        ui->tableWidgetCCUDate->item(itemCount, 1)->setText(QString(QObject::tr("离线")));
    else if(ccu.work_status == 4)
        ui->tableWidgetCCUDate->item(itemCount, 1)->setText(QString(QObject::tr("升级中")));

	itemCount ++;
    if(itemCount >= currentPageItemCount)
	{
		for(int row = itemCount; row<PAGE_SIZE; row++)
			for(int column = 0; column < ui->tableWidgetCCUDate->columnCount(); column ++)
				ui->tableWidgetCCUDate->item(row, column)->setText("");

        itemCount = 0;
	}
}


void DCChargerData::slotBusToOwn(InfoMap Map, InfoAddrType Type)
{
	if(Type == InfoAddrReal)
	{
		if(Map.contains(InfoRealDCModule))		//直流模块数据
		{
           stDCModuleRealData data = Map[InfoRealDCModule].value<stDCModuleRealData>();
            showModuleInfo(data);
		}
		else if(Map.contains(InfoRealDCPdu)	)			//直流pdu数据
		{
            stDCPduRealData data = Map[InfoRealDCPdu].value<stDCPduRealData>();
            showPDUInfo(data);
		}
		else if(Map.contains(InfoRealDCCcu)	)		//直流ccu数据
		{
            stDCCcuRealData data = Map[InfoRealDCCcu].value<stDCCcuRealData>();
            showCCUInfo(data);
		}
	}
	else if(Type == InfoAddrConfig)         //pdu、module、ccu 数量参数
	{
		if (Map.contains(InfoConfigDCMPCNum))
		{
			QVariant var = Map.value(InfoConfigDCMPCNum);
			addrParam = var.value<stAllDCChargerTypeNum>();

			totalNum = addrParam.listNum.size();
            //qDebug() << "now the totalNum is............/.........." <<totalNum;
			totalPage = totalNum /PAGE_SIZE;
			leftNum = totalNum % PAGE_SIZE;
			if(leftNum)
                totalPage++;
            currentPage = 1;
            //qDebug() << "total num, total page, left num, " << totalNum << " " << totalPage << " " << leftNum;
            currentPageInit(currentPage);
//            for(int i=0; i< addrParam.listNum.size(); i++)
//            {
//                stDCChargerTypeNum addr =  addrParam.listNum.at(i);
//                qDebug() << "now canid an inline id is@@@@@@@@@@@@@@@/@@@@@@@@@@" <<addr.canAddr <<"   "<<addr.id;
//            }
		}
	}
}
void DCChargerData::receiveFromBus(InfoMap Map, InfoAddrType type)
{
	emit sigFromBus(Map, type);
}
/**
 *定时查询终端参数
 */
void DCChargerData::slotQueryTerminal()
{
    if(queryCanList.size() == 0)
        return;
    if(queryCanList.size()<= queryPositon)
        queryPositon = 0;
    //qDebug() << "slot timer query 1...............";

		InfoBodyType type  = InfoBodyNone;
        stDCChargerTypeNum addr =  queryCanList.at(queryPositon);
		InfoProtocol infoPro;

		if(addrParam.type == 3)
			type = InfoRealDCModule;			//直流模块数据
		else if(addrParam.type == 2)
			type = InfoRealDCPdu; 				//直流pdu数据
		else if(addrParam.type == 1)
			type = InfoRealDCCcu;				//直流ccu数据

		infoPro.insert(InfoDataType, QByteArray(1, 0));
		infoPro.insert(type, QByteArray((char *)&addr, sizeof(stDCChargerTypeNum)));
		this->protocol->sendProtocolData(infoPro, InfoAddrReal);
        queryPositon++;
        //qDebug() << "slot timer query 2...............";
}
/**
 * 当前页初始化
 */
void DCChargerData::currentPageInit(int page)
{
	ui->labelShowTotal->setText(QObject::tr("总条数：") + QString::number(totalNum, 10)+ QObject::tr(", 共")+ QString::number(totalPage, 10) + QObject::tr("页 当前第")+QString::number(currentPage, 10)+QObject::tr("页"));
	queryCanList.clear();
    itemCount = 0;
    queryPositon = 0;
	if(totalNum == 0)
		return;

	int start = (page - 1)*PAGE_SIZE + 1;
	int end = start + PAGE_SIZE;

	if(page == totalPage && leftNum)
		end = start + leftNum;
   // qDebug() << "start end" << start << " " << end << "...............";

    for(int i = start; i< end; i++)
        queryCanList.append(addrParam.listNum.at(i-1));
	
	currentPageItemCount = end - start;				//记录当前也显示多少条数据
}
/**
 *点击上一页
 */
void DCChargerData::on_buttonUp_clicked()
{
	currentPage--;
    if(currentPage == 0)
        currentPage = totalPage;

	if(totalNum == 0)
		currentPage = 1;

    currentPageInit(currentPage);
}
/**
 *点击下一页
 */
void DCChargerData::on_buttonDown_clicked()
{
  currentPage++;
    if(currentPage > totalPage)
        currentPage = 1;

    currentPageInit(currentPage);
}
/**
 *当前标签改变, 查询地址数量
 */
void DCChargerData::on_tabWidgetDCInformation_currentChanged(int index)
{
	unsigned char param;
	InfoProtocol infoPro;
	
	queryCanList.clear();
    queryPositon =0;
    itemCount = 0;
	
	if(index == 0)
	{
		clearCurrentTable(ui->tableWidgetDcModuleDate);
		param = 3;
	}
	else if(index == 1)
	{
		clearCurrentTable(ui->tableWidgetPDUDate);
		param = 2;
	}
	else if(index == 2)
	{
		clearCurrentTable(ui->tableWidgetCCUDate);
		param = 1;
	}

    //qDebug() << "DCChargerData query..............param=" << param;
	infoPro.insert(InfoDataType, QByteArray(1, 0));
	infoPro.insert(InfoConfigDCMPCNum, QByteArray((char *)&param, 1));
	this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
}
