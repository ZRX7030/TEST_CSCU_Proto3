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

class dlt645_97: public QObject
{
    Q_OBJECT

public:
    dlt645_97(Log *);
    ~dlt645_97();

private:
     int g_ammeterHandle;
     uint8_t Dlt645_buff[200];
     uint8_t recData[212];//接收帧
     uint8_t targetData[200];//接收数据
     bool syncTimeFlag;
     int boardType;

    FRAME_SUB_STATION_INFO *ammeterData;
    InfoMap dataMap_645_97;

    Log * pLog_dlt64597;
	QString _strLogName;

    bool Init();
    void syncTime();
    uint8_t verify(uint8_t *data,uint32_t length);
    void decodeRecData(uint8_t *targetData,int valueFlag, bool &errFlag, FRAME_SUB_STATION_INFO&ammeterData,stAmmeterConfig &);
    void parseVol(uint8_t *targetData, bool &errFlag,FRAME_SUB_STATION_INFO&ammeterData,int,int&);
    void parseCur(uint8_t *targetData, bool &errFlag,FRAME_SUB_STATION_INFO&ammeterData,int,int&);
    void parsePower(uint8_t *targetData, bool &errFlag,FRAME_SUB_STATION_INFO&ammeterData,int,int&,int &);
    void parseRePower(uint8_t *targetData, bool &errFlag,FRAME_SUB_STATION_INFO&ammeterData,int,int&,int&);
    void parsePowerFactor(uint8_t *targetData, bool &errFlag,FRAME_SUB_STATION_INFO&ammeterData,int);
    void parseActive_absorb_energy(uint8_t *targetData,bool & errFlag,FRAME_SUB_STATION_INFO&ammeterData,int&,int&);
    void parseActive_liberate_energy(uint8_t *targetData,bool & errFlag,FRAME_SUB_STATION_INFO&ammeterData,int&,int &);
    void parsereactive_sensibility_energy1(uint8_t *targetData,bool & errFlag,int&,int&);
    void parsereactive_sensibility_energy3(uint8_t *targetData,bool & errFlag,int&,int&);
    void parsreactive_capacity_energy2(uint8_t *targetData,bool & errFlag,int&,int&);
    void parsereactive_capacity_energy4(uint8_t *targetData,bool & errFlag,int&,int&);
    bool checkRecData(int getBytes, uint8_t * recData,uint8_t *targetData,bool & errFlag, unsigned char*);
    void fixReadCMD(FRAME_DLT_645  &frame_dlt_645,unsigned char * addr,uint8_t * Dlt645_buff,uint8_t * cmdFrame);
    void readAmmeterData_97(stAmmeterConfig);

signals:
    void sendAmmeterData_645_97(InfoMap);
    void sigReadOver_645_97(int);

public slots:
    void slot_readAmmeter_dlt645_97(QList<stAmmeterConfig> infoList);
    void slot_getBoardType(int);
};
