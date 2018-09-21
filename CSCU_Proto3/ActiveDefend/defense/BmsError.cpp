#include "BmsError.h"

IMPLEMENT_RUNTIME(BmsError)

BmsError::BmsError()
{
	m_strDesc = "BMS终止充电错误原因";
	alarmCode = 0x02;
}

BmsError::~BmsError()
{

}

bool BmsError::isTriggered(TerminalStatus &status)
{
	if(status.stFrameBmsChargeTerm.BMSErrorReason > 0){
		writeLog(QString().sprintf("[CAN=%3d][%s][报警][%d]",
				status.cCanAddr,
				m_strDesc.toAscii().data(), 
				status.stFrameBmsChargeTerm.BMSErrorReason));
		return true;
	}
	return false;
}

Defense *BmsError::createInstance()
{
	BmsError *instance = NULL;

	if(!instance){
		instance = new BmsError();
	}

	return instance;	
}
