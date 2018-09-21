#include "chargeserver.h"
#include "tcpnet.h"
#include <QTimerEvent>
#include <math.h>

ProtobufServer *ProtobufServer::_server = NULL;

ProtobufServer::ProtobufServer()
{
	_setting = ParamSet::GetInstance();
	_cache = DevCache::GetInstance();
	_filter = RealDataFilter::GetInstance();
    _log = Log::GetInstance();
	_database = DBOperate::GetInstance();

	stateTimeout = CHECK_TIMEOUT;
	serverState = STATE_CHECK;
	stateTimer = -1;
    billAckTimer = -1;
	heartCnt = 0;
    billAckTimerCpunt = 0;
	serverLogged = false;
	offlineTime = 0;
    recvBillMap.clear();
    qstrRecvProtoMessage = "";
    loginTimeoutCnt = 0;

	stCSCUSysConfig confSys;
	_setting->querySetting(&confSys, PARAM_CSCU_SYS);
	acNum = confSys.singlePhase;
	tacNum = confSys.threePhase;
	dcNum = confSys.directCurrent;
    ctrlSwVersion = confSys.version;

    stServer0Config server0Config;
    _setting->querySetting(&server0Config, PARAM_SERVER0);
    chargeEncrypt = server0Config.encrypt;
    monitorEncrypt = server0Config.encrypt;
    stationNo = server0Config.stationNo;
    cscuKey = server0Config.aesKey;

	/*
	PbServerConfig confServer;
	_setting->querySetting(&confServer, PARAM_PROTOBUF_SERVER);
	chargeAddr = confServer.charge_host;
	chargePort = confServer.charge_port;
	chargeEncrypt = confServer.charge_encrypt;
	monitorAddr = confServer.monitor_host;
	monitorPort = confServer.monitor_port;
	monitorEncrypt = confServer.monitor_encrypt;
	stationNo = confServer.station;
	cscuKey = confServer.key;
	*/
}

ProtobufServer::~ProtobufServer()
{
	if(stateTimer > 0){
		killTimer(stateTimer);	
	}

    if(billAckTimer > 0){
        killTimer(billAckTimer);
        billAckTimer = -1;
    }

	if(proto){
		delete proto;
		proto = NULL;
	}

	if(net){
		delete net;	
		net = NULL;
	}
}

/*
 * 动态库接口函数，初始化模块
 * pThread	输入 模块将要运行的线程
 * 返回值 	0表示无错误，-1表示有错误
 */
int ProtobufServer::InitModule(QThread* pThread)
{
	m_pWorkThread = pThread;
	return 0;
}

/*
 * 动态库接口函数，向BUS注册模块
 * pBus		输入 BUS模块指针
 * 返回值 	0表示无错误，-1表示有错误
 */
int ProtobufServer::RegistModule()
{
	return 0;
}

/*
 * 动态库接口函数，启动模块
 * 返回值 	0表示无错误，-1表示有错误dadacdad
 *
 */
int ProtobufServer::StartModule()
{
	this->moveToThread(m_pWorkThread);

	QObject::connect(m_pWorkThread, SIGNAL(started()), this, SLOT(slot_onThreadRun()));
	m_pWorkThread->start();

	return 0;
}

/*
 * 动态库接口函数，停止模块
 * 返回值 	0表示无错误，-1表示有错误
 */
int ProtobufServer::StopModule()
{
	if(proto){
		proto->destroy();
	}
	if(net){
		net->destroy();
	}
	return 0;
}

/*
 * 动态库接口函数，模块工作状态
 * 返回值 	0表示无错误，-1表示有错误
 */
int ProtobufServer::ModuleStatus()
{
	return 0;
}

void ProtobufServer::cmdToBus(InfoMap map, InfoAddrType type)
{
	emit _server->sigToBus(map, type);
}

/*
 * 记录模块执行情况日志
 * strLog	输入 需记录的内容
 * iLevel	输入 日志输出等级
 * 返回值	无
 */
void ProtobufServer::writeLog(QString strLog, int iLevel)
{

}

void ProtobufServer::getCtrlSetting(string &station, string &key, string &version)
{
	station = stationNo;
	key = cscuKey;
    version = ctrlSwVersion;
}

void ProtobufServer::getTermSetting(int &ac, int &tac, int &dc)
{
	ac = acNum;
	tac = tacNum;
	dc = dcNum;
}

void ProtobufServer::getNetSetting(string &host, ushort &port, int &encrypt)
{

}

ProtobufServer *ProtobufServer::server()
{
	return _server;
}

DevCache* ProtobufServer::cache()
{
	return _cache;
}

RealDataFilter* ProtobufServer::filter()
{
	return _filter;
}

ParamSet* ProtobufServer::setting()
{
	return _setting;
}

Log* ProtobufServer::log()
{
	return _log;
}

DBOperate* ProtobufServer::database()
{
	return _database;
}


/*
 * 线程启动槽函数
 *
 * 参数		无
 * 返回值	无
 */
void ProtobufServer::slot_onThreadRun()
{	

}

void ProtobufServer::slotFromBus(InfoMap mapInfo, InfoAddrType type)
{
    if((type == AddrType_Comm_Addr) && (serverState == STATE_GETADDRESS)){
        procCommAddr(mapInfo);
    }
}

void ProtobufServer::onProtoActive(int logged)
{
	serverLogged = logged;
	heartCnt = 0;

	if(serverLogged){
		stateTimeout = HEART_TIMEOUT;
		serverState = STATE_HEART;
        loginTimeoutCnt = 0;
	}else{
        if(serverState != STATE_GETADDRESS){
            stateTimeout = RECONNECT_TIMEOUT;
            serverState = STATE_OFFLINE;
        }
	}

	setNetState(logged);
}

void ProtobufServer::onProtoLive()
{
	heartCnt--;
}

void ProtobufServer::onProtoParsed(InfoMap map, InfoAddrType type)
{
	stateTimeout = HEART_TIMEOUT;
	serverState = STATE_HEART;
	heartCnt = 0;
    loginTimeoutCnt = 0;
	if(type != AddrType_Unknown){
		emit sigToBus(map, type);	
	}
}

void ProtobufServer::timerEvent(QTimerEvent *event)
{
	if(event->timerId() == stateTimer){
		stateMachine();
	}
}

void ProtobufServer::stateMachine()
{
	if(stateTimeout > 0){
		stateTimeout--;
	}

	switch(serverState){
		case STATE_CHECK://数据有效性检查阶段，超时时间60秒，完成后切换至断开状态
			writeLog(QString().sprintf("Data check timeout=%d", stateTimeout), 1);
            if(_filter->isAllValid() || stateTimeout <= 0){
				dataCheckFinished();
				stateTimeout = 0;
                serverState = STATE_GETADDRESS;
			}
			break;
        case STATE_GETADDRESS:
            writeLog(QString().sprintf("Get net address timeout=%d", stateTimeout), 1);
            if(stateTimeout <= 0){
                stateTimeout = WAIT_ADDR_TIMEOUT;

                InfoMap map;
                emit sigToBus(map,AddrType_Comm_Addr_Request);
                writeLog(QString().sprintf("Send Get Address Signal"), 1);
            }
            break;
		case STATE_OFFLINE://离线状态下发起登录，状态向连接中状态切换
			writeLog(QString().sprintf("offline timeout=%d", stateTimeout), 1);
			if(stateTimeout <= 0){
				stateTimeout = LOGIN_TIMEOUT;
				serverState = STATE_LOGIN;

                loginTimeoutCnt++;
                writeLog(QString().sprintf("Apply login count = %d",loginTimeoutCnt),2);
                if(loginTimeoutCnt > 3){   //申请登录３次超时后，再次申请获取通信地址
                    stateTimeout = 0;
                    serverState = STATE_GETADDRESS;
                    loginTimeoutCnt = 0;
                    writeLog(QString().sprintf("Apply login count = %d  again get STATE_GETADDRESS",loginTimeoutCnt),2);
                    break;
                }

				proto->login();	
			}
			break;
		case STATE_LOGIN://登录超时45秒
			writeLog(QString().sprintf("Login timeout=%d", stateTimeout), 1);
			if(stateTimeout <= 0){
				serverState = STATE_OFFLINE;
				stateTimeout = RECONNECT_TIMEOUT;
			}
			break;
		case STATE_HEART://心跳20秒发送一次，两次无回应断开网络
            writeLog(QString().sprintf("Heart timeout=%d", stateTimeout), 1);
            if(stateTimeout <= 0){
				stateTimeout = HEART_TIMEOUT;
                if(heartCnt > 0){
					heartCnt = 0;
					serverState = STATE_OFFLINE;
					stateTimeout = RECONNECT_TIMEOUT;

                    proto->logout(3);

                    InfoMap map;
                    emit sigToBus(map,AddrType_Comm_Addr_Request);
                    writeLog(QString().sprintf("Logout,Send Get Address Signal"), 1);
					break;
				}
				proto->heart();
				heartCnt++;
			}
			break;
		default:
			break;
	}
}

void ProtobufServer::setNetState(bool state)
{

}

void ProtobufServer::dataCheckFinished()
{

}


bool ProtobufServer::makeAlarm(InfoMap map)
{
	if(!map.contains(Addr_CanID_Comm))
		return false;
	if(!map.contains(Addr_CSCU_AlarmCode))
		return false;
	if(!map.contains(Addr_CSCU_AlarmContent))
		return false;
	if(!map.contains(Addr_Order_GUID))
		return false;

	QString strSql, alarmContent, billUuid, billCode;
	QDateTime dt;
	int can, alarmCode;

	can = map[Addr_CanID_Comm].at(0);
	alarmCode = *(int *)map[Addr_CSCU_AlarmCode].data();
	alarmContent = map[Addr_CSCU_AlarmContent].data();
	billCode = map[Addr_Bill_Code].data();
	billUuid = map[Addr_Order_GUID].data();
	dt = QDateTime::currentDateTime();

	strSql.sprintf("INSERT INTO cscu_order_alarm (can_addr, alarm_code, alarm_content, order_code, \
			order_uuid, alarm_time, alarm_valid) VALUES (%d, %d, '%s', '%s', '%s', '%s', 1);",
			can, alarmCode, alarmContent.toAscii().data(), billCode.toAscii().data(), 
			billUuid.toAscii().data(), dt.toString("yyyy-MM-dd hh:mm:ss").toAscii().data());

    if(database()->DBSqlExec(strSql.toAscii().data(), DB_PROCESS_RECORD) != 0)
		return false;

	emit sigToBus(map, AddrType_CSCU_Alarm);

	return true;
}

void ProtobufServer::procCommAddr(InfoMap mapInfo)   //处理通信地址
{
    QString qstrCommAddr,qstrChargePort;
    if(mapInfo.contains(Addr_Error_Addr))
    {
        writeLog(QString().sprintf("Get Address Error Result = %d",mapInfo[Addr_Error_Addr].at(0)),2);
        return;
    }

    if(mapInfo.contains(Addr_Comm_Addr))
    {
        qstrCommAddr = mapInfo[Addr_Comm_Addr].data();
        writeLog(QString().sprintf("Recv Comm_Addr=%1").arg((qstrCommAddr)));
        int index = qstrCommAddr.lastIndexOf(':');
        chargeAddr = (const char *)qstrCommAddr.mid(0,index).toLocal8Bit();
        qstrChargePort = qstrCommAddr.mid(index + 1,qstrCommAddr.size() - index -1);
        chargePort = qstrChargePort.toInt();
        writeLog(QString().sprintf("Get Comm_Addr Success chargeAddr=%1 chargePort=%2").arg(QString::fromStdString(chargeAddr)).arg(chargePort),2);
    }

    if(mapInfo.contains(Addr_Monitor_Addr))
    {
        qstrCommAddr = mapInfo[Addr_Monitor_Addr].data();
        writeLog(QString().sprintf("Recv Monitor_Addr=%1").arg((qstrCommAddr)));
        int index = qstrCommAddr.lastIndexOf(':');
        monitorAddr = (const char *)qstrCommAddr.mid(0,index).toLocal8Bit();
        qstrChargePort = qstrCommAddr.mid(index + 1,qstrCommAddr.size() - index -1);
        monitorPort = qstrChargePort.toInt();
        writeLog(QString().sprintf("Get Monitor_Addr Success monitorAddr=%1 monitorPort=%2").arg(QString::fromStdString(monitorAddr)).arg(monitorPort),2);
    }

//    if((chargeAddr == "" || chargePort <= 0) || (monitorAddr == "" || monitorPort <= 0)){
//        serverState = STATE_GETADDRESS;
//        stateTimeout = WAIT_ADDR_TIMEOUT;
//        writeLog(QString().sprintf("Get Comm Addr NULL!"),2);
//    }
//    else{
        serverState = STATE_OFFLINE;
        stateTimeout = 0;
        writeLog(QString().sprintf("Start Apply Login!"),2);
//    }
}
