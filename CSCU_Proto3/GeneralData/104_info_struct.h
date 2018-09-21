/*
 * 104_info_struct.h
 *
 * 类描述：104网络通信结构体
 * 创建:  2015-05-13
 * 修改记录:
 * 2015-05-13 创建文件
 * 2015-05-14 1.子站增加总容量变量 2.直流侧数据命名修正
 */


#ifndef STRUCT_104_INFO__H
#define STRUCT_104_INFO__H

enum DEVICE_FAULT{
   NORMAL = 0x00,	//无故障
   UNDERVOLTAGE = 0x01,	//欠压
   OVERVOLTAGE = 0x02,	//过压
   OVERCURRENT = 0x03,	//过流
   // = ...,	//...
   CHARGE_MODULE_ERROR = 0x30,	//充电模块故障
   MAIN_MODULE_ERROR = 0x31,	//主接触器故障
   ASSIST_MODULE_ERROR = 0x32,	//辅助接触器故障
   AC_MODULE_ERROR = 0x33,	//交流接触器故障
   FUSING_ERROR = 0x34,	//熔断器故障
   LIGHTNTING_PROTECTING_ERROR = 0x35,	//防雷故障
   ELECTRONIC_LOCK_ERROR = 0x36,	//电磁锁故障
   ASSIST_POWER_ERROR = 0x37,	//辅助电源故障
   CHARGING_MODULE_FAN_ERROR = 0x38,	//充电模块风扇故障
   DC_OVERVOLTAGE = 0x39,	//直流输出过压
   DC_UNDERVOLTAGE = 0x3A,	//直流输出欠压
   DC_OVERCURRENT = 0x3B,	//直流输出过流
   DC_CONNECT_ERROR = 0x3C,	//直流输出反接
   CHARGING_MODULE_INPUT_OVERVOLTAGE = 0x3D,	//充电模块输入过压
   CHARGING_MODULE_INPUT_UNDERVOLTAGE = 0x3E,	//充电模块输入欠压
   AC_OVERFREQUENCY = 0x3F,	//交流输入频率过频
   AC_UNDERFREQUENCY = 0x40,	//交流输入频率欠频
   AC_VOLTAGE_UNBALANCE = 0x41,	//交流输入电压不平衡
   AC_A_ERROR = 0x42,	//交流输入A相缺相
   AC_B_ERROR  = 0x43,	//交流输入B相缺相
   AC_C_ERROR  = 0x44,	//交流输入C相缺相
   AC_INPUT_OVERLOAD  = 0x45,	//交流输入过载
   AC_INPUT_ERROR = 0x46,	//交流输入异常
   CHARGING_MODULE_OUTPUT_OVERVOLTAGE = 0x47,	//充电模块输出过压
   CHARGING_MODULE_OUTPUT_OVERCURRENT = 0x48,	//充电模块过流
   CHARGING_MODULE_TEMPERATURE_HIGH = 0x49,	//充电模块过温
   ENVIRONMENT_TEMPERATURE_HIGH_ = 0x4A,	//环境温度过温
   ENVIRONMENT_TEMPERATURE_LOW = 0x4B,	//环境温度过低
   CHARGING_MODULE_COMMUNICATION_ERROR = 0x4C,	//充电模块通信故障
   CHARGING_MODULE_COMMAND_ERROR = 0x4D,	//充电模块命令执行失败
   CHARGING_CONTROL_MODULE_COMMUNICATION_ERROR = 0x4E,	//充电控制器通信故障
   COLLECTION_COMMUNICATION_ERROR = 0x4F,	//采集板通讯故障
   AMMETER_COMMUNICATION_ERROR = 0x50,	//电表通讯异常
   TERMINAL_COMMUNICATION_ERROR = 0x51,	//与集控器通信终端
   CARD_READER_DEVICE_COMMUNICATION_ERROR = 0x52,	//读卡器通信故障
   ISOLATION_ERROR = 0x53,	//绝缘故障
   MODULE_TYPE_ERROR = 0x54,	//模块类型不一致
   EMERGY_STOP = 0x55,	//紧急停机
   // = ...,	//...
   BATTERY_SOC_OVERVOLTAGE = 0x70,	//电池电压过高
   BATTERY_SOC_HIGH = 0x71,	//电池SOC过高
   BATTERY_SOC_LOW = 0x72,	//电池SOC过低
   BATTERY_CHARGING_OVERCURRENT = 0x73,	//电池充电过流
   BATTERY_TEMPERATURE_HIGH = 0x74,	//电池温度过高
   BATTERY_IOSLATION_ERROR = 0x75,	//电池绝缘故障
  //  = ...,	//...
   ZIGBEE_OUT_OF_LINE = 0x90,	//ZIGBEE模块离线
   ZIGBEE_VOLTAGE_WARNING = 0x91	//车档器电压告警
};


/*****************************************************************
遥信信息体
**/
typedef struct
{
    char charge_interface_type;		//充电接口标识
    char link_status;               //连接确认开关状态 0断开、1连接、2插枪且车辆未确认、3插枪且车辆确认、4半连接
    char relay_status;				//输出继电器状态 0断开/1闭合
    char parking_space;				//停车位状态 高四位|低四位  高四位代表车档器立起放平：0未知，1立起，2放平；    低四位代表车位有无车辆：0未知，1有车，2无车；
    char charge_status;				//充电工作状态 0待机/1工作/2故障/3启动中/4暂停/5限制/6离线/7切换/8放电
    char status_fault;				//故障状态	1欠压故障/2过压故障/3过流故障
    char BMS_fault;					//BMS故障信息
    char Stop_Result;               //充电终止原因
    char QunLunCeLue;				//控制模式，群充策略,高4位为群充策略，低4位为控制模式。
    char AuxPowerType;				//辅助电源类型
}FRAME_REMOTE_SINGLE;

/*****************************************************************
遥测信息体1
**/
typedef struct
{
	float A_voltage;						//A相充电电压
	float B_voltage;						//B相充电电压
	float C_voltage;						//C相充电电压
	float A_current;						//A相充电电流	
	float B_current;						//B相充电电流	
	float C_current;						//C相充电电流	
	float active_power;						//总有功功率
	float reactive_power;					//总无功功率
	float power_factor;						//总功率因数
	float neutralLine_current;   			//零线电流
	float voltage_unbalance_rate;			//电压不平衡率
	float current_unbalance_rate;			//电流不平衡率
	float voltage_of_dc;                    //直流侧电压
	float current_of_dc;            		//直流侧电流
    float reserved1;							//预留
    float reserved2;							//预留
    float reserved3;							//预留
    float reserved4;							//预留
    float reserved5;							//预留
    float reserved6;							//预留
}FRAME_REMOTE_MEASUREMENT1;

/*****************************************************************
遥测信息体2
**/
typedef struct
{
	uint active_electric_energy;				//总有电能
	uint reactive_electric_energy;			//总无功电能
    uint ReverseActiveEnergy;		  //反向总有功电能
    uint ReverseReactiveEnergy;     //反向总无功电能

    uint ucRecordNum;  //历史电度记录数----电能过滤使用
    uint deltaActiveEnergy;    //有功电能差量----电能过滤使用(暂时未用)
    uint active_electric_energy_old[3];   //总有电能记录----电能过滤使用
    float active_electric_power_old[3];   //功率差记录----电能过滤使用
}FRAME_REMOTE_MEASUREMENT2;

/*****************************************************************
BMS信息体
**/

//BMS充电阶段
typedef struct
{
	unsigned char ChargeType;//充电模式
	float ChargeVoltageMeasured;//充电电压测量值
	float ChargeCurrentMeasured;//充电电流测量值
    unsigned char MaxSingleBatteryVoltageSerial;//最高单体动力蓄电池电压所在组号
    //unsigned char  MaxSingleBatterySerialNum;//当前荷电状态SOC（%）   //nihai modify 20170525
	short LeftTime;//估算剩余充电时间（min）
	char SingleBatteryNum;//最高单体动力蓄电池电压所在编号
	unsigned char MaxTempPointNum;//最高温度检测点编号
	unsigned char MinTempPointNum;//最低动力蓄电池温度检测点编号
	unsigned char ChargePermitFlag;//BMS充电允许标志

	float BMS_need_voltage;					//BMS需求电压
	float BMS_need_current;					//BMS需求电流
	unsigned char batery_SOC;				//当前SOC

	short max_batery_temperature;           //最高电池温度
	float max_batery_voltage;				//最高电池电压
	short lowest_battery_temperature;		//最低电池温度
	float lowest_charge_voltage;			//最低电池电压
	short reserved1;							//预留
	short reserved2;							//预留
	short reserved3;							//预留
	short reserved4;							//预留

	unsigned char BMS_car_VIN[17];           //BMS传输车辆识别码VIN
	unsigned char car_license_plate[7];      //车牌号信息
}__attribute__ ((packed)) FRAME_BMS_INFO;

//握手阶段
typedef struct
{
    char BMSProtocolVer[20];//BMS协议版本号
	float MaxAllowedVoltage;//最高允许充电总电压
	unsigned char BatteryType;//电池类型
	float BatteryRatedCapacity;//整车动力蓄电池系统额定容量（AH）
	float BatteryRatedVoltage;//整车动力蓄电池额定总电压
    char BatteryManufacturer[8];//电池生产厂商名称
    int BatterySerialNum;//电池组序号
    char BatteryProduceDate[20];//电池组生产日期
	int BatteryChargeTime;//电池组充电次数
	unsigned char BatteryOwnerFlag;//电池组产权标识
    char BMSSoftwareVer[20];//BMS软件版本号
}__attribute__ ((packed)) FRAME_BMS_HAND;

//参数配置阶段
typedef struct
{
	float SingleBatteryMaxAllowedVoltage;//单体动力蓄电池最高允许充电电压
	float MaxAllowedCurrent;//最高允许充电电流
	float BatteryTotalEnergy;//动力蓄电池标称总能量
	float MaxParamAllowedVoltage;//最高允许充电总电压
    float MaxtAllowedTemp;//最高允许温度
    float ParamSOC;//整车动力蓄电池荷电状态（SOC）(参数配置阶段)
	float BatteryVoltage;//整车动力蓄电池当前电池电压
	float MaxOutputVoltage;//最大输出电压
	float MinOutputVoltage;//最小输出电压
	float MaxOutputCurrent;//最大输出电流
	float MinOutputCurrent;//最小输出电流
}__attribute__ ((packed)) FRAME_BMS_PARAM;

//BMS中止充电阶段
typedef struct
{
     int BMSStopReason;//BMS中止充电原因
     int BMSFaultReason;//BMS中止充电故障原因
     int BMSErrorReason;//BMS中止充电错误原因
     int ChargerStopReason;//充电机中止充电原因
     int ChargerFaultReason;//充电机中止充电故障原因
     int ChargerErrorReason;//充电机中止充电错误原因
     float ChargeEndSOC;//中止荷电状态SOC（%）
     float MinSingleVoltage;//动力蓄电池单体最低电压
	 float MaxSingleVoltage;//动力蓄电池单体最高电压
     float MinTemp;//动力蓄电池最低温度
     float MaxTemp;//动力蓄电池最高温度
     unsigned int BMSErrorFrame;//BMS错误报文
     unsigned int ChargerErrorFrame;//充电机错误报文
}__attribute__ ((packed)) FRAME_BMS_CHARGE_TERM;


/*****************************************************************
  子站信息体  子站高压侧遥测数据2字节
 **/
typedef struct
{
	float A_voltage;						//A相充电电压
	float B_voltage;						//B相充电电压
	float C_voltage;						//C相充电电压
	float A_current;						//A相充电电流
	float B_current;						//B相充电电流
	float C_current;						//C相充电电流
	float active_power;						//总有功功率
	float active_powerA;						//有功功率
	float active_powerB;						//有功功率
	float active_powerC;						//有功功率
	float reactive_power;					//总无功功率
	float reactive_powerA;					//无功功率
	float reactive_powerB;					//无功功率
	float reactive_powerC;					//无功功率
	float power_factor;						//总功率因数
	float power_factorA;						//功率因数
	float power_factorB;						//功率因数
	float power_factorC;						//功率因数
	float neutralLine_current;   			//零线电流
	float voltage_unbalance_rate;			//电压不平衡率
	float current_unbalance_rate;			//电流不平衡率
	float harmonic_distortion_rate;			//谐波畸变率

    uint active_absorb_energy;				//吸收有功电能
    uint reactive_capacity_energy;			//容性无功电能
    uint active_liberate_energy;				//释放有功电能
    uint reactive_sensibility_energy;			//感性无功电能
}__attribute__ ((packed)) FRAME_SUB_STATION_INFO;

/*****************************************************************
远程抄表结构体-01H当前电能及最大需量数据
**/
typedef struct
{
    uint current_active_absorb_energy;                            //(当前)正向有功总电能
    uint current_active_absorb_rate1_energy;                 //(当前)正向有功费率1电能
    uint current_active_absorb_rate2_energy;                 //(当前)正向有功费率2电能
    uint current_active_absorb_rate3_energy;                 //(当前)正向有功费率3电能
    uint current_active_absorb_rate4_energy;                 //(当前)正向有功费率4电能
    uint current_active_liberate_energy;                          //(当前)反向有功总电能
    uint current_reactive_absorb_energy;						//(当前)正向无功总电能
    uint current_reactive_liberate_energy;						//(当前)反向无功总电能
    uint current_reactive_absorb_energy1;						//(当前)正向无功总电能-（当前）第一象限无功总电能
    uint current_reactive_absorb_energy2;						//(当前)正向无功总电能-（当前）第二象限无功总电能
    uint current_reactive_liberate_energy1;						//(当前)反向无功总电能-（当前）第三象限无功总电能
    uint current_reactive_liberate_energy2;						//(当前)反向无功总电能-（当前）第四象限无功总电能
    float current_active_absorb_max_demand;                 //(当前)正向有功总最大需量
    char current_active_absorb_max_demand_time[5];	 //(当前)正向有功总最大需量发生时间
    float current_active_liberate_max_demand;                //(当前)反向有功总最大需量
    char current_active_liberate_max_demand_time[5];	 //(当前)反向有功总最大需量发生时间
}__attribute__ ((packed)) FRAME_CURRENT_ENERGY_MAX_DEMAND_DATA;

/*****************************************************************
远程抄表结构体-02H整点冻结电能数据
**/
typedef struct
{
    char hour_freeze_time[5];                                              //（上 1 次）整点冻结时间
    uint hour_freeze_active_absorb_energy;                    //（上 1 次）整点冻结正向有功总电能
    uint hour_freeze_active_liberate_energy;                //（上 1 次）整点冻结反向有功总电能
}__attribute__ ((packed)) FRAME_HOUR_FREEZE_ENERGY_DATA;

/*****************************************************************
远程抄表结构体-03H日冻结电能及最大需量数据
**/
typedef struct
{
    char day_freeze_time[5];                                              //(上1次)日冻结时间
    uint day_freeze_active_absorb_energy;                    //（上1次）日冻结正向有功电能数据
    uint day_freeze_active_absorb_rate1_energy;         //（上1次）正向有功费率1电能
    uint day_freeze_active_absorb_rate2_energy;         //（上1次）正向有功费率2电能
    uint day_freeze_active_absorb_rate3_energy;         //（上1次）正向有功费率3电能
    uint day_freeze_active_absorb_rate4_energy;         //（上1次）正向有功费率4电能
    uint day_freeze_active_liberate_energy;                //（上1次）日冻结反向有功电能数据
    uint day_freeze_reactive_absorb_energy;                //（上1次）日冻结正向无功电能数据
    uint day_freeze_reactive_liberate_energy;               //（上1次）日冻结反向无功电能数据
    uint day_freeze_reactive_absorb_energy1;              //（上1次）日冻结正向无功电能数据-（当前）第一象限无功总电能
    uint day_freeze_reactive_absorb_energy2;              //（上1次）日冻结正向无功电能数据-（当前）第二象限无功总电能
    uint day_freeze_reactive_liberate_energy1;             //（上1次）日冻结反向无功电能数据-（当前）第三象限无功总电能
    uint day_freeze_reactive_liberate_energy2;             //（上1次）日冻结反向无功电能数据-（当前）第四象限无功总电能
    float day_freeze_active_absorb_max_demand;                  //（上1次）日冻结正向有功最大需量
    char day_freeze_active_absorb_max_demand_time[5];    //（上1次）日冻结正向有功最大需量发生时间数据
    float day_freeze_active_liberate_max_demand;                //（上1次）日冻结反向有功最大需量
    char day_freeze_active_liberate_max_demand_time[5];  //（上1次）日冻结反向有功最大需量及发生时间数据
}__attribute__ ((packed)) DAY_FREEZE_ENERGY_MAX_DEMAND_DATA;

/*****************************************************************
远程抄表结构体-04H结算日电能及最大需量数据
**/
typedef struct
{
    uint settlement_active_absorb_energy;                            //(上1结算日)正向有功总电能
    uint settlement_active_absorb_rate1_energy;                 //(上1结算日)正向有功费率1电能
    uint settlement_active_absorb_rate2_energy;                 //(上1结算日)正向有功费率2电能
    uint settlement_active_absorb_rate3_energy;                 //(上1结算日)正向有功费率3电能
    uint settlement_active_absorb_rate4_energy;                 //(上1结算日)正向有功费率4电能
    uint settlement_active_liberate_energy;                          //(上1结算日)反向有功总电能
    uint settlement_reactive_absorb_energy;						//(上1结算日)正向无功总电能
    uint settlement_reactive_liberate_energy;						//(上1结算日)反向无功总电能
    uint settlement_reactive_absorb_energy1;						//(上1结算日)正向无功总电能-（上1结算日）第一象限无功总电能
    uint settlement_reactive_absorb_energy2;						//(上1结算日)正向无功总电能-（上1结算日）第二象限无功总电能
    uint settlement_reactive_liberate_energy1;						//(上1结算日)反向无功总电能-（上1结算日）第三象限无功总电能
    uint settlement_reactive_liberate_energy2;						//(上1结算日)反向无功总电能-（上1结算日）第四象限无功总电能
    float settlement_active_absorb_max_demand;                 //(上1结算日)正向有功总最大需量
    char settlement_active_absorb_max_demand_time[5];	 //(上1结算日)正向有功总最大需量发生时间
    float settlement_active_liberate_max_demand;                //(上1结算日)反向有功总最大需量
    char settlement_active_liberate_max_demand_time[5];	 //(上1结算日)反向有功总最大需量发生时间
}__attribute__ ((packed))  SETTLEMENT_ENERGY_MAX_DEMAND_DATA;


/*****************************************************************
子站信息体  子站环境监测数据2字节
**/
typedef struct
{
    unsigned short smoke_alarm;                                                  //烟感报警器，D0~D15位状态分别表示第1~第16路报警器状态；1为触发，0为报警；
    unsigned short magnetometer;                                                 //门磁报警器，D0~D15位状态分别表示第1~第16路报警器状态；1为触发，0为报警；
    unsigned short temperature;                                                  //子站温度,数据分辨率：1 度/位，-50 度偏移量数据范围： -50-200
    unsigned short humidity;                                                     //子站湿度
    unsigned short reserved1;							//预留
    unsigned short reserved2;							//预留
    unsigned short reserved3;							//预留
    unsigned short reserved4;							//预留
    unsigned short reserved5;							//预留
    unsigned short reserved6;							//预留
    unsigned short reserved7;							//预留
    unsigned short reserved8;							//预留
    unsigned short reserved9;							//预留
    unsigned short reserved10;							//预留
    unsigned short reserved11;							//预留
    unsigned short reserved12;							//预留
    FRAME_REMOTE_MEASUREMENT2 stFrameMeasure;
}FRAME_SUB_STATION_ENVIRONMENT;

/*****************************************************************
充电消耗电能信息体
**/
//起始时间点、总电能——第1时间点、总电能——终止时间点、总电能
typedef struct
{
    int power;						//电能
    char time[7];					//4个字节+7字节时标
} POWER_INFO;

typedef struct
{
    struct
    {
        int power;						//电能
        char time[7];					//4个字节+7字节时标
    } POWER_STATUS[194];
}FRAME_POWER_INFO;

typedef struct
{
    int  power;						//电能
    char time[7];					//4个字节+7字节时标
} ONE_POWER_INFO;

/*****************************************************************
车位锁状态信息体
**/
typedef struct
{
    char rockerarm_states;		//摇臂状态　０未到位　１升锁到位　２降锁到位
    char work_status;               //工作状态　０初始化　１正常运行　３休眠
    char reserved1;				//预留１
    char parkingin_states;				//车位占用状态　０车为空　Ｆ车位占用　５未知
    char reserved2;				//预留２
    char ultrasonicprobe2_fault;				//超声探头２故障　１故障　２正常
    char ultrasonicprobe1_fault;					//超声探头１故障　１故障　２正常
    char geomagneticsensor_fault;               //地磁传感器故障　１故障　２正常
    char reserved3;				//预留３
    char timeoutnotinplace;		//摇臂升降超时未到位
    char deviationposition;         //摇臂升降过程中遇到阻力
    char unknown;                       //其他未知故障
    char reserved4;                      //预留４
}FRAME_CARLOCK_STATES;

#endif
