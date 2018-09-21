#include "Fault.h"

IMPLEMENT_RUNTIME(Fault)

Fault::Fault()
{
	m_strDesc = "整机故障";
	alarmCode = 0x05;
}

Fault::~Fault()
{

}

bool Fault::isTriggered(TerminalStatus &status)
{
	if(status.stFrameRemoteSingle.status_fault > 0){
		writeLog(QString().sprintf("[CAN=%3d][%s][报警][%d]",
					status.cCanAddr,
					m_strDesc.toAscii().data(), 
					status.stFrameRemoteSingle.status_fault));
		return true;
	}
	return false;
}

Defense *Fault::createInstance()
{
	Fault *instance = NULL;

	if(!instance){
		instance = new Fault();
	}

	return instance;	
}
