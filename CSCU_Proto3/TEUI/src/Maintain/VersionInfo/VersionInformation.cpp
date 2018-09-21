#include <QDebug>

#include "InfoData.h"
#include "VersionInformation.h"
#include "ui_VersionInformation.h"


VersionInformation::VersionInformation(QWidget *parent, CBus *bus, ProtocolBase *protocol) :
    QWidget(parent),
    ui(new Ui::VersionInformation)
{
    ui->setupUi(this);
    this->bus = bus;
    this->protocol = protocol;

    /*注册数据*/
    QList<InfoAddrType> list;
    list.append(InfoAddrMaintain);
    this->bus->registDev(this, list);

    QObject::connect(this,SIGNAL(sigFromBus(InfoMap, InfoAddrType)), this, SLOT(slotBusToOwn(InfoMap, InfoAddrType)));
    ui->labelTEUIProgramVersion->setText(QString("TEUI_A5_A_26044_G0"));    //teui程序版本

    /*发送查询版本信息命令*/
    InfoProtocol infoPro;
    infoPro.insert(InfoDataType, QByteArray(1, 0));
    infoPro.insert(InfoMaintainVersion,QByteArray());
    this->protocol->sendProtocolData(infoPro,InfoAddrMaintain);
}

VersionInformation::~VersionInformation()
{
    this->bus->cancelRegistDev(this);
    delete ui;
}

/**
 * @brief 接收到总线过来的数据

 */
void VersionInformation::receiveFromBus(InfoMap Map, InfoAddrType type)
{
     emit sigFromBus(Map, type);
}

void VersionInformation::slotBusToOwn(InfoMap Map, InfoAddrType Type)
{
	if(Type == InfoAddrMaintain)
	{
        //qDebug() << "receive version information.................";
		if(false == Map.contains(InfoMaintainVersion))
            return;

        QVariant var = Map.value(InfoMaintainVersion);
        stVersionInformation data = var.value<stVersionInformation>();
        showVersionInfo(data);
    }
}

/**
 *显示版本信息
 */
void VersionInformation::showVersionInfo(stVersionInformation data)
{
    ui->labelKernelVersion->setText(QString(data.kernelVersion));       //内核版本
    ui->labelFileSysVersion->setText(QString(data.fileSysVersion));  //文件系统版本
    ui->labelCSCUProgramVersion->setText(QString(data.cscuProgram));    //CSCU程序版本
    ui->labelHardwareVersion->setText(QString(data.hardwareVersion));   //硬件版本
    ui->labelMACAddress->setText(QString(data.macAddress));             //MAC地址
    ui->labelnum->setText(QString(data.serialNumber));               //序列号
}
