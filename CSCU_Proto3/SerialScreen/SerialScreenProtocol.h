#ifndef SERIALSCREENPROTOCOL_H
#define SERIALSCREENPROTOCOL_H
#include <QObject>
#include <QDateTime>
#include <QMap>
#include <QTextCodec>
#include <netinet/in.h>
#include "CommonFunc/commfunc.h"
#include "GeneralData/GeneralData.h"
#include "ScreenDef.h"
#include "Infotag/CSCUBus.h"
#include "SerialPort/SerialPort.h"
#include "DevCache/DevCache.h"
#include "ParamSet/ParamSet.h"
//#include "CommFunc_XX.h"

//参数设置界面----参数设置界面结构体
typedef struct _ParamSetPage_Screen
{
    unsigned short usACSinNum;   //交流单相个数
    unsigned short usACThrNum;   //交流三相个数
    unsigned short usDCNum;   //直流个数
    unsigned short usLocolIp[4];   //本地IP
    unsigned short usServerPort;   //服务器端口号
    char chStationAddr[17];   //CSCU站地址
    char chDomainName[29];   //域名
    unsigned short usGateWay[4];   //网关
    unsigned short usDNSServer[4];   //DNS服务器
    unsigned short usZigBeeID;   //ZIGBEE地址
}ParamSetPage_Screen;

//特殊功能设置界面----特殊功能设置结构体
typedef struct _SpecFuncSet_Screen
{
    unsigned short usCoupleGun;         //双枪充电设置
    unsigned short usVINOffline;          //断网后VIN启动充电
    unsigned short usLocalStop;          //本地结束 ---- (“普通版”终端信息结束按钮)
    unsigned short usCardType;          //刷卡类型
    unsigned short usVINAuto;          //VIN自动申请充电
    unsigned short usCardAuto;          //刷卡自动申请充电
    unsigned short usVINType;          //VIN数据类型
    unsigned short usBoardType;          //集控（CSCU）底板型号
    unsigned short usEnergyFilter;		//异常电度数过滤
}__attribute__ ((packed)) SpecFuncSet_Screen;
//页面属性
typedef struct
{
    unsigned char uc_can_addr;//页面CAN地址
    unsigned char uc_hold_time;//页面持续时间,FF为长页面
    unsigned int uiPageTimeOut;//超时时间
    unsigned char uc_page_num_return;//点击返回按钮应切换的页面
}PAGE_VALUE;

//页面状态机
typedef struct
{
    unsigned short us_page_num;//页面编号
    PAGE_VALUE st_page_value;//页面属性
}Menu_Status;

//图标编号枚举
enum ICON_PAGE
{
    ICON_SET_SPEC_SUCCESS = 0x05, //特殊功能 设置成功图标
    ICON_SET_SPEC_FAILED = 0x06, //特殊功能 设置失败图标
    ICON_INLINE_PARAM_FAILED = 0x07 //进线侧 获取进线侧配置失败图标
};

//vin申请充电图标枚举
typedef enum _ICON_VINResult_Screen
{
    ICON_VIN_CHECK_ERROR = 0x09,//VIN匹配失败
    ICON_VIN_CHARGE_SUCCESS = 0x01, //充电成功
    ICON_VIN_CHARGE_FAILED = 0x02 //充电失败
}ICON_VINResult_Screen;

typedef enum
{
    BUTTON_TYPE_RETURN = 1,//返回按钮
    BUTTON_TYPE_CHARGING_INFO = 10,//充电信息页面切换按钮 109
    BUTTON_TYPE_CHARGING_INFO_DETAIL = 11,//充电详细信息页面切换按钮 8
    BUTTON_TYPE_BMS_INFO = 12,//BMS信息页面 9
    BUTTON_TYPE_STOP_CHARGE = 13,//结束充电按钮
    BUTTON_TYPE_CHARGE_REPORT = 14,//切换到界面45-充电报告
    BUTTON_TYPE_CHARGE_REPORT_DETAIL = 15//切换到界面145-充电明细
}BUTTON_TYPE;

//页面编号枚举
enum MENU_PAGE
{
	MENU_PAGE_SPEC_GENERAL = 0x63,  //特殊功能常用功能
	MENU_PAGE_SPEC_EMERGENCY = 0x64,//特殊功能应急充电
	MENU_PAGE_SPEC_CFCD = 0x65,//特殊功能错峰充电显示页
	MENU_PAGE_SPEC_POLICY = 0x66,//特殊功能计费策略显示页
	MENU_PAGE_SPEC_FGPJ = 0x67,//特殊功能峰谷平尖显示页
	MENU_PAGE_SPEC_DOUBLE_GUN = 0x68,//特殊功能双枪
	MENU_PAGE_SPEC_FGPJ_SET = 0x69,//峰谷平尖设置页
	MENU_PAGE_SPEC_CFCD_SET = 0x6A,//错峰设置页
	MENU_PAGE_SPEC_POLICY_SET = 0x72,//本地策略设置页

    MENU_PAGE_FAULT_INFO = 0x0C,    //故障列表界面, 发送故障列表信息, 加延时

    MENU_PAGE_PASSWD_WRONG = 0xA0,  //特殊功能密码不正确提示页面, 切换页面, 加延时
    MENU_PAGE_SET_SPEC_RESULT = 0xC8, //特殊功能设置结果显示页面, 切换页面, 加延时
    MENU_PAGE_SET_SPEC_FUNC = 0xC9,  //特殊功能设置页面, 切换页面, 加延时
    MENU_PAGE_SHOW_PASSWD = 0xCC,  //特殊功能密码输入页面, 加延时(显示用,非主界面)
    MENU_PAGE_RESET_SPEC_FAILED = 0xCD,  //特殊功能设置页面, 重新设置密码成功, 加延时
    MENU_PAGE_RESET_PASSWD_SUCCESS = 0xCE,  //特殊功能设置页面, 重新设置密码失败, 加延时
    MENU_PAGE_LOG_OUT = 0x2B, //U盘插入, 日志导出中, 延时240s (显示用, 非主界面)
    MENU_PAGE_LOG_OUT_SUCCSESS = 0x35, //U盘插入, 日志导出成功, (显示用, 非主界面)
    MENU_PAGE_LOG_OUT_FAILED = 0x36, //U盘插入, 日志导出失败, (显示用, 非主界面)

    MENU_UPDATE_PROGRAM = 0x37, //U盘插入, 程序升级中, 切换页面, 延时240s (显示用, 非主界面)
    MENU_UPDATE_SUCCESS = 0x52, //U盘插入, 程序升级成功, 切换页面, 延时240s (显示用, 非主界面)
    MENU_UPDATE_FAILED = 0x53, //U盘插入, 程序升级失败, 切换页面, 延时240s (显示用, 非主界面)

    MENU_PAGE_PARAM_NOCHANGE = 0x28, //系统参数设置返回结果界面, 参数无变化返回, (显示用, 非主界面)
    MENU_PAGE_PARAM_CHANGE = 0x41, //系统参数设置返回结果界面, 参数有变化返回,  (显示用, 非主界面)
    MENU_PAGE_PARAM_SET_SUCCESS = 0x5B, //系统参数设置返回结果界面, 参数设置成功 (显示用, 非主界面)
    MENU_PAGE_MAIN = 0x00, //主界面
    MENU_PAGE_SET_MAIN = 0x04,//页面04H，设置主页面
    MENU_PAGE_TERM_INFO_NORMAL = 31,//详细信息(不带本地结束) -----------
    MENU_PAGE_TERM_INFO_NORMAL_LOCAL_STOP = 8,//详细信息(带本地结束) -----------
    MENU_PAGE_LOCAL_STOP_WAIT = 0x4C,//提示: 本地结束按下,等待结束充电
    MENU_PAGE_TERM_INFO_CARD = 30,// 充电信息, 刷卡版 (增加两个按钮) ++++++
    MENU_PAGE_VIN_CARD_CHARGE_NORMAL = 0x5F,//双枪充电使能，单枪刷卡和vin/车牌号充电(手动确认)
    MENU_PAGE_VIN_MANUAL_CHARGE = 0x60,//双枪充电使能，单枪vin/车牌号充电(需手动确认)
    MENU_PAGE_CARD_CHARGE_NORMAL = 0x61,//双枪充电使能，单枪刷卡充电(需手动确认)
    MENU_PAGE_COUPLECHARGE_MANNER = 0x62, //单双枪功能开启时单枪充电方式
    MENU_PAGE_TERM_INFO_CARD_STOP_BUTTON = 109,// 结束按钮充电详情, 刷卡版, (新增页,4个按钮) ++++++
    MENU_PAGE_CHARGE_CARD_REPORT_DETAIL = 145, //详细充电报告页面, 刷卡版 (新增页,2个按钮) ++++++
    MENU_PAGE_CHARGE_CARD_REPORT = 45, //充电报告页面, 刷卡版 (增加一个按钮) ++++++
    MENU_PAGE_CHARGE_NORMAL_REPORT = 0x2F, //充电报告页面, 普通版 -----------
    MENU_PAGE_VIN_START_CHARGE = 0xAA,  //输入VIN后6位, 开始充电界面 ----普通版专有 -----------
    MENU_PAGE_VIN_APPLY_START = 0x92,  //提示:  申请开始充电中, (状态机: 输入VIN后6位, 开始充电界面) ----普通版专有
    MENU_PAGE_VIN_APPLY_START_RESULT = 0x94,  //提示:  申请开始充电结果, (状态机: 输入VIN后6位, 开始充电界面) ----普通版专有

    MENU_PAGE_BMS = 32,//BMS详情页面 (新增数据项,4个按钮)++++++
    MENU_PAGE_BMS_STOP_BUTTON  = 9,//BMS详情页面,带结束充电按钮++++++
    MENU_PAGE_IN_LINE_INFO = 0x0A,//进线侧信息页面
    MENU_PAGE_SET_SYSTEM = 0x58,//系统参数设置页面
    MENU_PAGE_SET_PHASE = 0x59,//相别设置页面
    MENU_PAGE_PASSWD_INPUT = 0x9F,//密码输入页面
    MENU_PAGE_ENV_INFO = 0x7F,//子站环境信息页面
    MENU_PAGE_DC_SPEC = 0x7B,//直流特殊功能界面
    MENU_PAGE_UDISK = 0x2A,//U盘操作页面
    MENU_PAGE_SET_PRO = 0x71,//工程选择页面
    MENU_PAGE_LOAD_DISPATCH_CHOOSE = 0xD1, //负荷调度选择界面
    MENU_PAGE_SWIPE_CARD_MAIN = 110,//刷卡主页面 "请刷卡,获取余额信息"
    MENU_PAGE_CARD_RESULT = 62,//页面3EH, 刷卡结果返回页面
    MENU_PAGE_PROMPT_OFFLINK = 57,//页面39H, 提示: 枪未连接 (状态机:主界面)
    MENU_PAGE_PROMPT_OFFLINE = 61,//页面3DH, 提示: 离线 (状态机:主界面)
    MENU_PAGE_APPLAY_ACCOUNT_INFO = 72,//页面48H, 申请用户信息中(状态机:刷卡主界面)
    MENU_PAGE_POST_PAID_ACCOUNT = 78, //页面0x4EH, 提示: 后付费卡(状态机:刷卡主界面)
    MENU_PAGE_POST_PAID_ACCOUNT_BALANCE_ENQUIRY = 79, //页面0x70H, 提示: 后付费卡-余额查询(状态机:刷卡主界面)
    MENU_PAGE_NORMAL_ACCOUNT = 80, //"卡内余额X元,是否开始充电" (状态机:刷卡主界面)
    MENU_PAGE_NORMAL_ACCOUNT__BALANCE_ENQUIRY = 112, // "卡内余额X元" 仅查询余额-(状态机:刷卡主界面)
    MENU_PAGE_PEAK_STAGGER = 0xAD, //页面0xADH, 错峰充电显示页面
    MENU_PAGE_PEAK_SET_END = 0xB2, //页面0xB2H, 错峰充电设置界面, 最后按钮设置(状态机: 错峰充电显示页面)
    MENU_PAGE_PEAK_SET = 0xAF, //页面0xAFH, 错峰充电设置页面
    MENU_PAGE_LOAD_LIMIT = 0xD2, //页面0xD2H, 负荷约束电设置页面
    
    //小票机功能添加界面 add by zrx 2017-08-09
    MENU_PAGE_TICKET_PRINT_MAIN = 0x21,    //小票机打印提示主界面
    MENU_PAGE_TICKET_NOPAPER = 0x22   //缺纸提示
};

//串口接收数据解析在cSerialScreen类中实现,协议类主要工作为生成各帧并发送
class cSerialScreenProtocol : public QObject
{
    Q_OBJECT
public:
    cSerialScreenProtocol(DevCache * pDevCacheIn, DBOperate * pDBOperateIn, ParamSet *pSetting, Log *pLogIn);
    bool QueryDevStopReasonName(unsigned char ucCode, QString &StringName);
    bool QueryCSCUStopReasonName(unsigned char ucCode, QString &StringName);
    bool QueryCloudStopReasonName(unsigned char ucCode, QString &StringName);
    bool QueryOrderStatusName(int OrderStatus, QString &StringName);
    bool QueryCmdSrcName(unsigned char ucCode, QString &StringName);
    bool QueryStartReasonName(unsigned char ucCode, QString &StringName);
    bool QueryStartReasonCode(CHARGE_STEP &stChargeStep, unsigned char  &code);

    //发送屏幕重启指令
    void SendFrameScreenReboot();
    //发送 对时指令
    void SendFrameSetTime();
    //发送 屏幕灵敏度选择     add by zjq
    void SendFrameSetSensitivity();
    //发送 串口通信状态
    void SendFrameSerialState();
    //主界面----发送相关数据
    void SendMainPageData(TermNameMap &NameMap, stCSCUSysConfig &cscuConfig, char *pCSCUVer,TermNameMap  &NameMapShow);
    //发送 查询TEUI版本号
    void SendFrameQueryTEUIVer();

    //系统参数设置界面----发送设置信息
    void SendSysParamData(ParamSetPage_Screen &stParamSetPage);
    //发送 交直流终端数量----(系统参数设置界面, 重启后发送)
    void SendFrameTermNum(ParamSetPage_Screen &stParamSetPage);

    //发送 工程选择界面----当前工程类型
    void SendFrameProType(unsigned char ucProType);

    //相别设置界面----发送交流三项相别设置
    void SendFrameACPhaseSet(stThreePhaseTypeConfig &ThreePhaseTypeConfig);

    //终端-详细信息----发送相关数据
    void SendTermPageData(unsigned char ucCanID, unsigned char ucType, stThreePhaseTypeConfig &ThreePhaseTypeConfig);
    //终端-充电中信息
    void SendTermPageChargingData(unsigned char ucCanID, unsigned char ucType, stThreePhaseTypeConfig &ThreePhaseTypeConfig);
    //终端详情界面----发送终端详细信息----普通版
    void SendTermDetailData_Normal(TerminalStatus &stTerm);
    //终端详情界面----发送终端详细信息----刷卡版
    void SendTermDetailData_Card(TerminalStatus &stTerm, stThreePhaseTypeConfig &ThreePhaseTypeConfig);
    //终端详情界面----发送终端BMS信息
    void SendTermBMSInfo(unsigned char ucCanID);

    //VIN----发送终端VIN信息----普通版独有
    void SendTermVINInfo(TerminalStatus &stTerm);

    //充电报告界面----发送终端充电报告
    void SendTermChargeReport(unsigned char ucCanID);
    //充电明细1
    void SendTermChargeReportDetail1(unsigned char ucCanID);
    //充电明细2
    void SendTermChargeReportDetail2(unsigned char ucCanID);

    //特殊功能设置界面----发送设置信息
    void SendSpecFuncSet(SpecFuncSet_Screen &stSpecFunc);

    //进线侧数据显示界面----发送进线侧信息
    void SendInLineInfo(QByteArray ammeterAddr);
    //进线侧信息界面----发送电表地址
    void SendAmmeterAddr(QList<stAmmeterConfig> &ammeterConfig,unsigned short usNum);

    //子站环境信息界面----发送环境信息
    void SendEnvInfo();
    //子站环境信息界面----发送IO配置信息
    void SendAlarmSet(stIOConfig &IOConfig);

    //直流特殊功能信息界面----发送直流特殊功能设置
    void SendDCSpec(unsigned char ucDCNum);
    //直流特殊功能信息界面----发送直流特殊功能设置一条信息
    void SendDCSpecLine(unsigned char ucCanID);

    //发送 错峰充电状态提示图标----负荷调度界面
    void SendPeakAutoIcon(bool bPeakFlag);
    //发送 负荷调度使能设置----负荷调度界面
    void SendLoadDispatchEnable(bool bPeakFlag, bool bLoadFlag);
    //发送 错峰充电页面数据----负荷调度界面
    void SendPeakSetPage(stAllTPFVConfig &AllTPFVConfig, unsigned char ucPageNum);
    //发送 错峰充电详细设置数据----负荷调度界面
    void SendPeakSetDetail(stAllTPFVConfig &AllTPFVConfig, unsigned char ucType);
    //发送负荷约束设置数据----负荷调度界面
    void SendLoadLimit(stPowerLimitConfig &PowerLimitConfig);

    //发送 终端故障列表界面
    void SendFaultInfoList(TermNameMap NameMap);

    //发送 账户余额信息----刷卡界面
    void SendFrameAccountBalance(unsigned int uiAccountNum);
    //发送 刷卡等待信息----刷卡界面
    void SendFrameCardWait(unsigned char ucIconType);
    //发送 刷卡结果----刷卡界面
    void SendFrameCardResult(QByteArray arrayResult);

    //发送 切换界面指令
    void SendSwitchPage(unsigned short usPageNum);
    //发送 图标显示指令
    void SendShowIcon(unsigned short usIconNum);
    //发送 VIN申请充电结果
    void SendVINResult(unsigned short usIconNum);

    //发送 页面倒计时
    void SendPageCount(unsigned short usCountTime);
    //发送 中止页面倒计时
    void SendPageCountStop(unsigned short usPageNum);    

	void SendSpecGeneral();
	void SendSpecGeneral1();
	void SendSpecEmergency();
	void SendSpecCFCD(stAllTPFVConfig &config, uchar cPage);
	void SendSpecCFCDSet(stAllTPFVConfig &config, uchar cPage);
	void SendSpecCFCDSwitch(stTPFVConfig *config, uchar cPage);
	void SendSpecPolicy(AllLocalPolicyConfig &config, uchar cPage);
	void SendSpecPolicySet(AllLocalPolicyConfig &config, uchar cPage);
	void SendSpecPolicySwitch(LocalPolicyConfig *config, uchar cPage);
	void SendSpecFGPJ(AllFGPJConfig &config, uchar cPage);
	void SendSpecFGPJSet(FGPJConfig *config, uchar cPage);
	void SendSpecDoubleGun();

private:
    //转换  (缓存)终端状态 -> (屏幕显示)终端状态
    unsigned char CheckScreenTermState(unsigned char ucTermState, bool &CartoonFlag,int multitype=0);
    //生成 帧头
    inline void MakeFrameHead(FrameHead_Screen & strHead, unsigned char ucLength);

    //发送数据到串口写入类----输出接口
    void SendFrame(unsigned char *pData, int iLength);
    //发送 网络通信状态
    void SendFrameNetState(unsigned char ucState);
    //发送工作模式状态(本地应急模式, 其他模式)
    void SendFrameCSCUWorkState(unsigned char ucState);
    //发送 各终端名称,ID,状态----主界面
    void SendFrameTermInfo(TermNameMap &NameMap,TermNameMap &NameMapShow);
    //发送 终端名称----主界面
    void SendFrameTermName(unsigned char ucOffset,  char *pName, int iNameLength);
    //发送 终端ID----主界面
    void SendFrameTermID(unsigned char ucOffset,  unsigned char ucTermID);
    //发送 终端状态----主界面
    void SendFrameTermState(unsigned char ucOffset,  unsigned char ucTermState,int multitype=0);
    //发送 场站名称----主界面
    void SendFrameStationName(char * pName);
    //发送 CSCU版本号----主界面
    void SendFrameCSCUProVer(char * pName);
    //发送 故障图标----主界面
    void SendFrameFaultIcon(bool bFaultFlag);

    //发送 终端充电报告----充电报告界面
//    void SendTermChargeReport_Card(unsigned char ucCanID);

    //发送 终端故障列表界面----一条记录
    void SendFaultInfoLine(unsigned char ucAddrOffset, TerminalStatus &Status);

    //发送 本地IP和服务器端口号----系统参数设置界面
    void SendFrameIPAndPort(ParamSetPage_Screen &stParamSetPage);
    //发送 CSCU站地址----系统参数设置界面
    void SendFrameStationAddr(ParamSetPage_Screen &stParamSetPage);
    //发送 域名----系统参数设置界面
    void SendFrameDomainName(ParamSetPage_Screen &stParamSetPage);
    //发送 网关,DNS服务器,ZIGBEE地址----系统参数设置界面
    void SendFrameGateWay(ParamSetPage_Screen &stParamSetPage);

    //将刷卡结果打印规范化
    void GetFormatCardResult(QByteArray &retArray, char *pDest, unsigned char ucLenth);

    //多枪显示  hd
    int GetMultiType(unsigned char ucCanID);
public:
    unsigned int uiPageCount;
private:
    DevCache *pDevCache;
    DBOperate *pDBOperate;
	ParamSet *pParamSet;
    Log * pLog;
	QString _strLogName;
signals:
    //发送串口数据信号->串口写入类
    void sigSendSerialData(unsigned char *pData, int iLength);

};

#endif // SERIALSCREENPROTOCOL_H
