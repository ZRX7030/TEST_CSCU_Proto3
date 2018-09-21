#include "LanguageSelect.h"
#include "ui_LanguageSelect.h"
#include <QFile>
#include <QTextStream>
#include "StatusRemindWindow.h"

LanguageSelect::LanguageSelect(QWidget *parent, CBus * bus, ProtocolBase *protocol,void *param) :
    QWidget(parent),
    ui(new Ui::LanguageSelect)
{
    ui->setupUi(this);
    teuiParam = (stTeuiParam *)param;

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

LanguageSelect::~LanguageSelect()
{
    this->bus->cancelRegistDev(this);
    delete ui;
}

void LanguageSelect::receiveFromBus(InfoMap Map, InfoAddrType Type)
{
     emit sigFromBus(Map, Type);
}

void LanguageSelect::slotBusToOwn(InfoMap Map, InfoAddrType Type)
{
    if(Type == InfoAddrConfig)
    {
        if(Map.contains(InfoConfigSpecialFunc))
        {
            QVariant var = Map.value(InfoConfigSpecialFunc);
            stSpecialFunc param = var.value<stSpecialFunc>();
            memcpy(&special_set,&param,sizeof(param));
            if(param.languageSelect == 1)
                ui->comboBoxLanguage->setCurrentIndex(0);
            else
                ui->comboBoxLanguage->setCurrentIndex(1);
        }
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

void LanguageSelect::on_buttonSave_clicked()
{
    InfoProtocol infoPro;
    stSpecialFunc param;
    param.cardAuto = special_set.cardAuto;
    param.cardType = special_set.cardType;
    param.energyFilter = special_set.energyFilter;
    param.localStop = special_set.localStop;
    param.vinAuto = special_set.vinAuto;
    param.vinOffline = special_set.vinOffline;
    param.vinType = special_set.vinType;
    param.pileType = special_set.pileType;
    param.coupleGun = special_set.coupleGun;
    param.printPaper = special_set.printPaper;
    param.chargeMode = special_set.chargeMode;
    param.localType =special_set.localType;
    if(ui->comboBoxLanguage->currentIndex() == 0)
        param.languageSelect = 1;
    else
        param.languageSelect = 2;


    infoPro.insert(InfoDataType, QByteArray(1, 1));
    infoPro.insert(InfoConfigSpecialFunc, QByteArray((char *)&param, sizeof(stSpecialFunc)));
    this->protocol->sendProtocolData(infoPro, InfoAddrConfig);

    int number = 0;
    if(ui->comboBoxLanguage->currentIndex() == 0)
        number = 1;
    else
        number = 2;
    QFile file("/mnt/nandflash/etc/Language.conf");
    if(!file.open(QFile::WriteOnly | QFile::Text))
    {
        return;
    }
    else
    {
        QTextStream out(&file);        //写文件内容
        out << number << "\n";
    }

    file.close();
}
