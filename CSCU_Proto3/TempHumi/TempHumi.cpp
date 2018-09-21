#include <stdio.h>
#include <stdlib.h>
#include "QDebug"
#include "TempHumi/TempHumi.h"

//int g_ModbusHandle=-1;
int temperature_Humi = -1;

// --------------------------------------------------------------
//      CRC16计算方法1:使用2个256长度的校验表
// --------------------------------------------------------------
const char chCRCHTalbe[] =                                 // CRC 高位字节值表
{
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
        0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40
        };
const char chCRCLTalbe[] =                                 // CRC 低位字节值表
{
        0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7,
        0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,
        0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9,
        0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
        0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
        0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,
        0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D,
        0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,
        0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF,
        0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
        0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1,
        0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
        0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB,
        0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,
        0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
        0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
        0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97,
        0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,
        0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89,
        0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
        0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83,
        0x41, 0x81, 0x80, 0x40
        };

int cTempHumi::Serial_Modbus_Temperature_Init(int bitrate)
{
    struct termios tio;
    QString sPortNum;
    stCSCUSysConfig cscuSysConfig;       //CSCU参数设置缓存..
    //获取底板型号
    pParamSet->querySetting(&cscuSysConfig, PARAM_CSCU_SYS);
    switch(cscuSysConfig.boardType)
    {
    case 1:
        sPortNum = TEMPERATURE_SERIAL_NUM_1;
        break;
    case 2:
        sPortNum = TEMPERATURE_SERIAL_NUM_2;
        break;
    case 3:
        sPortNum = TEMPERATURE_SERIAL_NUM_3;
        break;
    default:
        sPortNum = TEMPERATURE_SERIAL_NUM_2;
        break;
    }
    if((temperature_Humi=open(sPortNum.toAscii().data(), O_RDWR|O_NOCTTY|O_NONBLOCK))<0)
    {
        return false;
    }
    else
    {
        ;
    }
    switch(bitrate)
    {
    case 0:
    default:
        tio.c_cflag=B2400|CS8|CREAD|CLOCAL;
        break;
    case 1:
        tio.c_cflag=B4800|CS8|CREAD|CLOCAL;
        break;
    case 2:
        tio.c_cflag=B9600|CS8|CREAD|CLOCAL;
        break;
    case 3:
        tio.c_cflag=B19200|CS8|CREAD|CLOCAL;
        break;
    case 4:
        tio.c_cflag=B38400|CS8|CREAD|CLOCAL;
        break;
    case 5:
        tio.c_cflag=B600|CS8|CREAD|CLOCAL;
        break;
    case 6:
        tio.c_cflag=B1200|CS8|CREAD|CLOCAL;
        break;
    }
    //tio.c_cflag=B115200|CS8|CREAD|CLOCAL;
    tio.c_cflag&=~HUPCL;
    //偶校验
    tio.c_iflag |= (INPCK | ISTRIP);
    tio.c_cflag |= PARENB;
    tio.c_cflag &= ~PARODD;
    tio.c_lflag=0;
    tio.c_iflag=IGNPAR;
    tio.c_oflag=0;
    tio.c_cc[VTIME]=150;
    tio.c_cc[VMIN]=1;

    tcflush(temperature_Humi,TCIFLUSH);
    tcsetattr(temperature_Humi,TCSANOW,&tio);
    return true;
}

int cTempHumi::Modbus_Temperature_Humi_Read(unsigned char *recData)
{
    int nbytes = -1;
    if(recData != NULL)
    {
        nbytes = read(temperature_Humi,recData, 100);
    }
    return nbytes;
}

int cTempHumi::Modbus_Temperature_Humi_Write(unsigned char *sendData,int num)
{
    int nbytes = -1;
    if(sendData != NULL)
        nbytes = write(temperature_Humi,sendData, num);

    return nbytes;
}

//温湿度1 L2W2K系统
int cTempHumi::Read_Modbus_Temperature()
{
    unsigned char sendData[10];
    int iOffset = 0;
    int crc16 = 0;

    sendData[0] = MODBUS_THERMOMETER1_DEFAULT_ADDRESS;
    iOffset ++;

    sendData[1] = MODBUS_READ_DATA;
    iOffset ++;

    //寄存器地址
    sendData[2] = 0;//
    sendData[3] = 0;
    iOffset = iOffset +2;

    //length
    sendData[4] = 0x00;
    sendData[5] = 0x02;
    iOffset = iOffset +2;

    crc16 = Modbus_CRC16(sendData,iOffset);
    sendData[6] = crc16 & 0xFF;
    sendData[7] = crc16 >> 8;
    iOffset = iOffset +2;

    Modbus_Temperature_Humi_Write(sendData,iOffset);
    return 1;
}

int cTempHumi::Modbus_CRC16(unsigned char* pchMsg, int wDataLen)
{
    char chCRCHi = 0xFF; // 高CRC字节初始化
    char chCRCLo = 0xFF; // 低CRC字节初始化
    short wIndex;            // CRC循环中的索引
    while (wDataLen--)
    {
        // 计算CRC
        wIndex = chCRCLo ^ *pchMsg++ ;
        chCRCLo = chCRCHi ^ chCRCHTalbe[wIndex];
        chCRCHi = chCRCLTalbe[wIndex] ;
    }
    return ((chCRCHi << 8) | chCRCLo) ;
}

bool cTempHumi::T_Init()
{
    int baudrate = 6;
    int ret = Serial_Modbus_Temperature_Init(baudrate);
    return !!ret;
}

void cTempHumi::run()
{
//    timer.start();
    Read_Modbus_Temperature();
    usleep(3000);
    if((length = Modbus_Temperature_Humi_Read(m_psRecvBuf))>2)
    {
        QString TempStr;
        TempStr.sprintf("modbusThread::run: ");
        TempStr += ConvertHex2Qstr(m_psRecvBuf, length);
        ParseCmd(m_psRecvBuf,length);
    }
    else
    {
        temphumiData->temperature = 0;
        temphumiData->humidity = 0;
        dataTempHumi.insert(Addr_StationEnvTemp,QByteArray((char*)&temphumiData->temperature,2));
        dataTempHumi.insert(Addr_StationEnvHumi,QByteArray((char*)&temphumiData->humidity,2));
        pRealDataFilter->realDataUpdate(dataTempHumi, AddrType_TempHumi);
        dataTempHumi.clear();
    }
}

// 函数功能:将16进制数组转换成qstring格式 YCZ
QString ConvertHex2Qstr(unsigned char *psData,int len){
    QString dst_str;
    for(int i = 0; i<len; i++){
        unsigned char outchar =* (psData + i);
        QString str = QString("%1 ").arg(outchar&0xFF, 2, 16, QLatin1Char('0'));
        dst_str += str;
    }
    return dst_str;
}

bool cTempHumi::ParseCmd(unsigned char *data, int length)
{
    int crc16,crc16_rec = 0;
    float temperature = 0;
    float humidity = 0;

    if(data == NULL)
        return false;
    if(length < 3)
        return false;
    crc16 = Modbus_CRC16(data,length-2);
    crc16_rec = (data[length-1] * 256) +  data[length-2];

    switch(data[0])
    {
    case MODBUS_THERMOMETER1_DEFAULT_ADDRESS://温湿度
        if((data[1] == 0x03) && (data[2] == 0x04))
        {
            temperature = ( data[3]*256+data[4]);
            humidity = (data[5]*256+data[6]);
        }
        temphumiData->temperature = temperature;
        temphumiData->humidity = humidity;
        dataTempHumi.insert(Addr_StationEnvTemp,QByteArray((char*)&temphumiData->temperature,2));
        dataTempHumi.insert(Addr_StationEnvHumi,QByteArray((char*)&temphumiData->humidity,2));
        pRealDataFilter->realDataUpdate(dataTempHumi, AddrType_TempHumi);
        dataTempHumi.clear();
        break;
    }
    return true;
}

cTempHumi::cTempHumi()
{
    temphumiData = new FRAME_SUB_STATION_ENVIRONMENT;
    pRealDataFilter = RealDataFilter::GetInstance();
    pParamSet = ParamSet::GetInstance();
}

cTempHumi::~cTempHumi()
{
    delete temphumiData;
}

//启动模块
void cTempHumi::ProcStartWork()
{
    T_Init();
    actionTimer = new QTimer();
    actionTimer->start(3000);
    connect(actionTimer,SIGNAL(timeout()), SLOT(ProcTimeOut()));
}

void cTempHumi::ProcTimeOut()
{
    run();
}

//根据配置选项初始化
int cTempHumi::InitModule( QThread* pThread)
{
    m_pWorkThread = pThread;

    this->moveToThread(m_pWorkThread);
    QObject::connect(m_pWorkThread, SIGNAL(started()), this, SLOT(ProcStartWork()));
    return 0;
}

//注册设备到总线
int cTempHumi::RegistModule()
{
	QList<int> List;

	CBus::GetInstance()->RegistDev(this, List);

	return 0;
}

//启动模块
int cTempHumi::StartModule()
{
    m_pWorkThread->start();
    return 0;
}

//停止模块
int cTempHumi::StopModule()
{
    if(actionTimer->isActive())
    {
        actionTimer->stop();
        delete actionTimer;
    }
    return 0;
}

//模块工作状态
int cTempHumi::ModuleStatus()
{
    return 0;
}

CModuleIO* CreateDevInstance()
{
    return new cTempHumi();
}

void DestroyDevInstance(CModuleIO* pModule)
{

    if(pModule)
    {
        delete pModule;
    }
}
