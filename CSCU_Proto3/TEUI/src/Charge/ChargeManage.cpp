#include <QDebug>

#include "ChargeManage.h"
#include "ChargeTerm.h"
#include "ChargingReport.h"
#include "ChargeReportFinish.h"
#include "ApplayCharge.h"

ChargeManage::ChargeManage(QWidget *parent, CBus *bus, ProtocolBase *protocol) :
    QWidget(parent)
{

    this->bus = bus;
    this->protocol = protocol;
}

ChargeManage::~ChargeManage()
{

}

void ChargeManage::changeCharger(unsigned char canaddr, int status)
{
    this->canAddr = canaddr;
    this->chargeStatus = status;
    qDebug() << "changeCharger  status=" << status;
    switch(chargeStatus)
    {
        case CHARGE_STATUS_CHARGING:		//充电中状态
            {
                ChargingReport *report = new ChargingReport(this,bus,protocol,canAddr);
                int ret = report->exec();
                delete report;
            }break;
        case CHARGE_STATUS_FINISH:		//充电完成状态
        case CHARGE_STATUS_FULL:
            {
                ChargeReportFinish *reportFinish = new ChargeReportFinish(this,bus,protocol,canAddr);
                int ret = reportFinish->exec();
                delete reportFinish;
            }break;
        case CHARGE_STATUS_FAULT:			//故障状态
            {

            }break;
        case CHARGE_STATUS_GUN_STANDBY:     //连接就绪
            {
                ApplayCharge *charge = new ApplayCharge(this);
                charge->show();
                delete charge;
            }break;
        default:
            break;
    }
}
