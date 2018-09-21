#include <QDebug>
#include <QDateTime>
#include <QVariant>
#include <stdlib.h>

#include "TeuiMainWindow.h"
#include "PasswordLogin.h"
#include "MainCharge.h"
#include "ExportData.h"
#include "ChargeManage.h"
#include "ChargeReportFinish.h"
#include "ChargingReport.h"
#include "ChagingFailureInformation.h"
#include "PasswordSet.h"
#include "BMSInformation.h"
#include "ApplayCharge.h"
#include "StatusRemindWindow.h"
#include "FlashWaitWidget.h"
#include "ChargeTypeSelect.h"
#include "ChargeMangeBase.h"
#include "ui_TeuiMainWindow.h"
#include "FlashExportLog.h"
#include "VINStartCharging.h"
#include "SpecialFeatureSet.h"
#include "PrintPaper.h"


TeuiMainWindow::TeuiMainWindow(QWidget *parent, CBus *bus, ProtocolBase *protocol, void *param) :
	QMainWindow(parent),
	ui(new Ui::TeuiMainWindow)
{
	teuiParam = (stTeuiParam *)param;

	needCheckReconnect = false;
    clickConnect = 0;
    widget = NULL;
    password = 0;

	heartCount = 0;
	heartRespond = false;
	
	maintainLock = false;

    this->bus = bus;
	this->protocol = protocol;
	this->canAddr = 0;
    this->emergencyCharge = 0;
    this->termStatus = 0;
    vinparam.vinOffline = 0;
    vinparam.languageSelect = 0;

    ui->setupUi(this);

    setLanguage();

    ui->labelTitleMain->setAttribute(Qt::WA_TranslucentBackground);
    ui->labelTitleSlave->setAttribute(Qt::WA_TranslucentBackground);
    readfile();
   // this->setWindowOpacity(1); //窗口整体透明度，0-1 从全透明到不透明
    this->setWindowFlags(Qt::FramelessWindowHint); //设置无边框风格
    //this->setAttribute(Qt::WA_TranslucentBackground); //设置背景透明，允许鼠标穿透

    QObject::connect(this, SIGNAL(sigClickLogo()), this, SLOT(on_buttonLogo_clicked())); //返回主界面
	
	//运维管理界面
    maintainWidget = new Maintain(ui->pageMaintain, this->bus, this->protocol, teuiParam);
    ui->layoutMaintain->addWidget(maintainWidget);
    
	//升级、数据导出界面
	updateExportWidget = new ExportData(ui->pageExportMain, this->bus, this->protocol, teuiParam);
    ui->layoutExportMain->addWidget(updateExportWidget);
    QObject::connect((ExportData *)updateExportWidget, SIGNAL(sigBackToMain(int, int, QVariant)), this, SLOT(slotUpdateExportSwitch(int, int, QVariant)));
    QObject::connect((ExportData *)updateExportWidget, SIGNAL(sigClickLogo()), this, SLOT(on_buttonLogo_clicked())); //返回主界面

    exportFlash = new FlashExportLog(ui->pageExportFlash, this->bus, this->protocol, teuiParam);
    ui->layoutExportFlash->addWidget(exportFlash);
    QObject::connect((FlashExportLog *)exportFlash, SIGNAL(sigBackToMain(int, int, QVariant)), this, SLOT(slotUpdateExportSwitch(int, int, QVariant)));
    QObject::connect((FlashExportLog *)exportFlash, SIGNAL(sigClickLogo()), this, SLOT(on_buttonLogo_clicked())); //返回主界面
	
	//刷卡充电管理界面
    chargeStandby  = new ApplayCharge(ui->pageStartCharge, this->bus, this->protocol);
    ui->layoutStartCharge->addWidget(chargeStandby);
    QObject::connect((ApplayCharge *)chargeStandby, SIGNAL(sigBackToMain(int, int, QVariant)), this, SLOT(slotChargeSwitch(int, int, QVariant)));

    chargeVINStart  = new VINStartCharging(ui->pageVINCharge, this->bus, this->protocol);
    ui->layoutVINCharge->addWidget(chargeVINStart);
    QObject::connect((VINStartCharging *)chargeVINStart, SIGNAL(sigBackToMain(int, int, QVariant)), this, SLOT(slotChargeSwitch(int, int, QVariant)));

    flashWait = new FlashWaitWidget(ui->pageFlashWait, this->bus, this->protocol);
    ui->layoutFlashWait->addWidget(flashWait);
    QObject::connect((FlashWaitWidget *)flashWait, SIGNAL(sigBackToMain(int, int, QVariant)), this, SLOT(slotChargeSwitch(int, int,QVariant)));

	chargeBase = new ChargeMangeBase(ui->pageBase);
    ui->layoutBase->addWidget(chargeBase);
    QObject::connect((ChargeMangeBase *)chargeBase, SIGNAL(sigBackToMain(int, int, QVariant)), this, SLOT(slotChargeSwitch(int, int,QVariant)));

    chargeSelect = new ChargeTypeSelect(ui->pageSelectCharge, this->bus, this->protocol);
    ui->layoutSelectCharge->addWidget(chargeSelect);
    QObject::connect((ChargeTypeSelect *)chargeSelect, SIGNAL(sigBackToMain(int, int, QVariant)), this, SLOT(slotChargeSwitch(int, int, QVariant)));

    chargingReport = new ChargingReport(ui->pageChargeing,this->bus, this->protocol);
    ui->layoutChargeing->addWidget(chargingReport);
    QObject::connect((ChargingReport *)chargingReport, SIGNAL(sigBackToMain(int, int, QVariant)), this, SLOT(slotChargeSwitch(int, int, QVariant)));

    chargeFinishReport = new ChargeReportFinish(ui->pageChargeFinish,this->bus,this->protocol);
    ui->layoutChargeFinish->addWidget(chargeFinishReport);
    QObject::connect((ChargeReportFinish *)chargeFinishReport, SIGNAL(sigBackToMain(int, int, QVariant)), this, SLOT(slotChargeSwitch(int, int, QVariant)));

    chargeFault = new ChagingFailureInformation(ui->pageFault,this->bus,this->protocol);
    ui->layoutFault->addWidget(chargeFault);
    QObject::connect((ChagingFailureInformation *)chargeFault,SIGNAL(sigBackToMain(int, int, QVariant)), this, SLOT(slotChargeSwitch(int, int, QVariant)));

    printPaper = new PrintPaper(ui->pagePrintPaper,this->bus,this->protocol);
    ui->layoutPrintPaper->addWidget(printPaper);
    QObject::connect((PrintPaper *)printPaper, SIGNAL(sigBackToMain(int, int, QVariant)), this, SLOT(slotChargeSwitch(int, int, QVariant)));

    mainChargeWidget = new MainCharge(ui->pageMainCharge, this->bus, this->protocol);
    ui->layoutMainCharge->addWidget(mainChargeWidget);
    QObject::connect((MainCharge *)mainChargeWidget, SIGNAL(sigChargerClicked(unsigned char, int)), this, SLOT(slotChargeClicked(unsigned char, int)));

    currentPage = PAGE_CHARGERMANAGE;
    //currentPage = PAGE_UPDATEEXPORT;
    ui->stackedWidget->setCurrentIndex(currentPage);
    ui->stackedtChargeManage->setCurrentIndex(PAGE_CHARGEMANAGE_MAIN);
    //ui->stackedUpdateExport->setCurrentIndex(0);

    /*注册数据*/
    QList<InfoAddrType> list;
    list.append(InfoAddrStatus);
    list.append(InfoAddrConfig);
    this->bus->registDev(this, list);

    QObject::connect(this,SIGNAL(sigFromBus(InfoMap, InfoAddrType)), this, SLOT(slotBusToOwn(InfoMap, InfoAddrType)));

    dispTime.setInterval(1000);
    QObject::connect(&dispTime, SIGNAL(timeout()), this, SLOT(slotDisplayTime()));
    dispTime.start();

    /*发送特殊更能设置查询*/           //2017-3-28 by songqb
    InfoProtocol infoPro;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoConfigSpecialFunc, QByteArray());
    this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
}

TeuiMainWindow::~TeuiMainWindow()
{
    this->bus->cancelRegistDev(this);
     if(widget)
         delete widget;
    delete ui;
}
void TeuiMainWindow::readfile()
{
    //logo设置增加　add　by　songqb 2017-6-16
    int currentnum = 0;
    QFile file("/mnt/nandflash/etc/teuilogo.conf");
    if(!file.open(QFile::ReadOnly | QFile::Text)) //只读方式打开文件
    {
        qDebug()<<"Can't open the file!";
        return;
    }
    QTextStream in(&file);                //读文件内容
    QString str = in.readAll();
    bool ok;
    currentnum = str.toInt(&ok,10);  //转int型
    if(currentnum == 2)
    {
        ui->buttonLogo->setStyleSheet("QPushButton{border-image: url(:/logo_03.png);}" );
        ui->labelTitleMain->setText("");
    }
    file.close();
}

void TeuiMainWindow::setLanguage()
{
    int currentnum = -1;
    QFile file("/mnt/nandflash/etc/Language.conf");
    if(!file.open(QFile::ReadOnly | QFile::Text)) //只读方式打开文件
    {
        qDebug()<<"Can't open the file!";
        return;
    }
    QTextStream in(&file);                //读文件内容
    QString str = in.readAll();
    bool ok;
    currentnum = str.toInt(&ok,10);  //转int型
    //qDebug()<<"the Language nub now is :"<< currentnum;
    if(currentnum == 2)
    {
        ui->labelEmergency->setStyleSheet("QLabel{border-image: url(:/localcharge.png);}" );
        translator = new QTranslator(this);
        translator->load("/mnt/nandflash/etc/language/Language_Eng");
        qApp->installTranslator(translator);
        ui->retranslateUi(this);
    }
    file.close();
}

void TeuiMainWindow::slotConnected(void)
{
	needCheckReconnect = true;

	emit sigCloseWidget();
    //qDebug() << "TeuiMainWindow: detect socket connect";
    if(currentPage == PAGE_CHARGERMANAGE){        
        ((MainCharge *)mainChargeWidget)->queryConfigSpecialFeatureSet();//merge by yanwei 20171011
        ((MainCharge *)mainChargeWidget)->queryConfig();
    }
}

void TeuiMainWindow::slotClosed(void)
{
	needCheckReconnect = false;
	if(teuiParam->showTermWin) 
	{
        StatusRemindWindow  *statusDialog= new StatusRemindWindow(this, 1, -1, QObject::tr("与显示屏通信中断"));
		QObject::connect(this, SIGNAL(sigCloseWidget()), statusDialog, SLOT(slotCloseWidget()));
		statusDialog->exec();
		delete statusDialog;
	}
}
/**
 *用户点击充电机
 */
void TeuiMainWindow::slotChargeClicked(unsigned char canAddr, int status)
{
	char cmdBuff[50];
	
	maintainLock = true;
    //qDebug() << ".............chargeClicked status is " << status;
	this->canAddr = canAddr;
	this->termStatus = status;
	switch(status)
    {
        case CHARGE_STATUS_FREE:   //待机-空闲   //add by songqb 2017-5-11
        {
            snprintf(cmdBuff, sizeof(cmdBuff), "aplay %s &", START_CHARGE_GUN);
            system(cmdBuff);
            maintainLock = false;

            StatusRemindWindow  *statusDialog= new StatusRemindWindow(this, 1, 5, QObject::tr("请插枪"));
            statusDialog->exec();
            delete statusDialog;

        }break;
		case CHARGE_STATUS_GUN_STANDBY:	//连接就绪状态
            {
               if(vinparam.coupleGun == 0)  //多枪充电未开启
               {
                   ui->stackedtChargeManage->setCurrentIndex(PAGE_CHARGEMANAGE_STANDBY);
                   ((ApplayCharge *)chargeStandby)->switchToApplayCharge(canAddr);
                   ((ApplayCharge *)chargeStandby)->queryConfigSpecialFeatureSet();  //查询特殊功能设置　add by songqb 2017-6-14
                   snprintf(cmdBuff, sizeof(cmdBuff), "aplay %s &", START_CHARGE_APPLAY);
                   system(cmdBuff);
               }
               else   //多枪充电开启
               {
                   if(vinparam.coupleGun == 2) //VIN充电  2018-1-16 by songqb
                   {
                       ui->stackedtChargeManage->setCurrentIndex(PAGE_CHARGEMANAGE_VINSTART);
                       ((VINStartCharging *)chargeVINStart)->switchToVINApplayCharge(canAddr,1);
                   }
                   if(vinparam.coupleGun == 3) //刷卡和VIN充电  2018-1-16 by songqb
                   {
                       ui->stackedtChargeManage->setCurrentIndex(PAGE_CHARGEMANAGE_VINSTART);
                       ((VINStartCharging *)chargeVINStart)->switchToVINApplayCharge(canAddr,2);
                   }
                   if(vinparam.coupleGun == 1)
                   {
                       ui->stackedtChargeManage->setCurrentIndex(PAGE_CHARGEMANAGE_STANDBY);
                       ((ApplayCharge *)chargeStandby)->switchToApplayCharge(canAddr);
                       ((ApplayCharge *)chargeStandby)->queryConfigSpecialFeatureSet();  //查询特殊功能设置　add by songqb 2017-6-14
                       snprintf(cmdBuff, sizeof(cmdBuff), "aplay %s &", START_CHARGE_APPLAY);
                       system(cmdBuff);
                   }
               }
			}break;
		case CHARGE_STATUS_CHARGING:		//充电中状态
			{
				ui->stackedtChargeManage->setCurrentIndex(PAGE_CHARGEMANAGE_CHARGING);
				((ChargingReport *)chargingReport)->switchToCharging(canAddr);
                ((ChargingReport *)chargingReport)->queryChargeSpecialFeatureSet();
//				snprintf(cmdBuff, sizeof(cmdBuff), "aplay %s &", STOP_CHARGE_APPLAY);
//				system(cmdBuff);
			}break;
		case CHARGE_STATUS_FINISH:		//充电完成状态
		case CHARGE_STATUS_FULL:
			{
				ui->stackedtChargeManage->setCurrentIndex(PAGE_CHARGEMANAGE_FINISH);
				((ChargeReportFinish *)chargeFinishReport)->switchToChargeFinish(canAddr);
                ((ChargeReportFinish *)chargeFinishReport)->queryChargeFinishSpecialFeatureSet();
			}break;
		case CHARGE_STATUS_FAULT:        //故障状态
			{
				ui->stackedtChargeManage->setCurrentIndex(PAGE_CHARGEMANAGE_FAULT);
				((ChagingFailureInformation *)chargeFault)->switchToChargeFalut(canAddr);
			}break;
#if 0
		case CHARGE_STATUS_DISCONNECT:   //离线状态
			{
				StatusRemindWindow  *statusDialog= new StatusRemindWindow(this, 1, 5, QObject::tr("当前为未通信状态，请致电特来电客服4001-300-001"));
				statusDialog->exec();
				delete statusDialog;
			}break;
#endif
		default:
			{
				ui->stackedtChargeManage->setCurrentIndex(PAGE_CHARGEMANAGE_MAIN);
				maintainLock = false;
			}break;
	}
	currentPage = PAGE_CHARGERMANAGE;
	ui->stackedWidget->setCurrentIndex(currentPage);
}
/**
 *由充电流程界面之间的切换
 */
void TeuiMainWindow::slotChargeSwitch(int oldStatus, int newStatus, QVariant var)
{
	char cmdBuff[100];

    //qDebug() << "enter TeuiMainwindow slot back to mian ...............lll;;  ;" << newStatus;
	ui->stackedtChargeManage->setCurrentIndex(newStatus);
	switch(oldStatus)
	{
		case PAGE_CHARGEMANAGE_STANDBY:
			{
				if(newStatus == PAGE_CHARGEMANAGE_FLASHWAIT)		//检测到刷卡弹出正在申请卡片信息界面
				{
                    //qDebug() << "switch flash wait..........1";
                    ((FlashWaitWidget *)flashWait)->switchToFlashWait(this->canAddr, this->emergencyCharge, var.value<QString>(),30, 1, InfoExchangeApplayChargeResult );

					snprintf(cmdBuff, sizeof(cmdBuff), "aplay %s &", START_CHARGE_CARDINFO);
					system(cmdBuff);
				}
			}break;
		case PAGE_CHARGEMANAGE_FLASHWAIT:
			{
				if(newStatus == PAGE_CHARGEMANAGE_SELECTCHARGE)			//flash动画跳转到充电模式选择
				{
                    //qDebug()  << "...............................flash";
                    ((ChargeTypeSelect  *)chargeSelect )-> switchToChargeTypeSelect(var);

					snprintf(cmdBuff, sizeof(cmdBuff), "aplay %s &", START_CHARGE_TYPE);
					system(cmdBuff);
				}
			}break;
		case PAGE_CHARGEMANAGE_SELECTCHARGE:
			{
				if(newStatus == PAGE_CHARGEMANAGE_FLASHWAIT)			//充电方式选择切换到 flash 动画
				{
                    //qDebug() << "switch flash wait..........2";
                    ((FlashWaitWidget *)flashWait)->switchToFlashWait(this->canAddr,this->emergencyCharge, var.value<QString>(), 90, 1, InfoExchangeChargeResult);

					snprintf(cmdBuff, sizeof(cmdBuff), "aplay %s &", START_CHARGE_STARTING);
					system(cmdBuff);
				}
			}break;
		case PAGE_CHARGEMANAGE_CHARGING:
			{
				if(newStatus == PAGE_CHARGEMANAGE_FLASHWAIT)		//充电中检测到刷卡
				{
                    //qDebug() << "switch flash wait..........3";
                    ((FlashWaitWidget *)flashWait)->switchToFlashWait(this->canAddr, this->emergencyCharge,var.value<QString>(),90, 2, InfoExchangeChargeResult );
//					snprintf(cmdBuff, sizeof(cmdBuff), "aplay %s &", STOP_CHARGE_STOPING);
//					system(cmdBuff);
				}
			}break;
        case PAGE_CHARGEMANAGE_FINISH:
            {
                if(newStatus == PAGE_CHARGEMANAGE_STANDBY)         //充电报告继续充电
                {
                    ((ApplayCharge *)chargeStandby)->switchToApplayCharge(this->canAddr);
                    ((ApplayCharge *)chargeStandby)->queryConfigSpecialFeatureSet();  //查询特殊功能设置　add by songqb 2017-6-14
                }
                if(newStatus == PAGE_CHARGEMANAGE_PRINTPAPER)
                {
                    ((PrintPaper *)printPaper)->switchToPrintPaper(this->canAddr);
                }
            }break;
        case PAGE_CHARGEMANAGE_PRINTPAPER:
            {
                if(newStatus == PAGE_CHARGEMANAGE_FLASHWAIT)   //打印小票
                {
                    ((FlashWaitWidget *)flashWait)->switchToFlashWait(this->canAddr,this->emergencyCharge, var.value<QString>(), 90, 1, InfoExchangeChargeResult);
                }
            }break;
        case PAGE_CHARGEMANAGE_VINSTART:
            {
                if(newStatus == PAGE_CHARGEMANAGE_FLASHWAIT)
                {
                    ((FlashWaitWidget *)flashWait)->switchToFlashWait(this->canAddr,this->emergencyCharge, var.value<QString>(), 90, 1, InfoExchangeChargeResult);
                }
                if(newStatus == PAGE_CHARGEMANAGE_STANDBY)
                {
                    ((ApplayCharge *)chargeStandby)->switchToApplayCharge(canAddr);
                }
            }break;
		default:break;
	}

	if(newStatus == PAGE_CHARGEMANAGE_BASE)
	{
		((ChargeMangeBase * )chargeBase)->switchToBase(var.value<QString>(),PAGE_CHARGEMANAGE_MAIN , var);
	}

	if(newStatus == PAGE_CHARGEMANAGE_MAIN)
		maintainLock = false;
}

/**
 *升级数据导出界面切换
 */
void TeuiMainWindow::slotUpdateExportSwitch(int oldStatus, int newStatus, QVariant var)
{
    ui->stackedWidgetUpdate->setCurrentIndex(newStatus);
	switch(newStatus)
	{
		case PAGE_UPDATEEXPORT_FLASH:
			{
				((FlashExportLog *)exportFlash)->switchToFlashExport(QString(""), 200, var.value<unsigned char>() );
			}break;
		default:break;
	}
}
/**
 *主界面时间显示，发送查询终端数据
 */
void TeuiMainWindow::slotDisplayTime()
{
	ui->labelTitleSlave->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

	/*信号捕捉检测*/
	if( (this->teuiParam->capSignal) == 1 )
	{
		this->teuiParam->capSignal = 0;
      
        if(currentPage == PAGE_MAINTAIN || currentPage == PAGE_UPDATEEXPORT)
			emit sigClickLogo();
	}
	
	/*心跳处理*/
	heartCount ++;
	if(heartCount >= 5)
	{
		if(heartRespond == false && needCheckReconnect == true)
		{
			sigReconnect();
			needCheckReconnect = false;
            //qDebug() << "heart detect offline, socket reconnect.";
		}
		
		heartCount = 0;
		heartRespond = false;
	}
	
	/*发送查询状态指令*/
	InfoProtocol infoPro;
	infoPro.insert(InfoDataType, QByteArray(1, 0));
	infoPro.insert(InfoStatusCSCU, QByteArray());
	this->protocol->sendProtocolData(infoPro, InfoAddrStatus);

}
void TeuiMainWindow::receiveFromBus(InfoMap Map, InfoAddrType Type)
{
    emit sigFromBus(Map, Type);
}

void TeuiMainWindow::slotBusToOwn(InfoMap Map, InfoAddrType Type)
{
	if(Type == InfoAddrStatus)
	{
		if(Map.contains(InfoStatusCSCU))
		{
			QVariant var = Map.value(InfoStatusCSCU);
			stCSCUStatus cscuStatus  = var.value<stCSCUStatus>();

			heartRespond = true;
            this->emergencyCharge = cscuStatus.chargeMode;
			int needIndex=1;
            int needTypeIndex = 0;
			if(cscuStatus.serverStatus == 1)   //已连接服务器
				needIndex = 0;
			if(ui->stackedWidgetConnectstate->currentIndex() != needIndex)
				ui->stackedWidgetConnectstate->setCurrentIndex(needIndex);

            if(cscuStatus.chargeMode == 0)   //0正常充电 1进入应急充电 add by songqb 2018-1-29
                needTypeIndex = 1;
            if(ui->stackedWidgetEmergencyCharge->currentIndex() != needTypeIndex)
                ui->stackedWidgetEmergencyCharge->setCurrentIndex(needTypeIndex);

			/*u盘插入状态检测*/
			if(cscuStatus.udiskStatus == 1)
			{
				if(currentPage == PAGE_CHARGERMANAGE && maintainLock == false)
				{
					currentPage = PAGE_UPDATEEXPORT;
					ui->stackedWidget->setCurrentIndex(currentPage);
					ui->stackedWidgetUpdate->setCurrentIndex(PAGE_UPDATEEXPORT_MAIN);
				}
			}
		}
	}
	else if(Type == InfoAddrConfig)
	{
		if(Map.contains(InfoConfigPassword))
		{

			QVariant var = Map.value(InfoConfigPassword);
			password  = var.value<int>();
		}

        else if(Map.contains(InfoConfigSpecialFunc))
        {
            QVariant var = Map.value(InfoConfigSpecialFunc);
            stSpecialFunc param = var.value<stSpecialFunc>();
            vinparam = param;
//            if(vinparam.languageSelect ==2)
//            {
//                translator = new QTranslator(this);
//                translator->load("/mnt/nandflash/bin/Language_Eng");
//                qApp->installTranslator(translator);
//                ui->retranslateUi(this);
//            }
        }
	}
}

/**
 * @brief 点击Logo
 */
void TeuiMainWindow::on_buttonLogo_clicked()
{
    //qDebug() << "on_buttonLogo_clicked" << clickConnect;
	if(widget)
	{
		delete widget;
		widget = NULL;
	}

    if(currentPage == PAGE_CHARGERMANAGE && maintainLock == false)
    {
		/*发送查询登陆密码指令*/
		password = 0;
		InfoProtocol infoPro;
		infoPro.insert(InfoDataType, QByteArray(1, 0));
		infoPro.insert(InfoConfigPassword, QByteArray());
		this->protocol->sendProtocolData(infoPro, InfoAddrConfig);

		/*弹出登陆窗口*/	
        PasswordLogin *loginDialog = new PasswordLogin(this);
        int retValue = loginDialog->exec();
		if(retValue == 1)
		{
			QString inputPassword = loginDialog->getInputPassword();
            //qDebug() << "The logo passwperd  is ........."  << QString::number(password, 10);
			if(((QString::number(password, 10) == inputPassword) && ( inputPassword.length() == 6 )))      //密码正确
			{
                currentPage = PAGE_MAINTAIN;
                ui->stackedWidget->setCurrentIndex(currentPage);
				((Maintain *)maintainWidget)->switchToMaintain();
			}
			else
			{
				StatusRemindWindow  *statusDialog= new StatusRemindWindow(ui->pageBase, 1, 5, QObject::tr("密码错误"));
				statusDialog->exec();
				delete statusDialog;
			}
		}
        delete loginDialog;
    }
    else if( currentPage == PAGE_MAINTAIN )			//当前页面为运维维护界面
    {
        currentPage = PAGE_CHARGERMANAGE;
        ui->stackedWidget->setCurrentIndex(currentPage);
        ui->stackedtChargeManage->setCurrentIndex(PAGE_CHARGEMANAGE_MAIN);

        ((MainCharge *)mainChargeWidget)->queryConfig();  //重新查询配置信息
        ((MainCharge *)mainChargeWidget)->queryConfigSpecialFeatureSet();  //重新查询特殊功能设置信息  用于VIN后6位充电 2017-3-28 by songqb
    }
	else if( currentPage == PAGE_UPDATEEXPORT )		//当前界面为数据导出，升级界面
    {
        currentPage = PAGE_CHARGERMANAGE;
        ui->stackedtChargeManage->setCurrentIndex(PAGE_CHARGEMANAGE_MAIN);
        ui->stackedWidget->setCurrentIndex(currentPage);

        ((MainCharge *)mainChargeWidget)->queryConfig();
       // ((MainCharge *)mainChargeWidget)->queryConfigSpecialFeatureSet();
    }
}
