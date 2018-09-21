#ifndef SERIALSCREEN_H
#define SERIALSCREEN_H
#include <QObject>
#include <QThread>
#include <QTimer>
//#include "CommFunc_XX.h"
#include "Infotag/CSCUBus.h"
#include "SerialScreenProtocol.h"
#include "SerialPort/SerialPort.h"
#include "GeneralData/ModuleIO.h"
#include "Bus/Bus.h"
#include "ParamSet/ParamSet.h"
#include "CommonFunc/commfunc.h"
 //用到的数据库相关说明:
//1. 数据库类型: DB_MANAGE, 表名称: terminal_name_table
//2. 数据库类型: DB_MANAGE, 表名称: phasetype_param_table

//const char * const pScreenSerialNum =  "/dev/ttyS2" ;//通用设备为ttyS2,串口屏V1为ttyS1

//指令发送时间间隔枚举
typedef enum _TimeInterval_Screen
{
    TI_SetTime_Screen = 300 //300s对时一次
}TimeInterval_Screen;

//页面超时时间枚举
typedef enum _TimeOut_Screen
{
    TO_Main_Screen = 255,//主界面 超时时间
    TO_TermDetail_Normal_Screen = 30,//终端详情界面----普通版 超时时间
    TO_TermDetail_Card_Screen = 30,//终端详情界面----刷卡版 超时时间
    TO_ApplyStart_Screen = 30,  //提示:  申请开始充电中, (状态机: 输入VIN后6位, 开始充电界面) 超时时间
    TO_TermBMS_Screen = 30,//终端BMS界面 超时时间
    TO_VINStart_Screen = 60,//输入VIN充电界面 超时时间
    TO_VINResult_Screen = 3,//VIN申请充电结果展示 超时时间
    TO_LocalStop_Screen = 3,//正在结束充电中展示 超时时间
    TO_ChangePwdResult = 5,//修改密码结果界面 超时时间
    TO_CoupleChargeMannerSet = 10,//双枪充电使能充电方式选择界面 超时时间
    TO_ParamMain_Screen = 30,//设置主页面 超时时间
    TO_ProChose_Screen = 30,//工程选择主页面 超时时间
    TO_SysParamSet_Screen = 90,//系统参数设置页面 超时时间
    TO_ACPhaseSet_Screen = 90,//交流相别设置页面 超时时间
    TO_PasswdInput_Screen = 30,//密码输入页面设置页面 超时时间
    TO_SpecFuncSet_Screen = 60,//特殊功能设置页面 超时时间
    TO_InLineInfo_Screen = 60,//进线侧信息页面 超时时间
    TO_AmmeterParamFault_Screen = 5,  //展示获取电表参数失败 超时时间
    TO_EnvInfo_Screen = 90,//子站环境信息页面 超时时间
    TO_DCSpec_Screen = 90,//直流特殊功能页面 超时时间
    TO_LoadDispatchChoose_Screen = 30,//负荷调度选择界面 超时时间
    TO_PeakSet_Screen = 90, //错峰设置 超时时间
    TO_FaultInfo_Screen = 30,//故障信息列表页面 超时时间
    TO_SwitchUDisk_Screen = 30,//U盘操作页面 超时时间
    TO_CardMain_Screen = 30,//刷卡主界面 超时时间
    TO_ChargeReport_Card_Screen = 30,//充电报告界面----刷卡版 超时时间
    TO_ChargeReport_Normal_Screen = 30,  //充电报告界面----普通版 超时时间
    TO_TermOfflineState_Screen = 5,  //终端离线状态展示----普通版 超时时间

    TO_CardResult_Screen = 5,  //刷卡结果返回页面 超时时间
    TO_CardStartChargeWait_Screen = 90,  //刷卡主界面----等待开始充电结果返回页面 超时时间
    TO_SysSetResultWait_Screen__CHANGE = 60,  //系统设置界面----设置结果(重启)展示页面 超时时间
    TO_SysSetResultWait_Screen__NOCHANGE = 5,  //系统设置界面----设置结果(无变化)展示页面 超时时间
    TO_SysSetResultWait_Screen__SUCCESS = 5,  //系统设置界面----设置结果(成功)展示页面 超时时间
    TO_PasswdWrong_Screen = 5,  //特殊功能密码不正确提示页面 超时时间
    TO_SpecResetPasswdResult_Screen = 5,  //特殊功能密码重设结果提示页面 超时时间
    TO_SpecFuncSetResult_Screen = 5,  //特殊功能设置结果提示页面 超时时间
    TO_UDiskResult_Screen = 5,  //U盘操作结果提示页面 超时时间
    TO_PeakSetResult_Screen = 5,  //本地设置错峰充电参数设置结果 超时时间
    TO_ApplyAccountInfo_Screen = 60,  //提示申请用户信息中 超时时间
    TO_ShowAccountInfo_Screen = 10,  //展示用户金额 超时时间
    TO_CardStartStopResult_Screen = 5,  //展示刷卡开始,结束充电结果 超时时间
    TO_TermAbnormalStatus_Screen = 5,  //展示刷卡版终端异常状态 超时时间

    TO_LogOut_Screen =420,   //日志导出 超时时间
    TO_UpdateProgram_Screen =60   //版本升级 超时时间
}TimeOut_Screen;

//串口读取类
class cSerialRead : public QObject
{
    Q_OBJECT
public:
    cSerialRead(cSerialPort * pSerialPortIn);
    ~cSerialRead();
    //读取串口数据
    void ReadSerialData();
public:
    bool bEndWork;
private:
    unsigned int uiRecvBufSize;
    //串口类指针
    cSerialPort * pSerialPort;
    //数据接收指针
    unsigned char * pSerialData;

signals:
    void sigRecvSerialData(unsigned char * pSerialData,int iLength);

public slots:
    void ProcStartWork();
};

//串口写入类
class cSerialWrite : public QObject
{
    Q_OBJECT
public:
    cSerialWrite(cSerialPort * pSerialPortIn);
    //读取串口数据
    void WriteSerialData();

private:
    //串口类指针
    cSerialPort * pSerialPort;
public slots:
    void ProcSendSerialData(unsigned char *pSerialData, int iLen);
};

//串口屏类(包括逻辑)
class cSerialScreen : public CModuleIO
{
    Q_OBJECT
public:
    cSerialScreen();
    ~cSerialScreen();

    //根据配置选项初始化
    int InitModule( QThread* pThread);
    //注册设备到总线
    int RegistModule();
    //启动模块
    int StartModule();
    //停止模块
    int StopModule();
    //模块工作状态
    int ModuleStatus();

private:
    //初始化
    void Init();
    //初始化终端名称列表(写数据库)
//    void InitTermNameDB();
    //初始化终端名称图(写TermNameMap)
    void InitTermNameMap();
    void InitTermNameMapShow();
    void InitTermNameMapMulti();

    //点击终端状态逻辑----刷卡版
    void CheckTermState_Card(TerminalStatus &stTerm);
    //点击终端状态逻辑----普通版
    void CheckTermState_Normal(TerminalStatus &stTerm);

    //查询终端充电开始原因
   // int queryStartReason(CHARGE_STEP stChargeStep);

    //状态机控制----切换至主界面
    void Ctrl_SwitchMain(unsigned char ucCanID);
    //状态机控制----切换至终端详情界面----普通版
    void Ctrl_SwitchTermDetail_Normal(unsigned char ucCanID);
    void Ctrl_SwitchVINManualCharge_Normal(unsigned char ucCanID);
    void Ctrl_SwitchVINCardCharge_Normal(unsigned char ucCanID);
    void Ctrl_SwitchCardCharge_Normal(unsigned char ucCanID);
    //状态机控制----切换至终端详情界面----刷卡版
    void Ctrl_SwitchTermDetail_Card(unsigned char ucCanID);
    //状态机控制----切换至终端BMS界面
    void Ctrl_SwitchTermBMS(unsigned char ucCanID);
    //状态机控制----切换至输入VIN充电界面----普通版专有
    void Ctrl_SwitchVINStart(unsigned char ucCanID);
    //状态机控制----切换至设置主页面
    void Ctrl_SwitchParamMain(unsigned char ucCanID);
    //状态机控制----切换至修改密码成功主页面
    void Ctrl_SwitchChangePwdSuccess(unsigned char ucCanID);
    //状态机控制----切换至修改密码失败主页面
    void Ctrl_SwitchChangePwdFail(unsigned char ucCanID);
    //状态机控制----切换至工程选择主页面
    void Ctrl_SwitchProChose(unsigned char ucCanID);
    //状态机控制----切换至系统参数设置页面
    void Ctrl_SwitchSysParamSet(unsigned char ucCanID);
    //状态机控制----切换至交流相别设置页面
    void Ctrl_SwitchACPhaseSet(unsigned char ucCanID);
    //状态机控制----切换至密码输入页面
    void Ctrl_SwitchPasswdInput(unsigned char ucCanID);
    //状态机控制----切换至特殊功能设置页面
    void Ctrl_SwitchSpecFuncSet(unsigned char ucCanID);
    //状态机控制----切换至进线侧信息页面
    void Ctrl_SwitchInLineInfo(unsigned char ucCanID);
    //状态机控制----切换至环境信息页面
    void Ctrl_SwitchEnvInfo(unsigned char ucCanID);
    //状态机控制----切换至直流特殊功能界面
    void Ctrl_SwitchDCSpec(unsigned char ucCanID);
    //状态机控制----切换至负荷调度选择界面
    void Ctrl_SwitchLoadDispatchChoose(unsigned char ucCanID);
    //状态机控制----切换至故障信息列表页面
    void Ctrl_SwitchFaultInfo(unsigned char ucCanID);
    //状态机控制----切换至U盘操作页面
    void Ctrl_SwitchUDisk(unsigned char ucCanID);
    //状态机控制----切换至刷卡主界面
    void Ctrl_SwitchCardMain(unsigned char ucCanID);
    //状态机控制----切换至充电报告界面----刷卡版
    void Ctrl_SwitchChargeReport_Card(unsigned char ucCanID);
    //状态机控制----切换至充电报告界面----普通版
    void Ctrl_SwitchChargeReport_Normal(unsigned char ucCanID);
    //状态机控制----切换至小票机是否打印主界面
    void Ctrl_SwitchTicketPrintMain(unsigned char ucCanID);
    //状态机控制----切换至小票机缺纸提示界面
    void Ctrl_SwitchNoPaperMain(unsigned char ucCanID);
    //状态机控制----切换至单双枪充电选择单枪充电方式页面
    void Ctrl_SwitchCoupleChargeManner(unsigned char ucCanID);

    //检查终端名称数据库内容
//    bool CheckDBTermName();
    //校验帧头
    bool CheckFrameHead(unsigned char * pData, int iLength);
    //检查串口屏工作状态
    void CheckWorkState();
    //检查串口屏页面超时
    void CheckPageTimeOut();
    //检查串口屏页面等待切换
    void CheckPageWaitTime();

    //发送 向控制总线发送数据
    void SendCenterData(InfoMap ToCenterMap, InfoAddrType enType);
    //发送 相控制总线发送升级请求
    void SendUpdateApply();
    //发送 日志导出请求
    void SendLogOutApply();

    //发送 刷卡请求, 让读卡器开始读卡
    void SendCardNumApply();
    //发送 刷卡结束请求, 让读卡器停止读卡
    void SendCardApplyStop();
    //发送 刷卡申请账户信息, 发送到总线
    void SendCardApplyAccountInfo();
    //发送 刷卡开始充电请求, 发送到总线
    void SendCardStartCharge();
    //发送 刷卡结束充电请求, 发送到总线
    void SendCardStopCharge();

    //发送 错峰充电设置, 发送到总线
    void SendPeakSet();
    //发送 VIN后6位申请充电, 发送到总线
    void SendVINApplyCharge();
    //发送 屏幕申请结束充电, 发送到总线
    void SendApplyStopCharge();

    //发送 直流特殊功能相关属性申请, 发送到总线
    void SendDCSpecApply();

    //发送 终端名称申请, 发送到总线
    void SendTermNameApply();

    //解析 主界面接收指令
    void ParseFrameMainPage(unsigned char * pData, int iLength);
    //解析 主界面下按钮
    void ParseFrameButtonMain(unsigned char * pData, int iLength);
    //解析 终端状态命令----主页面
    void ParseFrameTermState(unsigned char * pData, int iLength);
    //解析  TEUI版本号
    void ParseFrameTEUIVer(unsigned char * pData);

    //解析 VIN开始充电主页面
    void ParseFrameVINStartMain(unsigned char * pData, int iLength);
    //解析 VIN开始充电主页面--按钮按下
    void ParseFrameVINStartMainButton(unsigned char * pData, int iLength);
    //解析 VIN后6位
    bool ParseFrameVINEndInput(unsigned char * pData, int iLength);

    //解析 刷卡主界面
    void ParseFrameCardMain(unsigned char * pData, int iLength);
    //解析 刷卡主界面--按钮按下
    void ParseFrameCardMainButton(unsigned char * pData, int iLength);

    //解析 终端详情界面接收指令----普通版
    void ParseFrameTermDatil_Normal(unsigned char * pData, int iLength);
    //解析 终端详情界面按钮----普通版
    void ParseFrameButtonTermDatil_Normal(unsigned char * pData, int iLength);
    //解析 BMS信息申请
    unsigned char ParseFrameBMSApply(unsigned char * pData, int iLength);
    //解析 BMS页面下按钮
    void ParseFrameButtonBMSInfo(unsigned char * pData, int iLength);

    //解析 终端详情界面接收指令----刷卡版
    void ParseFrameTermDatil_Card(unsigned char * pData, int iLength);
    //解析 终端详情界面按钮----刷卡版
    void ParseFrameButtonTermDatil_Card(unsigned char * pData, int iLength);

    //解析 充电报告界面接收指令----普通版
    void ParseFrameChargeReport_Normal(unsigned char * pData, int iLength);
    //解析 充电报告界面接收指令----刷卡版
    void ParseFrameChargeReport_Card(unsigned char * pData, int iLength);
    //解析 充电报告界面按钮
    void ParseFrameButtonChargeReport(unsigned char * pData, int iLength);

    //解析 设置主页面下主函数
    void ParseFrameSetMain(unsigned char * pData, int iLength);
    //解析 设置主页面下按钮
    void ParseFrameButtonParamMain(unsigned char * pData, int iLength);

    //解析 工程选择界面下按钮
    void ParseFrameProSetButton(unsigned char * pData, int iLength);

    //解析 系统设置界面命令
    void ParseFrameSysSetMain(unsigned char * pData, int iLength);
    //解析 系统设置界面命令--命令下发
    ParamSetPage_Screen ParseFrameSysSet(unsigned char * pData, int iLength);
    //解析 系统设置界面命令--按钮按下
    void ParseFrameButtonSysSet(unsigned char * pData, int iLength);

    //解析 交流相别设置界面命令
    void ParseFrameACPhaseMain(unsigned char * pData, int iLength);
    //解析 交流相别设置界面命令--按钮按下
    void ParseFrameButtonACPhase(unsigned char * pData, int iLength);
    //解析 交流相别设置界面命令
    void ParseFrameACPhaseSet(unsigned char * pData, int iLength);

    //解析 密码登陆设置主界面
    void ParsePasswdMain(unsigned char * pData, int iLength);
    //解析 特殊功能设置密码主界面--按钮按下
    void ParsePasswdMainButton(unsigned char * pData, int iLength);
    //解析 特殊功能设置密码
    bool ParseSpecPasswd(unsigned char * pData, int iLength);
    //解析 重新设置密码
    bool ParseResetPasswd(unsigned char * pData, int iLength);
    //解析 密码修改成功主界面
    void ParsePasswdChangeResult(unsigned char * pData, int iLength);   //add by zrx
    //解析 小票机打印主界面
    void ParseTicketPrint(unsigned char * pData, int iLength);   //add by zrx
    void ParseCardChargeRequest_Normal(unsigned char * pData, int iLength);


    //解析 特殊功能设置主界面
    //解析　双枪充电使能时充电方式设置
    void ParseCoupleChargeMannerSet(unsigned char * pData, int iLength);
    void ParseSpecFuncMain(unsigned char * pData);
    //解析 特殊功能设置主界面--按钮按下
    void ParseFrameButtonSpecFunc(unsigned char * pData);

    //解析 进线侧数据主界面
    void ParseInLineInfoMain(unsigned char * pData, int iLength);
    //解析 进线侧数据主界面--按钮按下
    void ParseInLineButton(unsigned char * pData, int iLength);

    //解析 环境检测主界面
    void ParseEnvInfo(unsigned char * pData, int iLength);
    //解析 报警器设备设置信息----环境检测主界面
    void ParseAlarmSet(unsigned char * pData, int iLength);
    //解析 环境检测主界面--按钮按下
    void ParseEnvInfoButton(unsigned char * pData, int iLength);

    //解析 直流特殊功能设置主界面
    void ParseDCSpecMain(unsigned char * pData, int iLength);
    //解析 直流特殊功能设置主界面--按钮按下
    void ParseDCSpecButton(unsigned char * pData, int iLength);
    //解析 直流特殊功能设置
    void ParseDCSpecSet(unsigned char * pData, int iLength);

    //解析 负荷调度选择界面
    void ParseFrameLoadDispatchMain(unsigned char * pData, int iLength);
    //解析 负荷调度选择界面--按钮按下
    void ParseLoadDispatchButton(unsigned char * pData, int iLength);
    //解析 负荷调度,错峰充电使能信息指令
    void ParseLoadDispatchEnable(unsigned char * pData, int iLength);
    //解析 错峰充电设置界面
    void ParseFramePeakSet(unsigned char * pData, int iLength);
    //解析 负荷限制功能设置指令
    void ParseFrameLoadLimit(unsigned char * pData, int iLength);

    //解析 故障列表主界面
    void ParseFaultInfo(unsigned char * pData, int iLength);
    //解析 故障列表界面--按钮按下
    void ParseFaultInfoButton(unsigned char * pData, int iLength);

    //解析 U盘操作页面--按钮按下
    void ParseUDiskButton(unsigned char * pData, int iLength);

    //解析 刷卡主页面
    void ParseCardMain(unsigned char * pData, int iLength);

    //解析 总线接收U盘处理结果
    bool ParseCenterUdiskResult(InfoMap CenterMap);
    //解析 本地设置错峰充电参数设置结果
    bool ParseCenterPeakSetResult(InfoMap CenterMap);

    //解析 总线接收刷卡卡号
    bool ParseCenterCardNumber(InfoMap CenterMap);
    //解析 总线接收账户信息
    bool ParseCenterAccountInfo(InfoMap CenterMap);
    //解析 总线接收刷卡外部申请, 开始充电, 结束充电结果 ---- 外部结果(平台返回), ucType: 1, 开始充电; 2, 结束充电
    bool ParseCenterCardOutResult(InfoMap CenterMap, unsigned char ucType);
    //解析 总线接收刷卡内部申请, 开始充电, 结束充电结果 ---- 内部结果(充电服务模块返回), ucType: 1, 开始充电; 2, 结束充电
    bool ParseCenterCardInResult(InfoMap CenterMap, unsigned char ucType);
    //解析 总线接收VIN内部申请开始充电的结果
    bool ParseCenterVINResult(InfoMap CenterMap);
    //解析 总线接收终端名称
    bool ParseCenterTermName(InfoMap CenterMap);

    //处理 点击终端状态接收指令----主页面
    void GetTermState(unsigned char ucCanID);
    //处理 获取系统设置----系统设置界面
    ParamSetPage_Screen GetSysParamSet();
    //处理 获取特殊功能设置----特殊功能设置界面
    SpecFuncSet_Screen GetSpecFunSet();
    //系统参数比较, 返回FALSE, 无变化；返回TRUE, 有变化----系统设置界面
    bool SysParamCmp(ParamSetPage_Screen & stNew);
    //获取刷卡返回代码
//    QByteArray GetCardResult(unsigned char ucRetCode, unsigned char ucType);
    //获取配置信息
    void QueryParamInfo();

	void Ctrl_SwitchPage(int iPageTo, int iHoldTime = 30, bool bAsParent = true, int iPageReturn = MENU_PAGE_MAIN);
    bool ParseSpecFunc(uchar * pData, int iPage);
	bool ParseSpecGeneral(uchar * pData);
	bool ParseSpecEmergency(uchar * pData);
	bool GetSpecCFCD();
	bool ParseSpecCFCD(uchar * pData);
	void ParseSpecPolicy(uchar * pData);
	bool GetSpecPolicy();
	bool SaveSpecPolicy();
	bool ParseSpecFGPJ(uchar * pData);
	bool GetSpecFGPJ();
	bool SaveSpecFGPJ();
	//bool ParseSpecDoubleGun(uchar * pData, int iLen);
    bool ParseTicketPrintResult(InfoMap CenterMap);  //解析 小票机打印结果1-打印 2-缺纸
    bool ParseSpecDoubleGun(uchar * pData);

    //VIN申请开始充电结果
    void ShowVINApplyChargeResult(unsigned char ucResult);
    bool ParseCenterVinInResult(InfoMap CenterMap, unsigned char ucType);

private:
    //内部定义数据
    bool bWorkStartFlag;//开始工作标识位
    bool bCardResultFlag;//刷卡结果标识位
    bool bVinResultFlag;//刷卡结果标识位

    unsigned char ucVINResultStep;//VIN申请充电步骤
    bool bPageWaitFlag;//页面切换等待计时标志位
    bool bBanlanceFlag;//余额查询标志位
    bool bScreenRebootFlag; //系统参数设置屏幕重启标志位
    //bool bTicketPrint[250];  //小票机申请一次标志位add by zrx

    Menu_Status ScreenState;//页面状态机
    cSerialPort * pSerialPort;//串口类
    cSerialRead * pSerialRead;//读取串口操作类
    cSerialWrite * pSerialWrite;//写入串口操作类
    cSerialScreenProtocol * pProtocol;//串口屏协议指针
    QThread * pSerialReadThread; //串口接收线程
    QTimer * pSecTimer;
    QTimer * pMSecTimer;
    int iPageSwitchCount;//页面切换计数器
    int iSetTimeCount; //对时计数器
    int iPageWaitCount;  //页面等待计数器
    int iPageWaitTime;  //页面等待计时时间
    unsigned char ucDCSpecAckStep;//直流特殊功能回复设置所在步骤
    int iPageLoadDispatchChoose;  //当前负荷调度界面标志位  1-负荷调度主界面209(0XD1)  2-错锋充电设置一览表173(0xAD)  3-锋电设置界面175(0xAF)

    //部分显示数据存储
    stServer0Config ServerConfig;        //服务器参数设置缓存
    stNetConfig NetConfig;          //网口信息配置缓存
    stCSCUSysConfig cscuSysConfig;       //CSCU参数设置缓存
    stChargeConfig ChargeConfig;    //充电功能信息缓存
    stAllAmmeterConfig AllAmmeterConfig;  //全部电表参数
    stThreePhaseTypeConfig ThreePhaseTypeConfig;//三相相别全部参数
    stIOConfig IOConfig;  //IO配置信息
    PbServerConfig protobufServer;
    stSmartChargeConfig SmartChargeConfig;//错峰充电配置
    stPowerLimitConfig PowerLimitConfig;//负荷约束配置
    stAllTPFVConfig AllTPFVConfig;  //峰平谷尖配置
    stTPFVConfig PeakSet[20];   //错峰设置数据接收

	LocalPolicyConfig m_stPolicyCache[20];//本地策略时段缓存
	AllLocalPolicyConfig m_stAllPolicy;//本地策略查询记录
	FGPJConfig m_stFGPJCache[32];//峰谷平尖缓存
	AllFGPJConfig m_stAllFGPJ;//峰谷平尖查询记录

    TermNameMap  NameMap;     //终端名称图
    TermNameMap  NameMapShow;     //终端名称图hd显示
    QByteArray CardNum;//刷卡卡号
    QByteArray ScanCodeIDNum;//扫码客户ID
    unsigned char ucCardStep;//1: 申请卡片信息; 2: 申请充电中; 3: 结束充电中

    //VIN缓存(用于VIN后6位充电)
    char chVINCach[17];
    //外部输入参数
    DevCache * pDevCache;
    ParamSet * pParamSet;
    DBOperate * pDBOperate;
    Log * pLog;
signals:
    void sigToBus(InfoMap, InfoAddrType);

public slots:
    //接收串口数据
    void ProcRecvSerialData(unsigned char * pData, int iLength);
    //接收控制中心数据
    void slotFromBus(InfoMap RecvCenterMap, InfoAddrType enType);
    //开始工作
    void ProcStartWork();
private slots:
    void ProcOneSecTimeOut();
};

#endif // SERIALSCREEN_H
