#ifndef LCDDEF_H
#define LCDDEF_H

//数据内容----CSCU设置项
typedef struct _CSCUVersion_LCD
{
    char chCoreVer[20];   //内核版本
    char chFSVer[20];    //文件系统版本
    char chCSCUVer[20]; //cscu程序版本
    char chTEUIVer[20]; //teui程序版本
    char chHardVer[20];//硬件版本
    char chMACid[30];//mac地址
    char chSN[30];  // 集空产品序列号SN

}__attribute__ ((packed)) CSCUVersion_LCD;

//数据内容----CSCU设置项
typedef struct _CSCUSet_LCD
{
    char chLocolIp[16];   //本地IP
    char chMask[16];    //子网掩码
    char chGateWay[16]; //网关
    char chDNS[16]; //网关
	char chServerIP1[30];//平台地址1
	unsigned short usPortNum1;//端口号1
	char chServerIP2[30];//平台地址2
	unsigned short usPortNum2;//端口号2
	char chServerIP3[30];//平台地址3
	unsigned short usPortNum3;//端口号3
    char chStationAddr[20];   //CSCU站地址
    unsigned char ucZigbeeID;   //zigbee地址
    unsigned char ucACThrNum;   //交流三相个数
    unsigned char ucACSinNum;   //交流单相个数
    unsigned char ucDCNum;   //直流个数
}__attribute__ ((packed)) CSCUSet_LCD;

//数据内容----CSCU设置项
typedef struct _SysSpecSet_LCD
{
    unsigned char ucVINOffline;   //VIN后6位启动充电
    unsigned char ucLocalStop;    //本地结束
    unsigned char ucCardType;    //卡类型
    unsigned char ucVINAuto;    //vin自动充电
    unsigned char ucCardAuto;    //刷卡自动充电
    unsigned char ucVINType;    //vin或车牌号
    unsigned char ucEnergyFilter;    //异常电度数过滤
    unsigned char ucLocalChargeEnable;    //本地充电使能开关
    unsigned char ucLocalChargeType;    //本地充电方式选择
    unsigned char ucDevType;  //设备类型，1单桩 2群充
    unsigned char coupleGun;   //多枪设置
    unsigned char ucticketEnable; //小票机 0 无 1 有
    unsigned char ucLanguage;  //1汉语 2英语
}__attribute__ ((packed)) SysSpecSet_LCD;

//数据内容----负荷约束功能设置数据项
typedef struct _LoadLimit_LCD
{
    unsigned char ucLoadLimit;    //负荷约束功能  0:关闭, 1:开启
    unsigned char ucCCUNum;   //CCU个数
    unsigned short usTotalPower;    //总功率
    unsigned short usSecurePower;    //安全功率
    unsigned short usLimitPower;    //限制功率(手动设置充电限制功率)
    unsigned char ucDynamicEnable;    //动态计算使能
    unsigned char ucRemoteEnable;    //远端设置使能
    unsigned char ucLocalEnable;    //本地设置使能
}__attribute__ ((packed)) LoadLimit_LCD;

//数据内容----负荷约束功能设置数据项
typedef struct _EnvInfo_LCD
{
    unsigned short usTemp;    //温度
    unsigned short usHumi;    //湿度
    unsigned char ucIOState[10];    //IO状态
}__attribute__ ((packed)) EnvInfo_LCD;

//数据内容----错峰充电功能设置数据项
typedef struct _PeakSet_LCD
{
    unsigned char ucType;   //峰平谷尖类型
    unsigned char ucStartH;   //开始时间:小时
    unsigned char ucStartM;   //开始时间:分钟
    unsigned char ucStopH;   //结束时间:小时
    unsigned char ucStopM;   //结束时间:分钟
    unsigned char ucSOC;   //限制SOC
    unsigned char ucCurrent;   //限制电流
}__attribute__ ((packed)) PeakSet_LCD;

//数据内容----终端状态数据项
typedef struct _TermState_LCD
{
    unsigned char ucCanAddr;   //CAN地址
    unsigned char ucLinkState;   //枪连接状态
    unsigned char ucChargeState;   //充电状态
    unsigned char ucLogicState;   //逻辑状态
    unsigned char ucSOC;    //SOC
    char chName[20];    //终端名称
}__attribute__ ((packed)) TermState_LCD;


//数据内容----终端状态数据项----普通版
//有符号数
typedef struct _TermMeasure_Normal_LCD
{
    unsigned char ucCanAddr;  //CAN地址
    unsigned char ucState;  //终端状态
    short A_voltage;						//A相充电电压
    short A_current;						//A相充电电流
    short B_voltage;						//B相充电电压
    short B_current;						//B相充电电流
    short C_voltage;						//C相充电电压
    short C_current;						//C相充电电流
    short active_power;						//总有功功率
    short reactive_power;					//总无功功率
    short power_factor;						//总功率因数
    short voltage_unbalance_rate;			//电压不平衡率
    short current_unbalance_rate;			//电流不平衡率
    short neutralLine_current;   			//零线电流

    short voltage_of_dc;                    //直流侧电压
    short current_of_dc;            		//直流侧电流
    int active_electric_energy;				//总有电能
    int reactive_electric_energy;			//总无功电能
    short gun_temp;  //枪头温度

}__attribute__ ((packed)) TermMeasure_Normal_LCD;

//数据内容----终端状态数据项----刷卡版
typedef struct _FrameTermDetail_Card_LCD
{
    unsigned char ucCanAddr;  //CAN地址
    short A_voltage;						//A相充电电压
    short B_voltage;						//B相充电电压
    short C_voltage;						//C相充电电压
    short A_current;						//A相充电电流
    short B_current;						//B相充电电流
    short C_current;						//C相充电电流
    short voltage_of_dc;                    //直流侧电压
    short current_of_dc;            		//直流侧电流

    short sActivePower;						//总有功功率
    unsigned int uiTotalEnergy;       //总电量
    unsigned short usChargeEnergy;       //充电电量
    char chStartTime[20];   //开始充电时间
    unsigned short usChargeTime;       //充电时间--分钟
    unsigned char ucSOC;       //SOC
}__attribute__ ((packed)) FrameTermDetail_Card_LCD;

//数据内容----终端状态数据项
typedef struct _TermBMS_LCD
{
    unsigned char ucCanAddr;  //CAN地址
    unsigned char ucState;  //终端状态
    unsigned short usBMSNeedVoltage;//BMS 电压需求
    short sBMSNeedCurrent;//BMS 电流需求
    unsigned char ucBaterySOC;//电池当前SOC
    unsigned short usMaxBateryTemperature;//最高电池温度
    short sMaxBateryVoltage;//最高电池电压
    unsigned short usLowestBatteryTemperature;//最低电池温度
    unsigned short usLowestChargeVoltage;//最低电池电压数(单体电池电压)
}__attribute__ ((packed)) TermBMS_LCD;

//发送进线侧信息   CSCU→TEUI
typedef struct _FrameAmmeterData_LCD
{
    char chAddr[6];    //电表地址
    short A_voltage;						//A相充电电压
    short A_current;						//A相充电电流
    short B_voltage;						//B相充电电压
    short B_current;						//B相充电电流
    short C_voltage;						//C相充电电压
    short C_current;						//C相充电电流
    //int total_energy; //总有功电能
    int reactive_power;  //总无功功率  nihai modify  20170601 将总有功电能修正为总无功功率
    int active_power;   //总有功功率
    short power_factor;						//总功率因数
    int ReactiveSensibilityEnergy; //感性无功电能
    int ReactiveCapacityEnergy; //容性无功电能
    short neutralLine_current;   			//零线电流
    int ActiveAbsorbEnergy; //正向有功电能
    int ActiveLiberateEnergy; //反向有功电能

}__attribute__ ((packed)) FrameAmmeterData_LCD;

//直流特殊功能设置单条信息
typedef struct _Node_DCSpecSet_LCD
{
    unsigned char ucCanAddr; //CAN地址
    unsigned char ucGroupStrategy; //终端群充策略
    unsigned char ucTermWorkState; //终端工作状态
    unsigned char  ucAuxType;          //终端辅助电源类型
}__attribute__ ((packed)) Node_DCSpecSet_LCD;

//充电记录单条记录
typedef struct _Node_ChargeRecord_LCD
{
    unsigned char ucCanAddr; //CAN地址
    char chStartTime[20]; //开始充电时间
    char chStopTime[20]; //结束充电时间
    short sEnergy;  //充电电量
    char chReason[60]; //中止原因   //nihai modify 20170601 中止原因40Byte太短，增加到60Byte，解决乱码问题
}__attribute__ ((packed)) Node_ChargeRecord_LCD;

//故障记录单条记录
typedef struct _Node_FaultRecord_LCD
{
    unsigned char ucCanAddr; //CAN地址
    unsigned char ucInnerAddr; //内部地址
    unsigned char ucMinPDUAddr; //最小PDU地址
    unsigned char ucMaxPDUAddr; //最大PDU地址
    char chStartTime[20]; //开始充电时间
    char chStopTime[20]; //结束充电时间
    char chReason[40]; //故障信息
    char chType;    //故障类型 1 终端故障 2设备故障
}__attribute__ ((packed)) Node_FaultRecord_LCD;

//未消除故障记录单条记录 add by muty 20170913
typedef struct _Node_NoRemoveFaultRecord_LCD
{
    unsigned char ucCanAddr; //CAN地址
    unsigned char ucInnerAddr; //内部地址
    unsigned char ucMinPDUAddr; //最小PDU地址
    unsigned char ucMaxPDUAddr; //最大PDU地址
    char chStartTime[20]; //开始充电时间
    char chReason[40]; //故障信息
    char chType;    //故障类型 1 终端故障 2设备故障
}__attribute__ ((packed)) Node_NoRemoveFaultRecord_LCD;

//操作记录单条记录
typedef struct _Node_OperateRecord_LCD
{
    unsigned char ucOptType; //操作类型
    char chOptName[50]; //操作者身份
    char chOptTime[20]; //操作时间
    char chOptData[50]; //操作内容
}__attribute__ ((packed)) Node_OperateRecord_LCD;

//二维码记录单条记录
typedef struct _Node_2DbarCodeRecord_LCD
{
    unsigned char ucCanID; //CAN地址
    char chCode[20]; //二维码编码
}__attribute__ ((packed)) Node_2DbarCodeRecord_LCD;

//历史记录条目数据项
typedef struct _FrameRecordNum_LCD
{
    unsigned short usType;   //记录类型
    unsigned short usPageNum;   //总页数
    unsigned short usRecordNum;   //总条目数
}__attribute__ ((packed)) FrameRecordNum_LCD;

//未消除记录条目数据项 add by muty 20170913
typedef struct NoRemoveFrameRecordNum_LCD
{
    unsigned short usPageNum;   //总页数
    unsigned short usRecordNum;   //总条目数
}__attribute__ ((packed)) NoRemoveFrameRecordNum_LCD;


//当前故障节点
typedef struct _FrameTermNowFaultNode_LCD
{
    unsigned char ucCanAddr; //CAN地址
    unsigned char ucInnerID;    //内部ID
    char ucFaultStartTime[20]; //故障开始时间
    char chFaultReason[40]; //故障代码
}__attribute__ ((packed))FrameTermNowFaultNode_LCD;

//故障报告 nihai add 20170613
typedef struct _FrameTermFaultReport_LCD
{
    char chChargeStartTime[20]; //充电报告开始时间
    char chChargeStopTime[20];  //充电结束报告
    unsigned char ucSOC;  //当前soc
    char chStopReason[60];
}__attribute__ ((packed)) FrameTermFaultReport_LCD;

//发送终端充电报告
typedef struct _FrameTermChargeReport_LCD
{
    unsigned char ucCanAddr; //CAN地址
    unsigned char ucStartSOC; //开始SOC
    unsigned char ucStopSOC; //结束SOC
    short sEnergy;  //充电电量
    char chStartTime[20]; //开始充电时间
    char chStopTime[20]; //结束充电时间
    unsigned short usChargeTime;  //充电时间
    //char chStopReason[40]; //中止原因
    char chStopReason[60]; //中止原因   nihai modify 20170528 解决乱码问题
}__attribute__ ((packed)) FrameTermChargeReport_LCD;

//发送人机交互结果
typedef struct _FrameOperateResult_LCD
{
    unsigned short usDataType;  //数据类型
    unsigned char ucResult; //设置结果
    unsigned char ucReboot;//重启标志, 1重启, 0:不重启
}__attribute__ ((packed)) FrameOperateResult_LCD;

//刷卡信息申请数据项
typedef struct _CardInfoApply_LCD
{
    unsigned char ucCanID;
    unsigned char ucState;
    unsigned char ucCardFlag;   // 1 读取卡  2停止读卡  nihai add 20170523
    unsigned char ucFlag;   //1 开始充电 2 结束充电
}__attribute__ ((packed))CardInfoApply_LCD;

//刷卡命令申请数据项
typedef struct _CardCmdApply_LCD
{
    unsigned char ucCanID;
    unsigned char ucState;
    unsigned char ucFlag;   //1 开始充电 2 结束充电
    unsigned char ucType; //充电方式 1 自动充满 2 按电量充 3 多枪充电手动确认
    unsigned short usEnergy;    //充值电量
}__attribute__ ((packed))CardCmdApply_LCD;

//刷卡结果返回数据项
typedef struct _CardResult_LCD
{
    unsigned char ucCanID;
    unsigned char ucState;
    unsigned char ucFlag;   //1 开始充电 2 结束充电
    unsigned char ucResult; //1 成功 2 失败
    char chFaultReason[50];
    char chCardNum[8];
    unsigned int uiBalance;
}__attribute__ ((packed))CardResult_LCD;

//U盘结果返回数据项
typedef struct _UdiskResult_LCD
{
    unsigned char ucType;
    unsigned char ucResult;
}__attribute__ ((packed)) UdiskResult_LCD;

//参数查询结构体
typedef struct _QueryArg_LCD
{
    unsigned char ucType;
    unsigned char ucCanID;
}__attribute__ ((packed)) QueryArgLCD;

//直流终端参数设置结构体
typedef struct _DCTermParamSet_LCD
{
    unsigned char ucCanID;
    unsigned char ucAuxPowerSet;    //辅源类型
    unsigned char ucNoBelowTempProtect;  //低温使能 :1 使能；0 不使能
    unsigned char ucElecLockEnableFlag;     //电子锁使能
    unsigned char ucElecLockType;   //电子锁类型
    unsigned char ucVINEnableFlag;  //VIN使能标识
    unsigned char ucBMSProType;      //新老国标
    unsigned short usGunMaxCurrent;  //枪头最大电流
    unsigned char ucPriority;   //充电优先级
    unsigned char ucGroupType;  //群充轮充控制模式
    unsigned char ucGroupStrategy;  //群充策略
    unsigned char ucTermID; //终端ID设置

}__attribute__ ((packed)) DCTermParamSet_LCD;

//CCU参数设置结构体
typedef struct _DCCCUParamSet_LCD
{
    unsigned char ucCanID;
    unsigned short usCabMaxPower;//直流柜最大输出功率
    unsigned char ucTermStartID;    //设置直流柜终端的起始地址
    unsigned char ucCCUID; //设置CCU的ID

}__attribute__ ((packed)) DCCCUParamSet_LCD;

//功率模块数据结构体
typedef struct _MODData_LCD
{
    unsigned char ucCanID;
    unsigned char ucInnerID;
    unsigned char ucWorkState; //工作状态
    unsigned short usOutVoltage;//输出电压
    short sOutCurrent;//输出电流
    short sM1Temperature;//M1温度
    unsigned short usInAVoltage;//A相输入电压
    unsigned short usInBVoltage;//B相输入电压
    unsigned short usInCVoltage;//C相输入电压
    unsigned char ucGroupNum;//所属分组
    unsigned char ucAlarmState; //告警状态
    char chSerialNumber[40];  //序列号
    char chSoftwareVer[20];
    char chSoftwareVer1[20];
    char chSoftwareVer2[20];
    char chHardwareVer[20];

}__attribute__ ((packed)) MODData_LCD;

//PDU数据结构体
typedef struct _PDUData_LCD
{
    unsigned char ucCanID;
    unsigned char ucInnerID;
    unsigned char ucWorkState; //工作状态
    short sOutVoltage;//输出电压
    short sOutCurrent;//输出电流
    short sRadiatorTemperature;//散热器温度
    unsigned char ucAlarmState; //告警状态
    char chSerialNumber[40];  //序列号
    char chSoftwareVer[20];
    char chSoftwareVer1[20];
    char chSoftwareVer2[20];
    char chHardwareVer[20];

}__attribute__ ((packed)) PDUData_LCD;

//CCU数据结构体
typedef struct _CCUData_LCD
{
    unsigned char ucCanID;
    unsigned char ucInnerID;
    unsigned char ucWorkState; //工作状态
    short sEnvTemperature;//散热器温度
    unsigned short usNowPower; //当前需求功率
    unsigned char ucAlarmState; //告警状态
    char chSerialNumber[40];  //序列号
    char chSoftwareVer[20];
    char chSoftwareVer1[20];
    char chSoftwareVer2[20];
    char chHardwareVer[20];

}__attribute__ ((packed)) CCUData_LCD;

//本地充电结构体
typedef struct _LocalCharge_LCD
{
    unsigned char ucCanID;
    unsigned char ucState;
    unsigned char ucLocalType;   //1 本地密码 2本地按钮
    unsigned char ucChargeType; //1 开始充电 2 结束充电
    unsigned char ucResult; //0 失败  1成功
}__attribute__ ((packed)) LocalCharge_LCD;

//本地充电结构体
typedef struct _LocalChargeStop_LCD
{
    unsigned char ucCanID;
    unsigned char ucState;
    unsigned char ucChargeType; //1 开始充电 2 结束充电
    unsigned char ucResult; //0 失败  1成功
}__attribute__ ((packed)) LocalChargeStop_LCD;

//打印小票返回数据项
typedef struct _TicketDevResult_LCD
{
    unsigned char ucCanID;
    unsigned char ucResult; //1 成功 2 失败
    char chFaultReason[50];
}__attribute__ ((packed))TicketDevResult_LCD;

#endif // LCDDEF_H
