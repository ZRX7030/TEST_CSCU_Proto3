#include <stdio.h>
#include <QDebug>
#include <QMouseEvent>

#include "MainCharge.h"
#include "ui_MainCharge.h"
#include "StatusRemindWindow.h"
#include "ChargingReport.h"


MainCharge::MainCharge(QWidget *parent, CBus *bus, ProtocolBase *protocol) :
    QWidget(parent),
    ui(new Ui::MainCharge)
{
    ui->setupUi(this);

    this->bus = bus;
    this->protocol = protocol;

    currentPosition = 0;
    groupsingleflag = 0;
    readfile();

    /*注册数据*/
    QList<InfoAddrType> list;
    list.append(InfoAddrConfig);
    list.append(InfoAddrStatus);
    this->bus->registDev(this, list);

    timerQuery.setInterval(1000);
    QObject::connect(&timerQuery, SIGNAL(timeout()), this, SLOT(slotQueryTerminal()));
    QObject::connect(this,SIGNAL(sigFromBus(InfoMap, InfoAddrType)), this, SLOT(slotBusToOwn(InfoMap, InfoAddrType)));

}


#if 0
void MainCharge::mousePressEvent(QMouseEvent *event)
{
    if(event->y() >= 400 && timerLockFlag == false)
    {
        pressX = event->x();
        pressY = event->y();
        timerLockFlag = true;

        lockTimer = new QTimer();
        lockTimer->setInterval(1000);
        QObject::connect(lockTimer, SIGNAL(timeout()), this, SLOT(slotTimerLock()));
        lockTimer->start();
    }
    qDebug() << "MainCharge: mouse press event trigger xy" << event->x() << event->y();
    QWidget::mousePressEvent(event);
}

void MainCharge::mouseMoveEvent(QMouseEvent *event)
{
    if(timerLockFlag == true)
    {
        if(pressX > event->x() && (pressX-event->x()) >5)  //上一页
        {

        }
        else if (pressX < event->x() && (event->x() - pressX) > 5)
        {

        }
    }
    qDebug() << "MainCharge: mouse move event trigger" << event->x() << event->y();
    QWidget::mouseMoveEvent(event);
}
#endif
void MainCharge::readfile()
{
    //logo设置增加　add　by　songqb 2017-6-16
    int currentnum = 0;
    QFile file("/mnt/nandflash/etc/teuilogo.conf");
    if(!file.open(QFile::ReadOnly | QFile::Text)) //只读方式打开文件
    {
        qDebug()<<"Can't open the file!";
        //return;
    }
    QTextStream in(&file);                //读文件内容
    QString str = in.readAll();
    bool ok;
    currentnum = str.toInt(&ok,10);  //转int型
    if(currentnum == 2)
    {
        ui->label->setStyleSheet("QLabel{border-image:url(:/phnoe_0.png);;}");
        ui->label_2->setStyleSheet("QLabel{border-image:url(:/phnoe_0.png);;}");
    }
    file.close();
}
void MainCharge::chargeTermGroup()
{
    int row = 0;
    int column = 0;
    int count = 0;
    if(mapTotalCharge.empty() == false)       //解决点击主界面终端数据重发
        return;

    for(row=0; row <2; row++)
    {
        for(column = 0; column < 4; column ++)
        {
            count++;
            ChargeTerm *chargeWidget = (ChargeTerm *)ui->gridLayout->itemAtPosition(row, column)->widget();
            QObject::connect(chargeWidget, SIGNAL(signalChargerClicked(unsigned char, int)), this, SLOT(slotChargerClicked(unsigned char, int)));
            mapTotalCharge.insert(count, chargeWidget);
        }
    }
}

void MainCharge::chargeTermSingle()         //解决点击主界面终端数据重发
{
    int i = 0;
    if(mapTotalCharge.empty() == false)
        return;
    for(i=0; i<3; i++)
    {
        ChargeTerm *chargeWidget = (ChargeTerm *)ui->layouChargeTerm->itemAt(i)->widget();
        chargeWidget->singleChargeTermSet();      //设置单桩主界面插件比例、字体大小
        QObject::connect(chargeWidget, SIGNAL(signalChargerClicked(unsigned char, int)), this, SLOT(slotChargerClicked(unsigned char, int)));
        mapTotalCharge.insert(i, chargeWidget);
    }
}
MainCharge::~MainCharge()
{
    mapTotalCharge.clear();
    this->bus->cancelRegistDev(this);
    delete ui;
}
void MainCharge::slotChargerClicked(unsigned char canAddr, int chargeStatus)
{
    //qDebug() << "canaddr=" << canAddr << "chargestatus=" << chargeStatus <<"clicked";
    emit sigChargerClicked(canAddr, chargeStatus);
}

void MainCharge::queryConfig()
{
    InfoProtocol infoPro;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoConfigCSCU, QByteArray());
    this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
}

void MainCharge::queryConfigSpecialFeatureSet()   //重新查询特殊功能设置 用于VIN充电  2017-3-28 by songqb
{
    InfoProtocol infoPro;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoConfigSpecialFunc, QByteArray());
    this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
}

#if 0
void MainCharge::slotTimerLock()
{
    timerLockFlag = false;
}
#endif
/**
 *定时查询终端参数
 */
void MainCharge::slotQueryTerminal()
{
    for(int i=0; i< queryCanList.size(); i++)
    {
        unsigned char canAddr = queryCanList.at(i);

        InfoProtocol infoPro;
        infoPro.insert(InfoDataType, QByteArray(1, 0));
        infoPro.insert(InfoStatusBase, QByteArray((char *)&canAddr, 1));
        this->protocol->sendProtocolData(infoPro, InfoAddrStatus);
    }
}

void MainCharge::receiveFromBus(InfoMap Map, InfoAddrType Type)
{
    emit sigFromBus(Map, Type);
}

void MainCharge::slotBusToOwn(InfoMap Map, InfoAddrType Type)
{
    if(Type == InfoAddrStatus)
    {
        if(Map.contains(InfoStatusBase))
        {
            QVariant var = Map.value(InfoStatusBase);
            stTerminalStatus statusData = var.value<stTerminalStatus>();
            ChargeTerm *charge = mapChargerm.value(statusData.canAddr);
            if(charge)
            {
                stChargeTermData chargeData;

                chargeData.canAddr = statusData.canAddr;
                chargeData.status = statusData.status;
                chargeData.linkStatus = statusData.linkStatus;
                chargeData.soc = statusData.soc;
                snprintf(chargeData.name, sizeof(chargeData.name),"%s", statusData.name);

                charge->updateChargeStatus(chargeData);
            }
        }
    }
    else if(Type == InfoAddrConfig )
    {

        if(true == Map.contains(InfoConfigSpecialFunc))
        {
            QVariant var = Map.value(InfoConfigSpecialFunc);
            stSpecialFunc param = var.value<stSpecialFunc>();
            memcpy(&special_set,&param,sizeof(param));  //nihai add		//merge by yanwei 20171011
        }
        if(Map.contains(InfoConfigCSCU))
        {
            mapCanAddr.clear();
            mapChargerm.clear();
            queryCanList.clear();

            QVariant var = Map.value(InfoConfigCSCU);
            stCSCUParam cscuParam = var.value<stCSCUParam>();
			totalNum = cscuParam.singlePhaseNum + cscuParam.threePhaseNum + cscuParam.dcPhaseNUm;
            if(totalNum > 3)
            {
               ui->stackedWidget->setCurrentIndex(0);
               chargeTermGroup();
               groupsingleflag = 1;
            }
            else
            {
               ui->stackedWidget->setCurrentIndex(1);
               chargeTermSingle();
               groupsingleflag = 2;
            }

            totalPage = totalNum /8;
            leftNum = totalNum % 8;
			if(leftNum)
                totalPage++;
            currentPage = 1;

            /*获取个数与can地址的关系*/
            int count =1;
            if(special_set.pileType==1) //single,ocean debug
            {
                //printf("!!!!!!!!!!!!!!!!!!!!  InfoConfigSpecialFunc\n");
                for(int i=0; i< cscuParam.dcPhaseNUm; i++)//mod by yanwei 20170914, 修改终端显示顺序
                    mapCanAddr.insert(count++, 181+i);
                for(int i=0; i< cscuParam.threePhaseNum; i++)
                    mapCanAddr.insert(count++, 151+i);
                for(int i=0; i< cscuParam.singlePhaseNum; i++)
                    mapCanAddr.insert(count++, 1+i);
            }else
            {
                //printf("#######################  InfoConfigSpecialFunc\n");
                for(int i=0; i< cscuParam.singlePhaseNum; i++)
                    mapCanAddr.insert(count++, 1+i);
                for(int i=0; i< cscuParam.threePhaseNum; i++)
                    mapCanAddr.insert(count++, 151+i);
                for(int i=0; i< cscuParam.dcPhaseNUm; i++)
                    mapCanAddr.insert(count++, 181+i);
            }

            //qDebug() << "mapCanAddr......" << mapCanAddr.size() << "total num ..." << totalNum;

            if(groupsingleflag == 1)
            {
                /*获取can地址与充电终端的对应关系*/
                for(int i=0; i<totalNum; i++)
                {
                    int row = i%8/4;
                    int column = i%8%4;

                    mapChargerm.insert(mapCanAddr.value(i+1), (ChargeTerm *)ui->gridLayout->itemAtPosition(row, column)->widget());
                }
            }

            else
            {
                for(int i=0; i<totalNum; i++)
                {
                   mapChargerm.insert(mapCanAddr.value(i+1), (ChargeTerm *)ui->layouChargeTerm->itemAt(i)->widget());
                }
            }

            //qDebug() << "MainCharge query result" << totalNum << totalPage;
            currentPageInit(currentPage);

            currentPosition = 0;

            timerQuery.start();
        }
    }
}

void MainCharge::currentPageInit(int page)
{
    if(groupsingleflag == 1)
    {
        queryCanList.clear();


        if(totalNum == 0)
       {
           for(int i=0; i<8; i++)
                ((ChargeTerm *)mapTotalCharge.value(i+1))->hide();
           return;
       }

       int start = (page - 1)*8 + 1;
       int end = start + 8;

       if(page == totalPage && leftNum)
           end = start + leftNum;
       //qDebug() << "start end" << start << end << "...............";

       for(int i = start; i< end; i++)
       {
           int key = i%8;
           queryCanList.append(mapCanAddr.value(i));

           if(key)
               ((ChargeTerm *)mapTotalCharge.value(key%8))->show();
           else
               ((ChargeTerm *)mapTotalCharge.value(8))->show();
       }
       int hideCount = 8 - (end - start);
       //qDebug() << "hide count is ...................." << hideCount;
       if(hideCount)
       {
           for(int i = (8-hideCount); i<8; i++)
                ((ChargeTerm *)mapTotalCharge.value(i+1))->hide();
       }
    }
     else
    {
        queryCanList.clear();
        if(totalNum == 1)
        {

            ((ChargeTerm *)mapTotalCharge.value(0))->show();
            ((ChargeTerm *)mapTotalCharge.value(1))->hide();
            ((ChargeTerm *)mapTotalCharge.value(2))->hide();
            queryCanList.append(mapCanAddr.value(1));
            ui->horizontalLayout_4->setStretch(0, 7);
            ui->horizontalLayout_4->setStretch(1, 6);
            ui->horizontalLayout_4->setStretch(2, 7);
        }
        else if(totalNum == 2)
        {

            queryCanList.append(mapCanAddr.value(1));
            queryCanList.append(mapCanAddr.value(2));
            ((ChargeTerm *)mapTotalCharge.value(0))->show();
            ((ChargeTerm *)mapTotalCharge.value(1))->show();
            ((ChargeTerm *)mapTotalCharge.value(2))->hide();
            ui->layouChargeTerm->setSpacing(50);
            ui->horizontalLayout_4->setStretch(0, 1);
            ui->horizontalLayout_4->setStretch(1, 4);
            ui->horizontalLayout_4->setStretch(2, 1);
        }
        else
        {
            queryCanList.append(mapCanAddr.value(1));
            queryCanList.append(mapCanAddr.value(2));
            queryCanList.append(mapCanAddr.value(3));
            ((ChargeTerm *)mapTotalCharge.value(0))->show();
            ((ChargeTerm *)mapTotalCharge.value(1))->show();
            ((ChargeTerm *)mapTotalCharge.value(2))->show();
            ui->layouChargeTerm->setSpacing(26);
            ui->horizontalLayout_4->setStretch(0, 1);
            ui->horizontalLayout_4->setStretch(1, 26);
            ui->horizontalLayout_4->setStretch(2, 1);
        }
    }
}

void MainCharge::on_pushButtonFrontPage_clicked()
{
    currentPage--;
    if(currentPage == 0)
        currentPage = totalPage;

	if(totalNum == 0)
		currentPage = 1;

    currentPageInit(currentPage);
}

void MainCharge::on_pushButtonNextPage_clicked()
{
    currentPage++;
    if(currentPage > totalPage)
        currentPage = 1;

    currentPageInit(currentPage);
}

