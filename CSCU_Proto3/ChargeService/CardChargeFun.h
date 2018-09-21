#ifndef CARDCHARGEFUN_H
#define CARDCHARGEFUN_H
#include "GeneralData/GeneralData.h"
#include "Infotag/CSCUBus.h"
#include "DevCache/DevCache.h"
#include "ParamSet/ParamSet.h"
#include "Log/Log.h"


class CardChargeFun : public QObject
{
    Q_OBJECT
public:
    CardChargeFun();
    ~CardChargeFun();

private:
    //公用模块指针序号
    typedef enum _Common_Module_Index
    {
        INDEX_LOG = 0,	//日志模块序号
        INDEX_SETTING,	//配置模块序号
        INDEX_DATABASE,	//数据库模块序号
        INDEX_CACHE,	//缓存模块序号
        INDEX_FILTER,	//过滤器模块序号
        INDEX_BUS,		//总线模块序号
        INDEX_COUNT
    }Common_Module_Index;
    Log * gpLog;
    DevCache * gpDevCache;
    ParamSet * gpParamSet;
    TerminalStatus stEmptyTerminalStatus;//空的终端状态
    CHARGE_STEP stEmptyChargeStep;//空的终端状态
    __u8 ucNumTerminal[3];//充电桩数量
	QString _strLogName;

signals:
    void sigToChargeService(InfoMap qInfoMap, InfoAddrType InfoType);// 向充电服务发送数据

    //----------------------------------------------刷卡远程充电相关----------------------------------------------//
public:
    void ProcCardChargeSub(InfoMap &qInfoMap, InfoAddrType &InfoType);//处理刷卡远程充电相关主题 OK+

private:
    void InitTerminalStatus(TerminalStatus &st_TerminalStatus);
    void InitChargeStep(CHARGE_STEP &st_ChargeStep);//初始化状态机
    void InitOrderStatus(CHARGE_STEP &st_ChargeStep);//初始化状态机中订单相关变量
    bool CheckCanAddrValid(__u8 ucCanAddr);//判断CAN地址是否在配置范围内
    inline void LogOut(QString str, int Level);//日志记录

    //----------------------------------------------刷卡远程充电相关----------------------------------------------//
    bool ProcApplyAccountInfoSub(InfoMap &qInfoMap, InfoAddrType &InfoType);//处理申请账户信息相关主题// OK+
    bool ProcOutApplyAccountInfoResult(InfoMap &qInfoMap, InfoAddrType &InfoType);//处理外部服务器返回账户信息 OK
    bool ProcInApplyCharge(InfoMap &qInfoMap, InfoAddrType &InfoType);//处理内部申请开始或者结束充电 OK
    bool ProcInApplyStartChargeByScreen(InfoMap &qInfoMap);//处理内部显示屏申请开始充电 OK
    bool ProcInApplyStartChargeByChargeEquipment(InfoMap &qInfoMap);//处理内部充电设备申请开始充电 OK
    bool ProcInApplyStopChargeByScreen(InfoMap &qInfoMap);//处理内部显示屏申请结束充电OK
    bool ProcInApplyStopChargeByChargeEquipment(InfoMap &qInfoMap);//处理内部充电设备申请结束充电 OK

    int CheckInApplyStartChargeValidByCard(CHARGE_STEP &stChargeStep);//校验内部申请开始充电 OK
    int CheckInApplyStopChargeValidByCard(CHARGE_STEP &stChargeStep);//校验内部申请结束充电 OK
    int CheckInApplyStopChargeValidByCardFromChargeEquipment(CHARGE_STEP &stChargeStep);//校验内部申请结束充电 OK
    void PackageApplyAccountInfoToServer(InfoMap &qInfoMap, InfoAddrType &InfoType, CHARGE_STEP &stChargeStep);// OK
    bool SendApplyAccountInfoToServer(InfoMap &qInfoMap, CHARGE_STEP &stChargeStep);//  OK+
    bool SendInApplyStartChargeResultByCardToScreen(CHARGE_STEP &stChargeStep, int InApplyStartChargeResult);//发送内部申请开始充电结果 OK
    bool SendInApplyStopChargeResultByCardToScreen(CHARGE_STEP &stChargeStep, int InApplyStopChargeResult);//发送内部申请结束充电结果 OK
    bool SendInApplyStartChargeResultByCardToChargeEquipment(CHARGE_STEP &stChargeStep, int InApplyStartChargeResult);//发送内部申请开始充电结果 OK
    bool SendInApplyStoptChargeResultByCardToChargeEquipment(CHARGE_STEP &stChargeStep, int InApplyStartChargeResult);//发送内部申请结束充电结果 OK
    bool SendOuApplyStartChargeByCard(InfoMap &qInfoMap);//向服务器发送申请开始充电 OK
    bool SendOuApplyStopChargeByCard(InfoMap &qInfoMap);//向服务器发送申请结束充电 OK

    bool ProcOutApplyChargeResult(InfoMap &qInfoMap, InfoAddrType &InfoType);//处理服务器返回申请开始或者结束充电结果 OK
    int ParseCardChargeSub(InfoMap &qInfoMap, CHARGE_STEP &stTempChargeStep);//解析刷卡相关主题(充电服务当前只关注CAN地址和卡号) OK
    bool UpdateChargeStepBySendOutCardApplyStartCharge(__u8 ucCanAddr, char CardChargeType);//OK
    bool UpdateChargeStepBySendOutCardApplyStopCharge(__u8 ucCanAddr, char CardChargeType);//OK
    bool UpdateChargeStepByRecvOutCardApplyStartChargeResult(CHARGE_STEP &stChargeStep, int ApplyChargeResult);//OK
    bool UpdateChargeStepByRecvOutCardApplyStopChargeResult(CHARGE_STEP &stChargeStep, int ApplyChargeResult);//OK


    //----------------------------------------------2ChargeEquipment private----------------------------------------------//
    void PackageTelecontrol2ChargeEquipment(CHARGE_STEP &st_ChargeStep,InfoMap &qInfoMap, InfoAddrType &InfoType);//打包给充电设备的充电指令
    bool SendChargeCMDToChargeEquipment(CHARGE_STEP &stChargeStep);//发送预制指令给充电设备
    void SendApplyStopChargeResultToChargeEquipment(CHARGE_STEP &stChargeStep, unsigned char ucResult);//将外部申请结束充电结果发送给充电设备
    void SendApplyStopChargeResultToScreen(CHARGE_STEP &stChargeStep, unsigned char ucResult);//将外部申请结束充电结果发送给显示屏

    bool UpdateChargeStepBySendStopChargeCmd(CHARGE_STEP &st_ChargeStep);//由充电控制指令触发更新充电步骤状态机
};


#endif // CARDCHARGEFUN_H
