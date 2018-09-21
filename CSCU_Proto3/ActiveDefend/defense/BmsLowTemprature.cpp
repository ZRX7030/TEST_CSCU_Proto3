#include "BmsLowTemprature.h"

IMPLEMENT_RUNTIME(BmsLowTemprature)

BmsLowTemprature::BmsLowTemprature()
{
	m_strDesc = "电池低温";
	alarmCode = 0x08;
}

BmsLowTemprature::~BmsLowTemprature()
{

}

bool BmsLowTemprature::isTriggered(TerminalStatus &status)
{
	return false;

	if(status.stFrameBmsInfo.lowest_battery_temperature == 0)
		return false;

	if(status.stFrameBmsInfo.lowest_battery_temperature < -5){
		writeLog(QString().sprintf("[CAN=%3d][%s][报警]最小电池温度[%d]小于-5",
				status.cCanAddr,
				m_strDesc.toAscii().data(), 
				status.stFrameBmsInfo.lowest_battery_temperature));
		return true;
	}
	return false;
}

Defense *BmsLowTemprature::createInstance()
{
	BmsLowTemprature *instance = NULL;

	if(!instance){
		instance = new BmsLowTemprature();
	}

	return instance;	
}
