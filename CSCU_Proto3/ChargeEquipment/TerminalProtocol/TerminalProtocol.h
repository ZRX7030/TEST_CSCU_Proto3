#ifndef TERMINALPROTOCOL_H
#define TERMINALPROTOCOL_H

#include <QObject>
#include <QMap>
#include "Infotag/CSCUBus.h"
#include "GeneralData/GeneralData.h"
#include "ChargeEquipment/Can/CanBus.h"

typedef struct _CanMapNode
{
    mutable unsigned int enType;
    mutable InfoMap stCanMap;
}CanMapNode;

class cTerminalProtocol : public QObject
{
    Q_OBJECT
public:
    cTerminalProtocol();
    //是否有帧需要发送----到终端
    virtual bool HasFrameToSend();
    //是否有帧需要处理
    virtual bool HasFrameToDeal();
    //帧处理
    virtual void DealFrame() = 0;
    //接收到控制中心数据并解析
    virtual void ParseCenterData(InfoMap CenterMap, InfoAddrType enAddrType, unsigned char ucTermID);
    //发送数据到对应外部总线
    virtual void SendFrame() = 0;

    unsigned char ucACsingle;
    unsigned char ucACthree;
protected:
    //向指定的CanMap插入一条数据
    void InsertCanInfoMapLine(char * pData,  int iDataLength, unsigned int uiInfoAddr, InfoMap &ToCenterMap);

    //解析遥控指令<----控制中心
    virtual void ParseTelecontrol(InfoMap CenterMap, unsigned char ucTermID) = 0;
    //解析遥调指令<----控制中心
    virtual void ParseTeleadjust(InfoMap CenterMap, unsigned char ucTermID) = 0;
    //解析刷卡账户列表<----控制中心
    virtual void ParseAccountInfoList(InfoMap CenterMap, unsigned char ucTermID) = 0;
    //解析刷卡内部结果<----控制中心
    virtual void ParseCardInResult(InfoMap CenterMap, unsigned char ucTermID) = 0;
    //解析刷卡外部结果<----控制中心
    virtual void ParseCardOutResult(InfoMap CenterMap, unsigned char ucTermID) = 0;
    //解析限制充电电流<----控制中心
    virtual void ParseLimitChargeCurrent(InfoMap CenterMap, unsigned char ucTermID) = 0;
    virtual void ParseConrolEMSBreaker(InfoMap CenterMap, unsigned char ucTermID)=0;
    //解析限制终端功率<----控制中心
    virtual void ParseLimitPower(InfoMap CenterMap, unsigned char ucTermID) = 0;
    //解析模块升级包下载完成<----控制中心
    virtual void ParseMoudleUpdateDir(InfoMap CenterMap, unsigned char ucTermID) = 0;

    //解析主动防护设置<----控制中心
    virtual void ParseActiveProtect(InfoMap CenterMap, unsigned char ucTermID) = 0;
    //解析柔性充电设置<----控制中心
    virtual void ParseFlexibleCharge(InfoMap CenterMap, unsigned char ucTermID) = 0;
    //解析通用静态参数设置<----控制中心
    virtual void ParseGeneralStaticArg(InfoMap CenterMap, unsigned char ucTermID) = 0;
    //解析通用动态参数设置<----控制中心
    virtual void ParseGeneralDynamicArg(InfoMap CenterMap, unsigned char ucTermID) = 0;

    //解析查询主动防护设置<----控制中心
    virtual void ParseQueryActiveProtect(InfoMap CenterMap, unsigned char ucTermID) = 0;
    //解析查询柔性充电设置<----控制中心
    virtual void ParseQueryFlexibleCharge(InfoMap CenterMap, unsigned char ucTermID) = 0;
    //解析查询通用静态参数设置<----控制中心
    virtual void ParseQueryGeneralStaticArg(InfoMap CenterMap, unsigned char ucTermID) = 0;
    //解析查询通用动态参数设置<----控制中心
    virtual void ParseQueryGeneralDynamicArg(InfoMap CenterMap, unsigned char ucTermID) = 0;

    //解析CCU参数设置<----控制中心
    virtual void ParseCCUArg(InfoMap CenterMap, unsigned char ucTermID) = 0;
    //解析CCU参数查询<----控制中心
    virtual void ParseCCUArgQuery(InfoMap CenterMap, unsigned char ucTermID) = 0;

    //解析能效系统模块控制<----控制中心
    virtual void ParseCtrlModule_EP(InfoMap CenterMap, unsigned char ucTermID) = 0;
    //解析能效系统功率控制<----控制中心
    virtual void ParseSetPower_EP(InfoMap CenterMap, unsigned char ucTermID) = 0;

    //接收CAN终端接收的数据并解析<----CAN总线
    virtual void ParseCanData(can_frame *pCanFrame) = 0;

    //发送遥控指令---->终端
    virtual void SendCmd(unsigned int uiChargeCmdType, unsigned char ucTermID) = 0;

    //发送帧到对应的通信端口, 各子类向各自关联的通信接口发送信号
    virtual void SendEquipData();
    //发送帧到对应控制中心
    virtual void SendCenterData(unsigned int &uiInfoAddr, InfoMap &TermSendMap);

    virtual void ParseChargeTypeReault(InfoMap CenterMap, unsigned char ucTermID) =0;
    virtual void ParseChargeGunGroupInfo(InfoMap CenterMap, unsigned char ucTermID);
    virtual void ParseDoubleSys300kwSetting(InfoMap CenterMap, unsigned char ucTermID);

protected :
        unsigned char ucProVer;//协议版本号
        bool bCheckProVerFlag;//校验协议标志位, 0: 校验未完成; 1: 校验完成

signals:
    //将解析后数据传递给终端设备类(协议解析->终端设备)
    void sigSendToCenter(unsigned int uiInfoAddr, InfoMap TerminalDataMap);

public slots:
    //接收终端要发送的数据(终端->协议解析)
//    virtual void ProcRecvTermData(QList<InfoTag> *pTermSendList);

    //接收CAN终端接收的数据并解析(CAN->协议解析)
//    virtual void ProcRecvCanData(QList <can_frame *> *pCanRecvList);
};

#endif // TERMINALPROTOCOL_H
