#ifndef ENERGYPLANPROTOCOL_H
#define ENERGYPALNPROTOCOL_H
#include "J1939GeneralProtocol.h"
#include <QDateTime>
#include <QFile>

typedef enum _ProVerEnum_Energy_Plan
{
    ProVer_Old_Energy_Plan = 0, //1.1.0之前版本
    ProVer_1_0_0_Energy_Plan = 1 //1.1.0版本

}ProVerEnum_Energy_Plan;

//能效计划PGN枚举
typedef enum _EPPGNType
{
    PF_ModuleFaultInfo_EP = 0x77,
    //能效计划添加
    PF_FourQuadrantCabinet1 = 0xC0,
    PF_ACDCInverterCabinet = 0xC1,
    PF_EnergyStorageCabinetInfo1 = 0xC2,
    PF_EnergyStorageCabinetInfo2 = 0xC3,
    PF_EnergyStorageCabinetCmdSet = 0xC4,
    PF_PhotoVoltaicCabinetInfo1 = 0xC5,
    PF_PhotoVoltaicCabinetInfo2 = 0xC6,
    PF_ChargeDischargeCabinetInfo1 = 0xC7,
    PF_ChargeDischargeCabinetInfo2 = 0xC8,
    PF_SysControlCabinetInfo1 = 0xC9,
    PF_SysControlCabinetInfo2 = 0xCA,
    PF_ElectircCloset1 = 0xCB,

    PF_CtrlModule = 0xCD,
    PF_SetModulePower = 0xCE,

    PF_ACDCInfo1 = 0xCF,
    PF_ACDCInfo2 = 0xD0,
    PF_DCDCInfo1 = 0xD6,
    PF_DCDCInfo2 = 0xD7,
    PF_BothwayPDUInfo1 = 0xD8,
    PF_BothwayPDUInfo2 = 0xD9,
    PF_EnergyStorageCabinetBatteryInfo1 = 0xDA,
    PF_EnergyStorageCabinetBatteryInfo2 = 0xDB,
    PF_PowerOptimizerCabinetInfo1 = 0xDC,
    PF_PowerOptimizerCabinetInfo2 = 0xDD,
    PF_ModuleRePowerControl = 0xDE,
    PF_HygrothermographInfo1 = 0xDF,
    PF_HygrothermographInfo2 = 0xE0,
    PF_EMSSwitchPoint = 0xE2,
    PF_EMSHumityTemp1 = 0xE3,
    PF_EMSHumityTemp2 = 0xE4
}EPPGNType;

//直流短帧长度枚举
typedef enum _EPShortFrameLength
{


}_EPShortFrameLength;

//直流长帧长度枚举
typedef enum _EPLongFrameLength
{

}EPLongFrameLength;

//上传模块故障告警信息
typedef struct _FrameModuleFaultInfo_EP
{
    unsigned char ucReserved1;
    unsigned char ucModuleID;
    unsigned char ucAlarmCode;
    unsigned char ucMinPDUID;
    unsigned char ucMaxPDUID;
    unsigned char ucFaultState;
    unsigned char ucFaultRecordFlag;
    unsigned char ucReserved2;
}__attribute__ ((packed)) FrameModuleFaultInfo_EP;

//四象限柜子信息
//typedef struct _FrameFourQuadrantCabinetInfo1
//{
//    struct
//    {
//        unsigned char ACbreaker1		: 1;//0断开/1吸合
//        unsigned char ACbreaker2		: 1;//0断开/1吸合
//        unsigned char ACbreaker3		: 1;//0断开/1吸合
//        unsigned char DCbreaker1		: 1;//0断开/1吸合
//        unsigned char DCbreaker2		: 1;//0断开/1吸合
//        unsigned char DCbreaker3		: 1;//0断开/1吸合
//        unsigned char surgeFeedback		: 1;//浪涌反馈0异常/1正常
//        unsigned char fireExtinguisher		: 1;//灭火器0异常/1正常
//    }stStatus;
//    unsigned short reserve;//暂不解析，协议中有７个字节
//}__attribute__ ((packed)) FrameFourQuadrantCabinetInfo1;

typedef struct _FrameEMSSwitchPointInfo
{
    struct
    {
        unsigned char smokeSensor_lowVolIn		: 1;//低压进出线柜烟感　0故障/1正常
        unsigned char frameFeedback		: 1;//框架反馈　0断开/1吸合
        unsigned char minorLoadbreaker_630A1		: 1;//次要负荷断路器630A1反馈　0断开/1吸合
        unsigned char minorLoadbreaker_630A2		: 1;//次要负荷断路器630A2反馈　0断开/1吸合
        unsigned char smokeSensor_lowVolOut		: 1;//低压出线柜烟感　0故障/1正常
        unsigned char minorLoadbreaker_400A1		: 1;//次要负荷断路器400A1反馈　0断开/1闭合
        unsigned char minorLoadbreaker_400A2		: 1;//次要负荷断路器400A2反馈　0断开/1闭合
        unsigned char minorLoadbreaker_400A3		: 1;//次要负荷断路器400A3反馈　0断开/1闭合
        unsigned char acdcBreaker		: 1;//400KW ACDC断路器状态反馈　0断开/1闭合
        unsigned char importBreaker1		: 1;//重要负荷母线处断路器　0断开/1闭合
        unsigned char importBreaker2		: 1;//重要负荷断路器反馈　0断开/1闭合
        unsigned char emergncyStop		: 1;//急停反馈　0正常/1故障
        unsigned char reserve1		: 4;//

        unsigned char mainModuleSwitchCMD     :1;//并网400KW主模块的平滑切换命令 1:有效　0:无效
        unsigned char mainLoad_ACDCModuleSwitchCMD     :1;//主要负荷AC/DC模块发的平滑切换命令 1:有效　0:无效
        unsigned char storageUnit1_DCDCModuleSwitchCMD     :1;//储能单元第1组DC/DC模块发送的平滑切换命令 1:有效　0:无效
        unsigned char storageUnit2_DCDCModuleSwitchCMD     :1;//储能单元第2组DC/DC模块发送的平滑切换命令 1:有效　0:无效
        unsigned char gird_state : 1;   //电网状态, 1,离网, 0,并网
        unsigned char reserve2		: 3;//
    }__attribute__ ((packed))stStatus;
    unsigned char reserve;//暂不解析，协议中无定义
}__attribute__ ((packed)) FrameEMSSwitchPointInfo;

typedef struct _FrameFourQuadrantCabinetInfo1
{
    struct
    {
        unsigned char ACbreaker_Cabinet2		: 1;//0断开/1吸合
        unsigned char DCbreaker_Cabinet2		: 1;//0断开/1吸合
        unsigned char ACbreaker_Cabinet3		: 1;//0断开/1吸合
        unsigned char DCbreaker_Cabinet3		: 1;//0断开/1吸合
        unsigned char reserve		: 1;//0断开/1吸合
        unsigned char surgeFeedback_Cabinet2		: 1;//浪涌反馈0异常/1正常
        unsigned char fireExtinguisher_Cabinet2		: 1;//灭火器0异常/1正常
        unsigned char fireExtinguisher_Cabinet3		: 1;//灭火器0异常/1正常
    }stStatus;
    unsigned short reserve;//暂不解析，协议中无定义
}__attribute__ ((packed)) FrameFourQuadrantCabinetInfo1;
//独立逆变柜
typedef struct _FrameACDCInverterCabinetInfo
{
    struct
    {
        unsigned char ACbreaker		: 1;//0断开/1吸合
        unsigned char DCbreaker		: 1;//0断开/1吸合
        unsigned char reserve		: 4;//0断开/1吸合
        unsigned char surgeFeedback_Cabinet2		: 1;//浪涌反馈0异常/1正常
        unsigned char fireExtinguisher_Cabinet3		: 1;//灭火器0异常/1正常
    }stStatus;
    unsigned short reserve;//暂不解析，协议中无定义
}__attribute__ ((packed)) FrameACDCInverterCabinetInfo;

//上传交流电表信息
typedef struct _FrameACAmmeterInfo
{
    //    unsigned char ucDataType;
    //    unsigned char ucDataLength;
    //    char        chData[17];
    char ammeterID;
    short vol_U;
    short cur_U;
    short vol_V;
    short cur_V;
    short vol_W;
    short cur_W;
    short feq;

    short positiveActivePower_A;
    short positiveActivePower_B;
    short positiveActivePower_C;
    short positiveActivePower;
    short reverseActivePower_A;
    short reverseActivePower_B;
    short reverseActivePower_C;
    short reverseActivePower;
    short positiveReActivePower_A;
    short positiveReActivePower_B;
    short positiveReActivePower_C;
    short positiveReActivePower;
    short reverseReActivePower_A;
    short reverseReActivePower_B;
    short reverseReActivePower_C;
    short reverseReActivePower;
    short apparentPower_A;
    short apparentPower_B;
    short apparentPower_C;
    short apparentPower;

    short powerFactor_A;
    short powerFactor_B;
    short powerFactor_C;
    short powerFactor;

    int positiveActiveEnergy;
    int positiveReActiveEnergy;
    int reverseActiveEnergy;
    int reverseReActiveEnergy;
}__attribute__ ((packed)) FrameACAmmeterInfo;

//上传交流电表信息
typedef struct _FrameDCAmmeterInfo
{
    char ammeterID;
    short vol_U;
    short cur_U;

    short positiveActivePower;
    int positiveActiveEnergy;
    int reverseActiveEnergy;
}__attribute__ ((packed)) FrameDCAmmeterInfo;

//四象限柜子信息
typedef struct _FrameFourQuadrantCabinet
{
    struct
    {
        unsigned char ACbreaker1		: 1;//0断开/1吸合
        unsigned char ACbreaker2		: 1;//0断开/1吸合
        unsigned char ACbreaker3		: 1;//0断开/1吸合
        unsigned char DCbreaker1		: 1;//0断开/1吸合
        unsigned char DCbreaker2		: 1;//0断开/1吸合
        unsigned char DCbreaker3		: 1;//0断开/1吸合
        unsigned char surgeFeedback		: 1;//浪涌反馈0异常/1正常
        unsigned char fireExtinguisher		: 1;//灭火器0异常/1正常
    }stStatus;
    unsigned short envTemperature;//温湿度
    unsigned short envHumidity;
    unsigned short reserve;
}__attribute__ ((packed)) FrameFourQuadrantCabinet;

//
typedef struct _FrameDCACInfo
{
    unsigned char moduleID;
    unsigned short vol_U;
    unsigned short cur_U;
    unsigned short vol_V;
    unsigned short cur_V;
    unsigned short vol_W;
    unsigned short cur_W;
    unsigned short frequency;

    unsigned short sysActivePower;//系统有功功率
    unsigned short sysReActivePower;//系统无功功率
    unsigned short sysApparentPower;//系统视在功率
    unsigned short PF;
    unsigned short DCpositiveCur;//直流正电流
    unsigned short DCnegativeCur;//直流负电流
    unsigned short DCpositiveBusBarVol;//直流正母线电压
    unsigned short DCnegativeBusBarVol;//直流负母线电压
    unsigned short DCbilateralBusBarVol;//直流双边母线电压
    unsigned short DCpower;//直流功率
    unsigned short devStatus;//设备状态
    unsigned short warningStatus;//告警状态
    unsigned short faultStatus;//故障状态

}__attribute__ ((packed)) FrameDCACInfo;

//DCAC交流侧信息１
typedef struct _FrameDCACACInfo1
{
    unsigned short current_U;
    unsigned short current_V;
    unsigned short current_W;
    unsigned short voltage_U;
}__attribute__ ((packed)) FrameDCACACInfo1;
//DCAC交流侧信息２
typedef struct _FrameDCACACInfo2
{
    unsigned short voltage_V;
    unsigned short voltage_W;
    unsigned short activePower;
    unsigned short reserve;
}__attribute__ ((packed)) FrameDCACACInfo2;
//DCAC功率策略设置
typedef struct _FrameDCACPowerSchemeSet
{
    unsigned char cmd;//0x55： 开机   0xAA：关机
    unsigned short reserve1;
    unsigned short reserve2;
    unsigned short reserve3;
}__attribute__ ((packed)) FrameDCACPowerSchemeSet;
//储能柜信息
//typedef struct _FrameEnergyStorageCabinetInfo
//{
//    struct
//    {
//        unsigned char DCbreaker		: 1;//0断开/1半连接/2连接
//        unsigned char fireExtinguisher		: 1;//0待机状态/1充电状态/2故障状态/3启动中
//        unsigned char chargeDischargeMode		: 2;//0待机/1充电/2放电
//        unsigned char reserve	: 4;//1暂停/2限制
//    }stStatus;
//    //unsigned short envTemperature;
//    //unsigned short envHumidity;
//    //unsigned short reserve;
//}__attribute__ ((packed)) FrameEnergyStorageCabinetInfo;
typedef struct _FrameEnergyStorageCabinetInfo
{
    union{
        struct
        {
            unsigned char mainBreaker		: 1;//1闭合 0断开
            unsigned char slaveBreaker		: 1;//1闭合 0断开
            unsigned char DCBreaker1		: 1;//1闭合 0断开
            unsigned char DCBreaker2		: 1;//1闭合 0断开
            unsigned char reserve	: 3;//1暂停/2限制
            unsigned char fireExtinguisher		: 1;//1正常 0异常
        }__attribute__ ((packed))stStatus;

        struct
        {
            unsigned char DCBreaker3		: 1;//1闭合 0断开
            unsigned char DCBreaker4		: 1;//1闭合 0断开
            unsigned char reserve	: 1;//1暂停/2限制
            unsigned char tripFeedback :1;
            unsigned char fireExtinguisher		: 1;//1正常 0异常
            unsigned char waterIn		: 1;//1正常 0异常
            unsigned char reserve1	: 2;//1暂停/2限制
        }__attribute__ ((packed))stStatus2;
    }state;
}__attribute__ ((packed)) FrameEnergyStorageCabinetInfo;
//DC/DC信息
typedef struct _FrameChargeDischargeDCDCInfo
{
    unsigned char moduleID;
    unsigned char boardID;
    struct
    {
        unsigned char status		: 4;//模块工作状态
        unsigned char reserve  : 4;//预留
    }workMode;
    struct
    {
        unsigned char alarm0		: 1;//模块保护(2)
        unsigned char alarm1		: 1;//模块故障(1)
        unsigned char alarm2		: 1;//过温
        unsigned char alarm3		: 1;//输出过压
        unsigned char alarm4		: 1;//温度限功率状态
        unsigned char alarm5		: 1;//交流限功率状态
        unsigned char alarm6		: 1;//模块EEPROM故障
        unsigned char alarm7		: 1;//风扇故障
        unsigned char alarm8		: 1;//模块WALK-In功能使能
        unsigned char alarm9		: 1;//风扇全速
        unsigned char alarm10		: 1;//模块开关机
        unsigned char alarm11		: 1;//模块限功率
        unsigned char alarm12		: 1;//模块CAN错误状态
        unsigned char alarm13		: 1;//模块电流均流告警
        unsigned char alarm14		: 1;//模块识别
        unsigned char alarm15		: 1;//过压脱离继电器动作
        unsigned char alarm16		: 1;//模块交流缺相告警
        unsigned char alarm17		: 1;//模块交流不平衡告警
        unsigned char alarm18		: 1;//模块交流欠压告警
        unsigned char alarm19		: 1;//模块顺序起机功能使能
        unsigned char alarm20		: 1;//模块PFC保护
        unsigned char alarm21		: 1;//模块交流过压
        unsigned char alarm22		: 1;//模块ID重复
        unsigned char alarm23		: 1;//模块严重不均流
        unsigned char alarm24		: 1;//模块输出欠压告警(仅TEC模块)
        unsigned char alarm25		: 1;//模块重载告警
        unsigned char alarm26		: 1;//模块插拔故障
        unsigned char alarm27		: 1;//模块轻微不均流
        unsigned char alarm28		: 1;//模块PDU分组完成标志
        unsigned char alarm29		: 1;//DC模块地址识别标志
        unsigned char alarm30		: 1;//模块启动完成
        unsigned char alarm31		: 1;//模块内部通信异常告警
    } moduleStatus;
    unsigned char reserve;
    short outVol;
    short outCur;
    short inVol;
    short inCur;
    short boardTemp_M1;
    short envTemp;
    unsigned short runTime;
    unsigned short chargeDisChargeTimes;
}__attribute__ ((packed)) FrameChargeDischargeDCDCInfo;
//温湿度传感器信息１
typedef struct _FrameHumitureInfo1
{
    unsigned short addr;
    short temperature;
    short humidity;
    unsigned short reserve;

}__attribute__ ((packed)) FrameHumitureInfo1;
//EMS温湿度信息１
typedef struct _FrameEMSHumitureInfo1
{
    unsigned short tempHighVol;
    unsigned short humidityHighVol;
    unsigned short tempLowVol;
    unsigned short humidityLowVol;
}__attribute__ ((packed)) FrameEMSHumitureInfo1;
//EMS温湿度信息2
typedef struct _FrameEMSHumitureInfo2
{
    unsigned short tempLowVol;
    unsigned short humidityLowVol;
}__attribute__ ((packed)) FrameEMSHumitureInfo2;
//储能柜电池信息
typedef struct _FrameEnergyStorageCabinetBatteryInfo
{
    struct
    {
        //byte0
        unsigned char CRC;//
        //byte1
        unsigned char bmsHeartBeat		: 4;//0待机状态/1充电状态/2故障状态/3启动中
        unsigned char test		: 1;//0断开/1闭合
        unsigned char tankSwitch       :1;
        unsigned char reserve1	: 2;
        //byte2
        unsigned char singleOverVolAlarm    :1;
        unsigned char singleLowVolAlarm  :1;
        unsigned char OverTempAlarm  :1;
        unsigned char BelowTempAlarm : 1;
        unsigned char insulationAlarm  :1;
        unsigned char BMScommuFault  :1;
        unsigned char reserve2	: 2;
        //byte3
        unsigned char reserve3 ;
        //byte4
        unsigned char BMScontrolPower	: 1;
        unsigned char BMSfullPowerON	: 1;
        unsigned char BMSsysStatus	: 3;
        unsigned char reserve4	: 3;
        //byte5
        unsigned char ESSfullEnergy	: 1;
        unsigned char ESSfullDisCharge	: 1;
        unsigned char ApplyACInfo : 1;
        unsigned char ApplySysInfo : 1;
        unsigned char reserve5	: 4;
        //byte6-7
        unsigned short SOC;
    }__attribute__ ((packed)) B2C_STATUS;
    struct
    {
        unsigned char CRC;//
        unsigned char tankNum;
        unsigned short BMShighVol;
        short BMScur;
        short reserve;
    }__attribute__ ((packed))B2C_SUMDATA1;
    struct
    {
        unsigned char CRC;//
        unsigned char reserve;
        unsigned short BMSchargeEnergy;
        unsigned short BMSdisChargeEnergy;
        unsigned char  SOH;
        unsigned char  reverse;
    }__attribute__ ((packed))B2C_SUMDATA2;
    struct
    {
        unsigned char sysHumidity;//
        unsigned short singleMaxVol;
        unsigned short singleMinVol;
        char singleMaxTem;
        char singleMinTem;
        char  sysTemp;
    }__attribute__ ((packed))B2C_SUMDATA3;
    struct
    {
        short BMSlimitDischargeCur;//
        short BMSlimitChargeCur;
        unsigned short BMSlimitChargeVol;
        unsigned short BMSlimitDisChargeVol;
    }__attribute__ ((packed))B2C_LIMIT;

    unsigned short outVol;
    unsigned short fuseVol;
    unsigned short breakerVol;
    short cur;
    unsigned short dcVol;
    short dcCur;
    short dcPower;
    unsigned int dcPositiveEnergy;
    unsigned int dcDisPositiveEnergy;
    unsigned short dcPT;
    unsigned short dcCT;

}__attribute__ ((packed)) FrameEnergyStorageCabinetBatteryInfo;
//储能柜电表信息
//typedef struct _FrameEnergyStorageCabinetAmmeterInfo1
//{
//    unsigned short Volage;
//    unsigned short Current;
//    unsigned int reserve;
//}__attribute__ ((packed)) FrameEnergyStorageCabinetAmmeterInfo1;
//储能柜电表信息
//typedef struct _FrameEnergyStorageCabinetAmmeterInfo2
//{
//    unsigned int positiveActivePower;
//    unsigned int reverseActivePower;
//}__attribute__ ((packed)) FrameEnergyStorageCabinetAmmeterInfo2;
//储能柜命令设置
typedef struct _FrameEnergyStorageCabinetCMDSet
{

}__attribute__ ((packed)) FrameEnergyStorageCabinetCMDSet;
//充放电柜
typedef struct _FrameChargeDischargeCabinetInfo1
{
    struct
    {
        unsigned char DCDC1breaker		: 1;//0断开１闭合
        unsigned char DCDC1fireExtinguisher		: 1;//
        unsigned char reserve	: 4;//1暂停/2限制
        unsigned char DCDC2breaker		: 1;//0断开１闭合
        unsigned char DCDC2fireExtinguisher		: 1;//
    }stStatus;
    unsigned char reserve1;
    unsigned short DCDCsumVol;
    unsigned short DCDCsumCur;
    unsigned short reserve2;
}__attribute__ ((packed)) FrameChargeDischargeCabinetInfo1;

//充放电柜2
typedef struct _FrameChargeDischargeCabinetInfo2
{
    unsigned int DCDCpositiveActivePower;
    unsigned int DCDCreverseActivePower;
}__attribute__ ((packed)) FrameChargeDischargeCabinetInfo2;

//系统控制柜信息１
typedef struct _FrameSysControlCabinetInfo1
{
    struct
    {
        unsigned char lowVolTravelSwitch		: 1;//0关门/  １开门
        unsigned char highVolTravelSwitch		: 1;//0关门/  １开门
        unsigned char dormTravelSwitch		: 1;//0关门/  １开门
        unsigned char lowVolSmokeSensor		: 1;//0关门/  １开门
        unsigned char transformerSmokeSensor :1;//0关门/  １开门
        unsigned char highVolSmokeSensor		: 1;//0关门/  １开门
        unsigned char outEmergencyStop     :1;//0关门/  １开门
        unsigned char centerContorlEmergencyStop        :1;//0关门/  １开门总控柜面板急停
    }stStatus;

}__attribute__ ((packed)) FrameSysControlCabinetInfo1;
//系统控制柜信息2
typedef struct _FrameSysControlCabinetInfo2
{
    struct
    {
        unsigned char transformerOverTemp		: 1;//0关门/  １开门
        unsigned char transformerTempControlerFault		: 1;//0关门/  １开门
        unsigned char reserve                  : 5;//预留
        unsigned char waterIn  :1;
    }stStatus;
}__attribute__ ((packed)) FrameSysControlCabinetInfo2;
//总配电柜信息1
typedef struct _FrameDistributionCabinetInfo1
{
    struct
    {
        unsigned char sumBreaker		: 1;//0断开/1闭合
        unsigned char loadBreaker1		: 1;//0正常1异常
        unsigned char loadBreaker2		: 1;//0正常1异常
        unsigned char loadBreaker3		: 1;//0正常1异常
        unsigned char loadBreaker4		: 1;//0正常1异常
        unsigned char acBreaker		: 1;//0正常1异常
        unsigned char fireExtinguisher		: 1;//0正常1异常
        unsigned char reserve		: 1;//0断开/1闭合
    }stStatus;
}__attribute__ ((packed)) FrameDistributionCabinetInfo1;
//
typedef struct _FrameEnergyStorageDCDCInfo
{
    unsigned short moduleID;
    short cur_in;
    short cur_out;
    unsigned short vol_in;
    unsigned short vol_out;
    unsigned short vol_battery;
    short power_dc;
    struct
    {
        unsigned char warning		: 1;//0断开/1闭合
        unsigned char run		: 1;//0断开/1闭合
        unsigned char fault		: 1;//0断开/1闭合
        unsigned char offLine		: 1;//0断开/1闭合
        unsigned char reserve  :4;
        unsigned char reserve1;
    }__attribute__ ((packed))devStatus;
    struct
    {
        unsigned char fun1		: 1;//0断开/1闭合
        unsigned char fun2		: 1;//0断开/1闭合
        unsigned char fun3		: 1;//0断开/1闭合
        unsigned char reserve  : 5;
        unsigned char reserve1;
    }__attribute__ ((packed))warningStatus;
    struct
    {
        unsigned char fault0		: 1;//0断开/1闭合
        unsigned char fault1		: 1;//0断开/1闭合
        unsigned char fault2		: 1;//0断开/1闭合
        unsigned char fault3     : 1;
        unsigned char fault4		: 1;//0断开/1闭合
        unsigned char fault5		: 1;//0断开/1闭合
        unsigned char fault6		: 1;//0断开/1闭合
        unsigned char fault7     : 1;
        unsigned char fault8		: 1;//0断开/1闭合
        unsigned char fault9		: 1;//0断开/1闭合
        unsigned char fault10		: 1;//0断开/1闭合
        unsigned char fault11     : 1;
        unsigned char fault12		: 1;//0断开/1闭合
        unsigned char fault13     : 1;
        unsigned char fault14		: 1;//0断开/1闭合
        unsigned char fault15		: 1;//0断开/1闭合
    }__attribute__ ((packed))faultStatus;

    unsigned char ucReserved1[6];

    unsigned short HWVersion_high;
    unsigned short HWVersion_low;
    unsigned short SWVersion_high;
    unsigned short SWVersion_low;
    unsigned char reserve[28];
    unsigned short tmp_IGBT1;
    unsigned short tmp_IGBT2;
    unsigned short tmp_IGBT3;
    unsigned short tmp_IGBT4;
    unsigned short tmp_IGBT5;
    unsigned short tmp_IGBT6;
    unsigned short tmp_IN;
    unsigned short tmp_OUT;

}__attribute__ ((packed)) FrameEnergyStorageDCDCInfo;

typedef struct _FrameACDCInfo
{
    unsigned short moduleID;
    unsigned short vol_U;
    short cur_U;
    unsigned short vol_V;
    short cur_V;
    unsigned short vol_W;
    short cur_W;
    unsigned short frequency;

    short sysActivePower;//系统有功功率
    short sysReActivePower;//系统无功功率
    unsigned short sysApparentPower;//系统视在功率
    short PF;
    unsigned short reserve;
    short DCpositiveCur;//直流正电流
    short DCnegativeCur;//直流负电流
    unsigned short DCpositiveBusBarVol;//直流正母线电压
    unsigned short DCnegativeBusBarVol;//直流负母线电压
    unsigned short DCbilateralBusBarVol;//直流双边母线电压
    short DCpower;//直流功率
    unsigned short devStatus;//设备状态
    unsigned short warningStatus;//告警状态
    unsigned short faultStatus;//故障状态

    unsigned char reserve1[6];

    unsigned short HWVersion_high;
    unsigned short HWVersion_low;
    unsigned short SWVersion_high;
    unsigned short SWVersion_low;

    unsigned char reserve2[28];

    short tmp_IGBT1;
    short tmp_IGBT2;
    short tmp_IGBT3;
    short tmp_IGBT4;
    short tmp_IGBT5;
    short tmp_IGBT6;
    short tmp_IN;
    short tmp_OUT;

    unsigned char reserve3[8];

    short inductance1_cur;
    short inductance2_cur;
    short inductance3_cur;
    short inductance4_cur;
    short inductance5_cur;
    short inductance6_cur;

}__attribute__ ((packed)) FrameACDCInfo;

//进线总配电柜信息1
typedef struct _FrameInlineDistributionCabinetInfo1
{
    struct
    {
        unsigned char sumBreaker		: 1;//0断开/1闭合
        unsigned char loadBreaker1		: 1;//0正常1异常
        unsigned char loadBreaker2		: 1;//0正常1异常
        unsigned char loadBreaker3		: 1;//0正常1异常
        unsigned char loadBreaker4		: 1;//0正常1异常
        unsigned char acBreaker		: 1;//0正常1异常
        unsigned char inductive		: 1;//0正常1异常
        unsigned char reserve		: 1;//0断开/1闭合
        unsigned char reserve1;//
    }stStatus;

    unsigned short sumBreakerVol;//总架断路器端电压
    unsigned short sumBreakerCur;//总架断路器端电流
    unsigned short reserve2;
}__attribute__ ((packed)) FrameInlineDistributionCabinetInfo1;
//进线总配电柜信息2
typedef struct _FrameInlineDistributionCabinetInfo2
{
    unsigned int sumBreakerpositiveActivePower;//总架断路器端正向总电能
    unsigned int sumBreakerreverseActivePower;//总架断路器端负向总电能
}__attribute__ ((packed)) FrameInlineDistributionCabinetInfo2;
//进线总配电柜信息3
typedef struct _FrameInlineDistributionCabinetInfo3
{
    unsigned int loadBreaker1Vol;//负荷断路器1端电压
    unsigned int loadBreaker1Cur;//负荷断路器1端电流
    unsigned int reserve;
}__attribute__ ((packed)) FrameInlineDistributionCabinetInfo3;

//进线总配电柜信息4
typedef struct _FrameInlineDistributionCabinetInfo4
{
    unsigned int loadBreaker1positiveActivePower;//负荷断路器1端正向总电能
    unsigned int loadBreaker1reverseActivePower;//负荷断路器1端负向总电能
}__attribute__ ((packed)) FrameInlineDistributionCabinetInfo4;
//进线总配电柜信息5
typedef struct _FrameInlineDistributionCabinetInfo5
{
    unsigned int loadBreaker2Vol;//负荷断路器1端电压
    unsigned int loadBreaker2Cur;//负荷断路器1端电流
    unsigned int reserve;
}__attribute__ ((packed)) FrameInlineDistributionCabinetInfo5;

//进线总配电柜信息6
typedef struct _FrameInlineDistributionCabinetInfo6
{
    unsigned int loadBreaker2positiveActivePower;//负荷断路器1端正向总电能
    unsigned int loadBreaker2reverseActivePower;//负荷断路器1端负向总电能
}__attribute__ ((packed)) FrameInlineDistributionCabinetInfo6;
//进线总配电柜信息7
typedef struct _FrameInlineDistributionCabinetInfo7
{
    unsigned int loadBreaker3Vol;//负荷断路器1端电压
    unsigned int loadBreaker3Cur;//负荷断路器1端电流
    unsigned int reserve;
}__attribute__ ((packed)) FrameInlineDistributionCabinetInfo7;

//进线总配电柜信息8
typedef struct _FrameInlineDistributionCabinetInfo8
{
    unsigned int loadBreaker3positiveActivePower;//负荷断路器1端正向总电能
    unsigned int loadBreaker3reverseActivePower;//负荷断路器1端负向总电能
}__attribute__ ((packed)) FrameInlineDistributionCabinetInfo8;
//进线总配电柜信息9
typedef struct _FrameInlineDistributionCabinetInfo9
{
    unsigned int loadBreaker4Vol;//负荷断路器1端电压
    unsigned int loadBreaker4Cur;//负荷断路器1端电流
    unsigned int reserve;
}__attribute__ ((packed)) FrameInlineDistributionCabinetInfo9;

//进线总配电柜信息10
typedef struct _FrameInlineDistributionCabinetInfo10
{
    unsigned int loadBreaker4positiveActivePower;//负荷断路器1端正向总电能
    unsigned int loadBreaker4reverseActivePower;//负荷断路器1端负向总电能
}__attribute__ ((packed)) FrameInlineDistributionCabinetInfo10;
//进线总配电柜信息11
typedef struct _FrameInlineDistributionCabinetInfo11
{
    unsigned short ACBreaker1Vol;//负荷断路器1端电压
    unsigned short ACBreaker1Cur;//负荷断路器1端电流
    unsigned int reserve;
}__attribute__ ((packed)) FrameInlineDistributionCabinetInfo11;

//进线总配电柜信息12
typedef struct _FrameInlineDistributionCabinetInfo12
{
    unsigned int ACBreaker1positiveActivePower;//负荷断路器1端正向总电能
    unsigned int ACBreaker1reverseActivePower;//负荷断路器1端负向总电能
}__attribute__ ((packed)) FrameInlineDistributionCabinetInfo12;

typedef struct _FrameEnergyStorageCabinetInfo1
{
    struct
    {
        unsigned char DCBreaker		: 1;//0断开/1闭合
        unsigned char fireStop		: 1;//0正常1异常
        unsigned char runStatus		: 2;//0正常1异常
    }stStatus;
}__attribute__ ((packed)) FrameEnergyStorageCabinetInfo1;
//光伏柜信息1
typedef struct _FramePhotoVoltaicCabinetInfo1
{
    struct
    {
        unsigned char breaker1		: 1;//0断开1闭合
        unsigned char breaker2		: 1;//0断开1闭合
        unsigned char breaker3		: 1;//0断开1闭合
        unsigned char breaker4		: 1;//0断开1闭合
        unsigned char breaker5		: 1;//0断开1闭合
        unsigned char breaker6		: 1;//0断开1闭合
        unsigned char breaker7		: 1;//0断开1闭合
        unsigned char smokeSensor		: 1;//0异常/1正常
    }stStatus;
    unsigned char ucReserved[7];    //7字节预留
}__attribute__ ((packed)) FramePhotoVoltaicCabinetInfo1;

//功率优化器柜命令设置
typedef struct _FramePowerOptimizerCabinetCMDSet
{
    unsigned short busVol;//母线电压
    unsigned short powerModuleTem;//功率模块温度
    unsigned short DCcur;//直流侧电流
    unsigned short DCvol;//直流侧电压
    unsigned short DCpower;//直流侧功率
    unsigned short vol_AB;//电网AB线电压有效值
    unsigned short vol_BC;//电网BC线电压有效值
    unsigned short vol_CA;//电网CA线电压有效值
    unsigned short cur_A;//电网A相电流有效值
    unsigned short cur_B;//电网B相电流有效值
    unsigned short cur_C;//电网C相电流有效值
    unsigned short apparentPower;//电网视在功率
    unsigned short activePower;//电网有功功率
    unsigned short reactivePower;//电网无功功率
    unsigned short freq;//电网频率
    unsigned short sysStatus;//系统状态机
    unsigned short sysRunFlag;//系统运行标志
    struct
    {
        unsigned char bit0		: 1;//IGBT过流
        unsigned char bit1		: 1;//驱动板连接错误
        unsigned char bit2		: 1;//母线硬件过压
        unsigned char bit3		: 1;//辅助电源故障
        unsigned char bit4		: 1;//硬件过流
        unsigned char bit5		: 1;//保留
        unsigned char bit6		: 1;//保留
        unsigned char bit7		: 1;//总故障
        unsigned char bit8		: 1;//保留
        unsigned char bit9		: 1;//保留
        unsigned char bit10		: 1;//保留
        unsigned char bit11		: 1;//保留
        unsigned char bit12		: 1;//保留
        unsigned char bit13		: 1;//保留
        unsigned char bit14		: 1;//保留
        unsigned char bit15		: 1;//保留
    }HWfault;
    struct
    {
        unsigned char bit0		: 1;//RAM自检失败
        unsigned char bit1		: 1;//EEPROM参数回默认值
        unsigned char bit2		: 1;//CPLD版本不匹配
        unsigned char bit3		: 1;//AD采样零漂过大
        unsigned char bit4		: 1;//逆变器AC柜过温
        unsigned char bit5		: 1;//功率模块过热
        unsigned char bit6		: 1;//直流侧电压极性反
        unsigned char bit7		: 1;//直流侧电压过压
        unsigned char bit8		: 1;//正母线对地绝缘故障
        unsigned char bit9		: 1;//负母线对地绝缘故障
        unsigned char bit10		: 1;//漏电流突变故障
        unsigned char bit11		: 1;//漏电流连续超限
        unsigned char bit12		: 1;//逆变器DC柜过温
        unsigned char bit13		: 1;//直流配电柜过温
        unsigned char bit14		: 1;//历史故障存储失败
        unsigned char bit15		: 1;//调试错误
    }sysFault;
    struct
    {
        unsigned char bit0		: 1;//电网AB线电压过压
        unsigned char bit1		: 1;//电网BC线电压过压
        unsigned char bit2		: 1;//电网CA线电压过压
        unsigned char bit3		: 1;//电网AB线电压欠压
        unsigned char bit4		: 1;//电网BC线电压欠压
        unsigned char bit5		: 1;//电网CA线电压欠压
        unsigned char bit6		: 1;//电网跌落超限
        unsigned char bit7		: 1;//电网过频
        unsigned char bit8		: 1;//电网欠频
        unsigned char bit9		: 1;//电网电压相序错误
        unsigned char bit10		: 1;//模块A相软件过流
        unsigned char bit11		: 1;//模块B相软件过流
        unsigned char bit12		: 1;//模块C相软件过流
        unsigned char bit13		: 1;//孤岛故障
        unsigned char bit14		: 1;//滤波电容过流
        unsigned char bit15		: 1;//滤波电容欠压
    }gridFault;
    struct
    {
        unsigned char bit0		: 1;//母线预充电超时
        unsigned char bit1		: 1;//母线预充电过压
        unsigned char bit2		: 1;//母线预充电欠压
        unsigned char bit3		: 1;//母线不控整流过压
        unsigned char bit4		: 1;//母线不控整流欠压
        unsigned char bit5		: 1;//母线正常运行过压
        unsigned char bit6		: 1;//母线正常运行欠压
        unsigned char bit7		: 1;//直流侧电流过流
        unsigned char bit8		: 1;//母线短路
        unsigned char bit9		: 1;//直流侧负极接地故障
        unsigned char bit10		: 1;//电网漏电流超限故障
        unsigned char bit11		: 1;//直流反灌故障
        unsigned char bit12		: 1;//Chopper误触发
        unsigned char bit13		: 1;//Chopper触发失败
        unsigned char bit14		: 1;//Chopper单次动作时间超限
        unsigned char bit15		: 1;//Chopper自检失败
    }busFault;
    struct
    {
        unsigned char bit0		: 1;//并网接触器闭合失败
        unsigned char bit1		: 1;//并网接触器断开失败
        unsigned char bit2		: 1;//并网接触器闭合状态错误
        unsigned char bit3		: 1;//并网接触器断开状态错误
        unsigned char bit4		: 1;//充电接触器状态错误
        unsigned char bit5		: 1;//直流侧开关断开失败
        unsigned char bit6		: 1;//直流侧开关闭合失败
        unsigned char bit7		: 1;//直流侧开关状态错误
        unsigned char bit8		: 1;//保留
        unsigned char bit9		: 1;//保留
        unsigned char bit10		: 1;//烟雾传感器故障
        unsigned char bit11		: 1;//交流侧电抗器过温
        unsigned char bit12		: 1;//电源输入故障
        unsigned char bit13		: 1;//紧急停机
        unsigned char bit14		: 1;//变压器过温
        unsigned char bit15		: 1;//功率风扇驱动失败
    }interfaceFault;
    struct
    {
        unsigned char bit0		: 1;//模块电流不平衡告警
        unsigned char bit1		: 1;//加热除湿告警
        unsigned char bit2		: 1;//电网跌落告警
        unsigned char bit3		: 1;//逆变器禁止运行
        unsigned char bit4		: 1;//电网频率超出启动范围
        unsigned char bit5		: 1;//直流侧电压超出启动范围
        unsigned char bit6		: 1;//交流侧断路器未闭合
        unsigned char bit7		: 1;//启停按钮处于STOP
        unsigned char bit8		: 1;//电网电压超出启动范围
        unsigned char bit9		: 1;//配电柜开关断开告警
        unsigned char bit10		: 1;//过温减载告警
        unsigned char bit11		: 1;//Chopper不可用告警
        unsigned char bit12		: 1;//Chopper动作次数超限告警
        unsigned char bit13		: 1;//电网电流不平衡告警
        unsigned char bit14		: 1;//直流侧功率异常告警
        unsigned char bit15		: 1;//保留
    }warning1;
    struct
    {
        unsigned char bit0		: 1;//汇流箱通信错误告警
        unsigned char bit1		: 1;//汇流箱电流不平衡告警
        unsigned char bit2		: 1;//1号汇流箱电流异常告警
        unsigned char bit3		: 1;//2号汇流箱电流异常告警
        unsigned char bit4		: 1;//3号汇流箱电流异常告警
        unsigned char bit5		: 1;//4号汇流箱电流异常告警
        unsigned char bit6		: 1;//5号汇流箱电流异常告警
        unsigned char bit7		: 1;//6号汇流箱电流异常告警
        unsigned char bit8		: 1;//7号汇流箱电流异常告警
        unsigned char bit9		: 1;//8号汇流箱电流异常告警
        unsigned char bit10		: 1;//保留
        unsigned char bit11		: 1;//正母线对地绝缘故障
        unsigned char bit12		: 1;//负母线对地绝缘故障
        unsigned char bit13		: 1;//烟雾传感器故障
        unsigned char bit14		: 1;//交流侧防雷器故障
        unsigned char bit15		: 1;//直流侧防雷器故障
    }warning2;
    unsigned short cumulativeEnergyLow;//累计发电量低字
    unsigned short cumulativeEnergyHigh;//累计发电量高字
    unsigned short combineRuntimeLow;//累计并网运行时间低字
    unsigned short combineRuntimeHigh;//累计并网运行时间高字
    unsigned short cumulativeEnergyLow_year;//当年累计发电量低字
    unsigned short cumulativeEnergyHigh_year;//当年累计发电量高字
    unsigned short cumulativeEnergyLow_month;//当月累计发电量低字
    unsigned short cumulativeEnergyHigh_month;//当月累计发电量高字
    unsigned short cumulativeEnergyLow_day;//当日累计发电量低字
    unsigned short cumulativeEnergyHigh_day;//当日累计发电量高字
    unsigned short cumulativeEnergyLow_hour;//当前小时累计发电量低字
    unsigned short cumulativeEnergyHigh_hour;//当前小时累计发电量高字
    //系统上电时间：年月日时分秒
    unsigned short sysPowerOn_year;
    unsigned short sysPowerOn_month;
    unsigned short sysPowerOn_day;
    unsigned short sysPowerOn_hour;
    unsigned short sysPowerOn_minute;
    unsigned short sysPowerOn_second;
    //系统开机时间：年月日时分秒
    unsigned short sysStartUp_year;
    unsigned short sysStartUp_month;
    unsigned short sysStartUp_day;
    unsigned short sysStartUp_hour;
    unsigned short sysStartUp_minute;
    unsigned short sysStartUp_second;
    //系统关机时间：年月日时分秒
    unsigned short sysPowerOff_year;
    unsigned short sysPowerOff_month;
    unsigned short sysPowerOff_day;
    unsigned short sysPowerOff_hour;
    unsigned short sysPowerOff_minute;
    unsigned short sysPowerOff_second;

    unsigned short powerFactor;//功率因数
    unsigned short convertEfficiency;//系统转换效率
    //系统时间：年月日时分秒
    unsigned short sysTime_year;
    unsigned short sysTime_month;
    unsigned short sysTime_day;
    unsigned short sysTime_hour;
    unsigned short sysTime_minute;
    unsigned short sysTime_second;
    unsigned short runStatus;//运行状态
    unsigned short dayRunTime_Low;//日运行时间低字
    unsigned short dayRunTime_High;//日运行时间高字
}__attribute__ ((packed)) FramePowerOptimizerCabinetCMDSet;

typedef struct _FramePowerOptimizerInfo1
{
    short PowerOptimizerID;
    short inVol1;
    short inVol2;
    short inVol3;
    short inVol4;
    short curBranch1;
    short curBranch2;
    short curBranch3;
    short curBranch4;
    short curBranch5;
    short curBranch6;
    short curBranch7;
    short curBranch8;
    short curBranch9;
    short curBranch10;
    short curBranch11;
    short curBranch12;
    short curBranch13;
    short curBranch14;
    short curBranch15;
    short curBranch16;
    short realPower;
    short radiatorTemp;

    struct
    {
        unsigned char bit0		: 1;//
        unsigned char bit1		: 1;//
        unsigned char bit2		: 1;//
        unsigned char bit3		: 1;//
        unsigned char bit4		: 1;//
        unsigned char bit5		: 1;//
        unsigned char bit6		: 1;//
        unsigned char reserve		: 1;//启停按钮处于STOP
    }fault1;
    char reserve1;
    struct
    {
        unsigned char bit0		: 1;//汇流箱通信错误告警
        unsigned char bit1		: 1;//汇流箱电流不平衡告警
        unsigned char bit2		: 1;//1号汇流箱电流异常告警
        unsigned char bit3		: 1;//2号汇流箱电流异常告警
        unsigned char bit4		: 1;//3号汇流箱电流异常告警
        unsigned char bit5		: 1;//4号汇流箱电流异常告警
        unsigned char bit6		: 1;//5号汇流箱电流异常告警
        unsigned char bit7		: 1;//6号汇流箱电流异常告警
        unsigned char bit8		: 1;//7号汇流箱电流异常告警
        unsigned char bit9		: 1;//8号汇流箱电流异常告警
        unsigned char bit10		: 1;//
        unsigned char bit11		: 1;//正母线对地绝缘故障
        unsigned char bit12		: 1;//负母线对地绝缘故障
        unsigned char bit13		: 1;//烟雾传感器故障
        unsigned char bit14		: 1;//交流侧防雷器故障
        unsigned char bit15		: 1;//直流侧防雷器故障
    }fault2;

    struct
    {
        unsigned char bit0		: 1;//
        unsigned char bit1		: 1;//
        unsigned char bit2		: 1;//
        unsigned char bit3		: 1;//
        unsigned char bit4		: 1;//
        unsigned char bit5		: 1;//
        unsigned char bit6		: 1;//
        unsigned char bit7		: 1;//
        unsigned char bit8		: 1;//
        unsigned char bit9		: 1;//
        unsigned char bit10		: 1;//
        unsigned char bit11		: 1;//
        unsigned char bit12		: 1;//
        unsigned char bit13		: 1;//
        unsigned char bit14		: 1;//
        unsigned char bit15		: 1;//
    }warning;

    short combinerStatus;
    short softVer_L;
    short softVer_H;
    struct
    {
        unsigned char bit0		: 1;//
        unsigned char bit1		: 1;//
        unsigned char reserve		: 6;//启停按钮处于STOP
    }sysRequestStatus;
    char reserve2;

    short inVol5;
    short inVol6;
    short inVol7;
    short inVol8;

    short outVol;

}__attribute__ ((packed)) FramePowerOptimizerInfo1;

//能效模块控制指令
typedef struct _FrameCtrlModuleWork
{
    unsigned char ucInnerID;
    unsigned char ucCmd;
    unsigned char ucReserved[6];
}__attribute__ ((packed)) FrameCtrlModuleWork;

//能效功率设置指令
typedef struct _FrameSetModulePower
{
    unsigned char ucInnerID;
    short sPower;
    unsigned char ucReserved[6];
}__attribute__ ((packed)) FrameSetModulePower;

class cEnergyPlanProtocol : public cJ1939GeneralProtocol
{
public:
    cEnergyPlanProtocol();
    ~cEnergyPlanProtocol();
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

    //解析能效系统模块控制<----控制中心
    virtual void ParseCtrlModule_EP(InfoMap CenterMap, unsigned char ucTermID);
    //解析能效系统功率控制<----控制中心
    virtual void ParseSetPower_EP(InfoMap CenterMap, unsigned char ucTermID);

    //解析CCU参数设置<----控制中心
    virtual void ParseCCUArg(InfoMap CenterMap, unsigned char ucTermID);
    //解析CCU参数查询<----控制中心
    virtual void ParseCCUArgQuery(InfoMap CenterMap, unsigned char ucTermID);

    //解析长帧
    //    virtual void ParseLongFrame(FrameLongPackage * pLongPackage);

protected:
    //获取参数设置结果类型
    virtual int GetParamAckType(unsigned char ucPF);
    //解析遥调指令<----控制中心
    virtual void ParseTeleadjust(InfoMap CenterMap, unsigned char ucTermID);
    //获取协议版本号枚举
    virtual void GetProVerEnum(unsigned char * pVer);
private:

    char runStatus;
    char fireStop;
    char dcBreaker;
    //检查广播召唤帧状态
    //    void CheckBroadCastState();

    void MakeFrameCtrlModuleWork(InfoMap &CenterMap, can_frame *pFrame, unsigned char ucCanID);
    void MakeFrameSetModulePower(InfoMap &CenterMap, can_frame *pFrame, unsigned char ucCanID);
    void SendFrameCtrlModuleWork(InfoMap CenterMap, unsigned char ucTermID);
    void SendFrameSetModulePower(InfoMap CenterMap, unsigned char ucTermID);


    //能效计划
    void ParseFourQuadrantCabinetInfo1(unsigned char * pData, QList<CanMapNode> &ToCenterList,char,char);
    void ParseACDCInverterCabinetInfo(unsigned char * pData, QList<CanMapNode> &ToCenterList, char pgnID,char canAddr);
    //        void ParseEnergyStorageCabinetInfo1(unsigned char * pData, QList<CanMapNode> &ToCenterList,char,char);
    void ParsePhotoVoltaicCabinetInfo1(unsigned char * pData, QList<CanMapNode> &ToCenterList,char,char);
    void ParseEnergyStorageCabinetInfo(unsigned char * pData, QList<CanMapNode> &ToCenterList, char pgnID, char canAddr);
    void ParseEnergyStorageCabinetDCDCInfo(FrameLongPackage * pLongPackage, QList<CanMapNode> &ToCenterList, char pgnID,char canAddr);
    void ParseChargeDisChargeCabinetDCDCInfo(FrameLongPackage * pLongPackage, QList<CanMapNode> &ToCenterList, char pgnID,char canAddr);
    void ParseACDCInfo1(FrameLongPackage * pLongPackage, QList<CanMapNode> &ToCenterList, char,char);
    void ParseDCDCInfo(unsigned char * pData, QList<CanMapNode> &ToCenterList,char,char);
    void ParseHumitureInfo1(unsigned char * pData, QList<CanMapNode> &ToCenterList, char pgnID,char);
    void ParseEnergyStorageCabinetBatteryInfo(FrameLongPackage * pLongPackage, QList<CanMapNode> &ToCenterList,char,char);
    void ParseChargeDischargeCabinetInfo1(unsigned char * pData, QList<CanMapNode> &ToCenterList,char,char);
    void ParseSysControlCabinetInfo1(unsigned char * pData, QList<CanMapNode> &ToCenterList, char,char);
    void ParseSysControlCabinetInfo2(unsigned char * pData, QList<CanMapNode> &ToCenterList, char,char);
    void ParseMainDistributionCabinetInfo1(unsigned char * pData, QList<CanMapNode> &ToCenterList,char,char);
    void ParsePowerOptimizerInfo1(FrameLongPackage * , QList<CanMapNode> &ToCenterList,char,char);

    void ParseEMSHumitureInfo1(unsigned char * pData, QList<CanMapNode> &ToCenterList, char pgnID,char);
    void ParseEMSHumitureInfo2(unsigned char * pData, QList<CanMapNode> &ToCenterList, char pgnID,char);
    void ParseEMSSwitchPointInfo(unsigned char * pData, QList<CanMapNode> &ToCenterList, char pgnID,char);

    //解析模块故障信息
    void ParseFrameModuleFaultInfo(unsigned char * pData, QList<CanMapNode> &ToCenterList);

    //解析长帧
    virtual void ParseLongFrame(FrameLongPackage * pLongPackage);

};

#endif // DCCANPROTOCOL_H
