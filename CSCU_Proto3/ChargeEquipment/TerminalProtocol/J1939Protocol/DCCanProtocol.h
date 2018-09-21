#ifndef DCCANPROTOCOL_H
#define DCCANPROTOCOL_H
#include "J1939GeneralProtocol.h"
#include <QDateTime>
#include <QFile>

////广播召唤定时
//const unsigned char ucBroadCastInterval = 100;
//对时定时
const unsigned short usSetTimeInterval = 6000;

typedef enum _ProVerEnum_DC
{
    ProVer_Old_DC = 0, //V2.0.12及之前版本
    ProVer_2_0_19_DC = 1 //2.0.15版本(V2.0.13, 2.0.14 并未投入生产使用)

}ProVerEnum_DC;

//直流PGN枚举
typedef enum _DCPDUType
{
    PF_ChargerState1_DC = 0x30,
    PF_ChargerState2_DC = 0x31,
    PF_ChargerState3_DC = 0x32,
    PF_ChargerState4_DC = 0x33,
    PF_ChargerState5_DC = 0x34,
    PF_ChargerState6_DC = 0x35,
    PF_ChargerState7_DC = 0x36,
    PF_ChargerState8_DC = 0x37,
    PF_ChargerState9_DC = 0x38,
    PF_QueueInfoUpload_DC = 0x42,
    PF_CardSetChargeEnergy_DC = 0x48,
    PF_ChargeManner_DC = 0x49, //充电方式：单枪充电／双枪充电
    PF_ChargeGunGroup_DC = 0x50, //多枪分组信息下发
    PF_SetDispatchInfo_DC = 0x72,
    PF_EstimateChargeTimeUpload_DC = 0x73,
    PF_VIN_DC = 0x74,
    PF_GunTemperature_DC = 0x75,
    PF_LoadDispatch_DC = 0x76,
    PF_ModuleFaultInfo_DC = 0x77,
    PF_AlarmPointRecord_CCU = 0x78,
    PF_FaultRecord_CCU = 0x79,
    PF_BMS_HandShake1_DC = 0x7F,
    PF_TwoGunSet_Old_DC = 0x80, //原有双枪设置兼容
    PF_BMS_HandShake2_DC = 0x81,
    PF_BMS_HandShake3_DC = 0x82,
    PF_BMS_ParamSet1_DC = 0x83,
    PF_BMS_ParamSet2_DC = 0x84,
    PF_BMS_Charging1_DC = 0x85,
    PF_BMS_Charging2_DC = 0x86,
    PF_BMS_Charging3_DC = 0x87,
    PF_BMS_ChargeEnd1_DC = 0x88,
    PF_BMS_ChargeEnd2_DC = 0x89,
    PF_BMS_ChargeEnd3_DC = 0x8A,
    PF_BMS_ChargeEnd4_DC = 0x8B,
    PF_BMS_ChargeEnd5_DC = 0x8C,
    PF_ModuleInfo1_CCU = 0xA1,
    PF_ModuleInfo2_CCU = 0xA2,
    PF_ModuleInfo3_CCU = 0xA3,
    PF_PDUInfo1_CCU = 0xA5,
    PF_PDUInfo2_CCU = 0xA6,
    PF_PDUInfo3_CCU = 0xA7,
    PF_BranchInfo1_CCU = 0xA8,
    PF_BranchInfo2_CCU = 0xA9,
    PF_CCUInfo1_CCU = 0xAA,
    PF_CCUInfo2_CCU = 0xAB,
    PF_PDUInfo4_CCU = 0xAC,
    PF_ActiveProtectionSet_DC = 0xB0,
    PF_FlexibleChargeSet_DC = 0xB1,
    PF_GeneralStaticParamSet_DC = 0xB2,
    PF_GeneralDynamicParamSet_DC = 0xB3,
    PF_TwoGunParamSet_DC = 0xB4,    //未用
    PF_CCUParamSet_CCU = 0xB5,
    PF_BranchParamSet_CCU = 0xB6,
    PF_PDUParamSet_CCU = 0xB7,
    PF_ModuleParamSet_CCU = 0xB8,

    PF_JIGUI  = 0xDF   //机柜信息 smm add
}DCPDUType;

//直流短帧长度枚举
typedef enum _DCShortFrameLength
{
    DL_ChargerState1_DC = 8,
    DL_ChargerState2_DC = 8,
    DL_ChargerState3_DC = 8,
    DL_ChargerState4_DC = 7,
    DL_ChargerState5_DC = 8,
    DL_ChargerState6_DC = 8,
    DL_ChargerState7_DC = 8,
    DL_ChargerState8_DC = 4,
    DL_ChargerState9_DC = 8,
    DL_QueueInfoUpload_DC = 8,
    DL_CardSetChargeEnergy_DC = 2,
    DL_SetDispatchInfo_DC = 8,
    DL_EstimateChargeTimeUpload_DC = 8,
    DL_GunTemperature_DC = 8,
    DL_LoadDispatchUpload_DC = 8,
    DL_ModuleFaultInfoUpload_DC = 8,

    DL_BMS_HandShake1_DC = 3,
    DL_BMS_HandShake2_DC = 2,

    DL_BMS_ParamSet2_DC = 8,
    DL_BMS_Charging1_DC = 7,
    DL_BMS_Charging2_DC = 8,
    DL_BMS_Charging3_DC = 7,
    DL_BMS_ChargeEnd1_DC = 4,
    DL_BMS_ChargeEnd2_DC = 4,
    DL_BMS_ChargeEnd3_DC = 7,
    DL_BMS_ChargeEnd4_DC = 4,
    DL_BMS_ChargeEnd5_DC = 4,
    DL_ModuleInfo1_CCU = 8,
    DL_ModuleInfo2_CCU = 8,
    DL_ModuleInfo3_CCU = 8,
    DL_PDUInfo1_CCU = 8,
    DL_PDUInfo2_CCU = 8,
    DL_PDUInfo3_CCU = 8,
    DL_BranchInfo1_CCU = 8,
    DL_BranchInfo2_CCU = 8,
    DL_CCUInfo1_CCU = 8,
    DL_CCUInfo2_CCU = 8,
    DL_PDUInfo4_CCU = 8,
    //    DL_FlexibleChargeSet_DC = ,// 不定长
    DL_GeneralStaticParamSet_DC = 8,
    DL_GeneralDynamicParamSet_DC = 8,
    DL_TwoGunParamSet_DC = 8,
    DL_CCUParamSet_CCU = 8,
    DL_BranchParamSet_CCU = 8,
    DL_PDUParamSet_CCU = 8,
    DL_ModuleParamSet_CCU = 8,
    DL_300kwWorkMode_DC =2

}DCShortFrameLength;

//直流长帧长度枚举
typedef enum _DCLongFrameLength
{
    DL_VIN_DC = 19,
    DL_AlarmPointRecord_CCU = 92,
    DL_FaultRecord_CCU = 1282,
    DL_BMS_HandShake3_DC = 50,
    DL_BMS_ParamSet1_DC = 13,
    DL_ActiveProtectionSet_DC = 50,
    DL_FlexibleChargeSet_DC = 81

}DCLongFrameLength;

//直流充电状态1中相关故障结构体
typedef struct _State1Fault_DC
{
    unsigned char ucModuleError                 : 1;//充电模块故障 0 正常、1 故障
    unsigned char ucMainContactorError  	: 1;//主接触器故障 0 正常、1 故障
    unsigned char ucAuxContactorError		: 1;//辅助接触器故障 0 正常、1 故障
    unsigned char ucReserved		: 3;
    unsigned char ucOutputOverVoltage	: 1;//直流输出过压 0 正常、1 过压
    unsigned char ucOutputOverCurrent	: 1;//直流输出过流 0 正常、1 过流
}__attribute__ ((packed))State1Fault_DC;

//直流充电状态1
typedef struct _FrameChargerState1_DC
{
    struct
    {
        unsigned char ucLinkState		: 2;//0断开/1半连接/2连接
        unsigned char ucWorkState		: 2;//0待机状态/1充电状态/2故障状态/3启动中
        unsigned char ucRelyState		: 1;//0断开/1闭合
        unsigned char ucWorkStateChild	: 3;//1暂停/2限制
    }stStatus;
    State1Fault_DC stFault;
    unsigned short usChargeVoltage;//充电机充电电压
    short sChargeCurrent;//充电机充电电流
    unsigned char ucFaultCode;//故障代码
    unsigned char ucGroupQueueFlag;//群充轮充标识
}__attribute__ ((packed)) FrameChargerState1_DC;


//直流充电状态2
typedef struct _FrameChargerState2_DC
{
    unsigned short usBMSNeedVoltage;//BMS 电压需求
    short sBMSNeedCurrent;//BMS 电流需求
    unsigned char ucBaterySOC;//电池当前SOC
    unsigned char ucMaxBateryTemperature;//最高电池温度
    short sMaxBateryVoltage;//最高电池电压
}__attribute__ ((packed)) FrameChargerState2_DC;


//直流充电状态3
typedef struct _FrameChargerState3_DC
{
    unsigned short usChargeTime;//本次累计充电时间
    unsigned int uiTotalChargeEnergy;//累计充电电量
    unsigned char ucChargeEndCode;//充电中止代码
    unsigned char ucReserved;//预留
}__attribute__ ((packed)) FrameChargerState3_DC;

//直流充电状态4中相关故障结构体1
typedef struct _State4Fault1_DC
{
    unsigned char ucBatteryVoltage	: 2;//电池电压过高 00 正常；01 过高；10 不可信状态
    unsigned char ucBatterySOC		: 2;//电池SOC 过高/过低   00 正常；01 过高；10 过低
    unsigned char ucBatteryCurrent	: 2;//电池充电过流00 正常；01 不正常；10 不可信状态
    unsigned char ucBatteryTemperature	: 2; //电池温度过高00 正常；01 不正常；10 不可信状态
}__attribute__ ((packed))State4Fault1_DC;

//直流充电状态4中相关故障结构体2
typedef struct _State4Fault2_DC
{
    unsigned char ucBatteryInsulationState	: 2;//电池绝缘状态00 正常；01 不正常；10 不可信状态
    unsigned char ucBatteryConnecterState	: 2;//电池输出连接器连接状态 00 正常；01 不正常；10 不可信状态
    unsigned char ucChargePermit			: 2;//充电允许00 禁止；01 允许
    unsigned char ucReserved			: 2;//预留
}__attribute__ ((packed))State4Fault2_DC;

//直流充电状态4
typedef struct _FrameChargerState4_DC
{
    unsigned char ucLowestBatteryTemperature;//最低电池温度
    unsigned short usLowestChargeVoltage;//最低电池电压数(单体电池电压)
    State4Fault1_DC stFaultFlag1;//故障结构体1
    State4Fault2_DC stFaultFlag2;//故障结构体2
    unsigned short usActivePower;//充电有功功率
    unsigned char ucReserved;//预留
}__attribute__ ((packed)) FrameChargerState4_DC;

//直流充电状态5
typedef struct _FrameChargerState5_DC
{
    unsigned short usAPhaseVoltage;		//A相充电电压
    unsigned short usBPhaseVoltage;		//B相充电电压
    unsigned short usCPhaseVoltage;		//C相充电电压
    short sAPhaseCurrent;				//A相充电电流
}__attribute__ ((packed)) FrameChargerState5_DC;

/*****************************************************************
直流充电状态6 旧协议
**/
typedef struct _FrameChargerState6_old_DC
{
    short sBPhaseCurrent;		//B相充电电流
    short sCPhaseCurrent;		//C相充电电流
    short sZeroLineCurrent; //零线电流
    unsigned char ucReserved[2];//预留
}__attribute__ ((packed)) FrameChargerState6_old_DC;

/*****************************************************************
直流充电状态6 新协议
**/
typedef struct _FrameChargerState6_new_DC
{
    short sBPhaseCurrent;		//B相充电电流
    short sCPhaseCurrent;		//C相充电电流
    short sZeroLineCurrent; //零线电流
    unsigned short usActivePower; //有功功率
}__attribute__ ((packed)) FrameChargerState6_new_DC;

/*****************************************************************
直流充电状态7
**/
typedef struct _FrameChargerState7_DC
{
    unsigned short usReactivePower;		//总无功功率
    unsigned short usPowerFactor;		//总功率因数
    short sVoltageUnbalanceRate;	//电压不平衡率
    short sCurrentUnbalanceRate;	//电流不平衡率
}__attribute__ ((packed)) FrameChargerState7_DC;

/*****************************************************************
直流充电状态8
**/
typedef struct _FrameChargerState8_DC
{
    unsigned int uiReactiveElectricEnergy;	//总无功电能
    unsigned char ucReserved[4];	//预留
}__attribute__ ((packed)) FrameChargerState8_DC;

//直流充电状态9
typedef struct _FrameChargerState9_DC
{
    unsigned int uiReverseActiveEnergy;		  //反向总有功电能
    unsigned int uiReverseReactiveEnergy;     //反向总无功电能
}__attribute__ ((packed)) FrameChargerState9_DC;

//直流上传单双枪充电分组信息
typedef struct _FrameChargerMannerInfo_DC
{
    unsigned char chargeManner;//充电方式
    unsigned char canID_master;//主枪can地址
    unsigned char canID_slave1;//副枪1can地址
    unsigned char canID_slave2;//副枪2can地址
    unsigned char canID_slave3;//副枪3can地址
    unsigned char canID_slave4;//副枪4can地址
    unsigned char canID_slave5;//副枪5can地址
    unsigned char canID_slave6;//副枪6can地址
    //unsigned char ucReserved[5];     //预留字节
}__attribute__ ((packed)) FrameChargerMannerInfo_DC;

//设置直流双枪充电工作模式
typedef struct _FrameWorkMode_DC
{
    unsigned char cCanID;//终端CAN ID
    unsigned char cWorkMode;//工作模式 1-单枪单充　２－双枪单充
}__attribute__ ((packed)) FrameWorkMode_DC;

//直流上传排队信息
typedef struct _FrameQueueInfoUpload_DC
{
    unsigned char ucQueueInfo;		  //排队信息
    unsigned char ucReserved[7];     //预留字节
}__attribute__ ((packed)) FrameQueueInfoUpload_DC;

//刷卡设置充电量信息
typedef struct _FrameCardSetChargeEnergy_DC
{
    unsigned short usChargeEnergy;
}__attribute__ ((packed)) FrameCardSetChargeEnergy_DC;

//设置调度信息
typedef struct _FrameSetDispatchInfo_DC
{
    unsigned char ucParamType;
    union
    {
        struct  //ucCmdType == 1时
        {
            unsigned char ucPriority;
            unsigned char ucReserved[6];
        }stCmd01;
        struct  //ucCmdType == 2时,BCD码
        {
            unsigned char ucSec;
            unsigned char ucMin;
            unsigned char ucHour;
            unsigned char ucDay;
            unsigned char ucMonth;
            unsigned char ucYearLow;
            unsigned char ucYearHigh;
        }stCmd02;
        struct //ucCmdType == 3时
        {
            unsigned char ucChargeMode;
            unsigned char ucReserved[6];
        }stCmd03;
        struct //ucCmdType == 4时
        {
            unsigned int uiGridMaxLoad;
            unsigned char ucReserved[3];
        }stCmd04;
        struct //ucCmdType == 5时
        {
            unsigned char ucGroupChargeStrategy;
            unsigned char ucReserved[6];
        }stCmd05;
        struct //ucCmdType == 6时
        {
            unsigned char ucAuxPowerType;
            unsigned char ucReserved[6];
        }stCmd06;
    }Data;
}__attribute__ ((packed)) FrameSetDispatchInfo_DC;

//上传预估完成时间
typedef struct _FrameEstimateChargeTimeUpload_DC
{
    unsigned char ucSec;
    unsigned char ucMin;
    unsigned char ucHour;
    unsigned char ucDay;
    unsigned char ucMonth;
    unsigned char ucYearLow;
    unsigned char ucYearHigh;
    unsigned char ucReserved;
}__attribute__ ((packed)) FrameEstimateChargeTimeUpload_DC;

//上传VIN
typedef struct _FrameVIN_DC
{
    unsigned char ucDataType;
    unsigned char ucDataLength;
    char        chData[17];
}__attribute__ ((packed)) FrameVIN_DC;

//上传充电枪温度
typedef struct _FrameGunTemperature_DC
{
    unsigned short usGunTemperature;
    unsigned char ucReserved[6];
}__attribute__ ((packed)) FrameGunTemperature_DC;

//直流上传负荷调度相关状态
typedef struct _FrameLoadDispatch_DC
{
    unsigned char ucGroupType;       //群充轮充控制模式
    unsigned char ucGroupStrategy;          //群充策略
    unsigned char ucAuxPowerType; //辅助电源类型
    unsigned char ucFreeModuleFlag;//是否有空闲模块
    unsigned char ucQueueMsg;         //车辆排队信息
    unsigned char ucReserved[3];           //保留
}__attribute__ ((packed)) FrameLoadDispatch_DC;

//上传模块故障告警信息
typedef struct _FrameModuleFaultInfo_DC
{
    unsigned char ucReserved1;
    unsigned char ucModuleID;
    unsigned char ucAlarmCode;
    unsigned char ucMinPDUID;
    unsigned char ucMaxPDUID;
    unsigned char ucFaultState;
    unsigned char ucFaultRecordFlag;
    unsigned char ucReserved2;
}__attribute__ ((packed)) FrameModuleFaultInfo_DC;

//上传模块故障点记录
typedef struct _FrameAlarmPointRecord_DC
{
    unsigned char ucData[DL_AlarmPointRecord_CCU];
}__attribute__ ((packed)) FrameAlarmPointRecord_DC;

//上传故障录波信息
typedef struct _FrameFaultRecord_DC
{
    unsigned char ucData[DL_FaultRecord_CCU];
}__attribute__ ((packed)) FrameFaultRecord_DC;

//BMS握手阶段1
typedef struct _FrameBMSHandShake1
{
    unsigned char ucBMSProtocolVer1;//BMS协议版本号
    unsigned short usBMSProtocolVer2;//BMS协议版本号
}__attribute__ ((packed)) FrameBMSHandShake1;

//BMS握手阶段2
typedef struct _FrameBMSHandShake2
{
    unsigned short usMaxAllowedVoltage;//最高允许充电总电压
}__attribute__ ((packed)) FrameBMSHandShake2;

//BMS握手阶段3
typedef struct _FrameBMSHandShake3
{
    unsigned char ucBMSProtocolVer1;//BMS协议版本号1
    unsigned short usBMSProtocolVer2;   //BMS协议版本号2,3
    unsigned char ucBatteryType;//电池类型
    unsigned short usBatteryRatedCapacity;//电池额定容量
    unsigned short usBatteryRatedVoltage;//电池额定电压
    char cManufacturer[4]; //ASCII,生产厂商
    unsigned int uiBatterySerialNum;//电池组序号
    unsigned char ucProduceYear;//生产日期:年
    unsigned char ucProduceMonth;//生产日期:月
    unsigned char ucProduceDay;//生产日期:日
    unsigned char ucChargeTimeCount[3];//充电次数
    unsigned char ucOwnerFlag;//电池组产权标识
    unsigned char ucReserved;
    char chVIN[17];//ASCII, VIN号
    unsigned char ucBMSSoftwareVer[8];//BMS软件版本号

}__attribute__ ((packed)) FrameBMSHandShake3;

//BMS参数配置阶段1
typedef struct _FrameBMSParamSet1
{
    unsigned short usSingleBatteryMaxAllowedVlotage;//单体电池最大允许电压
    unsigned short usMaxAllowedCurrent;//最大允许电流
    unsigned short usBatteryTotalEnergy;//电池总容量,kwh
    unsigned short usMaxParamAllowedVoltage;//最大允许电压
    unsigned char ucMaxAllowedTemperature;//最高允许温度
    unsigned short usParamSOC;//参数配置阶段SOC
    unsigned short usBatteryVoltage;//电池电压

}__attribute__ ((packed)) FrameBMSParamSet1;

//BMS参数配置阶段2
typedef struct _FrameBMSParamSet2
{
    unsigned short usMaxOutputVoltage;//最大输出电压
    unsigned short usMinOutputVoltage;//最小输出电压
    unsigned short usMaxOutputCurrent;//最大输出电流
    unsigned short usMinOutputCurrent;//最小输出电流

}__attribute__ ((packed)) FrameBMSParamSet2;

//BMS充电阶段1
typedef struct _FrameBMSCharging1
{
    unsigned short usNeedVoltage;//需求电压
    unsigned short usNeedCurrent;//需求电流
    unsigned char ucChargeType;//充电类型
    unsigned short usRemainChargeTime;//剩余充电时间

}__attribute__ ((packed)) FrameBMSCharging1;

//BMS充电阶段2
typedef struct _FrameBMSCharging2
{
    unsigned short usChargeVoltageMeasured;//充电电压测量值
    unsigned short usChargeCurrentMeasured;//充电电流测量值
    unsigned short usHighestSingleBatteryVoltage;//最高单体电池电压
    unsigned char ucHighestSingleBatterySerialNum;//最高单体电池组号
    unsigned char ucNowSOC;//当前SOC

}__attribute__ ((packed)) FrameBMSCharging2;

typedef struct _BMSCharge3Fault_DC
{
    struct
    {
        unsigned char ucSingleBatteryVoltageError : 2;//单体动力蓄电池电压异常
        unsigned char ucSOCError : 2;//整车动力蓄电池SOC异常
        unsigned char ucOverCurrent : 2;//过流
        unsigned char ucTempTooHigh : 2;//过温
    }__attribute__ ((packed))stByte1;
    struct
    {
        unsigned char ucInsulationState : 2;//绝缘状态
        unsigned char ucOutputConnectorState : 2;//输出连接器状态
        unsigned char ucChargePermitFlag : 2;//BMS充电允许标识
        unsigned char ucReserved : 2;
    }__attribute__ ((packed))stByte2;
}__attribute__ ((packed))BMSCharge3Fault_DC;

//BMS充电阶段3
typedef struct _FrameBMSCharging3
{
    unsigned char ucSingleBatteryNum;//单体电池编号
    unsigned char ucHighestBatteryTemp;//最高电池温度
    unsigned char ucHighestTempPointNum;//最高温度检测点号
    unsigned char ucLowestBatteryTemp;//最低电池温度
    unsigned char ucLowestTempPointNum;//最低温度检测点号
    BMSCharge3Fault_DC stFault;
}__attribute__ ((packed)) FrameBMSCharging3;

//BMS充电结束阶段1
typedef struct _FrameBMSChargeEnd1
{
    struct
    {
        unsigned char ucSOCReach : 2;//达到需求的SOC值
        unsigned char ucTotalVoltageReach : 2;//达到总电压设定值
        unsigned char ucSingleVoltageReach : 2;//达到单体电压设定值
        unsigned char ucChargerAutoStop : 2;//充电机主动终止
    }__attribute__ ((packed))stBMSStopReason;//BMS中止原因
    struct
    {
        unsigned char ucInsulationFault : 2;//绝缘故障
        unsigned char ucLinkerOverTemp : 2;//输出连接器过温故障
        unsigned char ucBMSOverTemp : 2;//BMS过温
        unsigned char ucLinkerFault: 2;//充电连接器故障
        unsigned char ucBatteryTempTooHigh: 2;//电池组温度过高故障
        unsigned char ucHighVoltageRelayFault: 2;//高压继电器故障
        unsigned char ucTestPoint2Fault: 2;//检测点2故障
        unsigned char ucOtherFault: 2;//其他故障
    }__attribute__ ((packed))stBMSFaultReason;//BMS中止故障原因
    struct
    {
        unsigned char ucOverCurrent : 2;//电流过大
        unsigned char ucAbnormalVoltage : 2;//电压异常
        unsigned char ucReserved : 4;//预留
    }__attribute__ ((packed))stBMSErrorReason;//BMS错误原因
}__attribute__ ((packed)) FrameBMSChargeEnd1;

//BMS充电结束阶段2
typedef struct _FrameBMSChargeEnd2
{
    struct
    {
        unsigned char ucChargerPointReach : 2;//达到充电机设定条件
        unsigned char ucArtificalStop : 2;//人工中止
        unsigned char ucFaultStop : 2;//故障中止
        unsigned char ucBMSAutoStop : 2;//BMS主动终止
    }__attribute__ ((packed))stChargerStopReason;//充电机终止原因
    struct
    {
        unsigned char ucChargerOverTemp : 2;//充电机过温故障
        unsigned char ucChargerLinkerFault : 2;//充电机连接器故障
        unsigned char ucChargerInsideOverTempFault : 2;//充电机内部过温故障
        unsigned char ucEnergyTransFault: 2;//能量传输故障
        unsigned char ucEmergencyStopFault: 2;//急停故障
        unsigned char ucOtherFault: 2;//其他故障
        unsigned char ucReserved: 4;//预留
    }__attribute__ ((packed))stChargerFaultReason;//充电机故障原因
    struct
    {
        unsigned char ucCurrentNoMatching : 2;//电流不匹配
        unsigned char ucAbnormalVoltage : 2;//电压异常
        unsigned char ucReserved : 4;//预留
    }__attribute__ ((packed))stChargerErrorReason;//充电机错误原因

}__attribute__ ((packed)) FrameBMSChargeEnd2;

//BMS充电结束阶段3
typedef struct _FrameBMSChargeEnd3
{
    unsigned char ucChargeEndSOC;//中止SOC状态
    unsigned short usLowestSingleVoltage;//最低单体电池电压
    unsigned short usHighestSingleVoltage;//最高单体电池电压
    unsigned char ucLowestTemperature;//最低温度
    unsigned char ucHighestTemperature;//最高温度

}__attribute__ ((packed)) FrameBMSChargeEnd3;

//BMS充电结束阶段4
typedef struct _FrameBMSChargeEnd4
{
    struct
    {
        unsigned char ucRecvCRMFlag_0x00 : 2;
        unsigned char ucRecvCRMFlag_0xAA : 2;
        unsigned char ucReserved : 4;
    }stByte1;
    struct
    {
        unsigned char ucRecvCTSCMLTimeOut : 2;
        unsigned char ucRecvCROTimeOut : 2;
        unsigned char ucReserved : 4;
    }stByte2;
    struct
    {
        unsigned char ucRecvCCSTimeOut : 2;
        unsigned char ucRecvCSTTimeOut : 2;
        unsigned char ucReserved : 4;
    }stByte3;
    struct
    {
        unsigned char ucRecvCSDTimeOut : 2;
        unsigned char ucReserved : 6;
    }stByte4;
}__attribute__ ((packed)) FrameBMSChargeEnd4;

//BMS充电结束阶段5
typedef struct _FrameBMSChargeEnd5
{
    struct
    {
        unsigned char ucRecvBRMTimeOut : 2;
        unsigned char ucReserved : 6;
    }stByte1;
    struct
    {
        unsigned char ucRecvBCPTimeOut : 2;
        unsigned char ucRecvBROTimeOut : 2;
        unsigned char ucReserved : 4;
    }stByte2;
    struct
    {
        unsigned char ucRecvBCSTimeOut : 2;
        unsigned char ucRecvBCLTimeOut : 2;
        unsigned char ucRecvBSTTimeOut : 2;
        unsigned char ucReserved : 2;
    }stByte3;
    struct
    {
        unsigned char ucRecvBSDTimeOut : 2;
        unsigned char ucReserved : 6;
    }stByte4;
}__attribute__ ((packed)) FrameBMSChargeEnd5;

//直流模块信息1
typedef struct _FrameModuleInfo1_CCU
{
    unsigned char ucID;//内部ID
    struct
    {
        unsigned char ucWorkState : 4;//工作状态
        unsigned char ucReserved : 4;
    }stByte2;
    unsigned char ucReserved;
    unsigned short usOutVoltage;//输出电压
    unsigned short usOutCurrent;//输出电流
    unsigned char ucM1Temperature;//M1温度
}__attribute__ ((packed)) FrameModuleInfo1_CCU;

//直流模块信息2
typedef struct _FrameModuleInfo2_CCU
{
    unsigned char ucID; //内部ID
    unsigned short usInAVoltage;//A相输入电压
    unsigned short usInBVoltage;//B相输入电压
    unsigned short usInCVoltage;//C相输入电压
    unsigned char ucEnvTemperature;//环境温度
}__attribute__ ((packed)) FrameModuleInfo2_CCU;

//直流模块信息3
typedef struct _FrameModuleInfo3_CCU
{
    unsigned char ucID; //内部ID
    unsigned short usInCurrent;//输入电流
    unsigned short usTotalRunTime;//总运行时间
    unsigned short usSwitchTime;//切换次数
    unsigned char ucGroupNum;//所属分组
}__attribute__ ((packed)) FrameModuleInfo3_CCU;

//PDU信息1
typedef struct _FramePDUInfo1_CCU
{
    unsigned char ucID;//内部ID
    struct
    {
        unsigned char ucWorkState : 4;//工作状态
//        unsigned char ucAlarmState : 2;//告警状态
        unsigned char ucGreenLight : 1;//绿灯
        unsigned char ucYellowLight : 1;//黄灯
        unsigned char ucRedLight : 1;//红灯
        unsigned char ucReserved : 1;
    }stByte2;
    unsigned char ucReserved;
    unsigned short usOutVoltage;//输出电压
    unsigned short usOutCurrent;//输出电流
    unsigned char ucEnvTemperature; //环境温度
}__attribute__ ((packed)) FramePDUInfo1_CCU;

//PDU信息2
typedef struct _FramePDUInfo2_CCU
{
    unsigned char ucID;//内部ID
    unsigned char ucRadiatorTemperature;//散热器温度
    unsigned short usTotalRunTime;//总运行时间
    unsigned short usSwitchTime;//切换次数
    unsigned short usAmmeterEnergy;   //电表度数 分辨率 : 20kwh
}__attribute__ ((packed)) FramePDUInfo2_CCU;

//PDU信息3
typedef struct _FramePDUInfo3_CCU
{
    unsigned char ucID;//内部ID
    unsigned short usResistanceMinus;//负对地阻值
    unsigned short usResistancePlus;//正对地阻值
    unsigned char ucReserved[3];   //预留
}__attribute__ ((packed)) FramePDUInfo3_CCU;

//PDU信息4
typedef struct _FramePDUInfo4_CCU
{
    unsigned char ucID;//内部ID
    short sSetVol;  //PDU设模块输出电压
    short sSetCur;  //PDU设模块限流点
    unsigned char ucReserved[3];   //预留

}__attribute__ ((packed)) FramePDUInfo4_CCU;

//分支箱信息1
typedef struct _FrameBranchInfo1_CCU
{
    unsigned char ucID;//内部ID
    struct
    {
        unsigned char ucWorkState : 4;//工作状态
//        unsigned char ucAlarmState : 2;//告警状态
        unsigned char ucReserved : 4;
    }stByte2;
    unsigned char ucReserved;
    unsigned short usOutVoltage;//输出电压
    unsigned short usOutCurrent;//输出电流
    unsigned char ucEnvTemperature; //环境温度
}__attribute__ ((packed)) FrameBranchInfo1_CCU;

//分支箱信息2
typedef struct _FrameBranchInfo2_CCU
{
    unsigned char ucID;//内部ID
    unsigned char ucM1Temperature;//M1板温度
    unsigned short usTotalRunTime;//总运行时间
    unsigned short usSwitchTime;//切换次数
    unsigned char ucReserved[2];   //电表度数 分辨率 : 20kwh
}__attribute__ ((packed)) FrameBranchInfo2_CCU;

//CCU信息1
typedef struct _FrameCCUInfo1_CCU
{
    unsigned char ucReserved1;
    struct
    {
        unsigned char ucWorkState : 4; //工作状态
        unsigned char ucReserved : 4;
    }stByte2;
    unsigned char ucReserved3;//预留
    struct
    {
        unsigned char ucInputContactor : 2; //输入接触器状态
        unsigned char ucLinkageContactor : 2;//联动接触器状态
        unsigned char ucReserved : 4;
    }stByte4;
    unsigned char ucReserved5;
    unsigned char ucEnvTemperature; //环境温度
    unsigned char ucModleType;//设备类型
    unsigned char ucReserved8;
}__attribute__ ((packed)) FrameCCUInfo1_CCU;

//CCU信息2
typedef struct _FrameCCUInfo2_CCU
{
    unsigned short usRetedPower;//额定功率
    unsigned short usNowPower; //当前需求功率
    unsigned short uiTotalRunTime;//总运行时间
    unsigned short usOutPower; //当前out_power
}__attribute__ ((packed)) FrameCCUInfo2_CCU;

//双枪参数设置
typedef struct _FrameTwoGunParamSet
{
    unsigned char ucTheOtherCanID;//配对枪的CAN地址
    unsigned char ucTwoGunWorkMode;//双枪工作模式
    unsigned char ucTwoGunSwitchMode;//双枪切换模式
    unsigned char ucReserved[5];

}__attribute__ ((packed)) FrameTwoGunParamSet;

//smm add
//机柜结构体
typedef struct {
    unsigned short  SensorAddr; //传感器地址
    unsigned short  temp;       //温度
    unsigned short  humidty;     //湿度
    unsigned short  Reserved;
}__attribute__((packed)) JiGuiwenDuShiDu;
//qiang A B
typedef struct Qiang{
    short QiAtemp;//枪头A温度

}__attribute__((packed)) Qiang;
//smm end
class cDCCanProtocol : public cJ1939GeneralProtocol
{
public:
    cDCCanProtocol();
    ~cDCCanProtocol();
    //是否有帧需要处理(目前为长帧)
    virtual bool HasFrameToDeal();
    //解析CAN数据
    virtual void ParseCanData(can_frame *pCanFrame);
    //解析主动防护设置<----控制中心
    virtual void ParseActiveProtect(InfoMap CenterMap, unsigned char ucTermID);
    //解析柔性充电设置<----控制中心
    virtual void ParseFlexibleCharge(InfoMap CenterMap, unsigned char ucTermID);
    //解析通用静态参数设置<----控制中心
    virtual void ParseGeneralStaticArg(InfoMap CenterMap, unsigned char ucTermID);
    //解析通用动态参数设置<----控制中心
    virtual void ParseGeneralDynamicArg(InfoMap CenterMap, unsigned char ucTermID);

    //解析查询主动防护设置<----控制中心
    virtual void ParseQueryActiveProtect(InfoMap CenterMap, unsigned char ucTermID);
    //解析查询柔性充电设置<----控制中心
    virtual void ParseQueryFlexibleCharge(InfoMap CenterMap, unsigned char ucTermID);
    //解析查询通用静态参数设置<----控制中心
    virtual void ParseQueryGeneralStaticArg(InfoMap CenterMap, unsigned char ucTermID);
    //解析查询通用动态参数设置<----控制中心
    virtual void ParseQueryGeneralDynamicArg(InfoMap CenterMap, unsigned char ucTermID);

    //解析CCU参数设置<----控制中心
    virtual void ParseCCUArg(InfoMap CenterMap, unsigned char ucTermID);
    //解析CCU参数查询<----控制中心
    virtual void ParseCCUArgQuery(InfoMap CenterMap, unsigned char ucTermID);
     //解析机柜温度和湿度      //smm add
    virtual void ParseJiGui(unsigned char * pData,QList<CanMapNode> & ToCenterList,unsigned char canID);
    virtual void ParseQiangTou(unsigned char * pData,QList<CanMapNode> & ToCenterList);

    //解析能效系统模块控制<----控制中心
    virtual void ParseCtrlModule_EP(InfoMap CenterMap, unsigned char ucTermID);
    //解析能效系统功率控制<----控制中心
    virtual void ParseSetPower_EP(InfoMap CenterMap, unsigned char ucTermID);

protected:
    //规范充电机枪连接状态
    virtual unsigned char CheckLinkState(unsigned char ucLinkStateIn);
    //解析遥调指令<----控制中心
    virtual void ParseTeleadjust(InfoMap CenterMap, unsigned char ucTermID);
    //获取参数设置结果类型
    virtual int GetParamAckType(unsigned char ucPF);
    //获取协议版本号枚举
    virtual void GetProVerEnum(unsigned char * pVer);
    virtual void ParseChargeGunGroupInfo(InfoMap CenterMap, unsigned char ucTermID);
    virtual void ParseDoubleSys300kwSetting(InfoMap CenterMap, unsigned char ucTermID);
private:
    //检查广播召唤帧状态
    void CheckBroadCastState();
    //获取故障代码----直流充电状态1
    unsigned char CheckFaultCode_State1(State1Fault_DC stFault, unsigned char ucFaultCodeIn);
    //获取故障代码----BMS充电阶段3
    unsigned char CheckFaultCode_BMSCharge3(BMSCharge3Fault_DC &stFault);
    //获取充电机工作状态----直流充电状态1
    unsigned char CheckWorkState(unsigned char ucWorkState, unsigned char ucChildWorkState);
    //获取故障代码----直流充电状态4
    unsigned char CheckFaultCode_State4(State4Fault1_DC stFault1, State4Fault2_DC stFault2);

    //生成--设置调度信息帧
//    void MakeFrameSetDispatchInfo(can_frame *pCanFrame, unsigned char ucCanID, unsigned char ucType, unsigned char ucData);
    //生成终端主动防护帧
    void MakeFrameActiveProtect(InfoMap &CenterMap, cLongPackageModule * pModule);
    //生成终端柔性充电帧
    void MakeFrameFlexibleCharge(InfoMap &CenterMap, cLongPackageModule * pModule);
    //生成CCU设置帧----0xB5  旧接口, 具体设置项赋值
    void MakeFrameCCUParamSet(unsigned int uiKey, QByteArray CenterData, can_frame * &pFrame, unsigned char ucCanID);
    //生成CCU设置帧----0xB5  新接口, 直接内存拷贝
    void MakeFrameCCUParamSet(QByteArray argArray, can_frame * &pFrame, unsigned char ucCanID);

    //生成通用静态参数设置帧
    void MakeFrameStaticArg(InfoMap &CenterMap, can_frame *pCanFrame, unsigned char ucCanID);
    //发送通用动态参数设置帧
//    void MakeFrameDynamicArg(InfoMap &CenterMap, can_frame *pCanFrame, unsigned char ucCanID);

    //发送终端主动防护帧
    void SendFrameActiveProtect(InfoMap &CenterMap, unsigned char ucCanID);
    //发送终端柔性充电帧
    void SendFrameFlexibleCharge(InfoMap &CenterMap, unsigned char ucCanID);
    //发送通用静态参数设置帧
    void SendFrameStaticArg(InfoMap &CenterMap, unsigned char ucCanID);
    //发送CCU参数设置帧
    void SendFrameCCUArg(InfoMap &CenterMap, unsigned char ucCanID);

    //解析充电机状态1
    void ParseFrameState1(unsigned char * pData, QList<CanMapNode>  &ToCenterList);
    //解析充电机状态2
    void ParseFrameState2(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析充电机状态3
    void ParseFrameState3(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析充电机状态4
    void ParseFrameState4(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析充电机状态5
    void ParseFrameState5(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析充电机状态6----旧协议
    void ParseFrameState6_old(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析充电机状态6----新协议
    void ParseFrameState6_new(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析充电机状态7
    void ParseFrameState7(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析充电机状态8
    void ParseFrameState8(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析充电机状态9
    void ParseFrameState9(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析充电方式信息
   // bool CheckChargeMannerInfo(FrameChargerMannerInfo_DC strFrame, unsigned char canID);
    void ParseFrameChargeManner(can_frame *, QList<CanMapNode> &ToCenterList);
    //解析模块故障信息
    void ParseFrameModuleFaultInfo(unsigned char * pData, QList<CanMapNode> &ToCenterList);

    //解析负荷调度上传相关状态----0x76(旧协议)
    void ParseFrameLoadDispatch(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析BMS信息----握手阶段1
    void ParseFrameBMSHandShake1(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析BMS信息----握手阶段2
    void ParseFrameBMSHandShake2(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析BMS信息----握手阶段3
    void ParseFrameBMSHandShake3(FrameLongPackage * pLongPackage, QList<CanMapNode> &ToCenterList);

    //解析BMS信息----BMS参数配置阶段1
    void ParseFrameBMSParamSet1(FrameLongPackage * pLongPackage, QList<CanMapNode> &ToCenterList);
    //解析BMS信息----BMS参数配置阶段2
    void ParseFrameBMSParamSet2(unsigned char * pData, QList<CanMapNode> &ToCenterList);

    //解析BMS信息----BMS充电阶段1
    void ParseFrameBMSCharging1(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析BMS信息----BMS充电阶段2
    void ParseFrameBMSCharging2(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析BMS信息----BMS充电阶段3
    void ParseFrameBMSCharging3(unsigned char * pData, QList<CanMapNode> &ToCenterList);

    //解析BMS信息----BMS充电结束阶段1
    void ParseFrameBMSChargeEnd1(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析BMS信息----BMS充电结束阶段2
    void ParseFrameBMSChargeEnd2(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析BMS信息----BMS充电结束阶段3
    void ParseFrameBMSChargeEnd3(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析BMS信息----BMS充电结束阶段4
    void ParseFrameBMSChargeEnd4(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析BMS信息----BMS充电结束阶段5
    void ParseFrameBMSChargeEnd5(unsigned char * pData, QList<CanMapNode> &ToCenterList);

    //解析直流模块信息1
    void ParseFrameModuleInfo1(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析直流模块信息2
    void ParseFrameModuleInfo2(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析直流模块信息3
    void ParseFrameModuleInfo3(unsigned char * pData, QList<CanMapNode> &ToCenterList);

    //解析PDU信息1
    void ParseFramePDUInfo1(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析PDU信息2
    void ParseFramePDUInfo2(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析PDU信息3
    void ParseFramePDUInfo3(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析PDU信息4
    void ParseFramePDUInfo4(unsigned char * pData, QList<CanMapNode> &ToCenterList);

    //解析分支箱信息1
    void ParseFrameBOXInfo1(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析分支箱信息1
    void ParseFrameBOXInfo2(unsigned char * pData, QList<CanMapNode> &ToCenterList);

    //解析CCU信息2
    void ParseFrameCCUInfo1(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析CCU信息2
    void ParseFrameCCUInfo2(unsigned char * pData, QList<CanMapNode> &ToCenterList);

     //解析VIN 0x74
    void ParseFrameVIN(FrameLongPackage * pLongPackage, QList<CanMapNode> &ToCenterList);
    //解析故障点记录 0x78
    void ParseAlarmPointRecord(FrameLongPackage * pLongPackage, QList<CanMapNode> &ToCenterList);
    //解析故障录波数据 0x79
    void ParseFaultRecord(FrameLongPackage * pLongPackage, QList<CanMapNode> &ToCenterList);
    //解析终端主动防护参数 0xB0
    void ParseTermActiveArg(FrameLongPackage * pLongPackage, QList<CanMapNode> &ToCenterList);
    //解析终端柔性充电参数 0xB1
    void ParseTermFlexArg(FrameLongPackage * pLongPackage, QList<CanMapNode> &ToCenterList);
    //解析通用静态参数设置 0xB2
    void ParseTermGeneralStaticArg(unsigned char * pData, QList<CanMapNode>  &ToCenterList);
    //解析通用动态参数设置 0xB3
    void ParseTermGeneralDynamicArg(unsigned char * pData, QList<CanMapNode>  &ToCenterList);
    //解析CCU参数设置 0xB5(CAN)
    void ParseCCUQueryArgResult(unsigned char * pData, QList<CanMapNode>  &ToCenterList);
    //解析长帧
    virtual void ParseLongFrame(FrameLongPackage * pLongPackage);
    //解析CCU数量
    void ParseCCUNum(unsigned char	ucSa);

    //生成多枪分组下发信息
    void MakeFrameChargeGunGroup(InfoMap &CenterMap,  unsigned char ucCanID);
    //生成直流双枪充电工作模式下发信息
    void MakeFrameChargeGunWorkMode(InfoMap &CenterMap, unsigned char ucCanID);

private:
    //广播召唤设备状态计数器
    unsigned int uiBroadCastCount;
    //CCU数量
    unsigned char ucCCUNum[10];
};

#endif // DCCANPROTOCOL_H
