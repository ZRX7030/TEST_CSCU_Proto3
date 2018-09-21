#include "monitorprotocol.h"
#include "monitorserver.h"
#include "datacache.h"
#include <QDateTime>
#include <QTimerEvent>
#include <QStringList>

using namespace std;
using namespace Monitor;

const char *protoMonitor[] = {
	"心跳",          	"上线",          	"订单上传失败通知", "运行遥测数据召唤","运行遥测数据通知", 
	"运行遥信数据召唤",	"运行遥信数据通知",	"运行状态数据召唤",	"运行状态数据通知", "运行遥控指令", 	
	"运行告警数据", 	"错误"
};

int monitorId[] = {
	0x00, 0x02, 0x48, 0x51, 0x52, 
	0x53, 0x54, 0x55, 0x56, 0x57, 
	0x58, 0xFF 
};

MonitorProtocol::MonitorProtocol(Net* n) : Protocol(n)
{
	timerAlarm = -1;

	for(int i = 0; i < sizeof(monitorId) / sizeof(int); i++){
		QString str; 
		m_mapProtoName.insert(monitorId[i], protoMonitor[i]);    
		str += protoMonitor[i];
		str += "应答";
		m_mapProtoName.insert(monitorId[i] + 0x80, str);  
	}

	timerAlarm = startTimer(60 * 1000);
}

MonitorProtocol::~MonitorProtocol()
{
	if(timerAlarm > 0){
		killTimer(timerAlarm);	
	}
}

bool MonitorProtocol::parse(short type, char *buff, int len)
{
	infoType = AddrType_Unknown;
	info.clear();

	if(m_mapProtoName.contains(type)){
        writeLog(QString().sprintf("[recv][%x] [%s]",type, m_mapProtoName[type].toAscii().data()),2);
	}

	switch(type){
		case CMD_ACK(CMD_LOGIN):
			parseLogin(buff, len);
			break;
		case CMD_ACK(CMD_HEART):
			parseHeart(buff, len);
			break;
		case CMD_ACK(CMD_BILL_ALARM):
			parseAlarmAns(buff, len);
			break;
		case CMD_MONITOR_MEASURE_CALL:	
			measureCall(buff, len);
			break;
		case CMD_MONITOR_SIGNAL_CALL:
			signalCall(buff, len);
			break;
		case CMD_MONITOR_STATE_CALL:
			stateCall(buff, len);
			break;
		case CMD_MONITOR_CONTROL:
			control();
			break;
		default:
			break;
	}

	emit parsed(info, infoType);

	return true;
}

bool MonitorProtocol::command(InfoMap &map, InfoAddrType &type)
{
	switch(type){
		case AddrType_CSCU_Alarm://集控器内部告警信息
			orderAlarm();
			break;
		default:
			return false;
	}
	return true;
}

bool MonitorProtocol::burst(QDataPointList burst)
{
	QDataPointList measure, state, signal, alarm;

	for(int i = 0; i < burst.count(); i++){
		if(burst.at(i).dataType == "measure")	
			measure.append(burst.at(i));
		else if(burst.at(i).dataType == "state")	
			state.append(burst.at(i));
		else if(burst.at(i).dataType == "signal")	
			signal.append(burst.at(i));
		else if(burst.at(i).dataType == "alarm")	
			alarm.append(burst.at(i));
	}

	alarmNotify(alarm);
	measureNotify(measure);
	stateNotify(state);
	signalNotify(signal);

	return true;
}

void MonitorProtocol::measureCall(char *data, int len)
{
	MonitorTelemetryReq req;
	CommAns ans;
	QStringList keyList;

	req.ParseFromArray(data, len);
    writeLog(QString().sprintf("%s {\n%s}", req.GetDescriptor()->name().c_str(), req.DebugString().c_str()), 2);

	if(req.devindex_size() > 0){
		for(int i = 0; i < req.devindex_size(); i++){
			keyList.append(QString().sprintf("%d", req.devindex(i)));
		}
	}else{
		if(req.devtype().empty())
			return;
		keyList.append(req.devtype().c_str());
	}

	MonitorTelemetryNotifyReq notify;
	for(int i = 0; i < keyList.count(); i++){
		QDataPointList list;
		DataCache::query(keyList.at(i), list, "measure");

		notify.set_reason(CallAnswer);
		DateTime *time = notify.mutable_sendtime();
		time->set_time(QDateTime::currentDateTime().toMSecsSinceEpoch());

		for(int j = 0; j < list.count(); j++){
			MonitorTelemetryType *measure;
			measure = notify.add_datalist();
			measure->set_devindex(list.at(j).canAddr.toInt());
			measure->set_devtype(list.at(j).devType.toAscii().data());
			measure->set_devname(list.at(j).devName.toAscii().data());
			measure->set_measurename(list.at(j).dataName.toAscii().data());
			measure->set_value(list.at(j).value.toDouble());
		}

		sendFrame(CMD_MONITOR_MEASURE_NOTIFY, &notify);
	}

	ans.set_confirm(True);
	sendFrame(CMD_ACK(CMD_MONITOR_MEASURE_CALL), &ans);
}

void MonitorProtocol::stateCall(char *data, int len)
{
	MonitorStateReq req;
	CommAns ans;
	QStringList keyList;

	req.ParseFromArray(data, len);
    writeLog(QString().sprintf("%s {\n%s}", req.GetDescriptor()->name().c_str(), req.DebugString().c_str()), 2);

	if(req.devindex_size() > 0){
		for(int i = 0; i < req.devindex_size(); i++){
			keyList.append(QString().sprintf("%d", req.devindex(i)));
		}
	}else{
		if(req.devtype().empty())
			return;
		keyList.append(req.devtype().c_str());
	}

	for(int i = 0; i < keyList.count(); i++){
		MonitorStateNotifyReq notify;
		QDataPointList list;

		DataCache::query(keyList.at(i), list, "state");

		notify.set_reason(CallAnswer);
		DateTime *time = notify.mutable_sendtime();
		time->set_time(QDateTime::currentDateTime().toMSecsSinceEpoch());

		for(int j = 0; j < list.count(); j++){
			MonitorStateType *state;
			state = notify.add_datalist();
			state->set_devindex(list.at(j).canAddr.toInt());
			state->set_devtype(list.at(j).devType.toAscii().data());
			state->set_devname(list.at(j).devName.toAscii().data());
			state->set_measurename(list.at(j).dataName.toAscii().data());
			state->set_value(list.at(j).value.toInt());
		}

		sendFrame(CMD_MONITOR_STATE_NOTIFY, &notify);
	}

	ans.set_confirm(True);
	sendFrame(CMD_ACK(CMD_MONITOR_STATE_CALL), &ans);
}

void MonitorProtocol::signalCall(char *data, int len)
{
	MonitorSignalReq req;
	CommAns ans;
	QStringList keyList;

	req.ParseFromArray(data, len);
    writeLog(QString().sprintf("%s {\n%s}", req.GetDescriptor()->name().c_str(), req.DebugString().c_str()), 2);

	if(req.devindex_size() > 0){
		for(int i = 0; i < req.devindex_size(); i++){
			keyList.append(QString().sprintf("%d", req.devindex(i)));
		}
	}else{
		if(req.devtype().empty())
			return;
		keyList.append(req.devtype().c_str());
	}

	for(int i = 0; i < keyList.count(); i++){
		MonitorSignalNotifyReq notify;
		QDataPointList list;

		DataCache::query(keyList.at(i), list, "signal");

		notify.set_reason(CallAnswer);
		DateTime *time = notify.mutable_sendtime();
		time->set_time(QDateTime::currentDateTime().toMSecsSinceEpoch());

		for(int j = 0; j < list.count(); j++){
			MonitorSignalType *signal;
			signal = notify.add_datalist();
			signal->set_devindex(list.at(j).canAddr.toInt());
			signal->set_devtype(list.at(j).devType.toAscii().data());
			signal->set_devname(list.at(j).devName.toAscii().data());
			signal->set_measurename(list.at(j).dataName.toAscii().data());
			signal->set_value((SignalType)(list.at(j).value.toInt() + 1));
		}

		sendFrame(CMD_MONITOR_SIGNAL_NOTIFY, &notify);
	}

	ans.set_confirm(True);
	sendFrame(CMD_ACK(CMD_MONITOR_SIGNAL_CALL), &ans);
}

void MonitorProtocol::measureNotify(QDataPointList list)
{
	MonitorTelemetryNotifyReq notify;
	for(int i = 0; i < list.count(); i++){
		notify.set_reason(DataChange);
		DateTime *time = notify.mutable_sendtime();
		time->set_time(QDateTime::currentDateTime().toMSecsSinceEpoch());

		for(int j = 0; j < list.count(); j++){
			MonitorTelemetryType *measure;
			measure = notify.add_datalist();
			measure->set_devindex(list.at(j).canAddr.toInt());
			measure->set_devtype(list.at(j).devType.toAscii().data());
			measure->set_devname(list.at(j).devName.toAscii().data());
			measure->set_measurename(list.at(j).dataName.toAscii().data());
			measure->set_value(list.at(j).value.toDouble());
		}

		sendFrame(CMD_MONITOR_STATE_NOTIFY, &notify);
	}
}

void MonitorProtocol::stateNotify(QDataPointList list)
{
	MonitorStateNotifyReq notify;
	for(int i = 0; i < list.count(); i++){
		notify.set_reason(DataChange);
		DateTime *time = notify.mutable_sendtime();
		time->set_time(QDateTime::currentDateTime().toMSecsSinceEpoch());

		for(int j = 0; j < list.count(); j++){
			MonitorStateType *state;
			state = notify.add_datalist();
			state->set_devindex(list.at(j).canAddr.toInt());
			state->set_devtype(list.at(j).devType.toAscii().data());
			state->set_devname(list.at(j).devName.toAscii().data());
			state->set_measurename(list.at(j).dataName.toAscii().data());
			state->set_value(list.at(j).value.toInt());
		}

		sendFrame(CMD_MONITOR_STATE_NOTIFY, &notify);
	}
}

void MonitorProtocol::signalNotify(QDataPointList list)
{
	MonitorSignalNotifyReq notify;
	for(int i = 0; i < list.count(); i++){
		notify.set_reason(DataChange);
		DateTime *time = notify.mutable_sendtime();
		time->set_time(QDateTime::currentDateTime().toMSecsSinceEpoch());

		for(int j = 0; j < list.count(); j++){
			MonitorSignalType *signal;
			signal = notify.add_datalist();
			signal->set_devindex(list.at(j).canAddr.toInt());
			signal->set_devtype(list.at(j).devType.toAscii().data());
			signal->set_devname(list.at(j).devName.toAscii().data());
			signal->set_measurename(list.at(j).dataName.toAscii().data());
			signal->set_value((SignalType)(list.at(j).value.toInt() + 1));
		}

		sendFrame(CMD_MONITOR_SIGNAL_NOTIFY, &notify);
	}
}

void MonitorProtocol::alarmNotify(QDataPointList list)
{
	AlarmReq req;
	uchar canAddr;

	if(list.count() <= 0)
		return;

	for(int i = 0; i < list.count(); i++){
		req.set_devindex(list.at(i).canAddr.toInt());
		req.set_sn(list.at(i).devName.toAscii().data());
		req.set_devtype((DevDescType)list.at(i).devType.toInt());
		req.set_reason(DataChange);
		DateTime *time = req.mutable_alarmtime();
		time->set_time(QDateTime::currentDateTime().toMSecsSinceEpoch());
		AlarmDataType *alarm;
		alarm = req.add_alarmdatalist();
		alarm->set_alarmcode(list.at(i).value.toInt());
		alarm->set_alarmstate((AlarmStateEnumType)list.at(i).valueDesc.toInt());
	}

	sendFrame(CMD_MONITOR_ALARM, &req);
}

void MonitorProtocol::orderAlarm(bool burst)
{
    struct db_result_st res, result;
	QString strSql;
	int can;

	strSql.sprintf("SELECT can_addr FROM cscu_order_alarm WHERE alarm_valid = 1 GROUP BY can_addr;");
	if(ProtobufServer::server()->database()->DBSqlQuery(strSql.toAscii().data(),&res, DB_PROCESS_RECORD) == 0){
		for(int i = 0; i < res.row; i++){
			BillUploadFailReq req;
			can = atoi(res.result[res.column * i]);
			req.set_devindex(can);
			if(burst)
				req.set_reason(DataChange);
			else
				req.set_reason(IntervalSend);
			DateTime *time = req.mutable_sendtime();
			time->set_time(QDateTime::currentDateTime().toMSecsSinceEpoch());

			strSql.sprintf("SELECT alarm_code, order_code, order_uuid, alarm_content FROM cscu_order_alarm WHERE alarm_valid = 1 AND can_addr = %d;", can);
			if(ProtobufServer::server()->database()->DBSqlQuery(strSql.toAscii().data(),&result,DB_PROCESS_RECORD) == 0){
				for(int j = 0; j < result.row; j++){
					BillUpLoadFailDataType *alarm;
					alarm = req.add_failbilllist();
					alarm->set_reason(atoi(result.result[result.column * j]));
					alarm->set_unconfirmbillcode(result.result[result.column * j + 1]);
					alarm->set_unconfirmctrlbillcode(result.result[result.column * j + 2]);
					alarm->set_billdetail(result.result[result.column * j + 3]);
				}
				ProtobufServer::server()->database()->DBQueryFree(&result);
			}

			sendFrame(CMD_BILL_ALARM, &req);
		}
		ProtobufServer::server()->database()->DBQueryFree(&res);
	}
}

void MonitorProtocol::control()
{

}

bool MonitorProtocol::sendFrame(char frameType, PBMessage *message)
{
	if(!Protocol::sendFrame(frameType, message))
		return false;

	if(m_mapProtoName.contains(frameType)){
        writeLog(QString().sprintf("[send][%x] [%s]", frameType,m_mapProtoName[frameType].toAscii().data()),2);
	}

    writeLog(QString().sprintf("%s {\n%s}", message->GetDescriptor()->name().c_str(), message->DebugString().c_str()), 2);
	return true;
}

bool MonitorProtocol::login()
{
    ProtobufServer::server()->getNetSetting(serverAddr, serverPort, encryptNet);
    if(!connected && net){
		net->connect(serverAddr.c_str(), serverPort, encryptNet);
		return false;
	}

	LoginReq req;

	req.set_key(cscuKey.c_str());
	req.set_ctrlprotoversion("3");
	DateTime *time = req.mutable_sendtime();
	time->set_time(QDateTime::currentDateTime().toMSecsSinceEpoch());
	
	sendFrame(CMD_LOGIN, &req);

	return true;
}

void MonitorProtocol::logout(int type)
{
	OfflineReq req;

	req.set_reason((OfflineReason)type);
	DateTime *time = req.mutable_offlinetime();
	time->set_time(QDateTime::currentDateTime().toMSecsSinceEpoch());

	sendFrame(CMD_LOGOUT, &req);

	Protocol::logout(type);

	emit active(false);
}

void MonitorProtocol::heart()
{
	HeartbeatReq req;
	DateTime *time;

 	time = req.mutable_currenttime();
	QDateTime dt = QDateTime::currentDateTime();
	time->set_time(dt.toMSecsSinceEpoch());

	sendFrame(CMD_HEART, &req);
}

void MonitorProtocol::parseLogin(char *data, int len)
{
	LoginAns ans;
	DateTime time;

	ans.ParseFromArray(data, len);

    writeLog(QString().sprintf("%s {\n%s}", ans.GetDescriptor()->name().c_str(), ans.DebugString().c_str()), 2);
	
	logged = (ans.status() == Accepted) ? true : false;
	time = ans.currenttime();

	emit active(logged);
}

void MonitorProtocol::parseHeart(char *data, int len)
{
	HeartbeatAns ans;
	DateTime time;
	QDateTime dt;
	qint64 currentTime;

	dt = QDateTime::currentDateTime();
	currentTime = dt.toMSecsSinceEpoch();

	ans.ParseFromArray(data, len);
    writeLog(QString().sprintf("%s {\n%s}", ans.GetDescriptor()->name().c_str(), ans.DebugString().c_str()), 2);

	time = ans.currenttime();
	
	emit live();
}

void MonitorProtocol::parseAlarmAns(char *data, int len)
{
	BillUploadFailAns ans;
	QString strSql;

	ans.ParseFromArray(data, len);
	strSql.sprintf("UPDATE cscu_order_alarm SET alarm_valid = 0 WHERE can_addr = %d;", ans.devindex());

	ProtobufServer::server()->database()->DBSqlExec(strSql.toAscii().data(), DB_PROCESS_RECORD);
}

void MonitorProtocol::timerEvent(QTimerEvent *event)
{
	if(event->timerId() == timerAlarm && logged){
		orderAlarm(false);
	}
}
