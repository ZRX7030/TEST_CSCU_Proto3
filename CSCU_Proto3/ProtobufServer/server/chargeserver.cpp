#include "chargeserver.h"
#include "commfunc.h"
#include "chargeprotocol.h"
#include "tcpnet.h"
#include <QTimerEvent>
#include <math.h>
#include <QByteArray>
#include <QStringList>

ChargeServer::ChargeServer()
{
	_strLogName = "pb_chargeserver";

    writeLog(QString().sprintf("Terminal init ac=%d tac=%d dc=%d", acNum, tacNum, dcNum),2);
    writeLog(QString().sprintf("Encrypt=%d StationNo=%s Key=%s",
                chargeEncrypt,stationNo.c_str(), cscuKey.c_str()),2);

	memset(&configEmergency, 0, sizeof(EmergencyConfig));
	cycleTime = 0;
	localEmergency = false;
	remoteEmergency = false;
	setEmergencyState();
    bBillResendEnd = false;
}

ChargeServer::~ChargeServer()
{

}

/*
 * 动态库接口函数，向BUS注册模块
 * pBus		输入 BUS模块指针
 * 返回值 	0表示无错误，-1表示有错误
 */
int ChargeServer::RegistModule()
{
	QList<int> list;

	list.append(AddrType_CmdCtrl_AckResult);				//充电响应结果
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
    list.append(AddrType_Comm_Addr);                        //通信地址

	CBus::GetInstance()->RegistDev(this, list);
	return 0;
}

/*
 * 动态库接口函数，创建模块实例
 * pDepends 输入 公共模块指针列表
 * 返回值 	成功返回模块实例，失败返回NULL
 */
CModuleIO* CreateDevInstance()
{
	ProtobufServer::_server = new ChargeServer();

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
	connect(proto, SIGNAL(syncAccount()), this, SLOT(onProtoSyncAccount()));
	connect(proto, SIGNAL(live()), this, SLOT(onProtoLive()));
	connect(proto, SIGNAL(parsed(InfoMap, InfoAddrType)), this, SLOT(onProtoParsed(InfoMap, InfoAddrType)));

    connect(proto, SIGNAL(sendMessage(QString)), this, SLOT(onProtosendMessage(QString)));   //add by zrx
	stateTimer = startTimer(1000);
}

void ChargeServer::slotFromBus(InfoMap mapInfo, InfoAddrType type)
{
    ProtobufServer::slotFromBus(mapInfo,type);

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
        if(!remoteEmergency){
            offlineTime = 0;
        }

        if(selectHistoryBill() == false){//如果没有历史订单,应急退出
            localEmergency = false;
            setEmergencyState();
            saveEmergencyState();
            writeLog(QString().sprintf("Local emergency exit"), 2);
        }
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
            if(cycleTime % 3600 == 0){//周期同步账户信息 -原定3600
                syncWhite();    //同步白名单
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
            bBillResendEnd = true;

            //定时器关掉
            if(billAckTimer > 0){
                killTimer(billAckTimer);  //关闭定时器
                billAckTimer = -1;
            }
            int iCSCU_AlarmCode = BILL_ALARM_CONFIRM;
            map = recvBillMap;
            qByteArray.clear();
            qByteArray.append((char *)&iCSCU_AlarmCode,4);
            map.insert(Addr_CSCU_AlarmCode, qByteArray);

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
			_log->getLogPoint(_strLogName)->debug(strLog);
			break;
		case 2:
			_log->getLogPoint(_strLogName)->info(strLog);
			break;
		case 3:
			_log->getLogPoint(_strLogName)->warn(strLog);
			break;
		case 4:
			_log->getLogPoint(_strLogName)->error(strLog);
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
    if(!localEmergency && !remoteEmergency){
        writeLog(QString().sprintf("无应急状态不进入本地鉴权流程"),2);
		return false;
    }

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
                writeLog(QString().sprintf("Card ApplyStartChargeByChargeServic_Result: %d",ret),2);
			}else{
				//info.insert(Addr_CardStopCharge_Result, QByteArray(1, ret));
                //infoType = AddrType_OutApplyStopChargeByChargeServic_Result;
			}
			break;
		case 2:
			info.insert(Addr_VINApplyStartChargeType_Result, QByteArray(1, ret));
            info[Addr_BatteryVIN_BMS] = map[Addr_BatteryVIN_BMS];
            if(cmd == CHARGE_CMD_TYPE_START_CHARGE_NOW){
				infoType = AddrType_VinApplyStartCharge_Result;
                writeLog(QString().sprintf("VIN ApplyStartCharge_Result: %d",ret),2);
            }
            else{
				infoType = AddrType_VinApplyStopCharge_Result;
                writeLog(QString().sprintf("VIN ApplyStopCharge_Result: %d",ret),2);
            }
			break;
		case 3:
			info.insert(Addr_CarLicenseApplyStartChargeType_Result, QByteArray(1, ret));
            info[Addr_CarLicense] = map[Addr_CarLicense];
            if(cmd == CHARGE_CMD_TYPE_START_CHARGE_NOW){
				infoType = AddrType_CarLicenceApplyStartCharge_Result;
                writeLog(QString().sprintf("CarLicence ApplyStartCharge_Result: %d",ret),2);
            }
            else{
				infoType = AddrType_CarLicenceApplyStopCharge_Result;
                writeLog(QString().sprintf("CarLicence ApplyStopCharge_Result: %d",ret),2);
            }

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
        //充电类型
        info.insert(Addr_Energy_ChargeType, QByteArray(1,0x01));

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
        {
            idType = 3;
            QString qstr;
            qstr = map[Addr_CarLicense].data();
//			if(map.contains(Addr_CarLicense) && QueryCarLisenceName(map[Addr_CarLicense].data(), id)){
            if(map.contains(Addr_CarLicense)){
                id = map[Addr_CarLicense].data();
                if(type == AddrType_CarLicenceApplyStopCharge)
					cmd = CHARGE_CMD_TYPE_STOP_CHARGE;
				else
					cmd = CHARGE_CMD_TYPE_START_CHARGE_NOW;
            }
            else{
				id = "";	
			}
			break;
        }
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
            if(configEmergency.card_authenticate){
                writeLog(QString().sprintf("card_code=====%s",id.toAscii().data()),2);
        		strSql = QString("SELECT id FROM table_card_authentication WHERE card_code = '%1' AND is_delete = 0;").arg(id);
            }
            break;
	    case 2://VIN
            if(configEmergency.vin_authenticate){
                writeLog(QString().sprintf("car_vin=====%s",id.toAscii().data()),2);
        		strSql = QString("SELECT id FROM table_car_authentication WHERE car_vin = '%1' AND is_delete = 0;").arg(id);
            }
        	break;
    	case 3://车牌号
            if(configEmergency.car_authenticate){
                writeLog(QString().sprintf("car_no=====%s",id.toAscii().data()),2);
        		strSql = QString("SELECT id FROM table_car_authentication WHERE car_no = '%1' AND is_delete = 0;").arg(id);
            }
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

    strSql.sprintf("SELECT COUNT(id) FROM charge_order WHERE OrderType = %d AND OrderSync < 2 AND (OrderStatus = %d OR OrderStatus = %d);",\
                   ORDER_EMERGENCY, ORDER_STATUS_OK,ORDER_STATUS_ING);
    if(_database->DBSqlQuery(strSql.toAscii().data(), &result, DB_PROCESS_RECORD) != 0)
        return ret;

    if(result.row <= 0 || !result.result[0]){
        _database->DBQueryFree(&result);
        return ret;
    }

    ret = atoi(result.result[0]) >= config.order_count ? 248 : 255;
    _database->DBQueryFree(&result);

	return ret;
}

int ChargeServer::emergencyCheck()
{
	//已经进入本地应急不再计数
    if((serverState >= STATE_GETADDRESS && serverState < STATE_HEART) || remoteEmergency){
		offlineTime++;
		//writeLog(QString().sprintf("offline=%d", offlineTime), 2);

        //离网15分钟-
        if(offlineTime % (15 * 60) == 0){
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

            writeLog(QString().sprintf("Local emergency"), 2);
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
        writeLog(QString().sprintf("Get Emergency Failed!"),2);
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

bool ChargeServer::saveEmergencyState()
{
	struct db_result_st result;
	QString strSql;
	int record = 0;

	strSql.sprintf("SELECT id FROM emergency_time;");
    if(_database->DBSqlQuery(strSql.toAscii().data(), &result, DB_PROCESS_RECORD) == 0){
		record = result.row;
		_database->DBQueryFree(&result);
	}
    else
        return false;

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

    if(_database->DBSqlExec(strSql.toAscii().data(), DB_PROCESS_RECORD) != 0)
        return false;

    return true;
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
    strSql.sprintf("SELECT * FROM  charge_order WHERE OrderSync < 2  AND OrderStatus <> 0 AND \
                   ((OrderStatus <> %d AND OrderType = %d) OR (OrderType = %d));",\
                   ORDER_STATUS_ING,ORDER_NORMAL, ORDER_EMERGENCY);
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

bool ChargeServer::ProtoEmergency(int flag)
{
    if(flag != 3){  //作判断config里的localEmergency  flag=1 =0
        if(!configEmergency.emergency_enable)
            return false;
        remoteEmergency = flag;
        localEmergency = false;

        setEmergencyState();
        if(!saveEmergencyState())
            return false;

        if(flag == 0){
            offlineTime = 0;
        }
    }
    else if(flag == 3){
        localEmergency = false;
        setEmergencyState();
        if(!saveEmergencyState())
            return false;
    }
    return true;
}

//协议解析完成
void ChargeServer::onProtoParsed(InfoMap map, InfoAddrType type)
{
    ProtobufServer::onProtoParsed(map,type);
    QString SendOrderGUID,RecvOrderGUID;
    QString SendBillCode,RecvBillCode;
    char cBillSendReult = 0;
    if(type == AddrType_Unknown){
        if(map.contains(Addr_Resend_Cmd)){
            recvBillMap.clear();
            recvBillMap = map;
            recvBillMap.insert(Addr_ChargeOrder_State, QByteArray(1, 0x02));     //订单状态-结束
            if(billAckTimer == -1){
                billAckTimer = startTimer(20 * 1000); //开启20s定时器
            }
            bBillResendEnd = false;
        }
        else if(map.contains(Addr_Resend_CmdAck)){  //收到协议重发响应: 指令ACK
            if(billAckTimer > 0){
                killTimer(billAckTimer);  //关闭定时器
                billAckTimer = -1;
            }
            if(recvBillMap[Addr_Resend_Cmd].at(0) == (map[Addr_Resend_CmdAck].at(0)- 0x80)){
                SendOrderGUID = recvBillMap[Addr_Order_GUID].data();
                RecvOrderGUID = map[Addr_Order_GUID].data();
                SendBillCode = recvBillMap[Addr_Bill_Code].data();
                RecvBillCode = map[Addr_Bill_Code].data();

                if((SendOrderGUID == RecvOrderGUID) || (SendBillCode == RecvBillCode)){
                    cBillSendReult = 0x01;
                }
                else{//单条订单上传是失败
                    cBillSendReult = 0x00;
                }
                type = AddrType_BillSendReult;
                map.insert(Addr_BillSendReult,QByteArray(1,cBillSendReult));
                proto->command(map, type);
            }
        }
    }
}


