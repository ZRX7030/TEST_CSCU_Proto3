#ifndef CSCUBUS_H
#define CSCUBUS_H

#include <QMap>

#include "GeneralData/GeneralData.h"
#include "Infotag/Telecontrol.h"
#include "Infotag/Teleindication.h"
#include "Infotag/Telemetering.h"
#include "Infotag/Ammeter.h"
#include "Infotag/BMSInfo.h"

typedef QMap <unsigned int , QByteArray> InfoMap;

//终端类型枚举
typedef enum _TermType
{
    TermType_Undef = 0, //未定义终端类型

    TermType_ACSin, //001 ~ 98 交流单相终端
    TermType_Reserve1,//99~108 保留
    TermType_ACLoadDisBox,//109~112 交流负载总配电柜
    TermType_SysDisBox,//113~116 系统配电柜
    TermType_DCPVCtrlBox,//117~126 直流光伏控制柜
    TermType_4QuaConvertBox,//127 ~131 四象限变换柜
    TermType_IndeInventerBox,//132 ~136 独立逆变柜
    TermType_DCDischargeBox,//137 ~ 146 直流充放电柜
    TermType_DCDischargeSEBox,//147 ~ 150 直流充放电蓄能柜
    TermType_ACThr, //151 ~ 180 三相交流充电模块
    TermType_DC, //181 ~ 230 直流终端 = PDU
    TermType_CCU,    //231 ~ 240 CCU
    TermType_CSCU //241 ~ 253 集中控制器

}TermType;

//地址分配枚举
typedef enum _CanIDDivision
{
    ID_MinACSinCanID =1,
    ID_MaxACSinCanID =98,
    ID_EMSCanID =99,//能效系统EMS地址
//    ID_MinEnergyPlanCanID = 102,
//    ID_MaxEnergyPlanCanID = 150,
    ID_MinACLoadDistributionCabinet = 109,  //交流负载总配电柜
    ID_MaxACLoadDistributionCabinet = 112,  //交流负载总配电柜
    ID_MinSystemDistributionCabinet = 113,  //系统配电柜
    ID_MaxSystemDistributionCabinet = 116,  //系统配电柜
    ID_MinDCPhotovoltaicControlCabinet = 117,   //直流光伏控制柜
    ID_MaxDCPhotovoltaicControlCabinet = 126,   //直流光伏控制柜
    ID_MinFourQuadrantConverterCabinet = 127,   //四象限变换柜
    ID_MaxFourQuadrantConverterCabinet = 131,   //四象限变换柜
    ID_MinIndependentInverterCabinet = 132,   //独立逆变柜
    ID_MaxIndependentInverterCabinet = 136,   //独立逆变柜
    ID_MinDCChargeDischargeCabinet = 137,   //直流充放电柜
    ID_MaxDCChargeDischargeCabinet = 146,   //直流充放电柜
    ID_MinDCEnergyStorageCabinet = 147,   //直流储能柜
    ID_MaxDCEnergyStorageCabinet = 150,   //直流储能柜

    ID_MinACThrCanID = 151, //三相交流充电模块
    ID_MaxACThrCanID = 180,

    ID_MinDCCanID = 181,//直流终端
    ID_MaxDCCanID = 230,

    ID_MinCCUCanID = 231,//CCU
    ID_MaxCCUCanID = 240,

    ID_MinControlCenterCanID = 241,//集中控制器
    ID_DefaultControlCenterCanID = 250,
    ID_MaxControlCenterCanID = 253,

    ID_BroadCastCanID = 255
}CanIDDivision;

//内部地址分配枚举
typedef enum _InnerIDDivision
{
    InnerID_MinMod =0,
    InnerID_MaxMod =31,
    InnerID_MinPDU =181,
    InnerID_MaxPDU =230,
    //能耗,储能柜专用
    InnerID_ES_TM_101 = 147,
    InnerID_ES_TM_102 = 148,
    InnerID_MinBOX =110,
    InnerID_MaxBOX =123
}InnerIDDivision;

typedef enum _TermParamType
{
    Param_AutoProtect_DC, //主动防护参数设置
    Param_FlexableCharge_DC, //柔性充电参数设置
    Param_GeneralStatic_DC, //通用静态参数设置
    Param_GeneralDynamic_DC, //通用动态参数设置
    Param_TwoGun_DC, //双枪参数设置
    Param_CCU_DC, //CCU参数设置
    Param_Branch_DC, //分支箱参数设置
    Param_PDU_DC, //PDU参数设置
    Param_PowerModule_DC //模块参数设置
}TermParamType;

typedef enum _InfoAddrType
{
	AddrType_Unknown = 0,	//未知主题
    AddrType_Common,    //通用数据
    AddrType_EnergyPlanEnvSignal,//能效计划环境量信息
    AddrType_BatteryStatus,//能效储能电池信息
    AddrType_PowerOptimizerInfo,//能效功率优化器信息
    AddrType_CabinetDevChange,//能效系统柜子设备有变动
/********充电相关主题*****************/
    AddrType_TermSignal, //遥信_充电设备 √。服务器订阅
    AddrType_TermLogicChargeSignal, //遥信_终端状态 √

    AddrType_TermAdjustmentApply , //遥调_充电设备申请 √ 负荷控制发布，充电业务订阅
    AddrType_TermAdjustment , //遥调_充电设备 √。充电设备订阅
    AddrType_TermAdjustmentAck , //遥调_充电设备结果 √。服务器订阅

    AddrType_CmdCtrl_Apply,//遥控_充电控制申请 √。服务器发布，充电业务订阅
	AddrType_CmdCtrl_AckResult,//遥控_充电控制申请ACK(充电申请响应结果) √。服务器订阅
    AddrType_CmdCtrl_ExecResult,//遥控_充电控制申请RESULT(充电申请执行结果) √服务器订阅
	AddrType_CmdCtrl , //遥控_充电控制 √
    AddrType_CmdCtrl_Ack, //遥控_充电控制ACK √
    AddrType_FrozenEnergy, //遥控_充电冻结电量 √ 。充电服务发布，服务器订阅
    AddrType_FrozenEnergy_Result, //遥控_充电冻结电量结果 √ 。服务器发布，充电服务订阅     == AddrType_Response_Result 冻结电量结果和双枪结果共用
    AddrType_TermMeasure, //遥测_充电设备 √。服务器订阅

/********设备管理相关主题*****************/
    AddrType_ActiveEnergyFault_Term,    //终端有功电能告警, 信息体: CAN地址, 异常电量, 实时数据模块发布, 充电服务模块订阅
    AddrType_FaultState_DCcab,   //直流柜故障状态,  充电设备发布, 设备管理模块订阅
    AddrType_FaultStateChange_DCcab,   //直流柜故障状态变化,  设备管理模块发布, 设备管理模块订阅 , 内容: CCU地址, 模块ID, 最小PDU地址, 最大PDU地址, 故障代码, 直流柜故障状态
    AddrType_FaultRecord_DCcab, //直流柜故障点记录, 充电设备发布, 设备管理模块订阅, 服务器模块订阅, 内容: 故障记录信息, 无须解析
    AddrType_FaultDetail_DCcab,   //直流柜故障录波, 充电设备发布, 设备管理模块订阅, 服务器模块订阅, 内容: 故障录波文件绝对路径, 无须解析
    AddrType_ModuleSpecInfo,    //模块规格信息, 充电设备发布, 设备管理模块订阅

    AddrType_DetailParamApply_ACSin,    //交流单相详细参数申请, 其他模块发布, 设备管理模块订阅
    AddrType_DetailParamSet_ACSin,    //交流单相详细参数设置, 设备管理模块发布, 充电设备模块订阅
    AddrType_DetailParamSetResult_ACSin,    //交流单相详细参数设置结果返回, 充电设备模块发布, 其他模块订阅
    AddrType_DetailParamQuery_ACSin, //交流单相详细参数查询, 其他模块发布,充电设备模块订阅
    AddrType_DetailParamQueryResult_ACSin, //交流单相详细参数查询结果, 充电设备模块发布, 其他模块订阅

    AddrType_DetailParamApply_ACThr,    //交流三相详细参数申请, 其他模块发布, 设备管理模块订阅
    AddrType_DetailParamSet_ACThr,    //交流三相详细参数设置, 设备管理模块发布, 充电设备模块订阅
    AddrType_DetailParamSetResult_ACThr,    //交流三相详细参数设置结果返回, 充电设备模块发布, 其他模块订阅
    AddrType_DetailParamQuery_ACThr, //交流三相详细参数查询, 其他模块发布,充电设备模块订阅
    AddrType_DetailParamQueryResult_ACThr, //交流三相详细参数查询结果, 充电设备模块发布, 其他模块订阅

    AddrType_ActiveProtectApply,    //主动防护功能申请, 其他模块发布, 设备管理模块订阅
    AddrType_ActiveProtectSet,    //主动防护功能设置, 设备管理模块发布, 充电设备模块订阅
    AddrType_ActiveProtectSetResult,    //主动防护功能设置结果返回, 充电设备模块发布, 其他模块订阅
    AddrType_ActiveProtectQuery, //主动防护功能查询, 其他模块发布,充电设备模块订阅
    AddrType_ActiveProtectQueryResult, //主动防护功能查询结果, 充电设备模块发布, 其他模块订阅

    AddrType_FlexibleChargeApply,    //柔性充电功能申请, 其他模块发布, 设备管理模块订阅
    AddrType_FlexibleChargeSet,    //柔性充电功能设置, 设备管理模块发布, 充电设备模块订阅
    AddrType_FlexibleChargeSetResult,    //柔性充电功能设置结果返回, 充电设备模块发布, 其他模块订阅
    AddrType_FlexibleChargeQuery, //柔性充电功能查询, 其他模块发布,充电设备模块订阅
    AddrType_FlexibleChargeQueryResult, //柔性充电功能查询结果, 充电设备模块发布, 其他模块订阅

    AddrType_GeneralStaticArgApply,    //通用静态参数设置申请, 其他模块发布, 设备管理模块订阅
    AddrType_GeneralStaticArgSet,    //通用静态参数设置, 设备管理模块发布, 充电设备模块订阅
    AddrType_GeneralStaticArgResult,    //通用静态参数设置返回结果, 充电设备模块发布, 其他模块订阅
    AddrType_GeneralStaticArgRenew_DB,    //通用静态参数更新数据库请求, 其他模块发布, 设备管理模块订阅
    AddrType_GeneralStaticArgRenewAck_DB,    //通用静态参数更新数据库回复, 其他模块发布, 设备管理模块订阅
    AddrType_GeneralStaticArgQuery,    //通用静态参数设置查询, 其他模块发布, 充电设备模块订阅
    AddrType_GeneralStaticArgQueryResult,    //通用静态参数设置查询结果, 充电设备模块发布, 其他模块订阅

    AddrType_GeneralDynamicArgApply,    //通用动态参数设置申请, 其他模块发布, 设备管理模块订阅
    AddrType_GeneralDynamicArgSet,    //通用动态参数设置, 设备管理模块发布, 充电设备模块订阅
    AddrType_GeneralDynamicArgResult,    //通用动态参数设置结果, 充电设备模块发布, 其他模块订阅
    AddrType_GeneralDynamicArgRenew_DB,    //通用动态参数更新数据库请求, 设备管理模块发布, 充电设备模块订阅
    AddrType_GeneralDynamicArgRenewAck_DB,    //通用动态参数数据库更新数据库回复, 其他模块发布, 设备管理模块订阅
    AddrType_GeneralDynamicArgQuery,    //通用动态参数设置查询, 其他模块发布, 充电设备模块订阅(未用)
    AddrType_GeneralDynamicArgQueryResult,    //通用动态参数设置查询结果, 充电设备模块发布, 其他模块订阅(未用)
    AddrType_GeneralDynamicArgQueryEnd,    //通用动态参数设置查询最终结果, 设备管理模块发布, 其他模块订阅(未用)

    AddrType_CCUArgApply,    //CCU参数设置申请, 其他模块发布, 设备管理模块订阅
    AddrType_CCUArgSet,    //CCU参数设置, 设备管理模块发布, 充电设备模块订阅
    AddrType_CCUArgResult,    //CCU参数设置返回结果, 充电设备发布, 其他模块订阅
    AddrType_CCUArgQuery,    //CCU参数设置查询, 其他模块发布, 充电设备模块订阅
    AddrType_CCUQueryResult,    //CCU参数设置查询结果, 充电设备模块发布, 其他模块订阅


    AddrType_PowerDown_Sys,   //掉电检测,  IO_335X模块发布, 所有模块订阅
    AddrType_FormatChange,  //规格参数更新, 设备管理模块发布, 其他模块订阅

/********BMS相关主题*****************/
    AddrType_TermBMS, //终端BMS信息
    AddrType_ChargeBusiness,  //充电业务相关 √
/********直流柜相关主题*****************/
    AddrType_CCUSignal_DCCab,//CCU遥信
    AddrType_CCUMeasure_DCCab,//CCU遥测
    AddrType_PDUSignal_DCCab,//PDU遥信
    AddrType_PDUMeasure_DCCab,//PDU遥测
    AddrType_BOXSignal_DCCab,//分支箱遥信
    AddrType_BOXMeasure_DCCab,//分支箱遥测
    AddrType_MODSignal_DCCab,//模块遥信
    AddrType_MODMeasure_DCCab,//模块遥测
/********升级主题*****************/
    AddrType_Udisk_Insert,    //检测到U盘插入 ---- 屏幕接收
    AddrType_ExecUpdate,	 //执行升级命令 ---- 标识源, U盘, 导出日志, 升级
    AddrType_UpdateResult,    //升级结果 ---- 屏幕接收
/********设备升级主题*****************/
    AddrType_UpdatePackDir_Dev,    //CAN设备升级包路径, 发布: 升级模块, 订阅, 充电设备
    AddrType_UpdateResult_Dev, //CAN设备升级结果, 发布: 充电设备, 订阅: 升级模块

/********参数变更主题*****************/
    AddrType_ParamChange,	 //参数变更

/********远程抄表数据主题*****************/
    AddrType_RemoteAmmeterReadType,      //远程抄表接收数据主题
    AddrType_RemoteAmmeterSendType,      //远程抄表发送数据主题

/********环境信息数据主题*****************/
    AddrType_Ammeter,      //进线侧电表数据
    AddrType_Ammeter_Disable,      //负荷约束电表读取功率异常
    AddrType_Ammeter_Enable,      //负荷约束电表读取功率异常
    AddrType_TempHumi,    //温湿度计数据
/********配置类相关主题*****************/
	AddrType_TermIndex_Query,//开始获取子站名称、CAN地址和终端编号对应关系。显示屏发布，服务器订阅
	AddrType_TermIndex_QueryFinish,//结束始获取子站名称，CAN地址和终端编号对应关系。服务器发布，显示屏订阅
	AddrType_CarLock_Apply,//车位锁开关。发布未知，服务器订阅	
	AddrType_CarLock_Result,//车位锁开关执行结果。服务器发布，订阅未知
    AddrType_CarLock_ParamSet,//车位锁参数设置
	AddrType_Reboot_Apply,//重启子站。服务器发布，运维模块订阅
	AddrType_Reboot_Result,//重启子站结果。运维模块发布，服务器订阅
/********充电设备配置相关主题*****************/
//    AddrType_GeneralStaticArg_ChargeE,//主题名称:通用静态参数设置; 内容:1.CAN地址（必选） 2.VIN使能  3.低温充电使能  4.电子锁使能 5.设置辅助电源类型 6.枪头最大电流; 发出方:服务器,显示屏;订阅方:充电设备,配置服务
//    AddrType_GeneralDynamicArg_ChargeE,//主题名称:通用动态参数设置; 内容:1.CAN地址（必选）2.设置充电优先等级 3.设置充电完成时间 4.设置充电模式 5.设置群充策略;  发出方:服务器,显示屏;订阅方:充电设备,配置服务
//    AddrType_300kwArg_ChargeE,//主题名称:双系统 300kw 参数设置; 内容:1.配对的CAN地址  2.双枪工作模块 3. 双枪切换类型; 发出方:服务器,显示屏;订阅方:充电设备,配置服务.
/********负荷调度相关主题*****************/
	//置充电优先等级,设置充电完成时间,置充电模式,置充电模式,设置群充策略,设置辅助电源类型
	AddrType_ChargePriority,//设置充电优先等级。服务器发布，负荷调度模块订阅
	AddrType_ChargePriority_Result,//设置充电优先等级返回结果。负荷调度模块发布，服务器订阅
	AddrType_ChargeFinishTime,//设置充电完成时间。服务器发布，负荷调度模块订阅
	AddrType_ChargeFinishTime_Result,//设置充电完成时间返回结果。负荷调度模块发布，服务器订阅
	AddrType_ChargeMode,//设置充电模式。服务器发布，负荷调度模块订阅
	AddrType_ChargeMode_Result,//设置充电模式返回结果。负荷调度模块发布，服务器订阅
	AddrType_MaxLoad,//设置充电电网最大允许负荷。服务器发布，负荷调度模块订阅
	AddrType_MaxLoad_Result,//设置充电电网最大允许负荷返回结果。负荷调度模块发布，服务器订阅
	AddrType_GroupPolicy,//设置群充策略。服务器发布，负荷调度模块订阅
	AddrType_GroupPolicy_Result,//设置群充策略返回结果。负荷调度模块发布，服务器订阅
	AddrType_AuxPower,//设置辅助电源类型。服务器发布，负荷调度模块订阅
	AddrType_AuxPower_Result,//设置辅助电源类型返回结果。负荷调度模块发布，服务器订阅
	AddrType_PredictTime_Burst,//突发充电预估完成时间。负荷调度模块发布，服务器订阅
	AddrType_ModuleFree_Burst,//突发充电模块是否空闲。负荷调度模块发布，服务器订阅
	AddrType_QueueInfo_Burst,//突发充电排队信息。负荷调度模块发布，服务器订阅
    AddrType_QueueInfo,//下发充电排队信息。服务器发布，充电服务模块订阅
	AddrType_PredictTime_Apply,//获取充电预估完成时间。负荷调度模块发布，服务器订阅
	AddrType_PredictTime_Result,//下发充电预估完成时间。服务器发布，负荷调度模块订阅
	AddrType_ChargeDetail,//下发充电总费用和充电明细。服务器发布，负荷调度模块订阅
    AddrType_SmartChargeSet,//本地设置错峰充电参数。串口屏模块发布，负荷调度模块订阅
    AddrType_SmartChargeSet_Result,//本地设置错峰充电参数设置结果。负荷调度模块发布，串口屏模块订阅
    AddrType_LimitChargeCurrent,//错峰充电限制充电电流。负荷调度模块发布，充电设备模块订阅

    /********兼容*****************/
	AddrType_DoubleSys300kwSetting_Publish,//下发直流充电终端双系统300kw参数设置。服务器发布，订阅未知
	AddrType_DoubleSys300kwSetting_Result,//下发直流充电终端双系统300kw参数设置返回结果。发布未知，服务器订阅
	AddrType_DoubleSys300kwSetting_Upload,//上报直流充电终端双系统300kw参数设置。发布未知，服务器订阅

    /********显示屏VIN后6位开始充电*****************/
    AddrType_VINViaScreenApplyCharge,//主题一：VIN后6位申请开始充电, 信息体：CAN地址, VIN码  发出方：显示屏, 订阅方：充电服务
    AddrType_VINViaScreenApplyCharge_Result,//主题二：VIN后6位申请开始充电返回结果, 信息体：CAN地址,结果  发出方：充电服务, 订阅方：显示屏

    /********显示屏本地结束充电*****************/
    AddrType_ScreenApplyStopCharge, //主题一：显示屏申请结束充电, 信息体：CAN地址  发出方：显示屏, 订阅方：充电服务

    /********刷卡远程充电功能相关主题*****************/
    //阶段一
    AddrType_ScreenApplyReadCard,//主题一：显示屏申请读卡,信息体：CAN地址,发出方：显示屏,订阅方：读卡器
    AddrType_ScreenApplyStopCard,//主题一.一：显示屏申请结束读卡,信息体：无, 发出方：显示屏,订阅方：读卡器

    AddrType_CenterReadCard,//主题二：集中读卡卡号, 信息体：卡号、CAN地址, 发出方：读卡器, 订阅方：显示屏
    AddrType_SingleCardApplyAccountInfo,//主题三：单桩申请账户信息, 信息体：卡号、CAN地址, 发出方：充电设备
    AddrType_CenterCardApplyAccountInfo,//主题X：集中刷卡申请账户信息, 信息体：卡号、CAN地址, 发出方：显示屏  订阅方:充电服务
    //阶段二
    AddrType_ChargeServicApplyAccountInfo,//主题四：充电服务申请账户信息, 信息体：卡号、CAN地址, 发出方：充电服务, 订阅方：服务器
    AddrType_ChargeServicApplyAccountInfo_Result,//主题五：服务器返回申请账户信息结果, 信息体：卡号、CAN地址、账户列表及对应余额（只显示现金账户）, 发出方：服务器, 订阅方：充电服务
    AddrType_ApplyAccountInfoResult_ToScreen,//主题六：充电服务返回账户信息给显示屏, 卡号、CAN地址、账户列表及对应余额（只显示现金账户）, 发出方：充电服务, 订阅方:显示屏
    AddrType_ApplyAccountInfoResult_ToChargeEquipment,//主题七：充电服务返回账户信息给充电设备, 卡号、CAN地址、账户列表及对应余额（只显示现金账户）, 发出方：充电服务, 订阅方:充电设备
    //阶段三
    AddrType_InApplyStartChargeByScreen,//主题八：显示屏内部申请开始充电, 信息体：卡号、CAN地址、刷卡申请充电类型、充电金额、充电电量、充电时间, 发出方：显示屏, 订阅方：充电服务
    AddrType_InApplyStartChargeByChargeEquipment,//主题九：充电设备内部申请开始充电, 信息体：CAN地址、刷卡申请充电类型、充电金额、充电电量、充电时间, 发出方：充电设备, 订阅方：充电服务
    AddrType_InApplyStartChargeResult_ToScreen,//主题十：内部申请开始充电结果至显示屏, 信息体：卡号、CAN地址、内部申请充电结果, 发出方：充电服务, 订阅方：显示屏
    AddrType_InApplyStartChargeResult_ToChargeEquipment,//主题十一：内部申请开始充电结果至充电设备, 信息体：卡号、CAN地址、内部申请充电结果, 发出方：充电服务, 订阅方：充电设备
    AddrType_OutApplyStartChargeByChargeServic,//主题十二：充电服务远程申请开始充电, 信息体：卡号、CAN地址、刷卡申请充电类型、充电金额、充电电量、充电时间、申请源模块, 发出方：充电服务, 订阅方：服务器
    AddrType_OutApplyStartChargeByChargeServic_Result,//主题十三：服务器返回申请开始充电结果, 信息体：申请充电结果、CAN地址、卡号。发出方：服务器. 订阅方：充电服务
    AddrType_OutApplyStartChargeResult_ToScreen,//主题十四：远程申请开始充电结果至显示屏, 信息体：卡号、CAN地址、远程申请充电结果, 发出方：充电服务, 订阅方：显示屏
    AddrType_OutApplyStartChargeResult_ToChargeEquipment,//主题十五：远程申请开始充电结果至充电设备, 信息体：卡号、CAN地址、远程申请充电结果, 发出方：充电服务, 订阅方：充电设备
    //阶段四
    AddrType_InApplyStopChargeByScreen,//主题十六：显示屏内部申请结束充电, 信息体：卡号、CAN地址, 发出方：显示屏, 订阅方：充电服务
    AddrType_InApplyStopChargeByChargeEquipment,//主题十七：充电设备内部申请结束充电, 信息体：CAN地址, 发出方：充电设备, 订阅方：充电服务
    AddrType_InApplyStopChargeResult_ToScreen,//主题十八：内部申请结束充电结果至显示屏, 信息体：卡号、CAN地址、内部申请结束充电结果, 发出方：充电服务, 订阅方：显示屏
    AddrType_InApplyStopChargeResult_ToChargeEquipment,//主题十九：内部申请结束充电结果至充电设备, 信息体：卡号、CAN地址、内部申请结束充电结果, 发出方：充电服务, 订阅方：充电设备
    AddrType_OutApplyStopChargeByChargeServic,// 主题二十：充电服务远程申请结束充电, 信息体：卡号、CAN地址, 发出方：充电服务, 订阅方：服务器
    AddrType_OutApplyStopChargeByChargeServic_Result,//主题二十一：服务器返回申请结束充电结果, 信息体：申请充电结果、CAN地址、卡号, 发出方：服务器, 订阅方：充电服务
    AddrType_OutApplyStopChargeResult_ToScreen,//主题二十二：远程申请结束充电结果至显示屏, 信息体：卡号、CAN地址、远程申请结束充电结果, 发出方：充电服务, 订阅方：显示屏
    AddrType_OutApplyStopChargeResult_ToChargeEquipment,//主题二十三：远程申请结束充电结果至充电设备, 信息体：卡号、CAN地址、远程申请结束充电结果, 发出方：充电服务, 订阅方：充电设备


    /********VIN远程充电功能相关主题*****************/
    AddrType_VinNum,//主题一：VIN号, 信息体：CAN地址、VIN, 发出方：充电设备, 订阅方：充电服务
    AddrType_VinApplyStartCharge,//主题二：VIN外部申请开始充电, 信息体：CAN地址、VIN、VIN申请开始充电类型, 发出方：充电服务, 订阅方：服务器
    AddrType_VinApplyStartCharge_Result,//主题三：VIN外部申请开始充电结果, 信息体：CAN地址、VIN、VIN申请开始充电结果, 发出方：服务器, 订阅方：充电服务
    /**错峰充电(VIN)功能相关主题**/
    AddrType_InVinApplyStartCharge,//主题：VIN内部申请开始充电, 信息体：CAN地址、VIN、VIN申请开始充电类型, 发出方：负荷调度, 订阅方：充电服务
    AddrType_InVinApplyStartCharge_Result,//主题：VIN内部申请开始充电结果, 信息体：CAN地址、VIN、VIN内部申请开始充电结果, 发出方：充电服务, 订阅方：负荷调度
    AddrType_InVinApplyStopCharge,//主题：VIN内部申请结束充电, 信息体：CAN地址、VIN, 发出方：负荷调度, 订阅方：充电服务
    AddrType_InVinApplyStopCharge_Result,//主题：VIN内部申请结束充电结果, 信息体：CAN地址、VIN、VIN内部申请结束充电结果, 发出方：充电服务, 订阅方：负荷调度
    AddrType_VinApplyStopCharge,//主题：VIN外部申请结束充电, 信息体：CAN地址、VIN, 发出方：充电服务, 订阅方：服务器
    AddrType_VinApplyStopCharge_Result,//主题：VIN外部申请结束充电结果, 信息体：CAN地址、VIN、VIN申请结束充电结果, 发出方：服务器, 订阅方：充电服务
    /*错峰充电启停*/
    AddrType_ToPeakApplyCharge,//主题：收到经济充电指令后，询问削峰填谷是否允许充电  信息体：CAN地址、充电指令。发出方：充电服务。订阅方：负荷调度
    AddrType_InPeakApplyStartChargeACK,//主题：削峰填谷内部申请开始充电ACK, 信息体：CAN地址、申请开始充电类型, 发出方：负荷调度, 订阅方：充电服务
        //AddrType_InPeakApplyStartCharge_ServerResult,   //开始充电响应结果平台回复确认  发出方： 通信 订阅方：充电服务
       // AddrType_InPeakApplyStopCharge_ServerResult,  //结束充电上传订单平台回复确认  发出方： 通信 订阅方：充电服务
        AddrType_InPeakApplyCharge_ServerResult ,//充电响应结果平台回复确认  发出方： 通信 订阅方：充电服务

       AddrType_InPeakApplyStartCharge,//主题：削峰填谷内部申请开始充电, 信息体：CAN地址、申请开始充电类型, 发出方：负荷调度, 订阅方：充电服务
       AddrType_InPeakApplyStartCharge_Result,//主题：削峰填谷内部申请开始充电结果, 信息体：CAN地址、内部申请开始充电结果, 发出方：充电服务, 订阅方：负荷调度
       AddrType_InPeakApplyStopCharge,//主题：削峰填谷内部申请结束充电, 信息体：CAN地址, 发出方：负荷调度, 订阅方：充电服务
       AddrType_InPeakApplyStopCharge_Result,//主题：削峰填谷内部申请结束充电结果, 信息体：CAN地址、内部申请结束充电结果, 发出方：充电服务, 订阅方：负荷调度
    AddrType_InPeakApplyStopCharge_Result_Server,//主题：削峰填谷停止充电上传, 信息体：, 发出方：充电服务, 订阅方：通信
       AddrType_CmdCtrl_AckResult_Peak,//157遥控_充电控制申请ACK(充电申请响应结果) 削峰填谷使用。服务器订阅
    AddrType_CmdCtrl_AckFaile_Peak,//启动失败(充电申请响应结果) 削峰填谷使用。服务器订阅

    AddrType_ToPeakChargeCMD,//能效后台发来命令
    AddrType_ToPeakChargeCMD_Ack,//能效后台发来命令
    /********车牌号远程充电功能相关主题*****************/
    AddrType_CarLicenceApplyStartCharge,//主题六：车牌号外部申请开始充电, 信息体：CAN地址、车牌号、车牌号申请开始充电类型, 发出方：充电服务, 订阅方：服务器
    AddrType_CarLicenceApplyStartCharge_Result,//主题七：车牌号外部申请开始充电结果, 信息体：CAN地址、车牌号、车牌号申请开始充电结果, 发出方：服务器, 订阅方：充电服务
    AddrType_CarLicence,//主题十：车牌号, 信息体：CAN地址、车牌号, 发出方：实时数据处理模块, 订阅方：充电服务
    /**错峰充电(车牌号)功能相关主题**/
    AddrType_InCarLicenceApplyStartCharge,//主题：车牌号内部申请开始充电, 信息体：CAN地址、车牌号、车牌号申请开始充电类型, 发出方：负荷调度, 订阅方：充电服务
    AddrType_InCarLicenceApplyStartCharge_Result,//主题：车牌号内部申请开始充电结果, 信息体：CAN地址、车牌号、车牌号内部申请开始充电结果, 发出方：充电服务, 订阅方：负荷调度
    AddrType_InCarLicenceApplyStopCharge,//主题：车牌号内部申请结束充电, 信息体：CAN地址、车牌号, 发出方：负荷调度, 订阅方：充电服务
    AddrType_InCarLicenceApplyStopCharge_Result,//主题：车牌号内部申请结束充电结果, 信息体：CAN地址、车牌号、车牌号内部申请结束充电结果, 发出方：充电服务, 订阅方：负荷调度
    AddrType_CarLicenceApplyStopCharge,//主题八：车牌号外部申请结束充电, 信息体：CAN地址、车牌号, 发出方：充电服务, 订阅方：服务器
    AddrType_CarLicenceApplyStopCharge_Result,//主题九：车牌号外部申请结束充电结果, 信息体：CAN地址、车牌号、车牌号申请结束充电结果, 发出方：服务器, 订阅方：充电服务
    /********GPIO*****************/
	AddrType_RelayControl_Publish,//下发输出继电器控制指令。服务器发布，GPIO模块订阅
    AddrType_RelayControl_Result,//输出继电器控制指令返回结果。GPIO模块发布，服务器订阅
    AddrType_Alarm_Report,           //输出继电器控制指令返回结果。GPIO模块发布，服务器/串口屏订阅

     /********单双枪充电相关主题*****************/
    AddrType_CheckChargeManner_Success,
    AddrType_CheckChargeManner,
    AddrType_Response_Result,//数据项响应结果
    AddrType_Response_Result_IN,//内部数据项响应结果 充电服务发布 充电设备订阅
    AddrType_ChargeGunGroup_Info,		//下发多枪分组信息   服务器发布 充电服务订阅
    AddrType_ChargeGunGroup_Info_IN,  //内部多枪分组信息  充电服务发布，充电设备订阅
    AddrType_Change_ChargeGunGroup_Info,  //多枪分组信息更改显示需要调整  充电服务发布，显示设备订阅
    AddrType_VinApplyStartChargeImmed,//多枪点击屏幕VIN外部申请开始充电立即充电, 信息体：CAN地址、VIN、VIN申请开始充电类型, 发出方：屏幕, 订阅方：充电服务

	AddrType_RemoteEmergency_State,	//平台下发应急充电状态。服务器发布，充电服务订阅
	AddrType_RemoteEmergency_Result,//平台回应应急充电。服务器发布，充电服务订阅
	AddrType_LocalEmergency_Net,	//本地网络状态。服务器发布，充电服务订阅
	AddrType_LocalEmergency_State,	//CSCU上报应急充电状态。充电服务发布，服务器订阅
	AddrType_LocalEmergency_Result,	//CSCU回应平台应急充电。充电服务发布，服务器订阅
	AddrType_Emergency_Order, 		//紧急充电订单上报。充电服务发布，WebServer订阅
	AddrType_Emergency_Order_Result,//紧急充电订单上报结果。WebServer发布，充电服务订阅
    AddrType_Ecnomic_Order, 		//经济充电订单上报。充电服务发布，新协议订阅
	AddrType_QueueGroup_Info,		//轮充组信息
    AddrType_QueueGroup_State,		//排队状态带流水号

    /*******通信协议3.0********/
    AddrType_Order_State,			//订单状态, ChargeSergvice发布, 其他模块订阅, 信息体: CAN地址, 订单状态, GUID
    AddrType_Apply_Info_Data,       //申请信息。充电服务发布，服务器订阅。
    AddrType_Info_Data,             //信息数据。服务器发布，充电服务订阅。
    /*******能效计划********/
    AddrType_EnergyPlan_Signal_Burst,   //能效数据突发
    AddrType_EnergyPlanDevChange,   //能效数据节点变换, 发送CAN地址,内部ID,设备类型
    AddrType_EnergyPlanDevOffline,   //能效数据节点变换, 发送CAN地址,内部ID,设备类型
    AddrType_ES_Cab_Info,   //储能柜信息
    AddrType_ES_Bat_Info,   //储能柜电池信息
    AddrType_PH_Cab_Info,   //光伏柜信息
    AddrType_SC_Cab_Info,   //系统控制柜信息
    AddrType_FQ_Cab_Info,   //四象限柜子信息
    AddrType_CD_Cab_Info,   //充放电柜子信息
    AddrType_TD_Cab_Info,   //总配电柜信息
    AddrType_PO_Mod_Info,   //功率优化器信息
    AddrType_HY_Mod_Info,   //温湿度计信息
    AddrType_SI_Cab_Info,   //独立逆变器信息
    AddrType_ACDC_Mod_Info, //ACDC模块信息
    AddrType_DCDC_Mod_CD_Info,  //充放电柜下DCDC模块信息
    AddrType_DCDC_Mod_ES_Info,  //储能柜DCDC模块
    AddrType_EMS_Info,
    AddrType_TempContorlInfo,

    AddrType_TermCarLock,		//车位锁状态信息，车位锁模块发布

    AddrType_MagneticSwitch_State,   //门磁开关状态。GPIO发布

    AddrType_Ctrl_Module_EP,    //能效系统，模块控制主题, 能效服务器模块发布，充电设备模块订阅， 带模块ID， CAN地址， 控制指令
    AddrType_Set_Power_EP,      //能效系统，功率设置主题，能效服务器模块发布，充电设备模块订阅， 带模块ID， CAN地址， 功率值

    /********小票机功能相关主题*****************/
    AddrType_ApplyPrintTicket,  //225 申请打印小票一次, 信息体：CAN地址   发出方：充电服务, 订阅方：显示屏
    AddrType_MakePrintTicket,//执行打印小票, 信息体：CAN地址   发出方：显示屏, 订阅方：小票机
    AddrType_MakePrintTicketResult,    //执行打印小票结果, 信息体：CAN地址 , 发出方：小票机, 订阅方：显示屏
	//南京3.0协议相关
	AddrType_ChargeOrder_State,	//订单状态变化
	AddrType_LogicState,		//突发逻辑工作状态变化
	AddrType_Sync_Account,		//同步账户信息
	AddrType_Power_Curve,		//功率曲线下发
	AddrType_Offline_Time,		//离网时间
	AddrType_CSCU_Alarm,		//集控器告警信息 3.0充电服务器发布，运行监控服务器订阅
    AddrType_BillSendReult,    //未确认订单重传结果

    AddrType_ActiveDefend_StopCharge, //主动防护停止充电, 信息体: can地址  主动防御模块发布, 充电服务模块订阅
    AddrType_ActiveDefend_Alarm, //主动防护告警, 信息体: 主动防御告警码  主动防御模块发布, 服务器模块订阅

    AddrType_JiGuiTempHumidty,          //机柜温度湿度传感器结果  //smm add
    AddrType_Qiang,                      //枪类型的地址定义   //smm add
	//网络地址获取消息++++++++++++++dht
	AddrType_Comm_Addr,			//通信地址获取结果	网络地址获取模块发布，通信模块订阅
	AddrType_Comm_Addr_Request,	//通信地址获取请求	通信模块发布，网络地址获取模块订阅
	//------------------------------dht

}InfoAddrType;

//typedef const int InfoAddr;
/***********************通用信息体******************************/
InfoAddr Addr_CanID_Comm = 0x230002;//CAN地址
InfoAddr Addr_DevID_DC_Comm = 0x230AFF;//直流柜内部设备地址
InfoAddr Addr_EnergyPlan_ID_Comm = 0x230B00;    //能效设备地址

InfoAddr Addr_NetState_Comm = 0x23034F;//集控和云平台的联网状态
InfoAddr Addr_Default_Comm = 0x000000;//信息体默认值
InfoAddr Addr_ServerType_Comm = 0x400001;//服务器类型, 0:云平台 1:本地服务器
InfoAddr Addr_Ammeter_ID = 0x205001;//电表地址
InfoAddr Addr_Ammeter_Type = 0x205002;//电表类型

/***********************升级/日志相关信息体******************************/
InfoAddr Addr_Cmd_Source = 0x800001;			//1/网络  2/U盘
InfoAddr Addr_Cmd_Master = 0x800002;			//命令主类型  1/升级  2/上传
InfoAddr Addr_Cmd_Slave = 0x800003;				//命令层类型  1/日志  2/配置文件  3/升级程序 4/升级模块
InfoAddr Addr_Update_Upload_Param = 0x800004;	//传入参数字符串形式

InfoAddr Addr_Back_Result = 0x800005;			//返回结果 1 成功 2 失败
InfoAddr Addr_Checked_USB = 0x800006;			//检测到U盘挂载

InfoAddr Addr_TicketPrint_Result = 0x800007;   //小票机打印返回结果

/***********************CAN升级相关信息体******************************/
InfoAddr Addr_CANUpdateResult = 0x230014;			//CAN设备升级结果

/***********************冻结电量******************************/
InfoAddr Addr_StartEnergy_FrozenEnergy = 0x206001;//冻结电量起始时间点总电能
InfoAddr Addr_EndEnergy_FrozenEnergy = 0x230000;//冻结电量结束时间点总电能
InfoAddr Addr_FrozenEnergy_Result = 0x2403BD;//冻结电量返回结果
InfoAddr Addr_PeakShaving_Setting_Result = 0x2403CD;//削峰填谷平台确认
/***********************升级相关数据项******************************/
//CSCU相关
InfoAddr Addr_CSCUVer_CtrlInfo = 0x230007;//子站软件版本号

InfoAddr Addr_CSCULogUploadPath_CtrlInfo = 0x230009;//CSCU日志上传路径
InfoAddr Addr_CSCULogUploadResult_CtrlInfo = 0x230011;//CSCU日志上传结果
InfoAddr Addr_CSCUConfigUploadPath_CtrlInfo = 0x230043;//CSCU配置文件上传路径
InfoAddr Addr_CSCUConfigUploadResult_CtrlInfo = 0x230044;//CSCU配置文件上传结果

InfoAddr Addr_CSCUConfigUpdatePath_CtrlInfo = 0x23000E;//CSCU配置文件升级路径
InfoAddr Addr_CSCUConfigUpdateResult_CtrlInfo = 0x230012;//CSCU配置文件升级结果
InfoAddr Addr_CSCUProgramUpdatePath_CtrlInfo = 0x23000F;//CSCU主程序升级路径
InfoAddr Addr_CSCUProgramUpdateResult_CtrlInfo = 0x230013;//CSCU主程序升级结果

//充电模块相关
InfoAddr Addr_ChargerVer_CtrlInfo = 0x230008;//充电模块软件版本号
InfoAddr Addr_ChargerConfigUpdatePath_CtrlInfo = 0x230048;//充电模块配置文件升级路径
InfoAddr Addr_ChargerConfigUpdateResult_CtrlInfo = 0x230049;//充电模块配置文件升级结果
InfoAddr Addr_ChargerProgramUpdatePath_CtrlInfo = 0x230010;//充电模块配置文件升级路径
InfoAddr Addr_ChargerProgramUpdateResult_CtrlInfo = 0x230014;//充电模块配置文件升级结果

/***********************遥调相关数据项******************************/
InfoAddr Addr_AdjustCurrent_Adj = 0x230006;//调整充电电流大小
InfoAddr Addr_AdjustCurrent_Ack = 0x230100;//调整充电电流响应结果

InfoAddr Addr_AdjustPower_Adj = 0x23005A;//调整充电功率大小
InfoAddr Addr_AdjustPower_Ack = 0x23005B;//调整充电功率响应结果

InfoAddr Addr_AmeterAddr_Adj = 0x205001;//第一块电表地址
InfoAddr Addr_AmeterInfo_Adj = 0x205021;//第一块电表数据
InfoAddr Addr_AmeterEnable_Adj = 0x2306BE;//第一块电表使能
InfoAddr Addr_AmeterVoltageRate_Adj = 0x2306BF;//第一块电表电压变比
InfoAddr Addr_AmeterCurrentRate_Adj = 0x2306C0;//第一块电表电流变比
InfoAddr Addr_AmeterProtocol_Adj = 0x2306C1;//第一块电表协议类型
InfoAddr Addr_AmeterData_Adj = 0x2306C2;//第一块电表数据类型
InfoAddr Addr_AmeterSetResult_Adj = 0x23073B;//电表设置结果

/***********************远程抄表数据项******************************/
InfoAddr Addr_RemoteAmeterAddr_Adj = 0x250002;//第一块电表地址
InfoAddr Addr_RemoteAmeterType_Adj = 0x250042;  //电表数据类型地址
InfoAddr Addr_RemoteAmeterReadTime_Adj = 0x250043;  //电表数据抄读时间
InfoAddr Addr_RemoteAmeterCurrentInfo_1_Adj = 0x250049 ;//当前电能及最大需量数据-（当前)正向有功总电能
InfoAddr Addr_RemoteAmeterCurrentInfo_2_Adj = 0x25004A;//当前电能及最大需量数据-（当前)正向有功费率 1 电能 尖
InfoAddr Addr_RemoteAmeterCurrentInfo_3_Adj = 0x25004B;//当前电能及最大需量数据-（当前)正向有功费率 2 电能 峰
InfoAddr Addr_RemoteAmeterCurrentInfo_4_Adj = 0x25004C;//当前电能及最大需量数据-（当前)正向有功费率 3 电能 平
InfoAddr Addr_RemoteAmeterCurrentInfo_5_Adj = 0x25004D;//当前电能及最大需量数据-（当前)正向有功费率 4 电能 谷
InfoAddr Addr_RemoteAmeterCurrentInfo_6_Adj = 0x25004E;//当前电能及最大需量数据-（当前)反向有功总电能
InfoAddr Addr_RemoteAmeterCurrentInfo_7_Adj = 0x250053;//当前电能及最大需量数据-（当前)正向无功总电能
InfoAddr Addr_RemoteAmeterCurrentInfo_8_Adj = 0x250054;//当前电能及最大需量数据-（当前)反向无功总电能
InfoAddr Addr_RemoteAmeterCurrentInfo_9_Adj = 0x250059;//当前电能及最大需量数据-（当前)正向有功总最大需量及发生 时间
InfoAddr Addr_RemoteAmeterCurrentInfo_10_Adj = 0x25005A;//当前电能及最大需量数据-（当前)反向有功总最大需量及发生 时间
InfoAddr Addr_Remote_current_reactive_absorb_energy1_Term = 0x250055;						   //(当前)正向无功总电能-(当前)第一象限电能
InfoAddr Addr_Remote_current_reactive_absorb_energy2_Term = 0x250056;						   //(当前)正向无功总电能-(当前)第二象限电能
InfoAddr Addr_Remote_current_reactive_liberate_energy1_Term = 0x250057;					      //(当前)反向无功总电能-(当前)第三象限电能
InfoAddr Addr_Remote_current_reactive_liberate_energy2_Term = 0x250058;					      //(当前)反向无功总电能-(当前)第三象限电能


InfoAddr Addr_RemoteAmeterHourFreezeInfo_1_Adj = 0x25050B ;//整点冻结电能及最大需量数据-（整点冻结)时间
InfoAddr Addr_RemoteAmeterHourFreezeInfo_2_Adj = 0x25050C;//整点冻结电能及最大需量数据-（整点冻结)正向有功总电能
InfoAddr Addr_RemoteAmeterHourFreezeInfo_3_Adj = 0x25050D;//整点冻结电能及最大需量数据-（整点冻结)反向有功总电能

InfoAddr Addr_RemoteAmeterDayFreezeInfo_1_Adj = 0x25023F ;//日冻结电能及最大需量数据-（日冻结)时间
InfoAddr Addr_RemoteAmeterDayFreezeInfo_2_Adj = 0x250240;//日冻结电能及最大需量数据-（日冻结)正向有功电能数据 尖峰平谷
InfoAddr Addr_RemoteAmeterDayFreezeInfo_3_Adj = 0x250241;//日冻结电能及最大需量数据-（日冻结)反向有功电能数据
InfoAddr Addr_RemoteAmeterDayFreezeInfo_4_Adj = 0x250242;//日冻结电能及最大需量数据-（日冻结)正向无功电能数据
InfoAddr Addr_RemoteAmeterDayFreezeInfo_5_Adj = 0x250243;//日冻结电能及最大需量数据-（日冻结)反向无功电能数据
InfoAddr Addr_RemoteAmeterDayFreezeInfo_6_Adj = 0x250248;//日冻结电能及最大需量数据-（日冻结)正向有功总最大需量及发生 时间
InfoAddr Addr_RemoteAmeterDayFreezeInfo_7_Adj = 0x250249;//日冻结电能及最大需量数据-（日冻结)反向有功总最大需量及发生 时间
InfoAddr Addr_Remote_day_freeze_reactive_absorb_energy1_Term = 0x250244;				         //(日冻结)正向无功总电能-(日冻结)第一象限电能
InfoAddr Addr_Remote_day_freeze_reactive_absorb_energy2_Term = 0x250245;				     //(日冻结)正向无功总电能-(日冻结)第二象限电能
InfoAddr Addr_Remote_day_freeze_reactive_liberate_energy1_Term = 0x250246;			         //(日冻结)反向无功总电能-(日冻结)第三象限电能
InfoAddr Addr_Remote_day_freeze_reactive_liberate_energy2_Term = 0x250247;			         //(日冻结)反向无功总电能-(日冻结)第三象限电能

InfoAddr Addr_RemoteAmeterSettlementInfo_1_Adj = 0x250070 ;//结算日电能及最大需量数据-（结算日)正向有功总电能
InfoAddr Addr_RemoteAmeterSettlementInfo_2_Adj = 0x250071;//结算日电能及最大需量数据-（结算日)正向有功费率 1 电能 尖
InfoAddr Addr_RemoteAmeterSettlementInfo_3_Adj = 0x250072;//结算日电能及最大需量数据-（结算日)正向有功费率 2 电能 峰
InfoAddr Addr_RemoteAmeterSettlementInfo_4_Adj = 0x250073;//结算日电能及最大需量数据-（结算日)正向有功费率 3 电能 平
InfoAddr Addr_RemoteAmeterSettlementInfo_5_Adj = 0x250074;//结算日电能及最大需量数据-（结算日)正向有功费率 4 电能 谷
InfoAddr Addr_RemoteAmeterSettlementInfo_6_Adj = 0x250075;//结算日电能及最大需量数据-（结算日)反向有功总电能
InfoAddr Addr_RemoteAmeterSettlementInfo_7_Adj = 0x25007A;//结算日电能及最大需量数据-（结算日)正向无功总电能
InfoAddr Addr_RemoteAmeterSettlementInfo_8_Adj = 0x25007B;//结算日电能及最大需量数据-（结算日)反向无功总电能
InfoAddr Addr_RemoteAmeterSettlementInfo_9_Adj = 0x250080;//结算日电能及最大需量数据-（结算日)正向有功总最大需量及发生 时间
InfoAddr Addr_RemoteAmeterSettlementInfo_10_Adj = 0x250081;//结算日电能及最大需量数据-（结算日)反向有功总最大需量及发生 时间
InfoAddr Addr_Remote_settlement_reactive_absorb_energy1_Term = 0x25007C;				     //(结算日)正向无功总电能-(结算日)第一象限电能
InfoAddr Addr_Remote_settlement_reactive_absorb_energy2_Term = 0x25007D;				     //(结算日)正向无功总电能-(结算日)第二象限电能
InfoAddr Addr_Remote_settlement_reactive_liberate_energy1_Term = 0x25007E;			         //(结算日)反向无功总电能-(结算日)第三象限电能
InfoAddr Addr_Remote_settlement_reactive_liberate_energy2_Term = 0x25007F;			         //(结算日)反向无功总电能-(结算日)第三象限电能
/***********************远程抄表数据项END******************************/

InfoAddr Addr_TermName_Adj = 0x230045;//获取子站名称
InfoAddr Addr_TermIndex_Adj = 0x230046;//CAN地址和终端编号对应关系

InfoAddr Addr_StationEnvTemp = 0x205015;//子站环温度数据
InfoAddr Addr_StationEnvHumi = 0x205016;//子站环湿度测数据


InfoAddr Addr_CarLock = 0x23001A;//车位锁开关
InfoAddr Addr_CarLockOpen_Result = 0x23001B;//车位锁开锁响应结果
InfoAddr Addr_CarLockClose_Result = 0x23001C;//车位锁关锁响应结果
InfoAddr Addr_MagneticSwitch_Status = 0x23001D;//门磁开关状态 0=关闭门 1=打开门 add by weiwb

InfoAddr Addr_CarLockID = 0x2406EF;//车位锁地址
InfoAddr Addr_CarLockCmd = 0x2406F0;//车位锁控制指令
InfoAddr Addr_CarLockCmd_Result = 0x2406F1;//车位锁响应结果
InfoAddr Addr_CarLockStates = 0x2406F2;//车位锁状态
InfoAddr Addr_ParkingStates = 0x2406F3;//车位占用状态
InfoAddr Addr_CarLock_SensorFaultStates = 0x2406F4;//车位锁传感器故障状态
InfoAddr Addr_CarLock_StructureFaultStates = 0x2406F5;//车位锁结构或电机故障状态
InfoAddr Addr_CarLockParaSetCmd = 0x2406F6;//车位锁参数设置

InfoAddr Addr_Reboot_Result = 0x230028;//重启执行结果



//负荷调度
InfoAddr Addr_PredictTime_Burst = 0x23002F;//充电预估完成时间
InfoAddr Addr_MaxLoad = 0x230030;//设置电网最大允许负荷
InfoAddr Addr_MaxLoad_Result = 0x230031;//设置电网最大允许负荷返回结果
//InfoAddr Addr_AjuChargeCurrent = 0x230006;//调整充电电流
InfoAddr Addr_ModuleFree = 0x230036;//充电是否有空闲
InfoAddr Addr_QueueInfo_Burst = 0x230037;//突发充电排队信息
InfoAddr Addr_QueueInfo_Publish = 0x230038;//下发充电排队信息
InfoAddr Addr_ChargeDetail = 0x230040;//充电总费用和充电明细
InfoAddr Addr_PredictTime = 0x230042;//充电预估完成时间
InfoAddr Addr_SmartChargeSet_Result = 0x23067A;//错峰充电设置返回结果(本地设置)


//充电设备配置相关信息
InfoAddr Addr_ChargePriority = 0x230029;//设置充电优先等级
InfoAddr Addr_ChargePriority_Result = 0x23002A;//设置充电优先等级返回结果
InfoAddr Addr_ChargeFinishTime = 0x23002B;//设置充电完成时间
InfoAddr Addr_ChargeFinishTime_Result = 0x23002C;//设置充电完成时间返回结果
InfoAddr Addr_AuxPower = 0x230034;//设置辅助电源类型
InfoAddr Addr_AuxPower_Result = 0x230035;//设置辅助电源类型返回结果
InfoAddr Addr_ChargeMode = 0x23002D;//设置充电模式
InfoAddr Addr_ChargeMode_Result = 0x23002E;//设置充电模式返回结果
InfoAddr Addr_GroupPolicy = 0x230032;//设置群充策略
InfoAddr Addr_GroupPolicy_Result = 0x230033;//设置群充策略返回结果

InfoAddr Addr_300kw_Group = 0x2309E7;//双枪组别
InfoAddr Addr_300kw_PairCanID = 0x2309E8;//配对的充电终端的CAN地址
InfoAddr Addr_300kw_WorkMode = 0x2309E9;//工作模式设置
InfoAddr Addr_300kw_ChangeMode = 0x2309EA;//单充/双充切换模式类型
InfoAddr Addr_300kw_ChangeTime = 0x2309EB;//单充/双充自动切换时间段设置
InfoAddr Addr_300kw_Result = 0x2309EC;//双枪300kw功能参数配置结果

/**************************************************************/
/*******************  设备管理定义信息体  *************************/
/**************************************************************/

//参数设置通用选项
InfoAddr Addr_ArgNo = 0x230A6A0;    //配置参数编号
InfoAddr Addr_ArgData = 0x230A6A1;    //配置参数内容
InfoAddr Addr_ArgSetResult = 0x230A6A2; //配置参数返回结果

//告警
InfoAddr Addr_AbnormalActiveEnergy = 0x230148;//异常电度数告警

//直流柜报警相关信息体(自研直流设备专有)
InfoAddr Addr_DCAlarmNo = 0x23024A1;    //直流柜告警序号
InfoAddr Addr_DCcabFaultCode = 0x23024A2;//直流柜故障码(自研直流设备专有代码)
InfoAddr Addr_DCcabMinPDU_ID = 0x23024A3;//CCU下最小PDU地址代码
InfoAddr Addr_DCcabMaxPDU_ID = 0x23024A4;//CCU下最大PDU地址代码
InfoAddr Addr_DCcabFaultState = 0x23024A5;//直流柜故障状态 (0x01:存在, 0x00: 消失)

InfoAddr Addr_DCAlarmPoint = 0x23024C1;//直流柜故障点记录数据
InfoAddr Addr_DCFaultRecord = 0x23024C2;//故障录波数据记录数据
InfoAddr Addr_QueryParam_DM = 0x230501;//获取参数
InfoAddr Addr_QueryParamAck_DM = 0x230502;//获取参数结果返回

//主动防护功能相关参数配置信息
InfoAddr Addr_BalanceCurCoe_AP = 0x230A6A;//均衡阶段电流系数
InfoAddr Addr_BalanceTime_AP = 0x230A6B;//均衡时间

InfoAddr Addr_TempTh_AP = 0x230A6C;//温升阈值
InfoAddr Addr_TempRiseEnsureTime_AP = 0x230A6D;//温升确认时间
InfoAddr Addr_NoTempProtect_AP = 0x230A6E;//禁止热失控防护

InfoAddr Addr_SingleOVTh_AP = 0x230A6F;//单体过压阈值
InfoAddr Addr_SingleOVEnsureTime_AP = 0x230A70;//单体过压确认时间
InfoAddr Addr_NoSingleOVP_AP = 0x230A71;//禁止单体过压防护

InfoAddr Addr_TotalOVTh_AP = 0x230A72;//整体过压阈值
InfoAddr Addr_TotalOVEnsureTime_AP = 0x230A73;//整体过压确认时间
InfoAddr Addr_NoTotalOVP_AP = 0x230A74;//禁止整体过压防护

InfoAddr Addr_OCTh_AP = 0x230A75;//过流阈值
InfoAddr Addr_OCEnsureTime_AP = 0x230A76;//过流确认时间
InfoAddr Addr_NoOCP_AP = 0x230A77;//禁止过流防护

InfoAddr Addr_OTTh_AP = 0x230A78;//过温阈值
InfoAddr Addr_OTEnsureTime_AP = 0x230A79;//过温确认时间
InfoAddr Addr_NoOTP_AP = 0x230A7A;//禁止过温防护

InfoAddr Addr_BTTh_AP = 0x230A7B;//低温保护阈值
InfoAddr Addr_BTEnsureTime_AP = 0x230A7C;//低温确认时间
InfoAddr Addr_NoBTP_AP = 0x230A7D;//禁止低温防护

InfoAddr Addr_BMSRAVTh_AP = 0x230A7E;//继电器粘连电压阈值
InfoAddr Addr_BMSRAVEnsureTime_AP = 0x230A7F;//继电器粘连确认时间
InfoAddr Addr_NoBMSRAVP_AP = 0x230A80;//禁止继电器粘连防护

InfoAddr Addr_BMSRBOVTh_AP = 0x230A81;//继电器粘连电压阈值
InfoAddr Addr_BMSRBOCTh_AP = 0x230A82;//继电器开路电流阈值
InfoAddr Addr_BMSRBOEnsureTime_AP = 0x230A83;//继电器开路确认时间
InfoAddr Addr_NoBMSRBOP_AP = 0x230A84;//禁止继电器粘连防护

InfoAddr Addr_OCJC_AP = 0x230A85;//过充判断系数设置
InfoAddr Addr_OCERefValue_AP = 0x230A86;//过充判断电量参考值
InfoAddr Addr_OCEEnsureTime_AP = 0x230A87;//过充判断确认时间
InfoAddr Addr_NoOverChargeP_AP = 0x230A88;//禁止过充防护

InfoAddr Addr_BMSDataRepetTime_AP = 0x230A89;//BMS数据重复时间
InfoAddr Addr_NoBMSDataRepetProtect_AP = 0x230A8A;//禁止BMS数据重复防护
InfoAddr Addr_NoBMSCheck_AP = 0x230A8B;//禁止BMS数据校验告警检测

InfoAddr Addr_OVTh_LNCM_AP = 0x230A8C;//三元锂电池过压阈值
InfoAddr Addr_OVTh_LTO_AP = 0x230A8D;//钛酸锂电池过压阈值
InfoAddr Addr_OVTh_LMO_AP = 0x230A8E;//锰酸锂电池过压阈值
InfoAddr Addr_OTTh_LNCM_AP = 0x230A8F;//三元锂电池过温阈值
InfoAddr Addr_OTTh_LTO_AP = 0x230A90;//钛酸锂电池过温阈值
InfoAddr Addr_OTTh_LMO_AP = 0x230A91;//锰酸锂电池过温阈值

//柔性充电功能相关参数配置信息
InfoAddr Addr_SOCCoefficient_FC = 0x230ABA;//电流系数随SOC变化设置
InfoAddr Addr_TempCoefficient_FC = 0x230ABB;//电流系数随温度变化设置
InfoAddr Addr_TimeCoefficient_FC = 0x230ABC;//电流系数随充电时长变化设置

//通用静态参数设置信息
InfoAddr Addr_ElecLockType_GSA = 0x23095B;//电子锁类型
InfoAddr Addr_VINEnable_GSA = 0x23095C;//VIN使能
InfoAddr Addr_AuxPowerType_GSA = 0x23095D;//设置辅源类型
InfoAddr Addr_ElecLockEnable_GSA = 0x23095E;//电子锁使能
InfoAddr Addr_MaxGunCur_GSA = 0x23095F;//设置枪头最大电流
InfoAddr Addr_BMSProTypeSet_GSA = 0x230960;//BMS新老国标设置
InfoAddr Addr_TermIDSet_GSA = 0x230961;//终端ID设置

//通用动态参数设置信息
InfoAddr Addr_WorkMode_GDA = 0x2309A1;//充电终端工作模式
InfoAddr Addr_WorkState_GDA = 0x2309A2;//充电终端工作状态(群充轮充控制模式)
InfoAddr Addr_GroupStrategy_GDA = 0x2309A3;//群充策略
InfoAddr Addr_PriorityLevel_GDA = 0x2309A4;//充电优先等级
InfoAddr Addr_ChargeEndTime_GDA = 0x2309A5;//充电完成时间

//CCU参数设置信息
InfoAddr Addr_ArgSetResult_CCUArg = 0x230B01;//CCU参数设置返回结果
InfoAddr Addr_CCUidSet_CCUArg = 0x230B02;//设置CCU的ID
InfoAddr Addr_TermStartID_CCUArg = 0x230B03;//设置直流柜终端的起始地址
InfoAddr Addr_CabMaxPower_CCUArg = 0x230B04;//直流柜最大输出功率

//兼容
InfoAddr Addr_SetCanIDRange = 0x23084F;//CAN地址范围
InfoAddr Addr_300kw_CanID = 0x23084F;//CAN地址范围
InfoAddr Addr_Relay_ID = 0x2309ED;//输出继电器编号
InfoAddr Addr_Relay_CmdType = 0x2309EE;//0断开／1闭合命令
InfoAddr Addr_Relay_HoldType = 0x2309F0;//控制类型
InfoAddr Addr_Relay_Result = 0x2309EF;//下发输出继电器控制指令返回结果
InfoAddr Addr_Alarm_Type1 = 0x205015;//报警器１
InfoAddr Addr_Alarm_Type2 = 0x205016;//报警器２

//刷卡远程充电
InfoAddr Addr_CardAccount = 0x23001D;//刷卡卡号
InfoAddr Addr_CardType = 0x230638;//刷卡卡类型
InfoAddr Addr_CardApplyChargeType = 0x23001F;//刷卡申请充电类型
InfoAddr Addr_CardChargeTypeValue = 0x230020;//充电类型值
InfoAddr Addr_CardApplyCharge_Result = 0x230021;//刷卡申请充电返回结果-------远程申请充电结果
InfoAddr Addr_CardAccountType = 0x230022;//刷卡业务查询账户信息类型
InfoAddr Addr_CardUserName = 0x230023;//刷卡业务用户姓名
InfoAddr Addr_CardAccountList = 0x230024;//刷卡业务账户列表及余额
InfoAddr Addr_CardPolicy = 0x230025;//收费策略
InfoAddr Addr_CardChargeInfo = 0x230026;//用户当前充电信息
InfoAddr Addr_CardStopCharge_Result = 0x230027;//刷卡申请终止充电返回结果-----远程申请结束充电结果
InfoAddr Addr_InApplyStartCharge_Result = 0x100001;//CSCU内部申请开始充电返回结果 +++++++++++(待定义)
InfoAddr Addr_InApplyStopCharge_Result = 0x100002;//CSCU内部申请结束充电返回结果 +++++++++++(待定义)

//VIN,车牌号远程充电
//VIN见BMS
InfoAddr Addr_ZigbeeUserID = 0x230016;//ZIGBEE用户ID
InfoAddr Addr_ZigbeeChargeType = 0x230017;//zigbee申请充电类型
InfoAddr Addr_ZigbeeStopChargeResult = 0x230018;//zigbee申请结束充电结果
InfoAddr Addr_ZigbeeStartChargeResult = 0x230019;//zigbee申请开始充电结果

InfoAddr Addr_VINApplyStartChargeType = 0x100003;//VIN申请开始充电类型 +++++++++++(待定义)
InfoAddr Addr_VINApplyStartChargeType_Result = 0x100004;//VIN外部申请开始充电结果 +++++++++++(待定义)
InfoAddr Addr_VINApplyStopChargeType_Result = 0x100005;//VIN外部申请结束充电结果 +++++++++++(待定义)
InfoAddr Addr_CarLicense = 0x100006;//车牌号 +++++++++++(待定义)
InfoAddr Addr_CarLicenseApplyStartChargeType = 0x100007;//车牌号 申请充电类型 +++++++++++(待定义)
InfoAddr Addr_CarLicenseApplyStartChargeType_Result = 0x100008;//车牌号 申请开始充电结果 +++++++++++(待定义)
InfoAddr Addr_CarLicenseApplyStopChargeType_Result = 0x100009;//车牌号 申请结束充电结果 +++++++++++(待定义)

InfoAddr Addr_InVINApplyStartChargeType_Result = 0x10000A;//VIN内部申请开始充电结果 +++++++++++(待定义)
InfoAddr Addr_InVINApplyStopChargeType_Result = 0x10000B;//VIN内部申请结束充电结果 +++++++++++(待定义)
InfoAddr Addr_InCarLicenseApplyStartChargeType_Result = 0x10000C;//车牌号内部申请开始充电结果 +++++++++++(待定义)
InfoAddr Addr_InCarLicenseApplyStopChargeType_Result = 0x10000D;//车牌号内部申请结束充电结果 +++++++++++(待定义)

InfoAddr Addr_VINViaApplyStartCharge_Result = 0x10000E;//VIN后6位开始充电结果

//参数变化 信息体
InfoAddr Addr_Param_Change = 0x900001;			//参数变化

//应急充电相关
InfoAddr Addr_Remote_Emergency = 0x2402A7;			//平台应急充电状态 0xFF：正常 0x01:故障
InfoAddr Addr_Local_Emergency = 0x2402A8;			//本地应急充电状态 0xFF：应急 0x01：正常
InfoAddr Addr_QueueGroup_Info = 0x2402AB;			//第一组轮充组信息
InfoAddr Addr_QueueGroup_Result = 0x2402BD;			//下发轮充组保存结果
InfoAddr Addr_Emergency_Queue = 0x2300BE;			//应急充电排队信息
InfoAddr Addr_Emergency_Enable = 0x2402A9;			//平台配置本地应急充电开关 0xFF：开启 0x1：关闭
InfoAddr Addr_Emergency_Enable_Result = 0x2402AA;	//本地应急充电开关配置结果 0xFF：成功 0x1：失败
InfoAddr Addr_Emergency_Duration = 0x2402BF;		//应急充电使用时间
InfoAddr Addr_Emergency_OrderCount = 0x2402C0;		//应急充电订单数
InfoAddr Addr_Emergency_CheckTime = 0x2402C1;		//应急充电断网检测时间
InfoAddr Addr_Emergency_VinAuth = 0x2402C2;			//应急充电VIN鉴权
InfoAddr Addr_Emergency_CarAuth = 0x2402C3;			//应急充电车牌号鉴权
InfoAddr Addr_Emergency_CardAuth = 0x2402C4;		//应急充电卡号鉴权
InfoAddr Addr_Emergency_QueueGun = 0x2402C5;		//应急充电插枪触发轮充
InfoAddr Addr_Emergency_QueueCard = 0x2402C6;		//应急充电刷卡触发轮充
InfoAddr Addr_Emergency_QueueCar = 0x2402C7;		//应急充电车辆信息触发轮充
InfoAddr Addr_Emergency_Setting_Result = 0x2402C8;  //应急充电配置结果

//云平台下发服务器列表
InfoAddr Addr_First_Server = 0x23055C;  			//服务器列表第一个配置
InfoAddr Addr_First_Server_Result = 0x23055D;  		//第一个配置结果
InfoAddr Addr_Second_Server = 0x23055E;  			//服务器列表第二个配置
InfoAddr Addr_Second_Server_Result = 0x23055F;  	//第二个配置结果
InfoAddr Addr_Third_Server = 0x230560;  			//服务器列表第三个配置
InfoAddr Addr_Third_Server_Result = 0x230561;  		//第三个配置结果

//能效计划
InfoAddr Addr_TmpControlID = 0xAA0000;
InfoAddr Addr_TempA =0xAA0001;//温控仪
InfoAddr Addr_TempB =0xAA0002;
InfoAddr Addr_TempC =0xAA0003;
InfoAddr Addr_TempD =0xAA0004;
InfoAddr Addr_TempE =0xAA0005;
InfoAddr Addr_TempF =0xAA0006;
InfoAddr Addr_FaultAlarm =0xAA0007;
InfoAddr Addr_OverTempAlarm =0xAA0008;
InfoAddr Addr_OverTempTrip=0xAA0009;
InfoAddr Addr_FanControl =0xAA000A;

InfoAddr Addr_EnergyPlan_Dev = 0xBFFFFF;
InfoAddr Addr_DevID_EnergyPlan = 0xC00000;
InfoAddr Addr_PGN_ID = 0xC0000A;//

InfoAddr Addr_ACbreaker_Cabinet2 = 0xC00001;
InfoAddr Addr_DCbreaker_Cabinet2 = 0xC00002;
InfoAddr Addr_ACbreaker_Cabinet3 = 0xC00003;
InfoAddr Addr_DCbreaker_Cabinet3 = 0xC00004;
InfoAddr Addr_surgeFeedback_Cabinet2 = 0xC00005;
InfoAddr Addr_fireExtinguisher_Cabinet2 = 0xC00006;
InfoAddr Addr_fireExtinguisher_Cabinet3 = 0xC00007;

InfoAddr Addr_ACbreaker_Inverter = 0xC10001;
InfoAddr Addr_DCbreaker_Inverter = 0xC10002;
InfoAddr Addr_surgeFeedback_Cabinet2_Inverter = 0xC10003;
InfoAddr Addr_fireExtinguisher_Cabinet3_Inverter = 0xC10004;


InfoAddr Addr_humitureID =0xDF0000;
InfoAddr Addr_envTemperature = 0xDF0001;
InfoAddr Addr_envHumidity = 0xDF0002;

InfoAddr Addr_smokeSensor_lowVolIn	= 0xE20001;
InfoAddr Addr_frameFeedback	= 0xE20002;
InfoAddr Addr_minorLoadbreaker_630A1= 0xE20003;
InfoAddr Addr_minorLoadbreaker_630A2= 0xE20004;
InfoAddr Addr_smokeSensor_lowVolOut	= 0xE20005;
InfoAddr Addr_minorLoadbreaker_400A1= 0xE20006;
InfoAddr Addr_minorLoadbreaker_400A2= 0xE20007;
InfoAddr Addr_minorLoadbreaker_400A3= 0xE20008;
InfoAddr Addr_acdcBreaker		= 0xE20009;
InfoAddr Addr_importBreaker1		= 0xE2000A;
InfoAddr Addr_importBreaker2		= 0xE2000B;
InfoAddr Addr_emergncyStop	= 0xE2000C;

InfoAddr Addr_mainModuleSwitchCMD		= 0xE2000D;
InfoAddr Addr_mainLoad_ACDCModuleSwitchCMD		= 0xE2000E;
InfoAddr Addr_storageUnit1_DCDCModuleSwitchCMD		= 0xE2000F;
InfoAddr Addr_storageUnit2_DCDCModuleSwitchCMD	= 0xE20010;
InfoAddr Addr_Grid_State_EMS = 0x320011;


InfoAddr Addr_tmpEMSHighVol = 0x530001;
InfoAddr Addr_humityEMSHighVol = 0x530002;
InfoAddr Addr_tmpEMSLowVol = 0x530003;
InfoAddr Addr_humityEMSLowVol = 0x530004;
InfoAddr Addr_tmpEMSLowVol_out = 0x540001;
InfoAddr Addr_humityEMSLowVol_out = 0x540002;

InfoAddr Addr_moduleID = 0xCF0001;
InfoAddr Addr_vol_U = 0xCF0002;
InfoAddr Addr_cur_U = 0xCF0003;
InfoAddr Addr_vol_V = 0xCF0004;
InfoAddr Addr_cur_V = 0xCF0005;
InfoAddr Addr_vol_W = 0xCF0006;
InfoAddr Addr_cur_W = 0xCF0007;
InfoAddr Addr_frequency = 0xCF0008;
InfoAddr Addr_sysActivePower = 0xCF0009;
InfoAddr Addr_sysReActivePower = 0xCF000A;
InfoAddr Addr_sysApparentPower = 0xCF000B;
InfoAddr Addr_PF = 0xCF000C;
InfoAddr Addr_DCpositiveCur = 0xCF000D;
InfoAddr Addr_DCnegativeCur = 0xCF000E;
InfoAddr Addr_DCpositiveBusBarVol = 0xCF000F;
InfoAddr Addr_DCnegativeBusBarVol = 0xCF0010;
InfoAddr Addr_DCbilateralBusBarVol = 0xCF0011;
InfoAddr Addr_DCpower = 0xCF0012;
InfoAddr Addr_devStatus0 = 0xCF0013;
InfoAddr Addr_warningStatus0 = 0xCF0014;
InfoAddr Addr_warningStatus1 = 0xCF0015;
InfoAddr Addr_warningStatus2 = 0xCF0016;
InfoAddr Addr_faultStatus0 = 0xCF0017;
InfoAddr Addr_faultStatus1 = 0xCF0018;
InfoAddr Addr_faultStatus2 = 0xCF0019;
InfoAddr Addr_faultStatus3 = 0xCF001A;
InfoAddr Addr_faultStatus4 = 0xCF001B;
InfoAddr Addr_faultStatus5 = 0xCF001C;
InfoAddr Addr_faultStatus6 = 0xCF001D;
InfoAddr Addr_faultStatus7 = 0xCF001E;
InfoAddr Addr_faultStatus8 = 0xCF001F;
InfoAddr Addr_faultStatus9 = 0xCF0020;
InfoAddr Addr_faultStatus10 = 0xCF0021;
InfoAddr Addr_faultStatus11 = 0xCF0022;
InfoAddr Addr_faultStatus12 = 0xCF0023;
InfoAddr Addr_faultStatus13 = 0xCF0024;
InfoAddr Addr_faultStatus14 = 0xCF0025;
InfoAddr Addr_faultStatus15 = 0xCF0026;
InfoAddr Addr_devStatus1 = 0xCF0027;
InfoAddr Addr_devStatus2 = 0xCF0028;

InfoAddr Addr_HWVersion = 0xCF0029;
InfoAddr Addr_HWVersion_high= 0xCF0029; //以后废弃不用
InfoAddr Addr_HWVersion_low= 0xCF002A;  //以后废弃不用

InfoAddr Addr_SWVersion = 0xCF002B;
InfoAddr Addr_SWVersion_high= 0xCF002B; //以后废弃不用
InfoAddr Addr_SWVersion_low= 0xCF002C;  //以后废弃不用
InfoAddr Addr_tmp_IGBT1= 0xCF002D;
InfoAddr Addr_tmp_IGBT2= 0xCF002E;
InfoAddr Addr_tmp_IGBT3= 0xCF002F;
InfoAddr Addr_tmp_IGBT4= 0xCF0030;
InfoAddr Addr_tmp_IGBT5= 0xCF0031;
InfoAddr Addr_tmp_IGBT6= 0xCF0032;
InfoAddr Addr_tmp_IN= 0xCF0033;
InfoAddr Addr_tmp_OUT= 0xCF0034;
InfoAddr Addr_inductance1_cur= 0xCF0035;
InfoAddr Addr_inductance2_cur= 0xCF0036;
InfoAddr Addr_inductance3_cur= 0xCF0037;
InfoAddr Addr_inductance4_cur= 0xCF0038;
InfoAddr Addr_inductance5_cur= 0xCF0039;
InfoAddr Addr_inductance6_cur= 0xCF003A;


InfoAddr Addr_mainBreaker_EnergyStore = 0xC20001;
InfoAddr Addr_slaveBreaker_EnergyStore = 0xC20002;
InfoAddr Addr_DCBreaker1_EnergyStore = 0xC20003;
InfoAddr Addr_DCBreaker2_EnergyStore = 0xC20004;
InfoAddr Addr_fireExtinguisherStatus_EnergyStore = 0xC20005;
InfoAddr Addr_DCBreaker3_EnergyStore = 0xC20006;
InfoAddr Addr_DCBreaker4_EnergyStore = 0xC20007;
InfoAddr Addr_travelSwitch_EnergyStore = 0xC20008;
InfoAddr Addr_fireExtinguisher_EnergyStore = 0xC20009;
InfoAddr Addr_founder_EnergyStore = 0xC2000A;

InfoAddr Addr_DCbreaker = 0xC50001;
InfoAddr Addr_fireExtinguisher1 = 0xC50002;

InfoAddr Addr_boardID = 0xD60001;
InfoAddr Addr_moduleMode = 0xD60002;
InfoAddr Addr_moduleStatus0 = 0xD60003;
InfoAddr Addr_moduleStatus1 = 0xD60004;
InfoAddr Addr_moduleStatus2 = 0xD60005;
InfoAddr Addr_moduleStatus3 = 0xD60006;
InfoAddr Addr_moduleStatus4 = 0xD60007;
InfoAddr Addr_moduleStatus5 = 0xD60008;
InfoAddr Addr_moduleStatus6 = 0xD60009;
InfoAddr Addr_moduleStatus7 = 0xD6000A;
InfoAddr Addr_moduleStatus8 = 0xD6000B;
InfoAddr Addr_moduleStatus9 = 0xD6000C;
InfoAddr Addr_moduleStatus10 = 0xD6000D;
InfoAddr Addr_moduleStatus11 = 0xD6000E;
InfoAddr Addr_moduleStatus12 = 0xD6000F;
InfoAddr Addr_moduleStatus13 = 0xD60010;
InfoAddr Addr_moduleStatus14 = 0xD60011;
InfoAddr Addr_moduleStatus15 = 0xD60012;
InfoAddr Addr_moduleStatus16 = 0xD60013;
InfoAddr Addr_moduleStatus17 = 0xD60014;
InfoAddr Addr_moduleStatus18 = 0xD60015;
InfoAddr Addr_moduleStatus19 = 0xD60016;
InfoAddr Addr_moduleStatus20 = 0xD60017;
InfoAddr Addr_moduleStatus21 = 0xD60018;
InfoAddr Addr_moduleStatus22 = 0xD60019;
InfoAddr Addr_moduleStatus23 = 0xD6001A;
InfoAddr Addr_moduleStatus24 = 0xD6001B;
InfoAddr Addr_moduleStatus25 = 0xD6001C;
InfoAddr Addr_moduleStatus26 = 0xD6001D;
InfoAddr Addr_moduleStatus27 = 0xD6001E;
InfoAddr Addr_moduleStatus28 = 0xD6001F;
InfoAddr Addr_moduleStatus29 = 0xD60020;
InfoAddr Addr_moduleStatus30 = 0xD60021;
InfoAddr Addr_moduleStatus31 = 0xD60022;
InfoAddr Addr_outVol = 0xD60023;
InfoAddr Addr_outCur = 0xD60024;
InfoAddr Addr_inVol = 0xD60025;
InfoAddr Addr_inCur = 0xD60026;
InfoAddr Addr_boardTmp_M1 = 0xD60027;
InfoAddr Addr_envTmp = 0xD60028;
InfoAddr Addr_runTime = 0xD60029;
InfoAddr Addr_chargeDischargeTimes = 0xD6002A;


InfoAddr Addr_breaker1VoltaicCabinet1 = 0xC50001;
InfoAddr Addr_breaker2VoltaicCabinet1 = 0xC50002;
InfoAddr Addr_breaker3VoltaicCabinet1 = 0xC50003;
InfoAddr Addr_breaker4VoltaicCabinet1 = 0xC50004;
InfoAddr Addr_breaker5VoltaicCabinet1 = 0xC50005;
InfoAddr Addr_breaker6VoltaicCabinet1 = 0xC50006;
InfoAddr Addr_breaker7VoltaicCabinet1 = 0xC50007;
InfoAddr Addr_fireExtinguisherVoltaicCabinet1 = 0xC50008;

InfoAddr Addr_DCDC1breaker = 0xC70000;
InfoAddr Addr_DCDC1fireExtinguisher= 0xC70001;
InfoAddr Addr_DCDC2breaker = 0xC70002;
InfoAddr Addr_DCDC2fireExtinguisher = 0xC70003;
InfoAddr Addr_DCDCsumVol = 0xC70004;
InfoAddr Addr_DCDCsumCur = 0xC70005;

InfoAddr Addr_lowVolTravelSwitch = 0xC90000;
InfoAddr Addr_highVolTravelSwitch = 0xC90001;
InfoAddr Addr_dormTravelSwitch = 0xC90002;
InfoAddr Addr_lowVolSmokeSensor = 0xC90003;
InfoAddr Addr_highVolSmokeSensor = 0xC90004;
InfoAddr Addr_transformerSmokeSensor = 0xC90005;
InfoAddr Addr_outEmergencyStop = 0xC90006;
InfoAddr Addr_inEmergencyStop = 0xC90007;

InfoAddr Addr_transformerOverTemp = 0xCA0001;
InfoAddr Addr_transformerTempControlerFault = 0xCA0002;
InfoAddr Addr_waterIn_SysControl2 = 0xCA0003;

InfoAddr Addr_sumBreaker = 0xCB0000;
InfoAddr Addr_loadBreaker1 = 0xCB0001;
InfoAddr Addr_loadBreaker2 = 0xCB0002;
InfoAddr Addr_loadBreaker3 = 0xCB0003;
InfoAddr Addr_loadBreaker4 = 0xCB0004;
InfoAddr Addr_acBreaker = 0xCB0005;
InfoAddr Addr_fireExtinguisherDisCabinet = 0xCB0006;

InfoAddr Addr_emergencyStop = 0xCE0001;
InfoAddr Addr_limitSwitch = 0xCE0002;
InfoAddr Addr_smokeSensor = 0xCE0003;
InfoAddr Addr_envTemperature1 = 0xCE0004;
InfoAddr Addr_envHumidity1 = 0xCE0005;

InfoAddr Addr_inCur_EnergyStorage = 0xD70002;
InfoAddr Addr_inVol_EnergyStorage = 0xD70003;
InfoAddr Addr_outCur_EnergyStorage = 0xD70004;
InfoAddr Addr_outVol_EnergyStorage = 0xD70005;
InfoAddr Addr_batteryVol_EnergyStorage = 0xD70006;
InfoAddr Addr_dcPower_EnergyStorage = 0xD70007;
InfoAddr Addr_devStatus0_EnergyStorage = 0xD70008;
InfoAddr Addr_devStatus1_EnergyStorage = 0xD70009;
InfoAddr Addr_devStatus2_EnergyStorage = 0xD7000A;
InfoAddr Addr_warningStatus0_EnergyStorage = 0xD7000B;
InfoAddr Addr_warningStatus1_EnergyStorage = 0xD7000C;
InfoAddr Addr_warningStatus2_EnergyStorage = 0xD7000D;
InfoAddr Addr_faultStatus0_EnergyStorage = 0xD7000E;
InfoAddr Addr_faultStatus1_EnergyStorage = 0xD7000F;
InfoAddr Addr_faultStatus2_EnergyStorage = 0xD70010;
InfoAddr Addr_faultStatus3_EnergyStorage = 0xD70011;
InfoAddr Addr_faultStatus4_EnergyStorage = 0xD70012;
InfoAddr Addr_faultStatus5_EnergyStorage = 0xD70013;
InfoAddr Addr_faultStatus6_EnergyStorage = 0xD70014;
InfoAddr Addr_faultStatus7_EnergyStorage = 0xD70015;
InfoAddr Addr_faultStatus8_EnergyStorage = 0xD70016;
InfoAddr Addr_faultStatus9_EnergyStorage = 0xD70017;
InfoAddr Addr_faultStatus10_EnergyStorage = 0xD70018;
InfoAddr Addr_faultStatus11_EnergyStorage = 0xD70019;
InfoAddr Addr_faultStatus12_EnergyStorage = 0xD7001A;
InfoAddr Addr_faultStatus13_EnergyStorage = 0xD7001B;
InfoAddr Addr_faultStatus14_EnergyStorage = 0xD7001C;
InfoAddr Addr_faultStatus15_EnergyStorage = 0xD7001D;
InfoAddr Addr_devStatus3_EnergyStorage = 0xD7001E;

InfoAddr Addr_B2C_STATUS_CRC = 0xDA0001;
InfoAddr Addr_B2C_STATUS_bmsHeartBeat = 0xDA0002;
InfoAddr Addr_B2C_STATUS_test = 0xDA0003;
InfoAddr Addr_B2C_STATUS_tankSwitch = 0xDA0004;
InfoAddr Addr_B2C_STATUS_singleOverVolAlarm = 0xDA0005;
InfoAddr Addr_B2C_STATUS_singleLowVolAlarm = 0xDA0006;
InfoAddr Addr_B2C_STATUS_OverTempAlarm = 0xDA0007;
InfoAddr Addr_B2C_STATUS_BelowTempAlarm = 0xDA00070;
InfoAddr Addr_B2C_STATUS_insulationAlarm = 0xDA0008;
InfoAddr Addr_B2C_STATUS_BMScommuFault = 0xDA0009;
InfoAddr Addr_B2C_STATUS_BMScontrolPower = 0xDA000A;
InfoAddr Addr_B2C_STATUS_BMSfullPowerON = 0xDA000B;
InfoAddr Addr_B2C_STATUS_BMSsysStatus = 0xDA000C;
InfoAddr Addr_B2C_STATUS_ESSfullEnergy = 0xDA000D;
InfoAddr Addr_B2C_STATUS_ESSfullDisCharge = 0xDA000E;
InfoAddr Addr_B2C_STATUS_ApplyACInfo = 0xDA000E0;
InfoAddr Addr_B2C_STATUS_ApplySysInfo = 0xDA000E1;
InfoAddr Addr_B2C_STATUS_SOC = 0xDA000F;

InfoAddr Addr_B2C_SUMDATA2_CRC = 0xDA0010;
InfoAddr Addr_B2C_SUMDATA2_BMSchargeEnergy = 0xDA0011;
InfoAddr Addr_B2C_SUMDATA2_BMSdisChargeEnergy = 0xDA0012;
InfoAddr Addr_B2C_SUMDATA2_SOH = 0xDA0013;

InfoAddr Addr_B2C_SUMDATA3_sysHumidity = 0xDA0014;
InfoAddr Addr_B2C_SUMDATA3_singleMaxVol = 0xDA0015;
InfoAddr Addr_B2C_SUMDATA3_singleMinVol = 0xDA0016;
InfoAddr Addr_B2C_SUMDATA3_singleMaxTem = 0xDA0017;
InfoAddr Addr_B2C_SUMDATA3_singleMinTem = 0xDA0018;
InfoAddr Addr_B2C_SUMDATA3_sysTemp= 0xDA0019;

InfoAddr Addr_B2C_LIMIT_BMSlimitDischargeCur = 0xDA001A;
InfoAddr Addr_B2C_LIMIT_BMSlimitChargeCur = 0xDA001B;
InfoAddr Addr_B2C_LIMIT_BMSlimitChargeVol = 0xDA001C;
InfoAddr Addr_B2C_LIMIT_BMSlimitDisChargeVol = 0xDA001D;

InfoAddr Addr_BatteryOutVol = 0xDA001E;
InfoAddr Addr_BatteryFuseVol = 0xDA001F;
InfoAddr Addr_BatteryBreakVol = 0xDA0020;
InfoAddr Addr_BatteryCur = 0xDA0021;
InfoAddr Addr_BatteryDcVol = 0xDA0022;
InfoAddr Addr_BatteryDcCur = 0xDA0023;
InfoAddr Addr_BatteryDcPower = 0xDA0024;
InfoAddr Addr_BatteryDcPositiveEnergy = 0xDA0025;
InfoAddr Addr_BatteryDcDisPositiveEnergy = 0xDA0026;
InfoAddr Addr_BatteryDcPT = 0xDA0027;
InfoAddr Addr_BatteryDcCT = 0xDA0028;

InfoAddr Addr_B2C_SUMDATA1_CRC = 0xDA0029;
InfoAddr Addr_B2C_SUMDATA1_tankNum = 0xDA002A;
InfoAddr Addr_B2C_SUMDATA1_BMShighVol = 0xDA002B;
InfoAddr Addr_B2C_SUMDATA1_BMScur = 0xDA002C;


InfoAddr Addr_PowerOptimizerID = 0xDC0001;
InfoAddr Addr_inVol1 = 0xDC0002;
InfoAddr Addr_inVol2 = 0xDC0003;
InfoAddr Addr_inVol3 = 0xDC0004;
InfoAddr Addr_inVol4 = 0xDC0005;
InfoAddr Addr_curBranch1 = 0xDC0006;
InfoAddr Addr_curBranch2 = 0xDC0007;
InfoAddr Addr_curBranch3 = 0xDC0008;
InfoAddr Addr_curBranch4 = 0xDC0009;
InfoAddr Addr_curBranch5 = 0xDC000A;
InfoAddr Addr_curBranch6 = 0xDC000B;
InfoAddr Addr_curBranch7 = 0xDC000C;
InfoAddr Addr_curBranch8 = 0xDC000D;
InfoAddr Addr_curBranch9 = 0xDC000E;
InfoAddr Addr_curBranch10 = 0xDC000F;
InfoAddr Addr_curBranch11 = 0xDC0010;
InfoAddr Addr_curBranch12 = 0xDC0011;
InfoAddr Addr_curBranch13 = 0xDC0012;
InfoAddr Addr_curBranch14 = 0xDC0013;
InfoAddr Addr_curBranch15 = 0xDC0014;
InfoAddr Addr_curBranch16 = 0xDC0015;
InfoAddr Addr_realPower = 0xDC0016;
InfoAddr Addr_radiatorTemp = 0xDC0017;

InfoAddr Addr_fault1_bit0 = 0xDC0018;
InfoAddr Addr_fault1_bit1 = 0xDC0019;
InfoAddr Addr_fault1_bit2 = 0xDC001A;
InfoAddr Addr_fault1_bit3 = 0xDC001B;
InfoAddr Addr_fault1_bit4 = 0xDC001C;
InfoAddr Addr_fault1_bit5 = 0xDC001D;
InfoAddr Addr_fault1_bit6 = 0xDC001E;
InfoAddr Addr_fault1_reserve = 0xDC001F;
InfoAddr Addr_reserve1 = 0xDC0020;
InfoAddr Addr_fault2_bit0 = 0xDC0021;
InfoAddr Addr_fault2_bit1 = 0xDC0022;
InfoAddr Addr_fault2_bit2 = 0xDC0023;
InfoAddr Addr_fault2_bit3 = 0xDC0024;
InfoAddr Addr_fault2_bit4 = 0xDC0025;
InfoAddr Addr_fault2_bit5 = 0xDC0026;
InfoAddr Addr_fault2_bit6 = 0xDC0027;
InfoAddr Addr_fault2_bit7 = 0xDC0028;
InfoAddr Addr_fault2_bit8 = 0xDC0029;
InfoAddr Addr_fault2_bit9 = 0xDC002A;
InfoAddr Addr_fault2_bit10 = 0xDC002B;
InfoAddr Addr_fault2_bit11 = 0xDC002C;
InfoAddr Addr_fault2_bit12 = 0xDC002D;
InfoAddr Addr_fault2_bit13 = 0xDC002E;
InfoAddr Addr_fault2_bit14 = 0xDC002F;
InfoAddr Addr_fault2_bit15 = 0xDC0030;
InfoAddr Addr_warning_bit0 = 0xDC0031;
InfoAddr Addr_warning_bit1 = 0xDC0032;
InfoAddr Addr_warning_bit2 = 0xDC0033;
InfoAddr Addr_warning_bit3 = 0xDC0034;
InfoAddr Addr_warning_bit4 = 0xDC0035;
InfoAddr Addr_warning_bit5 = 0xDC0036;
InfoAddr Addr_warning_bit6 = 0xDC0037;
InfoAddr Addr_warning_bit7 = 0xDC0038;
InfoAddr Addr_warning_bit8 = 0xDC0039;
InfoAddr Addr_warning_bit9 = 0xDC003A;
InfoAddr Addr_warning_bit10 = 0xDC003B;
InfoAddr Addr_warning_bit11 = 0xDC003C;
InfoAddr Addr_warning_bit12 = 0xDC003D;
InfoAddr Addr_warning_bit13 = 0xDC003E;
InfoAddr Addr_warning_bit14 = 0xDC003F;
InfoAddr Addr_warning_bit15 = 0xDC0040;

InfoAddr Addr_combinerStatus = 0xDC0041;
InfoAddr Addr_SoftVer_PO = 0xDC00420;
InfoAddr Addr_softVer_L = 0xDC0042; //(现104模块中使用,后期废弃)
InfoAddr Addr_softVer_H = 0xDC0043; //(现104模块中使用,后期废弃)
InfoAddr Addr_sysRequestStatus_bit0 = 0xDC0044;
InfoAddr Addr_sysRequestStatus_bit1 = 0xDC0045;
InfoAddr Addr_sysRequestStatus_reserve = 0xDC0046;
InfoAddr Addr_reserve2 = 0xDC0047;
InfoAddr Addr_inVol5 = 0xDC0048;
InfoAddr Addr_inVol6 = 0xDC0049;
InfoAddr Addr_inVol7 = 0xDC004A;
InfoAddr Addr_inVol8 = 0xDC004B;

/*******通信协议3.0********/

//账户信息
InfoAddr Addr_UserStatus = 0x300000;    //请求账户的状态
InfoAddr Addr_UserId = 0x300001;		//用户标识
InfoAddr Addr_UserId_Type = 0x300002;	//用户标识类型 0：无效，1：卡号，2：VIN，3：车牌号，4：APP
InfoAddr Addr_MarkId = 0x300003;		//请求标识
InfoAddr Addr_Account_Detail = 0x300004;//账户详情
InfoAddr Addr_UserLevel = 0x300005; //int 1可欠费, 2不可欠费
InfoAddr Addr_ServiceFeeBalance = 0x300006; //float 服务费余额
InfoAddr Addr_ElectricFeeBalance = 0x300007;    //float 电费余额
InfoAddr Addr_CommonFeeBalance = 0x300008;  //float 通用余额

//计费策略
InfoAddr Addr_BillingChargeInfo = 0x300020; //充电计费策略
InfoAddr Addr_BillingDischargeInfo = 0x300021; //放电计费策略

//充电类型
InfoAddr Addr_ChargeType_Cloud = 0x300040;  //平台下发充电类型  int
InfoAddr Addr_ChargeValue_Cloud = 0x300041; //平台下发充电参数值 float
InfoAddr Addr_ChargeDesc_Cloud = 0x3000042; //平台下发充电描述
InfoAddr Addr_ChargeType = 0x300043;  //充电类型  充电or放电

//控制相关
InfoAddr Addr_Mod_Work_Ctrl = 0x300044; //模块开关机控制
InfoAddr Addr_Mod_Power_Set = 0x300045; //功率设置

//订单相关
InfoAddr Addr_Order_State = 0x300100;          //订单状态
InfoAddr Addr_Order_GUID = 0x300101;            //GUID, 订单唯一标识

//获取信息相关
InfoAddr Addr_ApplyUserInfo = 0x300200;         //获取用户信息
InfoAddr Addr_ApplyBillingStragetyInfo = 0x300201;         //获取计费策略
InfoAddr Addr_ApplyCarInfo = 0x300202;         //获取车辆信息
InfoAddr Addr_ApplyCardInfo = 0x300203;         //获取卡片信息
//反向扫码
InfoAddr Addr_ScanCode = 0x2405EA;//
InfoAddr Addr_ScanCode_customerID = 0x2405EB;//扫码客户ID
InfoAddr Addr_ScanCode_Type = 0x2405EC;//扫码业务查询账户信息类型
InfoAddr Addr_ScanCode_Charge_Type = 0x2405ED;//扫码申请充电类型
InfoAddr Addr_ScanCode_StartCharge_Result = 0x2405EE;//扫码申请开始充电返回结果
InfoAddr Addr_ScanCode_StopCharge_Result = 0x2405EF;//扫码申请结束充电返回结果
//多枪充电相关
InfoAddr Addr_Group_ChargeGun = 0x2404D2;			//下发多枪分组信息
InfoAddr Addr_ChargeGunGroup_Result = 0x2404E2;			//下发多枪分组信息保存结果
InfoAddr Addr_ChargeGunType = 0x2404E3;			//多枪模式
InfoAddr Addr_ChargeGun_Master = 0x2404E4;			//主枪
InfoAddr Addr_ChargeGun_Slave1 = 0x2404E5;			//副枪1
InfoAddr Addr_ChargeGun_Slave2 = 0x2404E6;			//副枪2
InfoAddr Addr_ChargeGun_Slave3 = 0x2404E7;			//副枪3
InfoAddr Addr_ChargeGun_Slave4 = 0x2404E8;			//副枪4
InfoAddr Addr_ChargeGun_Slave5 = 0x2404E9;			//副枪5
InfoAddr Addr_ChargeGun_Slave6 = 0x2404EA;			//副枪6
InfoAddr Addr_ChargeGunType_Reault = 0x2404EB;			//充电方式结果

//++++++++++++++++++++南京3.0协议相关
InfoAddr Addr_ChargeOrder_State = 0x330001;	//订单状态 1:开始 2:结束
InfoAddr Addr_Account_Balance = 0x330003;	//账户余额
InfoAddr Addr_Bill_Code = 0x330004;			//订单流水号
InfoAddr Addr_LogicState_Term = 0x330005;	//终端逻辑工作状态
InfoAddr Addr_LogicState_Burst = 0x330006;	//突发逻辑工作状态
InfoAddr Addr_DevData_Type = 0x330007;		//设备类型，枚举DevDataType
InfoAddr Addr_Power_Curve_State = 0x330008;	//功率曲线状态 0：不启用功率曲线 其它：功率曲线下发时间
InfoAddr Addr_Charge_Policy = 0x330009;		//计费策略 0：无策略 1：有策略
InfoAddr Addr_Clear_Power_Curve = 0x330010; //清理功率曲线
InfoAddr Addr_Resend_Cmd = 0x330011; 		//协议重发类: 指令号
InfoAddr Addr_Resend_CmdAck = 0x330012; 	//协议重发响应: 指令ACK
InfoAddr Addr_CSCU_AlarmCode = 0x330013; 	//集控告警码信息体: 告警码
InfoAddr Addr_CSCU_AlarmContent = 0x330014;	//集控告警内容信息体: 告警内容
InfoAddr Addr_BillSendReult = 0x330015; //未召唤订单上传结果 1-单条上传成功 2-全部上传成功
InfoAddr Addr_Energy_ChargeType = 0x330016;  //应急-充电类型
InfoAddr Addr_Energy_ChargeWay = 0x330017;  //应急-启动方式
//----------------南京3.0协议相关

//主动防护
InfoAddr Addr_ActiveDefend_AlarmCode = 0x230048;//主动防护告警码
InfoAddr Addr_ActiveDefend_Action = 0x230049;//主动防护动作 1：停止充电 2：跳闸

//网络地址获取消息++++++++++++++dht
InfoAddr Addr_Error_Addr = 0x440000;	//错误应答 1、集控地址非法 2、秘钥非法 3、版本号非法 FF、其它错误
InfoAddr Addr_Comm_Addr = 0x440001;		//充电通信地址及端口 格式：router.teld.cn:8998
InfoAddr Addr_Web_Url = 0x440002;		//WebServer地址及端口 格式：api.teld.cn:8001
InfoAddr Addr_Monitor_Addr = 0x440003;	//智能运维地址及端口 格式：api.teld.cn:8001
InfoAddr Addr_Setting_Addr = 0x440004;	//配置协议地址及端口 格式：api.teld.cn:8001
//------------------------------dht
#endif
