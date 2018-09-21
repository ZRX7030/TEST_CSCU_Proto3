#ifndef AMMETERDATA_H
#define AMMETERDATA_H

#include "GeneralData/GeneralData.h"
#include "GeneralData/ModuleIO.h"
#include "Ammeter/Protocol/dlt645_07.h"
#include "Ammeter/Protocol/dlt645_97.h"
#include "Ammeter/Protocol/modbus.h"
#include "RealDataFilter.h"
#include "Bus/Bus.h"
#include "ParamSet/ParamSet.h"
#include "Log/Log.h"
#include <QDebug>
#include <QList>


class AmmeterData : public CModuleIO
{
    Q_OBJECT
public:
    AmmeterData();
    ~AmmeterData();

private:
   //AmmeterDataMap 645_07List;
    dlt645_07 *cDlt645_07;
    dlt645_97 *cDlt645_97;
    modbus *cModbus;
    Log * pLog_Ammeterdlt64507;

    //实时数据过滤模块调用指针
    RealDataFilter * pRealDataFilter;

    stAllAmmeterConfig *AmmeterInfoList;
    QList<stAmmeterConfig> AmmeterInfoList_powerMonitor;
    QList<stAmmeterConfig> AmmeterInfoList_dlt645_07;//平台下发电表地址
    QList<stAmmeterConfig> AmmeterInfoList_dlt645_97;//平台下发电表地址
    QList<stAmmeterConfig> AmmeterInfoList_modbus;//平台下发电表地址

    QTimer * actionTimer;

    bool readInlineAmmeterFlag;
    bool powerMonitorFlag;//负荷约束功率监测功能开启
    bool readRemoteAmmeterFlag;   //07电表远程抄表进行中
    InfoMap qInfoMapPrivate;

    Log * pAmmeterLog;
    ParamSet * pParamSet;
    void init();
    int getAmmeterConfig(ParamSet* &pPara);
    void parseAmmeterInfo();
    void procParamChange(InfoMap &RecvBusDataMap);
    void procRemoterReadAmmeter(InfoMap &qInfoMap);  //解析远程抄表

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


signals:
    void sig_readAmmeter_dlt645_07(QList<stAmmeterConfig>);
    void sig_readAmmeter_dlt645_97(QList<stAmmeterConfig>);
    void sig_readAmmeter_modbus(QList<stAmmeterConfig>);
    void sig_readPowerMonitorAmmeter(stAmmeterConfig);
    void sig_stopRead(bool);
    void sig_sendBoardType(int);
    void sigToBus(InfoMap, InfoAddrType);
    void sig_readRemoteAmmeter_dlt645_07(unsigned char *,int,unsigned char *,stAmmeterConfig);
public slots:
    void slotParseAmmeterData(InfoMap);
    void slotsendToBusRemoteAmmeterData(InfoMap readRemoteAmmeterMap);  //发送数据给BUS
    void slotParseReadResult(int);
    //开始工作启动槽函数
    void ProcStartWork();
    void ProcTimeOut();
    void slotFromBus(InfoMap , InfoAddrType );
    void slotReadFail();
    void slotReadSuccess();



};

#endif // AMMETERDATA_H
