#ifndef COMMON_H
#define COMMON_H

#include <QMap>
#include <QVariant>
/*历史数据每页最大条数定义*/
#define			HISTNUM_PER_PAGE		8
#define			REALNUM_PER_PAGE		8

/*页面切换定义*/
#define         PAGE_MAINTAIN                   1
#define         PAGE_UPDATEEXPORT               2

#define         PAGE_UPDATEEXPORT_MAIN          0
#define         PAGE_UPDATEEXPORT_FLASH         1

#define         PAGE_CHARGERMANAGE              3

#define         PAGE_CHARGEMANAGE_STANDBY		0
#define         PAGE_CHARGEMANAGE_VINSTART      1    //2017-3-26 by songqb
#define         PAGE_CHARGEMANAGE_CHARGING		2
#define         PAGE_CHARGEMANAGE_FINISH		3
#define         PAGE_CHARGEMANAGE_MAIN			4
#define         PAGE_CHARGEMANAGE_FAULT         5
#define			PAGE_CHARGEMANAGE_FLASHWAIT		6
#define			PAGE_CHARGEMANAGE_SELECTCHARGE  7
#define			PAGE_CHARGEMANAGE_BASE			8
#define         PAGE_CHARGEMANAGE_PRINTPAPER    9


/*语音文件定义*/
#define			START_CHARGE_APPLAY			"/mnt/nandflash/wav/st-1.wav"		    //请刷卡启动充电
#define			START_CHARGE_CARDINFO		"/mnt/nandflash/wav/st-2.wav"			//正在获取卡片信息
#define			START_CHARGE_TYPE			"/mnt/nandflash/wav/st-3.wav"			//请选择充电方式
#define			START_CHARGE_STARTING		"/mnt/nandflash/wav/st-4.wav"			//启动充电中,请等待
#define			START_CHARGE_SUCCESS		"/mnt/nandflash/wav/st-5.wav"			//开始充电申请成功
#define         START_CHARGE_GUN            "/mnt/nandflash/wav/st-6.wav"           //请插枪

#define			STOP_CHARGE_APPLAY			"/mnt/nandflash/wav/sp-1.wav"			//请刷卡结束充电
#define			STOP_CHARGE_STOPING			"/mnt/nandflash/wav/sp-2.wav"			//正在结束充电,请等待
#define			STOP_CHARGE_SUCCESS			"/mnt/nandflash/wav/sp-3.wav"			//停止充电申请成功


/*主题类型定义*/
typedef enum
{
    InfoAddrNone=0,
    InfoAddrMaintain=0x01,               //维护类数据 1
    InfoAddrConfig=0x02,                 //配置类数据 2
    InfoAddrStatus=0x03,                 //状态类数据 3
    InfoAddrReal=0x04,                   //实时数据类 4
    InfoAddrHistory=0x05,                //历史数据类 5
    InfoAddrExchange=0x06               //界面交互类 6
}InfoAddrType;


/*信息体定义*/
typedef enum __InfoBodyType
{
	InfoBodyNone = 0,
    InfoMaintainVersion = 0x0102,      //版本信息

	//配置数据
    InfoConfigCSCU = 0x0201,            //集控设置
    InfoConfigSys = 0x0202,             //系统设置
    InfoConfigQRCode = 0x0203,          //二维码设置
    InfoConfigChargeModule = 0x0204,    //充电模式设置
    InfoConfigTime = 0x0205,            //时间设置
    //InfoConfigPassword = 0x0206,        //密码设置
    InfoConfigVINNumber = 0x0206,       //VIN码
    InfoConfigPhaseType = 0x0207,       //相别设置
    InfoConfigSpecialFunc = 0x0208,     //特殊功能
    InfoConfigDcSpecialFunc = 0x0209,   //直流特殊功能
    InfoConfigLoad = 0x020a,            //负荷约束设置
    InfoConfigTPFV = 0x020b,             //尖峰平谷设置
	InfoConfigAmmeterAddr = 0x020c,		//电表地址参数
	InfoConfigPassword = 0x020d,		//密码设置
	InfoConfigIO = 0x020e,				//IO配置参数
    InfoConfigTerminalQRCode = 0x020f,  //二维码生成终端编号
    InfoConfigDCChargerTypeNum = 0x0210,  //直流机终端参数、监控设备参数数量
    InfoConfigDCChargerTerm = 0x0211,  //直流机终端参数
    InfoConfigDCChargerMonitor = 0x0212,  //直流机监控设备参数
    //InfoConfigDCChargerSys = 0x0213,  //直流机系统参数类型
	InfoConfigDCMPCNum = 0x0214,    //直流机下pdu、ccu、模块数量

    InfoConfigChargeSelectPassword = 0x021a, //改变充电模式密码查询下发
    InfoConfigChangeChargePassword = 0x021b, //修改本地充电密码

	//状态数据
    InfoStatusBase = 0x0301,            //终端基础数据 连接状态 充电状态
    InfoStatusCSCU = 0x0302,			//cscu状态数据

	//实时数据
    InfoRealModule = 0x0401,            //实时模块信息
    InfoRealStatus = 0x0402,            //实时状态信息
    InfoRealBMS = 0x0403,               //实时bms信息
    InfoRealFault = 0x0404,				//故障数据
    InfoRealCharge = 0x0405,			//充电中数据
	InfoRealAmmeter = 0x0406,			//实时电表数据 
	InfoRealStation = 0x0407,			//实时站环境数据
	InfoRealDCModule = 0x0409,			//直流模块数据
	InfoRealDCPdu = 0x040a,				//直流pdu数据
	InfoRealDCCcu = 0x040b,				//直流ccu数据
    InfoRealtimeFaultTotal = 0x040c,        //实时故障总条数 //add by yanwei 20170914
    InfoRealtimeFault = 0x040d,         //实时故障数据    //add by yanwei 20170914

	//历史数据
    InfoHistoryTotal = 0x0501,			//总的历史数据数量，总共条数，总共页数
    InfoHistoryFault = 0x0502,          //历史故障数据
    InfoHistoryCharge = 0x0503,         //历史充电数据
    InfoHistoryOperate = 0x0504,        //历史操作记录
    InfoChargeReport = 0x0505,          //充电完成

	//人机交互
	InfoExchangeParamResult = 0x0601,	//设置结果
	InfoExchangeApplayChargeCmd = 0x0602,	//下发申请卡信息指令
	InfoExchangeApplayChargeResult = 0x0603,	//读申请开始充电/结束充电结果
	InfoExchangeChargeCmd = 0x0604,	//下发启动充电/结束充电命令
	InfoExchangeChargeResult = 0x0605,	//读取启动充电/结束充电结果
	
	InfoExchangeUpdateExportCmd = 0x0606,	//升级 日志导出命令
	InfoExchangeUpdateExportResult = 0x0607,	//升级 日志导出结果

    InfoExchangePasswordCharge = 0x060a,  //本地充电
    InfoExchangeButtonStopCharge = 0x060b,  //按钮结束充电
    InfoExchangePrintPaper = 0x060c,  //打印小票

    InfoDataType = 0xff01               //标示teui到CSCU的数据类型，是设置还是查询

}InfoBodyType;

/*socket数据传输*/
typedef struct __stDataBuffer
{
    unsigned char buff[2048];
    int len;
}stDataBuffer;
Q_DECLARE_METATYPE(stDataBuffer)


/*全局参数结构*/
typedef struct __stTeuiParam
{
	bool showTermWin;			//显示通信中断窗口标志  true 显示
	int capSignal;				//捕获到用户长时间无操作信号
}stTeuiParam;

/*消息之间数据传递结构*/
typedef QMap <InfoBodyType , QVariant> InfoMap;
typedef QMap <InfoBodyType , QByteArray> InfoProtocol;          //发送给协议基类的数据格式

#endif // COMMON_H
