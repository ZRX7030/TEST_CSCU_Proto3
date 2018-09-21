#ifndef PARMASET_H_
#define PARMASET_H_

#include <QObject>
#include <QDebug>
#include <QSettings>
#include <QVariant>
#include <QList>

#include "Infotag/CSCUBus.h"
#include "GeneralData/GeneralData.h"

#include "Bus/Bus.h"
#include "Log/Log.h"
#include "Database/DBOperate.h"


/*配置文件结构调用宏定义*/
enum
{
	PARAM_CAN0,
	PARAM_CAN1,
	PARAM_NET0,
	PARAM_NET1,
    PARAM_SERVER0,  	//云平台服务器
	PARAM_SERVER1,
    PARAM_SERVERLIST, 	//云平台服务器列表
    PARAM_SERVER_DM,  	//设备管理服务器
	PARAM_CHARGE,
	PARAM_CSCU_SYS,
	PARAM_CCU_SYS,
	PARAM_SCREEN,
    PARAM_POWERLIMIT,
    PARAM_SMARTCHARGE,
    PARAM_COUPLEGUN,
    PARAM_SMARTCAR,
	PARAM_EMERGENCY,	//应急充电
	PARAM_WEBSERVER,	//WebServer配置
    PARAM_MAGNETIC, 	//开门断电功能开关
    PARAM_EXTRA, 		//兼容非标项目配置项
	PARAM_IO,			//以上配置文件存储， 以下数据库存储
	PARAM_AMMETER,
    PARAM_PHASE_TYPE,
	PARAM_TPFV,			//尖峰平谷
	PARAM_TERMINAL,	    //辅助电源类型
	PARAM_LOCALPOLICY,	//本地策略
	PARAM_FGPJ,			//峰谷平尖
    PARM_COUNT
};

/*can信息配置*/
typedef struct _CanConfig
{
	int canAddr;				//can的地址、速率
	int canRate;		
}stCanConfig;

/*云平台服务器信息配置*/
typedef struct _Server0Config
{
	char serverIp1[50];			//服务器1IP地址
	int serverPort1;			//服务1端口
	char serverIp2[50];			//服务器2IP地址
	int serverPort2;			//服务2端口
	char serverIp3[50];			//服务器3IP地址
	int serverPort3;			//服务3端口
	char stationNo[20];			//站地址
	char aesKey[40];			//密钥
    bool encrypt;				//是否加密
}stServer0Config;

/*场站平台服务器信息配置*/
typedef struct _Server1Config
{
	char serverIp[50];			//服务器IP地址
	int serverPort;				//服务端口
	char stationNo[20];			//站地址
	char aesKey[40];			//密钥
	int encrypt;				//是否加密
}stServer1Config;


/*服务器列表配置*/
typedef struct _ServerListConfig
{
	char serverIp1[50];			//备份服务器1地址
	int serverPort1;			//备份服务器1端口
	char serverIp2[50];			//备份服务器2地址
	int serverPort2;			//备份服务器2端口
	char serverIp3[50];			//备份服务器3地址
	int serverPort3;			//备份服务器3端口
}stServerListConfig;

/*设备管理服务器配置  add by XX 2017-03-30*/
typedef struct _ServerDM_Config
{
    char serverIp[50];  //服务器IP地址
    int serverPort;				//服务端口
    char userName[30];  //登录用户名
    char passwd[30];    //登录密码
}stServerDM_Config;

/*网口信息配置*/
typedef struct _NetConfig
{
	char localIp[16];			//ip地址
	char netMask[16];			//子网掩码
	char gateway[16];			//网关
}stNetConfig;

/*CSCU系统配置*/
typedef struct _CSCUSysConfig
{
	char stationName[100];		//站名
	int normalCardType;			//普通版 1、刷卡板 2
	int boardType;				//底板型号 值1/v1.0  2/v2.0
    char version[30];
	char dns[16];
	int password;				//屏密码
    int localChargePassword;    //本地充电密码    add by XX 2016-06-14

	int directCurrent;			//直流桩的数量
    int singlePhase;			//单项桩的数量
    int threePhase;				//三相桩的数量
    unsigned char ucDevType;       //设备类型 1单装，2群充  nihai add
    bool coupleGunEnable; //双枪充电使能，用于区分是否有双枪充电需求
    int CCUnum;                //CCU数量
}stCSCUSysConfig;

/*CCU系统配置相关信息*/
typedef struct _CCUSysConfig
{

}stCCUSysConfig;

/*每个终端有相关参数*/
typedef struct _terminalConfig
{
	unsigned char canaddr;			//canid
	unsigned char strategy;			//群充策略
	unsigned char controlMode;		//控制模式
	unsigned char auxPowerType;		//辅源类型
}stTerminalConfig;
Q_DECLARE_METATYPE(stTerminalConfig)

/*充电功能信息配置*/
typedef struct _ChargeConfig
{
    int coupleGun;    //双枪充电使能0-关闭 1-单枪刷卡充电 2-单枪VIN/车牌号充电 3-单枪刷卡和VIN/车牌号充电
   // int coupleGunNum;  //多枪使能后，允许自动充电的枪个数 例：1,2,3,4    场站内存在的最多的充电枪个数
    int vinOffline;			//断网后VIN启动充电  值 0 1
	int localStop;			//本地结束  值 0 1
	int cardType;			//卡类型  值 1 2 3 4 5
	int vinAuto;			//vin自动充电  值 0 1
	int cardAuto;			//刷卡自动充电 值 0 1 
    int vinType;			//vin或车牌号  1 vin  2 车牌号 3自动识别
	int energyFilter;		//异常电度数过滤  0 1
	int localPolicy;		//本地计费策略
	int fgpjEnable;			//峰谷平尖开关
    int localChargeEnable;  //本地充电使能开关，1：本地充电，0：远程充电
    int localChargeType;    //本地充电方式选择，0：本地密码充电，1：本地按钮充电
    int meterType;			//计量类型 0直流 1交流
    int ticketEnable;        //小票机使能 0否   1是（默认否） add by zrx
    unsigned char ucDevType;       //设备类型 1单装，0群充  nihai add
    int languageType;
}stChargeConfig;

/*屏幕信息配置*/
typedef struct _ScreenConfig
{
	char version[30];
	char screenPwd[15];
}stScreenConfig;

/*IO信息配置*/
typedef struct _IOConfig
{
	char inOpenColse[10];
}stIOConfig;


/*负荷约束配置*/
typedef struct _PowerLimitConfig
{
    bool sPowerLimit_Enable;        //集控负荷约束功能开关

    unsigned int sCCUcount;          //集控下CCU数量
    unsigned int STATION_LIMT_POWER;//场站总限制功率
    unsigned int SAFE_CHARGE_POWER;//场站安全充电功率

    unsigned int sSUMPower_Manual;//手动设置充电限制功率(本地限制功率)
    unsigned int sSUMPower_Server;//服务器下发充电限制功率

    //集控获取限制功率，方式使能
    bool sSUMPower_Ammeter_Enable;//电表动态计算设置限制功率
    bool sSUMPower_Manual_Enable;//点屏设置限制功率
    bool sSUMPower_Server_Enable;//服务器下发设置限制功率

}stPowerLimitConfig;

/*错峰充电配置*/
typedef struct _SmartChargeConfig
{
    bool sSmartCharge_Enable;        //集控负荷约束功能开关
}stSmartChargeConfig;
/*车辆优先级调度*/     //hd 3-17 宝鸡公交新加功能

/*开门断电功能配置*/
typedef struct _MagneticConfig
{
    bool bOpenDoorPowerOutages;//开门断电功能开关，true= 开启　false=关闭
}MagneticConfig;

typedef struct _SmartCarConfig
{
    bool sSmartCar_Enable;        //集控车辆优先级调度功能开关
    //车辆优先级调度CCU相关参数不能从直流机直接获取则从配置文件获取   hd 3-31
    unsigned char CCUtotalnum ;  //CCU个数
    unsigned char gunnum[4];   //第一个CCU下带的PDU个数
    unsigned char ModuleNum[4];  //第一个CCU下带的模块数
}stSmartCarConfig;

/*电表参数*/
typedef struct _ammeterConfig
{
	unsigned char addr[6];			//电表地址
	int voltageRatio;
	int currentRatio;
    int enable;						//1 使能  0关闭
    int devType;					//设备类型  01：DLT-645-97, 02 DLT-645-07, 03 MODBUS
    int funType;                //功能类型　1:进线侧电表　2:功率监测电表　3:远程抄表功能电表
}stAmmeterConfig;

typedef struct _allAmmeterConfig
{
	QList<stAmmeterConfig> ammeterConfig;	
}stAllAmmeterConfig;
Q_DECLARE_METATYPE(stAllAmmeterConfig)


/*三相相别参数*/
typedef struct _phaseTypeConfig
{
	char canaddr;		
	char type;			//1 2 3   a b c 
}stPhaseTypeConfig;

typedef struct _threePhaseTypeConfig
{
	QList<stPhaseTypeConfig> phaseTypeConfig;
}stThreePhaseTypeConfig;
Q_DECLARE_METATYPE(stThreePhaseTypeConfig)


/*尖峰平谷时间段参数设置*/
typedef struct _TPFVConfig
{
	int time_seg;
	int start_hour;
	int start_minute;
	int stop_hour;
	int stop_minute;
    int limit_soc;    //限制充电SOC
    int limit_current;//超出限制SOC情况下限制充电的电流
}stTPFVConfig;

typedef struct _allTPFVConfig
{
	QList<stTPFVConfig> tpfvConfig;
}stAllTPFVConfig;
Q_DECLARE_METATYPE(stAllTPFVConfig)

//紧急充电配置信息
typedef struct _EmergencyChargeConfig
{
	bool emergency_enable;	//是否开启紧急充电功能
	bool vin_authenticate;	//是否开启VIN鉴权
	bool card_authenticate;	//是否开启卡号鉴权
	bool car_authenticate;	//是否开启车牌号鉴权
	bool queue_gun;			//轮充触发-插枪
	bool queue_card;		//轮充触发-刷卡
	bool queue_car;			//轮充触发-车辆信息
	int  check_time;		//触发应急状态的断网时间
	int  duration;			//应急充电持续时间
	int  order_count;		//应急充电最大订单数
}EmergencyConfig;

//WebServer配置信息
typedef struct _WebServerConfig
{
	char url[255];				//WebServer地址
	ushort port;				//端口
	char operator_id[30];		//组织机构代码
	char operator_secret[30];	//运营商密钥
	char data_secret[20];		//运营商密钥
	char aes_key[20];			//AES密钥
	char aes_iv[20];			//AES初始向量
}WebServerConfig;

/*本地策略时间段参数设置*/
typedef struct _LocalPolicyConfig
{
	int policy_index;
	int start_hour;
	int start_minute;
	int stop_hour;
	int stop_minute;
	int electric_fee;
	int service_fee;
}LocalPolicyConfig;

typedef struct _allLocalPolicyConfig
{
	QList<LocalPolicyConfig> policyConfig;
}AllLocalPolicyConfig;
Q_DECLARE_METATYPE(AllLocalPolicyConfig)

/*峰谷平尖时间段参数设置*/
typedef struct _FGPJConfig
{
	int time_seg;
	int start_hour;
	int start_minute;
	int stop_hour;
	int stop_minute;
}FGPJConfig;

typedef struct _allFGPJConfig
{
	QList<FGPJConfig> fgpjConfig;
}AllFGPJConfig;
Q_DECLARE_METATYPE(AllFGPJConfig)

/*Protobuf服务器信息配置*/
typedef struct _PbServerConfig
{
	char charge_host[50];	//充电服务器地址
	int charge_port;		//充电服务器端口
	int charge_encrypt;		//是否加密 0：不加密 1：ssl
	char monitor_host[50];	//运行监控服务器地址
	int monitor_port;		//运行监控服务器端口
	int monitor_encrypt;	//是否加密 0：不加密 1：ssl
	char station[20];		//站地址
	char key[40];			//密码
}PbServerConfig;

//兼容非标项目配置项 非标项不使用配置工具，需手动配置
typedef struct _ExtraConfig
{
	int coupleGun300KW;		//兼容武汉非标300KW双枪
	int localAddress;		//兼容对接第三方平台网络地址获取
}ExtraConfig;

/*配置文件联合体*/
typedef union _paramConfig
{
	stCanConfig can0Config;
	stCanConfig can1Config;
	stServer0Config server0Config;
	stServer1Config server1Config;
	stServerListConfig serverListConfig;
    stServerDM_Config serverDMConfig;   //add by XX 2017-03-30
	stNetConfig net0Config;
	stNetConfig net1Config;
	stCSCUSysConfig cscuSysConfig;
	stCCUSysConfig ccuSysConfig;
	stChargeConfig chargeConfig;
	stScreenConfig screenConfig;
	stIOConfig ioConfig;
    stPowerLimitConfig powerLimitConfig;
    stSmartChargeConfig smartChargeConfig;
    stSmartCarConfig smartCarConfig;
	EmergencyConfig emergencyConfig;
	WebServerConfig webserverConfig;
	LocalPolicyConfig localpolicyConfig;
	FGPJConfig fgpjConfig;
    MagneticConfig magneticConfig;
	ExtraConfig extraConfig;
}unParamConfig;
Q_DECLARE_METATYPE(unParamConfig)

//2018-3-8 hd
const int GUNTYPE =1;  //充电枪类型
const int BOWTYPE =2; //充电弓类型

class Q_DECL_IMPORT ParamSet : public CModuleIO
{
    Q_OBJECT
private:
	QString filename;
	stCanConfig can0Config;
	stCanConfig can1Config;
	stServer0Config server0Config;
	stServer1Config server1Config;
	stServerListConfig serverListConfig;
    stServerDM_Config serverDMConfig;   //add by XX 2017-03-30
	stNetConfig net0Config;
	stNetConfig net1Config;
	stCSCUSysConfig cscuSysConfig;
	stCCUSysConfig ccuSysConfig;
	stChargeConfig chargeConfig;
	stScreenConfig screenConfig;
	stIOConfig ioConfig;
    stPowerLimitConfig powerLimitConfig;
    stSmartChargeConfig smartChargeConfig;
    stSmartCarConfig smartCarConfig;
	EmergencyConfig emergencyConfig;
	WebServerConfig webserverConfig;
	LocalPolicyConfig localpolicyConfig;
	FGPJConfig fgpjConfig;
    MagneticConfig magneticConfig;
    ExtraConfig extraConfig;
	
	CBus *bus;
	Log *log;
	DBOperate *db;
	
	int rebootFlag;
	bool terminalNumChange; 
	bool dnsChange;
	bool configInited;

	void loadSetting();
	void initDefaultSetting();
	void updateConfig(int);
	/*电表参数*/	
	void queryAmmeterConfig(stAllAmmeterConfig *);
	void updateAmmeterConfig(stAllAmmeterConfig &config);
	/*相别参数*/
	void queryPhaseTypeConfig(stThreePhaseTypeConfig *);
	void updatePhaseTypeConfig(stThreePhaseTypeConfig &config);
	/*尖峰平谷参数*/
//	void queryTPFVConfig(stAllTPFVConfig *);
	void updateTPFVConfig(stAllTPFVConfig &config);
	/*直流机终端参数*/
	void queryTerminalConfig(stTerminalConfig *config, unsigned char canaddr);
	void updateTerminalConfig(stTerminalConfig config, unsigned char canaddr);
	
    bool queryLocalPolicyConfig(AllLocalPolicyConfig *conf);
    bool updateLocalPolicyConfig(AllLocalPolicyConfig *conf);
    bool queryFGPJConfig(AllFGPJConfig *conf);
    bool updateFGPJConfig(AllFGPJConfig *conf);

    //终端名称初始化(数据库)
    bool CheckDBTermName();
    void InitTermNameDB();
    //初始化终端名称列表(写数据库)--单桩  nihai add
    void InitSignleTermNameDB();

	bool loadAmmeterRange();
	QMap<uchar, double> m_mapAmmeterRange;

	bool loadAcMeterScale();
	QMap<uchar, double> m_mapAcMeterScale;
    void InitMultiGunName();

signals:
	void sigUpdateSetting(QVariant var, int type);
    void sigToBus(InfoMap TelecontrolMap, InfoAddrType enAddrType);	//配置变化

protected:
	ParamSet();

public:
	static ParamSet *GetInstance();
	~ParamSet();

	bool updateSetting(void *, int);
	bool querySetting(void *, int);
    /*尖峰平谷参数*/
    void queryTPFVConfig(stAllTPFVConfig *);
	int needReboot();					//获取重启标志  0 不许重启 1 重启程序 2 重启系统
	void setRebootFlag(int flag);		//设置重启标志

	double getAmmeterRange(uchar cCanAddr);
	double getAcMeterScale(uchar cCanAddr);

    //动态库接口moduleIO
    int InitModule(QThread* pThread);
    int RegistModule();
    int StartModule();
    int StopModule();
    int ModuleStatus();

private slots:
	void slotUpdateSetting(QVariant, int);
    //void slotFromBus(InfoMap TelecontrolMap, InfoAddrType enAddrType); //test
};

#endif
