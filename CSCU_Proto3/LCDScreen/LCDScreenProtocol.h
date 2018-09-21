#ifndef LCDSCREENPROTOCOL_H
#define LCDSCREENPROTOCOL_H

#include <QByteArray>
#include <QVariant>
#include <QDebug>
#include <QFile>
#include <netinet/in.h>

#include "LCDDef.h"
#include "Infotag/CSCUBus.h"
#include "DevCache/DevCache.h"
#include "GeneralData/ModuleIO.h"
#include "Bus/Bus.h"
#include "ParamSet/ParamSet.h"
#include "CommonFunc/commfunc.h"

#define VERSION_FILE_NAME "/tmp/version.info"

const int iFaultRecordPerPage = 8;  //故障记录每页条目数
const int iChargeRecordPerPage = 8; //充电记录每页条目数
const int iOperateRecordPerPage = 8;    //操作记录每页条目数
//const char * pVersionFile = "/tmp/version.info";

typedef struct _LCDMapNode
{
    InfoAddrType enType;
    mutable InfoMap stMap;
}LCDMapNode;

typedef enum _ProType_LCD
{
    NormalType_LCD = 0x01   //普通刷卡类型
}ProType_LCD;

typedef enum _CmdType_LCD
{
    Cmd_Mainten_LCD = 0x01, //维护类
    Cmd_Query_LCD = 0x02,   //查询类
    Cmd_ParaSet_LCD = 0x82,   //设置类
    Cmd_State_LCD = 0x03,   //状态类
    Cmd_RealData_LCD = 0x04,   //实时数据类
    Cmd_HisData_LCD = 0x05,   //历史数据类
    Cmd_InterActive_LCD = 0x06   //交互类指令

}CmdType_LCD;

typedef enum _DataType_LCD
{
    Data_Version_LCD = 0x0102,  //版本信息

    Data_SysParam_LCD = 0x0201, //系统设置类数据
    Data_PhaseType_LCD = 0x0207, //三相相别设置类数据
    Data_SysSpecSet_LCD = 0x0208, //系统特殊功能设置类数据
    Data_DCSpecSet_LCD = 0x0209, //直流特殊功能设置类数据
    Data_LoadLimit_LCD = 0x020A, //负荷约束设置类数据
    Data_PeakSet_LCD = 0x020B,   //错峰充电设置类数据
    Data_AmmeterAddr_LCD = 0x020C,   //电表地址数据
    Data_Passwd_LCD = 0x020D,   //登录密码数据
    Data_IOState_LCD = 0x020E,   //IO常开常闭状态
    Data_2DbarCodes_LCD = 0x020F,   //二维码设置,读取
    Data_DCCabArgTypeNum_LCD = 0x0210,  //直流柜某参数类型设备数量
    Data_TermParam_DC_LCD = 0x0211,    //直流终端参数设置
    Data_CCUParam_LCD= 0x0212,  //CCU参数设置
    Data_DCSysParam_LCD = 0x0213,   //直流柜系统参数设置
    Data_DCCabDeviceNum_LCD = 0x0214,   //直流机设备 数据个数查询
    Data_LocalDynamicPassword_LCD = 0x021A, //随机码上传,管理员密码下发
    Data_LocalChargePassword_LCD = 0x021B, //本地充电密码设置


    Data_TermState_LCD = 0x0301,    //终端状态
    Data_CSCUState_LCD = 0x0302,    //CSCU状态
    Data_TermMeasure_Normal_LCD = 0x0402,    //终端测量数据----普通版
    Data_TermMeasure_Card_LCD = 0x0405,    //终端测量数据----刷卡版
    Data_TermBMS_LCD = 0x0403,    //终端BMS数据
    Data_TermFaultInfo_LCD = 0x0404,    //终端故障信息数据
    Data_AmmeterData_LCD = 0x0406,    //进线侧数据
    Data_EnvInfo_LCD = 0x0407,    //环境信息
    Data_DCMOD_LCD = 0x0409,    //功率模块信息
    Data_DCPDU_LCD = 0x040A,    //pdu信息
    Data_DCCCU_LCD = 0x040B,    //ccu信息
    Data_NoRemoveFaultRecordNum_LCD = 0x040C,                         //未消除故障记录条目 add by muty 20170913
    Data_NoRemoveFaultRecord_LCD = 0x040D,                                  //未消除故障记录 add by muty 20170913

    Data_RecordNum_LCD = 0x0501,   //记录类型及条目
    Data_FaultRecord_LCD = 0x0502,   //故障记录
    Data_ChargeRecord_LCD = 0x0503,   //充电记录
    Data_OperateRecord_LCD = 0x0504,   //操作记录

    Data_TermChargeReport_LCD = 0x0505,   //终端充电报告

    Data_SetResult_LCD = 0x0601,   //参数设置结果

    Data_CardInfoApply_LCD = 0x0602,   //刷卡卡号,用户信息申请(开始充电,结束充电不同)
    Data_CardInfoAck_LCD = 0x0603,   //刷卡卡号,用户信息返回

    Data_CardCmdApply_LCD = 0x0604, //刷卡开始充电,结束充电命令下发
    Data_CardCmdAck_LCD = 0x0605, //刷卡开始充电,结束充电结果返回

    Data_UdiskUpdate_LCD = 0x0606,   //U盘升级数据导出
    Data_UdiskUpdateAck_LCD = 0x0607,   //U盘升级数据导出结果返回

    Data_LocalChargeCmd_LCD = 0x060A,    //本地开始充电/结束充电
    Data_LocalChargeStop_LCD = 0x060b,    //本地结束充电
    Data_TicketDev_LCD = 0x060c    //打印小票

}DataType_LCD;

typedef enum _ArgDevType
{
    Dev_GUN_LCD = 1,
    Dev_CCU_LCD
}ArgDevType;

typedef enum _OperateResult_LCD
{
    Ack_Failed_LCD = 0x00,
    Ack_Success_LCD = 0x01
}OperateResult_LCD;

class cLCDScreenProtocol
{
public:
    cLCDScreenProtocol();
    //解析帧
//    void ParseFrame(QByteArray recvArray, unsigned char * pData, unsigned char&Len);

    //生成  申请卡号---->控制中心
    void MakeCenterApplyCardNum(InfoMap &ToCenterMap, unsigned char ucCanID);
    //生成  申请用户信息---->控制中心
    void MakeCenterApplyUserInfo(InfoMap &ToCenterMap, unsigned char ucCanID, QByteArray CardNum);
    //生成  刷卡开始充电请求---->控制中心
    void MakeCenterApplyStartCharge_Card(InfoMap &ToCenterMap, CardCmdApply_LCD &stCmdApply, QByteArray CardNum);
    //生成  刷卡停止充电请求---->控制中心
    void MakeCenterApplyStopCharge_Card(InfoMap &ToCenterMap, unsigned char ucCanID, QByteArray CardNum);
    //生成CSCU版本信息
    void MakFrameCSCUVersion(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr);
    //生成  CSCU设置项
    void MakFrameCSCUSet(unsigned char *&pDestData, unsigned short &usLen, unsigned char &ucAddr);
    //生成  相别设置项
    bool MakFramePhaseTypeSet(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr);
    //生成 屏幕密码项
    bool MakFramePasswd(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr);
    //生成 IO常开常闭状态
    bool MakFrameIOState(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr);

    //生成  电表地址项
    bool MakFrameAmmeterAddr(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr);
    //生成  进线侧数据项
    bool MakFrameAmmeterData(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, const char * pRawData);
    //生成  环境信息
    bool MakFrameEnvInfo(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, const char * pRawData);
    //生成 模块实时数据
    bool MakFrameModData(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, unsigned char &ucCanID, unsigned char &ucInnerID);
    //生成 PDU实时数据
    bool MakFramePDUData(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, unsigned char &ucCanID, unsigned char &ucInnerID);
    //生成 CCU实时数据
    bool MakFrameCCUData(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, unsigned char &ucCanID, unsigned char &ucInnerID);

    //生成  错峰充电设置
    bool MakFramePeakSet(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr);
    //生成终端设置
    bool MakFrameGunSet(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, unsigned char ucCanID);
    //生成CCU设置
    bool MakFrameCCUSet(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, unsigned char ucCanID);
    //生成直流柜某参数类型设备个数
    bool MakFrameDCCabArgTypeNum(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, unsigned char ucType);

    //生成直流柜设备个数
    bool MakFrameDCCabDevNum(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, unsigned char ucType);

    //生成随机码
    bool MakFrameRandCode(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, unsigned int uiCode);

    //生成二维码查询结果
    void MakFrame2DbarCodes(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr);

    //生成  系统特殊功能设置项
    void MakFrameSysSpecSet(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr);
    //生成直流特殊功能设置项
    void MakFrameDCSpecSet(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr);
    //生成  负荷约束功能设置数据项
    void MakFrameLoadLimit(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr);
    //生成 本地充电密码数据项
    void MakFrameLocalChargePassword(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr);
    //生成  终端状态数据项
    bool MakFrameTermState(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, unsigned char &ucCanID);

    //生成  CSCU状态数据项
    bool MakFrameCSCUState(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr);

    //生成  终端测量数据项----普通版
    bool MakFrameTermMeasure_Normal(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, unsigned char &ucCanID);
    //生成  终端测量数据项----刷卡版
    bool MakFrameTermMeasure_Card(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, unsigned char &ucCanID);
    //生成  终端BMS数据项
    bool MakFrameTermBMS(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, unsigned char &ucCanID);
    //生成  终端故障信息数据项
    bool MakFrameTermFaultInfo(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, unsigned char &ucCanID);
    //生成  终端充电报告数据项
    bool MakFrameTermChargeReport(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, unsigned char &ucCanID);

    //生成  历史记录条目
    bool MakeFrameRecordNum(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, const QByteArray &recvArray);
    //生成  故障记录数据项
    bool MakFrameFaultRecord(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, unsigned char &ucPageNum);
    //生成  充电记录数据项
    bool MakFrameChargeRecord(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, unsigned char &ucPageNum);
    //生成  操作记录数据项
    bool MakFrameOperateRecord(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, unsigned char &ucPageNum);

    //生成 人机交互结果
    void MakeFrameOperateResult(unsigned char * &pSend, unsigned short &usLen, unsigned char &ucAddr, unsigned short usDataType, unsigned char ucResult, unsigned char ucReboot);
    //生成 刷卡结果
    void MakeFrameCardResult(unsigned char * &pSend, unsigned short &usLen, unsigned char &ucAddr, CardResult_LCD &stCardResult, unsigned int &uiDataType);
    //生成 U盘结果
    void MakeFrameUdiskResult(unsigned char * &pSend, unsigned short &usLen, unsigned char &ucAddr, UdiskResult_LCD &stResult);
    //生成 遥控指令结果
    void MakeFrameCtrlCmdResult(unsigned char * &pSend, unsigned short &usLen, unsigned char &ucAddr, LocalCharge_LCD &stLocalCharge);
    /***********************************************************************
     *函数名       ：MakFrameNoRemoveFaultRecordNum
     *参数说明   ：pDestData    出参 需要发送的数据流，usLen     出参，需要发送的数据流长度
     *                      ucAddr   客户端地址，recvArray 接收到的原始数据
     *返回值        :  true:生成成功,false:生成失败
     *功能           ：生成  未消除故障信息条目
     *备注            :  add by muty 20170913
     **********************************************************************/
    bool MakFrameNoRemoveFaultRecordNum(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, const QByteArray &recvArray);
    /***********************************************************************
     *函数名：MakFrameNoRemoveFaultRecord
     *参数说明：pDestData    出参 需要发送的数据流，usLen     出参，需要发送的数据流长度
     *                  ucAddr   客户端地址，ucPageNum 请求的第几页
     *返回值:      true:生成成功,false:生成失败
     *功能：生成  未消除故障记录
     *备注 : add by muty 20170913
     **********************************************************************/
    bool MakFrameNoRemoveFaultRecord(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, unsigned char &ucPageNum);

    //多枪初始化终端名称 add by songqb
    void InitTermNameMapShow();
    void InitTermNameMapMulti();

    //解析 系统参数设置指令
    int ParseSysParam(const char * pData, unsigned short usLen);
    //解析 相别参数设置指令
    int ParsePhaseParam(const char * pData, unsigned short usLen);
    //解析 系统特殊功能设置
    int ParseSysSpecSet(const char * pData, unsigned short usLen);
    //解析 负荷约束功能设置
    int ParseLoadLimitSet(const char * pData, unsigned short usLen);
    //解析 错峰充电功能设置
    int ParsePeakSet(const char * pData, unsigned short usLen, InfoMap &ToCenterMap);
    //解析 屏幕密码设置
    int ParsePasswd(const char * pData, unsigned short usLen);
    //解析 IO常开常闭设置
    int ParseIOState(const char * pData, unsigned short usLen);
    //解析 二维码设置
    int Parse2DbarCodes(const char * pData, unsigned short usLen);
    //解析 终端参数设置
    int ParseTermArgSet(const char *pData, unsigned short usLen, QList<LCDMapNode> &mapList);
    //解析 CCU参数设置
    int ParseCCUArgSet(const char *pData, unsigned short usLen, QList<LCDMapNode> &mapList);
    //解析 本地充电密码设置
    int ParseLocalChargePassword(const char *pData);

    //解析 刷卡申请卡号信息
    void ParseCardInfoApply(const char * pData, unsigned short usLen, CardResult_LCD & stResult);
    //解析 刷卡申请充电,结束充电
    void ParseCardCmdApply(const char * pData, unsigned short usLen, CardCmdApply_LCD & stResult);
    //解析 直流特殊功能设置
//    void ParseDCSpecSet(const char * pData, unsigned short usLen);
    //结束充电按钮结果
    void MakeFrameCtrlStopCmdResult(unsigned char * &pSend, unsigned short &usLen, unsigned char &ucAddr, LocalChargeStop_LCD &stLocalCharge);

    //打印小票结果
    void MakeFrameTickectDevResult(unsigned char * &pSend, unsigned short &usLen, unsigned char &ucAddr, TicketDevResult_LCD &stTicketDev);
private:
    //转换  错峰充电格式
    void ChangePeakFormat(PeakSet_LCD *&pPeakSet, const stTPFVConfig &stPeak);
    //转换  故障代码为字符串
    void ChangeFaultCodeToArray(unsigned char ucFaultCode, QByteArray &FaultArray);
    //转换  故障代码为字符串--英文
    void ChangeFaultCodeToEnglishArray(unsigned char ucFaultCode, QByteArray &FaultArray);
    //转换  设备中止原因为字符串
    void ChangeDevStopReasonToArray(unsigned char ucStopReason, QByteArray &FaultArray);
    //转换  设备中止原因为字符串--英文
    void ChangeDevStopReasonToEnglishArray(unsigned char ucStopReason, QByteArray &FaultArray);
    //转换  设备管理故障代码为字符串
    void ChangeFaultCodeToArray_DeviceManage(unsigned char ucFaultCode, QByteArray &FaultArray);
    //转换  设备管理故障代码为字符串--英文
    void ChangeFaultCodeToEnglishArray_DeviceManage(unsigned char ucFaultCode, QByteArray &FaultArray);

    //获取  配置信息
    void QueryParamInfo();

    //获取CSCU版本信息
    void GetFrameCSCUVersion(unsigned char * &pData, unsigned short &usLen);
    //获取  CSCU设置数据项
    void GetFrameCSCUSet(unsigned char *&pData, unsigned short &usLen);
    //获取  三相相别设置数据项
    bool GetFramePhaseTypeSet(unsigned char * &pData, unsigned short &usLen);
    //获取  屏幕密码数据项
    bool GetFramePasswd(unsigned char * &pData, unsigned short &usLen);
    //获取  IO常开常闭设置数据项
    bool GetFrameIOState(unsigned char * &pData, unsigned short &usLen);

    //获取  电表地址设置数据项
    bool GetFrameAmmeterAddr(unsigned char * &pData, unsigned short &usLen);
    //获取  错峰功能设置数据项
    bool GetFramePeakSet(unsigned char * &pData, unsigned short &usLen);
    //获取 终端参数设置数据项
    bool GetFrameGunArg(unsigned char * &pData, unsigned short &usLen, unsigned char ucCanID);
    //获取 CCU参数设置数据项
    bool GetFrameCCUArg(unsigned char * &pData, unsigned short &usLen, unsigned char ucCanID);
    //获取直流柜某参数类型设备个数
    bool GetDCCabArgTypeNum(unsigned char * &pData, unsigned short &usLen, unsigned char ucType);
    //获取直流机设备个数
    bool GetDCCabDevNum(unsigned char * &pData, unsigned short &usLen, unsigned char ucType);

    //获取二维码设置数据项
    bool GetFrame2DbarCodes(unsigned char * &pData, unsigned short &usLen);
    //获取  系统特殊功能设置数据项
    void GetFrameSysSpecSet(unsigned char * &pData, unsigned short &usLen);
    //获取  直流特殊功能设置数据项
    void GetFrameDCSpecSet(unsigned char * &pData, unsigned short &usLen);

    //获取  负荷约束功能设置数据项
    void GetFrameLoadLimit(unsigned char * &pData, unsigned short &usLen);
    //获取本地充电密码数据项
    void GetFrameLocalChargePassword(unsigned char * &pData, unsigned short &usLen);

    //获取  进线侧数据项
    bool GetFrameAmmeterData(unsigned char * &pData, unsigned short &usLen, QByteArray &AddrArray);
    //获取  环境信息
    bool GetFrameEnvInfo(unsigned char * &pData, unsigned short &usLen, QByteArray &AddrArray);
    //获取  模块实时数据
    bool GetFrameModData(unsigned char * &pData, unsigned short &usLen, unsigned char ucCanID, unsigned char ucInnerID);
    //获取  PDU实时数据
    bool GetFramePDUData(unsigned char * &pData, unsigned short &usLen, unsigned char ucCanID, unsigned char ucInnerID);
    //获取  CCU实时数据
    bool GetFrameCCUData(unsigned char * &pData, unsigned short &usLen, unsigned char ucCanID, unsigned char ucInnerID);

    //获取  历史记录条目
    bool GetFrameRecordNum(unsigned char * &pData, unsigned short &usLen, const QByteArray &AddrArray);

    //获取  终端状态
    bool GetFrameTermState(unsigned char * &pData, unsigned short &usLen, unsigned char ucCanID);
    //获取  终端遥测数据----普通版
    bool GetFrameTermMeasure_Normal(unsigned char * &pData, unsigned short &usLen, unsigned char ucCanID);
    //获取  终端遥测数据----刷卡版
    bool GetFrameTermMeasure_Card(unsigned char * &pData, unsigned short &usLen, unsigned char ucCanID);
    //获取 终端BMS数据
    bool GetFrameTermBMS(unsigned char * &pData, unsigned short &usLen, unsigned char ucCanID);
    //获取  终端故障信息
    bool GetFrameTermFaultInfo(unsigned char * &pData, unsigned short &usLen, unsigned char ucCanID);
    //获取  第X页充电记录
    bool GetFrameChargeRecord(unsigned char * &pData, unsigned short &usLen, unsigned char ucPageNum);
    //获取  第X页故障记录
    bool GetFrameFaultRecord(unsigned char * &pData, unsigned short &usLen, unsigned char ucPageNum);
    //获取  第X页操作记录
    bool GetFrameOperateRecord(unsigned char * &pData, unsigned short &usLen, unsigned char ucPageNum);
    //获取  终端充电报告
    bool GetFrameTermChargeReport(unsigned char * &pData, unsigned short &usLen, unsigned char ucCanID);
    //获取 未消除故障信息条目 add by muty 20170913
    bool GetFrameNoRemoveFaultRecordNum(unsigned char * &pDestData, unsigned short &usLen);
    //获取 未消除故障信息 add by muty 20170913
    bool GetFrameNoRemoveFaultRecord(unsigned char * &pData, unsigned short &usLen, unsigned char ucPageNum);

    //帧格式打包
    inline void MakeFrameFormat(unsigned char *&pSendFrame, unsigned char &ucAddr, unsigned char ucProType, unsigned char ucCmdType, unsigned short usDataType, unsigned short &usLen,
                                unsigned char * pData);

    //查询 充电记录
    void QueryChargeRecord();
    //查询 故障记录
    void QueryFaultRecord();
    //查询 未消除故障记录 add by muty 20170913
    void QueryNoRemoveFaultRecord();
    //查询 操作记录
    void QueryOperateRecord();
    //查询 数据库, 设备规格信息记录
    void queryDBSpecInfoRecord(QList<SpecificInfo_DCcab> &list);

    //更新 操作记录
    void UpdateOperateDB(unsigned char ucOptType,  const char * pname, const char *  pdata);

    //初始化终端名称列表(写数据库)
//    void InitTermNameDB();

    //初始化终端名称图(写TermNameMap)
    void InitTermNameMap();
    //多枪显示
    int GetMultiType(unsigned char ucCanID);

    //检查终端名称数据库内容
//    bool CheckDBTermName();

public:
    unsigned char ucUDiskState; //U盘插入状态: 1: 插入, 2: 拔下
    TermNameMap  NameMap;     //终端名称图  nihai add
private:
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
    QByteArray CardNum;//刷卡卡号

//    TermNameMap  NameMap;     //终端名称图
    TermNameMap  NameMapShow;     //终端名称图多枪显示
    //历史数据查询存储
    QList <Node_ChargeRecord_LCD> ChargeRecordList; //充电记录列表
    QList <Node_FaultRecord_LCD> FaultRecordList;   //故障记录列表
    QList <Node_OperateRecord_LCD> OperateRecordList;   //操作记录列表
    QList <Node_NoRemoveFaultRecord_LCD> NoRemoveFaultRecordList;   //未消除故障记录列表 add by muty 20170913

    QMap<unsigned char, unsigned char> canAddr;
    unsigned char terminalNum;
    //外部输入参数
    DevCache * pDevCache;
    ParamSet * pParamSet;
    DBOperate * pDBOperate;
    Log * pLog;
	QString _strLogName;
};


#endif // LCDSCREENPROTOCOL_H
