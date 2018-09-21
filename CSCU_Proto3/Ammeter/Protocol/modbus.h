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
#include "math.h"
#include "Infotag/CSCUBus.h"
#include "ParamSet/ParamSet.h"
#include "CommonFunc/commfunc.h"
#define MODBUS_READ_DATA          0x03
#define MODBUS_AMMETER_DEFAULT_ADDRESS 0x01

class modbus: public QObject
{
    Q_OBJECT

public:
    modbus(Log *);
    ~modbus();

private:
     int g_ammeterHandle;
     uint8_t Dlt645_buff[200];
     uint8_t recData[212];//接收帧
     uint8_t targetData[200];//接收数据
     bool syncTimeFlag;
     bool getDataFlag;
     int boardType;
     unsigned char tmpAddr;

    FRAME_SUB_STATION_INFO *ammeterData;
    InfoMap dataMap_modbus;

    Log * pLog_modbus;
	QString _strLogName;

    bool Init();
    void readAmmeterData_modbus(stAmmeterConfig);
    int Modbus_Read(unsigned char *recData);
    int Modbus_Write(unsigned char *sendData,int num);
    void getVoltage_Current();
    void getActive_reactivePower();
    void getEnergy();
    void read_modbus_register(short registerAddr,int length);

signals:
    void sendAmmeterData_modbus(InfoMap);
    void sigReadOver_modbus(int);

public slots:
    void slot_readAmmeter_modbus(QList<stAmmeterConfig> infoList);
    void slot_getBoardType(int);
};
