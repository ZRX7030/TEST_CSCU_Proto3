#include "BmsRepeatInfo.h"

IMPLEMENT_RUNTIME(BmsRepeatInfo)

BmsRepeatInfo::BmsRepeatInfo()
{
	m_strDesc = "BMS数据重复发送";
	alarmCode = 0x0E;

	m_iChangeCnt = 0;
	m_iCheckCnt = 0; 
	m_iPreTime = 0;
	memset(&m_stPreInfo, 0, sizeof(repeat_info));
}

BmsRepeatInfo::~BmsRepeatInfo()
{

}

bool BmsRepeatInfo::isTriggered(TerminalStatus &status)
{
	struct timespec ts;
	repeat_info info;

	if(status.stFrameBmsInfo.ChargeVoltageMeasured <= 0.0 ||
			status.stFrameBmsInfo.ChargeCurrentMeasured <= 0.0 ||
			status.stFrameBmsInfo.max_batery_voltage <= 0.0 ||
			status.stFrameBmsInfo.max_batery_temperature <= 0 ||
			status.stFrameBmsInfo.batery_SOC <= 0)
		return false;

	memset(&info, 0, sizeof(repeat_info));

	info.voltage = status.stFrameBmsInfo.ChargeVoltageMeasured;
	info.current = status.stFrameBmsInfo.ChargeCurrentMeasured;
	info.single_voltage = status.stFrameBmsInfo.max_batery_voltage;
	info.temprature = status.stFrameBmsInfo.max_batery_temperature;
	info.soc = status.stFrameBmsInfo.batery_SOC;
	info.left_time = status.stFrameBmsInfo.LeftTime;

	if(m_iPreTime <= 0){
		memcpy(&m_stPreInfo, &info, sizeof(repeat_info));

		clock_gettime(CLOCK_MONOTONIC, &ts);
		m_iPreTime = ts.tv_sec;
		writeLog(QString().sprintf("[CAN=%3d][%s][初始化]初始时间%d",
				status.cCanAddr,
				m_strDesc.toAscii().data(), 
				m_iPreTime));
	}

	//20个周期(一分钟)检查一次
	if(m_iCheckCnt++ > 20){
		clock_gettime(CLOCK_MONOTONIC, &ts);

		if(memcmp(&info, &m_stPreInfo, sizeof(repeat_info)) != 0){
			memcpy(&m_stPreInfo, &info, sizeof(repeat_info));
			m_iChangeCnt++;
		}

		if(((ts.tv_sec - m_iPreTime) / 60) >= 15){
			writeLog(QString().sprintf("[CAN=%3d][%s][检测]初始时间[%d]当前时间[%ld]数据改变次数[%d]",
						status.cCanAddr, 
						m_strDesc.toAscii().data(), 
						m_iPreTime,ts.tv_sec, 
						m_iChangeCnt));

			m_iPreTime = ts.tv_sec;
			if(m_iChangeCnt <= 0){
				writeLog(QString().sprintf("[CAN=%3d][%s][报警]数据改变次数[%d]",
							status.cCanAddr, 
							m_strDesc.toAscii().data(), 
							m_iChangeCnt));
				return true;
			}else{
				m_iChangeCnt = 0;
			}
		}
		m_iCheckCnt = 0;
	}

	return false;
}

Defense *BmsRepeatInfo::createInstance()
{
	BmsRepeatInfo *instance = NULL;

	if(!instance){
		instance = new BmsRepeatInfo();
	}

	return instance;	
}
