#ifndef PARKINGLOCKPROTOCOL_H
#define PARKINGLOCKPROTOCOL_H

#include <QObject>
#include <QTimer>
#include <QThread>
#include <QDateTime>
#include <QMap>
#include <QEventLoop>
#include <QString>

//#include "commfunc.h"
#include "Bus/Bus.h"
#include "GeneralData/ModuleIO.h"
#include "RealDataFilter/RealDataFilter.h"
#include "Log.h"
#include "Infotag/CSCUBus.h"
#include "GeneralData/GeneralData.h"
#include "ParkingLock/Can1/Can1Bus.h"
#include "ParkingLock/Can1/Can1Socket.h"

#define ID_CarLockCanID              0xFD    //车位锁主地址
//车位锁使用PGN枚举
typedef enum _ParkingLockPDUType
{
    PF_HeartBeat  = 0x20,
    PF_CtrlCmd = 0x21,
    PF_ParaSet = 0x22,
    PF_ParkingLockStates = 0x23,
    PF_CtrlCmdResponse = 0xE8
}ParkingLockPDUType;

//帧数据长度枚举
typedef enum _FrameDataLength
{
    DL_HeartBeat  = 2,
    DL_CtrlCmd = 2,
    DL_ParaSet = 5,
    DL_ParkingLockStates = 8,
    DL_CtrlCmdResponse  = 2
}FrameDataLength;

//CAN帧优先级枚举
typedef enum _ParkingLockFramePriority
{
    CAN1PriorityZero  = 0x00, //优先级最高
    CAN1PriorityOne  = 0x01,
    CAN1PriorityTwo  = 0x02,
    CAN1PriorityThree  = 0x03,
    CAN1PriorityFour  = 0x04,
    CAN1PriorityFive = 0x05,
    CAN1PrioritySix  = 0x06,
    CAN1PriorityDefault = 0x06,
    CAN1PrioritySeven  = 0x07   //优先级最低
}ParkingLockFramePriority;

//报文头j1939
typedef struct _ParkingLockFrameHead
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
} __attribute__ ((packed)) ParkingLockFrameHead;

//心跳帧
typedef struct _FrameHeartBeat
{
    unsigned char ucCarLockAddr;
    unsigned char ucLifeFlag;
}__attribute__ ((packed)) FrameHeartBeat;


//命令码枚举
typedef enum  _CmdType_CarLock
{
    Type_ResetControl = 0x01,       //复位控制
    Type_LiftLockControl= 0x02,      //升锁控制，并解除休眠
    Type_DropLockControl= 0x03,     //降锁控制
    Type_OpenBuzzer= 0x04,     //开峰鸣器
    Type_CloseBuzzer= 0x05,     //关峰鸣器
    Type_DropLockAndSleep= 0x06     //降锁控制，进入休眠
}CmdType_CarLock;

//控制命令帧
typedef struct _FrameCmd_CarLock
{
    unsigned char ucCarLockAddr;
    unsigned char ucCmdType;
}__attribute__ ((packed)) FrameCmd_CarLock;

//指令回复枚举
typedef enum  _CmdAck_CarLock
{
    ExecutionFailure= 0x00,       //指令不识别
    ExecutionSuccess= 0xAA      //指令正常执行
}CmdAck_CarLock;

//指令回复帧
typedef struct _FrameCmdAck_CarLock
{
    unsigned char ucCarLockAddr;
    unsigned char ucCmdAck;
}__attribute__ ((packed)) FrameCmdAck_CarLock;

//参数设置地址枚举
typedef enum  _ParaSetAddr_CarLock
{
    Addr_SetCarLockAddr = 0x0101,       //设置车位锁地址的地址
    Addr_SetCarLockInitValue = 0x0106      //设置车位锁初始值和检测时长的地址
}ParaSetAddr_CarLock;

//设置参数帧
typedef struct _FrameParaSet_CarLock
{
    unsigned char ucCarLockAddr;
    unsigned short usSetAddr;
    unsigned char ucSetValue[2];
}__attribute__ ((packed)) FrameParaSet_CarLock;

//车位锁状态信息帧
typedef struct _FrameStatesInfo_CarLock
{
    unsigned char ucCarLockAddr;
    struct
    {
        unsigned char ucWorkStates               :2;
        unsigned char ucRockerArmStates     :2;
        unsigned char ucBuzzerStates             :1;
        unsigned char ucReserve                     :1;
        unsigned char ucAutoUpEnable                     :1;
        unsigned char ucMagneticFluctuations     :1;
    }stCarLockStates;
    struct
    {
        unsigned char ucParkingStates          :4;
        unsigned char ucReserve                    :4;
    }stParkingStates;
    unsigned char ucReserve;
    struct
    {
        unsigned char ucAlarmGeomagneticSensor         :1;
        unsigned char ucAlarmUltrasonicProbe1              :1;
        unsigned char ucAlarmUltrasonicProbe2              :1;
        unsigned char ucReserve1                                      :1;
        unsigned char ucAlarmDeviationPosition              :1;
        unsigned char ucAlarmTimeOutNotInPlace          :1;
        unsigned char ucAlarmUnknown                           :1;
        unsigned char ucReserve2                                      :1;
    }stAlarmStates;
}__attribute__ ((packed)) FrameStatesInfo_CarLock;


typedef struct _ParkingLockCanMapNode
{
    mutable unsigned int enType;
    mutable InfoMap stCanMap;
}ParkingLockCanMapNode;

typedef struct _ParkingLockStatus
{
     unsigned char cCarLockStatus[5];//车位锁状态
     unsigned char cParkingStatus[2];//车位状态
    unsigned  char cSensorFault[4];//传感器故障
     unsigned char cLockStructureFault[4];//锁结构或电机故障
}__attribute__ ((packed))ParkingLockStatus;

typedef struct _RecvCmdCache
{
    QDateTime dt_RecvDataTime;//接受到指令的时间
    unsigned char  ParkingLockAddress;//车位锁地址
    unsigned char   ucType; //指令类型
    QMap<unsigned int,QByteArray> RecvOrderMap;//指令内容
}stRecvCmdCache;
Q_DECLARE_METATYPE(stRecvCmdCache)

typedef struct _RecvStatusCache
{
    QDateTime dt_RecvStatusTime;//接受到状态的时间
    bool bActive;
    unsigned char  ucParkingLockAddr;//车位锁地址
    QMap<unsigned int,QByteArray> StatusInfoMap;//车位锁状态信息
}stRecvStatusCache;
Q_DECLARE_METATYPE(stRecvStatusCache)

class cParkingLockProtocol : public QObject
{
    Q_OBJECT

public:
    cParkingLockProtocol();
    ~cParkingLockProtocol();

public:
    void ParseCenterData(InfoMap CenterMap , InfoAddrType enAddrType , unsigned char ucParkingLockAddr);//解析总线数据
    void ParseCanData(can_frame *pCanFrame);//解析CAN数据
    void SendCenterData(unsigned int &uiInfoAddr, InfoMap &TermSendMap);//发送数据到车位锁类

private:
    void ParseFrameParkingLockStates(unsigned char * pData, QList<ParkingLockCanMapNode>  &ToCenterList);   //解析车位锁状态
    void ParseFrameCtrlCmdAck(unsigned char * pData,QList<ParkingLockCanMapNode> &ToCenterList);    //解析控制指令响应数据
    void ParseFrameLifeInfo(unsigned char * pData); //解析生命信息帧
    void ParseCarLockCtrlCmd(InfoMap CenterMap , unsigned char ucParkingLockAddr);  //解析车位锁控制指令
    void ParseCarLockParaSetCmd(InfoMap CenterMap , unsigned char ucParkingLockAddr);//解析车位锁设置指令
    void SendParaSetCmd(unsigned char *ucSetvalue, unsigned char ucParkingLockAddr); //发送车位锁参数设置指令
    void SendCtrlCmd(unsigned char ucCmdType, unsigned char ucParkingLockAddr); //发送车位锁控制指令
    //pCanFrame CAN组帧后存储位置,ucSa 源地址,ucPf  PGN类型,ucPs 目的地址,ucPriority 优先级,ucLength 数据长度
    bool SetFrameHead(can_frame *pCanFrame, unsigned char ucSa, unsigned char ucPf, unsigned char ucPs,unsigned char ucPriority, unsigned char ucLength);

signals:
    void sigSendToCenter(unsigned int uiInfoAddr , InfoMap TerminalDataMap);
    void sigSendCanData(can_frame *pCanFrame);//CAN帧发送信号

public slots:
    void ProcParseData(QList <can_frame *> *pTerminalRecvList);

};

#endif
