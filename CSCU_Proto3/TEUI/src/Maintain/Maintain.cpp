#include <QDebug>

#include "Maintain.h"
#include "TeuiMainWindow.h"
#include "ui_Maintain.h"
#include "MaintainCSCUSet.h"
#include "ChargingRecord.h"
#include "BMSInformation.h"
#include "LineSideInformation.h"
#include "ActriphaseSet.h"
#include "SpecialFeatureSet.h"
#include "LoadConstraintSet.h"
#include "TerminalInformation.h"
#include "PeakChargingInformationView.h"
#include "FailureInformation.h"
#include "PasswordSet.h"
#include "VersionInformation.h"
#include "SubStationEnMonitoringInformation.h"
#include "ModuleInformation.h"
#include "OperateRecordInformation.h"
#include "QRcodeCreate.h"
#include "DCChargerSet.h"
#include "DCChargerData.h"
#include "SystemTimeSet.h"
#include "StartTypeSelect.h"
#include "LogoSet.h"
#include "ChargeModeSelect.h"
#include "ChargePassword.h"
#include "RealtimeFaultInformation.h"
#include "CoupleGunSet.h"
#include "LanguageSelect.h"


Maintain::Maintain(QWidget *parent, CBus *bus, ProtocolBase *protocol, void *param) :
    QWidget(parent),
    ui(new Ui::Maintain)
{
    ui->setupUi(this);
	teuiParam = (stTeuiParam *)param;

    ui->ModuleInformationButton->hide();    // 隐藏模块信息
    //ui->DCChargerDataButton->hide();
    //ui->DCChargerSetButton->hide();
    //ui->StartTypeSelect->hide();
    //ui->LogoSetButton->hide();  //隐藏版本选择

	funcWidget = NULL;
	this->bus = bus;
	this->protocol = protocol;
    QObject::connect(this,SIGNAL(sigShowDefault()), this, SLOT(on_versionButton_clicked()));
}

Maintain::~Maintain()
{
    //this->bus->cancelRegistDev(this);
    delete ui;
}

QRect Maintain::getFillSize()
{
    QRect rect  = ui->maintainSpacerFill->geometry();
    qDebug() << rect;
    return QRect(rect.left(), rect.top(), rect.width(), rect.height());
}

void Maintain::switchToMaintain(void)
{
    ui->toolBox->setCurrentIndex(4);			//版本信息界面
	emit sigShowDefault();
}

void Maintain::mousePressEvent(QMouseEvent *event)
{
    //qDebug() << "MainCharge: mouse press event trigger xy" << event->x() << event->y();
    QWidget::mousePressEvent(event);
}

/**
 * @brief 集控设置
 */
void Maintain::on_cscuSetButton_clicked()
{
	if(funcWidget)
	{
        delete funcWidget;
		funcWidget = NULL;
	}
   
    funcWidget = new MaintainCSCUSet(this, bus, protocol, teuiParam);
    QRect rect = getFillSize();
    funcWidget->setGeometry(rect);
    //mainTainCSCU->setWindowOpacity(1); //窗口整体透明度，0-1 从全透明到不透明
    //mainTainCSCU->setAttribute(Qt::WA_TranslucentBackground); //设置背景透明，允许鼠标穿透
    funcWidget->show();
}

/**
 * @brief 充电记录查询
 */
void Maintain::on_chargeRecordButton_clicked()
{
    //qDebug() << "on_chargeRecordButton_clicked";
    if(funcWidget)
	{
        delete funcWidget;
		funcWidget = NULL;
	}

    funcWidget = new ChargingRecord(this, bus, protocol);
    QRect rect = getFillSize();
    funcWidget->setGeometry(rect);

    //funcWidget->setWindowOpacity(1); //窗口整体透明度，0-1 从全透明到不透明
    //funcWidget->setAttribute(Qt::WA_TranslucentBackground); //设置背景透明，允许鼠标穿透
    funcWidget->show();
}
/**
 * @brief  终端bms数据查询
 */
void Maintain::on_BMSInformationButton_clicked()
{
    //qDebug() << "on_BMSInformationButton_clicked";
    if(funcWidget)
	{
        delete funcWidget;
		funcWidget = NULL;
	}

    funcWidget = new BMSInformation(this, bus, protocol);
    QRect rect = getFillSize();
    funcWidget->setGeometry(rect);
    funcWidget->show();
}
/**
 * @brief  进线侧电表数据
 */
void Maintain::on_UserLineSideInformationButton_clicked()
{
    //qDebug() << "on_UserLineSideInformationButton_clicked";
	if(funcWidget)
	{
        delete funcWidget;
		funcWidget = NULL;
	}
    
    funcWidget = new LineSideInformation(this, bus, protocol);
    QRect rect = getFillSize();
    funcWidget->setGeometry(rect);
    funcWidget->show();
}
/**
 * @brief 三相相别设置
 */
void Maintain::on_ActriphaseSetButton_clicked()
{
    //qDebug() << "on_ActriphaseSetButton_clicked";
	if(funcWidget)
	{
        delete funcWidget;
		funcWidget = NULL;
	}
    
    funcWidget = new ActriphaseSet(this, bus, protocol);
    QRect rect = getFillSize();
    funcWidget->setGeometry(rect);
    funcWidget->show();
}
/**
 * @brief 特殊功能设置
 */
void Maintain::on_SpecialFeatureButton_clicked()
{
    //qDebug() << "on_SpecialFeatureButton_clicked";
    if(funcWidget)
    {
        delete funcWidget;
        funcWidget = NULL;
    }
    funcWidget = new SpecialFeatureSet(this, bus, protocol,teuiParam);
    QRect rect = getFillSize();
    funcWidget->setGeometry(rect);
    funcWidget->show();
}
/**
 * @brief 负荷约束设置
 */
void Maintain::on_LoadConstraintSetButton_clicked()
{
    //qDebug() << "on_LoadConstraintSetButton_clicked";
	if(funcWidget)
	{
        delete funcWidget;
		funcWidget = NULL;
	}
    funcWidget = new LoadConstraintSet(this, bus, protocol);
    QRect rect = getFillSize();
    funcWidget->setGeometry(rect);
    funcWidget->show();
}
/**
 * @brief 终端实时数据查询
 */
void Maintain::on_TerminalInformationButton_clicked()
{
    //qDebug() << "on_TerminalInformationButton_clicked";
    if(funcWidget)
	{
        delete funcWidget;
		funcWidget = NULL;
	}
	
    funcWidget = new TerminalInformation(this,bus, protocol);
    QRect rect = getFillSize();
    funcWidget->setGeometry(rect);
    funcWidget->show();
}
/**
 * @brief 错峰充电设置
 */
void Maintain::on_PeakChargingFeatureSetButton_clicked()
{
   // qDebug() << "on_PeakChargingFeatureSetButton_clicked";
	if(funcWidget)
	{
        delete funcWidget;
		funcWidget = NULL;
	}
    funcWidget = new PeakChargingInformationView(this,bus, protocol);
    QRect rect = getFillSize();
    funcWidget->setGeometry(rect);
    funcWidget->show();
}
/**
 * @brief 系统时间设置
 */
void Maintain::on_TimeSetButton_clicked()
{
    //qDebug() << "on_TimeSetButton_clicked";
    if(funcWidget)
    {
        delete funcWidget;
        funcWidget = NULL;
    }

    funcWidget = new SystemTimeSet(this,bus, protocol);
    QRect rect = getFillSize();
    funcWidget->setGeometry(rect);
    funcWidget->show();
}
/**
 * @brief 故障信息
 */
void Maintain::on_FailureInformationButton_clicked()
{
    //qDebug() << "on_FailureInformationButton_clicked";
	if(funcWidget)
	{
        delete funcWidget;
		funcWidget = NULL;
	}
    
	funcWidget = new FailureInformation(this,bus, protocol);
    QRect rect = getFillSize();
    funcWidget->setGeometry(rect);
    funcWidget->show();
}

/**
 * @brief 修改密码
 */
void Maintain::on_PasswordSetButton_clicked()
{
    //qDebug() << "on_PasswordSetButton_clicked";
    if(funcWidget)
    {
        delete funcWidget;
        funcWidget = NULL;
    }

    funcWidget = new PasswordSet(this,bus, protocol);
    QRect rect = getFillSize();
    funcWidget->setGeometry(rect);
    funcWidget->show();
}

/**
 * @brief 修改本地充电密码
 */
void Maintain::on_ChargePassword_clicked()
{
    //qDebug() << "on_ChargePasswordSetButton_clicked";
    if(funcWidget)
    {
        delete funcWidget;
        funcWidget = NULL;
    }

    funcWidget = new ChargePassword(this,bus, protocol);
    QRect rect = getFillSize();
    funcWidget->setGeometry(rect);
    funcWidget->show();
}

/**
 * @brief 版本信息
 */
void Maintain::on_versionButton_clicked()
{
    //qDebug() << "on_versionButton_clicked";
    if(funcWidget)
    {
        delete funcWidget;
        funcWidget = NULL;
    }

    funcWidget = new VersionInformation(this,bus, protocol);
    QRect rect = getFillSize();
    funcWidget->setGeometry(rect);
    funcWidget->show();
}
/**
 * @brief 子站环境信息
 */
void Maintain::on_SubStationEnInformationButton_clicked()
{
    //qDebug() << "on_SubStationEnInformationButton_clicked";
    if(funcWidget)
    {
        delete funcWidget;
        funcWidget = NULL;
    }

    funcWidget = new SubStationEnMonitoringInformation(this,bus, protocol);
    QRect rect = getFillSize();
    funcWidget->setGeometry(rect);
    funcWidget->show();
}

/**
 * @brief 模块信息
 */
void Maintain::on_ModuleInformationButton_clicked()
{
    //qDebug() << "on_ModuleInformationButton_clicked";
    if(funcWidget)
    {
        delete funcWidget;
        funcWidget = NULL;
    }

    funcWidget = new ModuleInformation(this,bus, protocol);
    QRect rect = getFillSize();
    funcWidget->setGeometry(rect);
    funcWidget->show();
}
/**
 * @brief 操作记录
 */
void Maintain::on_operateRecordButton_clicked()
{
    //qDebug() << "on_operateRecordButton_clicked";
    if(funcWidget)
    {
        delete funcWidget;
        funcWidget = NULL;
    }

    funcWidget = new OperateRecordInformation(this,bus, protocol);
    QRect rect = getFillSize();
    funcWidget->setGeometry(rect);
    funcWidget->show();
}
/**
 * @brief 二维码生成
 */
void Maintain::on_QRcodeCreate_clicked()
{
    //qDebug() << "on_QRcodeCreate_clicked";
    if(funcWidget)
    {
        delete funcWidget;
        funcWidget = NULL;
    }
    funcWidget = new QRcodeCreate(this,bus, protocol);
    QRect rect = getFillSize();
    funcWidget->setGeometry(rect);
    funcWidget->show();
}
/**
 * @brief Maintain::on_DCChargerDataButton_clicked
 */
void Maintain::on_DCChargerDataButton_clicked()
{
    if(funcWidget)
    {
        delete funcWidget;
        funcWidget = NULL;
    }

    funcWidget = new DCChargerData(this,bus, protocol);
    QRect rect = getFillSize();
    funcWidget->setGeometry(rect);
    funcWidget->show();
}
/**
 * @brief Maintain::on_DCChargerSetButton_clicked
 */
void Maintain::on_DCChargerSetButton_clicked()
{
    if(funcWidget)
    {
        delete funcWidget;
        funcWidget = NULL;
    }

    funcWidget = new DCChargerSet(this,bus, protocol);
    QRect rect = getFillSize();
    funcWidget->setGeometry(rect);
    funcWidget->show();
}
/**
 * @brief Maintain::充电模式选择
 */
void Maintain::on_StartTypeSelect_clicked()
{
    if(funcWidget)
    {
        delete funcWidget;
        funcWidget = NULL;
    }

    funcWidget = new ChargeModeSelect(this,bus, protocol);
    QRect rect = getFillSize();
    funcWidget->setGeometry(rect);
    funcWidget->show();
}
/**
 * @brief 系统标题设置
 */

void Maintain::on_LogoSetButton_clicked()
{
    if(funcWidget)
	{
        delete funcWidget;
        funcWidget = NULL;
    }
    funcWidget = new LogoSet(this,bus, protocol);
    QRect rect = getFillSize();
    funcWidget->setGeometry(rect);
    funcWidget->show();
}

/**
 * @brief实时故障信息
 */
void Maintain::on_RealTimeFaultBtn_clicked()
{
    if(funcWidget)
    {
        delete funcWidget;
        funcWidget = NULL;
    }
    funcWidget = new RealtimeFaultInformation(this,bus, protocol);
    QRect rect = getFillSize();
    funcWidget->setGeometry(rect);
    funcWidget->setFixedWidth(rect.width());
    funcWidget->show();
}
/**
 * @brief 多枪设置
 */
void Maintain::on_CoupleGunSetButton_clicked()
{
    if(funcWidget)
    {
        delete funcWidget;
        funcWidget = NULL;
    }
    funcWidget = new CoupleGunSet(this,bus, protocol);
    QRect rect = getFillSize();
    funcWidget->setGeometry(rect);
    funcWidget->show();
}

void Maintain::on_languageSelectButton_clicked()
{
    if(funcWidget)
    {
        delete funcWidget;
        funcWidget = NULL;
    }
    funcWidget = new LanguageSelect(this,bus, protocol,teuiParam);
    QRect rect = getFillSize();
    funcWidget->setGeometry(rect);
    funcWidget->show();
}

void Maintain::on_toolBox_currentChanged(int index)   //add by songqb
{
    switch (index) {
        case 0:
        {
            on_cscuSetButton_clicked();
        }break;
        case 1:
        {
            on_SpecialFeatureButton_clicked();
        }break;
        case 2:
        {
            on_TerminalInformationButton_clicked();
        }break;
        case 3:
        {
            on_FailureInformationButton_clicked();
        }break;
        default:
        {
            on_versionButton_clicked();
        }break;
    }
}


