#define TELECONTROL_H

#include "GeneralData/GeneralData.h"
//typedef  const unsigned int InfoAddr;

/***********************遥控指令******************************/
InfoAddr Addr_ChargeCmd_Ctrl = 0x230003;//充电指令
InfoAddr Addr_AckResult_Ctrl = 0x230004;//响应结果
InfoAddr Addr_ExecResult_Ctrl = 0x230005;//执行结果

InfoAddr Addr_OrderNumber_Ctrl = 0x230001;//流水号=订单号
InfoAddr Addr_EventOccurTime_Ctrl = 0x23004A;//事件产生时间
InfoAddr Addr_EventSendTime_Ctrl = 0x23004B;//事件发送时间
InfoAddr Addr_GUID_Ctrl = 0x23004C;//GUID
InfoAddr Addr_GUIDFlag_Ctrl = 0x23004D;//GUID产生标识
InfoAddr Addr_ReSendOver_Ctrl = 0x23004E;//离线重发完成
InfoAddr Addr_CmdNum_Ctrl = 0x23004F;//指令编号
InfoAddr Addr_CmdSrc_Ctrl = 0x230050;//指令来源
InfoAddr Addr_ChargeStartReason_Ctrl = 0x230051;//开始充电原因
InfoAddr Addr_ChargeStopReason_Ctrl = 0x230052;//结束充电原因


/***********************遥调指令******************************/
InfoAddr Addr_CabMaxPower_Adj = 0x23035E;//直流柜最大输出功率
InfoAddr Addr_CCU_ParamResult_Adj = 0x230B01;//CCU参数设置结果

/***********************削峰填谷******************************/
InfoAddr Addr_AckChargeResult_Ctr_Peak = 0x2303CD;//充电结果
InfoAddr Addr_AckResult_Ctr_Peak = 0x2403CD;//平台返回确认结果
InfoAddr Addr_StopReasion_Peak = 0x2403CE;//终止充电原因
InfoAddr Addr_StartTime_Peak = 0x2403CF;//开始充电时间
InfoAddr Addr_StopTime_Peak = 0x2403D0;//结束充电时间
InfoAddr Addr_ChargeEnergy_Peak = 0x2403D1;//充电电能
InfoAddr Addr_AckResultFaile_Ctr_Peak = 0x2403D2;//启动失败的响应的响应结果
InfoAddr Addr_OrderType_Peak = 0x2403D3;//订单类型: 1充电订单2放电订单
InfoAddr Addr_GetCmdTime_Peak = 0x2403D4;//获取指令时间
