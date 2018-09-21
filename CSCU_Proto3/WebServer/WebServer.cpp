#include "WebServer.h"
#include <QDateTime>
#include <QCryptographicHash>
#include <QFile>
#include <QDebug>
#include "sqlite3.h"
#include "aes_cbc.h"
#include "json-c/json.h"
#include "commfunc.h"

#define PAGE_SIZE	100
#define DB_BEGIN	(char *)"BEGIN"
#define DB_ROLLBACK	(char *)"ROLLBACK"
#define DB_COMMIT	(char *)"COMMIT"
#define DB_EMERGENCY_ORDER_PATH (char*)"/mnt/nandflash/database/emergency_order.db"
#define DB_EMERGENCY_REAL_PATH (char*)"/mnt/nandflash/database/emergency_real.db"

#define REQ_MAX_NUMBER		9999
#define SEND_ORDER_TIMEOUT	6
#define REQ_CARD_TIMEOUT	1
#define REQ_CAR_TIMEOUT		1
#define REQ_ORDER_TIMEOUT	1

const char *interface[] = {
	"api/query_token/OffLineCharge",
	"api/GetCustomerCards/OffLineCharge",
	"api/GetCars/OffLineCharge",
	"api/AddOrder/OffLineCharge"
};

CWebServer::CWebServer()
{
	_strLogName = "webserver";

	m_pDatabase = DBOperate::GetInstance();
	m_pSetting = ParamSet::GetInstance();
	m_pLog = Log::GetInstance();

	WebServerConfig webconf;
    stServer0Config serverconf;

	m_pSetting->querySetting(&webconf, PARAM_WEBSERVER);

	m_bTerminate = false;
	m_strOperateId = webconf.operator_id;
	m_strOperateSecret = webconf.operator_secret;
	m_strDataSecret = webconf.data_secret;
	m_strKey = webconf.aes_key;
	m_strIv = webconf.aes_iv;
	WriteLog(QString("OperateId = %1, OperateSecret= %2, DataSecret = %3, AesKey = %4, AesIV = %5")
			.arg(m_strOperateId).arg(m_strOperateSecret).arg(m_strDataSecret).arg(m_strKey).arg(m_strIv));

	m_pSetting->querySetting(&serverconf, PARAM_SERVER0);
	m_strStation = serverconf.stationNo;
	WriteLog(QString("StationNo = %1").arg(m_strStation));

	m_iServerTimer = -1;
	m_strToken = "";
	m_iTokenAlive = 0;
	m_iRequest = 1;
	m_iCardPage = 1;
	m_iCarPage = 1;
	m_iCardTimeout = -1;
	m_iCarTimeout = -1;
	m_iOrderTimeout = -1;
	m_iSendTimeout = -1;
}

CWebServer::~CWebServer()
{
	if(m_iServerTimer > 0){
		killTimer(m_iServerTimer);
		m_iServerTimer = -1;
	}
}

int CWebServer::InitModule( QThread* pThread)
{
	m_pWorkThread = pThread;
	this->moveToThread(m_pWorkThread);
	QObject::connect(m_pWorkThread, SIGNAL(started()), this, SLOT(slot_onWorkThreadRun()));

	return 0;
}

int CWebServer::RegistModule()
{
	QList<int> list;

	list.append(AddrType_Emergency_Order);
	list.append(AddrType_Comm_Addr);
	CBus::GetInstance()->RegistDev(this, list);

	return 0;
}

int CWebServer::StartModule()
{
	m_pWorkThread->start();
	return 0;
}

int CWebServer::StopModule()
{
	m_bTerminate = true;
	return 0;
}

int CWebServer::ModuleStatus()
{
	return 0;
}

CModuleIO* CreateDevInstance()
{
	return new CWebServer();
}

void DestroyDevInstance(CModuleIO* pModule)
{
	if(pModule)
		delete pModule;
}

void CWebServer::slotFromBus(InfoMap mapInfo, InfoAddrType type)
{
	switch(type){
		case AddrType_Emergency_Order:
			if(!mapInfo.contains(Addr_Local_Emergency))
				return;

			if(mapInfo[Addr_Local_Emergency].at(0) == 0x01){
				m_iSendTimeout = 0;
				WriteLog(QString().sprintf("From Bus Send Order SendTimeout = %d", m_iSendTimeout));
				sendOrder();
			}
			break;
		case AddrType_Comm_Addr:
			ParseWebUrl(mapInfo);
			break;
		default:	
			break;
	}
}

void CWebServer::slot_onWorkThreadRun()
{
	requestToken();
	m_iServerTimer = startTimer(60 * 1000);

	if(QFile::exists(QString(DB_EMERGENCY_ORDER_PATH))){
		m_iSendTimeout = 0;
		WriteLog(QString().sprintf("WebServer Send Order On Start SendTimeout = %d", m_iSendTimeout));
	}
}

void CWebServer::timerEvent(QTimerEvent *event)
{
	if(event->timerId() == m_iServerTimer){
		if(m_iSendTimeout >= 0){
			m_iSendTimeout++;

			WriteLog(QString().sprintf("SendTimeout = %d", m_iSendTimeout));
		}

		if(m_iSendTimeout >= SEND_ORDER_TIMEOUT){
			WriteLog(QString().sprintf("Send Order Timeout = %d Minutes", m_iSendTimeout));

			m_iOrderTimeout = -1;
			m_iSendTimeout = -1;

			InfoMap map;
			map[Addr_Local_Emergency] = QByteArray(1, 0x0);
			emit sigToBus(map, AddrType_Emergency_Order_Result);
		}

		m_iTokenAlive--;
		if(m_strToken.isEmpty() || m_iTokenAlive < 0){
			WriteLog(QString("Request Token"));
			m_strToken.clear();
			m_iTokenAlive = 0;
			requestToken();
			return;
		}

		if(m_iCardTimeout > 0){
			m_iCardTimeout--;	
		}else if(m_iCardTimeout == 0){
			requestCard();	
		}

		if(m_iCarTimeout > 0){
			m_iCarTimeout--;	
		}else if(m_iCarTimeout == 0){
			requestCar();	
		}


		if(m_iOrderTimeout > 0){
			m_iOrderTimeout--;	
		}else if(m_iOrderTimeout == 0){
			WriteLog(QString("WebServer ReSend Order"));
			sendOrder();	
		}
	}
}

void CWebServer::slot_onHttpFinished(CHttpRequest *req)
{
	QByteArray arData;
	int iRet;

	do{
		if(req->error() != CURLE_OK){
			WriteLog(QString("CHttpRequest err = %1").arg(req->errorString()));
			break;
		}

		arData = req->readAll();
		if(arData.length() <= 0)
			break;

		if(req->url() == m_url[IF_TOKEN]){
			if(parseToken(arData, m_strToken, m_iTokenAlive)){
				WriteLog(QString("OK Token = %1, Alive = %2").arg(m_strToken).arg(m_iTokenAlive));

				if(m_iCardTimeout < 0)
					requestCard();

				if(m_iCarTimeout < 0)
					requestCar();
			}
		}else if(req->url() == m_url[IF_CARD]){
			iRet = parseCard(arData);
			switch(iRet){
				case 0:
					m_iCardTimeout = -1;
					break;
				case 1:
					requestCard();
					break;
				default:
					break;
			}
		}else if(req->url() == m_url[IF_CAR]){
			iRet = parseCar(arData);
			switch(iRet){
				case 0:
					m_iCarTimeout = -1;
					break;
				case 1:
					requestCar();
					break;
				default:
					break;
			}
		}else if(req->url() == m_url[IF_ORDER]){
			iRet = parseOrder(arData);
			switch(iRet){
				case 0:
					m_iOrderTimeout = -1;
					m_iSendTimeout = -1;
					WriteLog(QString().sprintf("WebServer Send Order Success SendTimeout = %d", m_iSendTimeout));
					break;
				case 1:
					m_iOrderTimeout = 0;
					break;
				default:
					break;
			}
		}
	}while(false);

	QObject::disconnect(req, SIGNAL(finished(CHttpRequest *)), this, SLOT(slot_onHttpFinished(CHttpRequest *)));
	delete req;
}

void CWebServer::request(QByteArray &arData, INTERFACE_TYPE type)
{
	QByteArray arJson;

	if(m_bTerminate)
		return;

	if(arData.length() <= 0)
		return;

	createRequestJson(arData, arJson);
	if(arJson.length() <= 0){
		WriteLog(QString("Encrypt Data Error!"));
		return;
	}

	WriteLog(QString("Request type = %1 With Json = %2").arg(type).arg(QString(arData)));

	switch(type){
		case IF_CARD:
			m_iCardTimeout = REQ_CARD_TIMEOUT;
			break;
		case IF_CAR:
			m_iCarTimeout = REQ_CAR_TIMEOUT;
			break;
		case IF_ORDER:
			m_iOrderTimeout = REQ_ORDER_TIMEOUT;
			break;
		default:
			break;
	}

	CHttpRequest *req = new CHttpRequest(m_strToken);
	QObject::connect(req, SIGNAL(finished(CHttpRequest *)), this, SLOT(slot_onHttpFinished(CHttpRequest *)));
	req->post(m_url[type], arJson);
}

void CWebServer::aesEncrypt(QByteArray arData, QByteArray &arEncrypt, bool enc)
{
	uchar *buff;
	int iLen;

	if(arData.length() < 0)
		return;
	
	buff = (uchar *)malloc(arData.length() + AES_BLOCK_SIZE + 1);
	iLen = AES_cbc_encrypt((uchar *)arData.data(), buff, arData.length(), 
			(uchar *)m_strKey.toAscii().data(),
			(uchar *)m_strIv.toAscii().data(), enc);
	if(iLen < 0){
		free(buff);
		return;
	}
	
	arEncrypt.append((char *)buff, iLen);
	free(buff);
}

void CWebServer::signatureData(QString key, QString secret, QByteArray &arSig)
{
	int text_length;
	QByteArray K;
	int K_length;

	K_length = secret.size();
	text_length = key.size();

	K = secret.toAscii();

	QByteArray ipad;
	QByteArray opad;

	if (K_length > 64) {
		QByteArray tempSecret;

		tempSecret.append(secret);

		K = QCryptographicHash::hash(tempSecret, QCryptographicHash::Md5);
		K_length = 20;
	}

	ipad.fill(0, 64);
	opad.fill(0, 64);

	ipad.replace(0, K_length, K);
	opad.replace(0, K_length, K);

	for (int i = 0; i < 64; i++) {
		ipad[i] = ipad[i] ^ 0x36;
		opad[i] = opad[i] ^ 0x5c;
	}

	QByteArray context;

	context.append(ipad, 64);
	context.append(key);

	QByteArray Sha1 = QCryptographicHash::hash(context, QCryptographicHash::Md5);

	context.clear();
	context.append(opad, 64);
	context.append(Sha1);

	arSig = QCryptographicHash::hash(context, QCryptographicHash::Md5);
}

void CWebServer::createRequestJson(QByteArray &arData, QByteArray &arJson)
{
	struct json_object *obj;
	QByteArray arEncrypt, arSig;
	QString strTime, strReq, strSig;

	if(arData.length() <= 0)
		return;

	aesEncrypt(arData, arEncrypt, AES_ENCRYPT);
	if(arEncrypt.length() <= 0)
		return;

	strTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
	strReq.sprintf("%04d", m_iRequest);
	m_iRequest++;
	if(m_iRequest > REQ_MAX_NUMBER)
		m_iRequest = 1;

	obj = json_object_new_object();
	json_object_object_add(obj, "OperatorID", json_object_new_string(m_strOperateId.toAscii().data()));
	json_object_object_add(obj, "Data", json_object_new_string(arEncrypt.toBase64().data()));
	json_object_object_add(obj, "TimeStamp", json_object_new_string(strTime.toAscii().data()));
	json_object_object_add(obj, "Seq", json_object_new_string(strReq.toAscii().data()));
	strSig.sprintf("%s%s%s%s", m_strOperateId.toAscii().data(), arEncrypt.toBase64().data(), strTime.toAscii().data(), strReq.toAscii().data());
	signatureData(strSig, m_strDataSecret, arSig);
	json_object_object_add(obj, "Sig", json_object_new_string(arSig.toHex().toUpper().data()));

	arJson.clear();
	arJson.append(json_object_to_json_string(obj));
}

bool CWebServer::parseResponseJson(QByteArray &arJson, QByteArray &arData)
{
	struct json_object *obj, *sub_obj;
	QString strRet, strData, strMsg, strSig;

	obj = json_tokener_parse(arJson.data());
	if(is_error(obj)){
		WriteLog(QString("Respons Parse Error With Json=%1").arg(QString(arJson)));
		return false;
	}

	json_object_object_get_ex(obj, "Ret", &sub_obj);
	strRet = json_object_get_string(sub_obj);

	if(strRet == "200"){
		json_object_object_get_ex(obj, "Sig", &sub_obj);
		strSig = json_object_get_string(sub_obj);
		json_object_object_get_ex(obj, "Data", &sub_obj);
		strData = json_object_get_string(sub_obj);

		arData.clear();
		aesEncrypt(QByteArray::fromBase64(strData.toAscii()), arData, AES_DECRYPT);
		if(arData.length() <= 0){
			WriteLog(QString("Decrypt Error With Json=%1").arg(QString(arJson)));
			return false;
		}

		return true;
	}else if(strRet == "004"){
		WriteLog(QString("Response Error With Json = %2").arg(QString(arJson)));
		m_strToken = "";
		m_iTokenAlive = 0;
		return false;
	}else{
		WriteLog(QString("Respons Error With Json = %2").arg(QString(arJson)));
		return false;
	}
}

void CWebServer::requestToken()
{
	struct json_object *obj;
	QByteArray arJson;
	QString strSig, strTime;

	obj = json_object_new_object();
	json_object_object_add(obj, "OperatorID", json_object_new_string(m_strOperateId.toAscii().data()));
	json_object_object_add(obj, "OperatorSecret", json_object_new_string(m_strOperateSecret.toAscii().data()));

	arJson.append(json_object_to_json_string(obj));
	json_object_put(obj);

	request(arJson, IF_TOKEN);
}

bool CWebServer::parseToken(QByteArray &arJson, QString &strToken, int &iAlive)
{
	struct json_object *obj, *sub_obj;
	QByteArray arData;

	if(!parseResponseJson(arJson, arData))
		return false;

	obj = json_tokener_parse(arData.data());
	if(is_error(obj)){
		WriteLog(QString("parseToken With Json=%1").arg(QString(arJson)));
		return false;
	}

	json_object_object_get_ex(obj, "Token", &sub_obj);
	strToken = json_object_get_string(sub_obj);
	json_object_object_get_ex(obj, "TokenAvailableTime", &sub_obj);
	iAlive = json_object_get_int(sub_obj); 
	if(strToken.isEmpty() || iAlive <= 0){
		strToken = "";
		iAlive = 0;
		return false;
	}

	return true;
}

void CWebServer::requestCard()
{
	struct json_object *obj;
	QByteArray arJson;
	struct db_result_st result;
	QString strSql, strTime, strSig, strTimeStamp;

	strSql = QString("SELECT card_update FROM table_update_time;");
	if(m_pDatabase->DBSqlQuery(strSql.toAscii().data(), &result, DB_AUTHENTICATION) != 0)
		return;

	if(result.row > 0)
		strTime = result.result[0];

	m_pDatabase->DBQueryFree(&result);

	obj = json_object_new_object();
	json_object_object_add(obj, "ctrlAddr", json_object_new_string(m_strStation.toAscii().data()));
	json_object_object_add(obj, "dateTime", json_object_new_string(strTime.toAscii().data()));
	json_object_object_add(obj, "page", json_object_new_int(m_iCardPage));
	json_object_object_add(obj, "size", json_object_new_int(PAGE_SIZE));

	arJson.append(json_object_to_json_string(obj));
	json_object_put(obj);

	request(arJson, IF_CARD);
}

int CWebServer::parseCard(QByteArray &arJson)
{
	struct json_object *obj, *sub_obj;
	QByteArray arData;
	QString strSql, strId, strCode, strTime;
	int len, is_del;

	if(!parseResponseJson(arJson, arData))
		return -1;

	obj = json_tokener_parse(arData.data());
	if(is_error(obj)){
		WriteLog(QString("parseCard Parse Error With Json=%1").arg(QString(arData)));
		return -1;
	}

	json_object_object_get_ex(obj, "dataTime", &sub_obj);
	strTime = json_object_get_string(sub_obj);
	json_object_object_get_ex(obj, "data", &sub_obj);
	len = json_object_array_length(sub_obj);

	/*
	if(m_pDatabase->DBSqlExec(DB_BEGIN, DB_AUTHENTICATION) != 0){
		WriteLog(QString("Database Error With Sql=%1").arg(QString(DB_BEGIN)));
		return -1;
	}
	*/

	WriteLog(QString("Card Count = %1").arg(len));

	for(int i = 0; i < len; i++){
		json_object *array, *val;
		array = json_object_array_get_idx(sub_obj, i);
		json_object_object_get_ex(array, "cardID", &val);
		strId = json_object_get_string(val);
		json_object_object_get_ex(array, "cardCode", &val);
		strCode  = json_object_get_string(val);
		json_object_object_get_ex(array, "isDel", &val);
		is_del = json_object_get_int(val);

		if(is_del == 0){
			strSql = QString("INSERT INTO table_card_authentication (card_id, card_code, is_delete) VALUES ('%1', '%2', %3);")
				.arg(strId).arg(strCode).arg(is_del);
		}else{
			strSql = QString("DELETE FROM table_card_authentication WHERE card_id = '%1';").arg(strId);
		}

		if(m_pDatabase->DBSqlExec(strSql.toAscii().data(), DB_AUTHENTICATION) != 0){
			WriteLog(QString("Database Error With Sql=%1").arg(strSql));
			return -1;
		}
	}

	/*
	if(m_pDatabase->DBSqlExec(DB_COMMIT, DB_AUTHENTICATION) != 0){
		m_pDatabase->DBSqlExec(DB_ROLLBACK, DB_AUTHENTICATION);
		WriteLog(QString("Database Error With Sql=%1").arg(QString(DB_COMMIT)));
		return 1;
	}
	*/

	if(strTime != ""){
		struct db_result_st result;

		strSql = QString("SELECT id FROM table_update_time;");

		m_pDatabase->DBSqlQuery(strSql.toAscii().data(), &result, DB_AUTHENTICATION);
		if(result.row <= 0)
			strSql = QString("INSERT INTO table_update_time (card_update) VALUES ('%1');").arg(strTime);
		else
			strSql = QString("UPDATE table_update_time SET card_update = '%1';").arg(strTime);

		m_pDatabase->DBQueryFree(&result);
		m_pDatabase->DBSqlExec(strSql.toAscii().data(), DB_AUTHENTICATION);
	}else{
		WriteLog(QString("Card Request Finish With Len = %1").arg(len));
	}

	if(len < PAGE_SIZE)
		return 0;

	return 1;
}

void CWebServer::requestCar()
{
	struct json_object *obj;
	QByteArray arJson;
	struct db_result_st result;
	QString strSql, strTime, strSig;

	strSql = QString("SELECT car_update FROM table_update_time;");
	if(m_pDatabase->DBSqlQuery(strSql.toAscii().data(), &result, DB_AUTHENTICATION) != 0)
		return;

	if(result.row > 0)
		strTime = result.result[0];

	m_pDatabase->DBQueryFree(&result);

	obj = json_object_new_object();
	json_object_object_add(obj, "ctrlAddr", json_object_new_string(m_strStation.toAscii().data()));
	json_object_object_add(obj, "dateTime", json_object_new_string(strTime.toAscii().data()));
	json_object_object_add(obj, "page", json_object_new_int(m_iCarPage));
	json_object_object_add(obj, "size", json_object_new_int(PAGE_SIZE));

	arJson.append(json_object_to_json_string(obj));
	json_object_put(obj);

	request(arJson, IF_CAR);
}

int CWebServer::parseCar(QByteArray &arJson)
{
	struct json_object *obj, *sub_obj;
	QString strSql, strId, strVin, strNo, strTime;
	QByteArray arData;
	int len, is_del, pri;

	if(!parseResponseJson(arJson, arData))
		return -1;

	obj = json_tokener_parse(arData.data());
	if(is_error(obj)){
		WriteLog(QString("parseCar Parse Error With Json=%1").arg(QString(arData)));
		return -1;
	}

	json_object_object_get_ex(obj, "dataTime", &sub_obj);
	strTime = json_object_get_string(sub_obj);
	json_object_object_get_ex(obj, "data", &sub_obj);
	len = json_object_array_length(sub_obj);

	/*
	if(m_pDatabase->DBSqlExec(DB_BEGIN, DB_AUTHENTICATION) != 0){
		WriteLog(QString("Database Error With Sql=%1").arg(QString(DB_BEGIN)));
		return -1;
	}
	*/

	WriteLog(QString("Car Count = %1").arg(len));

	for(int i = 0; i < len; i++){
		json_object *array, *val;
		array = json_object_array_get_idx(sub_obj, i);
		json_object_object_get_ex(array, "carID", &val);
		strId = json_object_get_string(val);
		json_object_object_get_ex(array, "carVIN", &val);
		strVin  = json_object_get_string(val);
		json_object_object_get_ex(array, "PRI", &val);
		pri = json_object_get_int(val);
		json_object_object_get_ex(array, "carNo", &val);
		strNo = json_object_get_string(val);
		json_object_object_get_ex(array, "isDel", &val);
		is_del = json_object_get_int(val);

		if(is_del == 0){
			strSql = QString("INSERT INTO table_car_authentication (car_id, car_vin, priority, car_no, is_delete) \
					VALUES ('%1', '%2', %3, '%4', %5);").arg(strId).arg(strVin).arg(pri).arg(strNo).arg(is_del);
		}else{
			strSql = QString("DELETE FROM table_car_authentication WHERE car_id = '%1';").arg(strId);
		}
		if(m_pDatabase->DBSqlExec(strSql.toAscii().data(), DB_AUTHENTICATION) != 0){
			WriteLog(QString("Database Error With Sql=%1").arg(strSql));
			return -1;
		}
	}

	/*
	if(m_pDatabase->DBSqlExec(DB_COMMIT, DB_AUTHENTICATION) != 0){
		m_pDatabase->DBSqlExec(DB_ROLLBACK, DB_AUTHENTICATION);
		WriteLog(QString("Database Error With Sql=%1").arg(QString(DB_COMMIT)));
		return 1;
	}
	*/

	if(strTime != ""){
		struct db_result_st result;

		strSql = QString("SELECT id FROM table_update_time;");
		m_pDatabase->DBSqlQuery(strSql.toAscii().data(), &result, DB_AUTHENTICATION);

		if(result.row <= 0)
			strSql = QString("INSERT INTO table_update_time (car_update) VALUES ('%1');").arg(strTime);
		else
			strSql = QString("UPDATE table_update_time SET car_update = '%1';").arg(strTime);
		m_pDatabase->DBQueryFree(&result);

		if(m_pDatabase->DBSqlExec(strSql.toAscii().data(), DB_AUTHENTICATION) != 0){
			WriteLog(QString("Database Error With Sql=%1").arg(strSql));
			return -1;
		}
	}else{
		WriteLog(QString("Car Request Finish With Len = %1").arg(len));
	}

	if(len < PAGE_SIZE){
		return 0;
	}

	return 1;
}

void CWebServer::sendOrder()
{
	struct json_object *main_obj, *obj, *sub_obj, *array, *detail;
	QByteArray arJson;
    QString str, strSql, strUUID, strEnergy, strCarNo, strBegin, strEnd,strcustomerID;
	int iStatus, iCanAddr;
	uint uiBeginEnergy, uiEndEnergy;
	sqlite3 *db, *db1;
	int row = 0, col = 0;
	int row1 = 0, col1 = 0;
	char **result = NULL, **result1 = NULL;

	if(!QFile::exists(QString(DB_EMERGENCY_ORDER_PATH)))
		return;

	main_obj = json_object_new_object();
	array = json_object_new_array();

	sqlite3_open((char *)DB_EMERGENCY_ORDER_PATH, &db);

	strSql = QString("SELECT UUIDOwn, EventNo, OrderStatus,StartTime, EndTime, \
            StartSoc, StopSoc, StartEnergy, EndEnergy, CanAddr, CardNo, customerID,VIN, \
			CarLisence, ChargeType, ChargeWay, DevStopReason, QueueIndex, GunNum \
			FROM charge_order WHERE OrderType = %1 AND OrderStatus >= %2 AND \
			ChargeType > 0 AND OrderSync < 2 ORDER BY OrderStatus DESC, OrderSync ASC;")
			 .arg(ORDER_EMERGENCY).arg(ORDER_STATUS_FAIL);
	if(sqlite3_get_table(db, strSql.toAscii().data(), &result, &row, &col, NULL) == SQLITE_OK){
		for(int i = 0; i < row; i++){
			obj = json_object_new_object();
			strUUID = result[col++];
			json_object_object_add(obj, "ID", json_object_new_string(strUUID.toAscii().data()));
			str = result[col++];
			if(str == "000000000000000000")
				str = "";
			json_object_object_add(obj, "billCode", json_object_new_string(str.toAscii().data()));
			iStatus = atoi(result[col++]);
			switch(iStatus){
				case ORDER_STATUS_ING:
					iStatus = 1;
					break;
				case ORDER_STATUS_QUEUE:
					iStatus = 2;
					break;
				case ORDER_STATUS_OK:
				case ORDER_STATUS_FAIL:
					iStatus = 3;
					break;
				default:
					iStatus = 0;
					break;
			}
			strBegin = result[col++];
			json_object_object_add(obj, "beginTime", json_object_new_string(strBegin.toAscii().data()));
			strEnd = result[col++];
			json_object_object_add(obj, "endTime", json_object_new_string(strEnd.toAscii().data()));
			json_object_object_add(obj, "beginSOC", json_object_new_string(result[col++]));
			json_object_object_add(obj, "endSOC", json_object_new_string(result[col++]));
			uiBeginEnergy = atoi(result[col++]);
			strEnergy.sprintf("%0.2f", (double)uiBeginEnergy / 100.0);
			json_object_object_add(obj, "beginPower", json_object_new_string(strEnergy.toAscii().data()));
			uiEndEnergy = atoi(result[col++]);
			strEnergy.sprintf("%0.2f", (double)uiEndEnergy / 100.0);
			json_object_object_add(obj, "endPower", json_object_new_string(strEnergy.toAscii().data()));
			json_object_object_add(obj, "ctrlAddr", json_object_new_string(m_strStation.toAscii().data()));
			iCanAddr = atoi(result[col++]);
			json_object_object_add(obj, "canSN", json_object_new_int(iCanAddr));
			json_object_object_add(obj, "cardNo", json_object_new_string(result[col++]));
            strcustomerID = result[col++];
            if(strcustomerID.length() > 10)
            {
                strcustomerID.insert(20,'-');
                strcustomerID.insert(16,'-');
                strcustomerID.insert(12,'-');
                strcustomerID.insert(8,'-');
            }
            json_object_object_add(obj, "customerID", json_object_new_string(strcustomerID.toAscii().data()));
            str = result[col++];
			json_object_object_add(obj, "carVIN", json_object_new_string(str.toAscii().data()));
			str = result[col++];
			if(str != "00000000000000" && str != ""){
				if(!QueryCarLisenceName(QByteArray::fromHex(str.toAscii()).data(), strCarNo)){
					strCarNo = str;
				}
			}else{
                strCarNo = "";
			}
			json_object_object_add(obj, "carNo", json_object_new_string(strCarNo.toAscii().data()));
			json_object_object_add(obj, "chargeType", json_object_new_int(atoi(result[col++])));
			json_object_object_add(obj, "chargeWay", json_object_new_int(atoi(result[col++])));
			json_object_object_add(obj, "endReason", json_object_new_int(atoi(result[col++])));
			strEnergy.sprintf("%0.2f", (double)(uiEndEnergy - uiBeginEnergy) / 100.0);
			json_object_object_add(obj, "power", json_object_new_string(strEnergy.toAscii().data()));
			json_object_object_add(obj, "limitPower", json_object_new_string(""));
			json_object_object_add(obj, "status", json_object_new_int(iStatus));
			json_object_object_add(obj, "orderNum", json_object_new_int(atoi(result[col++])));
			json_object_object_add(obj, "gunNum", json_object_new_int(atoi(result[col++])));

			detail = json_object_new_array();
			if(QFile::exists(QString(DB_EMERGENCY_REAL_PATH))){
				sqlite3_open((char *)DB_EMERGENCY_REAL_PATH, &db1);

				strSql.sprintf("SELECT NowTime, NowEnergy FROM charge_energy_%d_table WHERE UUIDOwn = \'%s\' ORDER BY NowTime ASC;",
						iCanAddr, strUUID.toAscii().data());
				if(sqlite3_get_table(db1, strSql.toAscii().data(), &result1, &row1, &col1, NULL) == SQLITE_OK){
					for(int j = 1; j < row1; j++){
						sub_obj = json_object_new_object();
						strBegin = result1[j * col1];
						uiBeginEnergy = atoi(result1[j * col1 + 1]);
						strEnd = result1[(j + 1) * col1];
						uiEndEnergy = atoi(result1[(j + 1) * col1 + 1]);
						strEnergy.sprintf("%0.2f", (double)(uiEndEnergy - uiBeginEnergy) / 100.0);
						json_object_object_add(sub_obj, "beginTime", json_object_new_string(strBegin.toAscii().data()));
						json_object_object_add(sub_obj, "endTime", json_object_new_string(strEnd.toAscii().data()));
						json_object_object_add(sub_obj, "power", json_object_new_string(strEnergy.toAscii().data()));
						json_object_array_add(detail, sub_obj);
					}
					sqlite3_free_table(result1);
				}

				sqlite3_close(db1);
			}
			json_object_object_add(obj, "billDetails", detail);
			json_object_array_add(array, obj);
		}

		sqlite3_free_table(result);
	}
	sqlite3_close(db);

	json_object_object_add(main_obj, "ctrlAddress", json_object_new_string(m_strStation.toAscii().data()));
	json_object_object_add(main_obj, "bills", array);

	arJson.append(json_object_to_json_string(main_obj));
	json_object_put(main_obj);

	request(arJson, IF_ORDER);
}

int CWebServer::parseOrder(QByteArray &arJson)
{
	struct json_object *obj, *sub_obj;
	QByteArray arData;
	QString strRet, strMsg, strId;

	if(!parseResponseJson(arJson, arData))
		return -1;

	obj = json_tokener_parse(arData.data());
	if(is_error(obj)){
		WriteLog(QString("Parse Error With Json=%1").arg(QString(arData)));
		return -1;
	}

	WriteLog(QString("Order Response = %1").arg(QString(arData)));

	json_object_object_get_ex(obj, "resultCode", &sub_obj);
	strRet = json_object_get_string(sub_obj);
	json_object_object_get_ex(obj, "resultMsg", &sub_obj);
	strMsg = json_object_get_string(sub_obj);
	json_object_object_get_ex(obj, "ID", &sub_obj);
	strId = json_object_get_string(sub_obj);

	if(strRet != "255"){
		WriteLog(QString("Order Upload Failed!"));
		return 1;
	}

	WriteLog(QString("Order Upload Scuccess!"));

	InfoMap map;
	map[Addr_Local_Emergency] = QByteArray(1, 0x1);
	emit sigToBus(map, AddrType_Emergency_Order_Result);

	return 0;
}

void CWebServer::WriteLog(QString strLog, int iLevel)
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

void CWebServer::ParseWebUrl(InfoMap &mapInfo)
{
	QList<QByteArray> list;

	if(mapInfo.contains(Addr_Web_Url)){
		list = mapInfo[Addr_Web_Url].split(':');
		if(list.count() >= 3){
			m_strUrl = list.at(0) + ":" + list.at(1);	
			m_iPort = list.at(2).toInt();
		}
	}

	if(m_strUrl.isEmpty() || m_iPort <= 0){
		//InfoMap map;
		//emit sigToBus(map, AddrType_Web_Url_Request);
		//WriteLog(QString("Send Get Address Signal"));
		WriteLog(QString("Get Address Failed Host=%1 Port=%2").arg(m_strUrl).arg(m_iPort));
	}else{
		for(int i = 0; i < IF_COUNT; i++){
			m_url[i].setUrl(m_strUrl);
			m_url[i].setPort(m_iPort);
			m_url[i].setPath(QString(interface[i]));
		}

		WriteLog(QString("Get Address Success Host=%1 Port=%2").arg(m_strUrl).arg(m_iPort));
	}
}
