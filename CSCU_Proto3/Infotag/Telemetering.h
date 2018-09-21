#ifndef TELEMETERING_H
#define TELEMETERING_H

#include "GeneralData/GeneralData.h"
//typedef  const unsigned int InfoAddr;

/***********************充电终端遥测******************************/
InfoAddr Addr_AVoltage_Term = 0x201001;//A相电压
InfoAddr Addr_BVoltage_Term = 0x201002;//B相电压
InfoAddr Addr_CVoltage_Term = 0x201003;//C相电压
InfoAddr Addr_ACurrent_Term = 0x201004;//A相电流
InfoAddr Addr_BCurrent_Term = 0x201005;//B相电流
InfoAddr Addr_CCurrent_Term = 0x201006;//C相电流
InfoAddr Addr_TotalActivePower_Term = 0x201007;//总有功功率
InfoAddr Addr_TotalReactivePower_Term = 0x201008;//总无功功率
InfoAddr Addr_TotalPowerFactor_Term = 0x201009;//总功率因数
InfoAddr Addr_ZeroLineCurrent_Term = 0x20100A;//零线电流
InfoAddr Addr_VoltageUnbalanceRate_Term = 0x20100B;//电压不平衡率
InfoAddr Addr_CurrentUnbalanceRate_Term = 0x20100C;//电流不平衡率
InfoAddr Addr_DCChargeVoltage_Term = 0x20100D;//直流电压
InfoAddr Addr_DCChargeCurrent_Term = 0x20100E;//直流电流

InfoAddr Addr_ChargeTime_Term = 0x20100F;//充电时间
InfoAddr Addr_TotalActivePower_original_Term = 0x201010;//总有功功率
InfoAddr Addr_TotalReactivePower_original_Term = 0x201011;//总无功功率

InfoAddr Addr_TotalActiveEnergy_Term = 0x203001;//总有功电能
InfoAddr Addr_TotalReactiveEnergy_Term = 0x203002;//总无功电能
InfoAddr Addr_TotalReverseActiveEnergy_Term = 0x203003;//反向总有功电能
InfoAddr Addr_TotalReverseReactiveEnergy_Term = 0x203004;//反向总无功电能

InfoAddr Addr_ACSinVoltage_Term = 0x203005;//交流单相电压
InfoAddr Addr_ACSinCurrent_Term = 0x203006;//交流单相电流

InfoAddr Addr_DCChargeManner_Term = 0x2401A4;//单双枪充电方式
InfoAddr Addr_DCChargeMasterCANID_Term = 0x2401A5;//主枪
InfoAddr Addr_DCChargeSlaveCANID_Term = 0x2401A6;//副枪
InfoAddr Addr_DCChargeMannerSetResult_Term = 0x2401A7;//平台设置单双枪充电结果

/***********************直流柜遥测******************************/
//CCU
InfoAddr Addr_CCUTotalRunTime_DCcab = 0x238001;//CCU总运行时间
InfoAddr Addr_CCUEnvTemp_DCcab = 0x238002;//CCU环境温度
InfoAddr Addr_CabRatedPower_DCCab = 0x238003;//直流柜额定输出功率
InfoAddr Addr_CabNowPower_DCCab = 0x238004;//直流柜当前xuqiu功率
InfoAddr Addr_CabOutPower_DCCab = 0x238005;//直流柜当前out功率
InfoAddr Addr_JiGuiTempHumidty =0x238006 ;        //机柜温度湿度传感器结果  //smm add

//直流模块
InfoAddr Addr_DCModuleInAVoltage_DCcab = 0x238021;//直流模块输入A相电压
InfoAddr Addr_DCModuleInBVoltage_DCcab = 0x238022;//直流模块输入B相电压
InfoAddr Addr_DCModuleInCVoltage_DCcab = 0x238023;//直流模块输入C相电压
InfoAddr Addr_DCModuleInCurrent_DCcab = 0x238024;//直流模块输入电流
InfoAddr Addr_DCModuleOutVoltage_DCcab = 0x238025;//直流模块输出电压
InfoAddr Addr_DCModuleOutCurrent_DCcab = 0x238026;//直流模块输出电流
InfoAddr Addr_DCModuleEnvTemp_DCcab = 0x238027;//直流模块环境温度
InfoAddr Addr_DCModuleM1Temp_DCcab = 0x238028;//直流模块M1温度
InfoAddr Addr_DCModuleTotalRunTime_DCcab = 0x238029;//直流模块总运行时间
InfoAddr Addr_DCModuleSwitchTime_DCcab = 0x23802A;//直流模块切换次数

//PDU
InfoAddr Addr_PDUOutVoltage_DCcab = 0x23817E;//PDU输出电压
InfoAddr Addr_PDUOutCurrent_DCcab = 0x23817F;//PDU输出电流
InfoAddr Addr_PDUEnvTemp_DCcab = 0x238180;//PDU环境温度
InfoAddr Addr_PDURadTemp_DCcab = 0x238181;//PDU散热器温度
InfoAddr Addr_PDUTotalRunTime_DCcab = 0x238182;//PDU总运行时间
InfoAddr Addr_PDUSwitchTime_DCcab = 0x238183;//PDU切换次数
InfoAddr Addr_PDUEnergy_DCcab = 0x238184;//电表度数
InfoAddr Addr_PDUResistanceMinus_DCcab = 0x238185;//绝缘电阻负对地阻值
InfoAddr Addr_PDUResistancePlus_DCcab = 0x238186;//绝缘电阻正对地阻值
InfoAddr Addr_PDUSetVol_DCcab = 0x238187;//PDU设模块输出电压
InfoAddr Addr_PDUSetCur_DCcab = 0x238188;//PDU设模块限流点

InfoAddr Addr_Qiang_Temp = 0x238189;     //配置枪A地址

//分支箱
InfoAddr Addr_BOXOutVoltage_DCcab = 0x238230;//分支箱输出电压
InfoAddr Addr_BOXOutCurrent_DCcab = 0x238231;//分支箱输出电流
InfoAddr Addr_BOXEnvTemp_DCcab = 0x238232;//分支箱环境温度
InfoAddr Addr_BOXM1Temp_DCcab = 0x238233;//分支箱M1板温度
InfoAddr Addr_BOXTotalRunTime_DCcab = 0x238234;//分支箱总运行时间
InfoAddr Addr_BOXSwitchTime_DCcab = 0x238235;//分支箱切换次数


#endif
