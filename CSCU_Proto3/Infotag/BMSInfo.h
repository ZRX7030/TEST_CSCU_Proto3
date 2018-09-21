#ifndef BMSINFO_H
#define BMSINFO_H

#include "GeneralData/GeneralData.h"

/***********************充电终端BMS信息******************************/
//握手阶段
InfoAddr Addr_BMSProtocolVer_BMS = 0x233001;//BMS协议版本号
InfoAddr Addr_MaxAllowedVoltage_BMS = 0x233002;//最高允许充电总电压
InfoAddr Addr_BatteryType_BMS = 0x233003;//电池类型
InfoAddr Addr_BatteryRatedCapacity_BMS = 0x233004;//整车动力蓄电池系统额定容量（AH）
InfoAddr Addr_BatteryRatedVoltage_BMS = 0x233005;//整车动力蓄电池额定总电压
InfoAddr Addr_BatteryManufacturer_BMS = 0x233006;//电池生产厂商名称
InfoAddr Addr_BatterySerialNum_BMS = 0x233007;//电池组序号
InfoAddr Addr_BatteryProduceDate_BMS = 0x233008;//电池组生产日期
InfoAddr Addr_BatteryChargeTime_BMS = 0x233009;//电池组充电次数
InfoAddr Addr_BatteryOwnerFlag_BMS = 0x23300A;//电池组产权标识
InfoAddr Addr_BatteryVIN_BMS = 0x23300B;//车辆识别码（VIN）
InfoAddr Addr_BMSSoftwareVer_BMS = 0x23300C;//BMS软件版本号

//参数配置阶段
InfoAddr Addr_SingleBatteryMaxAllowedVoltage_BMS = 0x23300D;//单体动力蓄电池最高允许充电电压
InfoAddr Addr_MaxAllowedCurrent_BMS = 0x23300E;//最高允许充电电流
InfoAddr Addr_BatteryTotalEnergy_BMS = 0x23300F;//动力蓄电池标称总能量
InfoAddr Addr_MaxParamAllowedVoltage_BMS = 0x233010;//最高允许充电总电压
InfoAddr Addr_MaxtAllowedTemp_BMS = 0x233011;//最高允许温度
InfoAddr Addr_ParamSOC_BMS = 0x233012;//整车动力蓄电池荷电状态（SOC）(参数配置阶段)
InfoAddr Addr_BatteryVoltage_BMS = 0x233013;//整车动力蓄电池当前电池电压
InfoAddr Addr_MaxOutputVoltage_BMS = 0x233014;//最大输出电压
InfoAddr Addr_MinOutputVoltage_BMS = 0x233015;//最小输出电压
InfoAddr Addr_MaxOutputCurrent_BMS = 0x233016;//最大输出电流
InfoAddr Addr_MinOutputCurrent_BMS = 0x233017;//最小输出电流

//BMS充电阶段
InfoAddr Addr_NeedVoltage_BMS = 0x233018;//需求电压
InfoAddr Addr_NeedCurrent_BMS = 0x233019;//需求电流
InfoAddr Addr_ChargeType_BMS = 0x23301A;//充电模式
InfoAddr Addr_ChargeVoltageMeasured_BMS = 0x23301B;//充电电压测量值
InfoAddr Addr_ChargeCurrentMeasured_BMS = 0x23301C;//充电电流测量值
InfoAddr Addr_MaxSingleBatteryVoltage_BMS = 0x23301D;//最高单体动力蓄电池电压
//InfoAddr Addr_MaxSingleBatteryVoltageSerial_BMS = 0x23301E;//最高单体动力蓄电池电压所在组号
//InfoAddr Addr_MaxSingleBatterySerialNum_BMS = 0x23301F;//当前荷电状态SOC（%）
//InfoAddr Addr_NowSOC_BMS = 0x233020;//估算剩余充电时间（min）
InfoAddr Addr_MaxSingleBatterySerialNum_BMS = 0x23301E;//最高单体动力蓄电池电压所在组号
InfoAddr Addr_NowSOC_BMS = 0x23301F;//当前荷电状态SOC（%）
InfoAddr Addr_LeftTime = 0x233020; //估算剩余充电时间（min）

InfoAddr Addr_SingleBatteryNum_BMS = 0x233021;//最高单体动力蓄电池电压所在编号
InfoAddr Addr_MaxBatteryTemp_BMS = 0x233022;//最高动力蓄电池温度
InfoAddr Addr_MaxTempPointNum_BMS = 0x233023;//最高温度检测点编号
InfoAddr Addr_MinBatteryTemp_BMS = 0x233024;//最低动力蓄电池温度
InfoAddr Addr_MinTempPointNum_BMS = 0x233025;//最低动力蓄电池温度检测点编号
InfoAddr Addr_ChargePermitFlag_BMS = 0x233026;//BMS充电允许标志

//BMS中止充电阶段
InfoAddr Addr_BMSStopReason_BMS = 0x233027;//BMS中止充电原因
InfoAddr Addr_BMSFaultReason_BMS = 0x233028;//BMS中止充电故障原因
InfoAddr Addr_BMSErrorReason_BMS = 0x233029;//BMS中止充电错误原因
InfoAddr Addr_ChargerStopReason_BMS = 0x23302A;//充电机中止充电原因
InfoAddr Addr_ChargerFaultReason_BMS = 0x23302B;//充电机中止充电故障原因
InfoAddr Addr_ChargerErrorReason_BMS = 0x23302C;//充电机中止充电错误原因
InfoAddr Addr_ChargeEndSOC_BMS = 0x23302D;//中止荷电状态SOC（%）
InfoAddr Addr_MinSingleVoltage_BMS = 0x23302E;//动力蓄电池单体最低电压(普通遥测)
InfoAddr Addr_MinSingleVoltage_BMS_END = 0x23302E0;//动力蓄电池单体最低电压(结束阶段发送)

InfoAddr Addr_MaxSingleVoltage_BMS = 0x23302F;//动力蓄电池单体最高电压
InfoAddr Addr_MinTemp_BMS = 0x233030;//动力蓄电池最低温度
InfoAddr Addr_MaxTemp_BMS = 0x233031;//动力蓄电池最高温度
InfoAddr Addr_BMSErrorFrame_BMS = 0x233032;//BMS错误报文
InfoAddr Addr_ChargerErrorFrame_BMS = 0x233033;//充电机错误报文

#endif // BMSINFO_H
