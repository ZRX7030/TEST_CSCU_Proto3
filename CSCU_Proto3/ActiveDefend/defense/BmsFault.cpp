#include "BmsFault.h"

IMPLEMENT_RUNTIME(BmsFault)

BmsFault::BmsFault()
{
	m_strDesc = "BMS终止充电故障原因";
	alarmCode = 0x01;
}

BmsFault::~BmsFault()
{

}

bool BmsFault::isTriggered(TerminalStatus &status)
{
	int fault = 0; 
	
	fault = status.stFrameBmsChargeTerm.BMSFaultReason;
	if(fault > 0){
		writeLog(QString().sprintf("[CAN=%3d][%s][报警][%d]",
				status.cCanAddr,
				m_strDesc.toAscii().data(), 
				fault));
		return true;
	}

	fault = status.stFrameRemoteSingle.BMS_fault;
	if(fault > 0){
		writeLog(QString().sprintf("[CAN=%3d][%s][报警][%d]",
				status.cCanAddr,
				m_strDesc.toAscii().data(), 
				fault));
		return true;
	}

	return false;
}

Defense *BmsFault::createInstance()
{	
	BmsFault *instance = NULL;

	if(!instance){
		instance = new BmsFault();
	}

	return instance;	
}
