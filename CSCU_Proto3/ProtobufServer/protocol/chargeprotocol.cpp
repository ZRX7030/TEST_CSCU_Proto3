#include "chargeprotocol.h"
#include "chargeserver.h"
#include "datacache.h"
#include <QByteArray>
#include <arpa/inet.h>
#include <endian.h>
#include <string>
#include <QTimer>
#include <QDateTime>
#include <QCoreApplication>
#include "CommonFunc/commfunc.h"
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
//#include <qcoreapplication.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <netinet/in.h>
#include <qtextcodec.h>

using namespace Charge;

const char *protoCharge[] = {
    "心跳",          	"上线",          	"离线",            	"申请开始充电", 	"开始充电",
    "申请结束充电",  	"结束充电",      	"暂停充电",        	"订单上传",    		"恢复充电",
    "充电机状态通知",	"充电机状态召唤",	"账户详情获取",    	"充电功率曲线",		"同步账户信息", 
    "BMS参数通知",	 	"遥测数据召唤",  	"遥测数据通知",    	"遥信数据召唤",    	"遥信数据通知",
	"BMS数据召唤",   	"BMS数据通知",   	"未确认订单召唤",  	"上报多枪分组",		"指定编号订单召唤",	
    "下发多枪分组信息",	"平台要求集控主动转为本地应急充电指令",        "错误"
};

int chargeId[] = {
    0x00, 0x02, 0x04, 0x10, 0x11, 
    0x12, 0x13, 0x15, 0x16, 0x17, 
    0x20, 0x21, 0x22, 0x23, 0x24, 
    0x26, 0x31, 0x32, 0x33, 0x34, 
	0x35, 0x36, 0x41, 0x42, 0x43, 
    0x45, 0x47, 0xFF
};

ChargeProtocol::ChargeProtocol(Net *n) : Protocol(n)
{
	for(uint i = 0; i < sizeof(chargeId) / sizeof(int); i++){
		QString str; 
		m_mapProtoName.insert(chargeId[i], protoCharge[i]);    
		str += protoCharge[i];
		str += "应答";
		m_mapProtoName.insert(chargeId[i] + 0x80, str);  
    }

	memset(&billThread, 0, sizeof(pthread_t));

    bSendBillStart = false;
}


ChargeProtocol::~ChargeProtocol()
{
	if(billThread)
		pthread_join(billThread, NULL);
}

bool ChargeProtocol::command(InfoMap &map, InfoAddrType &type)
{
	switch(type){
		//case AddrType_CmdCtrl_ExecResult://充电执行结果
		case AddrType_CmdCtrl_AckResult://充电响应结果
			chargeAns(map);
			break;
		case AddrType_TermSignal://突发遥信
			burstSignal(map);
			break;
		case AddrType_LogicState://突发充电机逻辑工作状态
			burstLogicState(map);
			break;
		case AddrType_ChargeServicApplyAccountInfo://刷卡申请帐户详情
			applyAccount(map);
			break;
		case AddrType_OutApplyStartChargeByChargeServic://刷卡申请开始充电
		case AddrType_VinApplyStartCharge://VIN申请开始充电
		case AddrType_CarLicenceApplyStartCharge://车牌号申请开始充电
			applyStartCharge(map);
			break;
		case AddrType_OutApplyStopChargeByChargeServic://刷卡申请结束充电
		case AddrType_VinApplyStopCharge://VIN申请结束充电
		case AddrType_CarLicenceApplyStopCharge://车牌号申请结束充电
			applyStopCharge(map);
			break;
		case AddrType_ChargeOrder_State://订单状态
			uploadBill(map);
			break;
		case AddrType_Sync_Account://同步账户信息
			syncAccount(1);
			break;
		case AddrType_CheckChargeManner_Success://多枪信息突发
			gunGroupNotify(map);
			break;
        case AddrType_BillSendReult:  //重发超时结果
            sendUnConfirmedBillSendReult(map);
            break;
		default:
			return false;
	}

	return true;
}

bool ChargeProtocol::login()
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

    //add by zrx 添加集控上线状态  当CtrlStatus=2时需要填写集控缓存的历史订单数
    struct db_result_st result;
    QString strSql;
    int iCtrlStatus,iBillCount;

    strSql.sprintf("SELECT * FROM  charge_order WHERE OrderSync < 2 AND OrderStatus <> 0 AND \
                   ((OrderStatus <> %d AND OrderType = %d) OR (OrderType = %d));",\
                   ORDER_STATUS_ING,ORDER_NORMAL, ORDER_EMERGENCY);
    if(ProtobufServer::server()->database()->DBSqlQuery(strSql.toAscii().data(),&result,DB_PROCESS_RECORD) == 0){
        if(result.row > 0){  //有历史订单
            iCtrlStatus = 2;
            iBillCount = result.row;
        }
        else{ //无历史订单
            iCtrlStatus = 1;
            iBillCount = 0;
        }
        req.set_ctrlstatus(iCtrlStatus);
        req.set_billcount(iBillCount);
    }
    req.set_ctrlswversion(ctrlSwVersion.c_str());
	
	sendFrame(CMD_LOGIN, &req);
    ProtobufServer::server()->database()->DBQueryFree(&result);
	return true;
}

void ChargeProtocol::logout(int type)
{
	OfflineReq req;
	req.set_reason((OfflineReason)type);
	DateTime *time = req.mutable_offlinetime();
	time->set_time(QDateTime::currentDateTime().toMSecsSinceEpoch());

	sendFrame(CMD_LOGOUT, &req);

	if(net){
        net->close();
	}

	connected = false;
	logged = false;
    emit active(logged);
}

void ChargeProtocol::heart()
{
	HeartbeatReq req;
	DateTime *time;

 	time = req.mutable_currenttime();
	QDateTime dt = QDateTime::currentDateTime();
	time->set_time(dt.toMSecsSinceEpoch());

	sendFrame(CMD_HEART, &req);
}

void ChargeProtocol::chargeAns(InfoMap &map)
{
	uchar can, cmd;
	BoolEnum result;
	QByteArray code;

	if(!map.contains(Addr_CanID_Comm))
		return;
	if(!map.contains(Addr_ChargeCmd_Ctrl))
		return;
	if(!map.contains(Addr_Bill_Code))
		return;
	//if(!map.contains(Addr_ExecResult_Ctrl))
	if(!map.contains(Addr_AckResult_Ctrl))
		return;

	can = map[Addr_CanID_Comm].at(0);
	cmd = map[Addr_ChargeCmd_Ctrl].at(0);
	code = map[Addr_Bill_Code];
	//result = map[Addr_ExecResult_Ctrl].at(0) == CMD_ACK_EXE_TYPE_SUCCESS ? True : False;
	result = map[Addr_AckResult_Ctrl].at(0) == CMD_ACK_TYPE_SUCCESS ? True : False;

	switch(cmd){
		case CHARGE_CMD_TYPE_START_CHARGE_NOW:
		case CHARGE_CMD_TYPE_START_CHARGE_ECONOMIC:
			sendStartChargeAns(can, code, result);
			break;
		case CHARGE_CMD_TYPE_STOP_CHARGE:
		case CHARGE_CMD_TYPE_STOP_CHARGE_URGENCY:
			sendStopChargeAns(can, code, result);
			break;
		case CHARGE_CMD_TYPE_PAUSH_CHARGE:
			sendPauseChargeAns(can, code, result);
			break;
		case CHARGE_CMD_TYPE_RESUME:
			sendResumeChargeAns(can, code, result);
			break;
		default:
			return;
	}
}

void ChargeProtocol::burstSignal(InfoMap &map)
{
	StateNotifyReq notify;
	uchar canAddr;
    CHARGE_STEP stChargeStep;//存放从缓存中读出的状态机

	if(!map.contains(Addr_CanID_Comm))
		return;

	canAddr = map[Addr_CanID_Comm].at(0);

	notify.set_canindex(canAddr);
	notify.set_reason(DataChange);

	if(map.contains(Addr_ChargeGunNum_Sudden)){
		notify.add_type(Interface);
		notify.add_value(map[Addr_ChargeGunNum_Sudden].at(0));
	}
	if(map.contains(Addr_LinkState_Sudden)){
		notify.add_type(Connect);
		notify.add_value(map[Addr_LinkState_Sudden].at(0));
	}
	if(map.contains(Addr_RelyState_Sudden)){
		notify.add_type(Relay);
		notify.add_value(map[Addr_RelyState_Sudden].at(0));
	}
	if(map.contains(Addr_ParkingSpaceFreeFlag_Sudden)){
		notify.add_type(Parking);
		notify.add_value(map[Addr_ParkingSpaceFreeFlag_Sudden].at(0));
	}
	if(map.contains(Addr_WorkState_Sudden)){
		notify.add_type(Charger);
		notify.add_value(map[Addr_WorkState_Sudden].at(0));
	}
	if(map.contains(Addr_FaultCode_Sudden)){
		notify.add_type(StateTypeFault);
		notify.add_value(map[Addr_FaultCode_Sudden].at(0));
	}
	if(map.contains(Addr_BMSFaultCode_Sudden)){
		notify.add_type(BmsFault);
		notify.add_value(map[Addr_BMSFaultCode_Sudden].at(0));
	}
	if(map.contains(Addr_ChargeEndCode_Sudden)){
		notify.add_type(Reason);
		notify.add_value(map[Addr_ChargeEndCode_Sudden].at(0));
	}
	if(map.contains(Addr_GroupQueueFlag_Sudden)){
		notify.add_type(Stategy);
		notify.add_value(map[Addr_GroupQueueFlag_Sudden].at(0));
	}
	if(map.contains(Addr_AuxPowerType_Sudden)){
		notify.add_type(AuxType);
		notify.add_value(map[Addr_AuxPowerType_Sudden].at(0));
	}
	if(map.contains(Addr_CtrlModeFlag_Sudden)){
		notify.add_type(ControlMode);
		notify.add_value(map[Addr_CtrlModeFlag_Sudden].at(0));
	}

    if(ProtobufServer::server()->cache()->QueryChargeStep(canAddr, stChargeStep)){
        //新增订单号 add by zrx
        notify.set_billcode(string().append(QByteArray((char *)stChargeStep.sBillCode, strlen(stChargeStep.sBillCode)).data()));
        if(strcmp(stChargeStep.sOrderUUID,"") != 0)
            notify.set_ctrlbillcode(string().append(QByteArray((char *)stChargeStep.sOrderUUID, LENGTH_GUID_NO).toHex().data()));
    }

	DateTime *time = notify.mutable_sendtime();
	time->set_time(QDateTime::currentDateTime().toMSecsSinceEpoch());

    sendFrame(CMD_SIGNAL_NOTIFY, &notify);
}

void ChargeProtocol::burstLogicState(InfoMap &map)
{
	ChargerStateChangeReq req;
	ChargerState *state;
	uchar canAddr, status;
    CHARGE_STEP step;

	if(!map.contains(Addr_CanID_Comm))
		return;
	if(!map.contains(Addr_LogicState_Burst))
		return;

	canAddr = map[Addr_CanID_Comm].at(0);
	status = map[Addr_LogicState_Burst].at(0);

	state = req.add_state();
	state->set_canindex(canAddr);
	createChargerState(state, status);
    req.set_reason(DataChange);

    if(ProtobufServer::server()->cache()->QueryChargeStep(canAddr, step)){
        state->set_billcode(string().append(QByteArray((char *)step.sBillCode, strlen(step.sBillCode)).data()));
        //if(step.sOrderUUID[0] != 0)
        if(strcmp(step.sOrderUUID, "") != 0)
            state->set_ctrlbillcode(string().append(QByteArray((char *)step.sOrderUUID, LENGTH_GUID_NO).toHex().data()));
    }

    DateTime *time = req.mutable_sendtime();
	time->set_time(QDateTime::currentDateTime().toMSecsSinceEpoch());

	sendFrame(CMD_CHARGER_NOTIFY, &req);
}

void ChargeProtocol::applyAccount(InfoMap &map)
{
	AccountDetailReq req;	
	AccountType type = Card;
	string card;
	uchar can;

	if(!map.contains(Addr_CardAccount))
		return;
	if(!map.contains(Addr_CanID_Comm))
		return;

	can = map[Addr_CanID_Comm].at(0);

	card.append(map[Addr_CardAccount].toHex().data(), map[Addr_CardAccount].toHex().length());
	req.set_canindex(can);
	req.set_type(type);
	req.set_accountid(card);
	DateTime *time = req.mutable_sendtime();
	time->set_time(QDateTime::currentDateTime().toMSecsSinceEpoch());
	
	sendFrame(CMD_APPLY_ACCOUNT, &req);
}

void ChargeProtocol::applyStartCharge(InfoMap &map)
{
	StartChargeReq req;
	ChargeRequestType type;
	CardInfo *card;
	string userId;
	uchar canAddr, apply;

	if(!map.contains(Addr_CanID_Comm))
		return;

	do{
		if(map.contains(Addr_BatteryVIN_BMS)){
			if(!map.contains(Addr_VINApplyStartChargeType))
				return;
			apply = map[Addr_VINApplyStartChargeType].at(0);
			if(apply != 1)
				return;

			userId.append(map[Addr_BatteryVIN_BMS].data(), map[Addr_BatteryVIN_BMS].length());
			req.set_vin(userId);
			type = VinCharge;
			break;
		}

		if(map.contains(Addr_CarLicense)){
			if(!map.contains(Addr_CarLicenseApplyStartChargeType))
				return;
			apply = map[Addr_CarLicenseApplyStartChargeType].at(0);
			if(apply != 1)
				return;

			userId.append(map[Addr_CarLicense].data(), map[Addr_CarLicense].length());
			req.set_plate(userId);
			type = PlateCharge;
			break;
		}
		if(map.contains(Addr_CardAccount)){
			userId.append(map[Addr_CardAccount].toHex().data(), map[Addr_CardAccount].toHex().length());
			card = req.mutable_card();
			card->set_cardcode(userId);
			type = CardCharge;
			break;
		}
		return;
	}while(false);

	canAddr = map[Addr_CanID_Comm].at(0);	
	req.set_canindex(canAddr);
	req.set_type(type);
	DateTime *time = req.mutable_sendtime();
	time->set_time(QDateTime::currentDateTime().toMSecsSinceEpoch());

	sendFrame(CMD_APPLY_START_CHARGE, &req);
}

void ChargeProtocol::applyStopCharge(InfoMap &map)
{
	StopChargeReq req;
	ChargeRequestType type;
	CardInfo *card;
	string code, userId;
	uchar canAddr;

	if(!map.contains(Addr_CanID_Comm))
		return;
	if(!map.contains(Addr_Bill_Code))
		return;

	do{
		if(map.contains(Addr_BatteryVIN_BMS)){
			userId.append(map[Addr_BatteryVIN_BMS].data(), map[Addr_BatteryVIN_BMS].length());
			req.set_vin(userId);
			type = VinCharge;
			break;
		}

		if(map.contains(Addr_CarLicense)){
			userId.append(map[Addr_CarLicense].data(), map[Addr_CarLicense].length());
			req.set_plate(userId);
			type = PlateCharge;
			break;
		}

		if(map.contains(Addr_CardAccount)){
			userId.append(map[Addr_CardAccount].toHex().data(), map[Addr_CardAccount].toHex().length());
			card = req.mutable_card();
			card->set_cardcode(userId);
			type = CardCharge;
			break;
		}
		return;
	}while(false);

	canAddr = map[Addr_CanID_Comm].at(0);
	code.append(map[Addr_Bill_Code].data(), map[Addr_Bill_Code].length());

	req.set_canindex(canAddr);
	req.set_stoptype(type);
	req.set_billcode(code);

	DateTime *time = req.mutable_sendtime();
	time->set_time(QDateTime::currentDateTime().toMSecsSinceEpoch());

	sendFrame(CMD_APPLY_STOP_CHARGE, &req);
}

void ChargeProtocol::uploadBill(InfoMap &map)
{
	uchar canAddr;
	QString strBillCode, strGuid;
	int status;

	canAddr = map[Addr_CanID_Comm].at(0);
	status = map[Addr_ChargeOrder_State].at(0);
	strBillCode = map[Addr_Bill_Code].data();
    strGuid = map[Addr_Order_GUID].data();

    billNotify(canAddr, strGuid);
}

void ChargeProtocol::parseLoginAns(char *data, int len)
{
	LoginAns ans;
	DateTime time;

	ans.ParseFromArray(data, len);

    writeLog(QString().sprintf("%s {\n%s}", ans.GetDescriptor()->name().c_str(), ans.DebugString().c_str()), 2);
	
	logged = (ans.status() == Accepted) ? true : false;
	time = ans.currenttime();

	if(logged && time.time() > 0){
		setSystemTime(time.time());
	}

	emit active(logged);

//    sendUnConfirmedBillOne(0);   //add by zrx 模拟测试未召唤订单上传超时
}

void ChargeProtocol::parseHeartAns(char *data, int len)
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

void ChargeProtocol::sendHeartAns(char *data, int len)
{
	HeartbeatReq req;
	HeartbeatAns ans;

	req.ParseFromArray(data, len);

	QDateTime dt = QDateTime::currentDateTime();

 	DateTime *time = ans.mutable_currenttime();
	time->set_time(dt.toMSecsSinceEpoch());

	sendFrame(CMD_ACK(CMD_HEART), &ans);
}

void ChargeProtocol::parseAccountAns(char *data, int len)
{
	AccountDetailAns ans;
	uint iBalance;

	ans.ParseFromArray(data, len);
    writeLog(QString().sprintf("%s {\n%s}", ans.GetDescriptor()->name().c_str(), ans.DebugString().c_str()), 2);

	iBalance = ans.money() * 100;

	QByteArray arCard;
	arCard.append(ans.accountid().c_str(), ans.accountid().length());

	info.insert(Addr_CardAccount, QByteArray::fromHex(arCard));
	info.insert(Addr_Account_Balance, QByteArray((char*)&iBalance, sizeof(uint)));
//    qAmmeterByteArray.append((char*)&dayFreezeEnergyMaxDemandData.day_freeze_active_absorb_energy,4);

	infoType = AddrType_ChargeServicApplyAccountInfo_Result;
}

void ChargeProtocol::parsePowerCurve(char *data, int len)
{
	PowerCruveCmdReq req;
	PowerCruveCmdAns ans;
	FailReason reason = DefaultFailReason;
	uint curveState = 0;

	req.ParseFromArray(data, len);
    writeLog(QString().sprintf("%s {\n%s}", req.GetDescriptor()->name().c_str(), req.DebugString().c_str()), 2);

	curveState = savePowerCurve(req.strategy(), req.canindex(), req.ctype(), req.code());
	if(curveState > 0){
		info[Addr_CanID_Comm] = QByteArray(1, (uchar)req.canindex());
		info[Addr_Bill_Code] = QByteArray().append(req.code().c_str(), req.code().length());
		info[Addr_Power_Curve_State] = QByteArray((char*)&curveState, sizeof(uint));
		infoType = AddrType_Power_Curve;
	}else{
		reason = GetStategyError; 
	}

	ans.set_confirm(True);
	ans.set_reason(reason);
	sendFrame(CMD_ACK(CMD_POWER_CURVE), &ans);
}

uint ChargeProtocol::savePowerCurve(PowerCurveList list, int canAddr, int curveType, string billCode)
{
	ChargingStrategy curve;
	QString strSql;
	QDateTime dt;
	QString strBegin, strEnd;
	double value;
	bool failed = false;
	uint curveState = 0;

	if(list.size() <= 0)
		return 0;

	curveState = QDateTime::currentDateTime().toTime_t();

	for(int i = 0; i < list.size(); i++){
		curve = list.Get(i);
		dt = QDateTime::fromMSecsSinceEpoch(curve.begintime().time());
		strBegin = dt.toString("HHmmss");
		dt = QDateTime::fromMSecsSinceEpoch(curve.endtime().time());
		strEnd = dt.toString("HHmmss");
		value = curve.suggestvalue();
		strSql.sprintf("INSERT INTO power_curve (can_addr, curve_state, curve_type, bill_code, \
			begin_time, end_time, suggest_value) VALUES(%d, %d, %d, '%s', %s, %s, '%f');", 
				canAddr, curveState, curveType, billCode.c_str(), strBegin.toAscii().data(), strEnd.toAscii().data(), value);
		if(ProtobufServer::server()->database()->DBSqlExec(strSql.toAscii().data(), DB_PROCESS_RECORD) != 0){
			failed = true;
			break;
		}
	}

	//失败恢复现场
	if(failed){
		strSql.sprintf("DELETE FROM power_curve WHERE can_addr = %d AND bill_code = '%s' AND curve_state = %d;", 
				canAddr, billCode.c_str(), curveState);
		ProtobufServer::server()->database()->DBSqlExec(strSql.toAscii().data(), DB_PROCESS_RECORD);
		return 0;
	}

	//成功清理旧数据
	strSql.sprintf("DELETE FROM power_curve WHERE can_addr = %d AND bill_code = '%s' AND curve_state < %d;", 
			canAddr, billCode.c_str(), curveState);
	ProtobufServer::server()->database()->DBSqlExec(strSql.toAscii().data(), DB_PROCESS_RECORD);

	return curveState;
}

uint ChargeProtocol::saveChargePolicy(PolicyList list, int canAddr, string billCode)
{
	AccountingStrategy policy;
	QString strSql;
	QDateTime dt;
	QString strBegin, strEnd;
	bool failed = false;

	if(list.size() <= 0)
		return 0;

	for(int i = 0; i < list.size(); i++){
		policy = list.Get(i);
		dt = QDateTime::fromMSecsSinceEpoch(policy.begintime().time());
		strBegin = dt.toString("HHmmss");
		dt = QDateTime::fromMSecsSinceEpoch(policy.endtime().time());
		strEnd = dt.toString("HHmmss");
		strSql.sprintf("INSERT INTO charge_policy (can_addr, bill_code, begin_time, end_time, \
			kwh_price, service_price) VALUES(%d, '%s', %s, %s, '%f', '%f');", 
				canAddr, billCode.c_str(), strBegin.toAscii().data(), strEnd.toAscii().data(), 
				policy.kwhprice(), policy.serviceprice());
		if(ProtobufServer::server()->database()->DBSqlExec(strSql.toAscii().data(), DB_PROCESS_RECORD) != 0){
			failed = true;
			break;
		}
	}

	//失败恢复现场
	if(failed){
		strSql.sprintf("DELETE FROM charge_policy WHERE can_addr = %d AND bill_code = '%s'", canAddr, billCode.c_str());
		ProtobufServer::server()->database()->DBSqlExec(strSql.toAscii().data(), DB_PROCESS_RECORD);
		return 0;
	}

	return 1;
}

void ChargeProtocol::parseApplyStartChargeAns(char *data, int len)
{
	StartChargeAns ans;
	uchar result = 0xFF;

	ans.ParseFromArray(data, len);
    writeLog(QString().sprintf("%s {\n%s}", ans.GetDescriptor()->name().c_str(), ans.DebugString().c_str()), 2);

	info.insert(Addr_CanID_Comm, QByteArray(1, ans.canindex()));

	if(ans.result() != True){
		result = ans.reason();
	}

	switch(ans.type()){
		case VinCharge:
			info.insert(Addr_VINApplyStartChargeType_Result, QByteArray(1, result));
			infoType = AddrType_VinApplyStartCharge_Result;
			break;
		case PlateCharge:
			info.insert(Addr_CarLicenseApplyStartChargeType_Result, QByteArray(1, result));
			infoType = AddrType_CarLicenceApplyStartCharge_Result;
			break;
		case CardCharge:
			info.insert(Addr_CardApplyCharge_Result, QByteArray(1, result));
			infoType = AddrType_OutApplyStartChargeByChargeServic_Result;
			break;
		default:
			return;
	}
}

void ChargeProtocol::parseApplyStopChargeAns(char *data, int len)
{
	StopChargeAns ans;
	ans.ParseFromArray(data, len);

    writeLog(QString().sprintf("%s {\n%s}", ans.GetDescriptor()->name().c_str(), ans.DebugString().c_str()), 2);
}

void ChargeProtocol::sendStartChargeAns(uchar can, QByteArray code, BoolEnum result)
{
	StartChargeCmdAns ans;

	ans.set_canindex(can);
	ans.set_billcode(string().append(code.data(), code.length()));
	ans.set_result(result);

	sendFrame(CMD_ACK(CMD_START_CHARGE), &ans);
}

void ChargeProtocol::parseStartCharge(char *data, int len)
{
	StartChargeCmdReq req;
	ChargingStrategy curve;
	QByteArray arCurve;
	uchar cmd;
	uint curveState = 0, policy = 0;

	req.ParseFromArray(data, len);
    writeLog(QString().sprintf("%s {\n%s}", req.GetDescriptor()->name().c_str(), req.DebugString().c_str()), 2);
	
	switch(req.type()){
		case ImmediatelyCharge:	
			cmd = CHARGE_CMD_TYPE_START_CHARGE_NOW;
			break;
		case SaveCharge:
			cmd = CHARGE_CMD_TYPE_START_CHARGE_ECONOMIC;
			break;
		default:
			return;
	}

	//CAN地址
	info.insert(Addr_CanID_Comm, QByteArray(1, req.canindex()));
	//流水号
	info.insert(Addr_Bill_Code, QByteArray().append(req.billcode().c_str(), req.billcode().length())); 
	//充电指令
	info.insert(Addr_ChargeCmd_Ctrl, QByteArray(1, cmd));
	//服务器类型
	info.insert(Addr_ServerType_Comm, QByteArray(1, serverType));
	//功率曲线
	curveState = savePowerCurve(req.cruve(), req.canindex(), req.ctype(), req.billcode());
	info[Addr_Power_Curve_State] = QByteArray((char*)&curveState, sizeof(uint));
	//充电策略
	policy = saveChargePolicy(req.strategy(), req.canindex(), req.billcode());
	info[Addr_Charge_Policy] = QByteArray((char*)&policy, sizeof(int));

    //充电类型
    info.insert(Addr_Energy_ChargeType, QByteArray(1,char(req.type())));

	infoType = AddrType_CmdCtrl_Apply;
}

void ChargeProtocol::parseStopCharge(char *data, int len)
{
	StopChargeCmdReq req;
	uchar cmd;

	req.ParseFromArray(data, len);
    writeLog(QString().sprintf("%s {\n%s}", req.GetDescriptor()->name().c_str(), req.DebugString().c_str()), 2);
	
	cmd = CHARGE_CMD_TYPE_STOP_CHARGE;

	QByteArray arCode;
	arCode.append(req.billcode().c_str(), req.billcode().length());

	info.insert(Addr_CanID_Comm, QByteArray(1, req.canindex()));
	info.insert(Addr_Bill_Code, arCode);
	info.insert(Addr_ChargeCmd_Ctrl, QByteArray(1, cmd));
	info.insert(Addr_ServerType_Comm, QByteArray(1, serverType));

	infoType = AddrType_CmdCtrl_Apply;
}

void ChargeProtocol::parsePauseCharge(char *data, int len)
{
	SuspendChargeReq req;

	req.ParseFromArray(data, len);
    writeLog(QString().sprintf("%s {\n%s}", req.GetDescriptor()->name().c_str(), req.DebugString().c_str()), 2);

	info.insert(Addr_CanID_Comm, QByteArray(1, req.canindex()));
	info.insert(Addr_Bill_Code, QByteArray().append(req.billcode().c_str(), req.billcode().length()));
	info.insert(Addr_ChargeCmd_Ctrl, QByteArray(1, CHARGE_CMD_TYPE_PAUSH_CHARGE));
	info.insert(Addr_ServerType_Comm, QByteArray(1, serverType));

	infoType = AddrType_CmdCtrl_Apply;
}

void ChargeProtocol::parseResumeCharge(char *data, int len)
{
	ResumeChargeReq req;

	req.ParseFromArray(data, len);
    writeLog(QString().sprintf("%s {\n%s}", req.GetDescriptor()->name().c_str(), req.DebugString().c_str()), 2);

	info.insert(Addr_CanID_Comm, QByteArray(1, req.canindex()));
	info.insert(Addr_Bill_Code, QByteArray().append(req.billcode().c_str(), req.billcode().length()));
	info.insert(Addr_ChargeCmd_Ctrl, QByteArray(1, CHARGE_CMD_TYPE_RESUME));
	info.insert(Addr_ServerType_Comm, QByteArray(1, serverType));

	infoType = AddrType_CmdCtrl_Apply;
}

void ChargeProtocol::sendStopChargeAns(uchar can, QByteArray code, BoolEnum result)
{
	StopChargeCmdAns ans;

	ans.set_canindex(can);
	ans.set_billcode(string().append(code.data(), code.length()));
	ans.set_result(result);

	sendFrame(CMD_ACK(CMD_STOP_CHARGE), &ans);
}

void ChargeProtocol::sendPauseChargeAns(uchar can, QByteArray code, BoolEnum result)
{
	SuspendChargeAns ans;

	ans.set_canindex(can);
	ans.set_billcode(string().append(code.data(), code.length()));
	ans.set_result(result);

	sendFrame(CMD_ACK(CMD_PAUSE_CHARGE), &ans);
}

void ChargeProtocol::sendResumeChargeAns(uchar can, QByteArray code, BoolEnum result)
{
	ResumeChargeAns ans;

	ans.set_canindex(can);
	ans.set_code(string().append(code.data(), code.length()));
	ans.set_result(result);

	sendFrame(CMD_ACK(CMD_RESUME_CHARGE), &ans);
}

bool ChargeProtocol::createBill(BillInfo *bill, QString strBillId, int idType)
{
	BillStatus bill_status;
	BillType bill_type = ChargingBill;
	QDateTime dt;
	QString strSql, strGuid;
	uchar canAddr;
	int col = 0;
	struct db_result_st result;
	DateTime *time;

	if(idType == 1){
		strSql.sprintf("SELECT CanAddr, UUIDOwn, BillCode, OrderStatus, DevStopReason, \
				StartTime, EndTime, StartEnergy, EndEnergy, StartSoc, StopSoc, CardNo, \
                VIN, CarLisence,OrderType,GunNum,ChargeType,ChargeWay FROM charge_order \
                WHERE OrderSync < 2 AND UUIDOwn = '%s';", strBillId.toAscii().data());
	}else{
		strSql.sprintf("SELECT CanAddr, UUIDOwn, BillCode, OrderStatus, DevStopReason, \
				StartTime, EndTime, StartEnergy, EndEnergy, StartSoc, StopSoc, CardNo, \
                VIN, CarLisence,OrderType,GunNum, ChargeType,ChargeWay FROM charge_order \
                WHERE OrderSync < 2 AND BillCode = '%s';", strBillId.toAscii().data());
	}

	if(ProtobufServer::server()->database()->DBSqlQuery(strSql.toAscii().data(), &result, DB_PROCESS_RECORD) == 0){
		if(result.row <= 0){
			ProtobufServer::server()->database()->DBQueryFree(&result);
			return false;
		}
		canAddr = atoi(result.result[col++]);
		strGuid = result.result[col++];
        bill->set_ctrlbillcode(result.result[1]);
		bill->set_billcode(result.result[col++]);

        switch(atoi(result.result[col++])){
			case ORDER_STATUS_ING:
				bill_status = ChargingBillStatus;
				break;
			case ORDER_STATUS_QUEUE:
				bill_status = QueueingBillStatus;
				break;
			case ORDER_STATUS_OK:
				bill_status = FinishedBillStatus;
				break;
			case ORDER_STATUS_FAIL:
				bill_status = StartChargeFailedBillStatus;
				break;
			default:
				bill_status = DefaultBill;
				break;
		}

		bill->set_status(bill_status);
		bill->set_billtype(bill_type);
		bill->set_reason(atoi(result.result[col++]));

		dt = QDateTime::fromString(QString(result.result[col++]), "yyyy-MM-dd HH:mm:ss");
		time = bill->mutable_begintime();
        if(dt.isValid())
            time->set_time(dt.toMSecsSinceEpoch());

		dt = QDateTime::fromString(QString(result.result[col++]), "yyyy-MM-dd HH:mm:ss");
		time = bill->mutable_endtime();
        if(dt.isValid())
            time->set_time(dt.toMSecsSinceEpoch());

		bill->set_beginmeter((double)atoi(result.result[col++]) / 100.0);
		bill->set_endmeter((double)atoi(result.result[col++]) / 100.0);
		bill->set_beginsoc(atoi(result.result[col++]));
		bill->set_endsoc(atoi(result.result[col++]));
		bill->set_cardno(result.result[col++]);
		bill->set_vin(result.result[col++]);
        bill->set_plate(result.result[col++]);
        bill->set_emergency(atoi(result.result[col++]) == ORDER_EMERGENCY ? True : False);
        bill->set_gunnum(atoi(result.result[col++]));
        bill->set_chargetype((StartChargeType)atoi(result.result[col++]));
        bill->set_chargeway((StartChargeWay)atoi(result.result[col++]));

		struct db_result_st resultFrozen;
		double dEnergy = 0.0;

		strSql.sprintf("SELECT NowTime, NowEnergy FROM charge_energy_%d_table \
				WHERE UUIDOwn = '%s' AND ChargeType = 1 ORDER BY NowTime ASC;",
				canAddr, strGuid.toAscii().data());
		if(ProtobufServer::server()->database()->DBSqlQuery(strSql.toAscii().data(), &resultFrozen, DB_REAL_RECORD) == 0){
			ChargeFrozen *frozen;
			for(int i = 0; i < resultFrozen.row; i++){
				frozen = bill->add_chargefrozen();
				dt = QDateTime::fromString(QString(resultFrozen.result[i * resultFrozen.column]), "yyyy-MM-dd HH:mm:ss");

                time = frozen->mutable_frozentime();
				time->set_time(dt.toMSecsSinceEpoch());
				dEnergy = (double)atoi(resultFrozen.result[i * resultFrozen.column + 1]) / 100.0;

				frozen->set_kwh(dEnergy);
			}
			ProtobufServer::server()->database()->DBQueryFree(&resultFrozen);
		}

		strSql.sprintf("SELECT NowTime, NowEnergy FROM charge_energy_%d_table \
				WHERE UUIDOwn = '%s' AND ChargeType = 2 ORDER BY NowTime ASC;",
				canAddr, strGuid.toAscii().data());
		if(ProtobufServer::server()->database()->DBSqlQuery(strSql.toAscii().data(), &resultFrozen, DB_REAL_RECORD) == 0){
			DischargeFrozen *frozen;
			for(int i = 0; i < resultFrozen.row; i++){
				frozen = bill->add_dischargefrozen();
				dt = QDateTime::fromString(QString(resultFrozen.result[i * resultFrozen.column]), "yyyy-MM-dd HH:mm:ss");

                time = frozen->mutable_frozentime();
				time->set_time(dt.toMSecsSinceEpoch());
				dEnergy = (double)atoi(resultFrozen.result[i * resultFrozen.column + 1]) / 100.0;

				frozen->set_kwh(dEnergy);
			}
			ProtobufServer::server()->database()->DBQueryFree(&resultFrozen);
		}
		
		ProtobufServer::server()->database()->DBQueryFree(&result);
		return true;
	}

	return false;
}

void *ChargeProtocol::billProc(void *p)
{
	ChargeProtocol *proto = (ChargeProtocol *)p;

	proto->sendUnConfirmedBill(proto->billCanAddr);

	proto->billCanAddr = 0;

	return NULL;
}

void ChargeProtocol::unconfirmedBillCall(char *data, int len)
{
	BillsNotConfirmedReq req;	

	req.ParseFromArray(data, len);
    writeLog(QString().sprintf("%s {\n%s}", req.GetDescriptor()->name().c_str(), req.DebugString().c_str()), 2);
	billCanAddr = req.canindex();

//	pthread_create(&billThread, NULL, billProc, (void *)this);	  //add by zrx 更改为一条一条上传

    sendUnConfirmedBillOne(billCanAddr);
}

void ChargeProtocol::codeBillCall(char *data, int len)
{
	SpecificBillReq req;	
	SpecificBillAns ans;
	BoolEnum result = False;
	BillUploadReq upload;
	BillInfo *bill;

	req.ParseFromArray(data, len);
    writeLog(QString().sprintf("%s {\n%s}", req.GetDescriptor()->name().c_str(), req.DebugString().c_str()), 2);

	bill = upload.mutable_bill();
	if(createBill(bill, req.billcode().c_str(), 0)){
		result = True;

		upload.set_canindex(req.canindex());
		DateTime *time = upload.mutable_sendtime();
		time->set_time(QDateTime::currentDateTime().toMSecsSinceEpoch());

        sendFrame(CMD_UPLOAD_BILL, &upload);
	}

	ans.set_result(result);
	ans.set_confirmed(True);

	sendFrame(CMD_ACK(CMD_CODE_BILL_CALL), &ans);
}

void ChargeProtocol::parseUploadBillAns(char *data, int len)
{
	BillUploadAns ans;	
    QString ctrlbillcode,billcode;
    QString sql;

	ans.ParseFromArray(data, len);
    writeLog(QString().sprintf("%s {\n%s}", ans.GetDescriptor()->name().c_str(), ans.DebugString().c_str()), 2);

    if(ans.confirm() == True){
        ctrlbillcode = ans.ctrlbillcode().c_str();
        billcode = ans.billcode().c_str();
        if((bSendBillStart == true) && (((ChargeServer *)ProtobufServer::server())->bBillResendEnd == false)){
            info.insert(Addr_Resend_CmdAck,QByteArray(1,0x96));
            info.insert(Addr_Order_GUID,QByteArray().append(ctrlbillcode));
            info.insert(Addr_Bill_Code,QByteArray().append(billcode));
        }
        else{
            if(ctrlbillcode != ""){
              sql.sprintf("UPDATE charge_order SET OrderSync = 3 WHERE UUIDOwn = '%s' AND ((OrderStatus <> %d AND OrderType = %d) OR (OrderType = %d));",\
                          ctrlbillcode.toAscii().data(),ORDER_STATUS_ING, ORDER_NORMAL, ORDER_EMERGENCY);
            }
            if(billcode != ""){
              sql.sprintf("UPDATE charge_order SET OrderSync = 3 WHERE BillCode = '%s' AND ((OrderStatus <> %d AND OrderType = %d) OR (OrderType = %d));",\
                          billcode.toAscii().data(),ORDER_STATUS_ING, ORDER_NORMAL, ORDER_EMERGENCY);
            }

            ProtobufServer::server()->database()->DBSqlExec(sql.toAscii().data(), DB_PROCESS_RECORD);
        }
    }
}

void ChargeProtocol::billNotify(uchar canAddr, QString strGuid)
{
	BillUploadReq req;
	BillInfo *bill;

	req.set_canindex(canAddr);
	bill = req.mutable_bill();

	if(createBill(bill, strGuid)){
		DateTime *time = req.mutable_sendtime();
		time->set_time(QDateTime::currentDateTime().toMSecsSinceEpoch());

        sendFrame(CMD_UPLOAD_BILL, &req);
	}
}

void ChargeProtocol::sendUnConfirmedBill(int canAddr)
{
    struct db_result_st result;
	QString strSql;
	QDateTime dt;

	//超过15分钟的失败订单不上传
	dt.setTime_t(QDateTime::currentDateTime().toTime_t() - 15 * 60);
    strSql.sprintf("UPDATE charge_order SET OrderSync = 255 WHERE OrderStatus = 1 AND StartTime < '%s';",
			dt.toString("yyyy-MM-dd HH:mm:ss").toAscii().data());
	ProtobufServer::server()->database()->DBSqlExec(strSql.toAscii().data(), DB_PROCESS_RECORD);

	if(canAddr > 0){
        strSql.sprintf("SELECT CanAddr, UUIDOwn FROM charge_order WHERE OrderSync = 255 AND \
				CanAddr = %d AND ((OrderStatus <> %d AND OrderType = %d) OR (OrderType = %d));", 
				canAddr, ORDER_STATUS_ING, ORDER_NORMAL, ORDER_EMERGENCY);
	}else{
		strSql.sprintf("SELECT CanAddr, UUIDOwn FROM charge_order WHERE OrderSync = 0 AND \
				CanAddr = %d AND ((OrderStatus <> %d AND OrderType = %d) OR (OrderType = %d));", 
				canAddr, ORDER_STATUS_ING, ORDER_NORMAL, ORDER_EMERGENCY);
	}
	if(ProtobufServer::server()->database()->DBSqlQuery(strSql.toAscii().data(), &result, DB_PROCESS_RECORD) == 0){
		for(int i = 0; i < result.row; i++){
			if(terminated){
				ProtobufServer::server()->database()->DBQueryFree(&result);
				return;
			}

			BillUploadReq req;
			BillInfo *bill;

			bill = req.mutable_bill();
			req.set_canindex(atoi(result.result[i * result.column]));

			createBill(bill, result.result[i * result.column + 1]);

			DateTime *time = req.mutable_sendtime();
			time->set_time(QDateTime::currentDateTime().toMSecsSinceEpoch());

			if(!sendFrame(CMD_UPLOAD_BILL, &req))
				return;
		}

		ProtobufServer::server()->database()->DBQueryFree(&result);
	}

	BillsNotConfirmedAns ans;
	ans.set_finished(True);

	DateTime *time = ans.mutable_sendtime();
	time->set_time(QDateTime::currentDateTime().toMSecsSinceEpoch());

	sendFrame(CMD_ACK(CMD_UNCONFIRMED_BILL_CALL), &ans);
}

//发送未被确认订单 add by zrx
void ChargeProtocol::sendUnConfirmedBillOne(int canAddr)
{
    struct db_result_st result;
    QString strSql;
    QDateTime dt;

    //超过15分钟的失败订单不上传
    dt.setTime_t(QDateTime::currentDateTime().toTime_t() - 15 *60);
    strSql.sprintf("UPDATE charge_order SET OrderSync = 255 WHERE OrderStatus = 1 AND StartTime < '%s';",dt.toString("yyyy-MM-dd HH:mm:ss").toAscii().data());
    ProtobufServer::server()->database()->DBSqlExec(strSql.toAscii().data(), DB_PROCESS_RECORD);

    //平台未确认订单召唤，订单上传前将订单状态0（应急&&充电中）更新为状态1
    strSql.sprintf("UPDATE charge_order SET OrderSync = 1 WHERE OrderSync = 0 AND OrderStatus = %d AND OrderType = %d;",ORDER_STATUS_ING, ORDER_EMERGENCY);
    ProtobufServer::server()->database()->DBSqlExec(strSql.toAscii().data(), DB_PROCESS_RECORD);

    //查询未被确认的订单
    queryUnConfirmedBillOne(canAddr);
}

void ChargeProtocol::queryUnConfirmedBillOne(int canAddr) //查询未被确认的订单
{
    struct db_result_st result;
    QString strSql;
    //查询未被确认的订单
    if(canAddr > 0){
        strSql.sprintf("SELECT CanAddr, UUIDOwn,BillCode FROM charge_order WHERE OrderSync < 2 \
                       AND CanAddr = %d AND OrderStatus <> 0 AND \
                       ((OrderStatus <> %d AND OrderType = %d) OR (OrderType = %d));",\
                       canAddr,ORDER_STATUS_ING,ORDER_NORMAL, ORDER_EMERGENCY);
    }
    else{
        strSql.sprintf("SELECT CanAddr, UUIDOwn,BillCode FROM charge_order WHERE OrderSync < 2 \
                       AND OrderStatus <> 0 AND \
                       ((OrderStatus <> %d AND OrderType = %d) OR (OrderType = %d));",\
                       ORDER_STATUS_ING,ORDER_NORMAL, ORDER_EMERGENCY);
    }
    if(ProtobufServer::server()->database()->DBSqlQuery(strSql.toAscii().data(), &result, DB_PROCESS_RECORD) == 0){  //查询成功
        if(result.row > 0){  //如果有上传订单，则上传第一条
            BillUploadReq req;
            BillInfo *bill;

            bill = req.mutable_bill();
            req.set_canindex(atoi(result.result[0]));

            createBill(bill, result.result[1]);

            DateTime *time = req.mutable_sendtime();
            time->set_time(QDateTime::currentDateTime().toMSecsSinceEpoch());

            if(!sendFrame(CMD_UPLOAD_BILL, &req))
                return;
            sendMessage(req.DebugString().c_str());
            //发送第一条后标志位为1
            bSendBillStart = true;  //未被确认订单流程开始标志位

            info.insert(Addr_CanID_Comm, QByteArray(1,(char)(atoi(result.result[0]))));
            info.insert(Addr_Resend_Cmd,QByteArray(1,0x16));
            QString qstrOrderGuid,qstrBillCode;
            qstrOrderGuid = result.result[1];
            qstrBillCode = result.result[2];
            info.insert(Addr_Order_GUID, qstrOrderGuid.toAscii());
            info.insert(Addr_Bill_Code, qstrBillCode.toAscii());

            ProtobufServer::server()->database()->DBQueryFree(&result);
        }
        else{//没有则上传全部完成
            //所有未召唤订单上传成功后，集控发送未被确认所有订单召唤.响应（0xC1）
            BillsNotConfirmedAns ans;
            ans.set_finished(True);

            DateTime *time = ans.mutable_sendtime();
            time->set_time(QDateTime::currentDateTime().toMSecsSinceEpoch());

            sendFrame(CMD_ACK(CMD_UNCONFIRMED_BILL_CALL), &ans);
            bSendBillStart = false;
//            emit emergency(3);  //3-只更改localEmergency的值
            ((ChargeServer *)ProtobufServer::server())->ProtoEmergency(3);
            ProtobufServer::server()->database()->DBQueryFree(&result);
        }
    }
}

void ChargeProtocol::sendUnConfirmedBillSendReult(InfoMap map)  //未确认订单召唤返回失败
{
    int result;
    result = map[Addr_BillSendReult].at(0);
    QString ctrlbillcode,billcode;
    QString sql,sqlselect;
    struct db_result_st selectresult;
    int iOrderStatus = -1;
    int iOrderSync = -1;
    ctrlbillcode = map[Addr_Order_GUID].data();
    billcode = map[Addr_Bill_Code].data();
    if(result == 1){  //单条上传成功，开始上传第二条
        if(ctrlbillcode != ""){
            sqlselect.sprintf("SELECT OrderStatus,OrderSync FROM charge_order WHERE UUIDOwn = '%s';",ctrlbillcode.toAscii().data());
        }
        if(billcode != ""){
            sqlselect.sprintf("SELECT OrderStatus,OrderSync FROM charge_order WHERE BillCode = '%s';",billcode.toAscii().data());
        }

        if(ProtobufServer::server()->database()->DBSqlQuery(sqlselect.toAscii().data(), &selectresult, DB_PROCESS_RECORD) == 0){  //查询成功 
            if(selectresult.row > 0){
                iOrderStatus = atoi(selectresult.result[0]); 
                iOrderSync = atoi(selectresult.result[1]); 
            }
            ProtobufServer::server()->database()->DBQueryFree(&selectresult);
        }
        
        if((iOrderStatus == ORDER_STATUS_OK)&&(iOrderSync == 0)){ //将订单应答后OrderSync的值更新为3
            if(ctrlbillcode != ""){
              sql.sprintf("UPDATE charge_order SET OrderSync = 3 WHERE UUIDOwn = '%s';",ctrlbillcode.toAscii().data());
            }
            if(billcode != ""){
                sql.sprintf("UPDATE charge_order SET OrderSync = 3 WHERE BillCode = '%s'",billcode.toAscii().data());
            }
        }
        else if((iOrderStatus == ORDER_STATUS_OK)&&(iOrderSync == 1)){ //将订单应答后OrderSync的值更新为0
            if(ctrlbillcode != ""){
              sql.sprintf("UPDATE charge_order SET OrderSync = 0 WHERE UUIDOwn = '%s';",ctrlbillcode.toAscii().data());
            }
            if(billcode != ""){
                sql.sprintf("UPDATE charge_order SET OrderSync = 0 WHERE BillCode = '%s'",billcode.toAscii().data());
            }
        }
        else if((iOrderStatus == ORDER_STATUS_ING)&&(iOrderSync == 1)){ //将订单应答后OrderSync的值更新为2
            if(ctrlbillcode != ""){
              sql.sprintf("UPDATE charge_order SET OrderSync = 2 WHERE UUIDOwn = '%s';",ctrlbillcode.toAscii().data());
            }
            if(billcode != ""){
                sql.sprintf("UPDATE charge_order SET OrderSync = 2 WHERE BillCode = '%s'",billcode.toAscii().data());
            }
        }


        if(ProtobufServer::server()->database()->DBSqlExec(sql.toAscii().data(), DB_PROCESS_RECORD) == 0){
            writeLog("Order upload success!",2);
        }

        if(bSendBillStart == true){
            infoType = AddrType_Unknown;
            info.clear();
            queryUnConfirmedBillOne(0);
            emit parsed(info, infoType);
        }
    }
    else{//单条上传失败，重传此条
        writeLog("Order upload fail,订单号不一致",2);
        queryUnConfirmedBillOne(map[Addr_CanID_Comm].at(0));
    }
}

void ChargeProtocol::createChargerState(ChargerState *state, char logicStatus)
{
	ChargerStateEnum en;

	switch(logicStatus){
		case CHARGE_STATUS_FREE:
			en = Standby;
			break;
		case CHARGE_STATUS_GUN_STANDBY:
			en = Plug;
			break;
		case CHARGE_STATUS_WAITING:
			en = Waiting;
			break;
		case CHARGE_STATUS_QUEUE1:
		case CHARGE_STATUS_QUEUE2:
		case CHARGE_STATUS_QUEUE3:
		case CHARGE_STATUS_QUEUE4:
		case CHARGE_STATUS_QUEUE5:
		case CHARGE_STATUS_QUEUE6:
		case CHARGE_STATUS_QUEUE7:
			en = Queueing;
			break;
		case CHARGE_STATUS_FINISH:
			en = Finished;
			break;
		case CHARGE_STATUS_FULL:
			en = Fulled;
			break;
		case CHARGE_STATUS_CHARGING:
			en = Charging;
			break;
		case CHARGE_STATUS_SWITCH:
			en = Changing;
			break;
		case CHARGE_STATUS_PAUSH:
			en = Pause;
			break;
		case CHARGE_STATUS_CARPAUSH:
			en = EVPause;
			break;
		case CHARGE_STATUS_DEVPAUSH:
			en = PlatformPause;
			break;
		case CHARGE_STATUS_CSCUPAUSH:
			en = CtrlPause;
			break;
		case CHARGE_STATUS_LIMIT:
			en = Restrict;
			break;
		case CHARGE_STATUS_STARTING:
			en = Starting;
			break;
		case CHARGE_STATUS_DISCONNECT:
			en = Offline;
			break;
		case CHARGE_STATUS_SLAVEGUN:
			en = Sublance;
			break;
		case CHARGE_STATUS_DISCHARGING:
			en = Discharging;
			break;
		case CHARGE_STATUS_FAULT:
			en = Fault;
			break;
        case CHARGE_STATUS_SCHEDUING:
            en = Dispatching;
            break;
		default:
			en = DefaultChargerState;
			break;
	}

	state->set_state(en);
	DateTime *time = state->mutable_timestamp();
	time->set_time(QDateTime::currentDateTime().toMSecsSinceEpoch());
}

void ChargeProtocol::chargerStateCall(char *data, int len)
{
	ChargerStateReq req;
	CommAns ans;
	QByteArray canRange;

	req.ParseFromArray(data, len);
    writeLog(QString().sprintf("%s {\n%s}", req.GetDescriptor()->name().c_str(), req.DebugString().c_str()), 2);

	if(req.canindex_size() <= 0){
		canRange = _canRange;
	}else{
		for(int i = 0; i < req.canindex_size(); i++){
			canRange.append(req.canindex(i));
		}
	}

	ChargerStateChangeReq charger;
	for(int i = 0; i < canRange.length(); i++){
		uchar canAddr = canRange.at(i);
		TerminalStatus status;
        CHARGE_STEP step;
        if(ProtobufServer::server()->cache()->QueryTerminalStatus(canAddr, status)){
			ChargerState *state = charger.add_state();
			state->set_canindex(status.cCanAddr);
			createChargerState(state, status.cStatus);

            if(ProtobufServer::server()->cache()->QueryChargeStep(canAddr, step)){
                state->set_billcode(string().append(QByteArray((char *)step.sBillCode, strlen(step.sBillCode)).data()));
                //if(step.sOrderUUID[0] != 0)
                if(strcmp(step.sOrderUUID, "") != 0)
                    state->set_ctrlbillcode(string().append(QByteArray((char *)step.sOrderUUID, LENGTH_GUID_NO).toHex().data()));
            }
        }
	}

	DateTime *time = charger.mutable_sendtime();
	time->set_time(QDateTime::currentDateTime().toMSecsSinceEpoch());
    charger.set_reason(CallAnswer);

	sendFrame(CMD_CHARGER_NOTIFY, &charger);

	ans.set_confirm(True);
	sendFrame(CMD_ACK(CMD_CHARGER_CALL), &ans);
}

void ChargeProtocol::measureCall(char *data, int len)
{
	TelemetryReq req;
	CommAns ans;
	QByteArray canRange;
	TerminalStatus status;

	req.ParseFromArray(data, len);
    writeLog(QString().sprintf("%s {\n%s}", req.GetDescriptor()->name().c_str(), req.DebugString().c_str()), 2);

	if(req.canindex_size() <= 0){
		canRange = _canRange;
	}else{
		for(int i = 0; i < req.canindex_size(); i++){
			canRange.append(req.canindex(i));
		}
	}

	for(int i = 0; i < canRange.count(); i++){
        if(!ProtobufServer::server()->cache()->QueryTerminalStatus(canRange.at(i), status)){
            continue;
        }

        sendMeasure(canRange.at(i), status);
	}

	ans.set_confirm(True);
	sendFrame(CMD_ACK(CMD_MEASURE_CALL), &ans);

}

void  ChargeProtocol::sendMeasure(uchar canAddr, TerminalStatus &status, bool bCall)
{
	TelemetryNotifyReq notify;

    notify.set_canindex(canAddr);
	notify.set_reason(bCall ? CallAnswer : IntervalSend);
	notify.add_type(Va);
	notify.add_value(status.stFrameRemoteMeSurement1.A_voltage);
	notify.add_type(Vb);
	notify.add_value(status.stFrameRemoteMeSurement1.B_voltage);
	notify.add_type(Vc);
	notify.add_value(status.stFrameRemoteMeSurement1.C_voltage);
	notify.add_type(Vdc);
	notify.add_value(status.stFrameRemoteMeSurement1.voltage_of_dc);
	notify.add_type(Ia);
	notify.add_value(status.stFrameRemoteMeSurement1.A_current);
	notify.add_type(Ib);
	notify.add_value(status.stFrameRemoteMeSurement1.B_current);
	notify.add_type(Ic);
	notify.add_value(status.stFrameRemoteMeSurement1.C_current);
	notify.add_type(Idc);
	notify.add_value(status.stFrameRemoteMeSurement1.current_of_dc);
	notify.add_type(N);
	notify.add_value(status.stFrameRemoteMeSurement1.neutralLine_current);
	notify.add_type(P);
	notify.add_value(status.stFrameRemoteMeSurement1.active_power);
	notify.add_type(Q);
	notify.add_value(status.stFrameRemoteMeSurement1.reactive_power);
	notify.add_type(PF);
	notify.add_value(status.stFrameRemoteMeSurement1.power_factor);
	notify.add_type(VU);
	notify.add_value(status.stFrameRemoteMeSurement1.voltage_unbalance_rate);
	notify.add_type(CU);
	notify.add_value(status.stFrameRemoteMeSurement1.current_unbalance_rate);
	notify.add_type(Fap);
	notify.add_value((double)status.stFrameRemoteMeSurement2.active_electric_energy / 100.0);
	notify.add_type(Rap);
	notify.add_value((double)status.stFrameRemoteMeSurement2.ReverseActiveEnergy / 100.0);
	notify.add_type(Frp);
	notify.add_value((double)status.stFrameRemoteMeSurement2.reactive_electric_energy / 100.0);
	notify.add_type(Rrp);
	notify.add_value((double)status.stFrameRemoteMeSurement2.ReverseReactiveEnergy /100.0);


    CHARGE_STEP step;
	//memset(&step, 0, sizeof(CHARGE_STEP));
    if(ProtobufServer::server()->cache()->QueryChargeStep(canAddr, step)){
        notify.set_billcode(string().append(QByteArray((char *)step.sBillCode, strlen(step.sBillCode)).data()));
        //if(step.sOrderUUID[0] != 0)
        if(strcmp(step.sOrderUUID, "") != 0)
            notify.set_ctrlbillcode(string().append(QByteArray((char *)step.sOrderUUID, LENGTH_GUID_NO).toHex().data()));
    }

	DateTime *time = notify.mutable_sendtime();
	time->set_time(QDateTime::currentDateTime().toMSecsSinceEpoch());

	sendFrame(CMD_MEASURE_NOTIFY, &notify);
}

void ChargeProtocol::signalCall(char *data, int len)
{
	StateReq req;
	CommAns ans;
	QByteArray canRange;

	req.ParseFromArray(data, len);
    writeLog(QString().sprintf("%s {\n%s}", req.GetDescriptor()->name().c_str(), req.DebugString().c_str()), 2);

	if(req.canindex_size() <= 0){
		canRange = _canRange;
	}else{
		for(int i = 0; i < req.canindex_size(); i++){
			canRange.append(req.canindex(i));
		}
	}

	for(int i = 0; i < canRange.count(); i++){
		StateNotifyReq notify;
		uchar canAddr = canRange.at(i);
		TerminalStatus status;
        CHARGE_STEP stChargeStep;//存放从缓存中读出的状态机
		if(ProtobufServer::server()->cache()->QueryTerminalStatus(canAddr, status)){
			notify.set_canindex(canAddr);
			notify.set_reason(CallAnswer);
			notify.add_type(Interface);
			notify.add_value(status.stFrameRemoteSingle.charge_interface_type);
			notify.add_type(Connect);
			notify.add_value(status.stFrameRemoteSingle.link_status);
			notify.add_type(Relay);
			notify.add_value(status.stFrameRemoteSingle.relay_status);
			notify.add_type(Parking);
			notify.add_value(status.stFrameRemoteSingle.parking_space);
			notify.add_type(Charger);
			notify.add_value(status.stFrameRemoteSingle.charge_status);
			notify.add_type(StateTypeFault);
			notify.add_value(status.stFrameRemoteSingle.status_fault);
			notify.add_type(BmsFault);
			notify.add_value(status.stFrameRemoteSingle.BMS_fault);
			notify.add_type(Reason);
			notify.add_value(status.stFrameRemoteSingle.Stop_Result);
			notify.add_type(Stategy);
			notify.add_value(status.stFrameRemoteSingle.QunLunCeLue & 0xF0 >> 4);
			notify.add_type(AuxType);
			notify.add_value(status.stFrameRemoteSingle.AuxPowerType);
			notify.add_type(ControlMode);
			notify.add_value(status.stFrameRemoteSingle.QunLunCeLue & 0x0F);
		}

        if(ProtobufServer::server()->cache()->QueryChargeStep(canAddr, stChargeStep)){
            notify.set_billcode(string().append(QByteArray((char *)stChargeStep.sBillCode, strlen(stChargeStep.sBillCode)).data()));
            if(strcmp(stChargeStep.sOrderUUID,"") != 0)
                notify.set_ctrlbillcode(string().append(QByteArray((char *)stChargeStep.sOrderUUID, LENGTH_GUID_NO).toHex().data()));
        }

        DateTime *time = notify.mutable_sendtime();
        time->set_time(QDateTime::currentDateTime().toMSecsSinceEpoch());
        sendFrame(CMD_SIGNAL_NOTIFY, &notify);
	}

	ans.set_confirm(True);
	sendFrame(CMD_ACK(CMD_SIGNAL_CALL), &ans);
}

void ChargeProtocol::bmsChargeCall(char *data, int len)
{
	BmsReq req;
	CommAns ans;
	QByteArray canRange;
	TerminalStatus status;
	uchar canAddr;

	req.ParseFromArray(data, len);
    writeLog(QString().sprintf("%s {\n%s}", req.GetDescriptor()->name().c_str(), req.DebugString().c_str()), 2);

	if(req.canindex_size() <= 0){
		canRange = _canRange;
	}else{
		for(int i = 0; i < req.canindex_size(); i++){
			canRange.append(req.canindex(i));
		}
	}

	for(int i = 0; i < canRange.count(); i++){
		canAddr = canRange.at(i);
		if(!ProtobufServer::server()->cache()->QueryTerminalStatus(canAddr, status))
			continue;

		if(canAddr >= ID_MinDCCanID && canAddr <= ID_MaxDCCanID){ 
			sendBmsCharge(canAddr, status);	
		}
	}

	ans.set_confirm(True);
	sendFrame(CMD_ACK(CMD_BMS_CHARGE_CALL), &ans);
}

void ChargeProtocol::sendBmsCharge(uchar canAddr, TerminalStatus &status, bool bCall)
{
	BmsNotifyReq notify;
	BmsCharging *bms;
    CHARGE_STEP stChargeStep;//存放从缓存中读出的状态机

	notify.set_canindex(canAddr);
	notify.set_vin((const char *)status.stFrameBmsInfo.BMS_car_VIN);
	notify.set_reason(bCall ? CallAnswer : IntervalSend);
	bms = notify.mutable_charginginfo();
	bms->set_vdemand(status.stFrameBmsInfo.BMS_need_voltage);
	bms->set_idemand(status.stFrameBmsInfo.BMS_need_current);
	bms->set_currentsoc(status.stFrameBmsInfo.batery_SOC);
	bms->set_remaintime(status.stFrameBmsInfo.LeftTime);
	bms->set_chargemode((ChargingMode)status.stFrameBmsInfo.ChargeType);
	bms->set_vmeasure(status.stFrameBmsInfo.ChargeVoltageMeasured);
	bms->set_imeasure(status.stFrameBmsInfo.ChargeCurrentMeasured);
	bms->set_vindmax(status.stFrameBmsInfo.max_batery_voltage);
	bms->set_vindmaxcode(status.stFrameBmsInfo.SingleBatteryNum);
	bms->set_vindmin(status.stFrameBmsInfo.lowest_charge_voltage);
	bms->set_vindmincode(status.stFrameBmsInfo.MinTempPointNum);
	bms->set_tmax(status.stFrameBmsInfo.max_batery_temperature);
	bms->set_tmaxcode(status.stFrameBmsInfo.MaxTempPointNum);
	bms->set_tmin(status.stFrameBmsInfo.lowest_battery_temperature);
	bms->set_tmincode(status.stFrameBmsInfo.MinTempPointNum);
	bms->set_chargeallow(status.stFrameBmsInfo.ChargePermitFlag == 1 ? True : False);
	switch(status.stFrameRemoteSingle.BMS_fault){
		case 0x70:
			bms->set_vindhigh(True);
			break;
		case 0x69:
			bms->set_vindlow(True);
			break;
		case 0x71:
			bms->set_sohigh(True);
			break;
		case 0x72:
			bms->set_soclow(True);
			break;
		case 0x73:
			bms->set_ihigh(True);
			break;
		case 0x74:
			bms->set_thigh(True);
			break;
		case 0x75:
			bms->set_insulation(True);
			break;
		case 0x76:
			bms->set_outputconnector(True);
			break;
	}

    if(ProtobufServer::server()->cache()->QueryChargeStep(canAddr, stChargeStep)){
        //新增订单号 add by zrx
        notify.set_billcode(string().append(QByteArray((char *)stChargeStep.sBillCode, strlen(stChargeStep.sBillCode)).data()));
        notify.set_ctrlbillcode(string().append(QByteArray((char *)stChargeStep.sOrderUUID, LENGTH_GUID_NO).toHex().data()));
    }

    DateTime *time = notify.mutable_sendtime();
	time->set_time(QDateTime::currentDateTime().toMSecsSinceEpoch());

	sendFrame(CMD_BMS_CHARGE_NOTIFY, &notify);
}

void ChargeProtocol::sendBmsParam(uchar canAddr, TerminalStatus &status)
{
	BmsParamNotifyReq req;
	BmsShakehands *shake;
	BmsConfig *config;
	BmsChargeFinish *finish;

	shake = req.mutable_shakehandsinfo();
	config = req.mutable_configinfo();
	finish = req.mutable_chargefinishinfo();

	req.set_canindex(canAddr);
	req.set_vin((const char*)status.stFrameBmsInfo.BMS_car_VIN);
	req.set_reason(IntervalSend);
	req.mutable_sendtime()->set_time(QDateTime::currentDateTime().toMSecsSinceEpoch());

	//握手阶段
	shake->set_bmsversion(status.stFrameBmsHand.BMSProtocolVer);
	shake->set_volmaxallowed(status.stFrameBmsHand.MaxAllowedVoltage);
	shake->set_batterytype(status.stFrameBmsHand.BatteryType);
	shake->set_capacityrated(status.stFrameBmsHand.BatteryRatedCapacity);
	shake->set_voltagerated(status.stFrameBmsHand.BatteryRatedVoltage);
	shake->set_batteryvendor(status.stFrameBmsHand.BatteryManufacturer);
	shake->set_batterysequence(status.stFrameBmsHand.BatterySerialNum);
	shake->set_producedate(status.stFrameBmsHand.BatteryProduceDate);
	shake->set_chargecount(status.stFrameBmsHand.BatteryChargeTime);
	shake->set_rightidentifier(status.stFrameBmsHand.BatteryOwnerFlag);
	shake->set_bmsversion(status.stFrameBmsHand.BMSSoftwareVer);
	//参数配置阶段
	config->set_vindallowedmax(status.stFrameBmsParam.SingleBatteryMaxAllowedVoltage);
	config->set_iallowedmax(status.stFrameBmsParam.MaxAllowedCurrent);
	config->set_energyrated(status.stFrameBmsParam.BatteryTotalEnergy);
	config->set_vallowedmax(status.stFrameBmsParam.MaxParamAllowedVoltage);
	config->set_tallowedmax(status.stFrameBmsParam.MaxtAllowedTemp);
	config->set_startsoc(status.stFrameBmsParam.ParamSOC);
	config->set_vcurrent(status.stFrameBmsParam.BatteryVoltage);
	config->set_vcoutputmax(status.stFrameBmsParam.MaxOutputVoltage);
	config->set_vcoutputmin(status.stFrameBmsParam.MinOutputVoltage);
	config->set_icoutputmax(status.stFrameBmsParam.MaxOutputCurrent);
	config->set_icoutputmin(status.stFrameBmsParam.MinOutputCurrent);
	//BMS中止充电阶段
	finish->set_bmsstopreason(status.stFrameBmsChargeTerm.BMSStopReason);
	finish->set_bmsfaultreason(status.stFrameBmsChargeTerm.BMSFaultReason);
	finish->set_bmserrorreason(status.stFrameBmsChargeTerm.BMSErrorReason);
	finish->set_chargerstopreason(status.stFrameBmsChargeTerm.ChargerStopReason);
	finish->set_chargerfaultreason(status.stFrameBmsChargeTerm.ChargerFaultReason);
	finish->set_chargererrorreason(status.stFrameBmsChargeTerm.ChargerErrorReason);
	finish->set_endsoc(status.stFrameBmsChargeTerm.ChargeEndSOC);
	finish->set_vminindividal(status.stFrameBmsChargeTerm.MinSingleVoltage);
	finish->set_vmaxindividal(status.stFrameBmsChargeTerm.MaxSingleVoltage);
	finish->set_temperaturemin(status.stFrameBmsChargeTerm.MinTemp);
	finish->set_temperaturemax(status.stFrameBmsChargeTerm.MaxTemp);
	finish->set_bmseframe(status.stFrameBmsChargeTerm.BMSErrorFrame);
	finish->set_chargereframe(status.stFrameBmsChargeTerm.ChargerErrorFrame);

	sendFrame(CMD_BMS_PARAM_NOTIFY, &req);
}

bool ChargeProtocol::syncAccount(int accountType)
{
	AccountSyncReq req;
    DateTime *synctime;
    DateTime *sendtime;
	QString strSql, strTime;
    struct db_result_st result;

    req.set_type((SyncAccountType)accountType);  //设置需要同步的账户类型 1-卡号；2-车辆信息
    req.set_size(MAX_SYNC_ACCOUNT);   //设置每页的记录条数

    synctime = req.mutable_synctime();   //同步时间
    sendtime = req.mutable_sendtime();  // 发送时间
	
	strSql.sprintf("SELECT card_update, car_update FROM table_update_time;");
	if(ProtobufServer::server()->database()->DBSqlQuery(strSql.toAscii().data(), &result, DB_AUTHENTICATION) == 0){
		if(result.row > 0){
			if(accountType == 1){
				strTime = result.result[0];
			}else if(accountType == 2){
				strTime = result.result[1];
			}
		}
		ProtobufServer::server()->database()->DBQueryFree(&result);
	}
    if(!strTime.isEmpty())  //如果最后一次的记录时间不为空，将当前时间更新为最后一次的记录时间
        synctime->set_time(QDateTime::fromString(strTime,"yyyy-MM-dd HH:mm:ss").toMSecsSinceEpoch());
    sendtime->set_time(QDateTime::currentDateTime().toMSecsSinceEpoch());
    return sendFrame(CMD_SYNC_ACCOUNT, &req);
}

void ChargeProtocol::parseSyncAccount(char *data, int len)
{
	AccountSyncAns ans;
	CardInfo card;
	CarInfo car;
	QDateTime dt;
	QString strSql, strTime, strField;
	struct db_result_st result;

	ans.ParseFromArray(data, len);

    writeLog(QString().sprintf("%s {\n%s}", ans.GetDescriptor()->name().c_str(), ans.DebugString().c_str()), 2);

    dt = QDateTime::fromMSecsSinceEpoch(ans.synctime().time());
	strTime = dt.toString("yyyy-MM-dd HH:mm:ss");

    writeLog(QString().sprintf("本次同步的卡信息数量============%d",ans.card_size()),2);
    for(int i = 0; i < ans.card_size(); i++){
		card = ans.card(i);	
		if(card.isdel() == False){
			strSql.sprintf("INSERT INTO table_card_authentication (card_id, card_code, is_delete) VALUES ('%s', '%s', %d);",
					card.cardid().c_str(), card.cardcode().c_str(), card.isdel() == True ? 1 : 0);
		}else{
			strSql.sprintf("DELETE FROM table_card_authentication WHERE card_id = '%s';", card.cardid().c_str());
		}

		if(ProtobufServer::server()->database()->DBSqlExec(strSql.toAscii().data(), DB_AUTHENTICATION) != 0){
            writeLog(QString().sprintf("Sync card info failed! Sql = %s", strSql.toAscii().data()),2);
			return;
		}
	}

    writeLog(QString().sprintf("本次同步的车信息数量============%d",ans.car_size()),2);
    for(int i = 0; i < ans.car_size(); i++){
		car = ans.car(i);	
		if(car.isdel() == False){
            strSql.sprintf("INSERT INTO table_car_authentication (car_id, car_vin, priority, car_no, is_delete) VALUES ('%s', '%s', %d, '%s', %d);",
                           car.carid().c_str(), car.carvin().c_str(), car.pri(), car.plate().c_str(), car.isdel() == True ? 1 : 0);
		}else{
            strSql.sprintf("DELETE FROM table_car_authentication WHERE car_id = '%s';", car.carid().c_str());
		}

		if(ProtobufServer::server()->database()->DBSqlExec(strSql.toAscii().data(), DB_AUTHENTICATION) != 0){
            writeLog(QString().sprintf("Sync car info failed! Sql = %s", strSql.toAscii().data()),2);
			return;
        }
    }
	//每次回应都需记录时间，防止网络断开重复请求
    if(ans.synctime().time() != 0){
        if(ans.type() == 1){
            strField = "card_update";
        }
        else if(ans.type() == 2){
            strField = "car_update";
        }

        strSql = "SELECT id FROM table_update_time;";
        if(ProtobufServer::server()->database()->DBSqlQuery(strSql.toAscii().data(), &result, DB_AUTHENTICATION) != 0){
            writeLog("Sync query time failed",2);
            return;
        }

        if(result.row <= 0)
            strSql.sprintf("INSERT INTO table_update_time (%s) VALUES ('%s');", strField.toAscii().data(), strTime.toAscii().data());
        else
            strSql.sprintf("UPDATE table_update_time SET %s = '%s';", strField.toAscii().data(), strTime.toAscii().data());
        ProtobufServer::server()->database()->DBQueryFree(&result);
        if(ProtobufServer::server()->database()->DBSqlExec(strSql.toAscii().data(), DB_AUTHENTICATION) != 0){
            writeLog("Sync time failed",2);
            return;
        }
      }

    switch (ans.type()){
    case 1:   //卡号信息    卡信息未同步完成，继续申请卡信息
        if(ans.card_size() >= MAX_SYNC_ACCOUNT){
            syncAccount(1);
            return;
        }
        else{
            //卡信息同步完成，开始申请车辆信息
            syncAccount(2);
        }
        break;
    case 2:   //车辆信息   车辆信息未同步完成，继续申请车辆信息
        if(ans.car_size() >= MAX_SYNC_ACCOUNT){
            syncAccount(2);
            return;
        }
        break;
    default:
        break;
    }
}

bool ChargeProtocol::sendFrame(char frameType, PBMessage *message)
{
	if(!Protocol::sendFrame(frameType, message))
		return false;

	if(m_mapProtoName.contains(frameType)){
        writeLog(QString().sprintf("[send][%x] [%s]",frameType,m_mapProtoName[frameType].toAscii().data()),2);
	}
    writeLog(QString().sprintf("%s {\n%s}", message->GetDescriptor()->name().c_str(), message->DebugString().c_str()), 2);

	return true;
}

bool ChargeProtocol::parse(short type, char *buff, int len)
{
	infoType = AddrType_Unknown;
	info.clear();

    if(m_mapProtoName.contains(type)){
        writeLog(QString().sprintf("[recv][%x] [%s]",type,m_mapProtoName[type].toAscii().data()),2);
    }

	switch(type){
		case CMD_ACK(CMD_HEART):
			parseHeartAns(buff, len);
			break;
		case CMD_ACK(CMD_LOGIN):
			parseLoginAns(buff, len);
			break;
		case CMD_ACK(CMD_LOGOUT):
			break;
		case CMD_ACK(CMD_UPLOAD_BILL):
			parseUploadBillAns(buff, len);
			break;
		case CMD_ACK(CMD_APPLY_ACCOUNT):
			parseAccountAns(buff, len);
			break;
		case CMD_POWER_CURVE:
			parsePowerCurve(buff, len);
			break;
		case CMD_ACK(CMD_APPLY_START_CHARGE):
			parseApplyStartChargeAns(buff, len);
			break;
		case CMD_ACK(CMD_APPLY_STOP_CHARGE):
			parseApplyStopChargeAns(buff, len);
			break;
		case CMD_ACK(CMD_SYNC_ACCOUNT):
			parseSyncAccount(buff, len);
			break;
		case CMD_ACK(CMD_GUN_GROUP_NOTIFY):
			parseGunGroupAns(buff, len);
			break;
		case CMD_HEART:
			sendHeartAns(buff, len);
			break;
		case CMD_START_CHARGE:
			parseStartCharge(buff, len);
			break;
		case CMD_STOP_CHARGE:
			parseStopCharge(buff, len);
			break;
		case CMD_PAUSE_CHARGE:
			parsePauseCharge(buff, len);
			break;
		case CMD_RESUME_CHARGE:
			parseResumeCharge(buff, len);
			break;
		case CMD_CHARGER_CALL:
			chargerStateCall(buff, len);
			break;
		case CMD_MEASURE_CALL:
			measureCall(buff, len);
			break;
		case CMD_SIGNAL_CALL:
			signalCall(buff, len);
			break;
		case CMD_BMS_CHARGE_CALL:
			bmsChargeCall(buff, len);
			break;
		case CMD_UNCONFIRMED_BILL_CALL:
            unconfirmedBillCall(buff, len);
			break;
		case CMD_CODE_BILL_CALL:
			codeBillCall(buff, len);
			break;
		case CMD_GUN_GROUP_SET:
			setGunGroup(buff, len);
			break;
		case CMD_EMERGENCY:
			parseEmergency(buff, len);
			break;
		case CMD_RM_WHITE:
			parseRmWhite(buff, len);
			break;
		default:
			break;
	}

	emit parsed(info, infoType);

	return true;
}

void ChargeProtocol::setSystemTime(qint64 time)
{
	QString strCmd;
	QDateTime dt;

	dt = QDateTime::fromMSecsSinceEpoch(time);
	strCmd.sprintf("date -s \"%s\"" , dt.toString("yyyy-MM-dd HH:mm:ss").toAscii().data());
	system(strCmd.toAscii().data()); 
	strCmd.sprintf("hwclock -w -u");
	system(strCmd.toAscii().data()); 
}

void ChargeProtocol::setGunGroup(char *data, int len)
{
	SetGunGroupInfoReq req;		
	CommAns ans;
	GunGroupType group;
	QByteArray ar;
	int addr;

	req.ParseFromArray(data, len);
    writeLog(QString().sprintf("%s {\n%s}", req.GetDescriptor()->name().c_str(), req.DebugString().c_str()), 2);

	addr = Addr_Group_ChargeGun;

	for(int i = 0; i < req.gungroupinfo_size(); i++){
		group = req.gungroupinfo(i);	
		if(i > 0)
			ar.append((char *)(addr + i), 3);

		ar.append((char)group.gun1canindex());
		ar.append((char)group.gun2canindex());
		ar.append((char)group.gun3canindex());
		ar.append((char)group.gun4canindex());
		ar.append((char)group.gun5canindex());
		ar.append((char)group.gun6canindex());
		ar.append((char)group.gun7canindex());
	}

	info[Addr_Group_ChargeGun] = ar;

	if(saveGunGroup(info)){
		infoType = AddrType_ChargeGunGroup_Info;
		ans.set_confirm(True);
	}

	sendFrame(CMD_ACK(CMD_GUN_GROUP_SET), &ans);
}

bool ChargeProtocol::saveGunGroup(InfoMap map)
{
    QByteArray ar;
    QString strSql;
    bool success = false;
    int iGroup = 1; 

    if(map.count() <= 0)
		return false;

    if(ProtobufServer::server()->database()->DBSqlExec((char *)"BEGIN", DB_PARAM) != 0)
        return false;

    strSql = "DELETE FROM chargegun_group_table;";
    if(ProtobufServer::server()->database()->DBSqlExec(strSql.toAscii().data(), DB_PARAM) != 0)
        goto rollback;

    ar = map[Addr_Group_ChargeGun];
    for(int i = 0; i < ar.length(); i = i + 10){ 
        strSql.sprintf("INSERT INTO chargegun_group_table (group_id, gun1, \
                gun2, gun3, gun4, gun5,gun6,gun7) VALUES(%d, %d, %d, %d, %d, %d,%d,%d);",
                iGroup++, ar.at(i), ar.at(i + 1), ar.at(i + 2), ar.at(i + 3), ar.at(i + 4),ar.at(i + 5),ar.at(i + 6)); 

        if(ProtobufServer::server()->database()->DBSqlExec(strSql.toAscii().data(), DB_PARAM) != 0)
            goto rollback;
    }    

    if(ProtobufServer::server()->database()->DBSqlExec((char *)"COMMIT", DB_PARAM) != 0)
        goto rollback;

    success = true;

rollback:
    if(!success){
        ProtobufServer::server()->database()->DBSqlExec((char *)"ROLLBACK", DB_PARAM);
		return false;
	}

	return true;
}

void ChargeProtocol::gunGroupNotify(InfoMap map)
{
	GunGroupInfoReq req;
	DateTime *time;

	if(!map.contains(Addr_ChargeGunType))
		return;
	if(!map.contains(Addr_ChargeGun_Master))
		return;

	time = req.mutable_sendtime();
	time->set_time(QDateTime::currentDateTime().toMSecsSinceEpoch());

	req.set_gunnumber(map[Addr_ChargeGunType].at(0));
	req.set_mastergun(map[Addr_ChargeGun_Master].at(0));

	if(map.contains(Addr_ChargeGun_Slave1))
		req.set_slavegun1(map[Addr_ChargeGun_Slave1].at(0));
	if(map.contains(Addr_ChargeGun_Slave2))
		req.set_slavegun2(map[Addr_ChargeGun_Slave2].at(0));
	if(map.contains(Addr_ChargeGun_Slave3))
		req.set_slavegun3(map[Addr_ChargeGun_Slave3].at(0));
	if(map.contains(Addr_ChargeGun_Slave4))
		req.set_slavegun4(map[Addr_ChargeGun_Slave4].at(0));
	if(map.contains(Addr_ChargeGun_Slave5))
		req.set_slavegun5(map[Addr_ChargeGun_Slave5].at(0));
	if(map.contains(Addr_ChargeGun_Slave6))
		req.set_slavegun6(map[Addr_ChargeGun_Slave6].at(0));

	sendFrame(CMD_GUN_GROUP_NOTIFY, &req);
}

void ChargeProtocol::parseGunGroupAns(char *data, int len)
{
	GunGroupInfoAns ans;	

	ans.ParseFromArray(data, len);

	infoType = AddrType_Response_Result;

	info[Addr_ChargeGunType] = QByteArray(1, (char)ans.gunnumber());
	info[Addr_ChargeGun_Master] = QByteArray(1, (char)ans.mastergun());
	info[Addr_ChargeGun_Slave1] = QByteArray(1, (char)ans.slavegun1());
	info[Addr_ChargeGun_Slave2] = QByteArray(1, (char)ans.slavegun2());
	info[Addr_ChargeGun_Slave3] = QByteArray(1, (char)ans.slavegun3());
	info[Addr_ChargeGun_Slave4] = QByteArray(1, (char)ans.slavegun4());
	info[Addr_ChargeGun_Slave5] = QByteArray(1, (char)ans.slavegun5());
	info[Addr_ChargeGun_Slave6] = QByteArray(1, (char)ans.slavegun6());
	info[Addr_ChargeGunType_Reault] = QByteArray(1, ans.finished() == True ? 0xFF : 0x01);
}

void ChargeProtocol::parseEmergency(char *data, int len)
{
	TurnToEmergencyCmdReq req;
    TurnToEmergencyCmdAns ans;

    bool bResult = false;

	req.ParseFromArray(data, len);
    writeLog(QString().sprintf("%s {\n%s}", req.GetDescriptor()->name().c_str(), req.DebugString().c_str()), 2);

	if(req.setflag() == 255){
        bResult = ((ChargeServer *)ProtobufServer::server())->ProtoEmergency(1);
	}else if(req.setflag() == 1){
        bResult = ((ChargeServer *)ProtobufServer::server())->ProtoEmergency(0);
	}

    ans.set_confirm(bResult ? True : False);
    ans.set_setflag(req.setflag());

    sendFrame(CMD_ACK(CMD_EMERGENCY), &ans);
}

void ChargeProtocol::parseRmWhite(char *data, int len)
{
	ClearAccountCmdReq req;
	BoolEnum success = True;

	req.ParseFromArray(data, len);

	if(req.clearflag() != 1)
		return;

	QString strSql;

	do{
		strSql.sprintf("DELETE FROM table_car_authentication;");
		if(ProtobufServer::server()->database()->DBSqlExec(strSql.toAscii().data(), DB_AUTHENTICATION) != 0){
			success = False;
			break;
		}
		strSql.sprintf("DELETE FROM table_card_authentication;");
		if(ProtobufServer::server()->database()->DBSqlExec(strSql.toAscii().data(), DB_AUTHENTICATION) != 0){
			success = False;
			break;
		}
		strSql.sprintf("DELETE FROM table_update_time;");
		if(ProtobufServer::server()->database()->DBSqlExec(strSql.toAscii().data(), DB_AUTHENTICATION) != 0){
			success = False;
			break;
		}
	}while(false);

	ClearAccountCmdAns ans;
	ans.set_confirm(success);

	sendFrame(CMD_ACK(CMD_RM_WHITE), &ans);

	emit syncAccount();
}
