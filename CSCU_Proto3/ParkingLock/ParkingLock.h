#ifndef PARKINGLOCK_H
#define PARKINGLOCK_H

#include <QObject>
#include <QTimer>
#include <QThread>
#include <QMap>
#include <QEventLoop>

#include "Infotag/CSCUBus.h"
#include "DevCache/DevCache.h"
#include "RealDataFilter/RealDataFilter.h"
#include "GeneralData/GeneralData.h"
#include "GeneralData/ModuleIO.h"
#include "ParamSet/ParamSet.h"
#include "ParkingLock/Can1/Can1Bus.h"
#include "ParkingLock/ParkingLockProtocol.h"


class ParkingLock : public CModuleIO    //继承main的类
{
    Q_OBJECT
public:
    ParkingLock();
    ~ParkingLock();

    //根据配置选项初始化
    int InitModule (QThread *pThread);
    //注册设备到总线
    int RegistModule();
    //启动模块
    int StartModule ();
    //停止模块
    int StopModule ();
    //模块工作状态
    int ModuleStatus ();

private:
    //初始化
    void Init();

public:
    cParkingLockProtocol *pParkingLockProtocol;
    QList <stRecvStatusCache> CarLockStatusCacheList;
    QList <stRecvCmdCache> CmdCacheList;

protected:
    QThread *m_pWorkThread;

private: 
    bool bWorkFlag;     //工作状态标志
    QTimer *pOneSecTimer;   //1s定时器
    QTimer *pMinutesTimer;   //定时器
    cCan1Bus  *pCan1Bus;    //CAN总线类指针
    DevCache *pDevCache;    //数据缓存模块调用指针
    ParamSet *pParamSet;    //参数设置模块调用指针
    RealDataFilter *pRealDataFilter;   //实时数据过滤模块调用指针
    Log * pLog;     //日志模块调用指针

public:
    void CarLockStatusListInit(QList <stRecvStatusCache> &CarLockStatusCacheList);
    void CarLockStatusDataUpdate(InfoMap TerminalDataMap, InfoAddrType enAddrType,QList <stRecvStatusCache> &CarLockStatusCacheList);//更新车位锁状态数据
    void InsertCmdCache(InfoMap CenterMap,InfoAddrType enAddrType,unsigned char ucParkingLockAddr,QList <stRecvCmdCache> &CacheList);

signals:
    void sigToBus(InfoMap mapInfo, InfoAddrType type);    //发送数据到总线

public slots:
    void slotFromBus(InfoMap RecvCenterDataMap,  InfoAddrType enAddrType);    //接收总线数据

private slots:
    void ProcStartWork(); //开始工作启动槽函数
    void ProcOneSecTimeOut();//1s定时处理函数,发送指令
    void ProcMinutesTimeOut();//定时发送车位锁状态
    void ProcRecvProtocolData(unsigned int uiInfoAddr , InfoMap TerminalDataMap);//接收协议解析后的数据---１更新状态　２发送到总线

};

#endif // PARKINGLOCK_H
