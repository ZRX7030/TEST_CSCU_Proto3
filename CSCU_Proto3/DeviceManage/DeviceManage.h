#ifndef DEVICEMANAGE_H
#define DEVICEMANAGE_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include <QTcpSocket>
#include <QHostAddress>
#include <QFile>
#include <netinet/in.h>

#include "GeneralData/GeneralData.h"
#include "DevCache/DevCache.h"
#include "Bus/Bus.h"
#include "ParamSet/ParamSet.h"
#include "GeneralData/ModuleIO.h"
#include "Database/DBOperate.h"
#include "Infotag/CSCUBus.h"

#define GPS_FILE_NAME  "/mnt/nandflash/gpsinfo";

const unsigned int PortNum_TrafficState= 6300;

const unsigned int uiFaultDuration = 60; //超过60s未收到故障帧, 则认为故障消失

typedef struct _DynamicArgResult
{
    mutable unsigned char ucCanID;
    mutable unsigned char ucArgNum;
}DynamicArgResult;

class cDeviceManage : public CModuleIO
{
    Q_OBJECT
public:
    cDeviceManage();
    ~cDeviceManage();

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
    //检查  故障持续时间
    void CheckFaultDuration();
    //创建 设备故障记录
    void CreatFaultRecord(const FaultRecord_DCcab &ListRecord);
    //创建 设备规格信息记录
    void CreatSpecInfoRecord(const SpecificInfo_DCcab &ListRecord);
    //加载 告警数据库中内容到故障列表(程序启动运行1次)
    void LoadFaultDBtoList();

    //查询 流量统计
    void QueryTrafficState(QByteArray Devname);
    //查询 GPS信息, 0 失败, 1 成功
    int QueryGPSInfo(QString FileName, GPS_DevMng &st );

    //发送  总线数据
    void SendCenterData(InfoMap &ToCenterMap, InfoAddrType enType);

    //发送  故障状态变化信号到总线
    void SendFaultStateChange(const FaultRecord_DCcab &stRecord);
    //解析  故障状态, 并更新故障列表
    void ParseFaultState(InfoMap &CenterMap);
    //解析  终端设备规格信息, 并更新终端设备规格列表
    void ParseTermSpecInfo(InfoMap &CenterMap);
    //解析  终端动态参数设置, 并更新终端动态参数设置表
    void ParseTermDynamicArg(InfoMap &CenterMap, unsigned int uiType);
    //解析  终端静态参数设置, 并更新终端动态参数设置表
    void ParseTermStaticArg(InfoMap CenterMap, unsigned int uiType);
    //解析  终端主动防护参数设置,并更新终端主动防护参数表
    void ParseTermActiveArg(InfoMap &CenterMap, unsigned int uiType);
    //解析  终端柔性充电参数设置,并更新终端柔性充电参数表
    void ParseTermFlexArg(InfoMap &CenterMap, unsigned int uiType);

    //解析  CCU参数设置, 并更新数据库CCU参数表
    void ParseCCUArg(InfoMap &CenterMap);

    //解析 更新终端动态参数表----根据缓存中设备实时状态刷新
    void ParseRenewGeneralDynamicArgDB();
    //解析 更新静端动态参数表----根据缓存中设备实时状态刷新
    void ParseRenewGeneralStaticArgDB();

    //更新 终端动态参数设置表----根据缓存中设备实时状态刷新
    void RenewGeneralDynamicArgDB();
    //更新 终端静态参数设置表----根据缓存中设备实时状态刷新
    void RenewGeneralStaticArgDB();

    //更新 设备故障数据库
    void UpdateFaultRecord(const FaultRecord_DCcab &ListRecord);
    //更新 主动防护库
    void UpdateActiveRecord(InfoMap &CenterMap);
    //更新 柔性充电库
    void UpdateFlexRecord(InfoMap &CenterMap);

private:
    bool bWorkFlag;// TRUE: 工作中, FALSE: 已经停止工作
    bool bTrafficConnectFlag;//TRUE: 流量统计连接建立, FALSE: 没有建立连接
    int iFaultSerialNum;
    //外部输入参数
    DevCache * pDevCache;
    ParamSet * pParamSet;
    DBOperate * pDBOperate;
    Log * pLog;
    QTimer * pOneSecTimer;
    QTcpSocket * pClient;
    QList <FaultRecord_DCcab> FaultRecordList;
    QList <SpecificInfo_DCcab> SpecificInfoList;
    QList <DynamicArgResult> DynamaicArgList;

signals:
    void sigToBus(InfoMap, InfoAddrType);

public slots:
    //接收控制中心数据
    void slotFromBus(InfoMap RecvCenterMap, InfoAddrType enType);

private slots:
    //开始工作
    void ProcStartWork();
    //每秒处理
    void ProcOneSecTimeOut();
    //流量统计 建立连接
    void ProcTrafficStateConnected();
    //流量统计 断开连接
    void ProcTrafficStateDisConnected();
};

#endif // DEVICEMANAGE_H
