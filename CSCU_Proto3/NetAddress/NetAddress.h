#ifndef NETADDRESS_H
#define NETADDRESS_H

#include <QTcpSocket>
#include <QStringList>
#include "DevCache.h"
#include "RealDataFilter.h"
#include "ParamSet.h"
#include "Log.h"
#include "DBOperate.h"
#include "ModuleIO.h"

//服务器状态机
typedef enum {
	DISCONNECT,
	CONNECTING,
	CONNECTED,
	REQUESTING,
	WAITING
}NET_STATE;

//数据帧帧头
typedef struct _FrameHdr{
	char header;		//帧头
	short len;			//帧长度
	char type;			//帧类型
	char value;			//功能码
	int time;			//时间
}__attribute__((packed)) FrameHdr;

//应答帧
typedef struct _Reply{
	FrameHdr hdr;
	char data[0];
}__attribute__((packed)) Reply;

class CNetAddress: public CModuleIO
{
    Q_OBJECT
public:
   	CNetAddress();
	~CNetAddress();

signals:
	//总线信号
    void sigToBus(InfoMap mapInfo, InfoAddrType type);
public slots:
	//总线信号槽
    void slotFromBus(InfoMap mapInfo, InfoAddrType type);

public:
	//动态库接口
    int InitModule(QThread* pThread);
    int RegistModule();
    int StartModule();
    int StopModule();
    int ModuleStatus();

private slots:
	//内部信号槽
	void slot_onThreadRun();
	void slot_onConnected();
	void slot_onDisconnected();
	void slot_onError(QAbstractSocket::SocketError err);
	void slot_onReceive();

private:
	void timerEvent(QTimerEvent *event);
	bool connectToServer();
	bool GetHostList(stServerListConfig &confServerList);
	void sendLoaclAddress();

	void RequestAddress();
	void ParseFrame(char *data, uint len);
	void ParseAddress(char *data, uint len);
	void ParseError(char *data, uint len);
    void WriteLog(QString strLog, int iLevel = 2); 

	QString m_strStation;	//集控器地址
	QString m_strKey;		//集控器密钥
	QString m_strVersion;	//集控器版本
	NET_STATE m_State;		//状态机
	QStringList m_listHost;	//服务器列表
	QString m_strHost;		//服务器地址
	int m_iPort;			//服务器端口
	int m_iServerTimer;		//定时器
	int m_iServerTimeout;	//网络状态超时时间

	QByteArray m_arData;	//数据接收缓存
	QTcpSocket* m_pSocket;	//socket
	int m_iHostIndex;		//服务器序号
	int m_iTryConnect;		//服务器尝试连接次数

	int m_iLocalAddress;	//兼容非标使用本地配置地址

	DevCache* m_pDevCache;	//数据缓存
	RealDataFilter* m_pFilter;//数据过滤
	ParamSet* m_pSetting;	//配置
	Log* m_pLog;			//日志
	DBOperate* m_pDatabase;	//数据库
};

#endif
