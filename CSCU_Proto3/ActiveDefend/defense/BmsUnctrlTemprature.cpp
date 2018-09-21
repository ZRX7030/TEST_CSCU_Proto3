#include "BmsUnctrlTemprature.h"
#include <time.h>

IMPLEMENT_RUNTIME(BmsUnctrlTemprature)

BmsUnctrlTemprature::BmsUnctrlTemprature()
{
	m_strDesc = "电池热失控";
	alarmCode = 0x09;

	m_iPreTime = 0;   
	m_iPreTemprature = 0;
}

BmsUnctrlTemprature::~BmsUnctrlTemprature()
{

}

bool BmsUnctrlTemprature::isTriggered(TerminalStatus &status)
{
	struct timespec ts; 

	if(m_iPreTemprature <= 0){ 
		if(status.stFrameBmsInfo.max_batery_temperature > 0){ 
			m_iPreTemprature = status.stFrameBmsInfo.max_batery_temperature;

			clock_gettime(CLOCK_MONOTONIC, &ts);
			m_iPreTime = ts.tv_sec;
			
			writeLog(QString().sprintf("[CAN=%3d][%s][初始化]初始温度[%d]初始时间[%d]",
						status.cCanAddr,
						m_strDesc.toAscii().data(), 
						m_iPreTemprature,
						m_iPreTime));
		}   

		return false;
	}   

	//温度差大于15度时，时间差小于20分钟报警，否则重新计量时间及温度
	if(status.stFrameBmsInfo.max_batery_temperature - m_iPreTemprature > 15){
		clock_gettime(CLOCK_MONOTONIC, &ts);

		if(((ts.tv_sec - m_iPreTime) / 60) < 20){
			writeLog(QString().sprintf("[CAN=%3d][%s][报警]温度差[%d]时间差[%d]分钟",
					status.cCanAddr,
					m_strDesc.toAscii().data(), 
					status.stFrameBmsInfo.max_batery_temperature - m_iPreTemprature,
					(ts.tv_sec - m_iPreTime) / 60));
			return true;
		}else{
			m_iPreTemprature = status.stFrameBmsInfo.max_batery_temperature;
			m_iPreTime = ts.tv_sec;
		}   
	}   

	return false;
}

Defense *BmsUnctrlTemprature::createInstance()
{
	BmsUnctrlTemprature *instance = NULL;

	if(!instance){
		instance = new BmsUnctrlTemprature();
	}

	return instance;	
}
