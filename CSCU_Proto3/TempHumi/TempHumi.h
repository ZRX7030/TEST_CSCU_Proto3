#ifndef TEMPHUMI_H
#define TEMPHUMI_H

#include <QObject>
#include <QTimer>
#include <QThread>
#include "GeneralData/ModuleIO.h"
#include "RealDataFilter/RealDataFilter.h"
#include "DevCache/DevCache.h"
#include "ParamSet/ParamSet.h"
#include <fcntl.h>
#include <termios.h>

#define MODBUS_THERMOMETER1_DEFAULT_ADDRESS   0x02

#define MODBUS_READ_DATA                0x03

class cTempHumi : public CModuleIO
{
    Q_OBJECT
   public:

        cTempHumi();
       ~cTempHumi();
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
            FRAME_SUB_STATION_ENVIRONMENT *temphumiData;
            InfoMap dataTempHumi;
            //实时数据过滤模块调用指针
            RealDataFilter * pRealDataFilter;

            void run();
            QTimer * actionTimer;
            //QList<stTempHumiConfig> TempHumiInfoList_modbus;
        private slots:
            void ProcTimeOut();
            void ProcStartWork();
            void slotParseTempHumiData(InfoMap);
        private:
            cTempHumi *TempHumi_Modbus;
            DevCache* p_pDevCache;	//数据缓存
            ParamSet * pParamSet;//读取参数指针
            unsigned char m_psRecvBuf[256];
            int         length;
            bool     T_Init();     //温度初始化
            bool ParseCmd(unsigned char *data,int length);
            int Serial_Modbus_Temperature_Init(int baudrate);
            int Modbus_Temperature_Humi_Read(unsigned char *recData);
            int Modbus_Temperature_Humi_Write(unsigned char *sendData,int length);

            int Modbus_CRC16(unsigned char* pchMsg, int wDataLen);
            int Read_Modbus_Temperature();
        signals:
            //void sig_readTempHumi_modbus(QList<stTempHumiConfig>);
    		void sigToBus(InfoMap mapInfo, InfoAddrType type);
            void sendTempHumiData(InfoMap);
};

/*****************************************************************
local function
**/


//// 函数功能:将16进制数组转换成qstring格式 YCZ
 QString ConvertHex2Qstr(unsigned char *psData,int len);


#endif // TEMPHUMI_H
