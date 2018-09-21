#include "IEC104LocalServer.h"
#include "aes.h"
#include "3DES.h"
#include "commfunc.h"
#include <QTimerEvent>
#include <QStringList>

IEC104LocalServer::IEC104LocalServer()
{
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");

	_strLogName = "server1";

	m_pSetting = ParamSet::GetInstance(); 
	m_pDevCache = DevCache:: GetInstance();
	m_pFilter = RealDataFilter::GetInstance();
    m_pLog = Log::GetInstance();
	m_pDatabase = DBOperate::GetInstance();

	stServerListConfig confServerList;
	stCSCUSysConfig confSys;
	EmergencyConfig confEmergency;

	memset(m_sz3DesKey, 0, sizeof(m_sz3DesKey));
	m_iServerTimer = -1;
	m_iServerTimeout = DATA_CHECK_TIME;
	m_ServerState = DATA_CHECK;
	m_iOfflineTime = 0;

	m_iRebootNo = -1;
	m_sRecvNo = 0;
	m_sSendNo = 0;
	m_iHeartCount = 0;
	m_bConnected = false;
	m_bActived = false;
	m_bEmergency = false;
	m_bExit = false;
    m_cServerType = PARAM_SERVER1;
	m_iHostIndex = -1;
	m_iTryConnect = 0;

    m_pSetting->querySetting(&m_confServer, PARAM_SERVER1);
	//转换站地址
	uchar c = m_confServer.stationNo[10];
	m_confServer.stationNo[10] = '\0';
	m_strStationNo.sprintf("%s%X%s", m_confServer.stationNo, c, &m_confServer.stationNo[11]);
	m_confServer.stationNo[10] = c;
	m_strAesKey = m_confServer.aesKey;

	if(m_cServerType == SERVER_REMOTE)
		m_pSetting->querySetting(&confServerList, PARAM_SERVERLIST);
	
	GetHostList(confServerList);

	m_pSetting->querySetting(&confSys, PARAM_CSCU_SYS);
	m_cCanRange[0][0] = ID_MinACSinCanID;
	m_cCanRange[0][1] = ID_MinACSinCanID + (uchar)confSys.singlePhase;
	m_cCanRange[1][0] = ID_MinACThrCanID;
	m_cCanRange[1][1] = ID_MinACThrCanID + (uchar)confSys.threePhase;
	m_cCanRange[2][0] = ID_MinDCCanID;
	m_cCanRange[2][1] = ID_MinDCCanID + (uchar)confSys.directCurrent;

	m_pSetting->querySetting(&confEmergency, PARAM_EMERGENCY);
	m_iEmergencyTime = confEmergency.check_time;

	WriteLog(QString("Server type=%1").arg(m_cServerType));
	WriteLog(QString("stationNo=%1").arg(m_strStationNo));
	WriteLog(QString("AC=%1 TAC=%2 DC=%3").arg(confSys.singlePhase).arg(confSys.threePhase).arg(confSys.directCurrent));
	WriteLog(QString("AesKey=%1").arg(m_strAesKey));

	m_pSocket = new QTcpSocket();
	connect(m_pSocket, SIGNAL(connected()), this, SLOT(slot_onConnected()));
	connect(m_pSocket, SIGNAL(disconnected()), this, SLOT(slot_onDisconnected()));
	connect(m_pSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slot_onError(QAbstractSocket::SocketError)));
	connect(m_pSocket, SIGNAL(readyRead()), this, SLOT(slot_onReceive()));
}

IEC104LocalServer::~IEC104LocalServer()
{
	disconnect(m_pSocket, SIGNAL(connected()), this, SLOT(slot_onConnected()));
	disconnect(m_pSocket, SIGNAL(disconnected()), this, SLOT(slot_onDisconnected()));
	disconnect(m_pSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slot_onError(QAbstractSocket::SocketError)));
	disconnect(m_pSocket, SIGNAL(readyRead()), this, SLOT(slot_onReceive()));

	if(m_iServerTimer > 0){
		killTimer(m_iServerTimer);	
		m_iServerTimer = -1;
	}

	if(m_pSocket){
		m_pSocket->close();	
		delete m_pSocket;
		m_pSocket = NULL;
	}
}

/*
 * 线程启动槽函数
 *
 * 参数		无
 * 返回值	无
 */
void IEC104LocalServer::slot_onThreadRun()
{
	WriteLog(QString("Thread Run"));

    m_iServerTimer = startTimer(1000);
}

/*
 * QTcpSocket连接成功槽函数，连接成功后先进行密钥协商,
 * 只有密钥协商成功后才认为网络连接真正建立
 *
 * 参数		无
 * 返回值	无
 */
void IEC104LocalServer::slot_onConnected()
{
	WriteLog(QString("Connect success with Host=%1 Port=%2").arg(m_strHost).arg(m_sPort));

	m_ServerState = NET_CONNETED;
	m_iServerTimeout = KEYAGREE_TIME;

	m_bConnected = true;
	m_iHeartCount = 0;
	m_arData.clear();

    SendKeyAgreement();
}

/*
 * QTcpSocket连接断开槽函数，关闭心跳计时器，
 * 重置连接状态，并处理客户端主动断开情况下的网络重连
 *
 * 参数		无
 * 返回值	无
 */
void IEC104LocalServer::slot_onDisconnected()
{
	WriteLog(QString("Disconnect from Host=%1 Port=%2").arg(m_strHost).arg(m_sPort));

	m_ServerState = NET_DISCONNECT;
	m_iServerTimeout = RECONNECT_TIME;

	m_bConnected = false;
	m_bActived = false;
	m_arData.clear();
}

/*
 * QTcpSocket网络错误槽函数，设置网络状态，处理链接未建立情况下的网络重连
 *
 * err		输入 socket错误类型	
 * 返回值	无
 */
void IEC104LocalServer::slot_onError(QAbstractSocket::SocketError err)
{
	WriteLog(m_pSocket->errorString());

	if(err == QAbstractSocket::RemoteHostClosedError)
		SetNetState(false, QString("Remote host close connection"));

	m_ServerState = NET_DISCONNECT;
	m_iServerTimeout = RECONNECT_TIME;
}

/*
 * QTcpSocket数据接收槽函数，每次信号触发时
 * 读出缓冲区内数据，检查网络帧是否符合协议
 * 不符合丢弃，符合进行进一步解析
 *
 * 参数		无
 * 返回值	无
 */
void IEC104LocalServer::slot_onReceive()
{
    int iRecvLen, iPackageLen;
    Net_Hdr* pNetHdr;

	//接收到数据，此时不需发送心跳，先关闭心跳计时器
	m_iServerTimeout = 0;
	m_ServerState = NET_TRANSFER;

	m_arData.append(m_pSocket->readAll());
	iRecvLen = m_arData.length();

	while(!m_bExit && iRecvLen > MIN_NET_PACKAGE_LEN){
		pNetHdr = (Net_Hdr *)m_arData.data();
		iPackageLen = ___constant_swab32(pNetHdr->iLen);
		iPackageLen += MIN_NET_PACKAGE_LEN;

		//数据包合法性检查，不符合则丢弃当前数据
		if(pNetHdr->cHead != NET_FLAG || 
				iPackageLen <= MIN_NET_PACKAGE_LEN || 
				iPackageLen > MAX_NET_PACKAGE_LEN){
			m_arData.clear();
			break;
		}
		//数据长度不够继续收取
		if(iRecvLen < iPackageLen)
			break;

		InfoMap mapInfo;
		InfoAddrType type = AddrType_Unknown;

		if(ParseFrame((uchar*)m_arData.left(iPackageLen).data(), iPackageLen, mapInfo, type) < 0){
			m_arData.clear();
			return;
		}

		if(type != AddrType_Unknown)
			emit sigToBus(mapInfo, type);

		m_arData = m_arData.right(iRecvLen - iPackageLen);
		iRecvLen = m_arData.length();
	}

	//数据处理完毕后，启动20秒心跳计时
	m_ServerState = NET_HEART;
	m_iServerTimeout = HEART_TIME;
}
//能效计划临时用
void IEC104LocalServer::parseBatteryStatus(InfoMap &mapInfo)
{
    batteryInfoMap = mapInfo;
}
void IEC104LocalServer::parsePowerOptimizerStatus(InfoMap &mapInfo)
{
    PowerOptimizerInfoMap = mapInfo;
}

/*
 * 接收其它模块消息
 * mapInfo 	输入 信息体集合
 * type    	输入 消息类型
 * 返回值	无
 */
void IEC104LocalServer::slotFromBus(InfoMap mapInfo, InfoAddrType type)
{
    QIteratorList list;
	QByteArray arData;
	uchar cCmd = 0, cRet = 0;
	ushort sReason = 0;
	int iInfoAddr = 0;
    if(!m_bConnected || !m_bActived){
        WriteLog(QString("slotFromBus type=[%1] When network is not built!").arg(type));
        return;
    }

    if(mapInfo.count() <= 0){
        WriteLog(QString("slotFromBus type=[%1] InfoMap error count=%2!").arg(type).arg(mapInfo.count()));
        return;
    }
	WriteLog(QString("Start slotFromBus type=[%1]").arg(type));
	switch(type){
		//充电响应结果
		case AddrType_CmdCtrl_AckResult:
			if(mapInfo.contains(Addr_OrderNumber_Ctrl))
            	list.append(mapInfo.find(Addr_OrderNumber_Ctrl));
			if(mapInfo.contains(Addr_CanID_Comm))
            	list.append(mapInfo.find(Addr_CanID_Comm));
			if(mapInfo.contains(Addr_AckResult_Ctrl))
            {
            	list.append(mapInfo.find(Addr_AckResult_Ctrl));
                //如果NACK先发送遥信，使用遥信中的终止原因  hd
                if(mapInfo[Addr_AckResult_Ctrl].at(0) !=0xFF)
                    SendSignal(mapInfo[Addr_CanID_Comm].at(0), true);
            }
			cCmd = CMD_RESPONSE_RET;
			sReason = REASON_RETURN;
			break;
    case AddrType_CmdCtrl_AckResult_Peak:
        if(mapInfo.contains(Addr_OrderNumber_Ctrl))
            list.append(mapInfo.find(Addr_OrderNumber_Ctrl));
        if(mapInfo.contains(Addr_CanID_Comm))
            list.append(mapInfo.find(Addr_CanID_Comm));
        if(mapInfo.contains(Addr_AckChargeResult_Ctr_Peak))
            list.append(mapInfo.find(Addr_AckChargeResult_Ctr_Peak));
        if(mapInfo.contains(Addr_StopReasion_Peak))     //hd 削峰填谷需要加入终止充电原因
            list.append(mapInfo.find(Addr_StopReasion_Peak));
        cCmd = CMD_RESPONSE_RET_PEAK;
        sReason = REASON_RETURN;
        break;
    case AddrType_CmdCtrl_AckFaile_Peak:
            if(mapInfo.contains(Addr_OrderNumber_Ctrl))
                list.append(mapInfo.find(Addr_OrderNumber_Ctrl));
            if(mapInfo.contains(Addr_CanID_Comm))
                list.append(mapInfo.find(Addr_CanID_Comm));
            if(mapInfo.contains(Addr_AckResult_Ctrl))
                list.append(mapInfo.find(Addr_AckResult_Ctrl));
            if(mapInfo.contains(Addr_StopReasion_Peak))     //hd 削峰填谷需要加入终止充电原因
                list.append(mapInfo.find(Addr_StopReasion_Peak));
            cCmd = CMD_RESPONSE_RET_FAILE_PEAK;
            sReason = REASON_BURST;
            break;
		//充电执行结果
		case AddrType_CmdCtrl_ExecResult:
			if(mapInfo.contains(Addr_OrderNumber_Ctrl))
            	list.append(mapInfo.find(Addr_OrderNumber_Ctrl));
			if(mapInfo.contains(Addr_CanID_Comm))
            	list.append(mapInfo.find(Addr_CanID_Comm));
			if(mapInfo.contains(Addr_ExecResult_Ctrl))
            	list.append(mapInfo.find(Addr_ExecResult_Ctrl));
			cCmd = CMD_EXEC_RET;
			sReason = REASON_RETURN;
			break;
		//充电电流调整结果
		case AddrType_TermAdjustmentAck:
			break;
			if(mapInfo.contains(Addr_CanID_Comm))
            	list.append(mapInfo.find(Addr_CanID_Comm));
			if(mapInfo.contains(Addr_AdjustCurrent_Ack))
            	list.append(mapInfo.find(Addr_AdjustCurrent_Ack));
			cCmd = CMD_RESPONSE_RET;
			sReason = REASON_RETURN;
			break;
		//突发遥信（遥测电度数同时突发）
		case AddrType_TermSignal:	
			if(mapInfo.contains(Addr_CanID_Comm)){
				if(IsBurstMeasure2(mapInfo))
                    SendMeasure2(mapInfo[Addr_CanID_Comm].at(0), true);

				SendSignal(mapInfo[Addr_CanID_Comm].at(0), true);
			}
			break;
		//突发遥测
		case AddrType_TermMeasure:	
			if(mapInfo.contains(Addr_CanID_Comm))
				SendMeasure2(mapInfo[Addr_CanID_Comm].at(0), true);

			break;
		//日志、配置上传下载执行结果，主程序、模块程序升级结果
		case AddrType_UpdateResult:
			sReason = REASON_RETURN;
			if(!mapInfo.contains(Addr_Cmd_Master))
				return;
            //下发b
			if(mapInfo[Addr_Cmd_Master].at(0) == 0x1){
				if(mapInfo[Addr_Cmd_Slave].at(0) == 0x2){
					cCmd = CMD_UPGRADE_CONFIG;
					iInfoAddr = Addr_CSCUConfigUpdateResult_CtrlInfo;
				}else if(mapInfo[Addr_Cmd_Slave].at(0) == 0x3){
					cCmd = CMD_UPGRADE_MAIN;
					iInfoAddr = Addr_CSCUProgramUpdateResult_CtrlInfo;
					m_iRebootNo = m_sSendNo;
				}else if(mapInfo[Addr_Cmd_Slave].at(0) == 0x4){
					cCmd = CMD_UPGRADE_MODULE;
					iInfoAddr = Addr_ChargerProgramUpdateResult_CtrlInfo;
					if(mapInfo.contains(Addr_CanID_Comm))
            			list.append(mapInfo.find(Addr_CanID_Comm));
				}
			//上传
			}else if(mapInfo[Addr_Cmd_Master].at(0) == 0x2){
				if(mapInfo[Addr_Cmd_Slave].at(0) == 0x1){
					cCmd = CMD_UPLOAD_LOG;
					iInfoAddr = Addr_CSCULogUploadResult_CtrlInfo;
				}else if(mapInfo[Addr_Cmd_Slave].at(0) == 0x2){
					cCmd = CMD_UPLOAD_CONFIG;
					iInfoAddr = Addr_CSCUConfigUploadResult_CtrlInfo;
				}
			}

			if(mapInfo[Addr_Back_Result].at(0) == 0x1)
				cRet = 0xFF;
			else
				cRet = 0x0;
			list.append(mapInfo.insert(iInfoAddr, QByteArray(1, cRet)));
			break;
			//上送冻结电量
		case AddrType_FrozenEnergy:
			SendFrozenEnergy(mapInfo);
            break;
        //车位锁开关执行结果
        case AddrType_CarLock_Result:
            cCmd = CMD_CARLOCK_PARAMSET;
            if(mapInfo.contains(Addr_CarLockID))
                list.append(mapInfo.find(Addr_CarLockID));
            if(mapInfo.contains(Addr_CarLockCmd))
            {
                 list.append(mapInfo.find(Addr_CarLockCmd));
                 cCmd = CMD_CAR_LOCK;
            }
            if(mapInfo.contains(Addr_CarLockCmd_Result))
                list.append(mapInfo.find(Addr_CarLockCmd_Result));            
            sReason = REASON_RETURN;
            break;
            //突发车位锁状态
    case AddrType_TermCarLock :
        if(mapInfo.contains(Addr_CarLockID))
            list.append(mapInfo.find(Addr_CarLockID));
        if(mapInfo.contains(Addr_CarLockStates))
            list.append(mapInfo.find(Addr_CarLockStates));
        if(mapInfo.contains(Addr_ParkingStates))
            list.append(mapInfo.find(Addr_ParkingStates));
        if(mapInfo.contains(Addr_CarLock_SensorFaultStates))
            list.append(mapInfo.find(Addr_CarLock_SensorFaultStates));
        if(mapInfo.contains(Addr_CarLock_StructureFaultStates))
            list.append(mapInfo.find(Addr_CarLock_StructureFaultStates));
            cCmd = CMD_CARLOCK_STATUS;
            sReason = REASON_BURST;
        break;
        //刷卡申请帐户信息
        case AddrType_ChargeServicApplyAccountInfo:
        if(mapInfo[Addr_ScanCode_customerID].length() > 10)
        {
            arData.fill(0xFF, 14);
            if(mapInfo.contains(Addr_CanID_Comm))
                list.append(mapInfo.find(Addr_CanID_Comm));
            arData[0] = QByteArray::fromHex(mapInfo[Addr_ScanCode_customerID]).length();
            if(arData.at(0) > 0 && arData.at(0) <= 16)
                arData.replace(1, arData.at(0), QByteArray::fromHex(mapInfo[Addr_ScanCode_customerID]).data());
            list.append(mapInfo.insert(Addr_ScanCode_customerID, arData));
            if(mapInfo.contains(Addr_ScanCode_Type))
                list.append(mapInfo.find(Addr_ScanCode_Type));
            cCmd = CMD_SCANCODE_CUSTOMER_ID;
            sReason = REASON_BURST;
        }
        else
        {
            arData.fill(0xFF, 11);
            if(mapInfo.contains(Addr_CanID_Comm))
                list.append(mapInfo.find(Addr_CanID_Comm));
            if(mapInfo.contains(Addr_CardAccount)){
                arData[0] = mapInfo[Addr_CardAccount].length();
                if(arData.at(0) > 0 && arData.at(0) <= 10)
                    arData.replace(1, arData.at(0), mapInfo[Addr_CardAccount]);
                list.append(mapInfo.insert(Addr_CardAccount, arData));
            }
            if(mapInfo.contains(Addr_CardAccountType))
                list.append(mapInfo.find(Addr_CardAccountType));
            cCmd = CMD_CARD_ACCOUNT;
            sReason = REASON_BURST;
        }
			break;
        //刷卡、扫码申请开始充电
		case AddrType_OutApplyStartChargeByChargeServic:
        if(mapInfo[Addr_ScanCode_customerID].length() > 10)
        {
            if(mapInfo.contains(Addr_CanID_Comm))
                list.append(mapInfo.find(Addr_CanID_Comm));
            if(mapInfo.contains(Addr_ScanCode_customerID)){
                arData.fill(0xFF, 14);
                arData[0] = QByteArray::fromHex(mapInfo[Addr_ScanCode_customerID]).length();
                if(arData.at(0) > 0 && arData.at(0) <= 16){
                    arData.replace(1, arData.at(0), QByteArray::fromHex(mapInfo[Addr_ScanCode_customerID]).data());
                    list.append(mapInfo.insert(Addr_ScanCode_customerID, arData));
                }
            }
            if(mapInfo.contains(Addr_ScanCode_Charge_Type))
                list.append(mapInfo.find(Addr_ScanCode_Charge_Type));
            if(mapInfo.contains(Addr_CardChargeTypeValue))
                list.append(mapInfo.find(Addr_CardChargeTypeValue));
            cCmd = CMD_SCANCODE_CUSTOMER_ID_CHARGE_APPLY;
            sReason = REASON_BURST;
        }
        else
        {
            if(mapInfo.contains(Addr_CanID_Comm))
                list.append(mapInfo.find(Addr_CanID_Comm));
            if(mapInfo.contains(Addr_CardAccount)){
                arData.fill(0xFF, 11);
                arData[0] = mapInfo[Addr_CardAccount].length();
                if(arData.at(0) > 0 && arData.at(0) <= 10){
                    arData.replace(1, arData.at(0), mapInfo[Addr_CardAccount]);
                    list.append(mapInfo.insert(Addr_CardAccount, arData));
                }
            }
            if(mapInfo.contains(Addr_CardApplyChargeType))
                list.append(mapInfo.find(Addr_CardApplyChargeType));
            if(mapInfo.contains(Addr_CardChargeTypeValue))
                list.append(mapInfo.find(Addr_CardChargeTypeValue));
            cCmd = CMD_CARD_CHARGE_APPLY;
            sReason = REASON_BURST;
        }
			break;
		//刷卡申请结束充电
		case AddrType_OutApplyStopChargeByChargeServic:
        if(mapInfo[Addr_ScanCode_customerID].length() > 10)
        {
            if(mapInfo.contains(Addr_CanID_Comm))
                list.append(mapInfo.find(Addr_CanID_Comm));
            if(mapInfo.contains(Addr_ScanCode_customerID)){
                arData.fill(0xFF, 14);
                arData[0] = QByteArray::fromHex(mapInfo[Addr_ScanCode_customerID]).length();
                if(arData.at(0) > 0 && arData.at(0) <= 16){
                    arData.replace(1, arData.at(0), QByteArray::fromHex(mapInfo[Addr_ScanCode_customerID]).data());
                    list.append(mapInfo.insert(Addr_ScanCode_customerID, arData));
                }
            }
            cCmd = CMD_SCANCODE_CUSTOMER_ID_CHARGE_STOP;
            sReason = REASON_BURST;
        }
        else
        {
            if(mapInfo.contains(Addr_CanID_Comm))
                list.append(mapInfo.find(Addr_CanID_Comm));
            if(mapInfo.contains(Addr_CardAccount)){
                arData.fill(0xFF, 11);
                arData[0] = mapInfo[Addr_CardAccount].length();
                if(arData.at(0) > 0 && arData.at(0) <= 10){
                    arData.replace(1, arData.at(0), mapInfo[Addr_CardAccount]);
                    list.append(mapInfo.insert(Addr_CardAccount, arData));
                }
            }
            cCmd = CMD_CARD_CHARGE_STOP;
            sReason = REASON_BURST;
        }
			break;
		//zigbee申请开始（结束）充电
		case AddrType_VinApplyStartCharge:
		case AddrType_VinApplyStopCharge:
			arData.fill(0xFF, 24);
			arData[0] = 0x02;
			arData[1] = mapInfo[Addr_BatteryVIN_BMS].length();
			arData.replace(2, arData.at(1), mapInfo[Addr_BatteryVIN_BMS]);
            list.append(mapInfo.insert(Addr_ZigbeeUserID, arData));
			if(mapInfo.contains(Addr_CanID_Comm))
            	list.append(mapInfo.find(Addr_CanID_Comm));
            list.append(mapInfo.insert(Addr_ZigbeeChargeType, mapInfo[Addr_VINApplyStartChargeType]));
			cCmd = CMD_ZIGBEE_CHARGE;
			sReason = REASON_BURST;
			break;
		//zigbee车牌号申请开始（结束）充电
		case AddrType_CarLicenceApplyStartCharge:
		case AddrType_CarLicenceApplyStopCharge:
			arData.fill(0xFF, 24);
			arData[0] = 0x03;
			arData[1] = mapInfo[Addr_CarLicense].length();
			arData.replace(2, arData.at(1), mapInfo[Addr_CarLicense]);
            list.append(mapInfo.insert(Addr_ZigbeeUserID, arData));
			if(mapInfo.contains(Addr_CanID_Comm))
            	list.append(mapInfo.find(Addr_CanID_Comm));
            list.append(mapInfo.insert(Addr_ZigbeeChargeType, mapInfo[Addr_CarLicenseApplyStartChargeType]));
			cCmd = CMD_ZIGBEE_CHARGE;
			sReason = REASON_BURST;
			break;
		//开始获取Can地址与终端对应关系
		case AddrType_TermIndex_Query:
			list.append(mapInfo.insert(Addr_TermName_Adj, QByteArray(0, '\0')));
			cCmd = CMD_TERMINAL_INDEX;
			sReason = REASON_BURST;
			break;
		//设置充电优先等级返回结果
		case AddrType_ChargePriority_Result:
			if(mapInfo.contains(Addr_CanID_Comm))
            	list.append(mapInfo.find(Addr_CanID_Comm));
			if(mapInfo.contains(Addr_ChargePriority_Result))
            	list.append(mapInfo.find(Addr_ChargePriority_Result));
			cCmd = CMD_CHANGE_PRIORITY;
			sReason = REASON_RETURN;
			break;
		//设置充电完成时间返回结果
		case AddrType_ChargeFinishTime_Result:
			if(mapInfo.contains(Addr_CanID_Comm))
            	list.append(mapInfo.find(Addr_CanID_Comm));
			if(mapInfo.contains(Addr_ChargeFinishTime_Result))
            	list.append(mapInfo.find(Addr_ChargeFinishTime_Result));
			cCmd = CMD_CHARGE_FINISH_TIME;
			sReason = REASON_RETURN;
		//设置充电模式返回结果
		case AddrType_ChargeMode_Result:
			if(mapInfo.contains(Addr_CanID_Comm))
            	list.append(mapInfo.find(Addr_CanID_Comm));
			if(mapInfo.contains(Addr_ChargeMode_Result))
            	list.append(mapInfo.find(Addr_ChargeMode_Result));
			cCmd = CMD_CHARGE_MODE;
			sReason = REASON_RETURN;
			break;
		//突发充电预估完成时间
		case AddrType_PredictTime_Burst:
			if(mapInfo.contains(Addr_CanID_Comm))
            	list.append(mapInfo.find(Addr_CanID_Comm));
			if(mapInfo.contains(Addr_PredictTime_Burst))
            	list.append(mapInfo.find(Addr_PredictTime_Burst));
			cCmd = CMD_AUXPOWER;
			sReason = REASON_BURST;
			break;
		//设置电网最大允许负荷返回结果
		case AddrType_MaxLoad_Result:
			if(mapInfo.contains(Addr_CanID_Comm))
            	list.append(mapInfo.find(Addr_CanID_Comm));
			if(mapInfo.contains(Addr_MaxLoad_Result))
            	list.append(mapInfo.find(Addr_MaxLoad_Result));
			cCmd = CMD_MAX_LOAD;
			sReason = REASON_RETURN;
			break;
		//设置群充策略返回结果
		case AddrType_GroupPolicy_Result:
			if(mapInfo.contains(Addr_CanID_Comm))
            	list.append(mapInfo.find(Addr_CanID_Comm));
			if(mapInfo.contains(Addr_GroupPolicy_Result))
            	list.append(mapInfo.find(Addr_GroupPolicy_Result));
			cCmd = CMD_GROUP_POLICY;
			sReason = REASON_RETURN;
			break;
		//设置辅助电源类型返回结果
		case AddrType_AuxPower_Result:
			if(mapInfo.contains(Addr_CanID_Comm))
            	list.append(mapInfo.find(Addr_CanID_Comm));
			if(mapInfo.contains(Addr_AuxPower_Result))
            	list.append(mapInfo.find(Addr_AuxPower_Result));
			cCmd = CMD_AUXPOWER;
			sReason = REASON_RETURN;
			break;
		//突发模块是否空闲
		case AddrType_ModuleFree_Burst:
			if(mapInfo.contains(Addr_CanID_Comm))
            	list.append(mapInfo.find(Addr_CanID_Comm));
			if(mapInfo.contains(Addr_ModuleFree))
            	list.append(mapInfo.find(Addr_ModuleFree));
			cCmd = CMD_MODULE_FREE;
			sReason = REASON_BURST;
			break;
		//突发充电排队信息
		case AddrType_QueueInfo_Burst:
			if(mapInfo.contains(Addr_CanID_Comm))
            	list.append(mapInfo.find(Addr_CanID_Comm));
			if(mapInfo.contains(Addr_QueueInfo_Burst))
            	list.append(mapInfo.find(Addr_QueueInfo_Burst));
			cCmd = CMD_QUEUE_INFO;
			sReason = REASON_BURST;
			break;
		//获取充电预估完成时间
		case AddrType_PredictTime_Apply:
			if(mapInfo.contains(Addr_CanID_Comm))
            	list.append(mapInfo.find(Addr_CanID_Comm));
			cCmd = CMD_CHARGE_PREDICT_TIME;
			sReason = REASON_BURST;
			break;
		//下发直流充电终端双系统300kw参数设置返回结果
		case AddrType_DoubleSys300kwSetting_Result:
			if(mapInfo.contains(Addr_300kw_Result))
            	list.append(mapInfo.find(Addr_300kw_Result));
			cCmd = CMD_300KW_TERM_PARAM;
			sReason = REASON_RETURN;
			break;
		//上报直流充电终端双系统300kw参数设置
		case AddrType_DoubleSys300kwSetting_Upload:
			if(mapInfo.contains(Addr_300kw_CanID))
            	list.append(mapInfo.find(Addr_300kw_CanID));
			if(mapInfo.contains(Addr_300kw_Group))
            	list.append(mapInfo.find(Addr_300kw_Group));
			if(mapInfo.contains(Addr_300kw_PairCanID))
            	list.append(mapInfo.find(Addr_300kw_PairCanID));
			if(mapInfo.contains(Addr_300kw_WorkMode))
            	list.append(mapInfo.find(Addr_300kw_WorkMode));
			if(mapInfo.contains(Addr_300kw_ChangeMode))
            	list.append(mapInfo.find(Addr_300kw_ChangeMode));
			if(mapInfo.contains(Addr_300kw_ChangeTime))
            	list.append(mapInfo.find(Addr_300kw_ChangeTime));
			cCmd = CMD_300KW_TERM_PARAM;
			sReason = REASON_BURST;
			break;
		//输出继电器控制指令返回结果
		case AddrType_RelayControl_Result:
			if(mapInfo.contains(Addr_Relay_ID))
            	list.append(mapInfo.find(Addr_Relay_ID));
			if(mapInfo.contains(Addr_Relay_Result))
            	list.append(mapInfo.find(Addr_Relay_Result));
			cCmd = CMD_RELAY_CONTROL;
			sReason = REASON_RETURN;
			break;
       //单双枪充电方式及分组信息校验成功后突发给平台
       case AddrType_CheckChargeManner_Success:
        if(mapInfo.contains(Addr_ChargeGunType))
            list.append(mapInfo.find(Addr_ChargeGunType));
        if(mapInfo.contains(Addr_ChargeGun_Master))
            list.append(mapInfo.find(Addr_ChargeGun_Master));
        if(mapInfo.contains(Addr_ChargeGun_Slave1) && (mapInfo.value(Addr_ChargeGun_Slave1).at(0)))
            list.append(mapInfo.find(Addr_ChargeGun_Slave1));
        if(mapInfo.contains(Addr_ChargeGun_Slave2) && (mapInfo.value(Addr_ChargeGun_Slave2).at(0)))
            list.append(mapInfo.find(Addr_ChargeGun_Slave2));
        if(mapInfo.contains(Addr_ChargeGun_Slave3) && (mapInfo.value(Addr_ChargeGun_Slave3).at(0)))
            list.append(mapInfo.find(Addr_ChargeGun_Slave3));
        if(mapInfo.contains(Addr_ChargeGun_Slave4) && (mapInfo.value(Addr_ChargeGun_Slave4).at(0)))
            list.append(mapInfo.find(Addr_ChargeGun_Slave4));
        if(mapInfo.contains(Addr_ChargeGun_Slave5) && (mapInfo.value(Addr_ChargeGun_Slave5).at(0)))
            list.append(mapInfo.find(Addr_ChargeGun_Slave5));
        if(mapInfo.contains(Addr_ChargeGun_Slave6) && (mapInfo.value(Addr_ChargeGun_Slave6).at(0)))
            list.append(mapInfo.find(Addr_ChargeGun_Slave6));
        cCmd = CMD_CHARGEGUN_GUN;
        sReason = REASON_BURST;
            break;
        case AddrType_RemoteAmmeterSendType:   //远程抄表数据主题
            SendAmmeterDataType(mapInfo,list);  //发送电表数据

            list.clear();
            if(mapInfo.contains(Addr_RemoteAmeterAddr_Adj))
                list.append(mapInfo.find(Addr_RemoteAmeterAddr_Adj));
            if(mapInfo.contains(Addr_RemoteAmeterType_Adj))
                list.append(mapInfo.find(Addr_RemoteAmeterType_Adj));
            if(mapInfo.contains(Addr_RemoteAmeterReadTime_Adj))
                list.append(mapInfo.find(Addr_RemoteAmeterReadTime_Adj));

            cCmd = CMD_AMMETER_DATA_SELECT;
            sReason = REASON_ACTIVE_END;
            break;
		//回应平台本地应急充电状态
		case AddrType_LocalEmergency_Result:
			if(mapInfo.contains(Addr_Local_Emergency))
				list.append(mapInfo.find(Addr_Local_Emergency));
			cCmd = CMD_LOCAL_EMERGENCY_RET;
			sReason = REASON_RETURN;
			break;
		//发送本地应急充电状态
		case AddrType_LocalEmergency_State:
			if(mapInfo.contains(Addr_Local_Emergency))
				list.append(mapInfo.find(Addr_Local_Emergency));
			cCmd = CMD_LOCAL_EMERGENCY;
			sReason = REASON_BURST;
			break;
    case AddrType_BatteryStatus://解析电池状态信息
        parseBatteryStatus(mapInfo);
        break;
    case AddrType_PowerOptimizerInfo:
        parsePowerOptimizerStatus(mapInfo);
        break;
    case AddrType_InPeakApplyStopCharge_Result_Server:    //削峰填谷结束充电上传结果和冻结电量hd
        WriteLog(QString("AAAAAAAAAAAAAAAAAAAAAAAAAAAa]"));
        SendFrozenEnergyPeakShaving(mapInfo);
        break;

	case AddrType_ActiveDefend_Alarm://主动防护告警
		SendActiveDefendAlarm(mapInfo);
		break;

	default:
			break;
	}
	if(list.count() > 0){
		SendFrameI(list, cCmd, sReason);
	}
	WriteLog(QString("Finish slotFromBus type=[%1]").arg(type));
}

/*
 * 服务器链接状态计时器处理函数
 * event	输入 Timer事件对象
 * 返回值	无
 */
void IEC104LocalServer::timerEvent(QTimerEvent *event)
{
	if(event->timerId() == m_iServerTimer){
		if(m_iServerTimeout > 0)
			m_iServerTimeout--;

		if(m_ServerState < NET_ACTIVED && m_ServerState > DATA_CHECK){
			m_iOfflineTime++;
			if(m_iOfflineTime >= m_iEmergencyTime){
				m_iOfflineTime = 0;
				if(!m_bEmergency){
					m_bEmergency = true;
					SendNetState(true);
					WriteLog(QString().sprintf("NetWork Offline Emit Signal To ChargeService!"));
				}
			}
		}

		//m_ServerState切换过程 check->disconnected->connect->connecting->connected->keyagreement->transfer->heart
		switch(m_ServerState){
			case DATA_CHECK://数据有效性检查阶段，超时时间60秒，完成后切换至断开状态
				WriteLog(QString().sprintf("Wait for data valid %d", m_iServerTimeout), 1);
				if(m_pFilter->isAllValid() || m_iServerTimeout <= 0){
					m_ServerState = NET_DISCONNECT;	
					m_iServerTimeout = 0;
				}
				break;
			case NET_DISCONNECT://断开状态下立即发起连接，状态向连接中状态切换
				WriteLog(QString().sprintf("Connect %d", m_iServerTimeout), 1);
				if(m_iServerTimeout <= 0){
					connectToServer();
					m_ServerState = NET_CONNECTING;
					m_iServerTimeout = CONNECTING_TIME;
				}
				break;
			case NET_CONNECTING://连接中超时60秒
				WriteLog(QString().sprintf("Connecting %d", m_iServerTimeout), 1);
				if(m_iServerTimeout <= 0){
					m_pSocket->abort();
					m_pSocket->close();
					m_ServerState = NET_DISCONNECT;
				}
				break;
			case NET_CONNETED://密钥协商超时45秒
				WriteLog(QString().sprintf("KeyAgree %d", m_iServerTimeout), 1);
				if(m_iServerTimeout <= 0){
					m_pSocket->close();	
					m_ServerState = NET_DISCONNECT;
					WriteLog(QString("KeyAgreement timeout!"));
				}
				break;
			case NET_HEART://心跳20秒发送一次，两次无回应断开网络
				WriteLog(QString().sprintf("Heart %d", m_iServerTimeout), 1);
				if(m_iServerTimeout <= 0){
					m_iServerTimeout = HEART_TIME;
					if(m_iHeartCount > 0){
						m_pSocket->close();
						SetNetState(false, "Not receive heart response");	
						WriteLog(QString("Not receive heart response"));
						return;
					}

					SendFrameU(CMD_TEST);	
					m_iHeartCount++;
				}
				break;
			case NET_TRANSFER:
				break;
			default:
				break;
		}
	}
}

/*
 * 网络帧解析入口函数
 * pszData 输入 帧数据
 * iLen	   输入 帧长度
 * mapInfo 输出	信息体数据
 * type    输出 指令类型
 * 返回值  成功返回0，有错误发生返回-1
 */
int IEC104LocalServer::ParseFrame(uchar *pszData, int iLen, InfoMap& mapInfo, InfoAddrType& type)
{
    Net_Hdr* pNetHdr;
	pNetHdr = (Net_Hdr *)pszData;
	InfoMap map;

	switch(pNetHdr->cType){
		case 0x02:
			if(!ParseKeyAgreement(pszData, iLen)){
				m_pSocket->close();
				m_ServerState = NET_DISCONNECT;
				WriteLog(QString("Parse KeyAgreement failed!"));
				return -1;
			}
			break;
		case 0x04:
			m_ServerState = NET_ACTIVED;
			m_iOfflineTime = 0;
			m_iTryConnect = 0;
			m_bActived = true;
			SendNetState(false);
			m_bEmergency = false;
			WriteLog(QString("KeyAgreement success!"));
			break;
		case 0x05:
			m_pSocket->close();
			m_ServerState = NET_DISCONNECT;
			WriteLog(QString("KeyAgreement failed!"));
			return -1;
		case 0x06:
			Parse104Frame(pszData + sizeof(Net_Hdr), iLen - sizeof(Net_Hdr), mapInfo, type);
			break;
		default:
			m_pSocket->close();
			m_ServerState = NET_DISCONNECT;
			WriteLog(QString("Invalid net frame!"));
			return -1;
	}

	return 0;
}

/* 
 * 发送密钥协商请求
 * 集控器与服务器的加密连接过程：
 * 1、集控器发送站地址至服务器，开始密钥协商
 * 2、集控器使用PK解密收到的密文，R1作为密钥加密R2，发送R2
 * 3、服务器端接收到R2密文，使用R1作为3DES密钥解密R2
 * 服务器将解密的R2与保存的R2对比，相同则加密连接建立成功
 * 4、集控器接收服务器处理结果，完成加密协商                
 * 返回值 	无	
 */
void IEC104LocalServer::SendKeyAgreement()
{
	Frame_Net stFrame;
	QString sStrKey, sTemp;
	int iLen;

	memset(&stFrame, 0, sizeof(Frame_Net));

	sStrKey = m_strAesKey;
    if(sStrKey.length() < 32)
        sStrKey = QString("A1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A1").left(32 - sStrKey.length()) + sStrKey;

	//转换AES使用的PK
	AsciToBin((uchar*)sStrKey.toAscii().data(), 32, m_sz3DesKey);

	stFrame.hdr.cHead = NET_FLAG;
	stFrame.hdr.cType = 0x01;
	sTemp = m_strStationNo + "                             ";
    CharToBCD((uchar*)sTemp.toAscii().data(), 16, stFrame.szData);
	iLen = sizeof(Net_Hdr) + 8;
	stFrame.hdr.iLen = ___constant_swab32(iLen - 5);

    m_pSocket->write((char *)&stFrame, iLen);
}

/*
 * 获取后续通信中3DES的密钥
 * 返回值 	成功返回true，失败返回false	
 */
bool IEC104LocalServer::ParseKeyAgreement(uchar* pszData, int iLen)
{
	Frame_Net stFrame;
	int iEncrypt;
    uchar psR1R2[128];

	memset(&stFrame, 0, sizeof(Frame_Net));
    memset(psR1R2,0x0,sizeof(psR1R2));

	//使用PK解密收到的Caes
	AesEncrypt(&pszData[8], iLen - 8, m_sz3DesKey, psR1R2, 0);
	//前16位(R1)作为3DES密钥
	memcpy(m_sz3DesKey, psR1R2, 16);

	stFrame.hdr.cHead = NET_FLAG;
	stFrame.hdr.cType = 0x03;

	//用R1作为密钥加密后16位(R2)，并将得到的密文发送至服务器
	//如果成功则R1作为通信中的密钥
    iEncrypt = data_convert(&psR1R2[16], 16, stFrame.szData, 1, m_sz3DesKey);
	if(iEncrypt < 0){
		WriteLog(QString("Encrypt agreement data failed!"));
		return false;
	}

	iLen = iEncrypt + sizeof(Net_Hdr);
	stFrame.hdr.iLen = ___constant_swab32(iLen - 5);

    m_pSocket->write((char *)&stFrame, iLen);

    return true;
}

/*
 * 解析U帧
 * pszData 输入 帧数据
 * iLen	   输入 数据长度
 * 返回值 	成功返回true，失败返回false	
 */
bool IEC104LocalServer::ParseFrameU(uchar* pszData, int iLen)
{
	APCI* pApci = (APCI*)pszData;

	if(iLen <= 0)
		return false;

	switch(pApci->cCtrl1){
		case CMD_STARTDT:
			WriteLog(QString("Recv startDT from server!"));
			SetNetState(true);	
			SendFrameU(CMD_STARTDTACK);
			break;
		case CMD_STARTDTACK:
			break;
		case CMD_STOPDT:
			WriteLog(QString("Recv stopDT from server!"));
			SendFrameU(CMD_STOPDTACK);
			break;
		case CMD_STOPDTACK:
			break;
		case CMD_TEST:
			WriteLog(QString("Recv heart from server!"));
			SendFrameU(CMD_TESTACK);
		case CMD_TESTACK:
			m_iHeartCount--;
			break;
		default:
			return false;
	}

	return true;
}

/*
 * U帧组包并发送
 * cCmd 	输入 U帧类型
 * 返回值 	无
 */
void IEC104LocalServer::SendFrameU(uchar cCmd)
{
	Frame_U stFrame;
	int iLen;

	if(!m_bConnected)
		return;

	memset(&stFrame, 0, sizeof(Frame_U));

	stFrame.apci.cHead = IEC_FLAG;
	stFrame.apci.cCtrl1 = cCmd;
	AddBCDStationNo(m_strStationNo, stFrame.szStation);
	iLen = sizeof(Frame_U);
	stFrame.apci.cLen =  iLen - 2;

	WriteLog((uchar *)&stFrame, iLen, FRAME_TYPE_U);

    SendEncryptData((uchar *)&stFrame, iLen);
}

/*
 * 解析I帧数据
 * pszData 	输入 I帧数据
 * iLen 	输入 数据长度
 * mapInfo 	输出 信息体集合
 * type 	输出 消息类型
 * 返回值 	成功返回true，失败返回false	
 */
bool IEC104LocalServer::ParseFrameI(uchar* pszData, int iLen, InfoMap& mapInfo, InfoAddrType& type)
{
	APCI* pApci;
	ASDU_Hdr* pAsduHdr;
    QIteratorList list;
	QByteArray arData;

	pApci = (APCI *)pszData;
	pAsduHdr = (ASDU_Hdr *)(pszData + sizeof(APCI));

	m_sRecvNo = (pApci->cCtrl1 | (pApci->cCtrl2 << 8)) >> 1;

	pszData = pszData + sizeof(APCI) + sizeof(ASDU_Hdr);
	iLen = iLen - sizeof(APCI) - sizeof(ASDU_Hdr);

    if(!ParseInfoTag(pszData, iLen, pAsduHdr->cLimit, mapInfo))     //解析信息体
		return false;

	switch(pAsduHdr->cType){
		//下发遥控类指令
		case CMD_CHARGE:
			mapInfo[Addr_ServerType_Comm] = QByteArray(1, m_cServerType);
			type = AddrType_CmdCtrl_Apply;
			break;
        //下发参数设置（电表）按0x99做
        case CMD_STATION_PARAM:
        {
            if(mapInfo.contains(Addr_AmeterAddr_Adj)){  //第一块电表地址
                list.append(mapInfo.find(Addr_AmeterAddr_Adj));

                if(!SetAmmeterParamSet(mapInfo)){
                    list.append(mapInfo.insert(Addr_AmeterSetResult_Adj, QByteArray(1, (uchar)0x01)));  //失败0x01
                    SendFrameI(list, CMD_STATION_PARAM, REASON_RETURN);
                }
                else{
                    list.append(mapInfo.insert(Addr_AmeterSetResult_Adj, QByteArray(1, (uchar)0xFF))); //成功
                    SendFrameI(list, CMD_STATION_PARAM, REASON_RETURN);
                }
            }
            else{
                WriteServerList(mapInfo);
            }
        }
			break;
		//总召唤
		case CMD_QUERY_ALL:
            list.append(mapInfo.insert(Addr_Default_Comm, QByteArray(1, (uchar)0x14)));
			SendFrameI(list, CMD_QUERY_ALL, REASON_ACTIVE_ACK);
			SendQueryAll(QUERY_MEASURE2);
            SendQueryAll(QUERY_MEASURE3);
			SendQueryAll(QUERY_MEASURE);
			SendQueryAll(QUERY_SIGNAL);
			SendFrameI(list, CMD_QUERY_ALL, REASON_ACTIVE_END);
			break;
		//电度总召
		case CMD_QUERY_MEASURE2:
            list.append(mapInfo.insert(Addr_Default_Comm, QByteArray(1, (uchar)0x14)));
			SendFrameI(list, CMD_QUERY_ALL, REASON_ACTIVE_ACK);
			SendQueryAll(QUERY_MEASURE2);
			SendFrameI(list, CMD_QUERY_ALL, REASON_ACTIVE_END);
			break;
		//召唤模块遥信
		case CMD_QUERY_SIGNAL:			
			if(mapInfo.contains(Addr_CanID_Comm)){
				list.append(mapInfo.find(Addr_CanID_Comm));
				SendFrameI(list, CMD_QUERY_SIGNAL, REASON_ACTIVE_ACK);
				SendSignal(mapInfo[Addr_CanID_Comm].at(0), true);
				SendFrameI(list, CMD_QUERY_SIGNAL, REASON_ACTIVE_END);
			}
			break;
		//召唤模块遥测
		case CMD_QUERY_MEASURE:
			if(mapInfo.contains(Addr_CanID_Comm)){
				list.append(mapInfo.find(Addr_CanID_Comm));
				SendFrameI(list, CMD_QUERY_MEASURE, REASON_ACTIVE_ACK);
				SendMeasure(mapInfo[Addr_CanID_Comm].at(0));
				SendMeasure2(mapInfo[Addr_CanID_Comm].at(0));
				SendFrameI(list, CMD_QUERY_MEASURE, REASON_ACTIVE_END);
			}
			break;
		//召唤BMS信息
		case CMD_QUERY_BMS:
			if(mapInfo.contains(Addr_CanID_Comm)){
				list.append(mapInfo.find(Addr_CanID_Comm));
				SendFrameI(list, CMD_QUERY_BMS, REASON_ACTIVE_ACK);
				SendBMS(mapInfo[Addr_CanID_Comm].at(0));
				SendFrameI(list, CMD_QUERY_BMS, REASON_ACTIVE_END);
			}
			break;
		//子站名称，CAN地址和终端编号对应关系 
		case CMD_TERMINAL_INDEX:
			if(mapInfo.contains(Addr_TermName_Adj) && pAsduHdr->sReason == 0x06){
				m_mapCanIndex.clear();
				m_mapCanIndex[Addr_TermName_Adj].append(mapInfo[Addr_TermName_Adj]);
			}
			if(mapInfo.contains(Addr_TermIndex_Adj)){
				m_mapCanIndex[Addr_TermIndex_Adj].append(mapInfo[Addr_TermIndex_Adj]);
			}
			if(pAsduHdr->sReason == 0x0A){
				mapInfo = m_mapCanIndex;
				m_mapCanIndex.clear();
				type = AddrType_TermIndex_QueryFinish;
			}
			break;
		//召唤子站版本号
		case CMD_VERSION:
            list.append(mapInfo.insert(Addr_Default_Comm, QByteArray(1, (uchar)0x14)));
			SendFrameI(list, CMD_VERSION, REASON_ACTIVE_ACK);
			SendVersion();
			SendFrameI(list, CMD_VERSION, REASON_ACTIVE_END);
			break;
		//召唤模块版本号
		case CMD_MODULE_VERSION:
			if(mapInfo.contains(Addr_CanID_Comm)){
				list.append(mapInfo.find(Addr_CanID_Comm));
				SendFrameI(list, CMD_MODULE_VERSION, REASON_ACTIVE_ACK);
				SendModuleVersion(mapInfo[Addr_CanID_Comm].at(0));
				SendFrameI(list, CMD_MODULE_VERSION, REASON_ACTIVE_END);
			}
			break;
		//召唤子站高压侧数据
		case CMD_QUERY_HIGHVOL:
            list.append(mapInfo.insert(Addr_Default_Comm, QByteArray(1, (uchar)0x14)));
			SendFrameI(list, CMD_QUERY_HIGHVOL, REASON_ACTIVE_ACK);
			SendHighVolate();
			SendFrameI(list, CMD_QUERY_HIGHVOL, REASON_ACTIVE_END);
			break;
		//召唤子站环境监测数据
		case CMD_ENV_DATA:
            list.append(mapInfo.insert(Addr_Default_Comm, QByteArray(1, (uchar)0x14)));
			SendFrameI(list, CMD_ENV_DATA, REASON_ACTIVE_ACK);
			SendStationEnv();
			SendFrameI(list, CMD_ENV_DATA, REASON_ACTIVE_END);
			break;
		//获取日志文件
		case CMD_UPLOAD_LOG:
			mapInfo[Addr_Cmd_Source] = QByteArray(1, (uchar)0x1);
			mapInfo[Addr_Cmd_Master] = QByteArray(1, (uchar)0x2);
			mapInfo[Addr_Cmd_Slave] = QByteArray(1, (uchar)0x1);
			mapInfo[Addr_Update_Upload_Param] = mapInfo[Addr_CSCULogUploadPath_CtrlInfo];
			type = AddrType_ExecUpdate;
			break;
		//上传配置文件
		case CMD_UPLOAD_CONFIG:
			mapInfo[Addr_Cmd_Source] = QByteArray(1, (uchar)0x1);
			mapInfo[Addr_Cmd_Master] = QByteArray(1, (uchar)0x2);
			mapInfo[Addr_Cmd_Slave] = QByteArray(1, (uchar)0x2);
			mapInfo[Addr_Update_Upload_Param] = mapInfo[Addr_CSCUConfigUploadPath_CtrlInfo];
			type = AddrType_ExecUpdate;
			break;
		//升级配置文件
		case CMD_UPGRADE_CONFIG:
			mapInfo[Addr_Cmd_Source] = QByteArray(1, (uchar)0x1);
			mapInfo[Addr_Cmd_Master] = QByteArray(1, (uchar)0x1);
			mapInfo[Addr_Cmd_Slave] = QByteArray(1, (uchar)0x2);
			mapInfo[Addr_Update_Upload_Param] = mapInfo[Addr_CSCUConfigUpdatePath_CtrlInfo];
			type = AddrType_ExecUpdate;
			break;
		//升级子站程序
		case CMD_UPGRADE_MAIN:
			mapInfo[Addr_Cmd_Source] = QByteArray(1, (uchar)0x1);
			mapInfo[Addr_Cmd_Master] = QByteArray(1, (uchar)0x1);
			mapInfo[Addr_Cmd_Slave] = QByteArray(1, (uchar)0x3);
			mapInfo[Addr_Update_Upload_Param] = mapInfo[Addr_CSCUProgramUpdatePath_CtrlInfo];
			type = AddrType_ExecUpdate;
			break;
		//升级模块程序
		case CMD_UPGRADE_MODULE:
			mapInfo[Addr_Cmd_Source] = QByteArray(1, (uchar)0x1);
			mapInfo[Addr_Cmd_Master] = QByteArray(1, (uchar)0x1);
			mapInfo[Addr_Cmd_Slave] = QByteArray(1, (uchar)0x4);
			mapInfo[Addr_Update_Upload_Param] = mapInfo[Addr_ChargerProgramUpdatePath_CtrlInfo];
			type = AddrType_ExecUpdate;
			break;
		//时间同步
		case CMD_SYNC_TIME:
            SyncTime(mapInfo);
			break;
		//重启子站
		case CMD_REBOOT:
            list.append(mapInfo.insert(Addr_Reboot_Result, QByteArray(1, (uchar)0xFF)));
			m_iRebootNo = m_sSendNo;
			SendFrameI(list, CMD_REBOOT, REASON_RETURN);
			break;
		//车位锁控制
		case CMD_CAR_LOCK:
			type = AddrType_CarLock_Apply;
			break;
            //车位锁参数设置
       case CMD_CARLOCK_PARAMSET:
           type = AddrType_CarLock_ParamSet;
           break;
		//刷卡帐户信息
		case CMD_CARD_ACCOUNT:
			type = AddrType_ChargeServicApplyAccountInfo_Result;
			break;
            //扫码帐户信息     add by zjq
    case CMD_SCANCODE_CUSTOMER_ID:
        type = AddrType_ChargeServicApplyAccountInfo_Result;
        break;
		//刷卡申请充电结果
		case CMD_CARD_CHARGE_APPLY:
			type = AddrType_OutApplyStartChargeByChargeServic_Result;
			break;
			//刷卡申请结束充电结果
		case CMD_CARD_CHARGE_STOP:
			type = AddrType_OutApplyStopChargeByChargeServic_Result;
			break;
            //扫码申请充电结果
      case CMD_SCANCODE_CUSTOMER_ID_CHARGE_APPLY:
            type = AddrType_OutApplyStartChargeByChargeServic_Result;
              break;
            //扫码申请结束充电结果
       case CMD_SCANCODE_CUSTOMER_ID_CHARGE_STOP:
          type = AddrType_OutApplyStopChargeByChargeServic_Result;
          break;
			//zigbee申请开始充电结果
		case CMD_ZIGBEE_CHARGE:
			{
				uchar cVinType, cLen;
				arData = mapInfo[Addr_ZigbeeUserID];
				cVinType = arData.at(0);
				cLen = arData.at(1);
				arData = arData.mid(2, cLen); 

				if(cVinType == 0x02){//VIN
					mapInfo[Addr_BatteryVIN_BMS] = arData;
					mapInfo.remove(Addr_ZigbeeUserID);
					if(mapInfo.contains(Addr_ZigbeeStartChargeResult)){
						mapInfo[Addr_VINApplyStartChargeType_Result] = mapInfo[Addr_ZigbeeStartChargeResult];
						mapInfo.remove(Addr_ZigbeeStartChargeResult);
						type = AddrType_VinApplyStartCharge_Result;
					}else if(mapInfo.contains(Addr_ZigbeeStopChargeResult)){
						mapInfo[Addr_VINApplyStopChargeType_Result] = mapInfo[Addr_ZigbeeStopChargeResult];
						mapInfo.remove(Addr_ZigbeeStopChargeResult);
						type = AddrType_VinApplyStopCharge_Result;
					}
				}else if(cVinType == 0x03){//车牌号
					mapInfo[Addr_CarLicense] = arData;
					mapInfo.remove(Addr_ZigbeeUserID);
					if(mapInfo.contains(Addr_ZigbeeStartChargeResult)){
						mapInfo[Addr_CarLicenseApplyStartChargeType_Result] = mapInfo[Addr_ZigbeeStartChargeResult];
						mapInfo.remove(Addr_ZigbeeStartChargeResult);
						type = AddrType_CarLicenceApplyStartCharge_Result;
					}else if(mapInfo.contains(Addr_ZigbeeStopChargeResult)){
						mapInfo[Addr_CarLicenseApplyStopChargeType_Result] = mapInfo[Addr_ZigbeeStopChargeResult];
						mapInfo.remove(Addr_ZigbeeStopChargeResult);
						type = AddrType_CarLicenceApplyStopCharge_Result;
					}
				}
			}
			break;
		case CMD_CHANGE_PRIORITY: //设置充电优先等级
			type = AddrType_ChargePriority;
			break;
		case CMD_CHARGE_FINISH_TIME://设置充电完成时间
			type = AddrType_ChargeFinishTime;
			break;
		case CMD_CHARGE_MODE://设置充电模式			
			type = AddrType_ChargeMode;
			break;
		case CMD_MAX_LOAD://设置充电电网最大允许负荷
          {
			type = AddrType_MaxLoad;
            InfoMap::iterator itTarget=mapInfo.find(Addr_CanID_Comm);
            if(itTarget.value().at(0)>=ID_MinDCCanID && itTarget.value().at(0)<= ID_MaxDCCanID)
            {
                type = AddrType_LimitChargeCurrent;
                TerminalStatus stStatus;
                m_pDevCache->QueryTerminalStatus(itTarget.value().at(0), stStatus);
                 int fCurrent =*((int*)mapInfo[Addr_MaxLoad].data())/stStatus.stFrameRemoteMeSurement1.voltage_of_dc;
                 mapInfo.remove(Addr_MaxLoad);
                 mapInfo.insert(Addr_AdjustCurrent_Adj,QByteArray((char*)fCurrent,4));
            }
         }
			break;
		case CMD_GROUP_POLICY://设置群充策略  		
			type = AddrType_GroupPolicy;
			break;
		case CMD_AUXPOWER://设置辅助电源类型
			type = AddrType_AuxPower;
			break;
		case CMD_QUEUE_PUBLISH://下发充电排队信息
			type = AddrType_QueueInfo;
			break;
		case CMD_CHARGE_PREDICT_TIME://发充电预估完成时间
			type = AddrType_PredictTime_Result;
			break;
		case CMD_CHAGE_DETAIL://下发充电总费用和充电明细
			type = AddrType_ChargeDetail;
			break;
		case CMD_300KW_TERM_PARAM://发直流充电终端双系统300kw参数设置
			type = AddrType_DoubleSys300kwSetting_Publish;
			break;
		case CMD_RELAY_CONTROL://下发输出继电器控制指令
			type = AddrType_RelayControl_Publish;
			break;
		case CMD_CHARGEGUN_RESULT:
			type = AddrType_Response_Result;
			break;
         case CMD_AMMETER_DATA_SELECT:    //电表数据查询  add by zrx 2017-03-24
            if(mapInfo.contains(Addr_RemoteAmeterAddr_Adj))
                list.append(mapInfo.find(Addr_RemoteAmeterAddr_Adj));
            if(mapInfo.contains(Addr_RemoteAmeterType_Adj))
                list.append(mapInfo.find(Addr_RemoteAmeterType_Adj));
            if(mapInfo.contains(Addr_RemoteAmeterReadTime_Adj))
                list.append(mapInfo.find(Addr_RemoteAmeterReadTime_Adj));
            SendFrameI(list, CMD_AMMETER_DATA_SELECT, REASON_ACTIVE_ACK);
            type = AddrType_RemoteAmmeterReadType;
            list.clear();
            break;
		case CMD_REMOTE_EMERGENCY://平台下发紧急充电状态
			type = AddrType_RemoteEmergency_State;
			break;
		case CMD_REMOTE_EMERGENCY_RET://平台返回在线状态
			type = AddrType_RemoteEmergency_Result;
			break;
		case CMD_EMERGENCY_ENABLE://平台配置应急充电开关
			WriteEmergencyEnable(mapInfo);
			break;
		case CMD_EMERGENCY_SETTING://平台配置应急充电选项
			WriteEmergencySetting(mapInfo);
			break;
		case CMD_QUEUE_GROUP_INFO://轮充组信息
			WriteQueueGroupInfo(mapInfo);
			type = AddrType_QueueGroup_Info;
			break;
		case CMD_QUEUE_GROUP_STATE://排队信息带流水号
			type = AddrType_QueueGroup_State;
			break;
		case CMD_RESPONSE_RET:
			{
				if((mapInfo.contains(Addr_PeakShaving_Setting_Result)))
					type = AddrType_InPeakApplyCharge_ServerResult;
				else if((mapInfo.contains(Addr_FrozenEnergy_Result)))
					type = AddrType_Response_Result;

				break;
			}
        case CMD_CHARGEGUN_GROUP_INFO://多枪组信息
            WriteChargeGunGroupInfo(mapInfo);
            type = AddrType_ChargeGunGroup_Info;    //充电服务需要订阅
            break;

        default:
			WriteLog(QString("Command [0x%1] not processed").arg(pAsduHdr->cType, 2, 16, QChar('0')));
			return false;
	}
	return true;
}

/*
 * I帧组包并发送
 * map        	输入 信息体集合
 * cFrameType 	输入 命令标识
 * sReason    	输入 传送原因
 * iSerial		输入 大于零表示连续数据的数量，等于零表示非连续数据
 * 返回值		无
 */
void IEC104LocalServer::SendFrameI(QIteratorList& list, uchar cFrameType, ushort sReason, int iSerial)
{
	Frame_I stFrame;
	InfoMap::iterator it;
	QByteArray ar;
	int iOffset, iLen;

    if(!m_bConnected || !m_bActived)
        return;

	memset(&stFrame, 0, sizeof(Frame_I));
	stFrame.apci.cHead = IEC_FLAG;
	stFrame.apci.cCtrl1 = (m_sSendNo & (ushort)0x007F) << 1;
	stFrame.apci.cCtrl2 = (m_sSendNo & (ushort)(~0x007F)) >> 7;
	stFrame.apci.cCtrl3 = ((m_sRecvNo + 1) & (ushort)0x007F) << 1;
	stFrame.apci.cCtrl4 = ((m_sRecvNo + 1) & (ushort)(~0x007F)) >> 7;
	stFrame.asdu.hdr.cType = cFrameType;
	if(iSerial > 0)
		stFrame.asdu.hdr.cLimit = 0x80 + iSerial;
	else
		stFrame.asdu.hdr.cLimit = (uchar)list.count();
	stFrame.asdu.hdr.sReason = ___constant_swab16(sReason);
    AddBCDStationNo(m_strStationNo, stFrame.asdu.hdr.szStation);

	iOffset = 0;
    for(int i = 0; i < list.count(); i++){
        it = list.at(i); 

		memcpy(&stFrame.asdu.szData[iOffset], (char *)&it.key(), 3);
        iOffset += 3;

		if(it.value().length() > 0){
			memcpy(&stFrame.asdu.szData[iOffset], it.value().data(), it.value().length());
			iOffset += it.value().length();
		}
    }

	iLen = sizeof(APCI) + sizeof(ASDU_Hdr) + iOffset;
	stFrame.apci.cLen = iLen - 2;

	if(iLen > MAX_NET_PACKAGE_LEN){
		WriteLog(QString("CMD=%1 and daata length=%2 too long!").arg(int(cFrameType)).arg(iLen));
		return;
	}

	WriteLog((uchar *)&stFrame, iLen, FRAME_TYPE_I);

	SendEncryptData((uchar *)&stFrame, iLen);

	m_sSendNo++;
	if(m_sSendNo >= MAX_NO)
		m_sSendNo = 0;
}

/*
 * S帧组包并发送
 * 返回值	无
 */
void IEC104LocalServer::SendFrameS()
{
	APCI stApci;
	int iLen;

	if(!m_bConnected)
		return;

	memset(&stApci, 0, sizeof(APCI));
	stApci.cHead = IEC_FLAG;
	stApci.cCtrl1 = FRAME_TYPE_S;
	stApci.cCtrl3 = ((m_sRecvNo + 1) & (ushort)0x007F) << 1;
	stApci.cCtrl4 = ((m_sRecvNo + 1) & (ushort)(~0x007F)) >> 7;
	iLen = sizeof(APCI);
	stApci.cLen = iLen - 2;

	WriteLog((uchar *)&stApci, iLen, FRAME_TYPE_S);

    SendEncryptData((uchar *)&stApci, iLen);
}

/*
 * 加密数据并发送
 * pData 	输入 发送数据
 * iLen  	输入 数据长度
 * 返回值	无
 */
void IEC104LocalServer::SendEncryptData(uchar *pszData, int iLen)
{
	Frame_Net stFrame;
    int iEncrypt;

    if(iLen <= 0 || iLen > MAX_NET_PACKAGE_LEN)
        return;

	memset(&stFrame, 0, sizeof(Frame_Net));

	stFrame.hdr.cHead = NET_FLAG;
	stFrame.hdr.cType = 0x06;

    iEncrypt = data_convert(pszData, iLen, stFrame.szData, 1, m_sz3DesKey);
	if(iEncrypt < 0){
		WriteLog(QString("Encrypt frame data failed!"));
		return;
	}

	iLen = iEncrypt + sizeof(Net_Hdr);
	stFrame.hdr.iLen = ___constant_swab32(iLen - MIN_NET_PACKAGE_LEN);

    m_pSocket->write((char *)&stFrame, iLen);
}

/*
 * 解析数据中的信息体，将信息体保存至QMap
 * pszData 	输入 要解析的数据
 * iLen    	输入 数据长度
 * cLimit	输入 限定词（信息体数量)
 * mapInfo 	输出 信息体集合
 * 返回值  	解析无错误返回true，否则返回false
 */
bool IEC104LocalServer::ParseInfoTag(uchar* pszData, uint iLen, uchar cLimit, InfoMap& mapInfo)
{
	QByteArray arData;
	uint iInfoAddr, iDataLen, iOffset;
	uchar* p;
	uchar cCount = 0;

    for(p = pszData; p < pszData + iLen; ){
		iInfoAddr = 0;
		iOffset = 0;

		memcpy((char *)&iInfoAddr, p, 3);
		p = p + 3;

		switch(iInfoAddr){
			case Addr_CanID_Comm: 			//模块地址 
			case Addr_ChargeCmd_Ctrl:		//充电指令
			case Addr_CmdSrc_Ctrl:			//指令来源
			case Addr_ChargeStartReason_Ctrl://开始充电原因
			case Addr_AckResult_Ctrl:		//充电模块响应结果
			case Addr_ExecResult_Ctrl:		//充电模块执行结果
			case Addr_GUIDFlag_Ctrl:		//GUID产生标识
			case Addr_ZigbeeStopChargeResult://zigbee申请开始充电结果
			case Addr_ZigbeeStartChargeResult://zigbee申请结束充电结果
			case Addr_CarLock:				//车位锁开关
			case Addr_CardApplyChargeType:	//卡申请充电类型
			case Addr_CardApplyCharge_Result://刷卡申请充电返回结果
			case Addr_CardAccountType:		//刷卡业务查询账户信息类型
			case Addr_CardStopCharge_Result://刷卡申请终止充电返回结果
            case Addr_ScanCode_Charge_Type:	//扫码申请充电类型
            case Addr_ScanCode_StartCharge_Result://扫码申请充电返回结果
            case Addr_ScanCode_Type:		//扫码业务查询账户信息类型
            case Addr_ScanCode_StopCharge_Result://扫码申请终止充电返回结果
			case Addr_ChargePriority:		//设置充电优先等级
			case Addr_ChargeMode:			//设置充电模式
			case Addr_300kw_Group:			//组别设置
			case Addr_300kw_PairCanID:		//配对的充电终端的CAN地址
			case Addr_300kw_WorkMode:		//工作模式设置
			case Addr_300kw_ChangeMode:		//单充/双充切换模式类型
			case Addr_Relay_ID:				//输出继电器编号
            case Addr_Relay_CmdType:		//单充/双充切换模式类型
            case Addr_Relay_HoldType:
            case Addr_AckResult_Ctr_Peak:
            case Addr_AckResultFaile_Ctr_Peak:
            case Addr_DCChargeManner_Term://双枪充电模式信息
            case Addr_DCChargeMannerSetResult_Term:
            case Addr_DCChargeMasterCANID_Term:
            case Addr_DCChargeSlaveCANID_Term:
            case Addr_AmeterEnable_Adj:             //第一块电表使能
            case Addr_RemoteAmeterType_Adj:  //电表数据类型地址
            case Addr_AmeterProtocol_Adj:         //第一块电表协议类型
            case Addr_AmeterData_Adj:               //第一块电表数据类型
			case Addr_Remote_Emergency:		//平台紧急充电状态
			case Addr_Emergency_Enable:		//平台设置紧急充电开关
			case Addr_Emergency_Duration: 	//应急充电使用时间
			case Addr_Emergency_VinAuth:	//应急充电VIN鉴权
			case Addr_Emergency_CarAuth:	//应急充电车牌号鉴权
			case Addr_Emergency_CardAuth:	//应急充电卡号鉴权
			case Addr_Emergency_QueueGun:	//应急充电插枪触发轮充
			case Addr_Emergency_QueueCard:	//应急充电刷卡触发轮充
			case Addr_Emergency_QueueCar:	//应急充电车辆信息触发轮充
            case Addr_CarLockID:    //车位锁地址
            case Addr_CarLockCmd://车位锁指令
            case Addr_FrozenEnergy_Result:	//冻结电量结果
            case Addr_ChargeGunType:   //多枪充电相关
            case Addr_ChargeGun_Master:
            case Addr_ChargeGun_Slave1:
            case Addr_ChargeGun_Slave2:
            case Addr_ChargeGun_Slave3:
            case Addr_ChargeGun_Slave4:
            case Addr_ChargeGun_Slave5:
            case Addr_ChargeGun_Slave6:
            case Addr_ChargeGunType_Reault:
				iDataLen = 1;
				break;
			case Addr_AdjustCurrent_Adj:	//被调整充电电流的大小
			case Addr_300kw_CanID:			//CAN地址
			case Addr_Emergency_OrderCount:	//应急充电订单数
			case Addr_Emergency_CheckTime: 	//应急充电断网检测时间
            case Addr_AmeterVoltageRate_Adj:   //第一块电表电压变比
            case Addr_AmeterCurrentRate_Adj:   //第一块电表电流变比
            case Addr_CarLockParaSetCmd://车位锁参数设置
				iDataLen = 2;
				break;
			case Addr_CardChargeTypeValue:	//充电类型值
			case Addr_MaxLoad:				//设置电网最大允许负荷
				iDataLen = 4;
				break;
            case Addr_AmeterAddr_Adj:    //第一块电表地址
            case Addr_RemoteAmeterAddr_Adj:   //电表地址
            case Addr_RemoteAmeterReadTime_Adj:   //电表数据抄读时间
                iDataLen = 6;
                break;
			case Addr_EventOccurTime_Ctrl:	//事件产生时间
			case Addr_EventSendTime_Ctrl:	//报文发送时间
				iDataLen = 7;
				break;
			case Addr_OrderNumber_Ctrl:		//流水号
				iDataLen = 9;
				break;
			case Addr_CmdNum_Ctrl:			//指令编号
			case Addr_GUID_Ctrl:			//GUID
				iDataLen = 16;
				break;
			case Addr_ZigbeeUserID:			//zigbee用户id
				iDataLen = 24;
				break;
			case Addr_CardAccount:			//刷卡卡号
				iDataLen = *p;
				p = p + 1;
				iOffset = 10;
				break;
        case Addr_ScanCode_customerID:			//扫码客户ID
            iDataLen = *p;
            p = p + 1;
            iOffset = 16;
            break;
			case Addr_CardUserName:			//刷卡用户姓名
			case Addr_CardAccountList:		//刷卡帐户列表及余额
			case Addr_CardPolicy:			//刷卡收费策略
			case Addr_CardChargeInfo:		//刷卡用户当前充电信息
			case Addr_CSCULogUploadPath_CtrlInfo:		//日志文件上传路径
			case Addr_CSCUConfigUploadPath_CtrlInfo:	//配置文件上传路径
			case Addr_CSCUConfigUpdatePath_CtrlInfo:	//配置文件下载路径
			case Addr_CSCUProgramUpdatePath_CtrlInfo:	//主程序升级文件路径
			case Addr_ChargerProgramUpdatePath_CtrlInfo://充电模块配置文件升级路径
			case Addr_TermName_Adj:			//获取子站名称
			case Addr_TermIndex_Adj:		//CAN地址和终端编号对应关系
			case Addr_Default_Comm:			//时间对时
			case Addr_ChargeFinishTime:		//充电完成时间
			case Addr_PredictTime:			//充电预估完成时间
			case AddrType_ChargeDetail:		//充电总费用和充电明细
			case Addr_300kw_ChangeTime:		//单充/双充自动切换时间段设置
			case Addr_QueueInfo_Publish:	//下发充电排队信息
			case Addr_QueueGroup_Info:		//轮充组信息
			case Addr_Emergency_Queue:		//应急充电排队信息
			case Addr_First_Server:			//服务器列表第一个配置
			case Addr_Second_Server:		//服务器列表第二个配置
			case Addr_Third_Server:			//服务器列表第二个配置
            case Addr_Group_ChargeGun: //下发多枪分组信息
				iDataLen = pszData + iLen - p;
				break;
			default:
				iDataLen = pszData + iLen - p;
				break;
		}
		
        if(iInfoAddr == Addr_ScanCode_customerID)
            arData = QByteArray::fromRawData((const char *)p,iDataLen).toHex().data();
        else
            arData.append((char *)p, iDataLen);
        mapInfo[iInfoAddr] = arData;

		if(iOffset > 0)
			p = p + iOffset;
		else
			p = p + iDataLen;

		cCount++;
		arData.clear();
	}


	Q_UNUSED(cLimit);

	/*
	if(cCount != (cLimit & 0x7F)){
		WriteLog(QString("Invalid infoAddr count Limit=%1, RealCount=%2").arg(int(cLimit)).arg(int(cCount)));
		return false;
	}
	*/

	return true;
}

/*
 * 104数据帧解析入口
 * pszData	输入 104数据
 * iLen		输入 数据长度
 * mapInfo	输出 信息体集合
 * type		输入 消息类型
 * 返回值	成功返回true，失败返回fasle
 */
bool IEC104LocalServer::Parse104Frame(uchar* pszData, int iLen, InfoMap& mapInfo, InfoAddrType& type)
{
	APCI* pApci;
	uchar szBuff[512];
	int iDecrypt = 0;

	if(!pszData || iLen <= 0)
		return false;

    iDecrypt = data_convert(pszData, iLen, szBuff, 0, m_sz3DesKey);
	if(iDecrypt <= 0){
		WriteLog(QString("Decrypt frame failed!"));
		return false;
	}

	pApci = (APCI *)szBuff;
	iLen = pApci->cLen + 2;

	if(iDecrypt < iLen){
		WriteLog(QString("Decrypt frame data invalid!"));
		return false;
	}

	if(pApci->cCtrl1 == FRAME_TYPE_S){
		int iSendNo = (pApci->cCtrl3 | (pApci->cCtrl4 << 8)) >> 1;
		if((m_iRebootNo + 1) == iSendNo){
			m_iRebootNo = -1;
			m_pSetting->setRebootFlag(2);
			WriteLog(QString("Set reboot flag!"));
		}
		WriteLog(szBuff, iLen, FRAME_TYPE_S, false);
		return true;
	}

	if((pApci->cCtrl1 & FRAME_TYPE_U) == FRAME_TYPE_U){
		WriteLog(szBuff, iLen, FRAME_TYPE_U, false);
		return ParseFrameU(szBuff, iLen);
	}
	else if((pApci->cCtrl1 & 0x01) == FRAME_TYPE_I){
		if(m_bConnected && m_bActived){
			WriteLog(szBuff, iLen, FRAME_TYPE_I, false);

			SendFrameS();
			return ParseFrameI(szBuff, iLen, mapInfo, type);		
		}
	}

	return false;
}

/*
 * 总召唤
 * type 	输入 召唤类型
 * 返回值	无
 */
void IEC104LocalServer::SendQueryAll(QueryType type)
{
    QIteratorList list;
	InfoMap mapInfo;
	QByteArray arData;
	uchar cCmd = 0;
	int iTotalAddr = 0, iInfoAddr, iBeginAddr = 1;
	int iPackIndex = 0;
	bool bInvalid = true;

	for(int i = 0; i < 3; i++){
		for(uchar cCanAddr = m_cCanRange[i][0]; cCanAddr < m_cCanRange[i][1]; cCanAddr++){
			switch(type){
				case QUERY_MEASURE2:
					bInvalid = QueryMeasure2(cCanAddr, iInfoAddr, arData, true);
					iTotalAddr = MEASURE2_PACKAGE_NUM;
					cCmd = 0x0F;
					break;
            case QUERY_MEASURE3:
            {
                TerminalStatus stStatus;
                memset(&stStatus, 0, sizeof(TerminalStatus));

                if(m_pDevCache)
                    m_pDevCache->QueryTerminalStatus(cCanAddr, stStatus);

                    if(stStatus.stFrameRemoteSingle.charge_status == 8)
                    {
                        bInvalid = QueryMeasure2(cCanAddr, iInfoAddr, arData, false);
                        iTotalAddr = MEASURE2_PACKAGE_NUM;
                        cCmd = 0x0F;
                    }
                    else
                        continue;
            }
                break;
				case QUERY_MEASURE:
					bInvalid = QueryMeasure(cCanAddr, iInfoAddr, arData);
					iTotalAddr = MEASURE_PACKAGE_NUM;
					cCmd = 0x0B;
					break;
				case QUERY_SIGNAL:
					bInvalid = QuerySignal(cCanAddr, iInfoAddr, arData);
					iTotalAddr = SIGNAL_PACKAGE_NUM;
					cCmd = 0x01;
					break;
            case QUERY_BATTERYStatus://能效计划临时用
            {
                if(cCanAddr == ID_MinDCCanID)
                {

				//++++++++++++++++++++++DHT 2018-03-09
                //bInvalid = QueryBatteryStatus(cCanAddr, iInfoAddr, arData);
                bInvalid = true;
    			iInfoAddr = 0xDA000A;
				//---------------------DHT
                iTotalAddr = BatteryStatus_PACKAGE_NUM;
                cCmd = 0x03;
                }
            }
                break;
            case QUERY_BATTERYEleInfo:
            {
                if(cCanAddr == ID_MinDCCanID)
                {
				//++++++++++++++++++++++DHT 2018-03-09
                //bInvalid = QueryBatteryEleInfo(cCanAddr, iInfoAddr, arData);
                bInvalid = true;
    			iInfoAddr = 0xDA0001;
				//----------------------DHT
                iTotalAddr = BatteryEleInfo_PACKAGE_NUM;
                cCmd = 0x04;
                }
            }
                break;
            case QUERY_PowerOptimizerInfo://功率优化器信息
            {
                if(cCanAddr == ID_MinDCCanID)
                {
                bInvalid = QueryPowerOptimizerStatus(cCanAddr, iInfoAddr, arData);
                iTotalAddr = PowerOptimizerInfo_PACKAGE_NUM;
                cCmd = 0x05;
                }
            }
                break;
				default:
					return;
			}
			
			if(bInvalid){
				iPackIndex++;
				if(iPackIndex == 1)
					iBeginAddr = iInfoAddr;	
			}

			//1、当有无效数据时，且arData有数据，说明数据不再连续，需进行打包
			//2、当打包数量达到最大时，需进行打包
			//3、当can地址到达最大时，且此时有数据，需进行打包
			if((!bInvalid && arData.length() > 0) ||
					(iPackIndex >= iTotalAddr) || 
					(iPackIndex > 0 && cCanAddr == m_cCanRange[i][1] - 1)){
                list.append(mapInfo.insert(iBeginAddr, arData));
				SendFrameI(list, cCmd, REASON_QUERY, iPackIndex);
				arData.clear();
				mapInfo.clear();
                list.clear();
				iPackIndex= 0;

                if(type == QUERY_BATTERYStatus || type==QUERY_BATTERYEleInfo)
                {
                    return;
                }
			}
		}
	}
}

/*
 * 从缓存模块获取遥信数据
 * cCanAddr		输入 模块地址
 * iInfoAddr	输出 模块地址对应的信息体
 * arData		输出 遥信数据
 * 返回值		有效数据返回true，无效返回false
 */
bool IEC104LocalServer::QuerySignal(uchar cCanAddr, int& iInfoAddr, QByteArray& arData)
{
	TerminalStatus stStatus;
	memset(&stStatus, 0, sizeof(TerminalStatus));

	if(m_pDevCache)
		m_pDevCache->QueryTerminalStatus(cCanAddr, stStatus);

	iInfoAddr = ___signal_infoaddr(cCanAddr);

	//充电接口标识
	arData.append(stStatus.stFrameRemoteSingle.charge_interface_type);
	//连接确认开关状态
	arData.append(stStatus.stFrameRemoteSingle.link_status);
	//输出继电器状态 0断开/1半连接/2连接/3其他
	arData.append(stStatus.stFrameRemoteSingle.relay_status);
	//停车位 0空闲/1占用/2其他
	arData.append(stStatus.stFrameRemoteSingle.parking_space);
	//充电工作状态 0待机/1工作/2故障/3启动中/4暂停/5限制/6离线
    if(stStatus.gunType == SLAVE_GUN)
    {//副枪
        stStatus.stFrameRemoteSingle.charge_status = CHARGE_STATUS_REALTIME_SLAVE;
    }
	arData.append(stStatus.stFrameRemoteSingle.charge_status);
	//故障状态	1欠压故障/2过压故障/3过流故障
    //临时
    stStatus.stFrameRemoteSingle.status_fault = 0;
	arData.append(stStatus.stFrameRemoteSingle.status_fault);
	//BMS故障信息
	arData.append(stStatus.stFrameRemoteSingle.BMS_fault);
	//终止充电原因
	arData.append(stStatus.stFrameRemoteSingle.Stop_Result);
	//控制模式，群充策略
	arData.append(stStatus.stFrameRemoteSingle.QunLunCeLue);
	//辅助电源类型
	arData.append(stStatus.stFrameRemoteSingle.AuxPowerType);
	
    return true;
}


/*
 * 从缓存模块获取遥信数据
 * cCanAddr		输入 模块地址
 * iInfoAddr	输出 模块地址对应的信息体
 * arData		输出 遥信数据
 * 返回值		有效数据返回true，无效返回false
 */
/*
bool IEC104LocalServer::QueryBatteryStatus(uchar cCanAddr, int& iInfoAddr, QByteArray& arData)
{
    iInfoAddr = 0xDA000A;

    QMap<unsigned int, QByteArray>::iterator it;
    //char tmpValue[2]={0};
    unsigned int key;

    for(it = batteryInfoMap.begin(); it != batteryInfoMap.end(); ++it)
    {
        key = it.key();
//        if(key>=Addr_chargeAllow && key <= Addr_runStatus)
//        {
//            memcpy(tmpValue, it.value().data(), it.value().size());
//            arData.append(tmpValue,1);
//        }
    }

    return true;
}
*/

/*
bool IEC104LocalServer::QueryBatteryEleInfo(uchar cCanAddr, int& iInfoAddr, QByteArray& arData)
{
    //uchar cQuality = 0;

    //char tmpValue[2]={0};
    iInfoAddr = 0xDA0001;

    unsigned int key;
     QMap<unsigned int, QByteArray>::iterator it;
    for(it = batteryInfoMap.begin(); it != batteryInfoMap.end(); ++it)
    {
        key = it.key();
//        if(key>=Addr_batteryID && key <= Addr_SOH)
//        {
//            memcpy(tmpValue, it.value().data(), it.value().size());
//            arData.append(tmpValue,2);
//            arData.append(cQuality);
//        }
    }

    return true;
}
*/

bool IEC104LocalServer::QueryPowerOptimizerStatus(uchar cCanAddr, int& iInfoAddr, QByteArray& arData)
{
    iInfoAddr = 0xDC0001;

    QMap<unsigned int, QByteArray>::iterator it;
    char tmpValue[4]={0};
    unsigned int key;

	Q_UNUSED(cCanAddr);

    for(it = PowerOptimizerInfoMap.begin(); it != PowerOptimizerInfoMap.end(); ++it)
    {
        key = it.key();
        if((key>=Addr_PowerOptimizerID && key <= Addr_radiatorTemp) ||
                (key>=Addr_combinerStatus && key<=Addr_softVer_H) ||
                (key>=Addr_inVol5 && key<=Addr_inVol8))//Addr_fault1_bit0    Addr_warning_bit15
        {
            memcpy(tmpValue, it.value().data(), it.value().size());
            arData.append(tmpValue,2);
        }
        if((key>=Addr_fault1_bit0 && key <= Addr_warning_bit15) ||
                (key>=Addr_sysRequestStatus_bit0 && key<=Addr_reserve2))//
        {
            memcpy(tmpValue, it.value().data(), it.value().size());
            arData.append(tmpValue,1);
        }
    }

    return true;
}

/*
 * 从缓存中获取遥测数据，根据偏移和精度进行转换
 * cCanAddr	输入 模块地址
 * iInfoAddr输出 模块地址对应的信息体
 * arData	输出 遥信数据
 * 返回值	有效数据返回true，无效返回false
 */
bool IEC104LocalServer::QueryMeasure(uchar cCanAddr, int& iInfoAddr, QByteArray& arData)
{
	TerminalStatus stStatus;
	QByteArray ar;
	uchar cQuality = 0;
	ushort sMeasure = 0;

	memset(&stStatus, 0, sizeof(TerminalStatus));

	if(m_pDevCache)
		m_pDevCache->QueryTerminalStatus(cCanAddr, stStatus);

	iInfoAddr = ___measure16_infoaddr(cCanAddr);

	//A相充电电压
	sMeasure = ___measure16(stStatus.stFrameRemoteMeSurement1.A_voltage, 0, 0.1);
	arData.append((char *)&sMeasure, 2);
	arData.append(cQuality);
	//B相充电电压
	sMeasure = ___measure16(stStatus.stFrameRemoteMeSurement1.B_voltage, 0, 0.1);
	arData.append((char *)&sMeasure, 2);
	arData.append(cQuality);
	//C相充电电压
	sMeasure = ___measure16(stStatus.stFrameRemoteMeSurement1.C_voltage, 0, 0.1);
	arData.append((char *)&sMeasure, 2);
	arData.append(cQuality);
	//A相充电电流
	sMeasure = ___measure16(stStatus.stFrameRemoteMeSurement1.A_current, -400, 0.1);
	arData.append((char *)&sMeasure, 2);
	arData.append(cQuality);
	//B相充电电流
	sMeasure = ___measure16(stStatus.stFrameRemoteMeSurement1.B_current, -400, 0.1);
	arData.append((char *)&sMeasure, 2);
	arData.append(cQuality);
	//C相充电电流
	sMeasure = ___measure16(stStatus.stFrameRemoteMeSurement1.C_current, -400, 0.1);
	arData.append((char *)&sMeasure, 2);
	arData.append(cQuality);
	//总有功功率
	sMeasure = ___measure16(stStatus.stFrameRemoteMeSurement1.active_power, 0, 0.01);
	arData.append((char *)&sMeasure, 2);
	arData.append(cQuality);
	//总无功功率
	sMeasure = ___measure16(stStatus.stFrameRemoteMeSurement1.reactive_power, 0, 0.01);
	arData.append((char *)&sMeasure, 2);
	arData.append(cQuality);
	//总功率因数
	sMeasure = ___measure16(stStatus.stFrameRemoteMeSurement1.power_factor, -1, 0.001);
	arData.append((char *)&sMeasure, 2);
	arData.append(cQuality);
	//零线电流
	sMeasure = ___measure16(stStatus.stFrameRemoteMeSurement1.neutralLine_current, -400, 0.1);
	arData.append((char *)&sMeasure, 2);
	arData.append(cQuality);
	//电压不平衡率
	sMeasure = ___measure16(stStatus.stFrameRemoteMeSurement1.voltage_unbalance_rate, 0, 0.1);
	arData.append((char *)&sMeasure, 2);
	arData.append(cQuality);
 	//电流不平衡率
	sMeasure = 0;
	sMeasure = ___measure16(stStatus.stFrameRemoteMeSurement1.current_unbalance_rate, 0, 0.1);
	arData.append((char *)&sMeasure, 2);
	arData.append(cQuality);
	//直流侧电压
	sMeasure = ___measure16(stStatus.stFrameRemoteMeSurement1.voltage_of_dc, 0, 0.1);
	arData.append((char *)&sMeasure, 2);
	arData.append(cQuality);
	//直流侧电流
	sMeasure = ___measure16(stStatus.stFrameRemoteMeSurement1.current_of_dc, -400, 0.1);
	arData.append((char *)&sMeasure, 2);
	arData.append(cQuality);
	//预留信息
	ar.fill('\0', 18);
	arData.append(ar);

    return true;
}

/*
 * 从缓存中获取遥测电度数，根据偏移和精度进行转换。总召唤时只获取
 * 单一方向的数据，即只获取充电或放电数据，保持数据一致性。
 * cCanAddr	输入 模块地址
 * iInfoAddr输出 模块地址对应的信息体
 * arData	输出 遥信数据
 * bCharge	输入 true表示获取充电电能，false表示根据实际情况获取充放电电能，该状态仅用于总召唤，即
 * 				 总召唤只召唤充电时的电能。
 * 返回值	有效数据返回true，无效返回false
 */
bool IEC104LocalServer::QueryMeasure2(uchar cCanAddr, int& iInfoAddr, QByteArray& arData, bool bCharge)
{
	TerminalStatus stStatus;
	int iMeasure2 = 0;
	uchar cQuality = 0; 

	memset(&stStatus, 0, sizeof(TerminalStatus));

	if(m_pDevCache)
		m_pDevCache->QueryTerminalStatus(cCanAddr, stStatus);

    if(bCharge){
        //总有功电能（充电）
        iInfoAddr = ___measure32_infoaddr(stStatus.cCanAddr);
        iMeasure2 = stStatus.stFrameRemoteMeSurement2.active_electric_energy;
        arData.append((char *)&iMeasure2, 4);
        arData.append(cQuality);
        //总无功电能
        iMeasure2 = stStatus.stFrameRemoteMeSurement2.reactive_electric_energy;
        arData.append((char *)&iMeasure2, 4);
        arData.append(cQuality);

    }else{
        //反向总有功电能（放电）
        iInfoAddr = ___measure32_infoaddr1(stStatus.cCanAddr);
        iMeasure2 = stStatus.stFrameRemoteMeSurement2.ReverseActiveEnergy;
        arData.append((char *)&iMeasure2, 4);
        arData.append(cQuality);
        //反向总无功电能
        iMeasure2 = stStatus.stFrameRemoteMeSurement2.ReverseReactiveEnergy;
        arData.append((char *)&iMeasure2, 4);
        arData.append(cQuality);

    }

    return true;
}

/*
 * 召唤（突发）模块遥信
 * cCanAddr 输入 模块地址
 * bBurst	输入 是否突发
 * 返回值	无
 */
void IEC104LocalServer::SendSignal(uchar cCanAddr, bool bBurst)
{
    QIteratorList list;
	InfoMap map;
	QByteArray arData;
	int iInfoAddr; 

	QuerySignal(cCanAddr, iInfoAddr, arData);

    list.append(map.insert(iInfoAddr, arData));

    SendFrameI(list, CMD_SINGLE_POINT, bBurst ? REASON_BURST : REASON_QUERY);
}

/*DHT Burst 定义需修改
 * 召唤模块遥测数据
 * cCanAddr 输入 模块地址
 * 返回值	无
 */
void IEC104LocalServer::SendMeasure(uchar cCanAddr)
{
    QIteratorList list;
	InfoMap map;
	QByteArray arData;
	int iInfoAddr;

	QueryMeasure(cCanAddr, iInfoAddr, arData);

    list.append(map.insert(iInfoAddr, arData));

    SendFrameI(list, CMD_MEASURE, REASON_QUERY);
}

/*
 * 召唤（突发）模块遥测电度数
 * cCanAddr 输入 模块地址
 * bBurst	输入 是否突发
 * 返回值	无
 */
void IEC104LocalServer::SendMeasure2(uchar cCanAddr, bool bBurst)
{
    QIteratorList list;
	InfoMap map;
	QByteArray arData;
	int iInfoAddr;

    TerminalStatus stStatus;
    memset(&stStatus, 0, sizeof(TerminalStatus));
    if(m_pDevCache)
        m_pDevCache->QueryTerminalStatus(cCanAddr, stStatus);

        if(stStatus.stFrameRemoteSingle.charge_status == 8)
            QueryMeasure2(cCanAddr, iInfoAddr, arData,false);//正向电能
        else
            QueryMeasure2(cCanAddr, iInfoAddr, arData,true);//正向电能

    list.append(map.insert(iInfoAddr, arData));
    SendFrameI(list, CMD_MEASURE2, bBurst ? REASON_BURST : REASON_QUERY);

    if(m_cServerType == SERVER_LOCAL)
    {
        list.clear();
        map.clear();
        iInfoAddr = 0;
        arData.clear();
        QueryMeasure2(cCanAddr, iInfoAddr, arData,false);//反向电能
        list.append(map.insert(iInfoAddr, arData));
        SendFrameI(list, CMD_MEASURE2, bBurst ? REASON_BURST : REASON_QUERY);
    }
}

/*
 * 检测充电机或继电器的状态变化，如果变化则需要突发遥测电度数
 * mapInfo 输入 旧状态信息体集合
 * 返回值  需要突发返回true，不需要返回false
 */
bool IEC104LocalServer::IsBurstMeasure2(InfoMap& mapInfo)
{
	TerminalStatus stStatus;
	memset(&stStatus, 0, sizeof(TerminalStatus));

	if(!mapInfo.contains(Addr_CanID_Comm))
		return false;

	//获取新状态
	if(m_pDevCache)
		m_pDevCache->QueryTerminalStatus(mapInfo[Addr_CanID_Comm].at(0), stStatus);

	//新老状态比较
	if(mapInfo.contains(Addr_WorkState_Term) &&
			mapInfo[Addr_WorkState_Term].at(0) != stStatus.stFrameRemoteSingle.charge_status)
		return true;

	if(mapInfo.contains(Addr_RelyState_Term) &&
			mapInfo[Addr_RelyState_Term].at(0) != stStatus.stFrameRemoteSingle.relay_status)
		return true;

	return false;
}

/*
 * 召唤BMS数据
 * cCanAddr 输入 模块地址
 * 返回值	无
 */
void IEC104LocalServer::SendBMS(uchar cCanAddr)
{
    QIteratorList list;
	InfoMap map;
	QByteArray ar, arData;
	int iInfoAddr;
	ushort sMeasure;
	uchar cQuality = 0;
	TerminalStatus stStatus;

	memset(&stStatus, 0, sizeof(TerminalStatus));

	if(m_pDevCache)
		m_pDevCache->QueryTerminalStatus(cCanAddr, stStatus);

	//需求电压
	sMeasure = ___measure16(stStatus.stFrameBmsInfo.BMS_need_voltage, 0, 0.1);
	arData.append((char *)&sMeasure, 2);
	arData.append(cQuality);
	//需求电流
	sMeasure = ___measure16(stStatus.stFrameBmsInfo.BMS_need_current, -400, 0.1);
	arData.append((char *)&sMeasure, 2);
	arData.append(cQuality);
	//当前SOC
	sMeasure = stStatus.stFrameBmsInfo.batery_SOC;
	arData.append((char *)&sMeasure, 2);
	arData.append(cQuality);
	//最高电池温度
	sMeasure = ___measure16(stStatus.stFrameBmsInfo.max_batery_temperature, -50, 1);
	arData.append((char *)&sMeasure, 2);
	arData.append(cQuality);
	//最高电池电压
	sMeasure = ___measure16(stStatus.stFrameBmsInfo.max_batery_voltage, 0, 0.01);
	arData.append((char *)&sMeasure, 2);
	arData.append(cQuality);
	//最低电池温度
	sMeasure = ___measure16(stStatus.stFrameBmsInfo.lowest_battery_temperature, -50, 1);
	arData.append((char *)&sMeasure, 2);
	arData.append(cQuality);
	//最低电池电压
	sMeasure = ___measure16(stStatus.stFrameBmsInfo.lowest_charge_voltage, 0, 0.01);
	arData.append((char *)&sMeasure, 2);
	arData.append(cQuality);
	//预留信息
	ar.fill('\0', 12);
	arData.append(ar);

	iInfoAddr = ___bms_infoaddr(cCanAddr);

    list.append(map.insert(iInfoAddr, arData));

    SendFrameI(list, CMD_BMS, REASON_QUERY);
}

/*
 * 召唤子站高压侧数据
 * 返回值	无
 */
void IEC104LocalServer::SendHighVolate()
{
    QIteratorList list;
	InfoMap map;
	QByteArray ar, arData;
	int iMeasure;
	uchar cQuality = 0;
	int iInfoAddr = 0;

	stAmmeterConfig a;
	stAllAmmeterConfig conf;
	stAmmeterData stAmmeter;
	m_pSetting->querySetting(&conf, PARAM_AMMETER);

	for(int i = 0; i < conf.ammeterConfig.count(); i++){
		arData.clear();
		iMeasure  = 0;
        list.clear();

		memset(&stAmmeter, 0, sizeof(stAmmeterData));

		a = conf.ammeterConfig.at(i);
		
		QVariant key, value;
		key.setValue(QByteArray((char *)a.addr, 6));
		m_pDevCache->QueryRealStatusMeter(value, CACHE_INLINE_AMMETER, key);
		stAmmeter = value.value<stAmmeterData>(); 

        //校准电流与功率符号
        if(stAmmeter.TotalPower<0)
        {
            if(stAmmeter.Cur_A>0)
                stAmmeter.Cur_A = stAmmeter.Cur_A * (-1);
            if(stAmmeter.Cur_B>0)
                stAmmeter.Cur_B = stAmmeter.Cur_B * (-1);
            if(stAmmeter.Cur_C>0)
                stAmmeter.Cur_C = stAmmeter.Cur_C * (-1);
        }

		//电表信息体地址
		iInfoAddr = Addr_AmeterAddr_Adj + i;
		//电表地址
		arData.append((char *)a.addr, 6);
		list.append(map.insert(iInfoAddr, arData));

		//电表数据
		arData.clear();
		iInfoAddr = Addr_AmeterInfo_Adj + (i * 0x100);
		//高压侧A相电压
		iMeasure = ___measure32(stAmmeter.Vol_A, 0, 0.1);
		arData.append((char *)&iMeasure, 4);
		arData.append(cQuality);
		//高压侧B相电压
		iMeasure = ___measure32(stAmmeter.Vol_B, 0, 0.1);
		arData.append((char *)&iMeasure, 4);
		arData.append(cQuality);
		//高压侧C相电压
		iMeasure = ___measure32(stAmmeter.Vol_C, 0, 0.1);
		arData.append((char *)&iMeasure, 4);
		arData.append(cQuality);
		//高压侧A相电流
		iMeasure = ___measure32(stAmmeter.Cur_A, 0, 0.01);
		arData.append((char *)&iMeasure, 4);
		arData.append(cQuality);
		//高压侧B相电流
		iMeasure = ___measure32(stAmmeter.Cur_B, 0, 0.01);
		arData.append((char *)&iMeasure, 4);
		arData.append(cQuality);
		//高压侧C相电流
		iMeasure = ___measure32(stAmmeter.Cur_C, 0, 0.01);
		arData.append((char *)&iMeasure, 4);
		arData.append(cQuality);
		//总有功功率
		iMeasure = ___measure32(stAmmeter.TotalPower, 0, 0.0001);
		arData.append((char *)&iMeasure, 4);
		arData.append(cQuality);
		//总无功功率
		iMeasure = ___measure32(stAmmeter.TotalRePower, 0, 0.0001);
		arData.append((char *)&iMeasure, 4);
		arData.append(cQuality);
		//总功率因数
		iMeasure = ___measure32(stAmmeter.PowerFactor, 0, 0.001);
		arData.append((char *)&iMeasure, 4);
		arData.append(cQuality);
		//零线电流
		iMeasure = ___measure32(stAmmeter.Cur_0, 0, 0.001);
		arData.append((char *)&iMeasure, 4);
		arData.append(cQuality);
		//电压不平衡率（待定）
		iMeasure = ___measure32(stAmmeter.VolUnbalance, 0, 0.1);
		//iMeasure = 0;
		arData.append((char *)&iMeasure, 4);
		arData.append(cQuality);
 		//电流不平衡率（待定）
		iMeasure = ___measure32(stAmmeter.CurUnbalance, 0, 0.1);
		//iMeasure = 0;
		arData.append((char *)&iMeasure, 4);
		arData.append(cQuality);
		//谐波畸变率（待定）
		iMeasure = ___measure32(stAmmeter.HarmDistortion, 0, 0.1);
		//iMeasure = 0;
		arData.append((char *)&iMeasure, 4);
		arData.append(cQuality);
		//A相有功功率
		iMeasure = ___measure32(stAmmeter.Power_A, 0, 0.0001);
		arData.append((char *)&iMeasure, 4);
		arData.append(cQuality);
		//B相有功功率
		iMeasure = ___measure32(stAmmeter.Power_B, 0, 0.0001);
		arData.append((char *)&iMeasure, 4);
		arData.append(cQuality);
		//C相有功功率
		iMeasure = ___measure32(stAmmeter.Power_C, 0, 0.0001);
		arData.append((char *)&iMeasure, 4);
		arData.append(cQuality);
		//A相无功功率
		iMeasure = ___measure32(stAmmeter.RePower_A, 0, 0.0001);
		arData.append((char *)&iMeasure, 4);
		arData.append(cQuality);
		//B相无功功率
		iMeasure = ___measure32(stAmmeter.RePower_B, 0, 0.0001);
		arData.append((char *)&iMeasure, 4);
		arData.append(cQuality);
		//C相无功功率
		iMeasure = ___measure32(stAmmeter.RePower_C, 80, 0.0001);
		arData.append((char *)&iMeasure, 4);
		arData.append(cQuality);
		//A相功率因数
		iMeasure = ___measure32(stAmmeter.PowerFactor_A, 0, 0.001);
		arData.append((char *)&iMeasure, 4);
		arData.append(cQuality);
		//B相功率因数
		iMeasure = ___measure32(stAmmeter.PowerFactor_B, 0, 0.001);
		arData.append((char *)&iMeasure, 4);
		arData.append(cQuality);
		//C相功率因数
		iMeasure = ___measure32(stAmmeter.PowerFactor_C, 0, 0.001);
		arData.append((char *)&iMeasure, 4);
		arData.append(cQuality);
		//总正向有功电能
		iMeasure = stAmmeter.ActiveAbsorbEnergy;
		arData.append((char *)&iMeasure, 4);
		arData.append(cQuality);
		//总反向有功电能
		iMeasure = stAmmeter.ActiveLiberateEnergy;
		arData.append((char *)&iMeasure, 4);
		arData.append(cQuality);
		//感性无功电能
		iMeasure = stAmmeter.ReactiveSensibilityEnergy;
		arData.append((char *)&iMeasure, 4);
		arData.append(cQuality);
		//容性无功电能
		iMeasure = stAmmeter.ReactiveCapacityEnergy;
		arData.append((char *)&iMeasure, 4);
		arData.append(cQuality);
		//预留
		ar.fill('\0', 45);
		arData.append(ar);
        list.append(map.insert(iInfoAddr, arData));

		SendFrameI(list, CMD_MEASURE, REASON_QUERY);
	}
}

///
/// \brief SendAmmeterDataType
/// \param cEnergyDataType
///召唤电表数据   add by zrx 2017-03-24
void IEC104LocalServer::SendAmmeterDataType(InfoMap mapInfo,QIteratorList list)
{
    int cEnergyDataType;
    cEnergyDataType = mapInfo[Addr_RemoteAmeterType_Adj].at(0);
    list.clear();
    switch (cEnergyDataType){
    case 1:
        SendAmmeterDataType1(mapInfo,list);   //发送电表数据-当前电能及最大需量数据
        break;
    case 2:
        SendAmmeterDataType2(mapInfo,list);   //发送电表数据-整点冻结电能数据
        break;
    case 3:
        SendAmmeterDataType3(mapInfo,list);  //发送电表数据-日冻结电能及最大需量数据
        break;
    case 4:
        SendAmmeterDataType4(mapInfo,list);  //发送电表数据-结算日电能及最大需量数据
        break;
    default:
        break;
    }
}

///
/// \brief IEC104LocalServer::SendAmmeterData
///召唤电表数据
void IEC104LocalServer::SendAmmeterDataType1(InfoMap mapInfo,QIteratorList list)
{
    QByteArray arData;
    int iMeasure = 0;
    uint uiEnergy = 0;
	float fEnergy = 0.0;
    if(mapInfo.empty()){
        return;
    }

    if(mapInfo.contains(Addr_RemoteAmeterAddr_Adj))
        list.append(mapInfo.find(Addr_RemoteAmeterAddr_Adj));
    if(mapInfo.contains(Addr_RemoteAmeterType_Adj))
        list.append(mapInfo.find(Addr_RemoteAmeterType_Adj));
    if(mapInfo.contains(Addr_RemoteAmeterReadTime_Adj))
        list.append(mapInfo.find(Addr_RemoteAmeterReadTime_Adj));

    //(当前)正向有功总电能
    if(mapInfo.contains(Addr_RemoteAmeterCurrentInfo_1_Adj)){
        arData.clear();
        uiEnergy = *(uint *)mapInfo[Addr_RemoteAmeterCurrentInfo_1_Adj].data(); //取数据
        arData.append((char *)&uiEnergy, 4);
        list.append(mapInfo.insert(Addr_RemoteAmeterCurrentInfo_1_Adj, arData));
    }

    //(当前)正向有功费率1电能
    if(mapInfo.contains(Addr_RemoteAmeterCurrentInfo_2_Adj)){
        arData.clear();
        uiEnergy = *(uint *)mapInfo[Addr_RemoteAmeterCurrentInfo_2_Adj].data();
        arData.append((char *)&uiEnergy, 4);
        list.append(mapInfo.insert(Addr_RemoteAmeterCurrentInfo_2_Adj, arData));
    }

    //(当前)正向有功费率2电能
    if(mapInfo.contains(Addr_RemoteAmeterCurrentInfo_3_Adj)){
        arData.clear();
        uiEnergy = *(uint *)mapInfo[Addr_RemoteAmeterCurrentInfo_3_Adj].data();
        arData.append((char *)&uiEnergy, 4);
        list.append(mapInfo.insert(Addr_RemoteAmeterCurrentInfo_3_Adj, arData));
    }

    //(当前)正向有功费率3电能
    if(mapInfo.contains(Addr_RemoteAmeterCurrentInfo_4_Adj)){
        arData.clear();
        uiEnergy = *(uint *)mapInfo[Addr_RemoteAmeterCurrentInfo_4_Adj].data();
        arData.append((char *)&uiEnergy, 4);
        list.append(mapInfo.insert(Addr_RemoteAmeterCurrentInfo_4_Adj, arData));
    }

    //(当前)正向有功费率4电能
    if(mapInfo.contains(Addr_RemoteAmeterCurrentInfo_5_Adj)){
        arData.clear();
        uiEnergy = *(uint *)mapInfo[Addr_RemoteAmeterCurrentInfo_5_Adj].data();
        arData.append((char *)&uiEnergy, 4);
        list.append(mapInfo.insert(Addr_RemoteAmeterCurrentInfo_5_Adj, arData));
    }

    //(当前)反向有功总电能
    if(mapInfo.contains(Addr_RemoteAmeterCurrentInfo_6_Adj)){
        arData.clear();
        uiEnergy = *(uint *)mapInfo[Addr_RemoteAmeterCurrentInfo_6_Adj].data();
        arData.append((char *)&uiEnergy, 4);
        list.append(mapInfo.insert(Addr_RemoteAmeterCurrentInfo_6_Adj, arData));
    }

    //(当前)正向无功总电能
    if(mapInfo.contains(Addr_RemoteAmeterCurrentInfo_7_Adj)){
        arData.clear();
        uiEnergy = *(uint *)mapInfo[Addr_RemoteAmeterCurrentInfo_7_Adj].data();
        arData.append((char *)&uiEnergy, 4);
        list.append(mapInfo.insert(Addr_RemoteAmeterCurrentInfo_7_Adj, arData));
    }

    //(当前)反向无功总电能
    if(mapInfo.contains(Addr_RemoteAmeterCurrentInfo_8_Adj)){
        arData.clear();
        uiEnergy = *(uint *)mapInfo[Addr_RemoteAmeterCurrentInfo_8_Adj].data();
        arData.append((char *)&uiEnergy, 4);
        list.append(mapInfo.insert(Addr_RemoteAmeterCurrentInfo_8_Adj, arData));
    }

    //（上 1 次）当前正向有功最大需量及发生时间数据
    if(mapInfo.contains(Addr_RemoteAmeterCurrentInfo_9_Adj)){
        arData.clear();
        arData = mapInfo[Addr_RemoteAmeterCurrentInfo_9_Adj];
        char * cData = arData.data();
        char temp[5];
        memcpy(temp,&cData[4],5);
        arData.clear();

        fEnergy = *(float *)&cData[0];
        iMeasure = ___measure32(fEnergy, 0, 0.0001);    //分辨率转换数据
        arData.append((char *)&iMeasure, 4);

        arData.append((char *)&temp, 5);
        list.append(mapInfo.insert(Addr_RemoteAmeterCurrentInfo_9_Adj, arData));
    }

    //（上 1 次）当前反向有功最大需量及发生时间数据
    if(mapInfo.contains(Addr_RemoteAmeterCurrentInfo_10_Adj)){
        arData.clear();
        arData = mapInfo[Addr_RemoteAmeterCurrentInfo_10_Adj];
        char * cData = arData.data();
        char temp[5];
        memcpy(temp,&cData[4],5);
        arData.clear();

        fEnergy = *(float *)&cData[0];
        iMeasure = ___measure32(fEnergy, 0, 0.0001);    //分辨率转换数据
        arData.append((char *)&iMeasure, 4);

        arData.append((char *)&temp, 5);
        list.append(mapInfo.insert(Addr_RemoteAmeterCurrentInfo_10_Adj, arData));
    }

    if(list.count() > 0){
        SendFrameI(list, CMD_AMMETER_DATA_SELECT_SEND, REASON_REQUEST);
        list.clear();
    }
}

///
/// \brief IEC104LocalServer::SendAmmeterDataType2
///召唤电表数据-整点冻结电能数据   add by zrx 2017-03-24
void IEC104LocalServer::SendAmmeterDataType2(InfoMap mapInfo,QIteratorList list)
{
    QByteArray arData;
    uint uiEnergy = 0.0;
    if(mapInfo.empty()){
        return;
    }

    if(mapInfo.contains(Addr_RemoteAmeterAddr_Adj)){
        list.append(mapInfo.find(Addr_RemoteAmeterAddr_Adj));
    }
    if(mapInfo.contains(Addr_RemoteAmeterType_Adj)){
        list.append(mapInfo.find(Addr_RemoteAmeterType_Adj));
    }
    if(mapInfo.contains(Addr_RemoteAmeterReadTime_Adj)){
        list.append(mapInfo.find(Addr_RemoteAmeterReadTime_Adj));
    }

    if(mapInfo.contains(Addr_RemoteAmeterHourFreezeInfo_1_Adj)){
        list.append(mapInfo.find(Addr_RemoteAmeterHourFreezeInfo_1_Adj));
    }

    //（上 1 次）整点冻结正向有功总电能
    if(mapInfo.contains(Addr_RemoteAmeterHourFreezeInfo_2_Adj)){
        arData.clear();
        uiEnergy = *(uint *)mapInfo[Addr_RemoteAmeterHourFreezeInfo_2_Adj].data(); //取数据
        arData.append((char *)&uiEnergy, 4);
        list.append(mapInfo.insert(Addr_RemoteAmeterHourFreezeInfo_2_Adj, arData));
    }

    //（上 1 次）整点冻结反向有功总电能
    if(mapInfo.contains(Addr_RemoteAmeterHourFreezeInfo_3_Adj)){
        arData.clear();
        uiEnergy = *(uint *)mapInfo[Addr_RemoteAmeterHourFreezeInfo_3_Adj].data(); //取数据
        arData.append((char *)&uiEnergy, 4);
        list.append(mapInfo.insert(Addr_RemoteAmeterHourFreezeInfo_3_Adj, arData));
    }

    if(list.count() > 0){
        SendFrameI(list, CMD_AMMETER_DATA_SELECT_SEND, REASON_REQUEST);
        list.clear();
    }
}

///
/// \brief SendAmmeterDataType3
///召唤电表数据-日冻结电能及最大需量数据   add by zrx 2017-03-24
void IEC104LocalServer::SendAmmeterDataType3(InfoMap mapInfo,QIteratorList list)
{
    QByteArray arData;
    QDateTime dt;
    int iMeasure = 0;
    float fEnergy = 0.0;
	uint uiEnergy = 0;
    if(mapInfo.empty()){
        return;
    }

    if(mapInfo.contains(Addr_RemoteAmeterAddr_Adj))
        list.append(mapInfo.find(Addr_RemoteAmeterAddr_Adj));
    if(mapInfo.contains(Addr_RemoteAmeterType_Adj))
        list.append(mapInfo.find(Addr_RemoteAmeterType_Adj));
    if(mapInfo.contains(Addr_RemoteAmeterReadTime_Adj))
        list.append(mapInfo.find(Addr_RemoteAmeterReadTime_Adj));

    //（上1次）日冻结时间
    if(mapInfo.contains(Addr_RemoteAmeterDayFreezeInfo_1_Adj))
        list.append(mapInfo.find(Addr_RemoteAmeterDayFreezeInfo_1_Adj));

    //（上1次）日冻结正向有功电能数据
    if(mapInfo.contains(Addr_RemoteAmeterDayFreezeInfo_2_Adj)){
        arData.clear();
        QByteArray data;
        data = mapInfo[Addr_RemoteAmeterDayFreezeInfo_2_Adj];

        uiEnergy = *((uint *)data.mid(0,4).data());
        arData.append((char *)&uiEnergy, 4);

        uiEnergy = *((uint *)data.mid(4,4).data());
        arData.append((char *)&uiEnergy, 4);

        uiEnergy = *((uint *)data.mid(8,4).data());
        arData.append((char *)&uiEnergy, 4);

        uiEnergy = *((uint *)data.mid(12,4).data());
        arData.append((char *)&iMeasure, 4);

        uiEnergy = *((uint *)data.mid(16,4).data());
        arData.append((char *)&uiEnergy, 4);

        list.append(mapInfo.insert(Addr_RemoteAmeterDayFreezeInfo_2_Adj, arData));
    }

    //（上1次）日冻结反向有功电能数据
    if(mapInfo.contains(Addr_RemoteAmeterDayFreezeInfo_3_Adj)){
        arData.clear();
        uiEnergy = *(uint *)mapInfo[Addr_RemoteAmeterDayFreezeInfo_3_Adj].data();
        arData.append((char *)&uiEnergy, 4);
        list.append(mapInfo.insert(Addr_RemoteAmeterDayFreezeInfo_3_Adj, arData));
    }

    //（上1次）日冻结正向无功电能数据
    if(mapInfo.contains(Addr_RemoteAmeterDayFreezeInfo_4_Adj)){
        arData.clear();
        uiEnergy = *(uint *)mapInfo[Addr_RemoteAmeterDayFreezeInfo_4_Adj].data();
        arData.append((char *)&uiEnergy, 4);
        list.append(mapInfo.insert(Addr_RemoteAmeterDayFreezeInfo_4_Adj, arData));
    }

    //（上1次）日冻结反向无功电能数据
    if(mapInfo.contains(Addr_RemoteAmeterDayFreezeInfo_5_Adj)){
        arData.clear();
        uiEnergy = *(uint *)mapInfo[Addr_RemoteAmeterDayFreezeInfo_5_Adj].data();
        arData.append((char *)&uiEnergy, 4);
        list.append(mapInfo.insert(Addr_RemoteAmeterDayFreezeInfo_5_Adj, arData));
    }

    //（上 1 次）日冻结着正向有功最大需量及发生时间数据
    if(mapInfo.contains(Addr_RemoteAmeterDayFreezeInfo_6_Adj)){
        arData.clear();
        arData = mapInfo[Addr_RemoteAmeterDayFreezeInfo_6_Adj];
        char * cData = arData.data();
        char temp[5];
        memcpy(temp,&cData[4],5);
        arData.clear();

        fEnergy = *(float *)&cData[0];
        iMeasure = ___measure32(fEnergy, 0, 0.0001);    //分辨率转换数据
        arData.append((char *)&iMeasure, 4);
        arData.append((char *)&temp, 5);
        list.append(mapInfo.insert(Addr_RemoteAmeterDayFreezeInfo_6_Adj, arData));
    }

    //（上 1 次）日冻结反向有功最大需量及发生时间数据
    if(mapInfo.contains(Addr_RemoteAmeterDayFreezeInfo_7_Adj)){
        arData.clear();
        arData = mapInfo[Addr_RemoteAmeterDayFreezeInfo_7_Adj];
        char * cData = arData.data();
        char temp[5];
        memcpy(temp,&cData[4],5);
        arData.clear();

        fEnergy = *(float *)&cData[0];
        iMeasure = ___measure32(fEnergy, 0, 0.0001);    //分辨率转换数据
        arData.append((char *)&iMeasure, 4);

        arData.append((char *)&temp, 5);
        list.append(mapInfo.insert(Addr_RemoteAmeterDayFreezeInfo_7_Adj, arData));
    }

    if(list.count() > 0){
        SendFrameI(list, CMD_AMMETER_DATA_SELECT_SEND, REASON_REQUEST);
    }
}

///
/// \brief SendAmmeterDataType4
///召唤电表数据-结算日电能及最大需量数据   add by zrx 2017-03-24
void IEC104LocalServer::SendAmmeterDataType4(InfoMap mapInfo,QIteratorList list)
{
    QByteArray arData;
    int iMeasure = 0;
    float fEnergy = 0.0;
	uint uiEnergy = 0;
    if(mapInfo.empty()){
        return;
    }

    if(mapInfo.contains(Addr_RemoteAmeterAddr_Adj))
        list.append(mapInfo.find(Addr_RemoteAmeterAddr_Adj));
    if(mapInfo.contains(Addr_RemoteAmeterType_Adj))
        list.append(mapInfo.find(Addr_RemoteAmeterType_Adj));
    if(mapInfo.contains(Addr_RemoteAmeterReadTime_Adj))
        list.append(mapInfo.find(Addr_RemoteAmeterReadTime_Adj));

    //(结算日)正向有功总电能
    if(mapInfo.contains(Addr_RemoteAmeterSettlementInfo_1_Adj)){
        arData.clear();
        uiEnergy = *(uint *)mapInfo[Addr_RemoteAmeterSettlementInfo_1_Adj].data(); //取数据
        arData.append((char *)&uiEnergy, 4);
        list.append(mapInfo.insert(Addr_RemoteAmeterSettlementInfo_1_Adj, arData));
    }

    //(结算日)正向有功费率1电能
    if(mapInfo.contains(Addr_RemoteAmeterSettlementInfo_2_Adj)){
        arData.clear();
        uiEnergy = *(uint *)mapInfo[Addr_RemoteAmeterSettlementInfo_2_Adj].data();
        arData.append((char *)&uiEnergy, 4);
        list.append(mapInfo.insert(Addr_RemoteAmeterSettlementInfo_2_Adj, arData));
    }

    //(结算日)正向有功费率2电能
    if(mapInfo.contains(Addr_RemoteAmeterSettlementInfo_3_Adj)){
        arData.clear();
        uiEnergy = *(uint *)mapInfo[Addr_RemoteAmeterSettlementInfo_3_Adj].data();
        arData.append((char *)&uiEnergy, 4);
        list.append(mapInfo.insert(Addr_RemoteAmeterSettlementInfo_3_Adj, arData));
    }

    //(结算日)正向有功费率3电能
    if(mapInfo.contains(Addr_RemoteAmeterSettlementInfo_4_Adj)){
        arData.clear();
        uiEnergy = *(uint *)mapInfo[Addr_RemoteAmeterSettlementInfo_4_Adj].data();
        arData.append((char *)&uiEnergy, 4);
        list.append(mapInfo.insert(Addr_RemoteAmeterSettlementInfo_4_Adj, arData));
    }

    //(结算日)正向有功费率4电能
    if(mapInfo.contains(Addr_RemoteAmeterSettlementInfo_5_Adj)){
        arData.clear();
        uiEnergy = *(uint *)mapInfo[Addr_RemoteAmeterSettlementInfo_5_Adj].data();
        arData.append((char *)&uiEnergy, 4);
        list.append(mapInfo.insert(Addr_RemoteAmeterSettlementInfo_5_Adj, arData));
    }

    //(结算日)反向有功总电能
    if(mapInfo.contains(Addr_RemoteAmeterSettlementInfo_6_Adj)){
        arData.clear();
        uiEnergy = *(uint *)mapInfo[Addr_RemoteAmeterSettlementInfo_6_Adj].data();
        arData.append((char *)&uiEnergy, 4);
        list.append(mapInfo.insert(Addr_RemoteAmeterSettlementInfo_6_Adj, arData));
    }

    //(结算日)正向无功总电能
    if(mapInfo.contains(Addr_RemoteAmeterSettlementInfo_7_Adj)){
        arData.clear();
        uiEnergy = *(uint *)mapInfo[Addr_RemoteAmeterSettlementInfo_7_Adj].data();
        arData.append((char *)&uiEnergy, 4);
        list.append(mapInfo.insert(Addr_RemoteAmeterSettlementInfo_7_Adj, arData));
    }

    //(结算日)反向无功总电能
    if(mapInfo.contains(Addr_RemoteAmeterSettlementInfo_8_Adj)){
        arData.clear();
        uiEnergy = *(uint *)mapInfo[Addr_RemoteAmeterSettlementInfo_8_Adj].data();
        arData.append((char *)&uiEnergy, 4);
        list.append(mapInfo.insert(Addr_RemoteAmeterSettlementInfo_8_Adj, arData));
    }

    //（上 1 次）结算日正向有功最大需量及发生时间数据
    if(mapInfo.contains(Addr_RemoteAmeterSettlementInfo_9_Adj)){
        arData.clear();
        arData = mapInfo[Addr_RemoteAmeterSettlementInfo_9_Adj];
        char * cData = arData.data();
        char temp[5];
        memcpy(temp,&cData[4],5);
        arData.clear();

        fEnergy = *(float *)&cData[0];
        iMeasure = ___measure32(fEnergy, 0, 0.0001);    //分辨率转换数据
        arData.append((char *)&iMeasure, 4);

        arData.append((char *)&temp, 5);
        list.append(mapInfo.insert(Addr_RemoteAmeterSettlementInfo_9_Adj, arData));
    }

    //（上 1 次）结算日反向有功最大需量及发生时间数据
    if(mapInfo.contains(Addr_RemoteAmeterSettlementInfo_10_Adj)){
        arData.clear();
        arData = mapInfo[Addr_RemoteAmeterSettlementInfo_10_Adj];
        char * cData = arData.data();
        char temp[5];
        memcpy(temp,&cData[4],5);
        arData.clear();

        fEnergy = *(float *)&cData[0];
        iMeasure = ___measure32(fEnergy, 0, 0.0001);    //分辨率转换数据
        arData.append((char *)&iMeasure, 4);

        arData.append((char *)&temp, 5);
        list.append(mapInfo.insert(Addr_RemoteAmeterSettlementInfo_10_Adj, arData));
    }

    if(list.count() > 0){
        SendFrameI(list, CMD_AMMETER_DATA_SELECT_SEND, REASON_REQUEST);
    }
}

/*
 * 召唤子站版本
 * 返回值	无
 */
void IEC104LocalServer::SendVersion()
{
    QIteratorList list;
	InfoMap map;
	QByteArray arData;

	stCSCUSysConfig confSys;
	m_pSetting->querySetting(&confSys, PARAM_CSCU_SYS);

	//int iLen = strlen(confSys.version);
	//arData.append(confSys.version, iLen <= 6 ? iLen : 6);
	arData.append(confSys.version, strlen(confSys.version));

    list.append(map.insert(Addr_CSCUVer_CtrlInfo, arData));

    SendFrameI(list, CMD_CHARACTER, REASON_QUERY);
}

/*
 * 召唤模块版本
 * cCanAddr	输入 模块地址
 * 返回值	无
 */
void IEC104LocalServer::SendModuleVersion(uchar cCanAddr)
{
    QIteratorList list;
	TerminalStatus stStatus;
	InfoMap map;
	QByteArray arData;

	memset(&stStatus, 0, sizeof(TerminalStatus));

	if(m_pDevCache)
		m_pDevCache->QueryTerminalStatus(cCanAddr, stStatus);

	arData.clear();
	arData.append(cCanAddr);
	map[Addr_CanID_Comm] = arData;
    list.append(map.insert(Addr_CanID_Comm, arData));

	arData.clear();
	arData.append((char *)stStatus.psTermianlVer, strlen((char *)stStatus.psTermianlVer));
    list.append(map.insert(Addr_ChargerVer_CtrlInfo, arData));

    SendFrameI(list, CMD_CHARACTER, REASON_QUERY);
}

/*
 * 召唤子站环境监测数据
 * 返回值	无
 */
void IEC104LocalServer::SendStationEnv()
{
    QIteratorList list;
	InfoMap map;
	QByteArray ar, arData;
	uchar cQuality = 0;
	ushort sMeasure = 0;
	RealStatusData real;
	QVariant vKey, vValue;

	m_pDevCache->QueryRealStatusMeter(vValue, CACHE_STATUS, vKey);
	real = vValue.value<RealStatusData>(); 

	//烟感报警器1
	sMeasure = real.alarm1;
	arData.append((char *)&sMeasure, 2);
	arData.append(cQuality);
	//门磁报警器1
	arData.append((char *)&sMeasure, 2);
	arData.append(cQuality);
	//子站温度
    sMeasure = real.temperature + 500;
	arData.append((char *)&sMeasure, 2);
	arData.append(cQuality);
	//子站湿度
	sMeasure = real.humidity;
	arData.append((char *)&sMeasure, 2);
	arData.append(cQuality);
	//预留数据
	ar.fill('\0', 12);
	arData.append(ar);

    list.append(map.insert(Addr_StationEnvTemp, arData));

    SendFrameI(list, CMD_MEASURE, REASON_QUERY);
}

/*
 * 上报冻结电量 
 * mapInfo	输入 信息体集合
 * 返回值	无
 */
void IEC104LocalServer::SendFrozenEnergy(InfoMap& mapInfo)
{
    QIteratorList list;
	InfoMap map;
	QByteArray ar, arData, arOrder, arAddr;
	int iPackageIndex, iEnergy;
	InfoMap::iterator it;
	QDateTime dt;

	arOrder.append(mapInfo[Addr_OrderNumber_Ctrl]);
	arAddr.append(mapInfo[Addr_CanID_Comm]);

	mapInfo.remove(Addr_OrderNumber_Ctrl);
	mapInfo.remove(Addr_CanID_Comm);

	iPackageIndex = 0;

	for(it = mapInfo.begin(); it != mapInfo.end(); ++it){
		arData.clear();

		ar = it.value();

		iEnergy = *((uint*)ar.mid(0, 4).data());
		arData.append((char *)&iEnergy, 4);

		dt = QDateTime::fromString(ar.mid(4, ar.length() - 4), "yyyy-MM-dd hh:mm:ss");
		ushort y=dt.toString("yy").toShort();
		ushort m=dt.toString("MM").toShort();
		ushort d=dt.toString("dd").toShort();
		ushort h=dt.toString("hh").toShort();
		ushort mm=dt.toString("mm").toShort();
		ushort s=dt.toString("ss").toShort() * 1000; 
		arData.append((char *)&s, 2);
		arData.append((char *)&mm, 1);
		arData.append((char *)&h, 1);
		arData.append((char *)&d, 1);
		arData.append((char *)&m, 1);
		arData.append((char *)&y, 1);
		
		if(iPackageIndex == 0){
            list.append(map.insert(Addr_OrderNumber_Ctrl, arOrder));
            list.append(map.insert(Addr_CanID_Comm, arAddr));
		}

        list.append(map.insert(it.key(), arData));
		iPackageIndex++;

		if(iPackageIndex >= FROZEN_ENERGY_PACKAGE_NUM || it.key() == Addr_EndEnergy_FrozenEnergy){
			SendFrameI(list, CMD_FROZEN_ENERGY, REASON_BURST, iPackageIndex);
			iPackageIndex = 0;
			map.clear();
			list.clear();
		}
	}
}

/*
* 同步系统时间，时间设置完成后将设置结果返回至平台
* mapInfo 输入 时间信息体集合
* 返回值  无
*/

void IEC104LocalServer::SyncTime(InfoMap& mapInfo)
{
    QIteratorList list;
    InfoMap map;
    QByteArray arData;
    QDateTime dt;
    QString strCmd, strDate;

    arData = mapInfo[Addr_Default_Comm];

    if(arData.length() < 7)
        return;

    strDate.sprintf("%d-%d-%d %d:%d:%d", 
                    (arData.at(6) & 0x7F) + 2000, 
                    arData.at(5) & 0x0F, 
                    arData.at(4) & 0x1F,
                    arData.at(3) & 0x1F, 
                    arData.at(2) & 0x3F,
                    (arData.at(0) + (arData.at(1) << 8)) / 1000);

    strCmd = "date -s  \"" + strDate + "\"";
    system(strCmd.toAscii().data()); 
    system("hwclock -w");

    dt = QDateTime::currentDateTime();
    ushort y=dt.toString("yy").toShort();
    ushort m=dt.toString("MM").toShort();
    ushort d=dt.toString("dd").toShort();
    ushort h=dt.toString("hh").toShort();
    ushort mm=dt.toString("mm").toShort();
    ushort s=dt.toString("ss").toShort() * 1000; 
    arData.clear();
    arData.append((char *)&s, 2);
    arData.append((char *)&mm, 1);
    arData.append((char *)&h, 1);
    arData.append((char *)&d, 1);
    arData.append((char *)&m, 1);
    arData.append((char *)&y, 1);

    list.append(map.insert(Addr_Default_Comm, arData));
    SendFrameI(list, CMD_SYNC_TIME, REASON_ACTIVE_ACK);
}
/*
 * 设置电表参数
 * mapInfo	输入 电表参数信息体集合
 * 返回值 	无
 */
bool IEC104LocalServer::SetAmmeterParam(uchar* pszData, int iLen)
{
	stAllAmmeterConfig conf;
	stAmmeterConfig stAmmeter;
	uchar* p = pszData;
	while(p < pszData + iLen){
		memset(&stAmmeter, 0, sizeof(stAmmeterConfig));

		//电表地址
        p += 3;   //信息体3个字节
		memcpy(stAmmeter.addr, p, 6);
		p += 6;
		//电表使能
		p += 3;
        stAmmeter.enable = *(uchar *)p;
        p += 1;
		//电压变比
        p += 3;
		stAmmeter.voltageRatio = *(ushort *)p;
		p = p + 2;
		//电流变比
        p += 3;
		stAmmeter.currentRatio = *(ushort *)p;
		p = p + 2;
		//协议类型
        stAmmeter.devType = *(uchar *)p;
        p = p + 1;
        //功能类型
        p += 3;
        stAmmeter.funType = *(uchar *)p;
        p = p + 1;
//		//预留
//        p = p + 2;

		conf.ammeterConfig.append(stAmmeter);
	}

	return m_pSetting->updateSetting(&conf, PARAM_AMMETER);
}

/*
 * 平台设置电表参数
 * mapInfo	输入 电表参数信息体集合
 * 返回值 	无
 */
bool IEC104LocalServer::SetAmmeterParamSet(InfoMap& mapInfo)
{
    stAllAmmeterConfig conf;
    stAmmeterConfig stAmmeter;
    QByteArray arrData;
    memset(&stAmmeter, 0, sizeof(stAmmeterConfig));

    if(mapInfo.contains(Addr_AmeterAddr_Adj)){
        arrData = mapInfo[Addr_AmeterAddr_Adj];
        memcpy(stAmmeter.addr,arrData.data(),sizeof(stAmmeter.addr));
        ConvertDataFormat(stAmmeter.addr,6);      //大小端转换
    }

    if(mapInfo.contains(Addr_AmeterEnable_Adj)){
        stAmmeter.enable = mapInfo[Addr_AmeterEnable_Adj].at(0);
    }

    if(mapInfo.contains(Addr_AmeterVoltageRate_Adj)){
        stAmmeter.voltageRatio = mapInfo[Addr_AmeterVoltageRate_Adj].at(0);
    }

    if(mapInfo.contains(Addr_AmeterCurrentRate_Adj)){
        stAmmeter.currentRatio = mapInfo[Addr_AmeterCurrentRate_Adj].at(0);
    }

    if(mapInfo.contains(Addr_AmeterProtocol_Adj)){
        stAmmeter.devType = mapInfo[Addr_AmeterProtocol_Adj].at(0);
    }

    if(mapInfo.contains(Addr_AmeterData_Adj)){
        stAmmeter.funType = mapInfo[Addr_AmeterData_Adj].at(0);
    }

    conf.ammeterConfig.append(stAmmeter);
    return m_pSetting->updateSetting(&conf, PARAM_AMMETER);
}


/*
 * 设置网络状态到实时数据模块，以备其它模块使用
 * bConnected	输入 网络连接状态
 * 返回值		无
 */
void IEC104LocalServer::SetNetState(bool bConnected, QString strError)
{	
	QString strSql;

	if(m_cServerType == SERVER_REMOTE){
		RealStatusMeterData& real = m_pDevCache->GetUpdateRealStatusMeter();
		real.realData.connectStatus = bConnected;
		m_pDevCache->FreeUpdateRealStatusMeter();

		if(bConnected){
			m_dtOnline = QDateTime::currentDateTime();
			strSql.sprintf("INSERT INTO cscu_online_table(terminal_online_time) VALUES ('%s');", 
					m_dtOnline.toString("yyyy-MM-dd hh:mm:ss").toAscii().data());
		}else{
			if(strError.isEmpty())
				return;

			QDateTime dt = QDateTime::currentDateTime();
			strSql.sprintf("UPDATE cscu_online_table SET terminal_offline_time='%s', \
					offline_reason='%s' WHERE terminal_online_time ='%s';", 
					dt.toString("yyyy-MM-dd hh:mm:ss").toAscii().data(), 
					strError.toAscii().data(),
					m_dtOnline.toString("yyyy-MM-dd hh:mm:ss").toAscii().data());
		}

		m_pDatabase->DBSqlExec(strSql.toAscii().data(), DB_PROCESS_RECORD);
	}
}

/*
 * 发送本地网络状态至总线
 * bOffline 	输入 true:本地离线 false:本地在线
 * 返回值		无
 */
void IEC104LocalServer::SendNetState(bool bOffline)
{
	InfoMap mapInfo;

	mapInfo[Addr_Local_Emergency] = QByteArray(1, bOffline ? 0xFF : 0x1);
	emit sigToBus(mapInfo, AddrType_LocalEmergency_Net);
}

/*
 * 设置紧急充电开关
 * mapInfo	输入 信息体集合
 * 返回值		无
 */
void IEC104LocalServer::WriteEmergencyEnable(InfoMap &mapInfo)
{
	EmergencyConfig config;
   	QIteratorList list;
	uchar cRet = 0x01;

	do{
		if(!mapInfo.contains(Addr_Emergency_Enable))
			break;

		if(!m_pSetting->querySetting(&config, PARAM_EMERGENCY))
			break;

		if(mapInfo[Addr_Emergency_Enable].at(0) == 0xFF)
			config.emergency_enable = true;
		else
			config.emergency_enable = false;

		if(!m_pSetting->updateSetting(&config, PARAM_EMERGENCY))
			break;

		if(!config.emergency_enable){
			RealStatusMeterData& real = m_pDevCache->GetUpdateRealStatusMeter();
			real.realData.emergencyStatus = false;
			m_pDevCache->FreeUpdateRealStatusMeter();
		}

		cRet = 0xFF;
	}while(false);

	list.append(mapInfo.insert(Addr_Emergency_Enable_Result, QByteArray(1, cRet)));
	SendFrameI(list, CMD_EMERGENCY_ENABLE_RET, REASON_RETURN);
}

/*
 * 设置应急充电详细配置
 * mapInfo	输入 信息体集合
 * 返回值		无
 */
void IEC104LocalServer::WriteEmergencySetting(InfoMap &mapInfo)
{
	EmergencyConfig config;
   	QIteratorList list;
	uchar cRet = 0x01;

	do{
		if(!m_pSetting->querySetting(&config, PARAM_EMERGENCY))
			break;

		if(mapInfo.contains(Addr_Emergency_Duration))
			config.duration = mapInfo[Addr_Emergency_Duration].at(0);

		if(mapInfo.contains(Addr_Emergency_OrderCount))
			config.order_count = *((short *)mapInfo[Addr_Emergency_OrderCount].data());

		if(mapInfo.contains(Addr_Emergency_CheckTime))
			config.check_time = *((short *)mapInfo[Addr_Emergency_CheckTime].data());

		if(mapInfo.contains(Addr_Emergency_VinAuth) && mapInfo[Addr_Emergency_VinAuth].at(0) == 0xFF)
			config.vin_authenticate = true;
		else
			config.vin_authenticate = false;

		if(mapInfo.contains(Addr_Emergency_CarAuth) && mapInfo[Addr_Emergency_CarAuth].at(0) == 0xFF)
			config.car_authenticate = true;
		else
			config.car_authenticate = false;

		if(mapInfo.contains(Addr_Emergency_CardAuth) && mapInfo[Addr_Emergency_CardAuth].at(0) == 0xFF)
			config.card_authenticate = true;
		else
			config.card_authenticate = false;

		if(mapInfo.contains(Addr_Emergency_QueueGun) && mapInfo[Addr_Emergency_QueueGun].at(0) == 0xFF)
			config.queue_gun = true;
		else
			config.queue_gun = false;

		if(mapInfo.contains(Addr_Emergency_QueueCar) && mapInfo[Addr_Emergency_QueueCar].at(0) == 0xFF)
			config.queue_car = true;
		else
			config.queue_car = false;

		if(mapInfo.contains(Addr_Emergency_QueueCard) && mapInfo[Addr_Emergency_QueueCard].at(0) == 0xFF)
			config.queue_card = true;
		else
			config.queue_card = false;

		if(!m_pSetting->updateSetting(&config, PARAM_EMERGENCY))
			break;

		cRet = 0xFF;
	}while(false);

	m_iEmergencyTime = config.check_time;

	list.append(mapInfo.insert(Addr_Emergency_Setting_Result, QByteArray(1, cRet)));
	SendFrameI(list, CMD_EMERGENCY_SETTING_RET, REASON_RETURN);
}

/*
 * 保存轮充组信息至数据库，响应保存结果
 * mapInfo	输入 信息体集合
 * 返回值		无
 */
void IEC104LocalServer::WriteQueueGroupInfo(InfoMap &mapInfo)
{
	QIteratorList list;
	InfoMap map;
	QByteArray ar;
	InfoMap::iterator it;
	QString strSql;
	uchar cRet = 0x01;
	int iGroup = 1;

	if(mapInfo.count() <= 0)
		goto finished;

	if(m_pDatabase->DBSqlExec((char *)"BEGIN", DB_PARAM) != 0)
		goto finished;

	strSql = "DELETE FROM queue_group_table;";
	if(m_pDatabase->DBSqlExec(strSql.toAscii().data(), DB_PARAM) != 0)
		goto rollback;

	ar = mapInfo[Addr_QueueGroup_Info];
	for(int i = 0; i < ar.length(); i = i + 8){
		strSql.sprintf("INSERT INTO queue_group_table (group_id, gun1, \
				gun2, gun3, gun4, gun5) VALUES(%d, %d, %d, %d, %d, %d);", 
				iGroup++, ar.at(i), ar.at(i + 1), ar.at(i + 2), ar.at(i + 3), ar.at(i + 4));

		if(m_pDatabase->DBSqlExec(strSql.toAscii().data(), DB_PARAM) != 0)
			goto rollback;
	}

	if(m_pDatabase->DBSqlExec((char *)"COMMIT", DB_PARAM) != 0)
		goto rollback;

	cRet = 0xFF;	

rollback:
	if(cRet != 0xFF)
		m_pDatabase->DBSqlExec((char *)"ROLLBACK", DB_PARAM);

finished:
	list.append(map.insert(Addr_QueueGroup_Result, QByteArray(1, cRet)));
	SendFrameI(list, CMD_QUEUE_GROUP_RESULT, REASON_RETURN);
}

/*
 * 从服务器列表中取出地址，进行连接
 */
bool IEC104LocalServer::connectToServer()
{
	m_strHost = "";
	m_sPort = 0;

	if(m_listHost.count() <= 0){
		WriteLog(QString("Host Error With Count=0"));
		return false;	
	}

	if(m_iHostIndex < 0){
		m_iHostIndex = 0;
	}else{
		if(m_iTryConnect >= 3){
			m_iTryConnect = 0;

			m_iHostIndex++;
			if(m_iHostIndex >= m_listHost.count())
				m_iHostIndex = 0;

			WriteLog(QString().sprintf("Switch Host Index = %d With Count = %d", m_iHostIndex, m_listHost.count()));
		}
	}

	HostNode node;
	node = m_listHost.at(m_iHostIndex);
	m_strHost = node.strHost;
	m_sPort = node.sPort;

	m_pSocket->connectToHost(m_strHost, m_sPort);
	m_iTryConnect++;
	WriteLog(QString("Connect To Host=%1, Port=%2, Try Times=%3").arg(m_strHost).arg(m_sPort).arg(m_iTryConnect));
	return true;
}

bool IEC104LocalServer::GetHostList(stServerListConfig &confServerList)
{
	HostNode node;

	m_listHost.clear();

	node.strHost = m_confServer.serverIp;
	node.sPort = m_confServer.serverPort;

	if(!node.strHost.isEmpty() && node.sPort > 0){
		WriteLog(QString("Main Host=%1 Port=%2").arg(node.strHost).arg(node.sPort));
		m_listHost.append(node);
	}

	if(m_cServerType == SERVER_REMOTE){
		node.strHost = confServerList.serverIp1;
		node.sPort = confServerList.serverPort1;
		if(!node.strHost.isEmpty() && node.sPort > 0){
			WriteLog(QString("First Host=%1 Port=%2").arg(node.strHost).arg(node.sPort));
			m_listHost.append(node);
		}

		node.strHost = confServerList.serverIp2;
		node.sPort = confServerList.serverPort2;
		if(!node.strHost.isEmpty() && node.sPort > 0){
			WriteLog(QString("Second Host=%1 Port=%2").arg(node.strHost).arg(node.sPort));
			m_listHost.append(node);
		}

		node.strHost = confServerList.serverIp3;
		node.sPort = confServerList.serverPort3;
		if(!node.strHost.isEmpty() && node.sPort > 0){
			WriteLog(QString("Third Host=%1 Port=%2").arg(node.strHost).arg(node.sPort));
			m_listHost.append(node);
		}
	}

	WriteLog(QString("Host List Count=%1").arg(m_listHost.count()));
	if(m_listHost.count() <= 0)
		return false;

	return true;
}

bool IEC104LocalServer::WriteServerList(InfoMap &mapInfo)
{
	uint addr = 0;
	uchar cRet = 0x01;
	QStringList listServer;
	QString str, strIp, strPort;
	stServerListConfig conf;

	do{
		if(!m_pSetting->querySetting(&conf, PARAM_SERVERLIST))
			break;

		if(mapInfo.contains(Addr_First_Server)){
			str = mapInfo[Addr_First_Server];
			listServer = str.split(":");
			strIp = listServer.at(0);
			strPort = listServer.at(1);
			snprintf(conf.serverIp1, sizeof(conf.serverIp1), "%s", strIp.toAscii().data());
			conf.serverPort1 = strPort.toInt();
			addr = Addr_First_Server_Result;
			break;
		}

		if(mapInfo.contains(Addr_Second_Server)){
			str = mapInfo[Addr_Second_Server];
			listServer = str.split(":");
			strIp = listServer.at(0);
			strPort = listServer.at(1);
			snprintf(conf.serverIp2, sizeof(conf.serverIp2), "%s", strIp.toAscii().data());
			conf.serverPort2 = strPort.toInt();
			addr = Addr_Second_Server_Result;
			break;
		}

		if(mapInfo.contains(Addr_Third_Server)){
			str = mapInfo[Addr_Third_Server];
			listServer = str.split(":");
			strIp = listServer.at(0);
			strPort = listServer.at(1);
			snprintf(conf.serverIp3, sizeof(conf.serverIp3), "%s", strIp.toAscii().data());
			conf.serverPort3 = strPort.toInt();
			addr = Addr_Third_Server_Result;
			break;
		}
	}while(false);

	if(addr <= 0)
		return false;

	if(m_pSetting->updateSetting(&conf, PARAM_SERVERLIST)){
		GetHostList(conf);
		cRet = 0xFF;
	}

	QIteratorList list;
	InfoMap map;

	list.append(map.insert(addr, QByteArray(1, cRet)));
	SendFrameI(list, CMD_STATION_PARAM, REASON_RETURN);
	
	return true;
}

/*
 * 保存多枪分组信息至数据库，响应保存结果
 * mapInfo	输入 信息体集合
 * 返回值		无
 */
void IEC104LocalServer::WriteChargeGunGroupInfo(InfoMap &mapInfo)
{
    QIteratorList list;
    InfoMap map;
    QByteArray ar;
    //InfoMap::iterator it;
    QString strSql;
    uchar cRet = 0x01;
    int iGroup = 1;

    if(mapInfo.count() <= 0)
        goto finished;

//    cRet =  (uchar)CheckChargeGunGroupInfo(mapInfo);
//    if(cRet !=0xFF)
//        goto finished;
//    cRet = 0x01;
    if(m_pDatabase->DBSqlExec((char *)"BEGIN", DB_PARAM) != 0)
        goto finished;

    strSql = "DELETE FROM chargegun_group_table;";
    if(m_pDatabase->DBSqlExec(strSql.toAscii().data(), DB_PARAM) != 0)
        goto rollback;

    ar = mapInfo[Addr_Group_ChargeGun];
    for(int i = 0; i < ar.length(); i = i + 10){
        strSql.sprintf("INSERT INTO chargegun_group_table (group_id, gun1, \
                gun2, gun3, gun4, gun5,gun6,gun7) VALUES(%d, %d, %d, %d, %d, %d,%d,%d);",
                iGroup++, ar.at(i), ar.at(i + 1), ar.at(i + 2), ar.at(i + 3), ar.at(i + 4),ar.at(i + 5),ar.at(i + 6));

        if(m_pDatabase->DBSqlExec(strSql.toAscii().data(), DB_PARAM) != 0)
            goto rollback;
    }

    if(m_pDatabase->DBSqlExec((char *)"COMMIT", DB_PARAM) != 0)
        goto rollback;

    cRet = 0xFF;

rollback:
    if(cRet != 0xFF)
        m_pDatabase->DBSqlExec((char *)"ROLLBACK", DB_PARAM);

finished:
    list.append(map.insert(Addr_ChargeGunGroup_Result, QByteArray(1, cRet)));
    SendFrameI(list, CMD_CHARGEGUN_GROUP_RESULT, REASON_RETURN);
}

/*
 * 记录104报文内容，将内容转为16进制格式记录至日志
 * pszData	输入 报文内容
 * iLen		输入 报文长度
 * cType	输入 报文帧类型
 * bSend	输入 标识报文为发送或从平台接收
 * 返回值	无
 */
void IEC104LocalServer::WriteLog(uchar* pszData, int iLen, uchar cType, bool bSend)
{
    QString str, strLog;
    uchar* p;
	int iPos = 0;

	if(bSend)
		str += "[send]";
	else
		str += "[recv]";

	switch(cType){
		case FRAME_TYPE_I:
			strLog = str + "[I]";
			break;
		case FRAME_TYPE_U:
			strLog = str + "[U]";
			break;
		case FRAME_TYPE_S:
			strLog = str + "[S]";
			break;
	}

	for(p = pszData; p < pszData + iLen; p++){
		if(cType == FRAME_TYPE_I){
			iPos = p - pszData;
			switch(iPos){
				case 6:
					str.sprintf(" [%02x]", *p);
					break;
				case 8:
					str.sprintf(" [%02x", *p);
					break;
				case 9:
					str.sprintf(" %02x]", *p);
					break;
				case 10:
					str.sprintf(" [%02x", *p);
					break;
				case 17:
					str.sprintf(" %02x]", *p);
					break;
				default:
					str.sprintf(" %02x", *p);
			}
		}
		else{
			str.sprintf(" %02x", *p);
		}

		strLog += str;
	}

	WriteLog(strLog);
}

/*
 * 记录模块执行情况日志
 * strLog	输入 需记录的内容
 * iLevel	输入 日志输出等级
 * 返回值	无
 */
void IEC104LocalServer::WriteLog(QString strLog, int iLevel)
{
	switch (iLevel) {
		case 1:
			m_pLog->getLogPoint(_strLogName)->debug(strLog);
			break;
		case 2:
			m_pLog->getLogPoint(_strLogName)->info(strLog);
			break;
		case 3:
			m_pLog->getLogPoint(_strLogName)->warn(strLog);
			break;
		case 4:
			m_pLog->getLogPoint(_strLogName)->error(strLog);
			break;
		default:
			break;
	}
}

/*
 * 动态库接口函数，初始化模块
 * pThread	输入 模块将要运行的线程
 * 返回值 	0表示无错误，-1表示有错误
 */
int IEC104LocalServer::InitModule(QThread* pThread)
{
	m_pWorkThread = pThread;
	return 0;
}

/*
 * 动态库接口函数，向BUS注册模块
 * pBus		输入 BUS模块指针
 * 返回值 	0表示无错误，-1表示有错误
 */
int IEC104LocalServer::RegistModule()
{
	QList<int> list;

	list.append(AddrType_TermSignal);			//突发遥信
	list.append(AddrType_TermAdjustmentAck);	//充电电流响应结果
	list.append(AddrType_CmdCtrl_AckResult);	//充电响应结果
	list.append(AddrType_CmdCtrl_ExecResult);	//充电执行结果
	list.append(AddrType_FrozenEnergy);			//突发冻结电量
	list.append(AddrType_TermMeasure);			//突发遥测
	list.append(AddrType_TermIndex_Query);		//获取CAN地址终端编号对应关系
	list.append(AddrType_UpdateResult);			//日志、配置、程序、模块升级及下载执行结果
	list.append(AddrType_ChargeServicApplyAccountInfo);			//刷卡申请帐户信息
	list.append(AddrType_OutApplyStartChargeByChargeServic);	//刷卡申请开始充电
	list.append(AddrType_OutApplyStopChargeByChargeServic);		//刷卡申请结束充电
	list.append(AddrType_VinApplyStartCharge);					//zigbee申请开始充电
	list.append(AddrType_VinApplyStopCharge);					//zigbee申请结束充电
	list.append(AddrType_CarLicenceApplyStartCharge);			//zigbee车牌号申请开始充电
	list.append(AddrType_CarLicenceApplyStopCharge);			//zigbee车牌号申请结束充电
	//list.append(AddrType_CarLock_Apply);    //modified by weiwb
	list.append(AddrType_CarLock_Result);    //add by weiwb
	list.append(AddrType_Reboot_Result);
	list.append(AddrType_ChargePriority_Result);
	list.append(AddrType_ChargeFinishTime_Result);
	list.append(AddrType_ChargeMode_Result);
	list.append(AddrType_MaxLoad_Result);
	list.append(AddrType_GroupPolicy_Result);
	list.append(AddrType_AuxPower_Result);
	list.append(AddrType_PredictTime_Burst);
	list.append(AddrType_ModuleFree_Burst);
	list.append(AddrType_QueueInfo_Burst);
	list.append(AddrType_PredictTime_Apply);
	list.append(AddrType_DoubleSys300kwSetting_Result);
	list.append(AddrType_DoubleSys300kwSetting_Upload);
	list.append(AddrType_RelayControl_Result);
	list.append(AddrType_CheckChargeManner_Success);//直流机上传单双枪信息不符合规则
	list.append(AddrType_RemoteAmmeterSendType);   //远程抄表数据主题
	list.append(AddrType_LocalEmergency_State);
	list.append(AddrType_LocalEmergency_Result);
	list.append(AddrType_TermCarLock);//车位锁状态主题

	list.append(AddrType_BatteryStatus);
	list.append(AddrType_PowerOptimizerInfo);

	list.append(AddrType_CmdCtrl_AckResult_Peak);
	list.append(AddrType_InPeakApplyStopCharge_Result_Server);
	list.append(AddrType_CmdCtrl_AckFaile_Peak);
	list.append(AddrType_ActiveDefend_Alarm);//主动防护告警

	return CBus::GetInstance()->RegistDev(this, list);
}

/*
 * 动态库接口函数，启动模块
 * 返回值 	0表示无错误，-1表示有错误
 */
int IEC104LocalServer::StartModule()
{
	m_pSocket->moveToThread(m_pWorkThread);
	this->moveToThread(m_pWorkThread);
	QObject::connect(m_pWorkThread, SIGNAL(started()), this, SLOT(slot_onThreadRun()));
	m_pWorkThread->start();

	return 0;
}

/*
 * 动态库接口函数，停止模块
 * 返回值 	0表示无错误，-1表示有错误
 */
int IEC104LocalServer::StopModule()
{
	SetNetState(false, "Module terminated");	
	WriteLog(QString("Module terminated"));

	m_bExit = true;
	m_pSocket->close();

	return 0;
}

/*
 * 动态库接口函数，模块工作状态
 * 返回值 	0表示无错误，-1表示有错误
 */
int IEC104LocalServer::ModuleStatus()
{
	return 0;
}

/*
 * 动态库接口函数，创建模块实例
 * pDepends 输入 公共模块指针列表
 * 返回值 	成功返回模块实例，失败返回NULL
 */
CModuleIO* CreateDevInstance()
{
	CModuleIO* pModule = new IEC104LocalServer();
	
	return pModule ? pModule : NULL;
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
 * 上报冻结电量 削峰填谷  hd
 * mapInfo	输入 信息体集合
 * 返回值	无
 */
void IEC104LocalServer::SendFrozenEnergyPeakShaving(InfoMap& mapInfo)
{WriteLog(QString("BBBBBBBBBBBBBBBBBBBBBBB]"));
    QIteratorList list;
    InfoMap map;
    QByteArray ar, arData, arOrder, arAddr,arResult,arStartTime,arEndTIme,arGetCmdTime,arChargeEnergy,arStopReasion,arChargeType;
    int iPackageIndex, iEnergy;
    InfoMap::iterator it;
    QDateTime dt;
    unsigned char reasion=0;

    iPackageIndex = 0;

    if(mapInfo.contains(Addr_WorkState_GDA))   //充电终端工作状态   借用作为突发还是回复原因hd
    {
         reasion = mapInfo[Addr_WorkState_GDA].at(0);
    }
    arOrder.append(mapInfo[Addr_OrderNumber_Ctrl]);
    arAddr.append(mapInfo[Addr_CanID_Comm]);

    arResult.append(mapInfo[Addr_AckResult_Ctrl]);

    arData.clear();
    ar = mapInfo[Addr_StartTime_Peak];
    dt = QDateTime::fromString(ar, "yyyy-MM-dd hh:mm:ss");
    ushort y=dt.toString("yy").toShort();
    ushort m=dt.toString("MM").toShort();
    ushort d=dt.toString("dd").toShort();
    ushort h=dt.toString("hh").toShort();
    ushort mm=dt.toString("mm").toShort();
    ushort s=dt.toString("ss").toShort() * 1000;
    arData.append((char *)&s, 2);
    arData.append((char *)&mm, 1);
    arData.append((char *)&h, 1);
    arData.append((char *)&d, 1);
    arData.append((char *)&m, 1);
    arData.append((char *)&y, 1);
    arStartTime.append(arData);

    arData.clear();
    ar = mapInfo[Addr_StopTime_Peak];
    dt = QDateTime::fromString(ar, "yyyy-MM-dd hh:mm:ss");
    y=dt.toString("yy").toShort();
    m=dt.toString("MM").toShort();
    d=dt.toString("dd").toShort();
    h=dt.toString("hh").toShort();
    mm=dt.toString("mm").toShort();
    s=dt.toString("ss").toShort() * 1000;
    arData.append((char *)&s, 2);
    arData.append((char *)&mm, 1);
    arData.append((char *)&h, 1);
    arData.append((char *)&d, 1);
    arData.append((char *)&m, 1);
    arData.append((char *)&y, 1);
    arEndTIme.append(arData);
    if(m_cServerType == SERVER_LOCAL)
    {
    arData.clear();
    ar = mapInfo[Addr_GetCmdTime_Peak];
    dt = QDateTime::fromString(ar, "yyyy-MM-dd hh:mm:ss");
    y=dt.toString("yy").toShort();
    m=dt.toString("MM").toShort();
    d=dt.toString("dd").toShort();
    h=dt.toString("hh").toShort();
    mm=dt.toString("mm").toShort();
    s=dt.toString("ss").toShort() * 1000;
    arData.append((char *)&s, 2);
    arData.append((char *)&mm, 1);
    arData.append((char *)&h, 1);
    arData.append((char *)&d, 1);
    arData.append((char *)&m, 1);
    arData.append((char *)&y, 1);
    arGetCmdTime.append(arData);
    }


    arChargeEnergy.append(mapInfo[Addr_ChargeEnergy_Peak]);
    arStopReasion.append(mapInfo[Addr_StopReasion_Peak]);
    arChargeType.append(mapInfo[Addr_OrderType_Peak]);//临时调试屏蔽 FJC

    mapInfo.remove(Addr_OrderNumber_Ctrl);
    mapInfo.remove(Addr_CanID_Comm);
    mapInfo.remove(Addr_AckResult_Ctrl);
    mapInfo.remove(Addr_StartTime_Peak);
    mapInfo.remove(Addr_StopTime_Peak);
    if(m_cServerType == SERVER_LOCAL)
    {
    mapInfo.remove(Addr_GetCmdTime_Peak);
    }
    mapInfo.remove(Addr_ChargeEnergy_Peak);
    mapInfo.remove(Addr_StopReasion_Peak);
    mapInfo.remove(Addr_OrderType_Peak);
    mapInfo.remove(Addr_WorkState_GDA);

    if(mapInfo.isEmpty())
    {
        list.append(map.insert(Addr_OrderNumber_Ctrl, arOrder));
        list.append(map.insert(Addr_CanID_Comm, arAddr));
        list.append(map.insert(Addr_AckResult_Ctrl, arResult));
        list.append(map.insert(Addr_StartTime_Peak, arStartTime));
        list.append(map.insert(Addr_StopTime_Peak, arEndTIme));
        if(m_cServerType == SERVER_LOCAL)
        {
        list.append(map.insert(Addr_GetCmdTime_Peak, arGetCmdTime));
        }
        list.append(map.insert(Addr_ChargeEnergy_Peak, arChargeEnergy));
        list.append(map.insert(Addr_StopReasion_Peak, arStopReasion));
        list.append(map.insert(Addr_OrderType_Peak, arChargeType));//临时调试屏蔽 FJC
        if(reasion == 0x03)
            SendFrameI(list, CMD_FROZEN_ENERGY_PEAK, REASON_BURST, iPackageIndex);
        else if(reasion == 0x0b)
            SendFrameI(list, CMD_FROZEN_ENERGY_PEAK, REASON_RETURN, iPackageIndex);
    }
    else
    {

    iPackageIndex = 0;

    for(it = mapInfo.begin(); it != mapInfo.end(); ++it){
        arData.clear();

        ar = it.value();

        iEnergy = *((uint *)ar.mid(0, 4).data());
        arData.append((char *)&iEnergy, 4);

        dt = QDateTime::fromString(ar.mid(4, ar.length() - 4), "yyyy-MM-dd hh:mm:ss");
        y=dt.toString("yy").toShort();
        m=dt.toString("MM").toShort();
        d=dt.toString("dd").toShort();
        h=dt.toString("hh").toShort();
        mm=dt.toString("mm").toShort();
        s=dt.toString("ss").toShort() * 1000;
        arData.append((char *)&s, 2);
        arData.append((char *)&mm, 1);
        arData.append((char *)&h, 1);
        arData.append((char *)&d, 1);
        arData.append((char *)&m, 1);
        arData.append((char *)&y, 1);

        if(iPackageIndex == 0){
            list.append(map.insert(Addr_OrderNumber_Ctrl, arOrder));
            list.append(map.insert(Addr_CanID_Comm, arAddr));
            list.append(map.insert(Addr_AckResult_Ctrl, arResult));
            list.append(map.insert(Addr_StartTime_Peak, arStartTime));
            list.append(map.insert(Addr_StopTime_Peak, arEndTIme));
            if(m_cServerType == SERVER_LOCAL)
            {
            list.append(map.insert(Addr_GetCmdTime_Peak, arGetCmdTime));
            }
            list.append(map.insert(Addr_ChargeEnergy_Peak, arChargeEnergy));
            list.append(map.insert(Addr_StopReasion_Peak, arStopReasion));
            list.append(map.insert(Addr_OrderType_Peak, arChargeType));//临时调试屏蔽 FJC
        }

        list.append(map.insert(it.key(), arData));
        iPackageIndex++;

        if(iPackageIndex >= StopResult_PACKAGE_NUM || it.key() == Addr_EndEnergy_FrozenEnergy){
            if(reasion == 0x03)
                SendFrameI(list, CMD_FROZEN_ENERGY_PEAK, REASON_BURST, iPackageIndex);
            else if(reasion == 0x0b)
                SendFrameI(list, CMD_FROZEN_ENERGY_PEAK, REASON_RETURN, iPackageIndex);
            iPackageIndex = 0;
            map.clear();
            list.clear();
        }
    }
    }
}
/*
 * 校验多枪分组信息返回结果
 * mapInfo	输入 信息体集合
 * 返回值		无
 */
int  IEC104LocalServer::CheckChargeGunGroupInfo(InfoMap &mapInfo)
{
    unsigned char canidtemp[120];
    int count =0;
    QByteArray ar;

    ar = mapInfo[Addr_Group_ChargeGun];
    for(int i = 0; i < ar.length(); i = i + 10)
    {
        for(int y=0;y<7;y++)
        {
         canidtemp[i *7+y] = ar.at(i+y);
         count ++;
        }
    }
    for(int i=0;i<count;i++)
    {
        for(int y=i+1;y<count;y++)
        {
            if((canidtemp[i]) && (canidtemp[i] == canidtemp[i]))
                return 0x01;
        }
    }
    return  0xFF;
}

/*
 *主动防护告警信息
 */
void IEC104LocalServer::SendActiveDefendAlarm(InfoMap &mapInfo)
{
    QIteratorList list;

	if(mapInfo.contains(Addr_CanID_Comm))
		list.append(mapInfo.find(Addr_CanID_Comm));
	if(mapInfo.contains(Addr_ActiveDefend_AlarmCode))
		list.append(mapInfo.find(Addr_ActiveDefend_AlarmCode));
	if(mapInfo.contains(Addr_ActiveDefend_Action))
		list.append(mapInfo.find(Addr_ActiveDefend_Action));

	SendFrameI(list, CMD_ACTIVEDEFEND_ALARM, REASON_BURST, 1);
}
