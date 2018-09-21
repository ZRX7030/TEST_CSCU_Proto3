#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#include "Infotag/CSCUBus.h"
#include "ProtobufServer/net/net.h"
#include "ProtobufServer/cache/datacache.h"
#include <google/protobuf/message.h>
#include <string>
#include <QByteArray>

using namespace std;

typedef google::protobuf::Message PBMessage;

typedef struct _NetFrame
{
	char	symbol;			//帧标识
	char	header_len;		//报文头部长度
	char	proto_ver;		//协议版本号
	char	encoding_type;	//消息体编码类型
	int		payload_len;	//帧载荷长度
	char	msgType;		//帧类型
	char	msgTypeEx;		//扩展帧类型
	char	reserved1[2];	//保留
	char	ctrlAddr[16];	//站地址
	char	reserved2[4];	//保留
	char	payload[0];		//帧载荷
}__attribute__((packed)) NetFrame;

#if(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
	#define htonll(value) ((((uint64_t)htonl(value & 0xFFFFFFFFLL)) << 32) | ((uint64_t)htonl(value >> 32)))
#else
	#define htonll(value) value		
#endif

#define MAX_SYNC_ACCOUNT			100		//同步账户信息最大条数
#define RECV_BUFF_LEN       		256		//数据接收缓存长度
#define FRAME_FLAG  				0x63	//充电协议网络帧标识

//消息类型定义
//充电部分
#define CMD_ACK(x)					x + 0x80//应答指令
#define CMD_HEART					0x00	//心跳
#define CMD_LOGIN					0x02	//上线
#define CMD_LOGOUT					0x04	//离线
#define CMD_APPLY_START_CHARGE		0x10	//申请开始充电
#define CMD_START_CHARGE			0x11	//开始充电指令
#define CMD_APPLY_STOP_CHARGE		0x12	//申请结束充电
#define CMD_STOP_CHARGE				0x13	//结束充电指令
#define CMD_PAUSE_CHARGE			0x15	//暂停充电指令
#define CMD_UPLOAD_BILL				0x16	//订单上传
#define CMD_RESUME_CHARGE			0x17	//恢复充电指令
#define CMD_CHARGER_NOTIFY			0x20	//充电机状态变化通知
#define CMD_CHARGER_CALL			0x21	//充电机状态召唤
#define CMD_APPLY_ACCOUNT			0x22	//账户详情获取
#define CMD_POWER_CURVE				0x23	//充电功率曲线
#define CMD_SYNC_ACCOUNT			0x24	//同步账户信息
#define CMD_BMS_PARAM_NOTIFY		0x26	//BMS参数通知
#define CMD_MEASURE_CALL			0x31	//遥测数据召唤
#define CMD_MEASURE_NOTIFY			0x32	//遥测数据变化通知
#define CMD_SIGNAL_CALL				0x33	//遥信数据召唤
#define CMD_SIGNAL_NOTIFY			0x34	//遥信数据变化通知
#define CMD_BMS_CHARGE_CALL			0x35	//BMS充电数据召唤
#define CMD_BMS_CHARGE_NOTIFY		0x36	//BMS充电数据通知
#define CMD_UNCONFIRMED_BILL_CALL	0x41	//未确认订单召唤
#define CMD_GUN_GROUP_NOTIFY		0x42	//上报多枪分组
#define CMD_CODE_BILL_CALL			0x43	//指定编号订单召唤
#define CMD_GUN_GROUP_SET			0x45	//下发多枪分组
#define CMD_EMERGENCY				0x47	//下发应急充电指令
#define CMD_RM_WHITE				0x49	//下发清除账户白名单

//运行监控部分
#define CMD_BILL_ALARM				0x48	//订单上报异常告警
#define CMD_MONITOR_MEASURE_CALL	0x51	//运行遥测数据召唤
#define CMD_MONITOR_MEASURE_NOTIFY	0x52	//运行遥测数据通知
#define CMD_MONITOR_SIGNAL_CALL		0x53	//运行遥信数据召唤
#define CMD_MONITOR_SIGNAL_NOTIFY	0x54	//运行遥信数据通知
#define CMD_MONITOR_STATE_CALL		0x55	//运行状态数据召唤
#define CMD_MONITOR_STATE_NOTIFY	0x56	//运行状态数据通知
#define CMD_MONITOR_CONTROL			0x57	//运行遥控指令
#define CMD_MONITOR_ALARM			0x58	//运行告警数据

//错误
#define CMD_ERROR					0xFF	//错误

class Protocol : public QObject
{
	Q_OBJECT
public:
	virtual ~Protocol();

	//内部模块指令
	virtual bool command(InfoMap &map, InfoAddrType &type);
	//数据突发
	virtual bool burst(QDataPointList burst);
	//数据帧解析
	virtual bool parse(short type, char *buff, int len);
	//上线
	virtual bool login();
	//离线
	virtual void logout(int type = 0);
	//心跳
	virtual void heart();
	//数据帧发送
	virtual bool sendFrame(char frameType, PBMessage *message);
	virtual void destroy();

signals:
	void active(int logged);
	void live();
	void parsed(InfoMap map, InfoAddrType type);
    void sendMessage(QString qstrMessage);

private slots:
	//网络状态
	void onNetConnected();
	void onNetDisconnected();
	void onNetError(int err);
	void onNetReceive();

protected:
	Protocol(Net *n);

	//调试信息
	QMap<int, QString> m_mapProtoName;
	QMap<int, int> m_mapDebug;

	//日志
	void writeLog(QString strLog, int level = 2);

	//当前总线消息类型
	InfoAddrType infoType;
	//当前总线消息内容
	InfoMap info;

	//单相、三相、直流终端数量
	QByteArray _canRange;
	int acNum;
	int tacNum;
	int dcNum;

	//接收缓存
	QByteArray arBuff;

	//集控器站地址
	string ctrlAddr; 
	//集控器密钥
	string cscuKey;
	//服务器类型
	int serverType;
    string ctrlSwVersion;//集控器版本号

	string serverAddr;
	ushort serverPort;
	int encryptNet;
	bool connected;
	bool logged;
	bool terminated;

	//网络层指针
	Net *net;
};

#endif
