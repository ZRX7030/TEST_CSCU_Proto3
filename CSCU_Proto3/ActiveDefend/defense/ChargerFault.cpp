#include "ChargerFault.h"

IMPLEMENT_RUNTIME(ChargerFault)

ChargerFault::ChargerFault()
{
	m_strDesc = "充电机终止充电故障原因";
	alarmCode = 0x03;
}

ChargerFault::~ChargerFault()
{

}

bool ChargerFault::isTriggered(TerminalStatus &status)
{
	if(status.stFrameBmsChargeTerm.ChargerFaultReason > 0){
		writeLog(QString().sprintf("[CAN=%3d][%s][报警][%d]",
					status.cCanAddr,
					m_strDesc.toAscii().data(), 
					status.stFrameBmsChargeTerm.ChargerFaultReason));
		return true;
	}
	return false;
}

Defense *ChargerFault::createInstance()
{
	ChargerFault *instance = NULL;

	if(!instance){
		instance = new ChargerFault();
	}

	return instance;	
}
