#ifndef SCREENDEF_H
#define SCREENDEF_H

//指令类型枚举
typedef enum  _CmdType_Screen
{
    Type_WriteCtrl_Screen = 0x80,    //写控制寄存器指令
    Type_WriteMem_Screen = 0x82,    //写数据寄存器指令
    Type_ReadMem_Screen = 0x83,     //读数据寄存器指令
    Type_ParamSet_Screen = 0x84,    //系统参数设置指令
    Type_ACPhaseSet_Screen = 0x85,    //交流三相相别设置指令
    Type_Button_Screen = 0x86,      //接收按钮按下指令(TEUI→CSCU)
    Type_TermState_Screen = 0x87,      //接收终端状态指令(TEUI→CSCU)
    Type_TermBMS_Screen = 0x88,      //接收申请BMS指令(TEUI→CSCU)
    Type_CardStartCharge_Screen = 0x89, //接收刷卡开始充电指令(TEUI→CSCU)
    Type_DCSpecSet_Screen = 0x8A, //接收直流特殊功能设置(TEUI→CSCU)
    Type_AlarmParamSet_Screen = 0x8B, //报警器配置参数下发指令(TEUI→CSCU)
    Type_VINEndInput_Screen = 0x8C, //VIN后6位下发指令(TEUI→CSCU)
    Type_TermName_Screen = 0x8E, //获取终端名称指令(TEUI→CSCU)
    Type_SpecFuncSet_Screen = 0x95,      //接收特殊功能设置指令(TEUI→CSCU)
    Type_SpecPasswd_Screen = 0x96,      //接收特殊功能设置密码指令(TEUI→CSCU)
    Type_SpecResetPasswd_Screen = 0x97,      //接收特殊功能重新设置密码指令(TEUI→CSCU)
    Type_PeakSet_Screen = 0x98,      //接收错峰充电功能设置指令(TEUI→CSCU)
    Type_LoadDispatchEnable_Screen = 0x99,      //接收负荷调度,错峰充电使能信息指令(TEUI→CSCU)
    Type_LoadLimit_Screen = 0x9A,      //接收负荷限制功能设置指令(TEUI→CSCU)
    Type_SpecFGPJ_Screen = 0x92,      //峰谷平尖确定指令
    Type_SpecCFCD_Screen = 0x9F,      //错峰充电确定指令
    Type_SpecPolicy_Screen = 0xA0      //本地策略确定指令
}CmdType_Screen;

//同一变量多寄存器地址间隔枚举
typedef enum _RegInterval_Screen
{
    RegIn_TermStateInterval_Screen = 0x05, //终端状态寄存器地址间隔----主界面
    RegIn_TermIDInterval_Screen = 0x01,  //终端CAN地址寄存器地址间隔----主界面
    RegIn_TermNameInterval_Screen = 0x14,  //终端名称寄存器地址间隔----主界面
    Reg_FaultInfoLineInterval_Screen = 0x03,   //故障列表记录寄存器地址间隔----故障列表记录界面
    Reg_PeakDetailInterval_Screen = 0x30,   //峰平谷尖寄存器地址间隔----错峰充电详细信息界面
    Reg_DCSpecInterval_Screen = 0x03   //直流特殊功能设置寄存器地址间隔----直流特殊功能设置界面
}RegInterval_Screen;

//寄存器地址枚举
typedef enum _RegAddr_Screen
{
    Reg_Spec_SwitchPage_Screen = 0x03,//切换界面  寄存器地址----单字节
    Reg_Spec_SetTime_Screen = 0x1F,//对时指令  寄存器地址----单字节
    Reg_Spec_Reboot_Screen = 0xEE,//重启指令  寄存器地址----单字节
    Reg_Spec_SetSensitivity_Screen = 0xE0,//设置屏幕灵敏度  寄存器地址----单字节

    Reg_ProSetIcon_Screen = 0x0030,//刷卡工程选择界面, 图标 寄存器地址
    Reg_IconPage_Screen = 0x624E,//图标显示界面 寄存器地址
    Reg_FaultInfoIcon_Screen = 0x0689,//主界面, 故障图标指令  寄存器地址
    Reg_SerialState_Screen = 0x068D, //主界面, CSCU串口通信状态  寄存器地址
    Reg_CSCUProVer_Screen = 0x6290, //主界面, CSCU版本号  寄存器地址
    Reg_NetState_Screen = 0x636F, //主界面, CSCU网络通信状态  寄存器地址
    Reg_CSCUWorkState_Screen = 0x6CCA, //主界面, CSCU工作模式状态  寄存器地址
    Reg_StationName_Screen = 0x6D30, //主界面, 站名称  寄存器地址

    Reg_TermStateBegin_Screen = 0x0001,//主界面, 1号终端充电状态  寄存器地址
    Reg_TermStateEnd_Screen = 0x031C,//主界面, 160号终端充电状态, 共160个  寄存器地址
    Reg_TermCanIDBegin_Screen = 0x0580,//主界面, 1号终端CAN地址  寄存器地址
    Reg_TermCanIDEnd_Screen = 0x061F,//主界面, 160号终端CAN地址, 共160个  寄存器地址
    Reg_TermNameBegin_Screen = 0x0770,//主界面, 1号终端名称  寄存器地址
    Reg_TermNameEnd_Screen = 0x13DC,//主界面, 160号终端名称, 共160个  寄存器地址

    Reg_TermDetail_Normal_Screen = 0x0752,//终端详情页面, 普通版, 终端详情  寄存器地址
    Reg_TermDetail_Card_Screen = 0x6140,//终端详情页面, 刷卡版, 终端详情  寄存器地址
    Reg_TermBMS_Screen = 0x6C6A,//终端详情页面, 终端BMS信息, 共160个  寄存器地址 -------

    Reg_VINApplyCharge_Screen = 0x6729,    //VIN申请充电返回结果展示页面, 充放电返回结果  寄存器地址
    Reg_TermVIN_Screen = 0x6790,    //输入VIN后6位充电界面, VIN信息  寄存器地址

//    Reg_CardWaitIcon_Screen = 0x0033,   //刷卡等待界面, 小图标 寄存器地址
    Reg_CardWaitIcon_Screen = 0x6D00,   //刷卡等待界面, 小图标 寄存器地址

    Reg_AccountBalance_Screen = 0x6030,//刷卡返回信息界面, 刷卡余额信息 寄存器地址
    Reg_CardResult_Screen = 0x62D0, //刷卡申请充电, 结束充电, 返回结果 寄存器地址
    Reg_PageCount_Screen = 0x6108,//页面倒计时 寄存器地址
    Reg_TEUIProVer_Screen = 0x6183,//TEUI屏软件版本号  寄存器地址
    Reg_ChargeReport_Screen = 0x6BFA,//充电报告 寄存器地址 -------
    Reg_ChargeReportDetail1_Screen = 0x6C20,//充电明细1 寄存器地址 ++++++
    Reg_ChargeReportDetail2_Screen = 0x6C69,//充电明细2 寄存器地址 ++++++

    Reg_AmmeterAddr_Screen = 0x62A0,//电表地址  寄存器地址
    Reg_EnvInfo_Screen = 0x6571,//子站环境信息  寄存器地址
    Reg_AlarmSetInfo_Screen = 0x6060, //报警器设置信息  寄存器地址
    Reg_RebootCmd_Screen = 0x6EB5, //重启命令  寄存器地址
    Reg_SmartChargeIcon_Screen = 0x6273,//错峰充电状态提示图标 寄存器地址add by zjq 2017-3-8
    Reg_PeakEnable_Screen = 0x6278, //错峰充电功能使能状态 寄存器地址
    Reg_LoadEnable_Screen = 0x6279, //负荷约束功能使能状态 寄存器地址
    Reg_IPAndPort_Screen = 0x0690,//参数设置界面, 本地IP和服务器端口号  寄存器地址
    Reg_StationAddr_Screen = 0x0696,//参数设置界面, CSCU站地址  寄存器地址
    Reg_GateWay_Screen = 0x0707,//参数设置界面, 网关,DNS服务器,ZIGBEE地址  寄存器地址
    Reg_TermNum_Screen = 0x0710,//参数设置界面, 交直流终端数量  寄存器地址
    Reg_DomainName_Screen = 0x6EC6,//参数设置界面, 域名, CSCU站地址  寄存器地址

    Reg_ACPhase_Screen = 0x6E20,//交流三相相别设置界面, 相别 寄存器地址
    Reg_InLineInfo_Screen = 0x62AA,//进线侧数据  寄存器地址
    Reg_SpecFuncSet_Screen = 0x6270,//特殊功能设置 寄存器地址
    Reg_DCSpecSet_Screen = 0x6670,//直流特殊功能设置 寄存器地址

    Reg_LoadLimit_Screen = 0x6300,//负荷约束设置 寄存器地址

    Reg_PeakPageOne_Screen = 0x6800,//错峰充电设置页面1 寄存器地址
    Reg_PeakPageTwo_Screen = 0x6850,//错峰充电设置页面2 寄存器地址
    Reg_PeakDetailBegin_Screen = 0x68A0,//峰平谷尖显示, 峰 寄存器地址
    Reg_PeakDetailEnd_Screen = 0x6930,//峰平谷尖显示, 谷 寄存器地址

    Reg_FaultInfoLineBeagin_Screen = 0x6D70, //故障列表记录1  寄存器地址
    Reg_FaultInfoLineEnd_Screen = 0x6D8B, //故障列表记录10  寄存器地址

    Reg_SpecCFCDFirst_Screen = Reg_PeakDetailBegin_Screen,//错峰充电显示第一组时段(1-5)
    Reg_SpecCFCDSecond_Screen = 0x68D0,//错峰充电显示第二组时段(5-10)
    Reg_SpecCFCDSet_Screen = 0x6BAF,//错峰充电设置显示第一页
	Reg_SpecPolicyFirst_Screen = 0x6900,//本地策略显示第一组时段（1-5）
	Reg_SpecPolicySecond_Screen = 0x6930,//本地策略显示第二组时段（5-10）
	Reg_SpecPolicySet_Screen = 0x6BD4,//本地策略显示设置第一页
	Reg_SpecFGPJFirst_Screen = 0x6B28,//峰谷平尖显示第一组时段（1-5）
	Reg_SpecFGPJSecond_Screen = 0x6B46,//峰谷平尖显示第二组时段（5-10）
	Reg_SpecFGPJSet_Screen = 0x6217,//峰谷平尖显示设置第一页
	Reg_SpecGeneral_Screen = 0x6B15,//常用设置
	Reg_SpecGeneral1_Screen = 0x6BA9,//常用设置
    Reg_SpecEmergency_Screen = 0x6B20,//应急充电设置
    Reg_SpecCoupleGun_Screen = 0x6B66//双枪充电设置
}RegAddr_Screen;

/*
 * 串口屏通信协议定义
 *
 * 1：写入版本号的数据结构
 *	实例：集控器启动后发送版本号：5A A5 09 82 068A 0001 0001 0001 （版本号是1.1.1）
 */

//串口屏通信协议帧头
typedef struct _FrameHead_Screen
{
    unsigned char ucHead1;         //帧头_1 0x5A
    unsigned char ucHead2;         //帧头_2 0xA5
    unsigned char ucDataLength;      //数据长度
}__attribute__ ((packed)) FrameHead_Screen;

/**************************所有界面****************************/

//返回按钮按下 TEUI→CSCU, TEUI发送返回按钮按下指令----新
typedef struct _FrameButtonData_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned  char ucCmdType;          //串口屏控制指令类型----0x86
    unsigned short usPageNum;          //当前页面编号
    unsigned short usButNum;          //按钮编号
    unsigned short usData1;          //数据1
    unsigned short usData2;          //数据2
}__attribute__ ((packed)) FrameButtonData_Screen;

//重启指令 CSCU→TEUI, CSCU发送重启指令给屏
typedef struct _FrameRebootCmd_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned  char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //寄存器地址
    unsigned short usREnsureFlag;          //标志位, 0x0001
}__attribute__ ((packed)) FrameRebootCmd_Screen;

//页面切换 CSCU→TEUI, CSCU发送页面切换指令
typedef struct _FrameSwitchPage_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned  char ucCmdType;          //串口屏控制指令类型
    unsigned char ucAddr;          //寄存器地址
    unsigned short usPageNum;          //页面编号
}__attribute__ ((packed)) FrameSwitchPage_Screen;

//图标显示 CSCU→TEUI, CSCU发送图标显示指令
typedef struct _FrameShowIcon_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned  char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //寄存器地址
    unsigned short usIconNum;          //图标编号
}__attribute__ ((packed)) FrameShowIcon_Screen;

//页面倒计时 CSCU→TEUI, CSCU发送倒计时时间给屏
typedef struct _FramePageCount_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned  char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    unsigned short usCountTime;          //倒计时时间
}__attribute__ ((packed)) FramePageCount_Screen;

//中止页面倒计时 CSCU→TEUI, CSCU设置参数后返回
typedef struct _FramePageCountStop_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned  char ucCmdType;          //串口屏控制指令类型
    unsigned char ucAddr;          //串口屏寄存器地址--------1字节
    unsigned short usPageNum;          //页面编号
}__attribute__ ((packed)) FramePageCountStop_Screen;

//1.4 串口通信状态----串口屏通信协议 CSCU→TEUI
//周期发送1s
typedef struct _FrameSerialState_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    unsigned short usSerialState;   //01: 固定值
}__attribute__ ((packed)) FrameSerialState_Screen;

//1.7.3.1   TEUI屏对时指令----串口屏通信协议
//CSCU→TEUI   ,周期5min 对时1次
typedef struct _FrameSetTime_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned char ucAddr;          //串口屏寄存器地址--------1字节
    unsigned char ucCheckFlag;   //对时标识, 固定为0x5A
    unsigned char ucYear;   //年, 若2016, 则为16,  BCD码
    unsigned char ucMonth;   //月, BCD码
    unsigned char ucDay;   //日 BCD码
    unsigned char ucCountFlag;   //计时方式, = 24, BCD码
    unsigned char ucHour;   //时, BCD码
    unsigned char ucMin;   //分, BCD码
    unsigned char ucSec;   //秒, BCD码
}__attribute__ ((packed)) FrameSetTime_Screen;

//CSCU→TEUI   ,上电时设置屏幕灵敏度
typedef struct _FrameSetSensitivity_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned char ucAddr;          //串口屏寄存器地址--------1字节
    unsigned char ucFlagOne;   // 固定为0x5A
    unsigned char ucFlagTwo;   // 固定为0x5A
    unsigned char ucSetValue;   //设置灵敏度值
    unsigned char ucFlagThree;   // 固定为0x1E
    unsigned char ucFlagFour;   // 固定为0x08
    unsigned char ucFlagFive;   // 固定为0x0A
}__attribute__ ((packed)) FrameSetSensitivity_Screen;

/**************************主界面****************************/

//1.1 CSCU版本号----串口屏通信协议 CSCU→TEUI
//上电上传1次,  周期上传---- 60s
typedef struct _FrameCSCUProVer_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    unsigned char ucProVer[20];   //协议版本号
}__attribute__ ((packed)) FrameCSCUProVer_Screen;


//1.2 TEUI版本号----串口屏通信协议
//1.2.1   读取TEUI屏软件版本号指令 CSCU→TEUI (固定数据)
//需要时主动发送1次
typedef struct _FrameApplyTEUIProVer_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    unsigned char ucDataLength;   //数据长度
}__attribute__ ((packed)) FrameApplyTEUIProVer_Screen;

//1.2.2   TEUI屏软件版本号 TEUI→CSCU
//接收,解帧
typedef struct _FrameGetTEUIProVer_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    unsigned char ucProVer[21];   //协议版本号, ucProVer[0] = 0; ucProVer[1] - ucProVer[21], 实际数据, ASCII
}__attribute__ ((packed)) FrameGetTEUIProVer_Screen;


//1.3 网络通信状态----串口屏通信协议
//1.3.1   TEUI屏软件版本号 TEUI→CSCU
//周期发送1s
typedef struct _FrameNetState_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    unsigned short usNetState;   //0001: 通信状态；0002: 未通信状态
}__attribute__ ((packed)) FrameNetState_Screen;

//CSCU 工作模式状态(本地应急模式, 其他模式)---串口屏通信协议
//周期发送1s
typedef struct _FrameCSCUWorkState_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    unsigned short usCSCUWorkState;   //0001: ；0000:
}__attribute__ ((packed)) FrameCSCUWorkState_Screen;

//(1) 发送故障图标指令   CSCU→TEUI, 主界面显示图标
//周期发送2s
typedef struct _FrameFaultInfoIcon_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    unsigned short usFaultFlag;   //故障标志 0x0001 : 故障；0x0000 : 无故障
}__attribute__ ((packed)) FrameFaultInfoIcon_Screen;

//1.5 站名称----串口屏通信协议  CSCU→TEUI
//上电发送1次, 周期60s发送1次
typedef struct _FrameStationName_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    unsigned char ucStationName[40];   //站名称----GBK
}__attribute__ ((packed)) FrameStationName_Screen;

//1.7 刷新终端数量----串口屏通信协议
//1.7.1.1 发送终端名称   CSCU→TEUI, 主界面, 1s周期发送
typedef struct _FrameSendTermName_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    unsigned char ucTermName[9];   //终端名称-----9字节
}__attribute__ ((packed)) FrameSendTermName_Screen;

//1.7.1.2 发送终端状态   CSCU→TEUI, 主界面, 1s周期发送
//终端状态枚举
typedef enum _TermState_Screen
{
    Term_Starting_Screen = 0x01,//启动中
    Term_Linking_Screen = 0x02,//待机-枪已连接
    Term_Limiting_Screen = 0x03,//充电-限制
    Term_Pausing_Screen = 0x04,//充电-暂停
    Term_Charging_Screen = 0x05,//充电-充电中(有动画)
    Term_Swithing_Screen = 0x06,//待机-切换中
    Term_Standby_Screen = 0x07,//待机-空闲(有动画)
    Term_Offline_Screen = 0x08,//离线-未通信
    Term_Finish_Screen = 0x09,//待机-已完成
    Term_Fault_Screen = 0x0A,//故障(有动画)
    Term_Discharge_Screen = 0x0B,//放电
    Term_Waiting_Screen = 0x0D,  //等待中
    Term_Full_Screen = 0x0E,//待机-车已充满
    Term_Queue1_Screen = 0x10,//排队1
    Term_Queue2_Screen = 0x11,//排队2
    Term_Queue3_Screen = 0x12,//排队3
    Term_Queue4_Screen = 0x13,//排队4
    Term_Queue5_Screen = 0x14,//排队5
    Term_Queue6_Screen = 0x15,//排队6
    Term_Queue7_Screen = 0x16,//排队7
    Term_Slave_Screen = 0x2F,//副枪
    Term_Couple_Error = 0x30,//配对错误
    //充电弓特殊显示
    Term_Bow_Linking_Screen = 0x32,//待机-枪已连接
    Term_Bow_Standby_Screen = 0x33,//待机-空闲(有动画)
    Term_Bow_Charging_Screen = 0x34,//充电-充电中(有动画)
    Term_Bow_Starting_Screen = 0x35,//启动中
    Term_Bow_Full_Screen = 0x36,//待机-车已充满
    Term_Bow_Fault_Screen = 0x37,//故障(有动画)
    Term_Bow_Offline_Screen = 0x38,//离线-未通信
    Term_Bow_Pausing_Screen = 0x39,//充电-暂停
    Term_Bow_Finish_Screen = 0x3A //待机-已完成
}TermState_Screen;
//发送终端状态帧结构
typedef struct _FrameSendTermState_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    unsigned short usState;   //终端状态
    unsigned short usCartoonType;   //动画类型 = 终端状态枚举
    unsigned char ucReserved[6]; //预留
}__attribute__ ((packed)) FrameSendTermState_Screen;
//1.7.1.2 发送终端ID   CSCU→TEUI, 主界面, 1s周期发送
typedef struct _FrameSendTermID_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    unsigned short usCanID;   //CAN地址
}__attribute__ ((packed)) FrameSendTermID_Screen;

//(2-1) 点击终端状态框   TEUI→CSCU, 终端将状态传给集控
typedef struct _FrameGetTermState_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usCanID;          //终端CAN地址
    unsigned short usState;          //终端状态,不判断,以CSCU为准,查状态机
}__attribute__ ((packed)) FrameGetTermState_Screen;

//发送终端充电报告
typedef struct _FrameTermChargeReport_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    char    cStartSOC[6];   //开始充电SOC
    char    cStopSOC[6];   //结束充电SOC
    unsigned short usEnergy;   //充电电能
    short sStartHour;       //开始时间----小时
    short sStartMin;       //开始时间----分钟
    short sStartSec;       //开始时间----秒
    short sStopHour;       //结束时间----小时
    short sStopMin;       //结束时间----分钟
    short sStopSec;       //结束时间----秒
    short sChargeHour;       //充电用时----小时
    short sChargeMin;       //充电用时----分钟
    short sChargeSec;       //充电用时----秒
    char strStopReasonDev[40]; //设备结束原因+++++
    char strEventNo[18];//订单流水号+++++
}FrameTermChargeReport_Screen;


//发送充电明细1 ++++++
typedef struct _FrameTermChargeReportDetail1_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    char strOrderUUID[32];//内部产生的uuid
    char strCmdSrc[12];           //指令来源
    char strStartReason[12];    //开始原因
    short sStartHour;       //开始时间----小时
    short sStartMin;       //开始时间----分钟
    short sStartSec;       //开始时间----秒
    short sStopHour;       //结束时间----小时
    short sStopMin;       //结束时间----分钟
    short sStopSec;       //结束时间----秒
    short sChargeHour;       //充电用时----小时
    short sChargeMin;       //充电用时----分钟
    short sChargeSec;       //充电用时----秒
    unsigned int u32StartEnergy;   //开始充电电能
    unsigned int u32StopsEnergy;   //结束充电电能
    unsigned short usEnergy;   //总用电电能
    char strEventNo[18];//订单流水号
    char strCardNo[16];//卡号
    char strVIN[18];//VIN号
    char strCarLisence[10];//车牌号
}__attribute__ ((packed)) FrameTermChargeReportDetail1_Screen;

//发送充电明细2 ++++++
typedef struct _FrameTermChargeReportDetail2_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    char strStopReasonCSCU[40];//CSCU中止充电原因
    char strStopReasonCloud[40];//服务器中止充电原因
    char strStopReasonDev[40]; //设备结束原因+++++
    char strOrderStatus[4];//订单状态
}__attribute__ ((packed)) FrameTermChargeReportDetail2_Screen;

/**************************工程选择界面****************************/
//工程选择
typedef struct _FrameProType_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    unsigned short usIconType;   //图标内容

}FrameProType_Screen;

/**************************系统参数设置界面****************************/

//1.7.3.1   点击“系统参数设置”----串口屏通信协议
//TEUI→CSCU
//点击时下发
typedef struct _FrameParamSet_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usStaticValue;          //固定值, 0x7777
    unsigned short usSwitchPageNum;   //要切换页面编号
}__attribute__ ((packed)) FrameParamSet_Screen;

//1.7.3.2   发送交直流终端数量----串口屏通信协议
//CSCU→TEUI
typedef struct _FrameTermNum_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    unsigned short usACSinNum;   //交流单相个数
    unsigned short usACThrNum;   //交流三相个数
    unsigned short usDCNum;   //直流个数
}__attribute__ ((packed)) FrameTermNum_Screen;

//1.7.3.2   发送本地IP和服务器端口号----串口屏通信协议
//CSCU→TEUI
typedef struct _FrameIPAndPort_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    unsigned short usLocolIp[4];   //本地IP
    unsigned short usServerPort;   //服务器端口号
}__attribute__ ((packed)) FrameIPAndPort_Screen;

//1.7.3.2   发送CSCU站地址----串口屏通信协议
//CSCU→TEUI
typedef struct _FrameStationAddr_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    char chStationAddr[16];   //CSCU站地址
}__attribute__ ((packed)) FrameStationAddr_Screen;

//1.7.3.2   发送域名----串口屏通信协议
//CSCU→TEUI
typedef struct _FrameDomainName_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    char chDomainName[28];   //域名
}__attribute__ ((packed)) FrameDomainName_Screen;

//1.7.3.2   发送网关,DNS服务器,ZIGBEE地址----串口屏通信协议
//CSCU→TEUI
typedef struct _FrameGateWay_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    unsigned short usGateWay[4];   //网关
    unsigned short usDNSServer[4];   //DNS服务器
    unsigned short usZigBeeID;   //ZIGBEE地址
}__attribute__ ((packed)) FrameGateWay_Screen;

//接收 参数设置信息 TEUI→CSCU
typedef struct _FrameRecvSysParam_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usACSinNum;   //交流单相个数
    unsigned short usACThrNum;   //交流三相个数
    unsigned short usDCNum;   //直流个数
    unsigned short usLocolIp[4];   //本地IP
    unsigned short usServerPort;   //服务器端口号
    char chStationAddr[16];   //CSCU站地址
    char chDomainName[28];   //域名
    unsigned short usGateWay[4];   //网关
    unsigned short usDNSServer[4];   //DNS服务器
    unsigned short usZigBeeID;   //ZIGBEE地址
}__attribute__ ((packed)) FrameRecvSysParam_Screen;

/**************************三相相别设置界面****************************/

//1.7.3.2   发送三相相别----串口屏通信协议
//CSCU→TEUI
typedef struct _FrameACPhaseSet_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    unsigned short usPhase[50];   //相别数组
}__attribute__ ((packed)) FrameACPhaseSet_Screen;

//1.7.3.2   发送三相相别----串口屏通信协议
//CSCU→TEUI
typedef struct _FrameACPhaseRecv_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usPhase[50];   //相别数组
}__attribute__ ((packed)) FrameACPhaseRecv_Screen;

/**************************终端详情界面****************************/

//1.7.2   发送终端详情结构体----串口屏通信协议
//(1) 发送  终端详情  CSCU→TEUI
typedef struct _FrameTermDetail_Normal_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    short A_voltage;						//A相充电电压
    short A_current;						//A相充电电流
    short B_voltage;						//B相充电电压
    short B_current;						//B相充电电流
    short C_voltage;						//C相充电电压
    short C_current;						//C相充电电流
    short active_power;						//总有功功率
    short reactive_power;					//总无功功率
    short power_factor;						//总功率因数
    short neutralLine_current;   			//零线电流
    short voltage_of_dc;                    //直流侧电压
    short current_of_dc;            		//直流侧电流
    short active_electric_energy_h;				//总有电能 高字节
    short active_electric_energy_l;				//总有电能 低字节
    short reactive_electric_energy_h;			//总无功电能 高字节
    short reactive_electric_energy_l;			//总无功电能 低字节
    short voltage_unbalance_rate;			//电压不平衡率
    short current_unbalance_rate;			//电流不平衡率
    short stop_reason;                      //充电终止代码
}__attribute__ ((packed)) FrameTermDetail_Normal_Screen;

//1.7.2   发送BMS信息结构体----串口屏通信协议
//(1) 接收 终端请求BMS信息   TEUI→CSCU
typedef struct _FrameGetTermBMS_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    unsigned short usCanID;   //CAN地址
}__attribute__ ((packed)) FrameGetTermBMS_Screen;

//(2) 发送BMS信息   CSCU→TEUI
typedef struct _FrameTermBMS_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    unsigned short usBMSNeedVoltage;//BMS 电压需求
    short sBMSNeedCurrent;//BMS 电流需求
    unsigned short usBaterySOC;//电池当前SOC
    short sMaxBateryVoltage;//最高单体电池电压
    unsigned short usLowestChargeVoltage;//最低电池电压数(单体电池电压)
    unsigned short usBatteryRatedVoltage;//电池组额定电压+++
    unsigned short usBatteryRatedCapacity;//电池组额定容量+++
    unsigned short usBatteryChargeTime;//电池组充电时间+++
    unsigned short usMaxBateryTemperature;//最高电池温度
    unsigned short usLowestBatteryTemperature;//最低电池温度
    char sBMSWorkMode[8];//BMS工作模式+++
    char sBatteryType[20];//电池类型+++
}__attribute__ ((packed)) FrameTermBMS_Screen;

/**************************输入VIN后6位开始充电界面****************************/

//(1) 发送VIN信息   CSCU→TEUI
typedef struct _FrameVINInfo_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    char chVIN[17];//VIN信息
}__attribute__ ((packed)) FrameVINInfo_Screen;

//(1) 接收VIN信息   TEUI→CSCU
typedef struct _FrameVINEnd_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    char chVINEnd[6];//VIN信息
}__attribute__ ((packed)) FrameVINEnd_Screen;

/**************************进线侧信息界面****************************/

//(2) 发送电表地址给屏幕   CSCU→TEUI
typedef struct _FrameAmmeterAddr_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    unsigned short usTotalNum;						//电表总个数
    unsigned short usCountNum;						//第几个电表(list num)
    char chAmmeterAddr[12];			//电表地址:ASCII
}__attribute__ ((packed)) FrameAmmeterAddr_Screen;

//(2) 发送进线侧信息   CSCU→TEUI
typedef struct _FrameInLineInfo_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    unsigned short A_voltage;						//A相充电电压
    short A_current;						//A相充电电流
    unsigned short B_voltage;						//B相充电电压
    short B_current;						//B相充电电流
    unsigned short C_voltage;						//C相充电电压
    short C_current;						//C相充电电流
    short active_power;						//总有功功率
    short reactive_power;					//总无功功率
    short power_factor;						//总功率因数
    short neutralLine_current;   			//零线电流

    unsigned short ActiveAbsorbEnergy_h; //正向有功电能 高字节
    unsigned short ActiveAbsorbEnergy_l; //正向有功电能 低字节
    unsigned short ActiveLiberateEnergy_h; //反向有功电能 高字节
    unsigned short ActiveLiberateEnergy_l; //反向有功电能 低字节
    unsigned short ReactiveSensibilityEnergy_h; //感性无功电能 高字节
    unsigned short ReactiveSensibilityEnergy_l; //感性无功电能 低字节
    unsigned short ReactiveCapacityEnergy_h; //容性无功电能 高字节
    unsigned short ReactiveCapacityEnergy_l; //容性无功电能 低字节

}__attribute__ ((packed)) FrameInLineInfo_Screen;

/**************************子站环境信息界面****************************/

//1.7.3.1   发送子站环境信息----串口屏通信协议
//子站环境信息
typedef struct _FrameEnvInfo_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    short sTemp;          //温度
    short sHum;          //湿度
    unsigned short usAlarm[10];          //报警器编号1-10, 状态: 0-无；1-正常；2-异常
}__attribute__ ((packed)) FrameEnvInfo_Screen;

//报警器设备设置信息
typedef struct _FrameAlarmSet_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    unsigned short usAlarm[10];          //报警器编号1-10, 设置: 1-常开；2-常闭
}__attribute__ ((packed)) FrameAlarmSet_Screen;

/**************************特殊功能密码设置界面****************************/

//1.7.3.1   接收特殊功能设置密码----串口屏通信协议
//接收特殊功能设置密码
typedef struct _FrameSpecPasswd_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usFlag;          //特殊功能设置标志位
    unsigned char ucPasswd[4];   //密码值
}__attribute__ ((packed)) FrameSpecPasswd_Screen;

//1.7.3.1   接收特殊功能设置密码----串口屏通信协议
//接收特殊功能设置密码
typedef struct _FrameResetPasswd_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usFlag;          //特殊功能设置标志位
    unsigned char ucPasswdOld[4];   //密码值
    unsigned char ucPasswdNew[4];   //密码值
    unsigned char ucPasswdEnsure[4];   //密码值
}__attribute__ ((packed)) FrameResetPasswd_Screen;

/**************************特殊功能设置界面****************************/

//1.7.3.1   特殊功能设置----串口屏通信协议
//接收特殊功能设置
typedef struct _FrameSpecFuncSet_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    unsigned short usCoupleGun;        //双枪充电功能开启关闭0-关闭 1-单枪刷卡充电 2-单枪VIN/车牌号充电 3-单枪刷卡和VIN/车牌号充电
    unsigned short usVINOffline;          //断网后VIN启动充电
    unsigned short usLocalStop;          //本地结束 ---- (“普通版”终端信息结束按钮)
    unsigned short usCardType;          //刷卡类型
    unsigned short usVINAuto;          //VIN自动申请充电
    unsigned short usCardAuto;          //刷卡自动申请充电
    unsigned short usVINType;          //VIN数据类型
    unsigned short usBoardType;          //集控（CSCU）底板型号
    unsigned short usEnergyFilter;		//异常电度数过滤
}__attribute__ ((packed)) FrameSpecFuncSet_Screen;

/**************************直流特殊功能界面****************************/

//1.7.3.1   直流特殊功能设置----串口屏通信协议
typedef struct _FrameDCSpec_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    unsigned short  usGroupStrategy;          //终端群充策略
    unsigned short  usTermWorkState;          //终端工作状态
    unsigned short  usAuxType;          //终端辅助电源类型
}__attribute__ ((packed)) FrameDCSpec_Screen;

//1.7.3.1   直流特殊功能设置----串口屏通信协议
typedef struct _FrameDCSpecSet_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usCanID;          //CAN地址
    short  sGroupStrategy;          //终端群充策略
    short  sTermWorkState;          //终端工作状态
    short  sAuxType;          //终端辅助电源类型 01: 12V; 02: 24V
}__attribute__ ((packed)) FrameDCSpecSet_Screen;

/**************************负荷调度界面****************************/
//错峰充电设置, 负荷约束设置
typedef struct _FrameLoadDispatch_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    unsigned short usPeakFlag;   //错峰功能生效标识
    unsigned short usLoadFlag;   //负荷功能生效标识
}__attribute__ ((packed)) FrameLoadDispatch_Screen;

//错峰充电状态提示图标
typedef struct _FramePeakIcon_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    unsigned short usPeakFlag;   //错峰功能生效标识
}__attribute__ ((packed)) FramePeakIcon_Screen;

//错峰充电设置, 负荷约束设置
typedef struct _FrameLoadEnable_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usOptFlag;   //0001-错峰充电状态标志0002-负荷约束状态标志
    unsigned short usEnableFlag;   //生效标识, 1开启, 0关闭
}__attribute__ ((packed)) FrameLoadEnable_Screen;

//错峰充电设置记录
typedef struct _PeakRecord_Screen
{
    unsigned short usType;   //峰平谷尖类型
    unsigned short usStartH;   //开始时间:小时
    unsigned short usStartM;   //开始时间:分钟
    unsigned short usStopH;   //结束时间:小时
    unsigned short usStopM;   //结束时间:分钟
    unsigned short usSOC;   //限制SOC
    unsigned short usCurrent;   //限制电流
}__attribute__ ((packed)) PeakRecord_Screen;

//错峰充电页面详细数据
typedef struct _FramePeakShowDetail_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    PeakRecord_Screen stRecord[5]; //页面记录5条
}__attribute__ ((packed)) FramePeakShowDetail_Screen;

//错峰充电页面总数据
typedef struct _FramePeakShowPage_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    PeakRecord_Screen stRecord[10]; //页面记录10条
}__attribute__ ((packed)) FramePeakShowPage_Screen;

//负荷约束设置数据
typedef struct _FrameLoadLimit_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    unsigned short usCCUNum;          //CCU数量
    unsigned short usTotalPower;          //总功率
    unsigned short usSecurePower;          //安全功率
    unsigned short usLimitPower;          //限制功率
    unsigned short usDynamicEnable;          //动态计算使能
    unsigned short usRemoteEnable;          //远端设置使能
    unsigned short usLocalEnable;          //本地设置使能
}__attribute__ ((packed)) FrameLoadLimit_Screen;

/**************************故障列表界面****************************/

//1.7.3.1   发送一条故障列表记录数据----串口屏通信协议
//故障列表记录界面发送, 多个记录发多条
typedef struct _FrameFaultInfoLine_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    unsigned short usFaultNum;   //故障编号
    unsigned short usCanID;   //对应终端CAN地址
    unsigned short usFaultCode;   //故障代码
}__attribute__ ((packed)) FrameFaultInfoLine_Screen;

/**************************刷卡相关指令****************************/
//刷卡终端详情页面
typedef struct _FrameTermDetail_Card_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    short sVoltage;						//充电电压
    short sCurrent;						//充电电流
    short sActivePower;						//总有功功率
    short sStartHour;       //开始时间----小时
    short sStartMin;       //开始时间----分钟
    short sStartSec;       //开始时间----秒
    short sChargeHour;       //充电时间----小时
    short sChargeMin;       //充电时间----分钟
    short sChargeSec;       //充电时间----秒
    unsigned short usChargeEnergy;       //充电电量
    unsigned int uiTotalEnergy;       //总电量
    char chSOC[6];       //SOC
}__attribute__ ((packed)) FrameTermDetail_Card_Screen;

//刷卡余额信息
typedef struct _FrameAccountBalance_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    unsigned int uiBalance; //余额
}__attribute__ ((packed))FrameAccountBalance_Screen;

//刷卡等待界面
typedef struct _FrameCardWait_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    unsigned short usIcon;          //小图标内容--------2字节
}__attribute__ ((packed))FrameCardWait_Screen;

//刷卡结果
typedef struct _FrameCardResult_Screen
{
    FrameHead_Screen strHead; //帧头
    unsigned char ucCmdType;          //串口屏控制指令类型
    unsigned short usAddr;          //串口屏寄存器地址--------2字节
    char chResult[64]; //刷卡结果
}__attribute__ ((packed))FrameCardResult_Screen;

//特殊功能-常用功能
typedef struct _Frame_SpecGeneral_Screen
{
    FrameHead_Screen strHead;	//帧头
    uchar cCmdType;    			//串口屏控制指令类型
    ushort sAddr;      			//串口屏寄存器地址--------2字节
    ushort sVINOffline;			//断网后VIN启动充电
    ushort sLocalStop; 			//本地结束 ---- (“普通版”终端信息结束按钮)
    ushort sVINAuto;   			//VIN自动申请充电
    ushort sCardAuto;  			//刷卡自动申请充电
    ushort sEnergyFilter;		//异常电度数过滤
	ushort sCarPrioty;			//车辆优先级调度
	ushort sLocalPolicy;		//本地计费策略
    ushort sCardType;  			//刷卡类型
    ushort sVINType;   			//VIN数据类型
    ushort sBoardType;     		//集控（CSCU）底板型号
}__attribute__ ((packed)) Frame_SpecGeneral_Screen;

//特殊功能-常用功能
typedef struct _Frame_SpecGeneral1_Screen
{
    FrameHead_Screen strHead;	//帧头
    uchar cCmdType;    			//串口屏控制指令类型
    ushort sAddr;      			//串口屏寄存器地址--------2字节
    ushort sCuoFeng;   			//错峰充电
    ushort sFGPJ;     			//峰谷平尖
}__attribute__ ((packed)) Frame_SpecGeneral1_Screen;

//特殊功能-双抢功能
typedef struct _Frame_CoupleGun_Screen
{
    FrameHead_Screen strHead;	//帧头
    uchar cCmdType;    			//串口屏控制指令类型
    ushort sAddr;      			//串口屏寄存器地址--------2字节
    ushort coupleGun;   			//双抢充电  --0:关闭 1：打开
    ushort chargemoudle;   			//充电模式 --0：请选择  1：单枪刷卡充电 2：单枪VIN/车牌号充电  3：单枪刷卡和VIN/车牌号码充电
}__attribute__ ((packed)) Frame_CoupleGun_Screen;

//特殊功能-应急充电
typedef struct _Frame_SpecEmergency_Screen
{
    FrameHead_Screen strHead; 	//帧头
    uchar cCmdType;    			//串口屏控制指令类型
    ushort sAddr;      			//串口屏寄存器地址
	ushort sVinAuth;			//VIN鉴权
	ushort sQueueForGun;		//轮充触发-插枪
	ushort sCardAuth;			//卡号鉴权
	ushort sQueueForCard;		//轮充触发-刷卡
	ushort sCarNoAuth;			//车牌号鉴权
	ushort sQueueForCar;		//轮充触发-车辆信息
}__attribute__ ((packed)) Frame_SpecEmergency_Screen;

//错峰充电记录
typedef struct _CFCDRecord_Screen
{
	ushort sIndex;		//页面序号
    ushort sStartH;   	//开始时间:小时
    ushort sStartM;   	//开始时间:分钟
    ushort sStopH;   	//结束时间:小时
    ushort sStopM;   	//结束时间:分钟
    ushort sSOC;   		//限制SOC
    ushort sCurrent;   	//限制电流
}__attribute__ ((packed)) CFCDRecord_Screen;

//错峰充电
typedef struct _Frame_SpecCFCD_Screen
{
    FrameHead_Screen strHead; 	//帧头
    uchar cCmdType;          	//串口屏控制指令类型
    ushort sAddr;          		//串口屏寄存器地址
    CFCDRecord_Screen stRecord[5]; //页面记录5条
}__attribute__ ((packed)) Frame_SpecCFCD_Screen;

//错峰充电设置
typedef struct _Frame_SpecCFCDSet_Screen
{
    FrameHead_Screen strHead; 	//帧头
    uchar cCmdType;          	//串口屏控制指令类型
    ushort sAddr;          		//串口屏寄存器地址
	ushort sPage;				//页面号
    CFCDRecord_Screen stRecord[5]; //页面记录5条
}__attribute__ ((packed)) Frame_SpecCFCDSet_Screen;

//策略时段
typedef struct _Policy
{
	ushort sIndex;		//策略序号
    ushort sStartH;   	//开始时间:小时
    ushort sStartM;   	//开始时间:分钟
    ushort sStopH;   	//结束时间:小时
    ushort sStopM;   	//结束时间:分钟
    ushort sElectricFee;//电费
    ushort sServiceFee; //服务费
}__attribute__ ((packed)) Policy;

//计费策略显示
typedef struct _Frame_SpecPolicy_Screen
{
    FrameHead_Screen strHead; 	//帧头
    uchar cCmdType;          	//串口屏控制指令类型
    ushort sAddr;          		//串口屏寄存器地址
    Policy stPolicy[5]; 		//页面记录5条
}__attribute__ ((packed)) Frame_SpecPolicy_Screen;

//计费策略设置
typedef struct _Frame_SpecPolicySet_Screen
{
    FrameHead_Screen strHead; 	//帧头
    uchar cCmdType;          	//串口屏控制指令类型
    ushort sAddr;          		//串口屏寄存器地址
	ushort sPage;				//页面号
    Policy stPolicy[5]; 		//页面记录5条
}__attribute__ ((packed)) Frame_SpecPolicySet_Screen;

//峰谷平尖时段
typedef struct _FGPJ_Segment
{
	ushort sIndex;		//时段显示序号
	ushort sSegment;	//时段类型
    ushort sStartH;   	//开始时间:小时
    ushort sStartM;   	//开始时间:分钟
    ushort sStopH;   	//结束时间:小时
    ushort sStopM;   	//结束时间:分钟
}__attribute__ ((packed)) FGPJ_Segment;

//峰谷平尖时段设置页
typedef struct _FGPJ_SegmentSet
{
	ushort sIndex;		//时段显示序号
    ushort sStartH;   	//开始时间:小时
    ushort sStartM;   	//开始时间:分钟
    ushort sStopH;   	//结束时间:小时
    ushort sStopM;   	//结束时间:分钟
}__attribute__ ((packed)) FGPJ_SegmentSet;

//峰谷平尖
typedef struct _Frame_SpecFGPJ5_Screen
{
    FrameHead_Screen strHead; 	//帧头
    uchar cCmdType;          	//串口屏控制指令类型
    ushort sAddr;          		//串口屏寄存器地址
	FGPJ_Segment stSegment[5];
}__attribute__ ((packed)) Frame_SpecFGPJ5_Screen;

typedef struct _Frame_SpecFGPJ8_Screen
{
    FrameHead_Screen strHead; 	//帧头
    uchar cCmdType;          	//串口屏控制指令类型
    ushort sAddr;          		//串口屏寄存器地址
	ushort sPage;				//页面号
	FGPJ_SegmentSet stSegment[8];
}__attribute__ ((packed)) Frame_SpecFGPJ8_Screen;

//TEUI版本号获取
typedef struct _Frame_TEUIQueryVer_Screen
{
    FrameHead_Screen strHead; 	//帧头
    uchar ucCmdType;          	//串口屏控制指令类型
    ushort usAddr;          		//串口屏寄存器地址
    uchar cNo;				//地址数据个数
}__attribute__ ((packed)) Frame_TEUIQueryVer_Screen;

//TEUI版本号
typedef struct _Frame_TEUIVer_Screen
{
    FrameHead_Screen strHead; 	//帧头
    uchar ucCmdType;          	//串口屏控制指令类型
    ushort usAddr;          		//串口屏寄存器地址
    uchar cNo;				//地址数据个数
    char Ver[20];               //串口屏版本号
}__attribute__ ((packed)) Frame_TEUIVer_Screen;

#endif // SCREENDEF_H
