#include "NetAddress.h"
#include <QDateTime>
#include <QTimerEvent>

#define HOST_ADDR1 "route.teld.cn:8998"
#define HOST_ADDR2 "route.teld.net:8998"
#define HOST_ADDR3 "route.teld.com:8998"

//帧属性
#define FRAME_HEADER		0x68	//帧头
#define DEVICE_CTRL			0x1		//集控器
#define DEVICE_SINGLE		0x2		//单桩

//帧类型
#define FRAME_ADDR_REQ		0x02	//请求帧
#define FRAME_ADDR_REP		0x82	//应答帧
#define FRAME_ERROR			0xFF	//错误帧

//信息体类型
#define INFO_CTRL_ADDR		0x1		//集控器地址
#define INFO_CTRL_KEY		0x2		//集控器秘钥
#define INFO_CTRL_VERSION	0x3		//集控器软件版本
#define INFO_PROTO_VERSION	0x4		//集控通信协议版本
#define INFO_CHARGE_ADDR	0x11	//通信协议充电地址
#define INFO_WEB_ADDR		0x12	//通信协议WEB地址
#define INFO_MONITOR_ADDR	0x13	//通信协议智能运维地址
#define INFO_SETTING_ADDR	0x14	//通信协议配置地址

CNetAddress::CNetAddress()
{
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");

	_strLogName = "netaddress";

	m_pSetting = ParamSet::GetInstance();
	m_pDevCache = DevCache::GetInstance();
	m_pFilter = RealDataFilter::GetInstance();
    m_pLog = Log::GetInstance();
	m_pDatabase = DBOperate::GetInstance();

	m_iLocalAddress = 0;
	m_iServerTimer = -1;
	m_iHostIndex = -1;
	m_iTryConnect = 0;
	m_State = WAITING;
	m_iServerTimeout = 0;
	m_strHost = "";
	m_iPort = 0;

    stServer0Config confServer;
	memset(&confServer, 0, sizeof(stServer0Config));
	m_pSetting->querySetting(&confServer, PARAM_SERVER0);
	m_strStation = confServer.stationNo;
	m_strKey = confServer.aesKey;

	stCSCUSysConfig confSys;
	memset(&confSys, 0, sizeof(stCSCUSysConfig));
	m_pSetting->querySetting(&confSys, PARAM_CSCU_SYS);
	m_strVersion = confSys.version;

	ExtraConfig confExtra;
	memset(&confExtra, 0, sizeof(ExtraConfig));
	m_pSetting->querySetting(&confExtra, PARAM_EXTRA);
	m_iLocalAddress = confExtra.localAddress;

	WriteLog(QString("集控器地址:%1 集控器秘钥:%2 集控器版本:%3 远程获取地址:%4")
			.arg(m_strStation).arg(m_strKey).arg(m_strVersion).arg(m_iLocalAddress > 0 ? "关闭" : "开启"));

	QString str;
	int addrCnt = 0;
	if(strcmp(confServer.serverIp1, "") != 0 && confServer.serverPort1 > 0 && confServer.serverPort1 <= 65535)
		addrCnt++;
	str.sprintf("%s:%d", confServer.serverIp1, confServer.serverPort1 );
	m_listHost.append(str);

	if(strcmp(confServer.serverIp2, "") != 0 && confServer.serverPort2 > 0 && confServer.serverPort2 <= 65535)
		addrCnt++;
	str.sprintf("%s:%d", confServer.serverIp2, confServer.serverPort2);
	m_listHost.append(str);

	if(strcmp(confServer.serverIp3, "") != 0 && confServer.serverPort3 > 0 && confServer.serverPort3 <= 65535)
		addrCnt++;
	str.sprintf("%s:%d", confServer.serverIp3, confServer.serverPort3);
	m_listHost.append(str);

	if(addrCnt <= 0){
		if(!m_iLocalAddress){
			WriteLog(QString("未配置地址，加载默认地址列表!"));
			m_listHost.clear();
			m_listHost.append(HOST_ADDR1);
			m_listHost.append(HOST_ADDR2);
			m_listHost.append(HOST_ADDR3);
		}
	}

	for(int i = 0; i < m_listHost.count(); i++){
		WriteLog(QString().sprintf("已加载连接地址%d %s", i + 1, m_listHost.at(i).toAscii().data()));
	}

	m_pSocket = new QTcpSocket();
	connect(m_pSocket, SIGNAL(connected()), this, SLOT(slot_onConnected()));
	connect(m_pSocket, SIGNAL(disconnected()), this, SLOT(slot_onDisconnected()));
	connect(m_pSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slot_onError(QAbstractSocket::SocketError)));
	connect(m_pSocket, SIGNAL(readyRead()), this, SLOT(slot_onReceive()));
}

CNetAddress::~CNetAddress()
{
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
 * 动态库接口函数，创建模块实例
 * pDepends 输入 公共模块指针列表
 * 返回值 	成功返回模块实例，失败返回NULL
 */
CModuleIO* CreateDevInstance()
{
	CModuleIO* pModule = new CNetAddress();
	
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
 * 动态库接口函数，初始化模块
 * pThread	输入 模块将要运行的线程
 * 返回值 	0表示无错误，-1表示有错误
 */
int CNetAddress::InitModule(QThread* pThread)
{
	m_pWorkThread = pThread;
	return 0;
}

/*
 * 动态库接口函数，向BUS注册模块
 * pBus		输入 BUS模块指针
 * 返回值 	0表示无错误，-1表示有错误
 */
int CNetAddress::RegistModule()
{
	QList<int> list;

	list.append(AddrType_Comm_Addr_Request);//通信地址获取

	CBus::GetInstance()->RegistDev(this, list);

	return 0;
}

/*
 * 动态库接口函数，启动模块
 * 返回值 	0表示无错误，-1表示有错误
 */
int CNetAddress::StartModule()
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
int CNetAddress::StopModule()
{
	m_pSocket->close();

	WriteLog(QString("模块退出成功"));

	return 0;
}

/*
 * 动态库接口函数，模块工作状态
 * 返回值 	0表示无错误，-1表示有错误
 */
int CNetAddress::ModuleStatus()
{
	return 0;
}

/*
 * 线程启动槽函数
 *
 * 参数		无
 * 返回值	无
 */
void CNetAddress::slot_onThreadRun()
{
    m_iServerTimer = startTimer(1000);

	WriteLog(QString("模块启动成功"));
}

/*
 * QTcpSocket连接成功槽函数
 * 参数		无
 * 返回值	无
 */
void CNetAddress::slot_onConnected()
{
	WriteLog(QString("连接成功:%1:%2").arg(m_strHost).arg(m_iPort));

	m_iTryConnect = 0;
	m_arData.clear();

	RequestAddress();

	m_State = REQUESTING;
	m_iServerTimeout = 60;
}

/*
 * QTcpSocket连接断开槽函数，关闭心跳计时器，
 * 重置连接状态，并处理客户端主动断开情况下的网络重连
 *
 * 参数		无
 * 返回值	无
 */
void CNetAddress::slot_onDisconnected()
{
	WriteLog(QString("断开连接:%1:%2").arg(m_strHost).arg(m_iPort));

	m_arData.clear();
	m_State = DISCONNECT;
	m_iServerTimeout = 10;
}

/*
 * QTcpSocket网络错误槽函数，设置网络状态，处理链接未建立情况下的网络重连
 *
 * err		输入 socket错误类型	
 * 返回值	无
 */
void CNetAddress::slot_onError(QAbstractSocket::SocketError err)
{
	Q_UNUSED(err);
	WriteLog(QString("QTcpSocket: %1").arg(m_pSocket->errorString()));

	m_arData.clear();
	m_State = DISCONNECT;
	m_iServerTimeout = 10;
}

/*
 * QTcpSocket数据接收槽函数，每次信号触发时
 * 读出缓冲区内数据，检查网络帧是否符合协议
 * 不符合丢弃，符合进行进一步解析
 *
 * 参数		无
 * 返回值	无
 */
void CNetAddress::slot_onReceive()
{
	Reply *rep;
	uint iCacheLen, iPackageLen;

	m_arData.append(m_pSocket->readAll());

	iCacheLen = m_arData.length();

	if(iCacheLen >= sizeof(Reply)){
		rep = (Reply *)m_arData.data();		
		iPackageLen = rep->hdr.len;
		if(rep->hdr.header != FRAME_HEADER){
			m_arData.clear();
			m_pSocket->close();
			m_State = DISCONNECT;
			return;
		}

		if(iCacheLen < iPackageLen){
			return;
		}

		ParseFrame(m_arData.left(iPackageLen + 3).data(), iPackageLen + 3);
		m_arData.clear();
		m_pSocket->close();
		m_State = WAITING;
	}
}

/*
 * 接收其它模块消息
 * mapInfo 	输入 信息体集合
 * type    	输入 消息类型
 * 返回值	无
 */
void CNetAddress::slotFromBus(InfoMap mapInfo, InfoAddrType type)
{
	Q_UNUSED(mapInfo);

	switch(type){
		case AddrType_Comm_Addr_Request:
			if(!m_iLocalAddress){
				if(m_State == WAITING){
					m_State = DISCONNECT;
					WriteLog(QString("收到总线获取地址请求"));
					return;
				}
				WriteLog(QString("正在获取地址，忽略总线请求"));
			}else{
				WriteLog(QString("收到总线获取地址请求，发送本地地址"));
				sendLoaclAddress();
			}
			break;
		default:
			break;
	}
}

/*
 * 获取通信地址请求
 *
 * 参数		无
 * 返回值	无
 */
void CNetAddress::RequestAddress()
{
	FrameHdr hdr;
	QByteArray arPayload, arFrame;

	memset(&hdr, 0, sizeof(FrameHdr));

	hdr.header = FRAME_HEADER;
	hdr.len = sizeof(FrameHdr) - 3;
	hdr.type = FRAME_ADDR_REQ;
	hdr.value = DEVICE_CTRL;
	hdr.time = QDateTime::currentDateTime().toTime_t();

	char buff[22];
	memset(buff, 0, 22);
	snprintf(buff, 21, "%c%s", INFO_CTRL_ADDR, m_strStation.toAscii().data());
	arPayload.append(buff, 21);

	memset(buff, 0, 22);
	snprintf(buff, 21, "%c%s", INFO_CTRL_KEY, m_strKey.toAscii().data());
	arPayload.append(buff, 21);

	memset(buff, 0, 22);
	snprintf(buff, 21, "%c%s", INFO_CTRL_VERSION, m_strVersion.toAscii().data());
	arPayload.append(buff, 21);

	memset(buff, 0, 22);
	snprintf(buff, 21, "%c%s", INFO_PROTO_VERSION, "3.0");
	arPayload.append(buff, 21);

	hdr.len += arPayload.length();
	arFrame.append((char *)&hdr, sizeof(FrameHdr));
	arFrame.append(arPayload);

	QString strLog = "[发送]";
	for(int i = 0; i < arFrame.length(); i++){
		QString str;
		str.sprintf("%02x ", arFrame.at(i));
		strLog += str;
	}

	WriteLog(strLog);		

	m_pSocket->write(arFrame.data(), arFrame.length());
}

/*
 * 解析数据帧
 *
 * 参数		data：接收数据 len：接收数据长度
 * 返回值	无
 */
void CNetAddress::ParseFrame(char *data, uint len)
{
	Reply *rep;

	rep = (Reply *)data;

	QString strLog = "[接收]";
	char *p = NULL;
	for(p = data; p < data + len; p++){
		QString str;
		str.sprintf("%02x ", *p);
		strLog += str;
	}

	WriteLog(strLog);

	switch(rep->hdr.type){
		case FRAME_ADDR_REP:
			ParseAddress(data, len);
			break;
		case FRAME_ERROR:
			ParseError(data, len);
			break;
		default:
			WriteLog(QString("帧类型错误 %1").arg(rep->hdr.type));
			break;
	}
}

/*
 * 解析通信地址
 *
 * 参数		data：接收数据 len：接收数据长度
 * 返回值	无
 */
void CNetAddress::ParseAddress(char *data, uint len)
{
	QList<QByteArray> list;
	QByteArray ar;
	InfoMap map;
	QDateTime dt;
	Reply *rep;

	rep = (Reply *)data;

	ar.append(rep->data, len - sizeof(Reply));
	dt.setTime_t(rep->hdr.time);
	WriteLog(QString("数据帧时间:%1").arg(dt.toString("yyyy-MM-dd HH:mm:ss")));

	list = ar.split(0);
	if(list.count() <= 0)
		return;

	for(int i = 0; i < list.count(); i++){
		ar = list.at(i);
		if(ar.length() <= 0){
			continue;
		}
		switch(ar.at(0)){
			case INFO_CHARGE_ADDR:
				map[Addr_Comm_Addr] = ar.right(ar.length() - 1);	
				WriteLog(QString("充电协议地址:%1").arg(ar.data()));
				break;
			case INFO_WEB_ADDR:
				map[Addr_Web_Url] = ar.right(ar.length() - 1);	
				WriteLog(QString("WEB服务地址:%1").arg(ar.data()));
				break;
			case INFO_MONITOR_ADDR:
				map[Addr_Monitor_Addr] = ar.right(ar.length() - 1);	
				WriteLog(QString("智能运维地址:%1").arg(ar.data()));
				break;
			case INFO_SETTING_ADDR:
				map[Addr_Setting_Addr] = ar.right(ar.length() - 1);	
				WriteLog(QString("配置协议地址:%1").arg(ar.data()));
				break;
			default:
				WriteLog(QString("信息体错误"));
				break;
		}
	}

	emit sigToBus(map, AddrType_Comm_Addr);
}

/*
 * 解析错误应答
 *
 * 参数		data：接收数据 len：接收数据长度
 * 返回值	无
 */
void CNetAddress::ParseError(char *data, uint len)
{
	Q_UNUSED(len);

	InfoMap map;
	QDateTime dt;
	Reply *rep;

	rep = (Reply *)data;

	dt.setTime_t(rep->hdr.time);

	WriteLog(QString("数据帧时间:%1 错误码:%2").arg(dt.toString("yyyy-MM-dd HH:mm:ss")).arg((int)(rep->hdr.value)));

	map[Addr_Error_Addr] = QByteArray(1, rep->hdr.value);
	emit sigToBus(map, AddrType_Comm_Addr);
}

/*
 * 日志记录
 *
 * 参数		strLog：日志内容 iLevel：日志输出级别
 * 返回值	无
 */
void CNetAddress::WriteLog(QString strLog, int iLevel)
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
 * 从服务器列表中取出地址，进行连接
 */
bool CNetAddress::connectToServer()
{
	QString strHostInfo;
	QStringList list;

	m_strHost = "";
	m_iPort = 0;

	if(m_listHost.count() <= 0){
		WriteLog(QString("地址列表为空"), 3);
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

			WriteLog(QString().sprintf("切换连接地址序号:%d 地址列表数量:%d", m_iHostIndex, m_listHost.count()));
		}
	}

	strHostInfo = m_listHost.at(m_iHostIndex);
	list = strHostInfo.split(":");
	if(list.count() < 2)
		return false;

	m_strHost = list.at(0);
	m_iPort = list.at(1).toInt();

	m_pSocket->connectToHost(m_strHost, m_iPort);
	m_iTryConnect++;
	WriteLog(QString("发起接地址:%1:%2 重连次数:%3").arg(m_strHost).arg(m_iPort).arg(m_iTryConnect));
	return true;
}

/*
 * 定时器事件触发函数
 *
 * 参数		event：定时器事件
 * 返回值	无
 */
void CNetAddress::timerEvent(QTimerEvent *event)
{
	if(event->timerId() == m_iServerTimer){
		if(m_iServerTimeout > 0)
			m_iServerTimeout--;

		switch(m_State){
			case DISCONNECT:
				WriteLog(QString().sprintf("Connect %d", m_iServerTimeout), 1);
				if(m_iServerTimeout <= 0){
					connectToServer();
					m_State = CONNECTING;
					m_iServerTimeout = 60;
				}
				break;
			case CONNECTING:
				WriteLog(QString().sprintf("Connecting %d", m_iServerTimeout), 1);
				if(m_iServerTimeout <= 0){
					m_pSocket->abort();
					m_pSocket->close();
					m_State = DISCONNECT;
				}
				break;
			case REQUESTING:
				WriteLog(QString().sprintf("Requesting %d", m_iServerTimeout), 1);
				if(m_iServerTimeout <= 0){
					m_pSocket->abort();
					m_pSocket->close();
					m_State = DISCONNECT;
				}
				break;
			default:
				break;
		}
	}
}

void CNetAddress::sendLoaclAddress()
{
	InfoMap map;
	QString str;

	if(m_listHost.count() < 3){
		WriteLog(QString("地址列表错误"), 3);
		return;
	}

	str = m_listHost.at(0);
	WriteLog(QString("本地充电通信地址 %1").arg(str));
	map[Addr_Comm_Addr] = str.toAscii();	
	str = m_listHost.at(1);
	WriteLog(QString("本地运维通信地址 %1").arg(str));
	map[Addr_Monitor_Addr] = str.toAscii();	
	str = m_listHost.at(2);
	WriteLog(QString("本地配置通信地址 %1").arg(str));
	map[Addr_Setting_Addr] = str.toAscii();	

	emit sigToBus(map, AddrType_Comm_Addr);
}
