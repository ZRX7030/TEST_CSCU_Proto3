#include "ChargerError.h"

IMPLEMENT_RUNTIME(ChargerError)

ChargerError::ChargerError()
{
	m_strDesc = "充电机终止充电错误原因";
	alarmCode = 0x04;
}

ChargerError::~ChargerError()
{

}

bool ChargerError::isTriggered(TerminalStatus &status)
{
	if(status.stFrameBmsChargeTerm.ChargerErrorReason > 0){
		writeLog(QString().sprintf("[CAN=%3d][%s][报警][%d]",
					status.cCanAddr,
					m_strDesc.toAscii().data(), 
					status.stFrameBmsChargeTerm.ChargerErrorReason));
		return true;
	}
	return false;
}

Defense *ChargerError::createInstance()
{
	ChargerError *instance = NULL;

	if(!instance){
		instance = new ChargerError();
	}

	return instance;	
}
