#ifndef CHARGESERVER_H
#define CHARGESERVER_H

#include "ProtobufServer/server/protobufserver.h"

class ChargeServer : public ProtobufServer
{
    Q_OBJECT
public:
    ChargeServer();
	~ChargeServer();

    //服务器
//    static ChargeServer *chargeserver();
//    //服务器实例
//    static ChargeServer *_chargeserver;

	//网络配置
	void getNetSetting(string &host, ushort &port, int &encrypt);
	//日志记录
	void writeLog(QString strLog, int iLevel = 2);
	//数据检查完毕
	void dataCheckFinished();

    //平台应急指令-处理应急状态
    bool ProtoEmergency(int flag);
    //应急充电本地配置
    EmergencyConfig configEmergency;
    int bBillResendEnd; //订单重传结束标志位 true-重传结束

public slots:
	void slot_onThreadRun();
    void slotFromBus(InfoMap mapInfo, InfoAddrType type);
	//协议链路激活
	void onProtoActive(int logged);
	//平台应急指令
	void onProtoEmergency(int flag);
	//平台同步账户指令
	void onProtoSyncAccount();

    //恢复中信号
    void onProtoReverting(int reverting); //add by zrx
    void onProtosendMessage(QString qstrMessage);  //消息内容

    //协议解析完成
    void onProtoParsed(InfoMap map, InfoAddrType type);
public:
	//模块导出接口
    int RegistModule();

protected:
	//定时器
	void timerEvent(QTimerEvent *event);
	//设置网络状态图标
	void setNetState(bool state);
	//周期计数
	int cycleTime;

private:
	//同步白名单账户信息
	void syncWhite();
	//本地鉴权入口
	bool localAuth(InfoMap &map, InfoAddrType &type);
	//解析申请信息
	bool parseApplyInfo(InfoMap map, InfoAddrType type, QString &id, int &idType, int &cmd);
	//鉴权账户信息
	int authId(QString id, int idType, EmergencyConfig config);
	//时间限制
	int timeCheck(EmergencyConfig config);
	//订单限制
	int billCheck(EmergencyConfig config);
	//本地应急诊断
	int emergencyCheck();
	//设置本地应急充电状态
	void setEmergencyState();
	//保存应急状态
    bool saveEmergencyState();

	//本地应急充电
	bool localEmergency;
	//平台应急充电
    bool remoteEmergency;    //优先级高于localEmergency

    bool selectHistoryBill(); //查询有无历史订单,有-true;无-false
};

#endif // CHARGESERVER_H
