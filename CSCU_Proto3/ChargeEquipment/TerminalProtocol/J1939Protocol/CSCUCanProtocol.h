#ifndef CSCUCANPROTOCOL_H
#define CSCUCANPROTOCOL_H
#include "J1939GeneralProtocol.h"

//刷卡申请结束充电
typedef struct _FrameLSCUQueue
{
    unsigned char ucValid;//0x55有效
    unsigned char ucQueueNo; //排队号
}FrameLSCUQueue;

class cCSCUCanProtocol:public cJ1939GeneralProtocol
{
public:
   cCSCUCanProtocol();
   ~cCSCUCanProtocol();

   //是否有帧需要处理(目前为长帧)
   virtual bool HasFrameToDeal();
   //解析CAN数据
   virtual void ParseCanData(can_frame *pCanFrame);
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

   //解析能效系统模块控制<----控制中心
   virtual void ParseCtrlModule_EP(InfoMap CenterMap, unsigned char ucTermID);
   //解析能效系统功率控制<----控制中心
   virtual void ParseSetPower_EP(InfoMap CenterMap, unsigned char ucTermID);

   //解析CCU参数设置<----控制中心
   virtual void ParseCCUArg(InfoMap CenterMap, unsigned char ucTermID);
   //解析CCU参数查询<----控制中心
   virtual void ParseCCUArgQuery(InfoMap CenterMap, unsigned char ucTermID);

   //解析长帧
      virtual void ParseLongFrame(FrameLongPackage * pLongPackage);
   //解析直流单桩的基本参数设置数据add by zrx
   virtual void ParseCenterDataSP(InfoMap CenterMap,unsigned char ucTermID);

protected:
   //获取参数设置结果类型
   virtual int GetParamAckType(unsigned char ucPF);
   //解析遥调指令<----控制中心
   virtual void ParseTeleadjust(InfoMap CenterMap, unsigned char ucTermID);
   //获取协议版本号枚举
   virtual void GetProVerEnum(unsigned char * pVer);

private:
//   void ParseFrameLSCUStart(unsigned char * pData,unsigned char ps, QList<CanMapNode>  &ToCenterList);
//   void ParseFrameQueue(unsigned char * pData,unsigned char ps, QList<CanMapNode>  &ToCenterList);

    //检查广播召唤帧状态
    void CheckBroadCastState();
    unsigned char GetNetworkStatus();
    //发送工作状态帧
    void SendFrameNetworkStatus(unsigned char ucNetworkStatus);
    //生成--工作状态帧
    void MakeFrameNetworkStatus(can_frame *pCanFrame, unsigned char ucCanID, unsigned char ucNetworkStatus);
    //集控联网状态
    unsigned char ucLatestNetworkStatus;
    //广播召唤设备状态计数器
    unsigned int uiBroadCastCount;
};

#endif//CSCUCANPROTOCOL_H
