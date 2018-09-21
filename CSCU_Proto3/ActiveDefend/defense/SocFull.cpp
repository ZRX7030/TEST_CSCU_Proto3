#include "SocFull.h"

IMPLEMENT_RUNTIME(SocFull)

SocFull::SocFull()
{
	m_strDesc = "SOC满";
	alarmCode = 0x06;
	m_iPreTime = 0;
}

SocFull::~SocFull()
{

}

bool SocFull::isTriggered(TerminalStatus &status)
{
	struct timespec ts;

	if(status.stFrameBmsInfo.batery_SOC < 100){
		return false;
	}

	clock_gettime(CLOCK_MONOTONIC, &ts);

	if(m_iPreTime <= 0){
		m_iPreTime = ts.tv_sec;
		return false;
	}

	if((ts.tv_sec - m_iPreTime) / 60 >= 30){
		writeLog(QString().sprintf("[CAN=%3d][%s][报警][%d]",
					status.cCanAddr,
					m_strDesc.toAscii().data(), 
					status.stFrameBmsInfo.batery_SOC));
		return true;
	}
	return false;
}

Defense *SocFull::createInstance()
{
	SocFull *instance = NULL;

	if(!instance){
		instance = new SocFull();
	}

	return instance;	
}
