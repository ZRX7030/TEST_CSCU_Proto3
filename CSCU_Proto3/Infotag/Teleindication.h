#ifndef TELEINDICATION_H
#define TELEINDICATION_H

#include "GeneralData/GeneralData.h"
//typedef  const unsigned int InfoAddr;

/***********************充电终端遥信******************************/
InfoAddr Addr_ChargeGunNum_Term = 0x200000;//充电接口标识（未用）
InfoAddr Addr_LinkState_Term = 0x200001;//连接确认开关状态
InfoAddr Addr_RelyState_Term = 0x200002;//输出继电器状态
InfoAddr Addr_ParkingSpaceFreeFlag_Term = 0x200003;//停车位状态（未用）
InfoAddr Addr_WorkState_Term = 0x200004;//充电机工作状态
InfoAddr Addr_FaultCode_Term = 0x200005;//故障代码
InfoAddr Addr_BMSFaultCode_Term = 0x200006;//BMS故障信息
InfoAddr Addr_ChargeEndCode_Term = 0x200007;//中止充电原因(104) = 充电中止代码(CAN)
InfoAddr Addr_CtrlModeFlag_Term = 0x2000080;//控制模式, 标识上传 6H: 轮充模式; 7H: 群充模式
InfoAddr Addr_GroupModeFlag_Term = 0x2000081;//群充策略, 标识上传 1XH: A策略 2XH: B策略 3XH: C策略
InfoAddr Addr_AuxPowerType_Term = 0x200009;//辅助电源类型

InfoAddr Addr_TermID_PairTerm = 0x2309E8;//配对终端ID
InfoAddr Addr_WorkState_PairTerm = 0x2309E9;//双枪工作模式

/***********************直流柜遥信******************************/
//CCU
InfoAddr Addr_CCUWorkState_DCcab = 0x237001;//CCU运行状态
//InfoAddr Addr_CCUAlarmState_DCcab = 0x237002;//CCU告警状态
InfoAddr Addr_CCUInputContactor_DCcab = 0x237003;//输入接触器状态
InfoAddr Addr_CCULinkageContactor_DCcab = 0x237004;//联动接触器状态
InfoAddr Addr_CCUSystemType_DCcab = 0x237005;//系统机型

//直流模块
InfoAddr Addr_DCModuleWorkState_DCcab = 0x237031;//模块运行状态
//InfoAddr Addr_DCModuleAlarmState_DCcab = 0x233032;//模块告警状态
InfoAddr Addr_DCModuleGroupNum_DCcab = 0x233033;//模块所属分组

//PDU
InfoAddr Addr_PDUWorkState_DCcab = 0x23308E;//PDU运行状态
InfoAddr Addr_PDUGreenLight_DCcab = 0x23308F;//PDU绿灯
InfoAddr Addr_PDUYellowLight_DCcab = 0x233090;//PDU黄灯
InfoAddr Addr_PDURedLight_DCcab = 0x233091;//PDU红灯


//InfoAddr Addr_PDUAlarmState_DCcab = 0x23308F;//PDU告警状态

//分支箱
InfoAddr Addr_BOXWorkState_DCcab = 0x2330C7;//分支箱运行状态
//InfoAddr Addr_BOXAlarmState_DCcab = 0x2330C8;//分支箱告警状态

/***********************突发遥信******************************/
InfoAddr Addr_ChargeGunNum_Sudden = 0x23005B;//突发充电接口标识（未用）
InfoAddr Addr_LinkState_Sudden = 0x23005C;//突发连接确认开关状态
InfoAddr Addr_RelyState_Sudden = 0x23005D;//突发输出继电器状态
InfoAddr Addr_ParkingSpaceFreeFlag_Sudden = 0x230064;//突发停车位状态
InfoAddr Addr_WorkState_Sudden = 0x23005E;//突发输出继电器状态
InfoAddr Addr_FaultCode_Sudden = 0x23005F;//突发故障代码
InfoAddr Addr_BMSFaultCode_Sudden = 0x230060;//突发BMS故障信息
InfoAddr Addr_ChargeEndCode_Sudden = 0x230061;//突发中止充电原因(104) = 充电中止代码(CAN)
InfoAddr Addr_CtrlModeFlag_Sudden = 0x230065;//突发控制模式
InfoAddr Addr_GroupQueueFlag_Sudden = 0x230062;//群充策略标识上传
InfoAddr Addr_AuxPowerType_Sudden = 0x230063;//突发辅助电源类型

/***********************子站规格******************************/
InfoAddr Addr_CoreVer_CSCU = 0x232002;//CSCU内核版本号
InfoAddr Addr_SoftwareVer_CSCU = 0x232003;//CSCU软件版本号
InfoAddr Addr_SerialScreenVer_CSCU = 0x232004;//CSCU串口屏版本号
InfoAddr Addr_SerialNumber_CSCU = 0x232005;//CSCU条码号

/***********************设备规格******************************/
InfoAddr Addr_SoftwareVer_Term = 0x232017;//软件版本号1
InfoAddr Addr_SoftwareVer1_Term = 0x232018;//软件版本号2
InfoAddr Addr_SoftwareVer2_Term = 0x232019;//软件版本号3
InfoAddr Addr_SerialNumber_Term = 0x23201A;//条码号
InfoAddr Addr_HardwareVer_Term = 0x23201B;//硬件版本
InfoAddr Addr_SlotNum_Term = 0x23201C;//槽位号
InfoAddr Addr_SepcEndFlag_Term = 0x23201C1;//设备规格信息传输完成标志(CAN)
#endif

