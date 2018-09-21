#ifndef SERVERDEF_H
#define SERVERDEF_H

#include "Infotag/CSCUBus.h"

#if(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
	#define ___constant_swab16(x) ((__u16)( \
			(((__u16)(x) & (__u16)0x00ffU) << 8 ) | \
			(((__u16)(x) & (__u16)0xff00U) >> 8 ) ))

	#define ___constant_swab32(x) ((__u32)( \
			(((__u32)(x) & (__u32)0x000000ffUL) << 24) | \
			(((__u32)(x) & (__u32)0x0000ff00UL) <<  8) | \
			(((__u32)(x) & (__u32)0x00ff0000UL) >>  8) | \
			(((__u32)(x) & (__u32)0xff000000UL) >> 24)))  
#else
	#define ___constant_swab16(x) x
	#define ___constant_swab32(x) x
#endif

#define ___measure16(value, offset, ratio) \
		(short)(((float)value - (int)offset) / (float)ratio)

#define ___measure32(value, offset, ratio) \
		(int)(((float)value - (int)offset) / (float)ratio)

//网络相关
#define NET_FLAG	0x79
#define IEC_FLAG	0x68
#define MAX_NO   	0x7FFF

#define MAX_IEC_DATA_LEN	237		//IEC信息体数据最大长度
#define MAX_IEC_PACKAGE_LEN	255		//IEC报文最大长度
#define MIN_NET_PACKAGE_LEN	5		//网络报文最小长度
#define MAX_NET_PACKAGE_LEN	MIN_NET_PACKAGE_LEN + MAX_IEC_PACKAGE_LEN //网络报文最大长度

#define TIME_READ_TIMEOUT	20 * 1000
#define TIME_RECONNECT		10 * 1000
#define TIME_OFFLINE		40 * 1000
#define WAIT_ADDR_TIME	60
#define	DATA_CHECK_TIME 60
#define RECONNECT_TIME  10
#define CONNECTING_TIME 60
#define KEYAGREE_TIME  	45
#define HEART_TIME  	20

//帧类型
#define FRAME_TYPE_I	0x00
#define FRAME_TYPE_S	0x01
#define FRAME_TYPE_U	0x03

//U帧指令
#define CMD_STARTDT		0x07
#define CMD_STARTDTACK	0x0B
#define CMD_STOPDT		0x13
#define CMD_STOPDTACK	0x23
#define CMD_TEST		0x43
#define CMD_TESTACK		0x83

//I帧指令
#define CMD_SINGLE_POINT			0x01//不带时标的单点信息
#define CMD_CARD_CHARGE_STOP		0x02//刷卡停止充电
#define CMD_SCANCODE_CUSTOMER_ID_CHARGE_APPLY		0x03//扫码申请开始充电
#define CMD_SCANCODE_CUSTOMER_ID_CHARGE_STOP		0x04//扫码申请结束充电
#define CMD_CARD_CHARGE_APPLY		0x07//刷卡申请充电
#define CMD_ZIGBEE_CHARGE			0x08//ZIGBEE充电命令
#define CMD_RESPONSE_RET_PEAK		0x0A//削峰填谷返回数据项响应的结果hd

#define CMD_SCANCODE_CUSTOMER_ID			0x09//扫码客户ID上传
#define CMD_ECARD_CHARGE_APPLY		0x14//e通卡上传信息并申请充电
#define CMD_MEASURE      			0x0B//测量值，标度化值
#define CMD_EXEC_RET				0x0C//返回数据项执行的结果
#define CMD_RESPONSE_RET			0x0D//返回数据项响应的结果
#define CMD_CHARACTER				0x0E//返回数据项响应的结果
#define CMD_MEASURE2				0x0F//累积量（不带时标）
#define CMD_BMS						0x10//上传模块BMS数据
#define CMD_BURST_PREDICT_TIME		0x11//突发充电预估完成时间
#define CMD_MODULE_FREE				0x12//突发充电模块是否空闲
#define CMD_QUEUE_INFO				0x13//突发充电排队信息
#define CMD_CARD_ACCOUNT			0x16//刷卡业务帐户信息
//#define CMD_CAR_LOCK				0x18//车位锁控制
#define CMD_CHANGE_PRIORITY   		0x19//设置充电优先等级
#define CMD_CHARGE_FINISH_TIME		0x1A//设置充电完成时间
#define CMD_CHARGE_MODE				0x1B//设置充电模式
#define CMD_MAX_LOAD		    	0x1C//设置充电电网最大允许负荷
#define CMD_GROUP_POLICY  			0x1D//设置群充策略
#define CMD_AUXPOWER	    		0x1E//设置辅助电源类型
#define CMD_QUEUE_PUBLISH  			0x1F//轮充模式下，服务器下发的排队信息
#define CMD_CHARGE_PREDICT_TIME		0x21//申请充电预估完成时间
#define CMD_CHAGE_DETAIL			0x22//下发充电总费用和充电明细
#define CMD_TERMINAL_INDEX			0x23//下发CAN地址和终端编号对应关系
#define CMD_SET_PARAM				0x24//下发参数设置
#define CMD_FROZEN_ENERGY			0x25//上送冻结电量
#define CMD_CHARGE					0x34//遥控命令
#define CMD_NET_STATE				0x35//集控和云平台的联网状态
#define CMD_COUPLE_GUN    0x40//集控上传单双枪充电方式及分组信息
#define CMD_REMOTE_EMERGENCY		0x41//平台下发紧急充电状态
#define CMD_LOCAL_EMERGENCY_RET		0x42//紧急充电回应
#define CMD_LOCAL_EMERGENCY			0x43//本地上报紧急充电状态
#define CMD_REMOTE_EMERGENCY_RET 	0x44//平台返回在线状态
#define CMD_EMERGENCY_ENABLE		0x45//平台设置应急充电开关
#define CMD_EMERGENCY_ENABLE_RET 	0x46//应急充电开关设置结果
#define CMD_QUEUE_GROUP_INFO		0x47//轮充组信息
#define CMD_QUEUE_GROUP_RESULT		0x48//轮充组信息
#define CMD_QUEUE_GROUP_STATE		0x49//排队信息带流水号
#define CMD_EMERGENCY_SETTING		0x4A//平台配置应急充电设置项
#define CMD_EMERGENCY_SETTING_RET 	0x4B//应急充电详细配置结果
#define CMD_CHARGEGUN_GROUP_INFO		0x4C//下发多枪分组信息
#define CMD_CHARGEGUN_GROUP_RESULT		0x4D//多枪分组信息返回结果
#define CMD_CHARGEGUN_GUN    0x4E//集控上传多枪充电方式及分组信息   跟0X40在不同工程中，作用部分重合hd
#define CMD_CHARGEGUN_RESULT    0x4F//集控上传多枪充电方式及分组信息返回结果
#define CMD_ACTIVEDEFEND_ALARM		0x52//主动防护告警
#define CMD_QUERY_ALL				0x64//总召唤命令
#define CMD_QUERY_MEASURE2			0x65//计数量总召命令
#define CMD_SYNC_TIME				0x67//时钟同步命令
#define CMD_FROZEN_ENERGY_PEAK   0x6A   //削峰填谷上传结束充电冻结电量和结果 hd
#define CMD_RESPONSE_RET_FAILE_PEAK		0x6B//削峰填谷返回数据项启动失败响应的结果hd
#define CMD_QUERY_SIGNAL			0x86//召唤模块遥信数据
#define CMD_QUERY_MEASURE			0x87//召唤模块遥测数据
#define CMD_QUERY_BMS				0x88//召唤模块BMS数据
#define CMD_VERSION					0x89//召唤子站软件版本号
#define CMD_MODULE_VERSION			0x8A//召唤模块软件版本号
#define CMD_QUERY_HIGHVOL			0x8B//召唤子站高压侧数据
#define CMD_ENV_DATA       			0x8C//召唤子站环境监测数据
#define CMD_UPLOAD_LOG				0x8D//上传日志文件
#define CMD_UPGRADE_CONFIG 			0x8E//升级配置文件
#define CMD_UPGRADE_MAIN			0x8F//升级子站程序
#define CMD_UPGRADE_MODULE  		0x90//升级模块程序
#define CMD_REBOOT					0x91//重启子站
#define CMD_UPLOAD_CONFIG			0x92//上传子站配置文件
#define CMD_STATION_PARAM			0x99//下发子站参数
#define CMD_300KW_TERM_PARAM		0x9C//发送直流充电终端双系统300kw参数设置
#define CMD_RELAY_CONTROL			0xA2//输出继电器控制指令
#define CMD_AMMETER_DATA_SELECT  0x68 //电表数据查询  add by zrx 2017-03-24
#define CMD_AMMETER_DATA_SELECT_SEND  0x69 //电表数据发送  add by zrx 2017-03-24
#define CMD_CAR_LOCK   0x26 //车位锁控制  add by weiwb 2017-08-15
#define CMD_CARLOCK_STATUS   0x50 //车位状态上传  add by weiwb 2017-08-16
#define CMD_CARLOCK_PARAMSET   0x29 //车位参数设置  add by weiwb 2017-10-23

//传送原因                  	
#define REASON_NONE				0x0000//保留
#define REASON_PERIOD			0x0100//周期、循环
#define REASON_SCAN				0x0200//背景扫描
#define REASON_BURST			0x0300//突发（自发）
#define REASON_INIT				0x0400//初始化
#define REASON_REQUEST			0x0500//请求或被请求
#define REASON_ACTIVE			0x0600//激活
#define REASON_ACTIVE_ACK		0x0700//激活确认
#define REASON_ACTIVE_END		0x0A00//激活终止
#define REASON_RETURN			0x0B00//返回
#define REASON_QUERY			0x1400//响应站总召
#define REASON_REQ_ENCRYPT		0x1500//请求或被请求（数据加密）
#define REASON_MEASURE2			0x2500//响应计数量召唤
#define REASON_UNKNOWN_TYPE		0x2C00//未知的类型标识
#define REASON_UNKNOWN			0x2D00//未知的传送原因
#define REASON_UNKNOWN_UNIT		0x2E00//未知的应用服务数据单元公共地址
#define REASON_UNKNOWN_ADDR		0x2F00//未知的信息对象地址
#define ___reason_8bit(reason)	(uchar)((reason & 0xFF00) >> 8)

//信息体相关
//104帧最大长度253字节，去掉固定长度部分，可容纳的数据长度为230字节
//每个遥信长度为1，共10个遥信数据，每帧最大包含的遥信23个 (230 / (1 * 10))
//每个两字节遥测长度为2品质长度1，共20个遥测数据，每帧最大包含遥测3个(230 / (2 + 1)*20)
//每个四字节遥测长度为4品质长度1，共2个遥测数据，每帧最大包含遥测23个(230 / (4 + 1) * 2)
//每条冻结电量数据（信息体+电能+时标）((230 - 10) / (3 + 4 + 7))

#define SIGNAL_PACKAGE_NUM			23		//每帧最多包含遥信数据条数
#define MEASURE_PACKAGE_NUM			3		//每帧最多包含遥测数据条数
#define MEASURE2_PACKAGE_NUM		23		//每帧最多包含遥测数据条数
#define FROZEN_ENERGY_PACKAGE_NUM	15		//每帧最多包含冻结电量数据条数
//能效计划临时用
#define BatteryStatus_PACKAGE_NUM			1		//每帧最多包含电池遥信数据条数
#define BatteryStatus_INFO_NUM		 		44   	//每条遥信数据包含的数据个数
#define PowerOptimizerInfo_PACKAGE_NUM			1		//每帧最多包含电池遥信数据条数
#define PowerOptimizerInfo_INFO_NUM		 		75   	//每条遥信数据包含的数据个数
#define BatteryEleInfo_PACKAGE_NUM			1		//每帧最多包含电池遥信数据条数
#define BatteryEleInfo_INFO_NUM		 		15   	//每条遥信数据包含的数据个数
#define FROZEN_ENERGY_PACKAGE_NUM	15		//每帧最多包含冻结电量数据条数  hd 响应结果和冻结电量一起上传造成冻结电量条数少三条
#define StopResult_PACKAGE_NUM	13		//每帧最多包含冻结电量数据条数  hd 响应结果和冻结电量一起上传造成冻结电量条数少三条


#define SIGNAL_INFO_NUM		 		10   	//每条遥信数据包含的数据个数
#define MEASURE_INFO_NUM   	 		20      //每条遥测数据包含的数据个数
#define MEASURE2_INFO_NUM       	2       //每条遥测数据包含的数据个数
#define BMS_INFO_NUM       			11      //每条BMS信息包含的数据个数

#define SIGNAL_BEING_ADDR			0x200000//遥信信息开始地址（大端，传输用小端格式） 
#define MEASURE_BEING_ADDR	 		0x201001//两字节遥测信息开始地址
#define MEASURE2_BEING_ADDR  		0x203001//四字节遥测开始地址
#define MEASURE2_BEING_ADDR1		0x203201//四字节遥测开始地址
#define BMS_BEING_ADDR 				0x204001//BMS信息开始地址
#define BEGIN_FROZEN_ENERGY_ADDR	0x206001//冻结电量开始充电电能起始信息体	
#define END_FROZEN_ENERGY_ADDR		0x230000//冻结电量结束充电电能起始信息体	

#define ___signal_infoaddr(canaddr)		(int)(SIGNAL_BEING_ADDR + (canaddr - 1) * SIGNAL_INFO_NUM)
#define ___measure16_infoaddr(canaddr)	(int)(MEASURE_BEING_ADDR + (canaddr - 1) * MEASURE_INFO_NUM)
#define ___measure32_infoaddr(canaddr)	(int)(MEASURE2_BEING_ADDR + (canaddr - 1) * MEASURE2_INFO_NUM)
#define ___measure32_infoaddr1(canaddr)	(int)(MEASURE2_BEING_ADDR1 + (canaddr - 1) * MEASURE2_INFO_NUM)
#define ___bms_infoaddr(canaddr)		(int)(BMS_BEING_ADDR + (canaddr - 1) * BMS_INFO_NUM)

typedef QList<InfoMap::iterator> QIteratorList;

//服务器列表节点
typedef struct _HostNode{
	QString strHost;	
	short sPort;
}HostNode;
typedef QList<HostNode> QHostList;

//服务器状态
typedef enum {
	DATA_CHECK,
	GET_ADDRESS,
	NET_DISCONNECT,
	NET_CONNECTING,
	NET_CONNETED,
	NET_ACTIVED,
	NET_TRANSFER,
	NET_HEART
}SERVER_STATE;

//服务器类型
typedef enum{
	SERVER_REMOTE = 0,	//云平台
	SERVER_LOCAL		//本地服务器	
}ServerType;

//召唤类查询类型
typedef enum{
	QUERY_MEASURE2 = 0,
	QUERY_MEASURE,
    QUERY_SIGNAL,
    QUERY_MEASURE3,
    QUERY_BATTERYStatus,
    QUERY_BATTERYEleInfo,
    QUERY_PowerOptimizerInfo
}QueryType;

//网络帧头部结构
typedef struct _Net_Hdr
{
	uchar  cHead;		//帧标识 0x79
	uint   iLen;		//帧长度
	ushort sSequence;	//帧序号
	uchar  cType;		//帧类型
}__attribute__((packed)) Net_Hdr;

//网络帧结构
typedef struct _Frame_Net
{
	Net_Hdr hdr;
	uchar szData[MAX_NET_PACKAGE_LEN];
}__attribute__((packed)) Frame_Net;

//APCI结构
typedef struct _APCI
{
    uchar cHead;		//帧标识 0x68
    uchar cLen;			//帧长度
    uchar cCtrl1;		//控制1
    uchar cCtrl2;		//控制2
    uchar cCtrl3;		//控制3
    uchar cCtrl4;		//控制4
}__attribute__((packed)) APCI;

//ASDU头部结构
typedef struct _ASDU_Hdr
{
    uchar cType;		//类型
    uchar cLimit;		//限定词
    ushort sReason;		//传送原因
    uchar szStation[8]; //公共地址
}__attribute__((packed)) ASDU_Hdr;

//ASDU结构
typedef struct _ASDU
{
	ASDU_Hdr hdr;
	uchar szData[MAX_IEC_DATA_LEN];
}__attribute__((packed)) ASDU;

//I帧结构
typedef struct _Frame_I
{
	APCI apci;
	ASDU asdu;
}__attribute__((packed)) Frame_I;

//U帧结构
typedef struct _Frame_U
{
	APCI apci;
	uchar szStation[8];
}__attribute__((packed)) Frame_U;

//S帧结构
typedef struct _Frame_S
{
	APCI apci;
}__attribute__((packed)) Frame_S;

#endif // SERVERDEF_H
