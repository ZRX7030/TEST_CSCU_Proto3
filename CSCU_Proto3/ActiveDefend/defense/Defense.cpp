#include "Defense.h"
#include "core/ActiveDefend.h"

Defense::Defense()
{
	m_strDesc = "";
	alarmCode = 0;
}

Defense::~Defense()
{

}

QString Defense::getDesc()
{
	return m_strDesc;
}

void Defense::alarm(uchar canAddr, uchar action)
{
	InfoMap map;
	map[Addr_CanID_Comm] = QByteArray(1, canAddr);
	map[Addr_ActiveDefend_AlarmCode] = QByteArray(1, alarmCode);
	map[Addr_ActiveDefend_Action] = QByteArray(1, action);
	ActiveDefend::cmdToBus(map, AddrType_ActiveDefend_Alarm);
}

void Defense::stopCharge(uchar canAddr)
{
	InfoMap map;

	map[Addr_CanID_Comm] = QByteArray(1, canAddr);
	ActiveDefend::cmdToBus(map, AddrType_ActiveDefend_StopCharge);
}

void Defense::trip()
{
	InfoMap map;
	map[Addr_Relay_ID] = QByteArray(1, 0x02);//继电器编号
	map[Addr_Relay_CmdType] = QByteArray(1, 0x01);//继电器命令：闭合0x01 断开0x02
	map[Addr_Relay_HoldType] = QByteArray(1, 0x01);//控制时间1s,0标识持续控制
	ActiveDefend::cmdToBus(map, AddrType_RelayControl_Publish);
}

bool Defense::dfLog(TriggeredInfo *info)
{
	QString strSql, strAction, strResult, strFail;
	int iIndex;

	if(!info)
		return false;

	iIndex = info->level - 1;

	switch(iIndex){
		case 0:
			strAction = "停止充电";
			break;
		case 1:
			strAction = "跳闸";
			break;
	}

	switch(info->dfLevel[iIndex]->result){
		case 0:
			strResult = "失败";
			strFail = "未执行";
			break;
		case 1:
			strResult = "成功";
			strFail = "";
			break;
	}

	strSql.sprintf("INSERT INTO active_defend (can_addr, event_level, trigger_reason, \
		trigger_time, defend_time, defend_action, defend_result, fail_desc) VALUES \
		(%d, %d, '%s', '%s', '%s', '%s', '%s', '%s');", 
			info->canAddr, info->level, info->df->getDesc().toAscii().data(), info->dfLevel[iIndex]->triggerTime,
			info->dfLevel[iIndex]->defendTime, strAction.toAscii().data(), strResult.toAscii().data(), 
			strFail.toAscii().data());

	if(ActiveDefend::getDb()->DBSqlExec(strSql.toAscii().data(), DB_PROCESS_RECORD) == 0){
		return true;
	}

	return false;
}

void Defense::writeLog(QString strLog, int level)
{
	ActiveDefend::writeLog(strLog, level);
}
