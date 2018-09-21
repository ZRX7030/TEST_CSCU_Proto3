#include "BmsSingleOverVoltage.h"

IMPLEMENT_RUNTIME(BmsSingleOverVoltage)

BmsSingleOverVoltage::BmsSingleOverVoltage()
{
	m_strDesc = "电池单体过压";
	alarmCode = 0x0B;
}

BmsSingleOverVoltage::~BmsSingleOverVoltage()
{

}

bool BmsSingleOverVoltage::isTriggered(TerminalStatus &status)
{
	if(status.stFrameBmsInfo.max_batery_voltage <= 0.0 ||
			status.stFrameBmsParam.SingleBatteryMaxAllowedVoltage <= 0.0)
		return false;

	if(status.stFrameBmsInfo.max_batery_voltage > status.stFrameBmsParam.SingleBatteryMaxAllowedVoltage){
		writeLog(QString().sprintf("[CAN=%3d][%s][报警]单体电压[%f]最大允许单体电压[%f]",
					status.cCanAddr,
					m_strDesc.toAscii().data(), 
					status.stFrameBmsInfo.max_batery_voltage,
					status.stFrameBmsParam.SingleBatteryMaxAllowedVoltage));
		return true;
	}
	return false;
}

Defense *BmsSingleOverVoltage::createInstance()
{
	BmsSingleOverVoltage *instance = NULL;

	if(!instance){
		instance = new BmsSingleOverVoltage();
	}

	return instance;	
}
