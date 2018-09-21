#include "ChargerRelay.h"

IMPLEMENT_RUNTIME(ChargerRelay)

ChargerRelay::ChargerRelay()
{
	m_strDesc = "充电机与继电器状态不匹配";
	alarmCode = 0x0F;
	m_iPreTime = 0;
	m_iAlarmCnt = 0;
}

ChargerRelay::~ChargerRelay()
{

}

bool ChargerRelay::isTriggered(TerminalStatus &status)
{
	struct timespec ts; 

	if(status.stFrameRemoteMeSurement1.active_power <= 0.0)
		return false;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	if(m_iPreTime <= 0){
		m_iPreTime = ts.tv_sec;
		return false;
	}

	//防护启动后30秒进行检测
	if(ts.tv_sec - m_iPreTime < 30)
		return false;

	//报警计数大于2进行告警，规避先控数据跳变
	if(status.stFrameRemoteSingle.relay_status == 1 && 
			(status.stFrameRemoteSingle.charge_status == 1 || 
			 status.stFrameRemoteSingle.charge_status == 8)){
		m_iAlarmCnt = 0;	
		return false;
	}else{
		m_iAlarmCnt++;	
	}

	if(m_iAlarmCnt > 2){
		writeLog(QString().sprintf("[CAN=%3d][%s][报警]充电机状态[%d]继电器状态[%d]",
					status.cCanAddr,
					m_strDesc.toAscii().data(), 
					status.stFrameRemoteSingle.charge_status,
					status.stFrameRemoteSingle.relay_status));
		return true;
	}

	return false;
}

Defense *ChargerRelay::createInstance()
{
	ChargerRelay *instance = NULL;

	if(!instance){
		instance = new ChargerRelay();
	}

	return instance;	
}
