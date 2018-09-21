#ifndef PROTOBUFSERVER_H
#define PROTOBUFSERVER_H

#include "DevCache.h"
#include "RealDataFilter.h"
#include "ParamSet.h"
#include "Log.h"
#include "DBOperate.h"
#include "ModuleIO.h"
#include "ProtobufServer/protocol/protocol.h"
#include <string>
#include <pthread.h>

using namespace std;

//状态机枚举
typedef enum {
	STATE_CHECK = 0,
    STATE_GETADDRESS,
	STATE_OFFLINE,
	STATE_LOGIN,
    STATE_HEART
}PROTOBUF_SERVER_STATE;

//协议类型枚举
typedef enum {
	PROTO_CHARGE = 0,
	PROTO_MONITOR,
	PROTO_COUNT
}PROTO_TYPE;

//订单告警枚举
typedef enum {
	ALARM_NONE = 0,
	BILL_ALARM_CONFIRM = 1,	//订单未回复确认告警
	BILL_ALARM_ENERGY = 2,	//电量计量告警
	BILL_ALARM_TRANSFER = 3,//设备传输错误告警
	BILL_ALARM_OTHER = 4,	//其它告警
}BILL_ALARM;

#define	CHECK_TIMEOUT		60	//数据检查超时时间
#define RECONNECT_TIMEOUT	10	//重连时间间隔
#define LOGIN_TIMEOUT		60	//登录超时时间
#define HEART_TIMEOUT  		20	//心跳超时时间
#define WAIT_ADDR_TIMEOUT   30  //获取通信地址时间

#define BILL_ALARM_SYNC		1
#define BILL_ALARM_ENERGY	2
#define BILL_ALARM_TRANSFER	2

class ProtobufServer : public CModuleIO
{
    Q_OBJECT
public:
	virtual ~ProtobufServer();

	//服务器
	static ProtobufServer *server();
	//指令发往总线
	static void cmdToBus(InfoMap map, InfoAddrType type);
	//服务器实例
	static ProtobufServer *_server;
	//数据缓存
	DevCache* cache();
	//实时数据
	RealDataFilter* filter();
	//配置
	ParamSet* setting();
	//日志
	Log* log();
	//数据库
	DBOperate* database();
	//集控器配置信息
    void getCtrlSetting(string &stationNo, string &key, string &version);
	//终端配置信息
	void getTermSetting(int &ac, int &tac, int &dc);

	//网络配置
	virtual void getNetSetting(string &host, ushort &port, int &encrypt);
	//日志记录
	virtual void writeLog(QString strLog, int iLevel = 2);
	//设置网络状态图标
	virtual void setNetState(bool state);
	//数据有效性检查完毕或超时
	virtual void dataCheckFinished();

	//登录成功标识
	bool serverLogged;

	//生成集控器内部告警信息
	bool makeAlarm(InfoMap map);
    void procCommAddr(InfoMap mapInfo);   //处理通信地址

signals:
    void sigToBus(InfoMap map, InfoAddrType type);

public slots:
	void slot_onThreadRun();
    void slotFromBus(InfoMap mapInfo, InfoAddrType type);

	//协议链路激活
	void onProtoActive(int logged);
	//协议链路状态
	void onProtoLive();
	//协议解析完成
	void onProtoParsed(InfoMap map, InfoAddrType type);

public:
	//模块导出接口
    int InitModule(QThread* pThread);
    int RegistModule();
    int StartModule();
    int StopModule();
    int ModuleStatus();

protected:
    ProtobufServer();

	//定时器
	void timerEvent(QTimerEvent *event);
	//状态机
	void stateMachine();

	//状态机当前状态
	PROTOBUF_SERVER_STATE serverState;
	//状态超时时间
	int stateTimeout;
	//心跳计数
	int heartCnt;
	//计时器id
	int stateTimer;
	//离线时间
	int offlineTime;
    int billAckTimer; //定时器id - 订单上传超时定时器
    InfoMap recvBillMap;
    int billAckTimerCpunt;  //计数
    QString qstrRecvProtoMessage;
    //登录超时计数
    int loginTimeoutCnt;

	//终端数量
	int acNum;
	int tacNum;
	int dcNum;

	//配置参数
	string stationNo;
	string cscuKey;
	string chargeAddr;
	ushort chargePort; 
	int chargeEncrypt;
	string monitorAddr;
	ushort monitorPort; 
	int monitorEncrypt;
    string ctrlSwVersion;

	//协议
	Protocol *proto;
	//网络
	Net *net;

	//公共模块指针
	DevCache* _cache;
	RealDataFilter* _filter;
	ParamSet* _setting;	
	Log* _log;
	DBOperate* _database;
};

#endif // PROTOBUFSERVER_H
