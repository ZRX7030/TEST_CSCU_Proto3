#include "BmsOverCurrent.h"

IMPLEMENT_RUNTIME(BmsOverCurrent)

BmsOverCurrent::BmsOverCurrent()
{
	m_strDesc = "车辆电流过大";
	alarmCode = 0x0D;
}

BmsOverCurrent::~BmsOverCurrent()
{

}

bool BmsOverCurrent::isTriggered(TerminalStatus &status)
{
	if(status.stFrameBmsInfo.ChargeCurrentMeasured <= 0.0 ||
			status.stFrameBmsParam.MaxAllowedCurrent <= 0.0)
		return false;

	if(status.stFrameBmsInfo.ChargeCurrentMeasured > status.stFrameBmsParam.MaxAllowedCurrent){
		writeLog(QString().sprintf("[CAN=%3d][%s][报警]当前电流[%f]最大允许电流[%f]",
				status.cCanAddr,
				m_strDesc.toAscii().data(), 
				status.stFrameBmsInfo.ChargeCurrentMeasured,
				status.stFrameBmsParam.MaxAllowedCurrent));
		return true;
	}
	return false;
}

Defense *BmsOverCurrent::createInstance()
{
	BmsOverCurrent *instance = NULL;

	if(!instance){
		instance = new BmsOverCurrent();
	}

	return instance;	
}
