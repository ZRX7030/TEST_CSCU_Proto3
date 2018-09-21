#include <QDebug>
#include "SpecialFeatureSet.h"
#include "StatusRemindWindow.h"
#include "ui_SpecialFeatureSet.h"

SpecialFeatureSet::SpecialFeatureSet(QWidget *parent, CBus *bus, ProtocolBase *protocol, void *param) :
    QWidget(parent),
    ui(new Ui::SpecialFeatureSet)
{
    ui->setupUi(this);
    teuiParam = (stTeuiParam *)param;

    ui->labelCardAutoApplyCharging->hide();
    ui->comboBoxCardAutoApplyCharging->hide();  //隐藏刷卡自动充电
    ui->labelNetworkoutVINCharging->hide();
    ui->comboBoxNetworkoutVINCharging->hide();   //隐藏VIN后6位充电

	this->bus = bus;
	this->protocol = protocol;
	
	/*注册数据*/
	QList<InfoAddrType> list;
	list.append(InfoAddrConfig);
    list.append(InfoAddrExchange);
	this->bus->registDev(this, list);
    
	QObject::connect(this,SIGNAL(sigFromBus(InfoMap, InfoAddrType)), this, SLOT(slotBusToOwn(InfoMap, InfoAddrType)), Qt::QueuedConnection);
    
	/*发送特殊更能设置查询*/
	InfoProtocol infoPro;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoConfigSpecialFunc, QByteArray());
    this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
}

SpecialFeatureSet::~SpecialFeatureSet()
{
	this->bus->cancelRegistDev(this);
    delete ui;
}

void SpecialFeatureSet::receiveFromBus(InfoMap Map, InfoAddrType Type)
{
	emit sigFromBus(Map, Type);
}

void SpecialFeatureSet::slotBusToOwn(InfoMap Map, InfoAddrType Type)
{
    //qDebug() << " SpecialFeatureSet receive data\n";
	if(Type == InfoAddrConfig)
	{
		if(false == Map.contains(InfoConfigSpecialFunc))
			return;
		QVariant var = Map.value(InfoConfigSpecialFunc);

		stSpecialFunc param = var.value<stSpecialFunc>();
        if(param.cardAuto == 1)
            ui->comboBoxCardAutoApplyCharging->setCurrentIndex(0);
        else
            ui->comboBoxCardAutoApplyCharging->setCurrentIndex(1);

        if(param.cardType < 9)
            ui->comboBoxCardTypeChoose->setCurrentIndex(param.cardType);
        else
            ui->comboBoxCardTypeChoose->setCurrentIndex(0);

        if(param.energyFilter == 1)
            ui->comboBoxAbnormalElectricPowerFilter->setCurrentIndex(0);
        else
            ui->comboBoxAbnormalElectricPowerFilter->setCurrentIndex(1);
        
		if(param.localStop == 1)
            ui->comboBoxTerminalInforEndButton->setCurrentIndex(0);
        else
            ui->comboBoxTerminalInforEndButton->setCurrentIndex(1);

        if(param.vinAuto == 1)
            ui->comboBoxVINAutoCharging->setCurrentIndex(0);
        else
            ui->comboBoxVINAutoCharging->setCurrentIndex(1);

        if(param.printPaper == 1)           //add by songqb
            ui->comboBoxPrintTicket->setCurrentIndex(0);
        else
            ui->comboBoxPrintTicket->setCurrentIndex(1);
        
		/*断网后VIN启动充电*/
        if(param.vinOffline == 1)
            ui->comboBoxNetworkoutVINCharging->setCurrentIndex(0);
        else
            ui->comboBoxNetworkoutVINCharging->setCurrentIndex(1);
        //vin或车牌号
        if(param.vinType < 3)
            ui->comboBoxVINNumberOrLicenseNumber->setCurrentIndex(param.vinType);
        else
            ui->comboBoxVINNumberOrLicenseNumber->setCurrentIndex(0);

        //add by yanwei 20171011 单桩/群充
        if(param.pileType == 1)
            ui->PileTypeSetBox->setCurrentIndex(0);
        else
            ui->PileTypeSetBox->setCurrentIndex(1);
		
		memcpy(&special_set,&param,sizeof(param));  //nihai add		//merge by yanwei 20171011 
	}
    else if(Type == InfoAddrExchange)
    {
        if(false == Map.contains(InfoExchangeParamResult))
            return;
        QVariant var = Map.value(InfoExchangeParamResult);
        stExchangeResult result = var.value<stExchangeResult>();

        if(result.type == InfoConfigSpecialFunc)  //back cscu config reuslt
        {
            int flag = 1;
            int time = 5;
            QString text;

            if(result.result == 0)
            {
                flag = 1;
                text = QObject::tr("特殊功能设置失败");
            }
            else
            {
                flag = 0;
                text = QObject::tr("特殊功能设置成功");
            }

            if(result.rebootFlag == 1)   //add by songqb  2017-5-23
            {
                time = 10;
                text.append(QObject::tr("系统正在重启，请稍等！"));
                teuiParam->showTermWin = false;
            }
            StatusRemindWindow  *statusDialog = new StatusRemindWindow(this, flag, time, text);
            statusDialog->exec();
            delete statusDialog;
        }
    }
}

void SpecialFeatureSet::on_buttonSave_clicked()
{
    InfoProtocol infoPro;
    stSpecialFunc param;

    if(ui->comboBoxCardAutoApplyCharging->currentIndex() == 0)
        param.cardAuto = 1;
    else
        param.cardAuto = 0;

    param.cardType = ui->comboBoxCardTypeChoose->currentIndex();

    if(ui->comboBoxAbnormalElectricPowerFilter->currentIndex() == 0)
        param.energyFilter = 1;
    else
        param.energyFilter = 0;
    
    if(ui->comboBoxTerminalInforEndButton->currentIndex() == 0)
        param.localStop = 1;
    else
        param.localStop = 0;

    if(ui->comboBoxVINAutoCharging->currentIndex() == 0)
        param.vinAuto = 1;
    else
        param.vinAuto = 0;

    if(ui->comboBoxPrintTicket->currentIndex() == 0)   //add by songqb 2-17-10-15
        param.printPaper = 1;
    else
        param.printPaper = 0;
    
	/*断网后VIN启动充电*/
    if(ui->comboBoxNetworkoutVINCharging->currentIndex() == 0 )
        param.vinOffline = 1;
    else
        param.vinOffline = 0;
    //vin或车牌号
    param.vinType = ui->comboBoxVINNumberOrLicenseNumber->currentIndex();

    //add by yanwei 20171011 单桩/群充
    if(ui->PileTypeSetBox->currentIndex() == 0)
        param.pileType = 1;
    else
        param.pileType = 0;

	//merge by yanwei 20171011
	//界面没有的数据，保留下设  nihai add
    param.chargeMode = special_set.chargeMode;
    param.localType = special_set.localType;
    param.coupleGun = special_set.coupleGun;
    param.languageSelect = special_set.languageSelect;
	infoPro.insert(InfoDataType, QByteArray(1, 1));
    infoPro.insert(InfoConfigSpecialFunc, QByteArray((char *)&param, sizeof(stSpecialFunc)));
    this->protocol->sendProtocolData(infoPro, InfoAddrConfig);
}
