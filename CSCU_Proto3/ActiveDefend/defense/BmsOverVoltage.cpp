#include "BmsOverVoltage.h"

IMPLEMENT_RUNTIME(BmsOverVoltage)

BmsOverVoltage::BmsOverVoltage()
{
	m_strDesc = "电池整体过压";
	alarmCode = 0x0C;
}

BmsOverVoltage::~BmsOverVoltage()
{

}

bool BmsOverVoltage::isTriggered(TerminalStatus &status)
{
	if(status.stFrameBmsInfo.ChargeVoltageMeasured <= 0.0 ||
			status.stFrameBmsHand.MaxAllowedVoltage <= 0.0)
		return false;

	if(status.stFrameBmsInfo.ChargeVoltageMeasured > status.stFrameBmsHand.MaxAllowedVoltage){
		writeLog(QString().sprintf("[CAN=%3d][%s][报警]当前电压[%f]最大允许电压[%f]",
					status.cCanAddr,
					m_strDesc.toAscii().data(),
					status.stFrameBmsInfo.ChargeVoltageMeasured,
					status.stFrameBmsHand.MaxAllowedVoltage));
		return true;
	}
	return false;
}

Defense *BmsOverVoltage::createInstance()
{
	BmsOverVoltage *instance = NULL;

	if(!instance){
		instance = new BmsOverVoltage();
	}

	return instance;	
}
