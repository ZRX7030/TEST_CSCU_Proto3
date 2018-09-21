#include "BmsHighTemprature.h"

IMPLEMENT_RUNTIME(BmsHighTemprature)

BmsHighTemprature::BmsHighTemprature()
{
	m_strDesc = "电池过温";
	alarmCode = 0x07;
}

BmsHighTemprature::~BmsHighTemprature()
{

}

bool BmsHighTemprature::isTriggered(TerminalStatus &status)
{
	if(status.stFrameBmsInfo.max_batery_temperature == 0 
			|| (int)status.stFrameBmsParam.MaxtAllowedTemp == 0)
		return false;

	if(status.stFrameBmsInfo.max_batery_temperature > status.stFrameBmsParam.MaxtAllowedTemp){
		writeLog(QString().sprintf("[CAN=%3d][%s][报警]电池温度[%d]最大允许温度[%f]",
				status.cCanAddr,
				m_strDesc.toAscii().data(), 
				status.stFrameBmsInfo.max_batery_temperature,
				status.stFrameBmsParam.MaxtAllowedTemp));
		return true;
	}
	return false;
}

Defense *BmsHighTemprature::createInstance()
{
	BmsHighTemprature *instance = NULL;

	if(!instance){
		instance = new BmsHighTemprature();
	}

	return instance;	
}
