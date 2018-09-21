#ifndef IEC104LOCALSERVER_H
#define IEC104LOCALSERVER_H

#include <QTcpSocket>
#include "IEC104Server/IEC104Define.h"
#include "DevCache.h"
#include "RealDataFilter.h"
#include "ParamSet.h"
#include "Log.h"
#include "DBOperate.h"
#include "ModuleIO.h"

class IEC104LocalServer : public CModuleIO
{
    Q_OBJECT
public:
    IEC104LocalServer();
	~IEC104LocalServer();

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
    void SendKeyAgreement();
    bool ParseKeyAgreement(uchar* pszData, int iLen);

    int ParseFrame(uchar *pszData, int iLen, InfoMap& mapInfo, InfoAddrType& type);
	bool Parse104Frame(uchar* pszData, int iLen, InfoMap& mapInfo, InfoAddrType& type);
	bool ParseInfoTag(uchar* pszData, uint iLen, uchar cLimit, InfoMap& mapInfo);
	bool ParseFrameU(uchar* pszData, int iLen);
	bool ParseFrameI(uchar* pszData, int iLen, InfoMap& mapInfo, InfoAddrType& type);
	void SendFrameS();
	void SendFrameU(uchar cCmd);
	void SendFrameI(QIteratorList& list, uchar cFrameType, ushort sReason, int iSerial = 0);
	void SendEncryptData(uchar *pszData,int iLen);
	void timerEvent(QTimerEvent *event);

	void SendQueryAll(QueryType type);
	void SendSignal(uchar cCanAddr, bool bBurst = false);
	void SendMeasure(uchar cCanAddr);
    void SendMeasure2(uchar cCanAddr, bool bBurst = true);
	void SendBMS(uchar cCanAddr);
	//两函数几乎无用处，删除 ++++++++++++++++++++DHT 2018-03-09
    //bool QueryBatteryStatus(uchar cCanAddr, int& iInfoAddr, QByteArray& arData);//能效计划临时用
    //bool QueryBatteryEleInfo(uchar cCanAddr, int& iInfoAddr, QByteArray& arData);//能效计划临时用
	//-----------------------DHT
    bool QueryPowerOptimizerStatus(uchar cCanAddr, int& iInfoAddr, QByteArray& arData);//能效计划临时用
	bool QuerySignal(uchar cCanAddr, int& iInfoAddr, QByteArray& arData);
	bool QueryMeasure(uchar cCanAddr, int& iInfoAddr, QByteArray& arData);
    bool QueryMeasure2(uchar cCanAddr, int& iInfoAddr, QByteArray& arData, bool bCharge = true);
	bool IsBurstMeasure2(InfoMap& mapInfo);
	void SendHighVolate();
	void SendVersion();
	void SendModuleVersion(uchar cCanAddr);
	void SendStationEnv();
	void SendFrozenEnergy(InfoMap& map);
    void SendAmmeterDataType(InfoMap mapInfo,QIteratorList list);   //发送电表数据   add by zrx 2017-03-24
    void SendAmmeterDataType1(InfoMap mapInfo,QIteratorList list);   //发送电表数据-当前电能及最大需量数据   add by zrx 2017-03-24
    void SendAmmeterDataType2(InfoMap mapInfo,QIteratorList list);   //发送电表数据-整点冻结电能数据   add by zrx 2017-03-24
    void SendAmmeterDataType3(InfoMap mapInfo,QIteratorList list);   //发送电表数据-日冻结电能及最大需量数据   add by zrx 2017-03-24
    void SendAmmeterDataType4(InfoMap mapInfo,QIteratorList list);   //发送电表数据-结算日电能及最大需量数据   add by zrx 2017-03-24

	void SyncTime(InfoMap& map);
	bool SetAmmeterParam(uchar* pszData, int iLen);
    bool SetAmmeterParamSet(InfoMap& mapInfo);     //平台设置电表参数
	void SetNetState(bool bConnected, QString strError = "");

	void SendNetState(bool bEmergency = false);
	void WriteEmergencyEnable(InfoMap &mapInfo);
	void WriteEmergencySetting(InfoMap &mapInfo);
	void WriteQueueGroupInfo(InfoMap &mapInfo);

	bool connectToServer();
	bool GetHostList(stServerListConfig &confServerList);
	bool WriteServerList(InfoMap &mapInfo);

    //多枪hd
    void WriteChargeGunGroupInfo(InfoMap &mapInfo);
    int  CheckChargeGunGroupInfo(InfoMap &mapInfo);

	void SendActiveDefendAlarm(InfoMap &mapInfo);

	void WriteLog(uchar* pszData, int iLen, uchar cType = FRAME_TYPE_I, bool bSend = true);
	void WriteLog(QString strLog, int iLevel = 2);

    void SendFrozenEnergyPeakShaving(InfoMap& map);   //hd

    void parseBatteryStatus(InfoMap &mapInfo);
    void parsePowerOptimizerStatus(InfoMap &mapInfo);

	QDateTime m_dtOnline;	//网络连接成功时间
    uchar m_cServerType;	//服务器类型0:云平台 1:本地服务器
	QString m_strAesKey;	//AES密钥
	uchar m_sz3DesKey[17]; 	//数据加解密密钥
	uchar m_cCanRange[3][2];//单相、三相、直流can地址范围
	QString m_strStationNo;	//转换之后的站地址
	ushort m_sRecvNo;		//发送接收序号
	ushort m_sSendNo;		//接收序号

	SERVER_STATE m_ServerState;//网络状态
	int m_iServerTimer;		//定时器
	int m_iServerTimeout;	//网络状态超时时间
	int m_iHeartCount;		//心跳计数
	int m_iRebootNo;		//重启时I帧发送序号
	int m_iOfflineTime;		//离线状态计数
	int m_iEmergencyTime;	//触发应急充电时间
	bool m_bEmergency;		//离线状态
	bool m_bConnected;		//网络连接状态
	bool m_bActived;		//集控器激活标志
	bool m_bExit;			//模块退出标志
	InfoMap m_mapCanIndex;	//can地址对应关系缓存
	QString m_strHost;		//云平台地址
	ushort m_sPort;			//云平台端口
	QByteArray m_arData;	//数据接收缓存
	QTcpSocket* m_pSocket;	//socket
	QHostList m_listHost;	//服务器列表
	int m_iHostIndex;		//服务器序号
	int m_iTryConnect;		//服务器尝试连接次数
    stServer1Config m_confServer;//服务器配置

	DevCache* m_pDevCache;	//数据缓存
	RealDataFilter* m_pFilter;//数据过滤
	ParamSet* m_pSetting;	//配置
	Log* m_pLog;			//日志
	DBOperate* m_pDatabase;	//数据库

    InfoMap batteryInfoMap;//能效计划临时用
    InfoMap PowerOptimizerInfoMap;

};

#endif // IEC104LOCALSERVER_H
