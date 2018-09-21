#include "BmsOverCharge.h"
#include <time.h>

IMPLEMENT_RUNTIME(BmsOverCharge)

BmsOverCharge::BmsOverCharge()
{
	m_strDesc = "电池过充";
	alarmCode = 0x0A;
	m_iPreTime = 0;
	m_fPreEnergy = 0.0;
	m_fLeftEnergy = 0.0;
}

BmsOverCharge::~BmsOverCharge()
{

}

bool BmsOverCharge::isTriggered(TerminalStatus &status)
{
	struct timespec ts;

	if(status.stFrameBmsInfo.batery_SOC <= 0 || 
			status.stFrameBmsHand.BatteryRatedCapacity <= 0.0 ||
			status.stFrameBmsHand.BatteryRatedVoltage <= 0.0 ||
			status.stFrameRemoteMeSurement2.active_electric_energy <= 0)
		return false;

	if(m_iPreTime <= 0){
		clock_gettime(CLOCK_MONOTONIC, &ts);
		m_iPreTime = ts.tv_sec;
		return false;
	}

	if(m_fLeftEnergy <= 0.0 || m_fPreEnergy <= 0){
		clock_gettime(CLOCK_MONOTONIC, &ts);
		//充电5分钟后计算电池剩余可充电量
		if(((ts.tv_sec - m_iPreTime) / 60) >= 5){
			m_fLeftEnergy = status.stFrameBmsHand.BatteryRatedCapacity * status.stFrameBmsHand.BatteryRatedVoltage / 1000.0;
			m_fLeftEnergy = m_fLeftEnergy * (float)(100 - status.stFrameBmsInfo.batery_SOC + 15) / 100.0;
			m_fPreEnergy = (double)status.stFrameRemoteMeSurement2.active_electric_energy / 100.0;
			if(m_fLeftEnergy <= 0.0 || m_fPreEnergy <= 0.0){
				return false;
			}

			writeLog(QString().sprintf("[CAN=%3d][%s][初始化]初始电量[%f]初始时间[%d]当前时间[%ld]可充电量[%f] ",
				status.cCanAddr,
				m_strDesc.toAscii().data(), 
				m_fPreEnergy, m_iPreTime, ts.tv_sec, m_fLeftEnergy));
		}

		return false;
	}

	if((double)status.stFrameRemoteMeSurement2.active_electric_energy / 100.0 - m_fPreEnergy > m_fLeftEnergy){
		writeLog(QString().sprintf("[CAN=%3d][%s][报警]已充电量[%f]可充电量[%f]",
				status.cCanAddr,
				m_strDesc.toAscii().data(), 
				(double)status.stFrameRemoteMeSurement2.active_electric_energy / 100.0 - m_fPreEnergy,
				m_fLeftEnergy));
		return true;
	}
	return false;
}

Defense *BmsOverCharge::createInstance()
{
	BmsOverCharge *instance = NULL;

	if(!instance){
		instance = new BmsOverCharge();
	}

	return instance;	
}
