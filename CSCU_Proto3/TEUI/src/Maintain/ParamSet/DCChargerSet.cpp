#include <QComboBox>
#include <QDebug>
#include <qscrollbar.h>

#include "DCChargerSet.h"
#include "ui_DCChargerSet.h"
#include "StatusRemindWindow.h"

#define			PAGE_SIZE				8

DCChargerSet::DCChargerSet(QWidget *parent, CBus *bus, ProtocolBase *protocol) :
    QWidget(parent),
    ui(new Ui::DCChargerSet)
{
    //qDebug() << "///////////////// .........1";
    ui->setupUi(this);
    createTableItems();
    setMonitorcolumn();

    terminalTableInit();

    this->bus = bus;
    this->protocol = protocol;
    
    //qDebug() << "///////////////// .........2";
	totalNum = 0;
    totalPage = 0;
	currentPage = 0;
	leftNum = 0;
    /*注册数据*/
    QList<InfoAddrType> list;
    list.append(InfoAddrConfig);
    list.append(InfoAddrExchange);
    this->bus->registDev(this, list);

//    timerInit = new QTimer();
//    timerInit->setInterval(500);
//	timerInit->setSingleShot(true);
//    QObject::connect(timerInit, SIGNAL(timeout()), this, SLOT(timerOver()));

    QObject::connect(this,SIGNAL(sigFromBus(InfoMap, InfoAddrType)), this, SLOT(slotBusToOwn(InfoMap, InfoAddrType)), Qt::QueuedConnection);

//    timerInit->start();
    
	/*发送查询直流机设置命令*/
    InfoProtocol infoPro;
    int index = ui->tabWidgetChargerSet->currentIndex();
    unsigned char param = (unsigned char)(index + 1);
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoConfigDCChargerTypeNum, QByteArray((char *)&param, 1));
    //qDebug() << "gaozai  end .........1";
    this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
    //qDebug() << "gaozai  end .........";
}

DCChargerSet::~DCChargerSet()
{
//	delete timerInit;
    this->bus->cancelRegistDev(this);
    delete ui;
}
/**
 *超时，初始化终端数据表
 */
void DCChargerSet::timerOver()
{
	terminalTableInit();
}
void DCChargerSet::setMonitorcolumn()
{
    ui->tableWidgetMonitiorParam->setColumnWidth(0,100);
    ui->tableWidgetMonitiorParam->setColumnWidth(1,100);
    ui->tableWidgetMonitiorParam->setColumnWidth(2,133);
    ui->tableWidgetMonitiorParam->setColumnWidth(3,172);
}
/**
 * @brief 单元格初始化
 * @param table
 */
void DCChargerSet::tablseItemInit(QTableWidget *table)
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
            ui->tableWidgetTermParam->horizontalScrollBar()->setStyleSheet("QScrollBar{height:26px;}");   //设置滚动条宽度
        }
    }
}
void DCChargerSet::clearTerminalInfor()      //清空终端参数数据　　add by songqb  2017-6-8
{
    QTableWidget *table = ui->tableWidgetTermParam;
    for(int i=0; i<table->rowCount(); i++)
    {
        for(int k=0; k<table->columnCount(); k++)
        {
            if(k < 4)
                table->item(i, k)->setText(QString(""));
            else
            {
                QComboBox *combox =(QComboBox *)table->cellWidget(i,k);  //获得widget
                combox->setCurrentIndex(0);
            }
         }
     }
}
void DCChargerSet::clearMonitorInfor()      //清空监控参数数据　　　add by songqb  2017-6-8
{
    QTableWidget *table = ui->tableWidgetMonitiorParam;
    for(int i=0; i<table->rowCount(); i++)
    {
        for(int k=0; k<table->columnCount(); k++)
        {
            table->item(i, k)->setText(QString(""));
        }
    }
}
/**
 *终端参数表格初始化
 */
void DCChargerSet::terminalTableInit(void)
{
	QTableWidget *tableTermParam = ui->tableWidgetTermParam;
    ui->tableWidgetTermParam->setColumnWidth(2,130);
	for(int i=0; i<tableTermParam->rowCount(); i++)
	{
		for(int m=0; m<tableTermParam->columnCount(); m++)
		{
			QComboBox *comboxTermParam= new QComboBox(); // 下拉选择框控件
			switch (m) 
			{
				case 4:
					{
						comboxTermParam->addItem(QObject::tr(""));
						comboxTermParam->addItem(QObject::tr("轮充"));
						comboxTermParam->addItem(QObject::tr("群充"));
						tableTermParam->setCellWidget(i, m, (QWidget*)comboxTermParam);
					}break;
				case 5:
					{
						comboxTermParam->addItem(QObject::tr(""));
						comboxTermParam->addItem(QObject::tr("模式A"));
						comboxTermParam->addItem(QObject::tr("模式B"));
						comboxTermParam->addItem(QObject::tr("模式C"));
						tableTermParam->setCellWidget(i, m, (QWidget*)comboxTermParam);
					}break;
				case 6:
					{
						comboxTermParam->addItem(QObject::tr(""));
						comboxTermParam->addItem(QObject::tr("12V"));
						comboxTermParam->addItem(QObject::tr("24V"));
						tableTermParam->setCellWidget(i, m, (QWidget*)comboxTermParam);
					}break;
				case 9:
					{
						comboxTermParam->addItem(QObject::tr(""));
						comboxTermParam->addItem(QObject::tr("电平式"));
						comboxTermParam->addItem(QObject::tr("脉冲式"));
						tableTermParam->setCellWidget(i, m, (QWidget*)comboxTermParam);
					}break;
				case 7:
				case 8:
				case 10:
					{
						comboxTermParam->addItem(QObject::tr(""));
						comboxTermParam->addItem(QObject::tr("否"));
						comboxTermParam->addItem(QObject::tr("是"));
						tableTermParam->setCellWidget(i, m, (QWidget*)comboxTermParam);
					}break;
				case 11:
					{
						comboxTermParam->addItem(QObject::tr(""));
                        comboxTermParam->addItem(QObject::tr("新老兼容"));
                        comboxTermParam->addItem(QObject::tr("旧国标"));
                        comboxTermParam->addItem(QObject::tr("GB27930_2015"));  //nihai modify
						tableTermParam->setCellWidget(i, m, (QWidget*)comboxTermParam);
					}break;

				default:
					break;
			}
		}
	}
}
/**
 * @brief 表格创建
 */
void DCChargerSet::createTableItems()   //by songqb 2017-4-18
{
	tablseItemInit(ui->tableWidgetTermParam);
	tablseItemInit(ui->tableWidgetMonitiorParam);
}


/**
 * @brief DCChargerSet::receiveFromBus
 * @param Map
 * @param Type
 */
void DCChargerSet::receiveFromBus(InfoMap Map, InfoAddrType Type)
{
	emit sigFromBus(Map, Type);
}
void DCChargerSet::slotBusToOwn(InfoMap Map, InfoAddrType Type)
{
    if(Type == InfoAddrConfig)      //收到配置数据
    {
		//地址参数数据
		if( Map.contains(InfoConfigDCChargerTypeNum))			
		{
			QVariant var = Map.value(InfoConfigDCChargerTypeNum);
			stDCChargerDeviceNum  tmpList = var.value<stDCChargerDeviceNum>();
			deviceAddrList = tmpList;
		
            //qDebug() << "............tmplist size is=" << tmpList.addrList.size() << " deviceAddr list addr=" << deviceAddrList.addrList.at(0);
			totalNum = deviceAddrList.addrList.size();
			totalPage = totalNum/8;
			leftNum = totalNum % PAGE_SIZE;
			if(leftNum)
				totalPage++;
			currentPage = 1;

			currentPageInit(currentPage);
		}
		//监控数据（ccu）
		else if( Map.contains(InfoConfigDCChargerMonitor))				
		{
            //qDebug() << "DCCChargerSet receive data..................monitor";
			QVariant var = Map.value(InfoConfigDCChargerMonitor);
			stDCChargerMonitorParam monData = var.value<stDCChargerMonitorParam>();
			showMonitorData(monData);	
		}
		//终端数据
		else if( Map.contains(InfoConfigDCChargerTerm))			
		{
			QVariant var = Map.value(InfoConfigDCChargerTerm);
			stDCChargerTermParam termData = var.value<stDCChargerTermParam>();
			showTermData(termData);	
            //qDebug() << "DCCChargerSet receive data..................terminal " << termData.canAddr;
		}
	}
	else if(Type == InfoAddrExchange) //设置结果返回
	{
		if(false == Map.contains(InfoExchangeParamResult))
			return;
		QVariant var = Map[InfoExchangeParamResult];
		stExchangeResult result = var.value<stExchangeResult>();
		StatusRemindWindow *statusDialog = NULL;
		if(result.type == InfoConfigDCChargerTerm)
		{
			if(result.result ==0)
				statusDialog = new StatusRemindWindow(this, 1, 5, QObject::tr("终端参数设置失败!"));
			else
				statusDialog= new StatusRemindWindow(this, 0, 5, QObject::tr("终端参数设置成功!"));
		}
		else if(result.type == InfoConfigDCChargerMonitor)
		{
			if(result.result ==0)
				statusDialog = new StatusRemindWindow(this, 1, 5, QObject::tr("监控参数设置失败!"));
			else
				statusDialog= new StatusRemindWindow(this, 0, 5, QObject::tr("监控参数设置成功!"));
		}

		statusDialog->exec();
		delete statusDialog;
	}
}

/**
 *当前页初始化, 主要查询数据跟新table
 */
void DCChargerSet::currentPageInit(int page)
{
	ui->labelShowTotal->setText(QObject::tr("总条数：") + QString::number(totalNum, 10)+ QObject::tr(", 共")+ QString::number(totalPage, 10) + QObject::tr("页 当前第")+QString::number(currentPage, 10)+QObject::tr("页"));
	if(totalNum == 0)
		return;

	int start = (page - 1)*PAGE_SIZE + 1;
	int end = start + PAGE_SIZE;

	if(page == totalPage && leftNum)
		end = start + leftNum;
    //qDebug() << "start end" << start << " " << end << "...............";

	itemCount = 0;
	currentPageItemCount = end - start;				//记录当前也显示多少条数据
	
	if(deviceAddrList.addrList.size() == 0)
		return;

	/*查询类型判断*/
	InfoBodyType type;
	char buff[2];

	if(deviceAddrList.type == 1)
		type =  InfoConfigDCChargerTerm;
	else if(deviceAddrList.type == 2)
		type =  InfoConfigDCChargerMonitor;
	else
		return;
	/*发送查询指令*/
	buff[0] = deviceAddrList.type;
	for(int i = start; i< end; i++)		//根据地址列表里的数据循环查找
	{
		buff[1] = deviceAddrList.addrList.at(i-1);
        //qDebug() << "send ask dc charget data.............. addr=" << buff[1];
		InfoProtocol infoPro;
		infoPro.insert(InfoDataType, QByteArray(1, 0));
		infoPro.insert(type, QByteArray(buff, 2));
		this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
	}
}
/**
 *table显示终端参数当前页配置数据
 */
void DCChargerSet::showTermData(stDCChargerTermParam termCode)
{
    //if(ui->tableWidgetCCUDate->columnCount() != 10)
      //  return;
	QTableWidget *table = ui->tableWidgetTermParam;
    //qDebug() << "recive canAddr//////////////"<< termCode.canAddr;
    //当控件未初始化完成，表明数据非本次请求的数据,防止快速点击切换界面，导致上一次请求的数据传送过来，界面还未初始化，导致界面崩溃的问题--yanwei 20171014
    if(table->item(itemCount,0) == NULL){
        return;
    }
	table->item(itemCount,0)->setText(QString::number(termCode.canAddr, 10));
	table->item(itemCount,1)->setText(QString::number(termCode.pduId, 10));
    table->item(itemCount,2)->setText(QString::number(termCode.maxChargeCurrent,10));
	table->item(itemCount,3)->setText(QString::number(termCode.chargePriority, 10));

	QComboBox *combox4 =(QComboBox *)table->cellWidget(itemCount,4);
	if(termCode.chargeType<= 2)
		combox4->setCurrentIndex(termCode.chargeType);
	else
		combox4->setCurrentIndex(0);
	QComboBox *combox5 =(QComboBox *)table->cellWidget(itemCount,5);
    if(termCode.chargeStrategy <= 3)
		combox5->setCurrentIndex(termCode.chargeStrategy);
	else
		combox5->setCurrentIndex(0);
	QComboBox *combox6 =(QComboBox *)table->cellWidget(itemCount,6);
	if(termCode.powerType <= 2)
		combox6->setCurrentIndex(termCode.powerType);
	else
		combox6->setCurrentIndex(0);
	QComboBox *combox7 =(QComboBox *)table->cellWidget(itemCount,7);
	if(termCode.lowTempEnable <= 2)
		combox7->setCurrentIndex(termCode.lowTempEnable);
	else
		combox7->setCurrentIndex(0);

	QComboBox *combox8 =(QComboBox *)table->cellWidget(itemCount,8);
	if(termCode.elcLockEnable <= 2)
		combox8->setCurrentIndex(termCode.elcLockEnable);
	else
		combox8->setCurrentIndex(0);

	QComboBox *combox9 =(QComboBox *)table->cellWidget(itemCount,9);
	if(termCode.elcLockTypeEnable <= 2)
        combox9->setCurrentIndex(termCode.elcLockTypeEnable);
	else
		combox9->setCurrentIndex(0);
	QComboBox *combox10 =(QComboBox *)table->cellWidget(itemCount,10);
	if(termCode.vinEnable <= 2)
		combox10->setCurrentIndex(termCode.vinEnable);
	else
		combox10->setCurrentIndex(0);

	QComboBox *combox11 =(QComboBox *)table->cellWidget(itemCount,11);
    if(termCode.newOldStd <= 3)
		combox11->setCurrentIndex(termCode.newOldStd);
	else
		combox11->setCurrentIndex(0);

	/*清空剩下行*/
	itemCount ++;
	if(itemCount >= currentPageItemCount)
	{
		for(int row = itemCount; row<PAGE_SIZE; row++)
		{
			table->item(itemCount, 0)->setText(QObject::tr(""));
			table->item(itemCount, 1)->setText(QObject::tr(""));
			table->item(itemCount, 2)->setText(QObject::tr(""));
			table->item(itemCount, 3)->setText(QObject::tr(""));

			for(int k = 4; k <12; k++)
			{
				QComboBox *combox =(QComboBox *)table->cellWidget(itemCount,k);  //获得widget
				combox->setCurrentIndex(0);
			}
		}
		itemCount = 0;
	}
}
/**
 *table显示监控参数当前页配置数据
 */
void DCChargerSet::showMonitorData(stDCChargerMonitorParam monitorCode)
{
	//if(ui->tableWidgetCCUDate->columnCount() != 10)
	//  return;
    QTableWidget *table = ui->tableWidgetMonitiorParam;
    //当控件未初始化完成，表明数据非本次请求的数据,防止快速点击切换界面，导致上一次请求的数据传送过来，界面还未初始化，导致界面崩溃的问题--yanwei 20171014
    if(table->item(itemCount,0) == NULL){
        return;
    }
	table->item(itemCount,0)->setText(QString::number(monitorCode.canAddr, 10));
    table->item(itemCount,1)->setText(QString::number(monitorCode.setCanAddr, 10));
	table->item(itemCount,2)->setText(QString::number(monitorCode.gunStartAddr, 10));
	table->item(itemCount,3)->setText(QString::number(monitorCode.maxPower, 10));

	/*清空剩下行*/
	itemCount ++;
	if(itemCount >= currentPageItemCount)
	{
		for(int row = itemCount; row<PAGE_SIZE; row++)
			for(int column = 0; column < table->columnCount(); column++)
				table->item(row, column)->setText(QObject::tr(""));

		itemCount = 0;
	}
}
/**
 *保存当前显示终端参数中的数据
 */
void DCChargerSet::saveTermData()
{
	bool ok;
	QTableWidget *table = ui->tableWidgetTermParam;
	for(int i=0; i<currentPageItemCount; i++ )
	{
		unsigned canAddr = table->item(i,0)->text().toInt(&ok,10);

		stDCChargerTermParam &termcode = terminalDataMap.operator[](canAddr);

		/*更新到map中*/	
		termcode.canAddr = table->item(i,0)->text().toInt(&ok,10);
		termcode.pduId = table->item(i,1)->text().toInt(&ok,10);
		termcode.maxChargeCurrent = table->item(i,2)->text().toInt(&ok,10);
		termcode.chargePriority = table->item(i,3)->text().toInt(&ok,10);
		QComboBox *combox4 =(QComboBox *)table->cellWidget(i,4);
		termcode.chargeType = combox4->currentIndex();
		QComboBox *combox5 =(QComboBox *)table->cellWidget(i,5);
		termcode.chargeStrategy = combox5->currentIndex();
		QComboBox *combox6 =(QComboBox *)table->cellWidget(i,6);
		termcode.powerType = combox6->currentIndex();
		QComboBox *combox7 =(QComboBox *)table->cellWidget(i,7);
		termcode.lowTempEnable = combox7->currentIndex();
		QComboBox *combox8 =(QComboBox *)table->cellWidget(i,8);
		termcode.elcLockEnable = combox8->currentIndex();
		QComboBox *combox9 =(QComboBox *)table->cellWidget(i,9);
		termcode.elcLockTypeEnable = combox9->currentIndex();
		QComboBox *combox10 =(QComboBox *)table->cellWidget(i,10);
		termcode.vinEnable = combox10->currentIndex();
		QComboBox *combox11 =(QComboBox *)table->cellWidget(i,11);
		termcode.newOldStd = combox11->currentIndex();
	}
}
/**
 *保存当前监控参数参数中的数据
 */
void DCChargerSet::saveMonitorData()
{
	bool ok;
	QTableWidget *table = ui->tableWidgetMonitiorParam;
	for(int j=0; j<currentPageItemCount; j++)
	{
		unsigned char canAddr = table->item(j,0)->text().toInt(&ok,10);
		stDCChargerMonitorParam &monitorcode = monitorDataMap.operator[](canAddr);

		/*更新到map中*/
		monitorcode.canAddr = table->item(j,0)->text().toInt(&ok,10);
		monitorcode.setCanAddr = table->item(j,1)->text().toInt(&ok,10);
		monitorcode.gunStartAddr = table->item(j,2)->text().toInt(&ok,10);
		monitorcode.maxPower = table->item(j,3)->text().toInt(&ok,10);
	}
}
/**
 *点击保存
 */
void DCChargerSet::on_buttonSave_clicked()
{
	/*保存一下当前表数据, 并且下发数据*/
	int index = ui->tabWidgetChargerSet->currentIndex();
	if(index == 0)				//终端数据
    {
        //qDebug() <<"send data///////////////////////////////////////////";
		saveTermData();
		QMap<unsigned char, stDCChargerTermParam>::const_iterator i;
		for (i = terminalDataMap.constBegin(); i != terminalDataMap.constEnd(); ++i) 
		{
			stDCChargerTermParam param = i.value();

			InfoProtocol infoPro;
            infoPro.insert(InfoDataType, QByteArray(1, 1));
			infoPro.insert(InfoConfigDCChargerTerm  ,QByteArray((char*)&param, sizeof(stDCChargerTermParam)));
			this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
		}
	}
	else if(index == 1)			//监控数据
	{
		saveMonitorData();
		
		QMap<unsigned char, stDCChargerMonitorParam>::const_iterator i;
		for (i = monitorDataMap.constBegin(); i != monitorDataMap.constEnd(); ++i) 
		{
			stDCChargerMonitorParam param = i.value();
			InfoProtocol infoPro;
            infoPro.insert(InfoDataType, QByteArray(1, 1));
			infoPro.insert(InfoConfigDCChargerMonitor  ,QByteArray((char*)&param, sizeof(stDCChargerMonitorParam)));
			this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
		}
	}
}
/**
 *点击上一页
 */
void DCChargerSet::on_buttonUp_clicked()
{
	/*保存一下当前表数据*/
	int index = ui->tabWidgetChargerSet->currentIndex();
	if(index == 0)
		saveTermData();
	else if(index == 1)
		saveMonitorData();

	/*改变当前页*/
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
void DCChargerSet::on_buttonDown_clicked()
{
	/*保存一下当前表数据*/
	int index = ui->tabWidgetChargerSet->currentIndex();
	if(index == 0)
		saveTermData();
	else if(index == 1)
		saveMonitorData();

	/*改变当前页*/
	currentPage--;
	if(currentPage == 0)
		currentPage = totalPage;

	if(totalNum == 0)
		currentPage = 1;

	currentPageInit(currentPage);
}
/**
 * @brief 创建表格中的复选框
 */
void DCChargerSet::on_tabWidgetChargerSet_currentChanged(int index)
{
    ui->labelShowTotal->setText("");
    unsigned param=0;
	itemCount = 0;
    if(index == 0)
    {
        param = 1;
        clearTerminalInfor();       //清空终端参数数据　　　add by songqb  2017-6-8
        //qDebug() << "send data query term addr data............index is--------" << index;
        deviceAddrList.type = 1;
        deviceAddrList.addrList.clear();
    }
    else if(index == 1)
    {
        //qDebug() << "send data query monitor addr data............ index is -------"  << index;
        clearMonitorInfor();        //清空监控参数数据　　　add by songqb  2017-6-8
        param = 2;
        deviceAddrList.type = 2;
        deviceAddrList.addrList.clear();
    }
	
    //qDebug() <<"tab chagge end.............1";
	
	InfoProtocol infoPro;
	infoPro.insert(InfoDataType, QByteArray(1, 0));
	infoPro.insert(InfoConfigDCChargerTypeNum ,QByteArray((char*)&param,1));
    //qDebug() <<"tab chagge end.............2";
	this->protocol->sendProtocolData(infoPro, InfoAddrConfig);

    //qDebug() <<"tab chagge end.............";
}


