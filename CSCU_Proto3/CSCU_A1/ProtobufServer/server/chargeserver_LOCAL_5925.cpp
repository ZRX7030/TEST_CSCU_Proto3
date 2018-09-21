#include "chargeserver.h"
#include "commfunc.h"
#include "chargeprotocol.h"
#include "tcpnet.h"
#include <QTimerEvent>
#include <math.h>
#include <QByteArray>

ChargeServer::ChargeServer(uchar cServerType, void* pDepend[]) : ProtobufServer(cServerType, pDepend)
{
	writeLog(QString().sprintf("Terminal init ac=%d tac=%d dc=%d", acNum, tacNum, dcNum));
	writeLog(QString().sprintf("Host=%s Port=%d Encrypt=%d StationNo=%s Key=%s",
				chargeAddr.c_str(), chargePort, chargeEncrypt,
				stationNo.c_str(), cscuKey.c_str()));

	memset(&configEmergency, 0, sizeof(EmergencyConfig));
	cycleTime = 0;
	localEmergency = false;
	remoteEmergency = false;
	setEmergencyState();
}

ChargeServer::~ChargeServer()
{

}

/*
 * 动态库接口函数，向BUS注册模块
 * pBus		输入 BUS模块指针
 * 返回值 	0表示无错误，-1表示有错误
 */
int ChargeServer::RegistModule(CBus* pBus)
{
	QList<int> list;

	if(pBus){
		list.append(AddrType_CmdCtrl_AckResult);				//充电响应结果
		//list.append(AddrType_CmdCtrl_ExecResult);				//充电执行结果
		list.append(AddrType_TermSignal);						//突发遥信
		list.append(AddrType_LogicState);						//突发逻辑工作状态
		list.append(AddrType_ChargeServicApplyAccountInfo);		//刷卡申请帐户信息
		list.append(AddrType_OutApplyStartChargeByChargeServic);//刷卡申请开始充电
		list.append(AddrType_OutApplyStopChargeByChargeServic);	//刷卡申请结束充电
		list.append(AddrType_VinApplyStartCharge);				//VIN申请开始充电
		list.append(AddrType_VinApplyStopCharge);				//VIN申请结束充电
		list.append(AddrType_CarLicenceApplyStartCharge);		//车牌号申请开始充电
		list.append(AddrType_CarLicenceApplyStopCharge);		//车牌号申请结束充电
		list.append(AddrType_ChargeOrder_State);				//订单状态
		list.append(AddrType_CheckChargeManner_Success);		//突发多枪信息

		return pBus->RegistDev(this, list, ID_PB_CHARGE_SERVER);
	}
	return -1;
}

/*
 * 动态库接口函数，创建模块实例
 * pDepends 输入 公共模块指针列表
 * 返回值 	成功返回模块实例，失败返回NULL
 */
CModuleIO* CreateDevInstance(int argc, void *pDepends[])
{
	ProtobufServer::_server = new ChargeServer((uchar)argc, pDepends);

	return (CModuleIO*)ProtobufServer::_server;
}

/*
 * 动态库接口函数，销毁模块实例
 * pModule 输入 模块实例
 * 返回值  无	
 */
void DestroyDevInstance(CModuleIO* pModule)
{
	if(pModule){
		delete pModule;
	}
}

/*
 * 线程启动槽函数
 *
 * 参数		无
 * 返回值	无
 */
void ChargeServer::slot_onThreadRun()
{	
	net = new TcpNet();
	proto = new Charge::ChargeProtocol(net);

	connect(net, SIGNAL(connected()), proto, SLOT(onNetConnected()));
	connect(net, SIGNAL(disconnect()), proto, SLOT(onNetDisconnected()));
	connect(net, SIGNAL(error(int)), proto, SLOT(onNetError(int)));
	connect(net, SIGNAL(readyRead()), proto, SLOT(onNetReceive()));

	connect(proto, SIGNAL(active(int)), this, SLOT(onProtoActive(int)));
	connect(proto, SIGNAL(emergency(int)), this, SLOT(onProtoEmergency(int)));
	connect(proto, SIGNAL(syncAccount()), this, SLOT(onProtoSyncAccount()));
	connect(proto, SIGNAL(live()), this, SLOT(onProtoLive()));
	connect(proto, SIGNAL(parsed(InfoMap, InfoAddrType)), this, SLOT(onProtoParsed(InfoMap, InfoAddrType)));

    connect(proto, SIGNAL(sendMessage(QString)), this, SLOT(onProtosendMessage(QString)));   //add by zrx
	stateTimer = startTimer(1000);
}

void ChargeServer::slotFromBus(InfoMap mapInfo, InfoAddrType type)
{
    if(!serverLogged || localEmergency || remoteEmergency){
		localAuth(mapInfo, type);
	}else{
		proto->command(mapInfo, type);
	}
}

void ChargeServer::onProtoActive(int logged)
{
    ProtobufServer::onProtoActive(logged);

    if(logged){
        offlineTime = 0;
        if(selectHistoryBill() == false){//如果没有历史订单,应急退出
            localEmergency = false;
            setEmergencyState();
            saveEmergencyState();
        }

        writeLog(QString().sprintf("Local emergency exit"), 1);
    }

    setNetState(logged);
}

void ChargeServer::onProtosendMessage(QString qstrMessage)  //消息内容
{
    qstrRecvProtoMessage = qstrMessage;
}

void ChargeServer::onProtoEmergency(int flag)
{
    if(flag != 3){  //作判断config里的localEmergency  flag=1 =0
        if(!configEmergency.emergency_enable)
            return;
        remoteEmergency = flag;
        localEmergency = false;

        setEmergencyState();
        saveEmergencyState();
    }
    else if(flag == 3){
        localEmergency = false;
        setEmergencyState();
        saveEmergencyState();
    }
}

void ChargeServer::onProtoSyncAccount()
{
	InfoMap map;
	InfoAddrType type = AddrType_Sync_Account;
	proto->command(map, type);
}

void ChargeServer::timerEvent(QTimerEvent *event)
{
	ProtobufServer::timerEvent(event);

	if(event->timerId() == stateTimer){
		emergencyCheck();

		if(serverLogged){
			if(cycleTime % 3600 == 0){//周期同步账户信息
//				syncWhite();
			}

			if(cycleTime > 3600)
				cycleTime = 0;

			cycleTime++;
		}else{
			cycleTime = 0;
		}
	}
    else if(event->timerId() == billAckTimer){  //订单上传超时20s处理
        billAckTimerCpunt ++;
        InfoMap map;
        InfoAddrType type;
        QByteArray qByteArray;
        if(billAckTimerCpunt > 2){   //重发三次,返回失败
            writeLog(QString().sprintf("订单上传超时20s处理,重发次数count===%d.超时3次,生成告警",billAckTimerCpunt), 2);
            billAckTimerCpunt = 0;
            //定时器关掉
            if(billAckTimer > 0){
                killTimer(billAckTimer);  //关闭定时器
            }
            int iCSCU_AlarmCode = 501;
            map = recvBillMap;
            qByteArray.clear();
            qByteArray.append((char *)&iCSCU_AlarmCode,4);
            map.insert(Addr_CSCU_AlarmCode,qByteArray);

            qByteArray.clear();
            qByteArray.append(qstrRecvProtoMessage);
            map.insert(Addr_CSCU_AlarmContent,qByteArray);

            makeAlarm(map);  //生成告警
            return;
        }
        writeLog(QString().sprintf("订单上传超时20s处理,重发次数count===%d",billAckTimerCpunt), 2);

        type = AddrType_ChargeOrder_State;
        map = recvBillMap;

        proto->command(map, type);
    }
}

void ChargeServer::setNetState(bool state)
{
	RealStatusMeterData& real = _cache->GetUpdateRealStatusMeter();
	real.realData.connectStatus = state;
	_cache->FreeUpdateRealStatusMeter();
}

void ChargeServer::getNetSetting(string &host, ushort &port, int &encrypt)
{
	host = chargeAddr;
	port = chargePort;
	encrypt = chargeEncrypt;
}

void ChargeServer::writeLog(QString strLog, int iLevel)
{
	switch (iLevel) {
		case 1:
			_log->getLogPoint(LOG_PB_CHARGE_SERVER)->debug(strLog);
			break;
		case 2:
			_log->getLogPoint(LOG_PB_CHARGE_SERVER)->info(strLog);
			break;
		case 3:
			_log->getLogPoint(LOG_PB_CHARGE_SERVER)->warn(strLog);
			break;
		case 4:
			_log->getLogPoint(LOG_PB_CHARGE_SERVER)->error(strLog);
			break;
		default:
			break;
	}
}

bool ChargeServer::localAuth(InfoMap &map, InfoAddrType &type)
{
	QString id = "";
	int idType = 0, cmd = 0, ret = 255;
	uchar canAddr;

	//无应急状态不进入本地鉴权流程
	if(!localEmergency && !remoteEmergency)
		return false;

	do{
		if(!map.contains(Addr_CanID_Comm))
			return false;

		canAddr = map[Addr_CanID_Comm].at(0);

		if(!configEmergency.emergency_enable)
			return false;

		if(!parseApplyInfo(map, type, id, idType, cmd))
			return false;

		ret = authId(id, idType, configEmergency);
		if(ret != 255)
			break;

		if(cmd == CHARGE_CMD_TYPE_START_CHARGE_NOW){
			ret = timeCheck(configEmergency);
			if(ret != 255)
				break;
			ret = billCheck(configEmergency);
			if(ret != 255)
				break;
		}

	}while(false);


    InfoMap info;
	InfoAddrType infoType;
	int addr;

	info[Addr_CanID_Comm] = map[Addr_CanID_Comm];

	switch(idType){
		case 1:
			info[Addr_CardAccount] = map[Addr_CardAccount];
			if(cmd == CHARGE_CMD_TYPE_START_CHARGE_NOW){
				info.insert(Addr_CardApplyCharge_Result, QByteArray(1, ret));
				infoType = AddrType_OutApplyStartChargeByChargeServic_Result;
			}else{
				//info.insert(Addr_CardStopCharge_Result, QByteArray(1, ret));
				//infoType = AddrType_OutApplyStopChargeByChargeServic_Result;
			}
			break;
		case 2:
			info.insert(Addr_VINApplyStartChargeType_Result, QByteArray(1, ret));
            info[Addr_BatteryVIN_BMS] = map[Addr_BatteryVIN_BMS];
            if(cmd == CHARGE_CMD_TYPE_START_CHARGE_NOW)
				infoType = AddrType_VinApplyStartCharge_Result;
			else
				infoType = AddrType_VinApplyStopCharge_Result;
			break;
		case 3:
			info.insert(Addr_CarLicenseApplyStartChargeType_Result, QByteArray(1, ret));
			if(cmd == CHARGE_CMD_TYPE_START_CHARGE_NOW)
				infoType = AddrType_CarLicenceApplyStartCharge_Result;
			else
				infoType = AddrType_CarLicenceApplyStopCharge_Result;

			break;
		default:
			return false;
	}

	emit sigToBus(info, infoType);

	if(cmd == CHARGE_CMD_TYPE_START_CHARGE_NOW && ret == 255){
		info.clear();
		infoType = AddrType_CmdCtrl_Apply;

		info[Addr_CanID_Comm] = map[Addr_CanID_Comm];
		info[Addr_CardAccount] = map[Addr_CardAccount];
		info[Addr_ChargeCmd_Ctrl] = QByteArray(1, cmd);
		info[Addr_Local_Emergency] = QByteArray(1, 0xFF);
		//info[Addr_Remote_Emergency] = QByteArray(1, 0xFF);

		//DHT 功率曲线测试代码

//		string billCode = "2018050810001";
//		info[Addr_Bill_Code] = QByteArray().append(billCode.c_str(), billCode.length());


		emit sigToBus(info, infoType);
	}

	return true;
}

bool ChargeServer::parseApplyInfo(InfoMap map, InfoAddrType type, QString &id, int &idType, int &cmd)
{
	InfoMap info;
	uint balance = 0;

	switch(type){
		case AddrType_ChargeServicApplyAccountInfo://刷卡申请帐户详情
			info[Addr_CanID_Comm] = map[Addr_CanID_Comm];
			info[Addr_CardAccount] = map[Addr_CardAccount];
			info[Addr_Account_Balance] = QByteArray((char *)&balance, sizeof(balance));
			emit sigToBus(info, AddrType_ChargeServicApplyAccountInfo_Result);
			break;
		case AddrType_OutApplyStartChargeByChargeServic://刷卡申请开始充电
		case AddrType_OutApplyStopChargeByChargeServic://刷卡申请结束充电
			if(map.contains(Addr_CardAccount)){
				idType = 1;
				if(type == AddrType_OutApplyStopChargeByChargeServic)
					cmd = CHARGE_CMD_TYPE_STOP_CHARGE;
				else
					cmd = CHARGE_CMD_TYPE_START_CHARGE_NOW;
				id = map[Addr_CardAccount].toHex().data();
				break;
			}
			if(map.contains(Addr_ScanCode_customerID)){
				idType = 4;
				if(type == AddrType_OutApplyStopChargeByChargeServic)
					cmd = CHARGE_CMD_TYPE_STOP_CHARGE;
				else
					cmd = CHARGE_CMD_TYPE_START_CHARGE_NOW;
				id = map[Addr_ScanCode_customerID].data();
				break;
			}
			break;
		case AddrType_VinApplyStartCharge://VIN申请开始充电
		case AddrType_VinApplyStopCharge://VIN申请结束充电
			idType = 2;
			if(!map.contains(Addr_VINApplyStartChargeType))
				return false;
			if(map[Addr_VINApplyStartChargeType].at(0) != 1)
				return false;
			if(map.contains(Addr_BatteryVIN_BMS)){
				id = map[Addr_BatteryVIN_BMS].data();
				if(type == AddrType_VinApplyStopCharge)
					cmd = CHARGE_CMD_TYPE_STOP_CHARGE;
				else
					cmd = CHARGE_CMD_TYPE_START_CHARGE_NOW;
			}
			break;
		case AddrType_CarLicenceApplyStartCharge://车牌号申请开始充电
		case AddrType_CarLicenceApplyStopCharge://车牌号申请结束充电
			idType = 3;
			if(map.contains(Addr_CarLicense) && QueryCarLisenceName(map[Addr_CarLicense].data(), id)){
				if(type == AddrType_CarLicenceApplyStopCharge)
					cmd = CHARGE_CMD_TYPE_STOP_CHARGE;
				else
					cmd = CHARGE_CMD_TYPE_START_CHARGE_NOW;
			}else{
				id = "";	
			}
			break;
		case AddrType_CheckChargeManner_Success:
			map[Addr_ChargeGunType_Reault] = QByteArray(1, 0xFF);
			emit sigToBus(map, AddrType_Response_Result);
			return false;
		default:
			return false; 
	}

	return true;
}

int ChargeServer::authId(QString id, int idType, EmergencyConfig config)
{
	struct db_result_st result;
	QString strSql = "";
	int ret = 255;

	switch(idType){
		case 1://卡号
			if(configEmergency.card_authenticate)
        		strSql = QString("SELECT id FROM table_card_authentication WHERE card_code = '%1' AND is_delete = 0;").arg(id);
			break;
	    case 2://VIN
			if(configEmergency.vin_authenticate)
        		strSql = QString("SELECT id FROM table_car_authentication WHERE car_vin = '%1' AND is_delete = 0;").arg(id);
        	break;
    	case 3://车牌号
			if(configEmergency.car_authenticate)
        		strSql = QString("SELECT id FROM table_car_authentication WHERE car_no = '%1' AND is_delete = 0;").arg(id);
			break;
		case 4://反扫码
			break;
		default:
			return false;
	}

	if(!strSql.isEmpty()){
		if(_database->DBSqlQuery(strSql.toAscii().data(), &result, DB_AUTHENTICATION) != 0)
			return false;

		ret = result.row > 0 ? 255 : 253;
		_database->DBQueryFree(&result);
	}

	return ret;
}

int ChargeServer::timeCheck(EmergencyConfig config)
{
	struct db_result_st result;
	QString strSql, strTime;
    int iTime;
	int ret = 255;

    strSql.sprintf("SELECT EmergencyTime FROM emergency_time;");
    if(_database->DBSqlQuery(strSql.toAscii().data(), &result, DB_PROCESS_RECORD) != 0)
        return ret;

    if(result.row <= 0){
        _database->DBQueryFree(&result);
        return ret;
    }

    strTime = result.result[0];
    _database->DBQueryFree(&result);

    QDateTime dt = QDateTime::fromString(QString(strTime),"yyyy-MM-dd HH:mm:ss");
    QDateTime current = QDateTime::currentDateTime();

    iTime = abs(dt.secsTo(current));
    ret = iTime > config.duration * 3600 ? 249 : 255;
	
	return ret;
}

int ChargeServer::billCheck(EmergencyConfig config)
{
	struct db_result_st result;
	QString strSql;
	int ret = 255;

    strSql.sprintf("SELECT COUNT(id) FROM charge_order WHERE OrderType = %d AND OrderSync = 0 AND OrderStatus = %d;", ORDER_EMERGENCY, ORDER_STATUS_OK);
    if(_database->DBSqlQuery(strSql.toAscii().data(), &result, DB_PROCESS_RECORD) != 0)
        return ret;

    if(result.row <= 0 || !result.result[0]){
        _database->DBQueryFree(&result);
        return ret;
    }

    ret = atoi(result.result[0]) > config.order_count ? 248 : 255;
    _database->DBQueryFree(&result);

	return ret;
}

int ChargeServer::emergencyCheck()
{
	//已经进入本地应急不再计数
	if(serverState >= STATE_OFFLINE && serverState < STATE_HEART){
		offlineTime++;
		//writeLog(QString().sprintf("offline=%d", offlineTime), 2);

		//离网15分钟
		if(offlineTime == 15 * 60){
			InfoMap info;
			InfoAddrType infoType = AddrType_Offline_Time;
			info[Addr_Clear_Power_Curve] = QByteArray(1, 1);
			emit sigToBus(info, infoType);
			writeLog(QString().sprintf("Offline 15 minutes %d", offlineTime), 2);
		}

		//应急开关关闭不进行检测
		if(!configEmergency.emergency_enable) 
			return -1;

		//平台设置应急后不进行检测
		if(remoteEmergency)
			return -1;

		if(!localEmergency && offlineTime > configEmergency.check_time){
			localEmergency = true;

			setEmergencyState();
			saveEmergencyState();

			writeLog(QString().sprintf("Local emergency"), 1);
		}
	}

    static int powercurve = 0;
    powercurve++;
    if(powercurve % 120 == 0){
//        InfoMap info;
//        InfoAddrType infoType;
//        uchar canAddr = 181;
//        string billCode = "2018050810001";
//        int curveState = 30008;

//        info[Addr_CanID_Comm] = QByteArray(1, (uchar)canAddr);
//        info[Addr_Bill_Code] = QByteArray().append(billCode.c_str(), billCode.length());
//        info[Addr_Power_Curve_State] = QByteArray((char*)&curveState, sizeof(uint));
//        infoType = AddrType_Power_Curve;
//        emit sigToBus(info, infoType);
    }

}

void ChargeServer::dataCheckFinished()
{
	struct db_result_st result;
    QString strSql;

	if(!_setting->querySetting(&configEmergency, PARAM_EMERGENCY)){
		writeLog(QString().sprintf("Get Emergency Failed!"));
	}

	if(!configEmergency.emergency_enable)
		return;

    strSql = "SELECT EmergencyTime, RemoteEmergency, LocalEmergency FROM emergency_time;";
    if(_database->DBSqlQuery(strSql.toAscii().data(), &result, DB_PROCESS_RECORD) != 0)
        return;

    if(result.row > 0){
        remoteEmergency = atoi(result.result[1]);
        localEmergency = atoi(result.result[2]);

		setEmergencyState();
   	}

    _database->DBQueryFree(&result);
}

void ChargeServer::setEmergencyState()
{
	RealStatusMeterData& real = _cache->GetUpdateRealStatusMeter();
    real.realData.emergencyStatus = localEmergency || remoteEmergency;
    _cache->FreeUpdateRealStatusMeter();
}

void ChargeServer::saveEmergencyState()
{
	struct db_result_st result;
	QString strSql;
	int record = 0;

	strSql.sprintf("SELECT id FROM emergency_time;");
    if(_database->DBSqlQuery(strSql.toAscii().data(), &result, DB_PROCESS_RECORD) == 0){
		record = result.row;
		_database->DBQueryFree(&result);
	}

	if(localEmergency || remoteEmergency){
		if(record > 0){
			strSql.sprintf("UPDATE emergency_time SET RemoteEmergency = %d , LocalEmergency = %d;", remoteEmergency, localEmergency);
		}else{
			strSql.sprintf("INSERT INTO emergency_time (EmergencyTime, RemoteEmergency, LocalEmergency) VALUES ('%s', %d, %d);", 
					QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss").toAscii().data(), remoteEmergency, localEmergency);
		}	
	}else{
		strSql.sprintf("DELETE FROM emergency_time;");
	}

    _database->DBSqlExec(strSql.toAscii().data(), DB_PROCESS_RECORD);
}

void ChargeServer::syncWhite()
{
	InfoMap map;
	InfoAddrType type = AddrType_Sync_Account;
	proto->command(map, type);
}

bool ChargeServer::selectHistoryBill() //查询有无历史订单,有-true;无-false
{
    bool bResult = false;
    struct db_result_st result;
    QString strSql;
    strSql.sprintf("SELECT * FROM  charge_order WHERE OrderSync = 0;");
    if(ProtobufServer::server()->database()->DBSqlQuery(strSql.toAscii().data(),&result,DB_PROCESS_RECORD) == 0){
        if(result.row > 0){  //有历史订单
            bResult = true;
        }
        else{ //无历史订单
            bResult = false;
        }
    }
    ProtobufServer::server()->database()->DBQueryFree(&result);
    return bResult;
}
