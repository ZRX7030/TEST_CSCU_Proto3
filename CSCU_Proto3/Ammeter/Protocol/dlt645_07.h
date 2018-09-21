/*
 * GeneralData.cpp
 *
 * 类描述：公共数据类
 * 创建:
 * 修改记录:
 * 见 GeneralData.cpp
 */

#include "GeneralData/GeneralData.h"
#include "GeneralData/104_info_struct.h"
#include <QObject>
#include <QMap>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <termios.h>

#include "ParamSet/ParamSet.h"
#include "Infotag/CSCUBus.h"
#include "Log/Log.h"
#include "CommonFunc/commfunc.h"


class dlt645_07: public QObject
{
    Q_OBJECT

public:
    dlt645_07(Log *);
    ~dlt645_07();

private:
     int g_ammeterHandle;
     uint8_t Dlt645_buff[20];
     uint8_t recData[212];//接收帧
     uint8_t targetData[200];//接收数据
     bool syncTimeFlag;
     int boardType;
     bool initFlag;
     bool stopReadFlag;
     uint uiCurrentReactiveLiberateEnergy;      //当前正向无功电能-第一象限无功总电能
     uint uiCurrentReactiveAbsortEnergy;         //当前反向无功电能-第三象限无功总电能
     uint uiDayFreezeReactiveLiberateEnergy; //日冻结正向无功电能-第一象限无功总电能
     uint uiDayFreezeReactiveAbsortEnergy;    //日冻结反向无功电能-第三象限无功总电能
     uint uiSettlementReactiveLiberateEnergy; //结算日正向无功电能-第一象限无功总电能
     uint uiSettlementReactiveAbsortEnergy;    //结算日反向无功电能-第三象限无功总电能

     struct termios tio;

    FRAME_SUB_STATION_INFO *ammeterData;
    InfoMap dataMap_645_07;

    Log * pLog_dlt64507;
	QString _strLogName;

    bool Init();
    void syncTime();
    uint8_t verify(uint8_t *data,uint32_t length);
    void decodeRecData(uint8_t *targetData,int valueFlag, bool &errFlag, FRAME_SUB_STATION_INFO&ammeterData,stAmmeterConfig &info);
    void parseVol(uint8_t *targetData, bool &errFlag,FRAME_SUB_STATION_INFO&ammeterData,int &);
    void parseCur(uint8_t *targetData, bool &errFlag,FRAME_SUB_STATION_INFO&ammeterData,int &);
    void parsePower(uint8_t *targetData, bool &errFlag,FRAME_SUB_STATION_INFO&ammeterData,int &,int &);
    void parseRePower(uint8_t *targetData, bool &errFlag,FRAME_SUB_STATION_INFO&ammeterData,int &,int &);
    void parsePowerFactor(uint8_t *targetData, bool &errFlag,FRAME_SUB_STATION_INFO&ammeterData);
    void parseCur_0(uint8_t *targetData,FRAME_SUB_STATION_INFO&ammeterData,int &);
    void parseActive_absorb_energy(uint8_t *targetData,bool & errFlag,FRAME_SUB_STATION_INFO&ammeterData,int &,int &);
    void parseActive_liberate_energy(uint8_t *targetData,bool & errFlag,FRAME_SUB_STATION_INFO&ammeterData,int &,int &);
    void parsereactive_sensibility_energy1(uint8_t *targetData,bool & errFlag,int &,int &);
    void parsereactive_sensibility_energy3(uint8_t *targetData,bool & errFlag,int &,int &);
    void parsreactive_capacity_energy2(uint8_t *targetData,bool & errFlag,int &,int &);
    void parsereactive_capacity_energy4(uint8_t *targetData, bool & errFlag,int &,int &);
    bool checkRecData(int getBytes, uint8_t * recData,uint8_t *targetData,bool & errFlag, unsigned char*);
    void fixReadCMD(FRAME_DLT_645  &frame_dlt_645,unsigned char * addr,uint8_t * Dlt645_buff,uint8_t * cmdFrame);
    void readAmmeterData_07(stAmmeterConfig);
    void ReadInlineAmmeter(stAmmeterConfig &info);
    void ReadPowerMonitorAmmeter(stAmmeterConfig &info);

    /**************************************远程读07电表数据*****************************************************/
    bool remoteReadAmmeterData_07(unsigned char *ammeterId,int readDataType,unsigned char *readingTime,stAmmeterConfig info);   //远程读07电表数据
    void remoteReadAmmeterData_07_CurrentEnergy(unsigned char *ammeterId,unsigned char *readingTime, stAmmeterConfig &info);   //远程读07电表数据-当前电能
    void remoteReadAmmeterData_07_HourFreezeEnergy(unsigned char *ammeterId,unsigned char *readingTime,stAmmeterConfig &info);   //远程读07电表数据-整点冻结电能
    void remoteReadAmmeterData_07_DayFreezeEnergy(unsigned char *ammeterId,unsigned char *readingTime, stAmmeterConfig &info);   //远程读07电表数据-日冻结电能
    void remoteReadAmmeterData_07_SettlementEnergy(unsigned char *ammeterId,unsigned char *readingTime, stAmmeterConfig &info);   //远程读07电表数据-结算日电能
    void decodeRemoteRecDataCurrent(uint8_t *targetData,int valueFlag, bool &errFlag,stAmmeterConfig &info);   //解析数据-当前
    void decodeRemoteRecDataHourFreeze(uint8_t *targetData,bool &errFlag,stAmmeterConfig &info);   //解析数据-整点冻结
    void decodeRemoteRecDataDayFreeze(uint8_t *targetData,int valueFlag, bool &errFlag,stAmmeterConfig &info);   //解析数据-日冻结
    void decodeRemoteRecDataSettlement(uint8_t *targetData,int valueFlag, bool &errFlag,stAmmeterConfig &info);   //解析数据-结算日
    void parse_current_energy(int valueFlag,uint8_t *targetData,bool & errFlag,FRAME_CURRENT_ENERGY_MAX_DEMAND_DATA &currentEnergyMaxDemandData,int &PT_value,int &CT_value);  //(当前)电能解析函数
    void parse_current_max_demand_time(int valueFlag,uint8_t *targetData,bool & errFlag,FRAME_CURRENT_ENERGY_MAX_DEMAND_DATA &currentEnergyMaxDemandData,int &PT_value,int &CT_value);  //(当前)最大需量及发生时间解析函数
    void parse_hour_freeze_energy_data(uint8_t *targetData,bool & errFlag,FRAME_HOUR_FREEZE_ENERGY_DATA &hourFreezeEnergyData,int &PT_value,int &CT_value);  //整点冻结电能解析函数
    void parse_day_freeze_energy(int valueFlag,uint8_t *targetData,bool & errFlag,DAY_FREEZE_ENERGY_MAX_DEMAND_DATA &dayFreezeEnergyMaxDemandData,int &PT_value,int &CT_value);  //日冻结电能解析函数
    void parse_day_freeze_max_demand_time(int valueFlag,uint8_t *targetData,bool & errFlag,DAY_FREEZE_ENERGY_MAX_DEMAND_DATA &dayFreezeEnergyMaxDemandData,int &PT_value,int &CT_value);  //日冻结最大需量及发生时间函数
    void parse_settlement_energy(int valueFlag,uint8_t *targetData,bool & errFlag,SETTLEMENT_ENERGY_MAX_DEMAND_DATA &settlementEnergyMaxDemandData,int &PT_value,int &CT_value);  //日冻结电能解析函数
    void parse_settlement_max_demand_time(int valueFlag,uint8_t *targetData,bool & errFlag,SETTLEMENT_ENERGY_MAX_DEMAND_DATA &settlementEnergyMaxDemandData,int &PT_value,int &CT_value);  //日冻结最大需量及发生时间函数

signals:
    void sendAmmeterData_645_07(InfoMap);
    void sig_readFail_645_07();
    void sig_readSucess_645_07();
    void sigReadOver_645_07(int);
    void sig_sendToBusRemoteAmmeterData_645_07(InfoMap);

public slots:
    void slot_readRemoteAmmeter_dlt645_07(unsigned char *,int,unsigned char *,stAmmeterConfig);

    void slot_readAmmeter_dlt645_07(QList<stAmmeterConfig>);
    void slot_readPowerMonitorAmmeter(stAmmeterConfig);
    void slot_stopRead(bool);
    void slot_getBoardType(int);
};

