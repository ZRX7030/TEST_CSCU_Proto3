#include <QDebug>
#include <QComboBox>
#include <QList>

#include "ActriphaseSet.h"
#include "ui_ActriphaseSet.h"
#include "StatusRemindWindow.h"

ActriphaseSet::ActriphaseSet(QWidget *parent, CBus * bus, ProtocolBase *protocol) :
    QWidget(parent),
    ui(new Ui::ActriphaseSet)
{
    ui->setupUi(this);

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
    /*发送查询相别设置命令*/
    InfoProtocol infoPro;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoConfigPhaseType, QByteArray());
    this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
}

ActriphaseSet::~ActriphaseSet()
{
    this->bus->cancelRegistDev(this);
    delete ui;
}
void ActriphaseSet::receiveFromBus(InfoMap Map, InfoAddrType Type)
{
    emit sigFromBus(Map, Type);
}

void ActriphaseSet::slotBusToOwn(InfoMap Map, InfoAddrType Type)
{
    //qDebug() << " ActriphaseSet receive data";
	if(Type == InfoAddrConfig)			//收到配置数据
	{
		if(false == Map.contains(InfoConfigPhaseType))
			return;

		QVariant var = Map.value(InfoConfigPhaseType);
		stAllPhaseType param = var.value<stAllPhaseType>();
		phaseTypeList.clear();
		phaseTypeList =  param.phaseList;

		totalNum = param.phaseList.size();
		totalPage = totalNum/10;
		if(totalNum%10)
			totalPage += 1;

        //qDebug() << "total num, totalpage  " << totalNum << "  " << totalPage;
		currentPage = 1;
        //QString showLable = QObject::tr("总条数：") + QString::number(totalNum, 10) + QObject::tr(", ") + QObject::tr("共") + QString::number(totalPage, 10) + QObject::tr("页 ") + QObject::tr("当前第") + QString::number(currentPage, 10) + QObject::tr("页");
		//ui->labelTotal->setText(showLable);

		int startPos=0, count=0;
		if(findListPosition(currentPage, startPos, count))
		{
            //qDebug() << " ActriphaseSet receive data3333333333333" << startPos << "    " << count;
			showCurrnetPage(startPos, count);
		}
	}
	else if(Type == InfoAddrExchange)			//设置结果返回
	{
		if(false == Map.contains(InfoExchangeParamResult))
			return;
		QVariant var = Map.value(InfoExchangeParamResult);
		stExchangeResult result = var.value<stExchangeResult>();

		if(result.type == InfoConfigPhaseType)  //back cscu config reuslt
		{
			StatusRemindWindow  *statusDialog = NULL;
			if(result.result == 0)
                statusDialog = new StatusRemindWindow(this, 1, 5, QObject::tr("三相相别设置失败"));
			else
                statusDialog= new StatusRemindWindow(this, 0, 5, QObject::tr("三相相别设置成功"));

			statusDialog->exec();
			delete statusDialog;
		}
	}
}
/**
 * @brief 根据当前页，计算在list中的开始位置及显示的数量
 */
int ActriphaseSet::findListPosition(int page, int &startPos, int &count)
{
	int currentShowNum = 0;
	if(totalNum == 0)
		return 0;
	if( (totalNum/10) >= page)
		currentShowNum = 10;
	else 
		currentShowNum = totalNum%10;

	startPos = (page - 1)*10;
	count = currentShowNum;

	return 1;
}

/**
 *table显示当前页配置数据
 */
void ActriphaseSet::showCurrnetPage(int startPos, int count)
{
	QTableWidget *table = ui->tableWidget;
	for(int i=0; i<count; i++)
	{
		stPhaseType phaseType = phaseTypeList.at(startPos+i);

		table->item(i, 0)->setText(QString::number(startPos+i+1, 10));
		table->item(i, 1)->setText(QString::number(phaseType.canAddr, 10));

		QComboBox *combox =(QComboBox *)table->cellWidget(i,2);  //获得widget
		if(phaseType.phaseType <=3)
			combox->setCurrentIndex(phaseType.phaseType);
		else
			combox->setCurrentIndex(0);
	}
	/*清空剩下行*/
	for(int i = count; i <table->rowCount(); i++ )
	{
		table->item(i, 0)->setText(QObject::tr(""));
		table->item(i, 1)->setText(QObject::tr(""));
		QComboBox *combox =(QComboBox *)table->cellWidget(i,2);  //获得widget
		combox->setCurrentIndex(0);
	}

    QString showLable = QObject::tr("总条数：") + QString::number(totalNum, 10) + QObject::tr(", ") + QObject::tr("共") + QString::number(totalPage, 10) + QObject::tr("页 ") + QObject::tr("当前第") + QString::number(currentPage, 10) + QObject::tr("页");
	ui->labelTotal->setText(showLable);
}
/**
 *保存当前table中的数据
 */
void ActriphaseSet::saveCurrnetPage(int startPos, int count)
{
	for(int i = 0; i < count; i++ )
	{
		stPhaseType &phaseType = phaseTypeList.operator[](startPos+i);
		QComboBox *combox =(QComboBox *)ui->tableWidget->cellWidget(i,2);  //获得widget
		phaseType.phaseType = combox->currentIndex();
	}
}

/**
 * @brief 表格创建
 */
void ActriphaseSet::createTableItems()
{
	QTableWidget *table = ui->tableWidget;

	for(int i=0; i<table->rowCount(); i++)
    {
        // table->insertRow(i);
        for(int k=0; k<table->columnCount(); k++)
        {
            if(k == 2)
            {
                QComboBox *comboxPhase= new QComboBox(); // 下拉选择框控件
                comboxPhase->addItem(QObject::tr(""));
                comboxPhase->addItem(QObject::tr("A相"));
                comboxPhase->addItem(QObject::tr("B相"));
                comboxPhase->addItem(QObject::tr("C相"));
                table->setCellWidget(i, k, (QWidget*)comboxPhase);
            }
            else
            {
                QTableWidgetItem *tableItem = new QTableWidgetItem();
                tableItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter );
                table->setItem(i, k, tableItem);
                table->item(i, k)->setText(QString(""));
            }
        }
    }
}

void ActriphaseSet::on_buttonDown_clicked()
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

void ActriphaseSet::on_buttonSave_clicked()
{
	if(currentPage == 0)
		return;
    
	int startPos=0, count=0;
	if(findListPosition(currentPage, startPos, count))
		saveCurrnetPage(startPos, count);

    InfoProtocol infoPro;
    unsigned char tmpBuff[1024];
    unsigned char *point = tmpBuff + 1;
    stPhaseType param;

    tmpBuff[0] = phaseTypeList.size();
    for(int i=0; i<phaseTypeList.size(); i++)
    {
        param = phaseTypeList.at(i);
        point[0] = param.canAddr;
        point[1] = param.phaseType;
        point += sizeof(stPhaseType);
    }

    infoPro.insert(InfoDataType, QByteArray(1, 1));
    infoPro.insert(InfoConfigPhaseType, QByteArray((char *)tmpBuff, point - tmpBuff));
    this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
}

void ActriphaseSet::on_buttonUp_clicked()
{
	if(currentPage == 0)
		return;

    int startPos=0, count=0;
	if(findListPosition(currentPage, startPos, count))
		saveCurrnetPage(startPos, count);

    currentPage--;
    if(currentPage == 0)
        currentPage = totalPage;
	if(findListPosition(currentPage, startPos, count))
		showCurrnetPage(startPos, count);
}
