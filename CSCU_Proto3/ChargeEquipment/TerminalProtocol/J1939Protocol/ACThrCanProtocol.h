#ifndef ACTHRCANPROTOCOL_H
#define ACTHRCANPROTOCOL_H
#include "J1939GeneralProtocol.h"

typedef enum _ProVerEnum_AC_Thr
{
    ProVer_Old_AC_Thr = 0, //V1.1.5及之前版本
    ProVer_1_0_12_AC_Thr = 1 //1.0.12版本

}ProVerEnum_AC_Thr;

//交流三相PGN指令枚举
typedef enum _ACThrPDUType
{
    PF_ChargerState1_ACThr = 0x32,
    PF_ChargerState2_ACThr = 0x33,
    PF_ChargerState3_ACThr = 0x34,
    PF_ChargerState4_ACThr = 0x35,
    PF_ChargerState5_ACThr = 0x36,
    PF_StopReason_ACThr = 0x37,  //中止原因  add by zrx 2018-03-20
    PF_ChargeManner_ACThr = 0x49, //充电方式：单枪充电／双枪充电  hd2017-5-22
    PF_DetailParamSet_ACThr = 0X72
}ACThrPDUType;

//交流三相帧长度枚举
typedef enum _ACThrFrameLength
{
    DL_ChargerState1_ACThr = 8,
    DL_ChargerState2_ACThr = 8,
    DL_ChargerState3_ACThr = 8,
    DL_ChargerState4_ACThr= 8,
    DL_ChargerState5_ACThr = 7,
    DL_DetailParamSet_ACThr = 8
}ACThrFrameLength;

//充电机状态1
typedef struct _FrameChargerState1_ACThr
{
    short sAPhaseChargeVoltage;//A相充电电压
    short sBPhaseChargeVoltage;//B相充电电压
    short sCPhaseChargeVoltage;//C相充电电压
    short sAPhaseChargeCurrent;//A相充电电流
}__attribute__ ((packed))FrameChargerState1_ACThr;

//充电机状态2
typedef struct _FrameChargerState2_ACThr
{
    short sBPhaseChargeCurrent;//B相充电电流
    short sCPhaseChargeCurrent;//B相充电电流
    unsigned short usZeroLineCurrent;//零线充电电流
    unsigned short usTotalActivePower;//总有功功率
}__attribute__ ((packed))FrameChargerState2_ACThr;

//充电机状态3
typedef struct _FrameChargerState3_ACThr
{
    unsigned short usTotalReactivePower;//总无功功率
    unsigned short usTotalPowerFactor;//总功率因数
    short sVoltageUnbalance;//电压不平衡率(预留)
    short sCurrentUnbalance;//电流不平衡率(预留)
}__attribute__ ((packed))FrameChargerState3_ACThr;


//充电机状态4
typedef struct _FrameChargerState4_ACThr
{
    unsigned int uiTotalActiveEnergy;//总有功电能
    unsigned int uiTotalReactiveEnergy;//总无功电能
}__attribute__ ((packed))FrameChargerState4_ACThr;

//充电机状态5
typedef struct _FrameChargerState5_ACThr
{
    unsigned short usFaultInfo;	//故障状态	1欠压故障/2过压故障/3过流故障
    unsigned short usWorkState;//工作状态
    unsigned short usChargeTime;//充电时间
    struct {					//各种标识
        unsigned char ucInterFaceType: 1;	//充电接口标识
        unsigned char ucLinkState	: 2;	//连接确认开关状态 0未插抢 1插抢且车辆未确认 2插抢且车辆确认可以充电
        unsigned char ucRelyState	: 1;	//输出继电器状态   0关  1开
        unsigned char ucReserved	: 4;
                }stByte7;
}__attribute__ ((packed))FrameChargerState5_ACThr;

//充电中止原因
typedef struct _FrameStopReason_ACThr
{
    unsigned char ucCode;//中止原因代码
    unsigned char ucReserved[7];//预留
}__attribute__ ((packed)) FrameStopReason_ACThr;

//设置指令控制字枚举
typedef enum ACThrParamSetType_ACThr
{
    Set_ACThrParamTypeOne = 01,
    Set_ACThrParamTypeTwo = 02
}ACThrParamSetType_ACThr;

//设置详细参数设置命令
typedef struct _FrameDetailParamSet_ACThr
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
}__attribute__ ((packed))FrameDetailParamSet_ACThr;
//上传单双枪充电分组信息  屏蔽哈尔滨双枪逻辑解析add by zrx 2018-02-10
//typedef struct _FrameChargerMannerInfo_ACThr
//{
//    unsigned char chargeManner;//充电方式
//    unsigned char canID_master;//主枪can地址
//    unsigned char canID_slave;//副枪can地址
//    unsigned char ucReserved[5];     //预留字节
//}__attribute__ ((packed)) FrameChargerMannerInfo_ACThr;

//三项交流上传单双枪充电分组信息
typedef struct _FrameChargerMannerInfo_ACThr
{
    unsigned char chargeManner;//充电方式
    unsigned char canID_master;//主枪can地址
    unsigned char canID_slave1;//副枪1can地址
    unsigned char canID_slave2;//副枪2can地址
    unsigned char canID_slave3;//副枪3can地址
    unsigned char canID_slave4;//副枪4can地址
    unsigned char canID_slave5;//副枪5can地址
    unsigned char canID_slave6;//副枪6can地址
}__attribute__ ((packed)) FrameChargerMannerInfo_ACThr;


class cACThrCanProtocol : public cJ1939GeneralProtocol
{
public:
    cACThrCanProtocol();
    ~cACThrCanProtocol();
    //是否有帧需要处理(目前为长帧)
    virtual bool HasFrameToDeal();
    //解析CAN数据
    virtual void ParseCanData(can_frame *pCanFrame);
protected:
    //修改充电机工作状态
    virtual unsigned char CheckLinkState(unsigned char ucLinkStateIn);
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
    void ParseFrameState1(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析充电机状态2
    void ParseFrameState2(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析充电机状态3
    void ParseFrameState3(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析充电机状态4
    void ParseFrameState4(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析充电机状态5
    void ParseFrameState5(unsigned char * pData, QList<CanMapNode> &ToCenterList);
    //解析充电中止原因
    void ParseStopReason(unsigned char * pData, QList<CanMapNode> &ToCenterList);


// hd couplegun
    //解析充电方式信息
//    bool CheckChargeMannerInfo(FrameChargerMannerInfo_ACThr strFrame, unsigned char canID);  //屏蔽哈尔滨双枪逻辑解析add by zrx 2018-02-10
//    CtrlCmdAck ParseFrameChargeManner(can_frame *, QList<CanMapNode> &ToCenterList);
    void ParseFrameChargeManner(can_frame *srcFrame, QList<CanMapNode>  &ToCenterList);

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


#endif // ACTHRCANPROTOCO_H
