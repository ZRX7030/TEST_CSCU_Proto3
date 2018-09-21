#include "monitorserver.h"
#include "monitorprotocol.h"
#include "datacache.h"
#include "tcpnet.h"
#include <QTimerEvent>
#include <math.h>

MonitorServer::MonitorServer()
{
	qRegisterMetaType<QDataPointList>("QDataPointList");
	_strLogName = "pb_monitorserver";

	writeLog(QString().sprintf("Terminal init ac=%d tac=%d dc=%d", acNum, tacNum, dcNum));
    writeLog(QString().sprintf("Encrypt=%d StationNo=%s Key=%s",
                monitorEncrypt,stationNo.c_str(), cscuKey.c_str()));

	_filter->setRealDataCallBack(onRealData);
	DataCache::createTable();
	connect(this, SIGNAL(signalBurst(QDataPointList)), this, SLOT(slot_onDataBurst(QDataPointList)));
}

MonitorServer::~MonitorServer()
{
	DataCache::saveCache();
	DataCache::destroyTable();
}

/*
 * 动态库接口函数，向BUS注册模块
 * pBus		输入 BUS模块指针
 * 返回值 	0表示无错误，-1表示有错误
 */
int MonitorServer::RegistModule()
{
	QList<int> list;

	list.append(AddrType_FaultState_DCcab);
	list.append(AddrType_CSCU_Alarm);
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
	ProtobufServer::_server = new MonitorServer();

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

void MonitorServer::slot_onDataBurst(QDataPointList burst)
{
	proto->burst(burst);
}

/*
 * 实时数据回调函数
 */
bool MonitorServer::onRealData(InfoMap map, InfoAddrType type)
{
	QDataPointList burst;
	int canAddr;

	//附加设备名称，以更新点表
	switch(type){
		case AddrType_TermSignal://遥信
			if(map.contains(Addr_CanID_Comm)){
				canAddr = map[Addr_CanID_Comm].at(0);

				if((canAddr >= ID_MinACSinCanID && canAddr <= ID_MaxACSinCanID) ||
						(canAddr >= ID_MinACThrCanID && canAddr <= ID_MaxACThrCanID)){
					map.insert(Addr_DevData_Type, QByteArray("acmodular"));
				}
			}
			break;
		case AddrType_TermMeasure://遥测
			if(map.contains(Addr_CanID_Comm)){
				canAddr = map[Addr_CanID_Comm].at(0);
				if((canAddr >= ID_MinACSinCanID && canAddr <= ID_MaxACSinCanID) ||
						(canAddr >= ID_MinACThrCanID && canAddr <= ID_MaxACThrCanID)){
					map.insert(Addr_DevData_Type, QByteArray("acmodular"));
				}
			}
			break;
		case AddrType_FaultState_DCcab://模块故障告警信息
			DataCache::alarm(map, burst);
			if(_server->serverLogged && burst.count() > 0){
				emit ((MonitorServer *)_server)->signalBurst(burst);
			}
			return true;
		case AddrType_ModuleSpecInfo://设备规格信息
			return DataCache::saveDevSpec(map);
		default:
			break;
	}

	//数据更新
	DataCache::update(map, burst);

	if(!_server->serverLogged){
		return false;
	}

	//利用信号的线程安全来进行突发
	if(burst.count() > 0){
		emit ((MonitorServer *)_server)->signalBurst(burst);
	}

	return true;
}

/*
 * 线程启动槽函数
 *
 * 参数		无
 * 返回值	无
 */
void MonitorServer::slot_onThreadRun()
{	
	net = new TcpNet();
	proto = new Monitor::MonitorProtocol(net);

	connect(net, SIGNAL(connected()), proto, SLOT(onNetConnected()));
	connect(net, SIGNAL(disconnect()), proto, SLOT(onNetDisconnected()));
	connect(net, SIGNAL(error(int)), proto, SLOT(onNetError(int)));
	connect(net, SIGNAL(readyRead()), proto, SLOT(onNetReceive()));

	connect(proto, SIGNAL(active(int)), this, SLOT(onProtoActive(int)));
	connect(proto, SIGNAL(live()), this, SLOT(onProtoLive()));
	connect(proto, SIGNAL(parsed(InfoMap, InfoAddrType)), this, SLOT(onProtoParsed(InfoMap, InfoAddrType)));

	stateTimer = startTimer(1000);
}

void MonitorServer::slotFromBus(InfoMap mapInfo, InfoAddrType type)
{
    ProtobufServer::slotFromBus(mapInfo,type);
	if(!serverLogged){
		writeLog(QString().sprintf("From bus when net not ready with type=%d!", type));
		return;
	}

	proto->command(mapInfo, type);
}

void MonitorServer::getNetSetting(string &host, ushort &port, int &encrypt)
{
	host = monitorAddr;
	port = monitorPort;
	encrypt = monitorEncrypt;
}

void MonitorServer::writeLog(QString strLog, int iLevel)
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
