#ifndef J1939GENERALPROTOCOL_H
#define J1939GENERALPROTOCOL_H
#include <QObject>
#include <QFile>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QDebug>
#include <QDateTime>

#include "GeneralData/GeneralData.h"
#include "ChargeEquipment/Can/CanSocket.h"
#include "ChargeEquipment/Can/CanBus.h"
#include "ChargeEquipment/TerminalProtocol/TerminalProtocol.h"
#include "CommonFunc/commfunc.h"

//升级单包重发次数
const unsigned char ucMaxUpPackReSend = 3;
//升级分割长包长度
const unsigned int UpdatePerLongPackLongth = 0x800;//2k
//广播召唤定时
const unsigned char ucBroadCastInterval = 100;

//const unsigned short usDCCurrentOffset = 400;//直流电流偏移量400 A

//枪连接状态转换枚举
typedef enum _GunLinkState_Can
{
    Unlinked_Can = 0,//未连接, 断开
    Linked_Can = 1,//连接
    EVUnlinked_Can = 2,//插枪且车辆未连接
    EVEnsured_Can = 3,//插枪且车辆确认
    HalfLinked_Can = 4//半连接

}_GunLinkState_Can;

//短帧数据长度枚举
typedef enum _ShortFrameDataLength
{
    DL_ProVerInfo = 4,
    DL_LifeInfo  = 1,
    DL_ApplyArg = 8,
    DL_ParamAck = 8,
    DL_CardNumber = 8,
    DL_CardApplyStartCharge = 8,
    DL_CardApplyStopCharge = 1,
    DL_AccBAckInfoMoney = 5,
    DL_AccBAckInfoAbnormal = 3,
    DL_BillingPolicyTime = 7,
    DL_BillingPolicyPrice = 5,
    DL_EmergentStopCharge = 2,
    DL_StartCharge = 2,
    DL_StopCharge = 2,
    DL_ChargeParamSet = 4,
    DL_TimeSync = 7,
    DL_PauseCharge = 2,
    DL_LimitCharge = 2,
    DL_RestCharge = 2,
    DL_StartDischarge = 2,
    DL_StopDischarge = 2,
    DL_RecoverCharge = 2,
    DL_UpdateRequest = 1,
    DL_UpdateRequestAck = 1,
    DL_UpadteManage = 8,
    DL_ProgramRecvFinsh = 1,
    DL_ContinueTrans = 1,
    DL_ContinueTransAck = 4,
    DL_PackageRecvAck = 1,
    DL_ProgramFinishSend = 8,
    DL_DeviceState = 1,
    DL_UpdateAck = 4,
    DL_CtrlCmdAck = 8,
    DL_ApplyPGN = 3,
    DL_DataTran = 8,
    DL_LinkManage = 8,
    DL_NetworkStatus = 1,

    DL_PVSwitch = 3   //能效添加光伏开关机
}ShortFrameDataLength;

//长帧数据长度枚举
typedef enum _LongFrameDataLength
{
    DL_SpecificInfo = 47,//47字节
    DL_MaxLongFrameLength = 1785//255*7
}LongFrameDataLength;

//通用PGN种类枚举
typedef enum _GeneralPDUType
{
    PF_ProVerInfo = 0x10,
    PF_LifeInfo  = 0x20,
    PF_ApplyArg = 0x40,
    PF_ParamAck = 0x41,
    PF_CardNumber = 0x43,
    PF_CardApplyStartCharge = 0x44,
    PF_CardApplyStopCharge = 0x45,
    PF_AccBAckInfo = 0x46,
    PF_BillingPolicy = 0x47,
    PF_CSCUNetworkStatus = 0x4C,
    PF_EmergentStopCharge = 0x51,
    PF_StartCharge = 0x52,
    PF_StopCharge = 0x53,
    PF_ChargeParamSet = 0x54,
    PF_TimeSync = 0x55,
    PF_PauseCharge = 0x56,
    PF_LimitCharge = 0x57,
    PF_ResetCharge = 0x58,
    PF_StartDischarge = 0x59,
    PF_RecoverCharge = 0x5A,
    PF_StopDischarge = 0x60,
    PF_UpdateRequest = 0x61,
    PF_UpdateRequestAck = 0x62,
    PF_UpadateManage = 0x63,
    PF_ProgramRecvFinsh = 0x64,
    PF_ContinueTrans = 0x65,
    PF_ContinueTransAck = 0x66,
    PF_PackageRecvAck = 0x67,
    PF_ProgramFinishSend = 0x68,
    PF_DeviceState = 0x70,
    PF_UpdateAck = 0x71,
    PF_SpecificInfo = 0x9F,
    PF_CtrlCmdAck = 0xE8,
    PF_ApplyPGN = 0xEA,
    PF_DataTran = 0xEB,
    PF_LinkManage = 0xEC,

    PF_PVSwitch = 0xCD,   //能效添加控制光伏开关机
    PF_ModulePowerControl = 0xCE
}GeneralPDUType;

//CAN帧优先级枚举
typedef enum _FramePriority
{
    PriorityZero  = 0x00, //优先级最高
    PriorityOne  = 0x01,
    PriorityTwo  = 0x02,
    PriorityThree  = 0x03,
    PriorityFour  = 0x04,
    PriorityFive = 0x05,
    PrioritySix  = 0x06,
    PriorityDefault = 0x06,
    PrioritySeven  = 0x07   //优先级最低
}FramePriority;

//报文头j1939
typedef struct _FrameHead
{
    unsigned char	ucSa;
    unsigned char	ucPs;
    unsigned char	ucPf;
    struct
    {
        unsigned char ucDp			        : 1;
        unsigned char ucReserved		: 1;
        unsigned char ucPriority        	: 3;
        unsigned char ucUnuse			: 3;
    }stBitFlag;
} __attribute__ ((packed)) FrameHead;

//长帧定义
typedef struct _FrameLongPackage
{
    unsigned char ucTermID;//终端ID
    unsigned char ucPGN[3];
    unsigned char * pData;
    unsigned int uiDataLength;
}FrameLongPackage;

//命令回复j1939
typedef enum _CtrlCmdAck
{
    Ack_CmdAck = 0,
    Ack_CmdNAck,
    Ack_CmdCallRefuse,
    Ack_CmdCanNotAck,

    //控制字节定义
    Ack_CmdPDUFault = 13,  //充电机故障无法完成充电05
    Ack_CmdNoPowerAccept,//当前无模块可用，无法完成充电3B
    Ack_CmdScram,//急停告警，无法完成充电 39
    Ack_CmdLinkBreak,//连接器断开，无法完成充电 07
    Ack_CmdLastProtectNoEnd,//前一次充电因主动防护终止未拔枪，无法完成充电 --
    Ack_CmdLaskBMSOutTime,//前一次充电BMS通信超时终止，无法完成充电 36
    Ack_CmdChargerInUsed//充电机正在配置，充电或升级，无法完成充电 40
}CtrlCmdAck;

//命令返回结果
typedef struct _FrameCmdAck
{
    unsigned char ucCtrlAck;
    unsigned char ucGroupFunc;
    unsigned char ucReserved[3];
    unsigned char ucPGNAcked[3];
}__attribute__ ((packed)) FrameCmdAck;

//请求PGNj1939
typedef struct _FrameAppyPGN
{
    unsigned char ucDestPGN[3];
}__attribute__ ((packed)) FrameAppyPGN;

//连接管理j1939
typedef enum _LinkManageCtrl //命令字枚举
{
    Link_ApplySend = 0x10,//请求发送
    Link_AllowSend = 0x11,//允许发送
    Link_MsgEndAck = 0x13,//结束应答
    Link_Abandon = 0xFF//放弃连接
}LinkManageCtrl;

typedef struct _FrameLinkManage
{
    unsigned char ucCtrlManage;
    union
    {
        struct
        {
            unsigned short usDataLength;
            unsigned char ucPackageTotalNum;
            unsigned char ucReserved;
            unsigned char ucPackagePGN[3];
        }stApplySend;//请求发送
        struct
        {
            unsigned char ucPackageAllowedSendNum;
            unsigned char ucPackageNextSendNum;
            unsigned char ucReserved[2];
            unsigned char ucPackagePGN[3];
        }stAllowSend;//允许发送
        struct
        {
            unsigned short usDataLength;
            unsigned char ucPackageTotalNum;
            unsigned char ucReserved;
            unsigned char ucPackagePGN[3];
        }stMsgEndAck;//结束应答
        struct
        {
            unsigned char ucReserved[4];
            unsigned char ucPackagePGN[3];
        }stLinkAbandon;//放弃连接
    }data;

}__attribute__ ((packed)) FrameLinkManage;

//数据传输j1939
typedef struct _FrameDataTran
{
    unsigned char ucPackageId;
    unsigned char ucData[7];
}__attribute__ ((packed)) FrameDataTran;

//CAN协议版本信息
typedef struct _FrameProVerInfo
{
    unsigned char ucProVersion[3];
    unsigned char ucVerAck;
}__attribute__ ((packed)) FrameProVerInfo;

//生命信息帧
typedef struct _FrameLifeInfo
{
    unsigned char ucLifeFlag;
}__attribute__ ((packed)) FrameLifeInfo;

//参数查询
typedef struct _FrameApplyArg
{
    unsigned char ucInnerID;
    unsigned char ucType;
    unsigned char ucReserved[3];
    unsigned char ucPGN[3];
}__attribute__ ((packed)) FrameApplyArg;

//参数配置结果
typedef struct _FrameParamAck
{
    unsigned char ucAck;
    unsigned char ucInnerID;
    unsigned char ucArgNo;
    unsigned char ucReserved[2];
    unsigned char ucPGN[3];
}__attribute__ ((packed)) FrameParamAck;

//刷卡卡号
typedef struct _FrameCardNumber
{
    unsigned char ucCardNumber[8];
}__attribute__ ((packed)) FrameCardNumber;

//刷卡申请开始充电
typedef struct _FrameCardApplyStartCharge
{
    unsigned char ucValid;//0x55有效
    unsigned char ucChargeType;//1充满为止/2按金额充/3按电量充/4按时间充
    unsigned short usChargeAmount;//充电金额
    unsigned short usChargeEnergy;//充电电量
    unsigned short usChargeTime;//充电时间
}__attribute__ ((packed)) FrameCardApplyStartCharge;

//刷卡申请结束充电
typedef struct _FrameCardApplyStopCharge
{
    unsigned char ucValid;//0x55有效
}FrameCardApplyStopCharge;

//刷卡返回异常信息
typedef struct _FrameAbnormalInfo
{
    unsigned char ucAccType;//账户类型
    unsigned short usAbInfo;//异常信息
}__attribute__ ((packed))FrameAbnormalInfo;

//刷卡返回账户信息
typedef struct _FrameAccountInfo
{
    unsigned char ucAccType;//账户类型
    unsigned int uiAccBalance;//账户余额
}__attribute__ ((packed))FrameAccountInfo;

//计费策略时间
typedef struct _FrameBillingPolicyTime
{
    struct
    {
        unsigned char ucCmdType		: 3;
        unsigned char ucPriceTactic		: 5;
    }stByte1;
    unsigned char ucMin;
    unsigned char ucHour;
    unsigned char ucDay;
    unsigned char ucMonth;
    unsigned short usYear;
}__attribute__ ((packed)) FrameBillingPolicyTime;

//计费策略金额
typedef struct _FrameBillingPolicyPrice
{
    struct
    {
        unsigned char ucCmdType		: 3;
        unsigned char ucPriceTactic		: 5;
    }stByte1;
    unsigned short usEnergyPrice;
    unsigned short usServicePrice;
}__attribute__ ((packed)) FrameBillingPolicyPrice;

//充电机时间同步报文
typedef struct _FrameTimeSync
{
    unsigned char ucSec;
    unsigned char ucMin;
    unsigned char ucHour;
    unsigned char ucDay;
    unsigned char ucMonth;
    unsigned short usYear;
}__attribute__ ((packed)) FrameTimeSync;

//充电机充电参数设置命令
typedef struct _FrameChargeParamSet
{
    short sMaxChargeVoltage;
    short sMaxChargeCurrent;
}__attribute__ ((packed)) FrameChargeParamSet;
//
typedef struct _FrameChargePowerSet
{
    unsigned char moduleID;
    short sMaxChargePower;
    int reserve1;
    char reserve2;
}__attribute__ ((packed)) FrameChargePowerSet;

//集控下发模块升级请求
typedef struct _FrameUpdateRequest
{
    unsigned char ucVaildFlag;//0x55有效
}__attribute__ ((packed)) FrameUpdateRequest;

//充电模块升级响应
typedef enum _UpdateRequestAck
{
    NotAllowUpdate = 0x00,
    NotAllowGunState = 0x01,
    NotAllowFlashTooSmall = 0x02,
    AllowUpdate = 0xAA
}__attribute__ ((packed)) UpdateRequestAck;

typedef struct _FrameUpdateRequestAck
{
    unsigned char ucUpdateAck;//0x55命令有效标识
}__attribute__ ((packed)) FrameUpdateRequestAck;

//升级管理报文
typedef struct _FrameUpdateManage
{
    unsigned char ucCtrlManage;
    union
    {
        struct
        {
            unsigned int uiProgramLength;
            unsigned short usPackageNum;
            unsigned char ucReserved;
        }stApplySend;
        struct
        {
            unsigned short usAllowSendPackNum;
            unsigned char ucPackageNextSendNum;
            unsigned char ucReserved[4];
        }stAllowSend;
        struct
        {
            unsigned int uiProgramLength;
            unsigned short usCRCValue;
            unsigned char ucReserved;
        }stMsgEndAck;
        struct
        {
            unsigned char ucLinkAbandon[7];
        }stLinkAbandon;
    }data;
}__attribute__ ((packed)) FrameUpdateManage;

//升级管理报文
typedef struct _FrameEMSControl
{
unsigned int controlCMD;

}__attribute__ ((packed)) FrameEMSControl;
//程序接收完成报文
typedef enum _ProgramRecvFinsh
{
    RecvFail = 0x00,
    CRCFail = 0x03,
    CopyProFail = 0x04,
    TransError = 0x05,
    NotUpdateCmd = 0x06,
    UpdateSuccess = 0xAA
}__attribute__ ((packed)) ProgramRecvFinsh;

typedef struct _FrameProgramRecvFinsh
{
    unsigned char ucFinshRecvAck;//0x55命令有效标识
}__attribute__ ((packed)) FrameProgramRecvFinsh;

//集控下发断点续传报文
typedef struct _FrameContinueTrans
{
    unsigned char ucVaildFlag;//0x55有效
}__attribute__ ((packed)) FrameContinueTrans;

//模块回复断点续传报文
typedef struct _FrameContinueTransAck
{
    unsigned char ucTransAck;//0x55有效
    unsigned char TransPointAddr[3];//续传点地址
}__attribute__ ((packed)) FrameContinueTransAck;

//本包数据接收完成返回
typedef struct _FramePackageRecvAck
{
    unsigned char ucPackageRecvAck;//0x55有效
}__attribute__ ((packed)) FramePackageRecvAck;

//集控程序全部下发结束
typedef struct _FrameProgramFinishSend
{
    unsigned int uiProgramLength;
    unsigned short usCRCValue;
    unsigned char ucReserved[2]; //填充0xFF
}__attribute__ ((packed)) FrameProgramFinishSend;

//停车位和充电枪归位状态
typedef struct _FrameDeviceState
{
    struct
    {
        unsigned char ucParkingSpaceFreeFlag:   4;
        unsigned char ucGunBackFlag:    4;
    }stFlagState;
}__attribute__ ((packed)) FrameDeviceState;

//充电模块版本信息
typedef struct _FrameUpdateAck
{
    unsigned char ucVersion[3];
    unsigned char ucUpdateResult;
}__attribute__ ((packed)) FrameUpdateAck;

//设备规格信息
typedef struct _FrameSpecificInfo
{
    unsigned char ucDevID;
    unsigned char ucEndFlag;
    unsigned char ucSlotNum;
    char chSerialNumber[32];
    unsigned char ucSoftwareVer[3];
    unsigned char ucSoftwareVer1[3];
    unsigned char ucSoftwareVer2[3];
    unsigned char ucHardwareVer[3];
}__attribute__ ((packed)) FrameSpecificInfo;

//充电机紧急停止充电命令
typedef struct _FrameEmergentStopCharge
{
    unsigned short usVaildFlag;//0x55有效
}__attribute__ ((packed)) FrameEmergentStopCharge;

//开光伏命令
typedef struct _FramePVPowerON
{
    unsigned char moduleID;
    unsigned short usVaildFlag;//0x55有效
}__attribute__ ((packed))  FramePVPowerON;

//关光伏命令
typedef struct _FramePVPowerOFF
{
    unsigned char moduleID;
    unsigned short usVaildFlag;//0x55有效
}__attribute__ ((packed)) FramePVPowerOFF;

//充电机启动充电命令
typedef struct _FrameStartCharge
{
    unsigned short usVaildFlag;//0x55有效
}__attribute__ ((packed))  FrameStartCharge;

//充电机终止充电命令
typedef struct _FrameStopCharge
{
    unsigned short usVaildFlag;//0x55有效
}__attribute__ ((packed)) FrameStopCharge;

//充电机暂停充电命令
typedef struct _FramePauseCharge
{
    unsigned short usVaildFlag;//0x55有效
}__attribute__ ((packed)) FramePauseCharge;

//充电机限制充电命令
typedef struct _FrameLimitCharge
{
    unsigned short usVaildFlag;//0x55有效
}__attribute__ ((packed)) FrameLimitCharge;

//充电模块充电复位命令
typedef struct _FrameRestCharge
{
    unsigned short usVaildFlag;//0x55有效
}__attribute__ ((packed)) FrameRestCharge;

//充电模块开始放电命令
typedef struct _FrameStartDischarge
{
    unsigned char ucVaildFlag;//0x55有效
}__attribute__ ((packed)) FrameStartDischarge;

//充电模块停止放电命令
typedef struct _FrameStopDischarge
{
    unsigned char ucVaildFlag;//0x55有效
}__attribute__ ((packed)) FrameStopDischarge;

//长包超时处理
typedef enum _ProtocolTimeInterval
{
    TI_LongPackageOverTime = 100,
    TI_LongPackageUpdataOverTime = 100
}ProtocolTimeInterval;

//处理长包的接收和发送(包括升级)
class cLongPackageModule
{
public:
    cLongPackageModule(unsigned char ucCanIDIn);
    ~cLongPackageModule();
    void Clear();

public:
    unsigned char ucCanID;//当前终端CAN地址
    bool bFreeFlag; //TRUE:空闲,FALSE:有长包在处理
    bool bUsedFlag;//TRUE:使用过的, FALSE:未使用过的
    bool bValidFlag;//TRUE:数据有效, FALSE:数据无效, 用于数据解析
    bool bUpdateFlag;//TRUE: 进行升级处理, FALSE: 进行普通长包处理
    mutable unsigned char ucCounter;//超时处理;若超时未收到消息则放弃连接

    //通用长包指针
    FrameLongPackage * pLongPackage;//长包指针
    //共用变量
    unsigned char * pDataOffset;//长包数据偏移
    unsigned char ucPackNext;//下一个要传输的数据包编号(发送排序)
    unsigned char ucPackLast;//上一个已经传输的数据包的编号(接收校验)
    //正常长包
    unsigned short usDataLength;//长包数据长度    
    unsigned char ucPackTotalNum;//数据总包数
    unsigned char ucPackTransNum;//数据包已经传输数(已经发送/接收, 总数)
    unsigned char ucPackAllowed;//允许发送或接收的数据包数
    //升级长包
    unsigned int uiDataLength_U;//程序总字节数----升级用
    unsigned int uiLongPackLength_U;//长包数据长度(在每次传输长包时赋值)
    unsigned int uiDataTransLength_U;//程序已经传输字节数
    unsigned char * pDataOffset_old;//长包原始位置
    unsigned int uiLongPackNum_U; //分割的长包数量
    unsigned int uiLongPackTransNum_U;//已经传输的长包数量
    unsigned short usPackTotalNum_U;//程序总包数----升级用
    unsigned short usPackTransNum_U;//数据包已经传输数(已经发送/接收, 总数)----升级用
    unsigned short usPackAllowed_U;//允许发送或接收的数据包数----升级用
    unsigned char ucReSendCount;//单包接收失败,重发计数
};

//长包处理模块图
typedef QMap <unsigned char , cLongPackageModule *> ModuleMap;

//CAN协议处理基类
class cJ1939GeneralProtocol : public cTerminalProtocol
{
    Q_OBJECT
public:
    cJ1939GeneralProtocol();
    ~cJ1939GeneralProtocol();
    //解析遥控指令<----控制中心
    virtual void ParseTelecontrol(InfoMap CenterMap, unsigned char ucTermID);
    //解析遥调指令<----控制中心
    virtual void ParseTeleadjust(InfoMap CenterMap, unsigned char ucTermID) = 0;
    //解析刷卡账户列表<----控制中心
    virtual void ParseAccountInfoList(InfoMap CenterMap, unsigned char ucTermID);
    //解析刷卡内部返回结果<----控制中心
    virtual void ParseCardInResult(InfoMap CenterMap, unsigned char ucTermID);
    //解析刷卡远程返回结果<----控制中心
    virtual void ParseCardOutResult(InfoMap CenterMap, unsigned char ucTermID);
    //解析限制充电电流<----控制中心
    virtual void ParseLimitChargeCurrent(InfoMap CenterMap, unsigned char ucTermID);
    virtual void ParseConrolEMSBreaker(InfoMap CenterMap, unsigned char ucTermID);
    //解析限制终端功率<----控制中心
    virtual void ParseLimitPower(InfoMap CenterMap, unsigned char ucTermID);
    //解析模块升级包下载完成<----控制中心
    virtual void ParseMoudleUpdateDir(InfoMap CenterMap, unsigned char ucTermID);
    //给帧头赋值
    bool SetFrameHead(can_frame *pCanFrame, unsigned char ucSa, unsigned char ucPf, unsigned char ucPs, unsigned char ucPriority, unsigned char ucLength);
    //将组好的帧列表发送给CAN总线模块
    virtual void SendFrame();
    //是否有帧需要发送
    virtual bool HasFrameToSend();
    //是否有帧需要处理
    virtual bool HasFrameToDeal();
    //是否有长帧需要处理
    virtual bool HasLongFrameToDeal();
    //某一CAN地址是否有长帧要发送
    virtual bool HasLongFrameToSend();
    //帧处理
    virtual void DealFrame();
    //检查长包模块图, 若对应CAN地址没有在对应的长包处理模块中,则创建, 返回对应模块指针
    cLongPackageModule *CheckModule(unsigned char ucCanID);
    //检查长包是否空闲, 若空闲, 则处理长包并删除
    void CheckModuleFree();
    //检查终端是否有长帧要发送
    bool CheckLongPackSendMap(unsigned char ucCanID);
    //升级准备处理
    void PrepareUpdate(const char *pProPath, cLongPackageModule *pModule, unsigned char ucCanID);

    virtual void ParseChargeTypeReault(InfoMap CenterMap, unsigned char ucTermID);
    //virtual void ParseChargeGunGroupInfo(InfoMap CenterMap, unsigned char ucTermID) {}
protected:
    //规范充电机枪连接状态
    virtual unsigned char CheckLinkState(unsigned char ucLinkStateIn);

    //检查指令返回结果----解析命令返回帧
    void CheckCmdAck(unsigned char &ucCtrlAck, unsigned char &ucFrameACK);
    //检查对应指令类型----解析命令返回帧--CAN
    bool CheckCmdType(unsigned char &ucCmdType, unsigned char &ucPF);
    //将程序加载到内存
    bool GetUpdateProgramData(const char *pProPath, cLongPackageModule * pModule);

    //获取参数设置结果类型
    virtual int GetParamAckType(unsigned char ucPF) = 0;
    //获取协议版本号枚举
    virtual void GetProVerEnum(unsigned char * pVer) = 0;

    //生成长帧(帧内各字段定义在终端类实现)
    void MakeFrameLongPackage(FrameLongPackage * &pLongFrame, unsigned char ucTermID, unsigned char ucPF, unsigned int uiLength);

    //生成单双枪充电信息应答帧
    void MakeFrameAck(can_frame *pCanFrame, unsigned char ucCanID,unsigned char result);

    //生成--版本确认帧
    void MakeFrameProVerAck(can_frame *pCanFrame, unsigned char * pProVer, unsigned char ucCanID);

    //生成--对时帧
    void MakeFrameTimeSync(can_frame *pCanFrame, unsigned char ucCanID);

    //生成--请求PGN帧
    void MakeFrameAppyPGN(can_frame *pCanFrame, unsigned char ucCanID, unsigned char ucPf);
    //生成--参数查询帧
    void MakeFrameAppyArg(can_frame *pCanFrame, unsigned char ucCanID, unsigned char ucInnerID, unsigned char ucType, unsigned char ucPf);

    void MakeFrameStartPV(can_frame *pCanFrame, unsigned char ucCanID, unsigned char moduleID);
    void MakeFrameStopPV(can_frame *pCanFrame, unsigned char ucCanID, unsigned char moduleID);

    //生成--开始充电帧
    void MakeFrameStartCharge(can_frame *pCanFrame, unsigned char ucCanID);
    //生成--结束充电帧
    void MakeFrameStopCharge(can_frame *pCanFrame, unsigned char ucCanID);
    //生成--开始充电帧
    void MakeFrameStartDisCharge(can_frame *pCanFrame, unsigned char ucCanID);
    //生成--结束充电帧
    void MakeFrameStopDisCharge(can_frame *pCanFrame, unsigned char ucCanID);
    //生成--暂停充电帧
    void MakeFramePauseCharge(can_frame *pCanFrame, unsigned char ucCanID);
    //生成--恢复充电帧
    void MakeFrameRecoverCharge(can_frame *pCanFrame, unsigned char ucCanID);
    //生成--调整充电电流电压帧
    void MakeFrameAdjustCurrentVoltage(can_frame *pCanFrame, unsigned char ucCanID, float fCur, float fVol);

    void MakeFrameAdjustPower(can_frame *pCanFrame, unsigned char ucCanID, float fPower);

    //生成--账户信息帧
    void MakeFrameAccountInfo(can_frame *pCanFrame, unsigned char ucCanID, AccountInfo &stInfo);
    //生成--计费策略帧
    void MakeFramePricePolicy(can_frame *pCanFrame, unsigned char ucCanID, PolicyInfo &stInfo, unsigned char ucType, unsigned char ucNum);
    //生成--刷卡结果帧
    void MakeFrameCardResult(can_frame *pCanFrame, unsigned char ucCanID, unsigned char ucResult);

    //生成--数据传输帧
    void MakeFrameDataTran(can_frame *pCanFrame, cLongPackageModule *pModule);
    //生成--连接管理帧
    bool MakeFrameLinkManage(can_frame *pCanFrame, unsigned char ucCtrlManage, cLongPackageModule *pModule);

    //生成--升级请求帧
    void MakeFrameUpdateRequest(can_frame *pCanFrame, unsigned char ucCanID);
    //生成--升级管理帧
    bool MakeFrameUpdateManage(can_frame *pCanFrame, unsigned char ucCtrlManage, cLongPackageModule *pModule);
    //生成--升级完成帧
    void MakeFrameProgramFinishSend(can_frame *pCanFrame, cLongPackageModule *pModule);

    //发送控制命令到CAN总线
    virtual void SendCmd(unsigned int uiChargeCmdType, unsigned char ucCanID);

    //发送单双枪分组信息应答帧
    void SendFrameAck(unsigned char ucCanID,unsigned char result);

    //发送--版本确认帧
    void SendFrameProVerAck(unsigned char ucCanID, unsigned char * pVer);
    //发送--对时帧
    void SendFrameTimeSync( unsigned char ucCanID);
    //发送--请求PGN帧
    void SendFrameAppyPGN(unsigned char ucCanID, unsigned char ucPf);
    //发送--请求参数帧
    void SendFrameAppyArg(unsigned char ucCanID, unsigned char ucInnerID, unsigned char ucType, unsigned char ucPf);

    void SendFrameStartPV(unsigned char ucCanID, unsigned char moduleID);
    void SendFrameStopPV(unsigned char ucCanID, unsigned char moduleID);

    //发送--开始充电帧
    void SendFrameStartCharge(unsigned char ucCanID);
    //发送--结束充电帧
    void SendFrameStopCharge(unsigned char ucCanID);
    //发送--开始充电帧
    void SendFrameStartDisCharge(unsigned char ucCanID);
    //发送--结束充电帧
    void SendFrameStopDisCharge(unsigned char ucCanID);
    //发送--暂停充电帧
    void SendFramePauseCharge(unsigned char ucCanID);
    //发送--恢复暂停充电帧
    void SendFrameRecoverCharge(unsigned char ucCanID);
    //发送--调整充电电压电流帧(参数设置帧)
    void SendFrameAdjustCurrentVoltage(unsigned char ucCanID, float fCur, float fVol);

    void SendFrameConrolEMSBreaker(char);
    void MakeFrameConrolEMSBreaker(can_frame *,char);

    void SendFrameAdjustPower(unsigned char ucCanID, int fPower);

    //生成--账户信息帧
    void SendFrameAccountInfo(unsigned char ucCanID, AccountInfo &stInfo);
    //生成--计费策略帧
    void SendFramePricePolicy(unsigned char ucCanID, PolicyInfo &stInfo, unsigned char ucType, unsigned char ucNum);
    //生成--刷卡结果帧
    void SendFrameCardResult(unsigned char ucCanID, unsigned char ucResult);

    //发送数据传输帧----长包传输
    void SendFrameDataTran(cLongPackageModule *pModule);
    //发送发送请求帧----连接管理
    void SendFrameApplySend(cLongPackageModule *pModule);
    //发送允许发送帧----连接管理
    void SendFrameAllowSend(cLongPackageModule *pModule);
    //发送放弃连接帧----连接管理
    void SendFrameLinkAbandon(cLongPackageModule *pModule);
    //发送传输结束帧----连接管理
    void SendFrameLinkMsgEndAck(cLongPackageModule *pModule);

    //发送--升级请求帧
    void SendFrameUpdateRequest(unsigned char ucCanID);

    //发送升级请求----升级管理
    void SendFrameUpdateApply(cLongPackageModule *pModule);
    //发送放弃连接帧----升级管理
    void SendFrameUpdateAbandon(cLongPackageModule *pModule);
    //发送传输结束帧----升级管理
    void SendFrameUpdateMsgEndAck(cLongPackageModule *pModule);
    //发送程序发送完成帧
    void SendFrameProgramFinishSend(cLongPackageModule *pModule);

    //解析协议版本号
    void ParseFrameProVer(unsigned char * pData, unsigned char ucCanID);
    //解析卡号
    void ParseFrameCardNum(unsigned char * pData, unsigned char ucLen, QList<CanMapNode>  &ToCenterList);
    //解析申请账户信息
    void ParseFrameApplyAccount(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析刷卡申请开始充电
    void ParseFrameCardStart(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析刷卡申请结束充电
    void ParseFrameCardStop(unsigned char * pData, QList<CanMapNode> &ToCenterList);

    //解析遥控命令返回帧
    void ParseFrameCmdAck(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析遥调命令返回帧
    void ParseFrameParamAck(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析连接管理帧
    void ParseFrameLinkMange(unsigned char * pData, cLongPackageModule * pModule);
    //解析数据传输帧
    void ParseFrameDataTran(unsigned char * pData, cLongPackageModule * pModule);
    //解析升级请求回复帧
    void ParseFrameUpdateRequestAck(unsigned char * pData, cLongPackageModule * pModule, QList<CanMapNode>  &ToCenterList);
    //解析本包数据接收完成回复帧
    void ParseFramePackageRecvAck(unsigned char * pData, cLongPackageModule * pModule, QList<CanMapNode>  &ToCenterList);
    //解析升级管理帧
    void ParseFrameUpdateManage(unsigned char * pData, cLongPackageModule * pModule);
    //解析升级完成帧
    void ParseFrameProgramRecvFinsh(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析升级版本信息
    void ParseFrameUpdateAck(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析设备模块信息
    void ParseSpecificInfo(FrameLongPackage * pLongPackage, QList<CanMapNode>  &ToCenterList);
    //解析长帧
    virtual void ParseLongFrame(FrameLongPackage * pLongPackage) = 0;

protected:
    ModuleMap * pModuleMap;    //CAN地址--长包模块 对应关系图
    ModuleMap * pModuleMapToSend;   //准备发送长帧图(主动下发)
    QList <can_frame *> *pTerminalSendList;//终端CAN帧发送列表
    QMutex * pSendListMutex;//发送列表操作锁
    QMutex * pModuleMapMutex;//长包模块操作锁
    QMutex * pModuleSendMapMutex;   //发送长包MAP锁

signals:
    //CAN帧列表发送信号(在SendFrame中发送)
    void sigSendCanData(QList <can_frame *> *pTerminalSendList, QMutex * pSendListMutex);
};
#endif // J1939GENERALPROTOCOL_H
