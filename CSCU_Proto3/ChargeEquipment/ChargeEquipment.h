#ifndef CHARGEEQUIPMENT_H
#define CHARGEEQUIPMENT_H
#include <QObject>
#include <QTimer>
#include <QList>
#include <QEventLoop>

#include "Bus/Bus.h"
#include "GeneralData/ModuleIO.h"
#include "RealDataFilter.h"
#include "Log.h"
#include "ChargeEquipment/TerminalProtocol/J1939Protocol/ACSinCanProtocol.h"
#include "ChargeEquipment/TerminalProtocol/J1939Protocol/ACThrCanProtocol.h"
#include "ChargeEquipment/TerminalProtocol/J1939Protocol/DCCanProtocol.h"
#include "ChargeEquipment/TerminalProtocol/J1939Protocol/EnergyPlanProtocol.h"
#include "ChargeEquipment/TerminalProtocol/J1939Protocol/CSCUCanProtocol.h"
//终端协议类型枚举
typedef enum _TermProType
{
    Pro_J1939_ACSin = 1,
    Pro_J1939_ACThr = 2,
    Pro_J1939_DC = 3,
    Pro_J1939_EnergyPlan = 4,
    Pro_J1939_CSCU = 5
}TermProType;

//终端协议列表存储结构体
typedef struct _TermProNode
{
    TermProType enTermProType; //协议类型枚举值
    cTerminalProtocol * pTerminalProtocol;//协议指针
}TermProNode;

//J1939预处理类
class cJ1939PreParse : public QObject
{
    Q_OBJECT

public:
    cJ1939PreParse(QList <TermProNode *> * pTerminalProtocolListIn, Log * pLogIn);

private:
    cACSinCanProtocol * pACSinCanPro;
    cACThrCanProtocol * pACThrCanPro;
    cDCCanProtocol * pDCCanPro;
    cEnergyPlanProtocol * pEnergyPlanPro;
    cCSCUCanProtocol * pCSCUCanPro;
    Log * pLog;

public slots:
    //接收CAN总线类发送的数据解析信号
    void ProcParseData(QList <can_frame *> *pTerminalRecvList, QMutex * pRecvListMutex);
};

//充电设备类
class cChargeEquipment : public CModuleIO
{
    Q_OBJECT
public:
    cChargeEquipment();
    ~cChargeEquipment();

    ParamSet * gpParamSet;  //add by zrx
    stCanConfig can0Config;

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
    //初始化
    void Init();
    //CAN线程初始化
    void CanThreadInit();
    //终端协议初始化
    void TerminalProtocolInit();
    //绑定CAN协议
    bool BindCanPro();

public:
    //终端协议列表
    QList <TermProNode *> * pTerminalProtocolList;
    //J1939接收预处理类
    cJ1939PreParse * pJ1939PreParse;
protected:
    QThread* m_pWorkThread;
private:
    //工作状态标识
    bool bWorkFlag;
    //1s定时器
    QTimer * pOneSecTimer;
    //CAN总线处理线程指针
    QThread * pCanThread;
    //CAN总线类指针
    cCanBus * pCanBus;
    //实时数据过滤模块调用指针
    RealDataFilter * pRealDataFilter;
    //缓存模块调用指针
    DevCache * pDevCache;
    //日志模块调用指针
    Log * pLog;
    ParamSet * pParamSet;

signals:
    //发送数据到总线
    void sigToBus(InfoMap TelecontrolMap, InfoAddrType enAddrType);

public slots:
    //开始工作启动槽函数
    void ProcStartWork();
    //接收总线数据 -- 调用终端总线数据处理函数
    void slotFromBus(InfoMap RecvCenterDataMap,  InfoAddrType enAddrType);

private slots:
    //1s定时处理函数
    void ProcOneSecTimeOut();
    //接收协议解析后数据 -- 发送给控制总线
    void ProcRecvProtocolData(unsigned int uiInfoAddr, InfoMap TerminalDataMap);
};

#endif // CHARGEEQUIPMENT_H
