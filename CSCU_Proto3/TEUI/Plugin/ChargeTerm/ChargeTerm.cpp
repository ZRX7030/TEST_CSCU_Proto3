#include <stdlib.h>
#include <QDebug>

#include "ChargeTerm.h"
#include "ui_ChargeTerm.h"

ChargeTerm::ChargeTerm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChargeTerm)
{
    ui->setupUi(this);
    memset(&chargeStatus, 0, sizeof(stChargeTermData));
    chargeStatus.status = -1;

    flashTimer = NULL;
    timerCount = 0;
}

ChargeTerm::~ChargeTerm()
{
    if(flashTimer)
    {
        delete flashTimer;
        flashTimer = NULL;
    }
    delete ui;
}

void ChargeTerm::singleChargeTermSet()
{
    ui->layoutStatus->setStretch(0, 3);
    ui->layoutStatus->setStretch(1, 1);
    ui->layoutStatus->setStretch(2, 3);
    ui->layoutStatus->setStretch(3, 9);
    ui->layoutStatus->setStretch(4, 1);
    ui->layoutStatus->setStretch(5, 1);
    ui->layoutStatus->setStretch(6, 2);
    ui->layoutStatus->setStretch(7, 3);
   // ui->labelSOC->setMinimumHeight(36);

    QFont ftSOC;
    ftSOC.setPointSize(20);
    ui->labelSOC->setFont(ftSOC);
    QFont ftCanId;
    ftCanId.setPointSize(20);
    ui->labelCANID->setFont(ftCanId);
    QFont ftState;
    ftState.setPointSize(16);
    ui->labelState->setFont(ftState);
}

void ChargeTerm::flashOverTimer()
{
    timerCount++;
    if(timerCount > 5)
        timerCount = 1;
    ui->stackedWidgetStatepicture->setCurrentIndex(LABEL_INDEX_CHARGEING1 + timerCount - 1);
}

void ChargeTerm::mousePressEvent(QMouseEvent *event)
{
    //qDebug() << "ChargeTerm::eventFilter canadd=" << chargeStatus.canAddr;
    emit signalChargerClicked(chargeStatus.canAddr, chargeStatus.status);
    QWidget::mousePressEvent(event);
}
/**
 * @brief 充电数据发生了改变
 * @param chargeData
 */
void ChargeTerm::updateChargeStatus(stChargeTermData chargeData)
{
    /*充电状态发生变化*/
    if(QString(chargeData.name) != QString(chargeStatus.name))
        ui->labelCANID->setText(QObject::tr(chargeData.name));

    if(chargeData.linkStatus == 2)       //充电弓
    {
        ui->stackedWidgetStatepicture->setCurrentIndex(LABEL_INDEX_OFFLINE);
        ui->labelOffline->setText("");
        switch(chargeData.status)
        {
            case CHARGE_STATUS_FREE:                //待机
            {
                ui->widgetChargeTerminal->setStyleSheet(QString::fromUtf8("border-image: url(:/bowfree.png);"));
                ui->labelState->setText(QString(QObject::tr("待机")));
            }break;
            case CHARGE_STATUS_GUN_STANDBY:         //连接就绪
            {
                ui->widgetChargeTerminal->setStyleSheet(QString::fromUtf8("border-image: url(:/bowlink.png);"));
                ui->labelState->setText(QString(QObject::tr("连接就绪")));
            }break;
            case CHARGE_STATUS_STARTING:            //启动中
            {
                ui->widgetChargeTerminal->setStyleSheet(QString::fromUtf8("border-image: url(:/bowstart.png);"));
                ui->labelState->setText(QString(QObject::tr("启动中")));
            }break;
            case CHARGE_STATUS_CHARGING:            //充电中
            {
                ui->widgetChargeTerminal->setStyleSheet(QString::fromUtf8("border-image: url(:/bowcharge.png);"));
                ui->labelState->setText(QString(QObject::tr("充电中")));
            }break;
            case CHARGE_STATUS_FINISH:              //充电完成
            case CHARGE_STATUS_FULL:
            {
                ui->widgetChargeTerminal->setStyleSheet(QString::fromUtf8("border-image: url(:/bowfinish.png);"));
                ui->labelState->setText(QString(QObject::tr("充电完成")));
            }break;
            case CHARGE_STATUS_FAULT:               //故障
            {
                ui->widgetChargeTerminal->setStyleSheet(QString::fromUtf8("border-image: url(:/bowfault.png);"));
                ui->labelState->setText(QString(QObject::tr("故障")));
            }break;
            case CHARGE_STATUS_DISCONNECT:      //离线
            {
                ui->widgetChargeTerminal->setStyleSheet(QString::fromUtf8("border-image: url(:/bowoff.png);"));
                ui->labelState->setText(QString(QObject::tr("离线")));
            }break;
#if 0
            case CHARGE_STATUS_SLAVEGUN:
            {
                ui->stackedWidgetStatepicture->setCurrentIndex(LABEL_INDEX_LINK);
                ui->labelState->setText(QString(QObject::tr("副枪")));
            }break;
            case CHARGE_STATUS_COUPLE_ERR:
            {
                ui->stackedWidgetStatepicture->setCurrentIndex(LABEL_INDEX_LINK);
                ui->labelState->setText(QString(QObject::tr("配对错误")));
            }break;
            case CHARGE_STATUS_QUEUE1:
            case CHARGE_STATUS_QUEUE2:
            case CHARGE_STATUS_QUEUE3:
            case CHARGE_STATUS_QUEUE4:
            case CHARGE_STATUS_QUEUE5:
            case CHARGE_STATUS_QUEUE6:
            case CHARGE_STATUS_QUEUE7:
            {
                ui->stackedWidgetStatepicture->setCurrentIndex(LABEL_INDEX_QUEUE);
                ui->labelState->setText(QString(QObject::tr("排队"))+ QString::number(chargeData.status-14, 10));
            }break;
#endif
            default:
            {
                ui->widgetChargeTerminal->setStyleSheet(QString::fromUtf8("border-image: url(:/bowfree.png);"));
                ui->labelState->setText(QString(QObject::tr("未定义状态")));
            }break;
        }
    }
    else
    {
        if(chargeData.status != chargeStatus.status)
        {
            if(flashTimer)
            {
                delete flashTimer;
                flashTimer = NULL;
            }

            if(chargeData.status == CHARGE_STATUS_FAULT)
                ui->widgetChargeTerminal->setStyleSheet(QString::fromUtf8("border-image: url(:/chargefalut.png);"));
            else if(chargeData.status == CHARGE_STATUS_DISCONNECT)
                ui->widgetChargeTerminal->setStyleSheet(QString::fromUtf8("border-image: url(:/ofline.png);"));
            else if(chargeStatus.status == CHARGE_STATUS_FAULT || chargeStatus.status == CHARGE_STATUS_DISCONNECT)
                ui->widgetChargeTerminal->setStyleSheet(QString::fromUtf8("border-image: url(:/charge.png);"));
            switch(chargeData.status)
            {
                case CHARGE_STATUS_FREE:                //待机
                {
                    ui->stackedWidgetStatepicture->setCurrentIndex(LABEL_INDEX_SLEEP);
                    ui->labelState->setText(QString(QObject::tr("待机")));
                }break;
                case CHARGE_STATUS_GUN_STANDBY:         //连接就绪
                {
                    ui->stackedWidgetStatepicture->setCurrentIndex(LABEL_INDEX_LINK);
                    ui->labelState->setText(QString(QObject::tr("连接就绪")));
                }break;
                case CHARGE_STATUS_STARTING:            //启动中
                {
                    ui->stackedWidgetStatepicture->setCurrentIndex(LABEL_INDEX_CHARGEING1);
                    ui->labelState->setText(QString(QObject::tr("启动中")));
                    timerCount = 1;

                    flashTimer = new QTimer();
                    flashTimer->setInterval(500);
                    QObject::connect(flashTimer,SIGNAL(timeout()), this, SLOT(flashOverTimer()));
                    flashTimer->start();
                }break;
                case CHARGE_STATUS_CHARGING:            //充电中
                {
                    ui->stackedWidgetStatepicture->setCurrentIndex(LABEL_INDEX_CHARGEING1);
                    ui->labelState->setText(QString(QObject::tr("充电中")));

                    timerCount = 1;

                    flashTimer = new QTimer();
                    flashTimer->setInterval(1000);
                    QObject::connect(flashTimer,SIGNAL(timeout()), this, SLOT(flashOverTimer()));
                    flashTimer->start();
                }break;
                case CHARGE_STATUS_FINISH:              //充电完成
                case CHARGE_STATUS_FULL:
                {
                    ui->stackedWidgetStatepicture->setCurrentIndex(LABEL_INDEX_FINSH);
                    ui->labelState->setText(QString(QObject::tr("充电完成")));
                }break;
                case CHARGE_STATUS_FAULT:               //故障
                {
                    ui->stackedWidgetStatepicture->setCurrentIndex(LABEL_INDEX_FAULT);
                    ui->labelState->setText(QString(QObject::tr("故障")));
                    //->setBackgroundRole();
                }break;
                case CHARGE_STATUS_DISCONNECT:      //离线
                {
                    ui->stackedWidgetStatepicture->setCurrentIndex(LABEL_INDEX_OFFLINE);
                    ui->labelState->setText(QString(QObject::tr("离线")));

                    //this->setBackgroundRole();
                }break;
                case CHARGE_STATUS_SLAVEGUN:
                {
                    ui->stackedWidgetStatepicture->setCurrentIndex(LABEL_INDEX_LINK);
                    ui->labelState->setText(QString(QObject::tr("副枪")));
                }break;
                case CHARGE_STATUS_COUPLE_ERR:
                {
                    ui->stackedWidgetStatepicture->setCurrentIndex(LABEL_INDEX_LINK);
                    ui->labelState->setText(QString(QObject::tr("配对错误")));
                }break;
                case CHARGE_STATUS_QUEUE1:
                case CHARGE_STATUS_QUEUE2:
                case CHARGE_STATUS_QUEUE3:
                case CHARGE_STATUS_QUEUE4:
                case CHARGE_STATUS_QUEUE5:
                case CHARGE_STATUS_QUEUE6:
                case CHARGE_STATUS_QUEUE7:
                {
                    ui->stackedWidgetStatepicture->setCurrentIndex(LABEL_INDEX_QUEUE);
                    ui->labelState->setText(QString(QObject::tr("排队"))+ QString::number(chargeData.status-14, 10));
                }break;
                default:
                {
                    ui->stackedWidgetStatepicture->setCurrentIndex(LABEL_INDEX_QUEUE);
                    ui->labelState->setText(QString(QObject::tr("未定义状态")));
                }break;
            }
        }
    }
    /*soc更新显示处理*/
    if(chargeData.canAddr > 180 && chargeData.canAddr < 230 && chargeData.status == CHARGE_STATUS_CHARGING && chargeData.linkStatus!= 2)
    {
        if(true == ui->labelSOC->isHidden())
            ui->labelSOC->show();

        QString socStr = QString::number(chargeData.soc, 10);
        socStr.append("%");
        ui->labelSOC->setText(socStr);
    }
    else
    {
        if(false == ui->labelSOC->isHidden())
            ui->labelSOC->hide();
    }

    chargeStatus = chargeData;
}
