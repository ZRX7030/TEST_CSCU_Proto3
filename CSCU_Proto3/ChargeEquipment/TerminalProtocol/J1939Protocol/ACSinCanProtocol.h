#ifndef ACSINCANPROTOCOL_H
#define ACSINCANPROTOCOL_H
#include "J1939GeneralProtocol.h"

typedef enum _ProVerEnum_AC_Sin
{
    ProVer_Old_AC_Sin = 0, //V1.1.5及之前版本
    ProVer_1_1_6_AC_Sin = 1 //1.1.6版本

}ProVerEnum_AC_Sin;

//交流单相PGN枚举
typedef enum _ACSinPDUType
{
    PF_ChargerState1_ACSin = 0x32,
    PF_ChargerState2_ACSin = 0x33,
    PF_ChargerState3_ACSin = 0x34,
    PF_ChargerState5_ACSin = 0x35,
    PF_StopReason_ACSin = 0x36,
    PF_DetailParamSet_ACSin = 0X72
}ACSinPDUType;

//交流单相帧长度枚举
typedef enum _ACSinFrameLength
{
     DL_ChargerState1_ACSin = 8,
     DL_ChargerState2_ACSin = 8,
     DL_ChargerState3_ACSin = 8,
     DL_ChargerState5_ACSin = 8,
     DL_DetailParamSet_ACSin = 8
}ACSinFrameLength;

//充电机状态1
typedef struct _FrameChargerState1_ACSin
{
    short sChargeVoltage;//充电电压
    short sChargeCurrent;//充电电流
    unsigned short usActivePower;//有功功率
    unsigned short usWorkState;//工作状态
}__attribute__ ((packed)) FrameChargerState1_ACSin;

//充电机状态2
typedef struct _FrameChargerState2_ACSin
{
    unsigned short usFaultInfo;	//故障状态	1欠压故障/2过压故障/3过流故障
    unsigned int uiForwordActiveEnergy;//正向有功总电能
    struct {					//各种标识
        unsigned char ucInterFaceType: 1;	//充电接口标识
                unsigned char ucLinkState	: 1;	//连接确认开关状态 0关 1开
                unsigned char ucRelyState	: 1;	//输出继电器状态   0关  1开
        unsigned char ucReserved	: 5;
    }stByte7;
}__attribute__ ((packed)) FrameChargerState2_ACSin;

//充电机状态3
typedef struct _FrameChargerState3_ACSin
{
    unsigned short usChargeTime;//充电时间
     short sReactivePower;//无功功率
    unsigned short usPowerFactor;//功率因数
    unsigned short usZeroLineCurrent;//零线电流
}__attribute__ ((packed)) FrameChargerState3_ACSin;

//充电机状态5
typedef struct _FrameChargerState5_ACSin
{
    unsigned int uiReverseActiveEnergy;//反向有功总电能
    unsigned int uiReverseReactiveEnergy;//反向无功总电能
}__attribute__ ((packed)) FrameChargerState5_ACSin;

//充电机状态5
typedef struct _FrameStopReason_ACSin
{
    unsigned char ucCode;//中止原因代码
    unsigned char ucReserved[7];//预留
}__attribute__ ((packed)) FrameStopReason_ACSin;

//设置指令控制字枚举
typedef enum ACSinParamSetType_ACSin
{
    Set_ACSinParamTypeOne = 01,
    Set_ACSinParamTypeTwo = 02
}ACSinParamSetType_ACSin;

//详细参数设置指令
typedef struct _FrameDetailParamSet_ACSin
{
    unsigned char ucParamSetType;
    union
    {
        struct
        {
            unsigned short uiPWMDutyCycle;//PWM占空比
            unsigned short usOverTempThreshold;//过温阈值
            unsigned char ucReserved[3];//预留
        }stType1;
        struct
        {
            unsigned short usOverCurrentThreshold;//过流阈值
            unsigned short usOverVoltageThreshold;//过压阈值
            unsigned short usUnderVoltageThreshold;//欠压阈值
            unsigned char ucReserved;//预留
        }stType2;
    }Param;
}__attribute__ ((packed)) FrameDetailParamSet_ACSin;

class cACSinCanProtocol : public cJ1939GeneralProtocol
{
public:
    cACSinCanProtocol();
    ~cACSinCanProtocol();
    //是否有帧需要处理(目前为长帧)
    virtual bool HasFrameToDeal();
    //解析CAN数据
    virtual void ParseCanData(can_frame *pCanFrame);

protected:
    //解析遥调指令<----控制中心
    virtual void ParseTeleadjust(InfoMap CenterMap, unsigned char ucTermID);
    //获取参数设置结果类型
    virtual int GetParamAckType(unsigned char ucPF);
    //获取协议版本号枚举
    virtual void GetProVerEnum(unsigned char * pVer);
private:
    //检查广播召唤帧状态
    void CheckBroadCastState();
    //解析充电机状态1
    void ParseFrameState1(unsigned char * pData, QList<CanMapNode>  &ToCenterList);
    //解析充电机状态2
    void ParseFrameState2(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析充电机状态3
    void ParseFrameState3(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析充电机状态5
    void ParseFrameState5(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析充电中止原因
    void ParseStopReason(unsigned char * pData, QList<CanMapNode> &ToCenterList);

    //解析主动防护设置<----控制中心
    virtual void ParseActiveProtect(InfoMap CenterMap, unsigned char ucTermID);
    //解析柔性充电设置<----控制中心
    virtual void ParseFlexibleCharge(InfoMap CenterMap, unsigned char ucTermID);
    //解析通用静态参数设置<----控制中心
    virtual void ParseGeneralStaticArg(InfoMap CenterMap, unsigned char ucTermID);
    //解析通用动态参数设置<----控制中心
    virtual void ParseGeneralDynamicArg(InfoMap CenterMap, unsigned char ucTermID);

    //解析查询主动防护设置<----控制中心
    virtual void ParseQueryActiveProtect(InfoMap CenterMap, unsigned char ucTermID);
    //解析查询柔性充电设置<----控制中心
    virtual void ParseQueryFlexibleCharge(InfoMap CenterMap, unsigned char ucTermID);
    //解析查询通用静态参数设置<----控制中心
    virtual void ParseQueryGeneralStaticArg(InfoMap CenterMap, unsigned char ucTermID);
    //解析查询通用动态参数设置<----控制中心
    virtual void ParseQueryGeneralDynamicArg(InfoMap CenterMap, unsigned char ucTermID);

    //解析CCU参数设置<----控制中心
    virtual void ParseCCUArg(InfoMap CenterMap, unsigned char ucTermID);
    //解析CCU参数查询<----控制中心
    virtual void ParseCCUArgQuery(InfoMap CenterMap, unsigned char ucTermID);

    //解析能效系统模块控制<----控制中心
    virtual void ParseCtrlModule_EP(InfoMap CenterMap, unsigned char ucTermID);
    //解析能效系统功率控制<----控制中心
    virtual void ParseSetPower_EP(InfoMap CenterMap, unsigned char ucTermID);

    //解析长帧
    virtual void ParseLongFrame(FrameLongPackage * pLongPackage);

private:
    //广播召唤设备状态计数器
    unsigned int uiBroadCastCount;
};

#endif // ACSINCANPROTOCOL_H
