#ifndef INFODATA_H
#define INFODATA_H

#include  <QObject>

/*集控设置*/
typedef struct _CSCUParam
{
	char ipAddr[16];
	char netmask[16];
	char gateway[16];
	char dns[16];
    char server1Host[30];
    unsigned short server1Port;
    char server2Host[30];
    unsigned short server2Port;
    char server3Host[30];
    unsigned short server3Port;
	char stationAddr[20];
	unsigned char canAddr;
	char threePhaseNum;
	char singlePhaseNum;
    char dcPhaseNUm;
}__attribute__((packed)) stCSCUParam;
Q_DECLARE_METATYPE(stCSCUParam)

/*三相相别设置*/
typedef struct _PhaseType
{
	unsigned char canAddr;
    unsigned char phaseType;
}__attribute__((packed)) stPhaseType;

typedef struct _AllPhaseType
{
	QList<stPhaseType> phaseList;
}stAllPhaseType;
Q_DECLARE_METATYPE(stAllPhaseType)

/*二维码生成*/
typedef struct _QRcodeCreate
{
    unsigned char canAddr;
    char terminalCode[20];
}__attribute__((packed)) stQRcodeCreate;

typedef struct _AllQRcodeCreate
{
    QList<stQRcodeCreate> codeList;
}stAllQRcodeCreate;
Q_DECLARE_METATYPE(stAllQRcodeCreate)
/*特殊功能设置*/
typedef struct _SpecialFunc
{
   unsigned char vinOffline;         //断网后VIN启动充电(后六位)  值 0 1
   unsigned char localStop;          //本地结束  值 0 1
   unsigned char cardType;           //卡类型  值 1 2 3 4 5
   unsigned char vinAuto;            //vin自动充电  值 0 1
   unsigned char cardAuto;           //刷卡自动充电 值 0 1
   unsigned char vinType;            //vin或车牌号  1 vin  2 车牌号
   unsigned char energyFilter;       //异常电度数过滤  0 1
   unsigned char chargeMode;         //充电方式使能　　0远程   1本地
   unsigned char localType;          //本地充电方式　0密码　　1按钮
   unsigned char pileType;           //单桩/群充设置  add by yanwei 20171011
   unsigned char coupleGun;          //多枪设置
   unsigned char printPaper;          //打印小票
   unsigned char languageSelect;      //语言选择　　1 中文　2英文
}__attribute__((packed)) stSpecialFunc;
Q_DECLARE_METATYPE(stSpecialFunc)

/*直流终端设置*/
typedef struct __DCTerminalParam
{
	unsigned char canAddr;				//终端id
	char strategy;				//群冲策略
	char controlMode;			//控制模式
	char powerType; 			//辅助电源类型
}__attribute__((packed)) stDCTerminalParam;
Q_DECLARE_METATYPE(stDCTerminalParam)

/*负荷约束配置*/
typedef struct _PowerLimitParam
{
	char sPowerLimit_Enable;			//集控负荷约束功能开关

	unsigned char sCCUcount;				//集控下CCU数量
	unsigned short STATION_LIMT_POWER;	//子站总限制功率
	unsigned short SAFE_CHARGE_POWER;		//子站安全充电功率

	unsigned short sSUMPower_Manual;		//本地设置充电限制功率
	//unsigned short sSUMPower_Server;		//服务器下发充电限制功率

	//集控获取限制功率，方式使能
	char sSUMPower_Ammeter_Enable;		//电表动态计算设置限制功率
	char sSUMPower_Server_Enable;		//服务器下发设置限制功率
	char sSUMPower_Manual_Enable;		//本地设置限制功率
}__attribute__((packed)) stPowerLimitParam;
Q_DECLARE_METATYPE(stPowerLimitParam)

/*错峰充电结构体*/
typedef struct _TPFVParam
{
    unsigned char time_seg;
    unsigned char start_hour;
    unsigned char start_minute;
    unsigned char stop_hour;
    unsigned char stop_minute;
    unsigned char limit_soc;
    unsigned char limit_current;
}__attribute__((packed)) stTPFVParam;

typedef struct _allTPFVParam
{
    char peakCharegeEnable;		 //错峰充电开启关闭 0 1
	QList<stTPFVParam> tpfvList;
}stAllTPFVParam;
Q_DECLARE_METATYPE(stAllTPFVParam)


/*IO 配置结构体*/
typedef struct _IOConfigParam
{
	unsigned char io[10];
}__attribute__((packed)) stIOConfigParam;
Q_DECLARE_METATYPE(stIOConfigParam)

/*直流机终端设备、监控设备数量*/
typedef struct _DCChargerDeviceNum
{
	unsigned char type;				//1 终端设备， 2 监控设备
	QList<unsigned char> addrList;	//can地址数量
}stDCChargerDeviceNum;
Q_DECLARE_METATYPE(stDCChargerDeviceNum)

/*直流机终端参数*/
typedef struct _DCChargerTermParam
{
	unsigned char canAddr;				//终端can地址
	unsigned char powerType;			//辅源类型 1代表 12v， 2 代表24v
    unsigned char lowTempEnable;		//低温使能 1 代表不使能，2代表使能
	unsigned char elcLockEnable;		//电子锁使能
    unsigned char elcLockTypeEnable;	//电子锁类型
	unsigned char vinEnable;			//Vin使能
	unsigned char newOldStd;			//新老国标
	unsigned short maxChargeCurrent;	//充电最大电流
	unsigned char chargePriority;		//充电优先等级
    unsigned char chargeType;			//充电方式（轮充、群充）
    unsigned char chargeStrategy;		//充电策略（a,b,c）
	unsigned char pduId;				//Pdu  ID 号
}__attribute__((packed)) stDCChargerTermParam;
Q_DECLARE_METATYPE(stDCChargerTermParam)

/*直流机监控参数*/
typedef struct _DCChargerMonitorParam
{
	unsigned char canAddr;				//Ccu 的can地址
    unsigned short maxPower;				//直流柜最大功率设置
	unsigned char gunStartAddr;			//直流柜枪头起始地址
    unsigned char setCanAddr;				//保留字节 设置时表示更改的ccu id
}__attribute__((packed)) stDCChargerMonitorParam;
Q_DECLARE_METATYPE(stDCChargerMonitorParam)

/*终端状态数据*/
typedef struct _TerminalStatus
{
	unsigned char canAddr;
	char linkStatus;
	char chargeStatus;
	char status;
    char soc;
    char name[20];          //终端名称
}__attribute__((packed)) stTerminalStatus;
Q_DECLARE_METATYPE(stTerminalStatus)

/*cscu常用状态*/
typedef struct _CSCUStatus
{
	char serverStatus;			//服务器连接状态 1 连接  2未连接
    char udiskStatus;           //u盘插入状态	 1 插入  2 未插入
    char chargeMode;            //是否进入应急模式　0正常模式　1进入应急模式
}__attribute__((packed)) stCSCUStatus;
Q_DECLARE_METATYPE(stCSCUStatus)


/*终端信息实时查询*/
typedef struct _TerminalReal
{
	unsigned char canAddr;					//can地址
	unsigned char status;					//当前终端状态
	short voltageA;		//a相电压
	short currentA;		//a相电流
	short voltageB;		//b相电压
	short currentB;		//b相电流
	short voltageC;				//c相电压
	short currentC;				//c相电流
	short totalActivePower;		//总有功功率/kw
	short totalNactivePower;	//总无功功率/kw
	short powerFactor;			//总功率因数
    short volUnbalance;			//电压不平衡率
	short curUnbalance;			//电流不平衡率
	short zeroLineCurrent;		//零线电流
	short DCVolatge;			//直流侧电压
	short DCCurrent;			//直流侧电流
	int totalActiveEnergy;		//总有功电能
	int totalNactiveEnergy;		//总无功电能
    short guntemp;  //枪头温度	//merge by yanwei 20171011
}__attribute__((packed)) stTerminalReal;
Q_DECLARE_METATYPE(stTerminalReal)

/*终端BMS信息查询*/
typedef struct _TerminalBMS
{
	unsigned char canAddr;					//can地址
	unsigned char status;					//当前终端状态
	short BMSNeedVoltage;
	short BMSNeedCurrent;
	char currentSoc;
	short maxBatteryTemp;
	short maxBatteryVoltage;
	short minBatteryTemp;
	short minBatteryVoltage;
}__attribute__((packed)) stTerminalBMS;
Q_DECLARE_METATYPE(stTerminalBMS)

/*终端故障信息查询*/
typedef struct _TerminalFault
{
    unsigned char canAddr;			//can地址
    unsigned char numID;             //内部ID
    char failureStarttime[20];       //故障开始时间
    char faultInfo[40];				//故障内容
}__attribute__((packed)) stTerminalFault;
Q_DECLARE_METATYPE(stTerminalFault)

typedef struct _AllTerminalFault
{
    unsigned char totalnum;         //条目数量
    QList<stTerminalFault>faultList;
    char chargestartTime[20];       //充电开始时间
    char chargestopTime[20];        //充电结束时间
    unsigned char currentSOC;       //当前SOC
    char stopReson[60];             //终止原因
}stAllTerminalFault;
Q_DECLARE_METATYPE(stAllTerminalFault)

/*电表地址查询*/
typedef struct _AmmeterAddr
{
	unsigned char ammeterAddr[6];
}__attribute__((packed)) stAmmeterAddr;

typedef struct _AllAmmeterAddr
{
	QList<stAmmeterAddr> ammeterList;
}stAllAmmeterAddr;
Q_DECLARE_METATYPE(stAllAmmeterAddr)

/*电表数据查询*/
typedef struct _AmmeterData
{
	unsigned char addr[6];
	short voltageA;		//a相电压
	short currentA;		//a相电流
	short voltageB;		//b相电压
	short currentB;		//b相电流
	short voltageC;				//c相电压
	short currentC;				//c相电流
    int totalNactivePower;		//总无功功率
    int totalACtivePower;		//总有功功率
    short powerFactor;			//总功率因数
	int sensNactiveEnenrgy;			//感性无功电能
	int capNactiveEnergy;			//容性无功电能
	short zeroLineCurrent;		//零线电流
	int ActiveAbsorbEnergy; //正向有功电能
	int ActiveLiberateEnergy;//反向有功电能
}__attribute__((packed)) stAmmeterData;
Q_DECLARE_METATYPE(stAmmeterData)

/*充电中实时信息查询*/
typedef struct _ChargeReal
{
	unsigned char canAddr;
	short voltageA;		//电压
	short voltageB;
    short voltageC;
	short currentA;		//电流
    short currentB;
    short currentC;
    short DCvoltage;
    short DCcurrent;
	short power;		//功率
    unsigned int currentEnergy;		//当前电能值
    unsigned short chargeEnergy;		//已充电能值
	char startTime[20];		//开始充电时间
	short chargeTime;		//充电时长 min
	unsigned char currentSoc;	//当前soc
}__attribute__((packed)) stChargeReal;
Q_DECLARE_METATYPE(stChargeReal)

/*环境数据查询*/
typedef struct _StationRealData
{
	short tempeture;
	unsigned short humidity;
	unsigned char alarm[10];
}__attribute__((packed)) stStationRealData;
Q_DECLARE_METATYPE(stStationRealData)

/*直流机下pdu、模块、ccu个数以及id号*/
typedef struct _DCChargerTypeNum
{
	unsigned char canAddr;
	unsigned char id;					//ID号
}__attribute__((packed)) stDCChargerTypeNum;

typedef struct _AllDCChargerTypeNum
{
	unsigned char type;				//1/pdu, 2/ccu, 3/module
	QList<stDCChargerTypeNum> listNum;
}stAllDCChargerTypeNum;
Q_DECLARE_METATYPE(stAllDCChargerTypeNum)

/*直流模块数据结构*/
typedef struct _DCModuleRealData
{
	unsigned char canAddr;
	unsigned char id;
	unsigned char work_status;
	unsigned short out_volatge;
    short out_current;
	unsigned short m1_tempeture;
	unsigned short in_a_volatge;
	unsigned short in_b_volatge;
	unsigned short in_c_volatge;
	unsigned char module_group;			//模块分组
	unsigned char warning_status;		//告警状态
	char seq[40];						//序列号
	char soft_version1[20];
	char soft_version2[20];
	char soft_version3[20];
    char hard_version[20];
}__attribute__((packed)) stDCModuleRealData;
Q_DECLARE_METATYPE(stDCModuleRealData)

/*pdu数据结构*/
typedef struct _DCPduRealData
{
	unsigned char canAddr;
	unsigned char id;
	unsigned char work_status;
	unsigned short out_volatge;
    short out_current;
    unsigned short tempeture;			//散热器温度
	unsigned char warning_status;		//告警状态
	char seq[40];						//序列号
	char soft_version1[20];
	char soft_version2[20];
	char soft_version3[20];
    char hard_version[20];
}__attribute__((packed)) stDCPduRealData;
Q_DECLARE_METATYPE(stDCPduRealData)

/*ccu数据结构*/
typedef struct _DCCcuRealData
{
	unsigned char canAddr;
	unsigned char id;
	unsigned char work_status;
	unsigned short tempeture;			//散热器温度
	unsigned short out_power;			//当前输出功率
	unsigned char warning_status;		//告警状态
	char seq[40];						//序列号
	char soft_version1[20];
	char soft_version2[20];
	char soft_version3[20];
    char hard_version[20];
}__attribute__((packed)) stDCCcuRealData;
Q_DECLARE_METATYPE(stDCCcuRealData)

/*查询历史数据数量*/
typedef struct _HistoryInfo
{
	short type;
	short totalPage;	//页数
    short totalNum;		//总共条数
}__attribute__((packed)) stHistoryInfo;
Q_DECLARE_METATYPE(stHistoryInfo)

/*查询历史充电数据*/
typedef struct _HistoryCharge
{
	unsigned char canAddr;
	char startTime[20];
	char stopTime[20];
	short chargeEnergy;
    char stopReson[60];  //40改60 by songqb
}__attribute__((packed)) stHistoryCharge;

typedef struct _AllHistoryCharge
{
	unsigned char currentPage;
	QList<stHistoryCharge> chargeList;
}stAllHistoryCharge;
Q_DECLARE_METATYPE(stAllHistoryCharge)

/*查询历史故障*/
typedef struct _HistoryFault
{
	unsigned char canAddr;
    unsigned char inID;
    unsigned char minPDUAddr;
    unsigned char maxPDUAddr;
	char startTime[20];
	char stopTime[20];
	char faultReson[40];
    unsigned char property;    //属性　１代表终端故障　２代表设备故障
}__attribute__((packed)) stHistoryFault;


typedef struct _AllHistoryFault
{
	unsigned char currentPage;
	QList<stHistoryFault> faultList;
}stAllHistoryFault;
Q_DECLARE_METATYPE(stAllHistoryFault) 

/*查询历史操作*/
typedef struct _HistoryOperate
{
	char type;					//操作类型
	char operateId[50];			//操作者身份			
    char operateTime[20];
    char operateType[50];
}__attribute__((packed)) stHistoryOperate;

typedef struct _AllHistoryOperate
{
    unsigned char currentPage;
    QList<stHistoryOperate> operateList;
}stAllHistoryOperate;
Q_DECLARE_METATYPE(stAllHistoryOperate)

/* add by yanwei 20170914 */
/*查询实时故障数据数量*/
typedef struct _RealtimeInfo
{
    short totalPage;	//页数
    short totalNum;		//总共条数
}__attribute__((packed)) stRealtimeInfo;
Q_DECLARE_METATYPE(stRealtimeInfo)

/* add by yanwei 20170914 */
/*查询实时故障*/
typedef struct _RealTimeFault
{
    unsigned char canAddr;
    unsigned char inID;
    unsigned char minPDUAddr;
    unsigned char maxPDUAddr;
    char startTime[20];
    char faultReson[40];
    unsigned char property;    //属性　１代表终端故障　２代表设备故障
}__attribute__((packed)) stRealtimeFault;

/* add by yanwei 20170914 */
typedef struct _AllRealtimeFault
{
    unsigned char currentPage;
    QList<stRealtimeFault> faultList;
}stAllRealtimeFault;
Q_DECLARE_METATYPE(stAllRealtimeFault)

/*查询充电报告*/
typedef struct _ChargeReport
{
	unsigned char canAddr;
	unsigned char startSoc;
	unsigned char stopSoc;
	short chargeEnergy;
	char startTime[20];
	char stopTime[20];
	short chargeTime;
    char stopReson[60];
}__attribute__((packed)) stChargeReport;
Q_DECLARE_METATYPE(stChargeReport)


/*版本信息*/
typedef struct _VersionInformation
{
    char kernelVersion[20];    //内核版本
    char fileSysVersion[20];   //文件系统版本
    char cscuProgram[20];     //CSCU软件版本
    char teuiProgram[20];      //teui程序版本
    char hardwareVersion[20];  //硬件版本
    char macAddress[30];       //mac地址
    char serialNumber[30];     //序列号
}__attribute__((packed)) stVersionInformation;
Q_DECLARE_METATYPE(stVersionInformation)


/*设置交互结果*/

typedef struct _ExchangeResult
{
    unsigned short type;
    unsigned char result;
	unsigned char rebootFlag;  //0 不重启 1 重启
}__attribute__((packed)) stExchangeResult;
Q_DECLARE_METATYPE(stExchangeResult)

/*充电交互下发刷卡指令*/
typedef struct _ApplayCardCmd
{
	unsigned char canAddr;
	unsigned char status;
    unsigned char cardcmd;          //1 申请读卡 2结束读卡
	unsigned char cmd;				//1 开始充电  2 结束充电
}__attribute__((packed)) stApplayCardCmd;
Q_DECLARE_METATYPE(stApplayCardCmd)
/*充电交互下发用户信息指令*/
typedef struct _ApplayCmd
{
    unsigned char canAddr;
    unsigned char status;
    unsigned char cmd;				//1 开始充电  2 结束充电
}__attribute__((packed)) stApplayCmd;
Q_DECLARE_METATYPE(stApplayCmd)
/*本地充电指令*/
typedef struct _LocalApplayCharge
{
    unsigned char canAddr;
    unsigned char status;
    unsigned char type;        //1 本地密码　　2本地按钮
    unsigned char chargecmd;  //1 开始充电  2 结束充电
}__attribute__((packed)) stLocalApplayCharge;
Q_DECLARE_METATYPE(stLocalApplayCharge)

/*申请结果*/
typedef struct _ApplayChargeResult
{
	unsigned char canAddr;
	unsigned char status;
	unsigned char cmd;
	unsigned char result;
	char failReson[50];
	unsigned char cardNum[8];
	int leftMoney;
}__attribute__((packed)) stApplayChargeResult;
Q_DECLARE_METATYPE(stApplayChargeResult)

/*启动充电 停电*/
typedef struct _ChargeCmd
{
	unsigned char canAddr;
	unsigned char status;
	unsigned char cmd;
	unsigned char chargeType;
	unsigned short chargeEnergy;
}__attribute__((packed)) stChargeCmd;
Q_DECLARE_METATYPE(stChargeCmd)

/*启动 结束结果*/
typedef struct _ChargeResult
{
	unsigned char canAddr;
	unsigned char status;
	unsigned char cmd;
	unsigned char result;
	char failReson[50];
	unsigned char cardNum[8];
	unsigned int leftMoney;
}__attribute__((packed)) stChargeResult;
Q_DECLARE_METATYPE(stChargeResult)

/*按钮结束结果*/
typedef struct _ButtonStopResult
{
    unsigned char canAddr;
    unsigned char status;
    unsigned char cmd;
    unsigned char result;
}__attribute__((packed)) stButtonStopResult;
Q_DECLARE_METATYPE(stButtonStopResult)

/*打印小票结果*/
typedef struct _PrintPaperResult
{
    unsigned char canAddr;
    unsigned char result;
    char failReson[50];
}__attribute__((packed)) stPrintPaperResult;
Q_DECLARE_METATYPE(stPrintPaperResult)

/*升级 导出命令下发*/
typedef struct _UpdateExportCmd
{
	unsigned char cmdType;	// 1 升级 2 导出日志 3 导出数据库
}__attribute__((packed)) stUpdateExportCmd;
Q_DECLARE_METATYPE(stUpdateExportCmd)

typedef struct _UpdateExportResult
{
	unsigned char cmdType;	// 1 升级 2 导出日志 3 导出数据库
	unsigned char result;
}__attribute__((packed)) stUpdateExportResult;
Q_DECLARE_METATYPE(stUpdateExportResult)

///*充电模式选择*/
//typedef struct _ChargeModeSelect
//{
//    unsigned char chargeType;  //0 远程充电模式 2 本地充电模式
//}__attribute__((packed)) stChargeModeSelect;
//Q_DECLARE_METATYPE(stChargeModeSelect)

///*本地充电模式选择*/
//typedef struct _LocalChargeSelect
//{
//    unsigned char localType;  //0 本地密码充电模式 2 本地按钮充电模式
//}__attribute__((packed)) stLocalChargeSelect;
//Q_DECLARE_METATYPE(stLocalChargeSelect)

#endif // INFODATA_H
