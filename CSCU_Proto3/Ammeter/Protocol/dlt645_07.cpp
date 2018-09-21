#include "dlt645_07.h"
#include <QDebug>
#include <qcoreapplication.h>

uint8_t DLT645_07_item[12][4]={{0x33,0x32,0x34,0x35},{0x33,0x32,0x35,0x35},{0x33,0x32,0x36,0x35},{0x33,0x32,0x37,0x35},
                               {0x33,0x32,0x39,0x35},{0x34,0x33,0xB3,0x35},{0x33,0x33,0x34,0x33},{0x33,0x33,0x35,0x33},
                               {0x33,0x33,0x38,0x33},{0x33,0x33,0x39,0x33},{0x33,0x33,0x3a,0x33},{0x33,0x33,0x3b,0x33}};
uint8_t DLT645_07_readType01[12][4]={{0x33,0x33,0x34,0x33},{0x33,0x34,0x33,0x33},{0x33,0x35,0x33,0x33},{0x33,0x36,0x33,0x33},
                                     {0x33,0x37,0x33,0x33},{0x33,0x33,0x35,0x33},{0x33,0x33,0x38,0x33},{0x33,0x33,0x39,0x33},
                                     {0x33,0x33,0x3A,0x33},{0x33,0x33,0x3B,0x33},{0x33,0x33,0x34,0x34},{0x33,0x33,0x35,0x34}};
uint8_t DLT645_07_readType03[9][4]={{0x34,0x33,0x39,0x38},{0x34,0x34,0x39,0x38},{0x34,0x35,0x39,0x38},{0x34,0x38,0x39,0x38},
                                     {0x34,0x39,0x39,0x38},{0x34,0x3A,0x39,0x38},{0x34,0x3B,0x39,0x38},{0x34,0x3C,0x39,0x38},
                                     {0x34,0x3D,0x39,0x38}};
uint8_t DLT645_07_readType04[12][4]={{0x34,0x33,0x34,0x33},{0x34,0x34,0x34,0x33},{0x34,0x35,0x34,0x33},{0x34,0x36,0x34,0x33},
                                     {0x34,0x37,0x34,0x33},{0x34,0x33,0x35,0x33},{0x34,0x33,0x38,0x33},{0x34,0x33,0x39,0x33},
                                     {0x34,0x33,0x3A,0x33},{0x34,0x33,0x3B,0x33},{0x34,0x33,0x34,0x34},{0x34,0x33,0x35,0x34}};


dlt645_07::dlt645_07(Log * log)
{
	_strLogName = "ammeter07";
    //exitFlag = false;
    pLog_dlt64507 = log;

     g_ammeterHandle = -1;
     memset(Dlt645_buff,0x0,sizeof(Dlt645_buff));
     memset(recData,0x0,sizeof(recData));
     memset(targetData,0x0,sizeof(targetData));

     ammeterData = new FRAME_SUB_STATION_INFO;
//     currentEnergyMaxDemandData = new FRAME_CURRENT_ENERGY_MAX_DEMAND_DATA;

     syncTimeFlag = false;
//     monitorPowerFlag = false;//监控功率电表存在标志
     initFlag = false;//初始化标志位
     stopReadFlag = false;

     uiCurrentReactiveLiberateEnergy = 0;
     uiCurrentReactiveAbsortEnergy = 0;
     uiDayFreezeReactiveLiberateEnergy = 0;
     uiDayFreezeReactiveAbsortEnergy = 0;
     uiSettlementReactiveLiberateEnergy = 0;
     uiSettlementReactiveAbsortEnergy = 0;
}

dlt645_07::~dlt645_07()
{
    if(g_ammeterHandle != -1)
    {
       close(g_ammeterHandle);
    }

    delete ammeterData;
}

void dlt645_07::slot_getBoardType(int getBoardType)
{
   boardType = getBoardType;
//   initFlag = Init();
}

//bool dlt645_07::Init(int readType)//串口通信初始化
bool dlt645_07::Init()//串口通信初始化
{
    if(initFlag)//已初始化
    {
        return true;
    }

//    struct termios tio;

    //临时屏蔽
    if(boardType == 1)//1.0硬件
    {
        g_ammeterHandle = open(AMMETER_SERIAL_NUM_1,O_RDWR|O_NOCTTY);
    }
    else if(boardType == 2)//2.0硬件
    {
        g_ammeterHandle = open(AMMETER_SERIAL_NUM_2,O_RDWR|O_NOCTTY);
    }
    else if(boardType == 3)//模块化集控硬件
    {
        g_ammeterHandle = open(AMMETER_SERIAL_NUM_3,O_RDWR|O_NOCTTY);
    }

    pLog_dlt64507->getLogPoint(_strLogName)->info("dlt645_07 Init, handle 　　  ===     "+QString::number(g_ammeterHandle));
    pLog_dlt64507->getLogPoint(_strLogName)->info("dlt645_07 Init, boardType    ===     "+QString::number(boardType));
//    pLog_dlt64507->getLogPoint(_strLogName)->info("dlt645_07 Init, readType    ===     "+QString::number(readType));

    if(g_ammeterHandle < 0)
    {
        pLog_dlt64507->getLogPoint(_strLogName)->info("dlt645_07 Init Fail  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!   ");
        return false;
    }
    else
    {
        cfsetispeed(&tio,B2400 );
        cfsetospeed(&tio,B2400);

        //tio.c_cflag &= ~CSTOPB;//一位停止位

        tio.c_cflag |= PARENB;
        tio.c_cflag &= ~PARODD;
        tio.c_cflag &= ~CSIZE;
        tio.c_cflag |= CS8;            // 8n1 (8 位元, 不做同位元检查,1 个终止位元)
        tio.c_cflag |= CLOCAL | CREAD;

        tio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
        tio.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
        tio.c_iflag &= ~ (INLCR | ICRNL | IGNCR);

        tio.c_oflag &= ~(ONLCR | OCRNL);
        tio.c_oflag &= ~OPOST;

        tio.c_cc[VTIME]=1;
        tio.c_cc[VMIN]=1;

        tcflush(g_ammeterHandle,TCIFLUSH);
        tcsetattr(g_ammeterHandle,TCSANOW,&tio);
        fcntl(g_ammeterHandle,F_SETFL,FNDELAY);
    }

    initFlag = true;
    return true;
}

void dlt645_07::syncTime()
{
    uint8_t  timeFrame[6];

    //每天04:01对时
    QDateTime dt = QDateTime::currentDateTime();
    short year=dt.toString("yy").toInt();
    short month = dt.toString("MM").toInt();
    short day = dt.toString("dd").toInt();
    short mm=dt.toString("mm").toInt();
    short hh = dt.toString("hh").toInt();
    short ss = dt.toString("ss").toInt();
    timeFrame[0]=uint8_t(ss);                timeFrame[1]=uint8_t(mm);
    timeFrame[2]=uint8_t(hh);                timeFrame[3]=uint8_t(day);
    timeFrame[4]=uint8_t(month);         timeFrame[5]=uint8_t(year);

    if(hh == 04 && mm == 01 && !syncTimeFlag)
    {//
        pLog_dlt64507->getLogPoint(_strLogName)->info("sync time with ammeter ");

        FRAME_DLT_645 frame_dlt_645;
        frame_dlt_645.frame_start = FRAME_START;//帧起始符
        frame_dlt_645.frame_start2 = FRAME_START;//帧起始符
        frame_dlt_645.controlCode =0x08;//控制码(读数据)
        frame_dlt_645.data_length = 0x06;//数据域长度(仅含数据标识，长度固定为4字节)
        memset(&frame_dlt_645.addr,0x99,6);

        memcpy(Dlt645_buff,(unsigned char *)&frame_dlt_645,10);
        memcpy(&Dlt645_buff[10],timeFrame,frame_dlt_645.data_length);//数据域(仅含数据标识)
        Dlt645_buff[frame_dlt_645.data_length+10] = verify(Dlt645_buff,frame_dlt_645.data_length+10);//校验码
        Dlt645_buff[frame_dlt_645.data_length+11] = FRAME_END;//结束符

        write(g_ammeterHandle,Dlt645_buff,18);

        syncTimeFlag = true;
    }
    else if(hh != 04 && mm != 01 && syncTimeFlag)
    {
        syncTimeFlag = false;
    }

}


uint8_t dlt645_07::verify(uint8_t *data,uint32_t length)//计算校验位
{
    uint32_t i;
    uint8_t result=0;
    for(i=0;i<length;i++)
    {
        result+=data[i];
    }
    return (result &0x00FF);
}


void dlt645_07::decodeRecData(uint8_t *targetData,int valueFlag, bool &errFlag, FRAME_SUB_STATION_INFO&ammeterData,stAmmeterConfig &info)
{
    switch(valueFlag)
    {
       case 0:
        parseVol(targetData, errFlag,ammeterData,info.voltageRatio);
        break;
    case 1:
        parseCur(targetData, errFlag,ammeterData,info.currentRatio);
        break;
    case 2:
        parsePower(targetData, errFlag,ammeterData,info.voltageRatio,info.currentRatio);
        break;
    case 3:
        parseRePower(targetData, errFlag,ammeterData,info.voltageRatio,info.currentRatio);
        break;
    case 4:
        parsePowerFactor(targetData, errFlag,ammeterData);
        break;
    case 5:
        parseCur_0(targetData, ammeterData,info.currentRatio);
        break;
    case 6:
        parseActive_absorb_energy(targetData, errFlag,ammeterData,info.voltageRatio,info.currentRatio);
        break;
    case 7:
        parseActive_liberate_energy(targetData, errFlag,ammeterData,info.voltageRatio,info.currentRatio);
        break;
    case 8:
        parsereactive_sensibility_energy1(targetData,errFlag,info.voltageRatio,info.currentRatio);
        break;
    case 9:
        parsreactive_capacity_energy2(targetData,errFlag,info.voltageRatio,info.currentRatio);
        break;
    case 10:
        parsereactive_sensibility_energy3(targetData, errFlag,info.voltageRatio,info.currentRatio);
        break;
    case 11:
        parsereactive_capacity_energy4(targetData,errFlag,info.voltageRatio,info.currentRatio);
        break;
    default:
        break;
    }
}

void dlt645_07::parseVol(uint8_t *targetData, bool &errFlag,FRAME_SUB_STATION_INFO&ammeterData, int & PT_value)
{
   short lowByte,highByte;
   int tmpValue;

    lowByte = ((targetData[0]>>4) &0x0f)*10 +(targetData[0]&0x0f) ;
    highByte =  ((targetData[1]>>4) &0x0f)*10 +(targetData[1]&0x0f) ;
    if(!errFlag)
        tmpValue = (highByte*100+lowByte)*PT_value;
    else
        tmpValue = 0x7FFFFFFF;

    ammeterData.A_voltage = (float)tmpValue/10;
    dataMap_645_07.insert(Addr_Vol_A_Term,QByteArray((char*)&ammeterData.A_voltage,4));
    pLog_dlt64507->getLogPoint(_strLogName)->info("Vol A:    "+QString::number(tmpValue));
    //B相电压
    lowByte = ((targetData[2]>>4) &0x0f)*10 +(targetData[2]&0x0f) ;
    highByte =  ((targetData[3]>>4) &0x0f)*10 +(targetData[3]&0x0f) ;
    if(!errFlag)
        tmpValue = (highByte*100+lowByte)*PT_value;
    else
        tmpValue = 0x7FFFFFFF;

     ammeterData.B_voltage =  (float)tmpValue/10;
     dataMap_645_07.insert(Addr_Vol_B_Term,QByteArray((char*)&ammeterData.B_voltage,4));
     pLog_dlt64507->getLogPoint(_strLogName)->info("Vol B:    "+QString::number(tmpValue));
    //C相电压
    lowByte = ((targetData[4]>>4) &0x0f)*10 +(targetData[4]&0x0f) ;
    highByte =  ((targetData[5]>>4) &0x0f)*10 +(targetData[5]&0x0f) ;
    if(!errFlag)
        tmpValue = (highByte*100+lowByte)*PT_value;
    else
        tmpValue = 0x7FFFFFFF;

    ammeterData.C_voltage =  (float)tmpValue/10;
    dataMap_645_07.insert(Addr_Vol_C_Term,QByteArray((char*)&ammeterData.C_voltage,4));
     pLog_dlt64507->getLogPoint(_strLogName)->info("Vol C:    "+QString::number(tmpValue));
}

void dlt645_07::parseCur(uint8_t *targetData, bool &errFlag,FRAME_SUB_STATION_INFO&ammeterData, int &CT_value)
{
   short lowByte,highByte;
   int tmpValue;

   lowByte =   (targetData[1]&0x0f)*100+((targetData[0]>>4) &0x0f)*10 +(targetData[0]&0x0f) ;
   highByte =  ((targetData[2]>>4) &0x07)*100 +(targetData[2]&0x0f)*10+((targetData[1]>>4) &0x0f) ;
   if(!errFlag)
   {
       tmpValue =(highByte*1000+lowByte)*CT_value;

       if((targetData[2]>>4)&0x08)
           tmpValue=tmpValue*(-1);
   }
   else
       tmpValue = 0x7FFFFFFF;
    pLog_dlt64507->getLogPoint(_strLogName)->info("Cur A:    "+QString::number(tmpValue));
    ammeterData.A_current =  (float)tmpValue/1000;
    dataMap_645_07.insert(Addr_Cur_A_Term,QByteArray((char*)&ammeterData.A_current,4));
   //B相电流
   lowByte =   (targetData[4]&0x0f)*100+((targetData[3]>>4) &0x0f)*10 +(targetData[3]&0x0f) ;
   highByte =  ((targetData[5]>>4) &0x07)*100 +(targetData[5]&0x0f)*10+((targetData[4]>>4) &0x0f) ;

   if(!errFlag)
   {
       tmpValue =(highByte*1000+lowByte)*CT_value;

       if((targetData[5]>>4)&0x08)
           tmpValue=tmpValue*(-1);
   }
   else
       tmpValue = 0x7FFFFFFF;
   pLog_dlt64507->getLogPoint(_strLogName)->info("Cur B:    "+QString::number(tmpValue));
   ammeterData.B_current =  (float)tmpValue/1000;
   dataMap_645_07.insert(Addr_Cur_B_Term,QByteArray((char*)&ammeterData.B_current,4));  //nihai modify  20170531 修正B相电流显示错误
   //C相电流
   lowByte =   (targetData[7]&0x0f)*100+((targetData[6]>>4) &0x0f)*10 +(targetData[6]&0x0f) ;
   highByte =  ((targetData[8]>>4) &0x07)*100 +(targetData[8]&0x0f)*10+((targetData[7]>>4) &0x0f) ;
   if(!errFlag)
   {
       tmpValue =(highByte*1000+lowByte)*CT_value;

       if((targetData[8]>>4)&0x08)
           tmpValue=tmpValue*(-1);
   }
   else
       tmpValue = 0x7FFFFFFF;

   ammeterData.C_current =  (float)tmpValue/1000;
   dataMap_645_07.insert(Addr_Cur_C_Term,QByteArray((char*)&ammeterData.C_current,4));
   pLog_dlt64507->getLogPoint(_strLogName)->info("Cur C:    "+QString::number(tmpValue));

}


void dlt645_07::parsePower(uint8_t *targetData, bool &errFlag,FRAME_SUB_STATION_INFO&ammeterData,int &PT_value,int &CT_value)
{
    short lowByte,highByte;
    int tmpValue,activePowerA,activePowerB,activePowerC;

       lowByte = ((targetData[1]>>4) &0x0f)*1000 +(targetData[1]&0x0f)*100 + ((targetData[0]>>4) &0x0f)*10 +(targetData[0]&0x0f);
       highByte =  ((targetData[2]>>4) &0x07)*10 +(targetData[2]&0x0f) ;
       if(!errFlag)
       {
           tmpValue = (highByte*10000+lowByte)*CT_value*PT_value;

           if((targetData[2]>>4)&0x08)
               tmpValue=tmpValue*(-1);
       }
       else
           tmpValue = 0x7FFFFFFF;

       pLog_dlt64507->getLogPoint(_strLogName)->info("SUM Power:    "+QString::number(tmpValue));
       ammeterData.active_power =  (float)tmpValue/10000;
       dataMap_645_07.insert(Addr_Power_Term,QByteArray((char*)&ammeterData.active_power,4));

       lowByte = ((targetData[4]>>4) &0x0f)*1000 +(targetData[4]&0x0f)*100 + ((targetData[3]>>4) &0x0f)*10 +(targetData[3]&0x0f);
       highByte =  ((targetData[5]>>4) &0x07)*10 +(targetData[5]&0x0f) ;
       activePowerA = (highByte*10000+lowByte)*CT_value*PT_value;

       if((targetData[5]>>4)&0x08)
           activePowerA=activePowerA*(-1);

       lowByte = ((targetData[7]>>4) &0x0f)*1000 +(targetData[7]&0x0f)*100 + ((targetData[6]>>4) &0x0f)*10 +(targetData[6]&0x0f);
       highByte =  ((targetData[8]>>4) &0x07)*10 +(targetData[8]&0x0f) ;
       activePowerB = (highByte*10000+lowByte)*CT_value*PT_value;

       if((targetData[8]>>4)&0x08)
           activePowerB=activePowerB*(-1);

       lowByte = ((targetData[10]>>4) &0x0f)*1000 +(targetData[10]&0x0f)*100 + ((targetData[9]>>4) &0x0f)*10 +(targetData[9]&0x0f);
       highByte =  ((targetData[11]>>4) &0x07)*10 +(targetData[11]&0x0f) ;
       activePowerC = (highByte*10000+lowByte)*CT_value*PT_value;

       if((targetData[11]>>4)&0x08)
           activePowerC=activePowerC*(-1);

       if(errFlag)
           activePowerA =activePowerB =activePowerC = 0x7FFFFFFF;

       pLog_dlt64507->getLogPoint(_strLogName)->info("Power A:    "+QString::number(activePowerA));
       pLog_dlt64507->getLogPoint(_strLogName)->info("Power B:    "+QString::number(activePowerB));
       pLog_dlt64507->getLogPoint(_strLogName)->info("Power C:    "+QString::number(activePowerC));
       ammeterData.active_powerA =  (float)activePowerA/1000;
       ammeterData.active_powerB =  (float)activePowerB/1000;
       ammeterData.active_powerC =  (float)activePowerC/1000;
       dataMap_645_07.insert(Addr_Power_A_Term,QByteArray((char*)&ammeterData.active_powerA,4));
       dataMap_645_07.insert(Addr_Power_B_Term,QByteArray((char*)&ammeterData.active_powerB,4));
       dataMap_645_07.insert(Addr_Power_C_Term,QByteArray((char*)&ammeterData.active_powerC,4));
}

void dlt645_07::parseRePower(uint8_t *targetData, bool &errFlag,FRAME_SUB_STATION_INFO&ammeterData,int &PT_value,int &CT_value)
{
    short lowByte,highByte;
    int tmpValue,reactivePowerA,reactivePowerB,reactivePowerC;

    lowByte = ((targetData[1]>>4) &0x0f)*1000 +(targetData[1]&0x0f)*100 + ((targetData[0]>>4) &0x0f)*10 +(targetData[0]&0x0f);
    highByte =  ((targetData[2]>>4) &0x07)*10 +(targetData[2]&0x0f) ;
    if(!errFlag)
    {
        tmpValue = (highByte*10000+lowByte)*CT_value*PT_value;

        if((targetData[2]>>4)&0x08)
            tmpValue=tmpValue*(-1);
    }
    else
        tmpValue = 0x7FFFFFFF;

     ammeterData.reactive_power =  (float)tmpValue/1000;
     dataMap_645_07.insert(Addr_rePower_Term,QByteArray((char*)&ammeterData.reactive_power,4));
     pLog_dlt64507->getLogPoint(_strLogName)->info("SUM re-Power :    "+QString::number(tmpValue));
    lowByte = ((targetData[4]>>4) &0x0f)*1000 +(targetData[4]&0x0f)*100 + ((targetData[3]>>4) &0x0f)*10 +(targetData[3]&0x0f);
    highByte =  ((targetData[5]>>4) &0x07)*10 +(targetData[5]&0x0f) ;
    reactivePowerA = (highByte*10000+lowByte)*CT_value*PT_value;

    if((targetData[5]>>4)&0x08)
        reactivePowerA=reactivePowerA*(-1);

    lowByte = ((targetData[7]>>4) &0x0f)*1000 +(targetData[7]&0x0f)*100 + ((targetData[6]>>4) &0x0f)*10 +(targetData[6]&0x0f);
    highByte =  ((targetData[8]>>4) &0x07)*10 +(targetData[8]&0x0f) ;
    reactivePowerB = (highByte*10000+lowByte)*CT_value*PT_value;

    if((targetData[8]>>4)&0x08)
        reactivePowerB=reactivePowerB*(-1);

    lowByte = ((targetData[10]>>4) &0x0f)*1000 +(targetData[10]&0x0f)*100 + ((targetData[9]>>4) &0x0f)*10 +(targetData[9]&0x0f);
    highByte =  ((targetData[11]>>4) &0x07)*10 +(targetData[11]&0x0f) ;
    reactivePowerC = (highByte*10000+lowByte)*CT_value*PT_value;

    if((targetData[11]>>4)&0x08)
        reactivePowerC=reactivePowerC*(-1);

    if(errFlag)
        reactivePowerA = reactivePowerB = reactivePowerC = 0x7FFFFFFF;

    pLog_dlt64507->getLogPoint(_strLogName)->info("re-Power A:    "+QString::number(reactivePowerA));
    pLog_dlt64507->getLogPoint(_strLogName)->info("re-Power B:    "+QString::number(reactivePowerB));
    pLog_dlt64507->getLogPoint(_strLogName)->info("re-Power C:    "+QString::number(reactivePowerC));
    ammeterData.reactive_powerA =  (float)reactivePowerA/1000;
    ammeterData.reactive_powerB =  (float)reactivePowerB/1000;
    ammeterData.reactive_powerC =  (float)reactivePowerC/1000;
    dataMap_645_07.insert(Addr_rePower_A_Term,QByteArray((char*)&ammeterData.reactive_powerA,4));
    dataMap_645_07.insert(Addr_rePower_B_Term,QByteArray((char*)&ammeterData.reactive_powerB,4));
    dataMap_645_07.insert(Addr_rePower_C_Term,QByteArray((char*)&ammeterData.reactive_powerC,4));
}

void dlt645_07::parsePowerFactor(uint8_t *targetData, bool &errFlag,FRAME_SUB_STATION_INFO&ammeterData)
{
    short lowByte,highByte;
    int tmpValue,PowerFactorA,PowerFactorB,PowerFactorC;

    lowByte =  (targetData[1]&0x0f)*100+ ((targetData[0]>>4) &0x0f)*10 +(targetData[0]&0x0f);
    highByte =  (targetData[1]>>4) &0x07;
    if(!errFlag)
    {
        tmpValue=highByte*1000+lowByte;

        if((targetData[1]>>4)&0x08)
            tmpValue=tmpValue*(-1);
    }
    else
        tmpValue = 0x7FFFFFFF;

    ammeterData.power_factor =  (float)tmpValue/1000;
    dataMap_645_07.insert(Addr_PowerFactor_Term,QByteArray((char*)&ammeterData.power_factor,4));
    pLog_dlt64507->getLogPoint(_strLogName)->info("SUM Power factor :    "+QString::number(tmpValue));
    lowByte =  (targetData[3]&0x0f)*100+ ((targetData[2]>>4) &0x0f)*10 +(targetData[2]&0x0f);
    highByte =  (targetData[3]>>4) &0x07;
    PowerFactorA=highByte*1000+lowByte;

    if((targetData[3]>>4)&0x08)
        PowerFactorA=PowerFactorA*(-1);

    lowByte =  (targetData[5]&0x0f)*100+ ((targetData[4]>>4) &0x0f)*10 +(targetData[4]&0x0f);
    highByte =  (targetData[5]>>4) &0x07;
    PowerFactorB=highByte*1000+lowByte;

    if((targetData[5]>>4)&0x08)
        PowerFactorB=PowerFactorB*(-1);

    lowByte =  (targetData[7]&0x0f)*100+ ((targetData[6]>>4) &0x0f)*10 +(targetData[6]&0x0f);
    highByte =  (targetData[7]>>4) &0x07;
    PowerFactorC=highByte*1000+lowByte;

    if((targetData[7]>>4)&0x08)
        PowerFactorC=PowerFactorC*(-1);

    if(errFlag)
    {
        PowerFactorA=PowerFactorB=PowerFactorC=0x7FFFFFFF;
    }
    pLog_dlt64507->getLogPoint(_strLogName)->info("Power factor A:    "+QString::number(PowerFactorA));
    pLog_dlt64507->getLogPoint(_strLogName)->info("Power factor  B:    "+QString::number(PowerFactorB));
    pLog_dlt64507->getLogPoint(_strLogName)->info("Power factor  C:    "+QString::number(PowerFactorC));
    ammeterData.power_factorA =  (float)PowerFactorA/1000;
    ammeterData.power_factorB =  (float)PowerFactorB/1000;
    ammeterData.power_factorC =  (float)PowerFactorC/1000;
    dataMap_645_07.insert(Addr_PowerFactor_A_Term,QByteArray((char*)&ammeterData.power_factorA,4));
    dataMap_645_07.insert(Addr_PowerFactor_B_Term,QByteArray((char*)&ammeterData.power_factorB,4));
    dataMap_645_07.insert(Addr_PowerFactor_C_Term,QByteArray((char*)&ammeterData.power_factorC,4));
}

void dlt645_07::parseCur_0(uint8_t *targetData,FRAME_SUB_STATION_INFO&ammeterData,int &CT_value)
{
    short lowByte,highByte;
    int tmpValue;

    lowByte =   (targetData[1]&0x0f)*100+((targetData[0]>>4) &0x0f)*10 +(targetData[0]&0x0f) ;
    highByte =  ((targetData[2]>>4) &0x07)*100 +(targetData[2]&0x0f)*10+((targetData[1]>>4) &0x0f) ;
    tmpValue= (highByte*1000+lowByte)*CT_value/100;

    if((targetData[2]>>4)&0x08)
        tmpValue=tmpValue*(-1);

    if(recData[0]==0xfe && recData[1]==0xfe
            && recData[2]==0xfe && recData[3]==0xfe)
    {
        if(recData[12]==0xd1)//异常应答
            tmpValue=0;
    }
    else
    {
        if(recData[8]==0xd1)//异常应答
            tmpValue=0;
    }

    ammeterData.neutralLine_current =  (float)tmpValue/1000;
    dataMap_645_07.insert(Addr_Cur_0_Term,QByteArray((char*)&ammeterData.neutralLine_current,4));
    pLog_dlt64507->getLogPoint(_strLogName)->info("Cur 0:    "+QString::number(tmpValue));
}

void dlt645_07::parseActive_absorb_energy(uint8_t *targetData,bool & errFlag,FRAME_SUB_STATION_INFO&ammeterData,int &PT_value,int &CT_value)
{
    int lowByte,highByte;
    int elePower;

    lowByte =   ((targetData[1]>>4) &0x0f)*1000+(targetData[1]&0x0f)*100+((targetData[0]>>4) &0x0f)*10 +(targetData[0]&0x0f) ;
    highByte =  ((targetData[3]>>4) &0x0f)*10000000 +(targetData[3]&0x0f)*1000000+((targetData[2]>>4) &0x0f)*100000+ (targetData[2]&0x0f)*10000;
    if(!errFlag)
        elePower = (highByte+lowByte)*CT_value*PT_value;
    else
        elePower = 0x7FFFFFFF;

    pLog_dlt64507->getLogPoint(_strLogName)->info("SUM Energy :    "+QString::number(elePower));
    ammeterData.active_absorb_energy =  elePower;
    dataMap_645_07.insert(Addr_active_absorb_energy_Term,QByteArray((char*)&ammeterData.active_absorb_energy,4));
}

void dlt645_07::parseActive_liberate_energy(uint8_t *targetData,bool & errFlag,FRAME_SUB_STATION_INFO&ammeterData,int &PT_value,int &CT_value)
{
    int lowByte,highByte;
    int elePower;

    lowByte =   ((targetData[1]>>4) &0x0f)*1000+(targetData[1]&0x0f)*100+((targetData[0]>>4) &0x0f)*10 +(targetData[0]&0x0f) ;
    highByte =  ((targetData[3]>>4) &0x0f)*10000000 +(targetData[3]&0x0f)*1000000+((targetData[2]>>4) &0x0f)*100000+ (targetData[2]&0x0f)*10000;
    if(!errFlag)
        elePower = (highByte+lowByte)*CT_value*PT_value;
    else
        elePower = 0x7FFFFFFF;

    pLog_dlt64507->getLogPoint(_strLogName)->info("re SUM Energy :    "+QString::number(elePower));
    ammeterData.active_liberate_energy = elePower;
    dataMap_645_07.insert(Addr_active_liberate_energy_Term,QByteArray((char*)&ammeterData.active_liberate_energy,4));
}

void dlt645_07::parsereactive_sensibility_energy1(uint8_t *targetData, bool & errFlag,int &PT_value,int &CT_value)
{
    int lowByte,highByte;
    int elePower;

    lowByte =   ((targetData[1]>>4) &0x0f)*1000+(targetData[1]&0x0f)*100+((targetData[0]>>4) &0x0f)*10 +(targetData[0]&0x0f) ;
    highByte =  ((targetData[3]>>4) &0x0f)*10000000 +(targetData[3]&0x0f)*1000000+((targetData[2]>>4) &0x0f)*100000+ (targetData[2]&0x0f)*10000;

    if(!errFlag)
       elePower = (highByte+lowByte)*CT_value*PT_value;
    else
        elePower = 0;

    if(elePower == 0)
    {
        ammeterData->reactive_sensibility_energy = 0;
    }
    else
    {
        ammeterData->reactive_sensibility_energy = elePower;
    }

}


void dlt645_07::parsereactive_sensibility_energy3(uint8_t *targetData, bool & errFlag,int &PT_value,int &CT_value)
{
    int lowByte,highByte;
    int elePower;

    lowByte =   ((targetData[1]>>4) &0x0f)*1000+(targetData[1]&0x0f)*100+((targetData[0]>>4) &0x0f)*10 +(targetData[0]&0x0f) ;
    highByte =  ((targetData[3]>>4) &0x0f)*10000000 +(targetData[3]&0x0f)*1000000+((targetData[2]>>4) &0x0f)*100000+ (targetData[2]&0x0f)*10000;
    if(!errFlag)
        elePower = (highByte+lowByte)*CT_value*PT_value;
    else
        elePower = 0x7FFFFFFF;

    ammeterData->reactive_sensibility_energy += elePower;
    dataMap_645_07.insert(Addr_reactive_sensibility_energy_Term,QByteArray((char*)&ammeterData->reactive_sensibility_energy,4));
    pLog_dlt64507->getLogPoint(_strLogName)->info("gan SUM Energy :    "+QString::number((int)ammeterData->reactive_sensibility_energy));
}


void dlt645_07::parsreactive_capacity_energy2(uint8_t *targetData, bool & errFlag,int &PT_value,int &CT_value)
{
    int lowByte,highByte;
    int elePower;

    lowByte =   ((targetData[1]>>4) &0x0f)*1000+(targetData[1]&0x0f)*100+((targetData[0]>>4) &0x0f)*10 +(targetData[0]&0x0f) ;
    highByte =  ((targetData[3]>>4) &0x0f)*10000000 +(targetData[3]&0x0f)*1000000+((targetData[2]>>4) &0x0f)*100000+ (targetData[2]&0x0f)*10000;

    if(!errFlag)
         elePower = (highByte+lowByte)*CT_value*PT_value;
    else
        elePower = 0;

    if(elePower == 0)
        ammeterData->reactive_capacity_energy = 0;
    else
        ammeterData->reactive_capacity_energy = elePower;
}


void dlt645_07::parsereactive_capacity_energy4(uint8_t *targetData, bool & errFlag,int &PT_value,int &CT_value)
{
    int lowByte,highByte;
    int elePower;

    lowByte =   ((targetData[1]>>4) &0x0f)*1000+(targetData[1]&0x0f)*100+((targetData[0]>>4) &0x0f)*10 +(targetData[0]&0x0f) ;
    highByte =  ((targetData[3]>>4) &0x0f)*10000000 +(targetData[3]&0x0f)*1000000+((targetData[2]>>4) &0x0f)*100000+ (targetData[2]&0x0f)*10000;
    if(!errFlag)
        elePower =(highByte+lowByte)*CT_value*PT_value;
    else
        elePower = 0x7FFFFFFF;


    ammeterData->reactive_capacity_energy += elePower;
    dataMap_645_07.insert(Addr_reactive_capacity_energy_Term,QByteArray((char*)&ammeterData->reactive_capacity_energy,4));
    pLog_dlt64507->getLogPoint(_strLogName)->info("rong SUM Energy :    "+QString::number((int)ammeterData->reactive_sensibility_energy));
}


bool dlt645_07::checkRecData(int getBytes, uint8_t * recData,uint8_t *targetData,bool & errFlag, unsigned char *addr)
{
    if(getBytes != -1)
    {
        int index,startIndex;
         errFlag = false;

        for( index=0;index<getBytes-7;index++)
        {
            if(recData[index] == 0x68 && recData[index+7] == 0x68)
            {
                if(memcmp(&addr[0],&recData[index+1],6))//目标地址与应答地址不一致,不上传数据
                {
                    pLog_dlt64507->getLogPoint(_strLogName)->info("Data not form target ammeter !!!! ");
                    pLog_dlt64507->getLogPoint(_strLogName)->info("get data ==: "+ConvertHex2Qstr((unsigned char *)recData, getBytes));
                    errFlag = true;
                    return true;
                }

                if((recData[index+8] == 0x91)||(recData[index+8] == 0xB1))   //从站正常应答
                {
                    startIndex=recData[index+9]-4;
                    //pLog_dlt64507->getLogPoint(_strLogName)->info("############################################ ");
                    for (int i=0;i<startIndex;i++)//获取数据域(不包括数据标识部分)
                    {
                        targetData[i]=recData[index+14+i]- OFFSET;
                    }
                    pLog_dlt64507->getLogPoint(_strLogName)->info("get data ==: "+ConvertHex2Qstr((unsigned char *)recData, getBytes));
                    pLog_dlt64507->getLogPoint(_strLogName)->info("Get value ==: "+ConvertHex2Qstr((unsigned char *)targetData, startIndex));
                }
                else if(recData[index+8] == 0xD1) //异常应答
                {
                    errFlag = true;
                    pLog_dlt64507->getLogPoint(_strLogName)->info("Get fault response !!!!!!!!!!!!!!!!!!!!!! ");
                    pLog_dlt64507->getLogPoint(_strLogName)->info("get data ==: "+ConvertHex2Qstr((unsigned char *)recData, getBytes));
                }
                else //非法数据
                {
                    errFlag = true;
                    pLog_dlt64507->getLogPoint(_strLogName)->info("Get illegal data !!!!!!!!!!!!!!!!!!!!!! ");
                    pLog_dlt64507->getLogPoint(_strLogName)->info("get data ==: "+ConvertHex2Qstr((unsigned char *)recData, getBytes));
                }
                break;       //找到数据，退出循环
            }
        }
        if(index == getBytes-7)//未读到合法数据
        {
            errFlag = true;
            pLog_dlt64507->getLogPoint(_strLogName)->info("Get wrong format data !!!!!!!!!!!!!!!!!!!!! ");
            pLog_dlt64507->getLogPoint(_strLogName)->info(ConvertHex2Qstr((unsigned char *)recData, getBytes));
            return true;
        }

            return true;
    }
    else//未读到数据
    {
        errFlag = true;
        pLog_dlt64507->getLogPoint(_strLogName)->info("NO get data !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ");
        return true;
    }
}

 void dlt645_07::fixReadCMD(FRAME_DLT_645  &frame_dlt_645,unsigned char * addr,uint8_t * Dlt645_buff,uint8_t * cmdFrame)
 {

     frame_dlt_645.frame_start = FRAME_START;//帧起始符
     memcpy(frame_dlt_645.addr,addr,6);

     frame_dlt_645.frame_start2 = FRAME_START;//帧起始符
     frame_dlt_645.controlCode =0x11;//控制码(读数据)
     frame_dlt_645.data_length = 0x04;//数据域长度(仅含数据标识，长度固定为4字节)

     memcpy(Dlt645_buff,(unsigned char *)&frame_dlt_645,10);
     memcpy(&Dlt645_buff[10],cmdFrame,frame_dlt_645.data_length);//数据域(仅含数据标识)
     Dlt645_buff[frame_dlt_645.data_length+10] = verify(Dlt645_buff,frame_dlt_645.data_length+10);//校验码
     Dlt645_buff[frame_dlt_645.data_length+11] = FRAME_END;//结束符

     pLog_dlt64507->getLogPoint(_strLogName)->info("Send cmd ==: "+ConvertHex2Qstr((unsigned char *)Dlt645_buff, 16));
//     for(int i=0;i<16;i++)
 }

 void dlt645_07::ReadInlineAmmeter(stAmmeterConfig &info)
 {
     FRAME_DLT_645 frame_dlt_645;
          FRAME_SUB_STATION_INFO ammeterData;
          uint8_t Dlt645_buff[200];
          uint8_t recData[200];
          uint8_t targetData[200];
          int getBytes,sendBytes;
          int readCount;
          bool errFlag;

          for(int index = 0; index<12;index++)
          {
     //         if(info.funType == 3 && stopReadFlag)//远程抄表功能召唤时，停止进线侧抄读
     //             break;

              getBytes = 0;
              sendBytes = 0;
              readCount = 0;
              errFlag = false;

              memset(Dlt645_buff,0x0,sizeof(Dlt645_buff));
              memset(recData,0x0,sizeof(recData));
              memset(targetData,0x0,sizeof(targetData));

              fixReadCMD(frame_dlt_645,info.addr,Dlt645_buff,DLT645_07_item[index]);

              tcflush(g_ammeterHandle,TCIFLUSH);  //清空一次接收缓存
              sendBytes=write(g_ammeterHandle,Dlt645_buff,16);
              pLog_dlt64507->getLogPoint(_strLogName)->info("write count ===     "+QString::number(sendBytes));
              while(1)
              {
                  readCount++;
                  if(readCount > 3)
                      break;
                  usleep(500000);
                  memset(recData,0x0,sizeof(recData));
                  getBytes = read(g_ammeterHandle,recData, 200);
                  if(getBytes != 0 && getBytes != -1)
                  {
                      pLog_dlt64507->getLogPoint(_strLogName)->info("................get data ==: "+ConvertHex2Qstr((unsigned char *)recData, getBytes));
                      if(recData[10 + recData[9]] == verify(recData,10 + recData[9]))   //校验值相等
                      {
                          break;
                      }
                      else
                      {
                          pLog_dlt64507->getLogPoint(_strLogName)->info("22Send cmd ==: "+ConvertHex2Qstr((unsigned char *)Dlt645_buff, 16));
                          sendBytes=write(g_ammeterHandle,Dlt645_buff,16);
                          pLog_dlt64507->getLogPoint(_strLogName)->info("22write count ===     "+QString::number(sendBytes));
                      }
                  }
                  else
                  {
                      pLog_dlt64507->getLogPoint(_strLogName)->info("22Send cmd ==: "+ConvertHex2Qstr((unsigned char *)Dlt645_buff, 16));
                      sendBytes=write(g_ammeterHandle,Dlt645_buff,16);
                      pLog_dlt64507->getLogPoint(_strLogName)->info("22write count ===     "+QString::number(sendBytes));
                  }
              }
              pLog_dlt64507->getLogPoint(_strLogName)->info("get count    ===     "+QString::number(getBytes));
              pLog_dlt64507->getLogPoint(_strLogName)->info("read count    ===     "+QString::number(readCount));
              if(checkRecData(getBytes,recData,targetData,errFlag,info.addr))
              {
                  decodeRecData(targetData,index, errFlag,ammeterData,info);
              }
              //usleep(1000000);
          }
     dataMap_645_07.insert(Addr_Ammeter_Type,QByteArray('2',1));//07协议电表
     dataMap_645_07.insert(Addr_Ammeter_ID,QByteArray((const char *) info.addr,6));

     emit sendAmmeterData_645_07(dataMap_645_07);
     dataMap_645_07.clear();
 }

 void dlt645_07::ReadPowerMonitorAmmeter(stAmmeterConfig &info)
 {
     FRAME_DLT_645 frame_dlt_645;
     FRAME_SUB_STATION_INFO ammeterData;
     uint8_t Dlt645_buff[200];
     uint8_t recData[200];
     uint8_t targetData[200];
     int getBytes,sendBytes;
     bool errFlag;

     getBytes = 0;
     sendBytes = 0;
     errFlag = false;

     memset(Dlt645_buff,0x0,sizeof(Dlt645_buff));
     memset(recData,0x0,sizeof(recData));
     memset(targetData,0x0,sizeof(targetData));

     fixReadCMD(frame_dlt_645,info.addr,Dlt645_buff,DLT645_07_item[2]);

     sendBytes=write(g_ammeterHandle,Dlt645_buff,16);
     pLog_dlt64507->getLogPoint(_strLogName)->info("write count ===     "+QString::number(sendBytes));
     usleep(500000);
     getBytes = read(g_ammeterHandle,recData, 200);
     pLog_dlt64507->getLogPoint(_strLogName)->info("get count    ===     "+QString::number(getBytes));
     if(checkRecData(getBytes,recData,targetData,errFlag,info.addr))
     {
         decodeRecData(targetData,2, errFlag,ammeterData,info);
         emit sig_readSucess_645_07();

         dataMap_645_07.insert(Addr_Ammeter_Type,QByteArray('2',1));//07协议电表
         dataMap_645_07.insert(Addr_Ammeter_ID,QByteArray((const char *) info.addr,6));
         emit sendAmmeterData_645_07(dataMap_645_07);
         dataMap_645_07.clear();
     }
     else
     {
         emit sig_readFail_645_07();
     }
 }
 void dlt645_07::readAmmeterData_07(stAmmeterConfig info)
 {
     uint8_t Dlt645_buff[200];
     memset(Dlt645_buff,0x0,sizeof(Dlt645_buff));

     pLog_dlt64507->getLogPoint(_strLogName)->info("#####################################################");
     pLog_dlt64507->getLogPoint(_strLogName)->info("Start to read ammeter ==: "+ConvertHex2Qstr((unsigned char *)info.addr, 6));

     for(int i=0;i<4;i++)
         Dlt645_buff[i] =0xFE;

     write(g_ammeterHandle,Dlt645_buff,4);//发送四个前导字节，唤醒接收方
     usleep(500000);
     ReadInlineAmmeter(info);
 }

//读取进线侧电表数据
 void dlt645_07::slot_readAmmeter_dlt645_07(QList<stAmmeterConfig> infoList)
 {

     if(!Init())//串口初始化
     {
         return;
     }

     for(int i=0;i<infoList.length();i++)//逐块电表读取
     {
         readAmmeterData_07(infoList.at(i));
     }

     emit sigReadOver_645_07(2);//07协议电表

 }

 ///
 /// \brief dlt645_07::remoteReadAmmeterData_07
 /// \param ammeterId
 /// \param readDataType
 /// \param readingTime
 /// \param info
 ///远程读07电表数据
bool dlt645_07::remoteReadAmmeterData_07(unsigned char *ammeterId,int readDataType,unsigned char *readingTime,stAmmeterConfig info)   //远程读07电表数据
{
    pLog_dlt64507->getLogPoint(_strLogName)->info("远程抄表###############");
    pLog_dlt64507->getLogPoint(_strLogName)->info("电表地址："+ConvertDec2Qstr(ammeterId,6)+"  数据类型："+QString::number(readDataType)+"  抄读时间："+ConvertDec2Qstr(readingTime, 6));
//    QByteArray qAmmerterDataByteArray;
    switch (readDataType){
    case 1:  //当前电能及最大需量数据
        remoteReadAmmeterData_07_CurrentEnergy(ammeterId,readingTime,info);
        break;
    case 2:  //整点冻结电能
        remoteReadAmmeterData_07_HourFreezeEnergy(ammeterId,readingTime,info);
        break;
    case 3:  //日冻结电能及最大需量数据(最大可查上62次)
        remoteReadAmmeterData_07_DayFreezeEnergy(ammeterId,readingTime,info);
        break;
    case 4:  //结算日电能及最大需量数据  (结算日默认是每月1日0点  最大可查上12次)
        remoteReadAmmeterData_07_SettlementEnergy(ammeterId,readingTime,info);
        break;
    default:
        break;
    }
//    qAmmerterDataByteArray = QByteArray::fromRawData((char *)ammeterId, 6);
//    dataMap_645_07.insert(Addr_RemoteAmeterAddr_Adj,qAmmerterDataByteArray);
//    dataMap_645_07.insert(Addr_RemoteAmeterAddr_Adj,QByteArray((const char *)&ammeterId,6));
    ConvertDataFormat(ammeterId,6);      //大小端转换
    dataMap_645_07.insert(Addr_RemoteAmeterAddr_Adj,QByteArray((char *)ammeterId,6));
    dataMap_645_07.insert(Addr_RemoteAmeterType_Adj,QByteArray((char *)&readDataType,1));
    dataMap_645_07.insert(Addr_RemoteAmeterReadTime_Adj,QByteArray((char *)readingTime,6));

    emit sig_sendToBusRemoteAmmeterData_645_07(dataMap_645_07);  //发送到总线
    dataMap_645_07.clear();

    return true;
}

///
/// \brief dlt645_07::remoteReadAmmeterData_07_CurrentEnergy
/// \param ammeterId
/// \param readingTime
/// \param info
///远程读07电表数据-当前电能
void dlt645_07::remoteReadAmmeterData_07_CurrentEnergy(unsigned char *ammeterId,unsigned char *readingTime, stAmmeterConfig &info)
{
    int getBytes;
    int index = 0;
    int readCount;

    FRAME_DLT_645 frame_dlt_645;
    bool errFlag = false;
    uint8_t Dlt645_buff[200];    //写电表指令数组
    uint8_t recData[200];          //读到的电表数据
    uint8_t targetData[200];     //
    memset(Dlt645_buff,0x0,sizeof(Dlt645_buff));
    memset(recData,0x0,sizeof(recData));
    memset(targetData,0x0,sizeof(targetData));

    pLog_dlt64507->getLogPoint(_strLogName)->info("##############remoteReadAmmeterData_07-Current##################");
    pLog_dlt64507->getLogPoint(_strLogName)->info("Start to read ammeter ==: "+ConvertHex2Qstr((unsigned char *)ammeterId, 6));
    pLog_dlt64507->getLogPoint(_strLogName)->info("Start to read ammeter Time==: "+ConvertHex2Qstr((unsigned char *)readingTime, 6));

    for(index = 0;index < 12;index ++)
    {
        readCount = 0;
        fixReadCMD(frame_dlt_645,ammeterId,Dlt645_buff,DLT645_07_readType01[index]);   //组合读电表命令

        tcflush(g_ammeterHandle,TCIFLUSH);  //清空一次接收返回
        int wr = write(g_ammeterHandle,Dlt645_buff,16);
        pLog_dlt64507->getLogPoint(_strLogName)->info("01H-CURRENT write count---NUM:" + QString::number(index) + "===    " + QString::number(wr));
//        usleep(500000);
//        getBytes = read(g_ammeterHandle,recData, 212);
        while(1)
        {
            readCount++;
            if(readCount > 3)
                break;
            usleep(500000);
            memset(recData,0x0,sizeof(recData));
            getBytes = read(g_ammeterHandle,recData, 212);
            if(getBytes != 0 && getBytes != -1)
            {
                if(recData[10 + recData[9]] == verify(recData,10 + recData[9]))   //校验值相等
                {
                    break;
                }
                else
                {
                    pLog_dlt64507->getLogPoint(_strLogName)->info("Second Send cmd ==: "+ConvertHex2Qstr((unsigned char *)Dlt645_buff, 16));
                    tcflush(g_ammeterHandle,TCIFLUSH);  //清空一次接收返回
                    write(g_ammeterHandle,Dlt645_buff,16);
                }
            }
            else
            {
                pLog_dlt64507->getLogPoint(_strLogName)->info("Second Send cmd ==: "+ConvertHex2Qstr((unsigned char *)Dlt645_buff, 16));
                write(g_ammeterHandle,Dlt645_buff,16);
            }
        }
        pLog_dlt64507->getLogPoint(_strLogName)->info("01H-CURRENT get count---NUM:" + QString::number(index) + "===    " + QString::number(getBytes));
        if(checkRecData(getBytes,recData,targetData,errFlag,ammeterId))   //校验接收到的数据
        {
            decodeRemoteRecDataCurrent(targetData,index, errFlag,info);  //解析远程抄表的接收数据
        }
        usleep(1000000);
    }
}

///
/// \brief dlt645_07::remoteReadAmmeterData_07_HourFreezeEnergy
/// \param ammeterId
/// \param readDataType
/// \param readingTime
/// \param info
///远程读07电表数据-整点冻结电能
void dlt645_07::remoteReadAmmeterData_07_HourFreezeEnergy(unsigned char *ammeterId,unsigned char *readingTime, stAmmeterConfig &info)
{
    int getBytes;
    int readCount;
    QDateTime dt = QDateTime::currentDateTime();
    short hh = dt.toString("hh").toInt();
    short year=dt.toString("yyyy").toInt();
    short month = dt.toString("MM").toInt();
    short day = dt.toString("dd").toInt();

    int select_year = ((readingTime[0]>>4) &0x0f)*1000 +(readingTime[0]&0x0f)*100 + ((readingTime[1]>>4) &0x0f)*10 +(readingTime[1]&0x0f);
    int select_month = ((readingTime[2]>>4) &0x0f)*10 +(readingTime[2]&0x0f);
    int select_day = ((readingTime[3]>>4) &0x0f)*10 +(readingTime[3]&0x0f);
    int select_hh = ((readingTime[4]>>4) &0x0f)*10 +(readingTime[4]&0x0f);
    int select_mm =((readingTime[5]>>4) &0x0f)*10 +(readingTime[5]&0x0f);
    QString select_time = QString::number(select_year) + "-" + QString::number(select_month) + "-" + QString::number(select_day) + " " +QString::number(select_hh) + ":" + QString::number(select_mm);

    int nDayNum;
    int nHour;//定义整点查询次数
    QDate d1(select_year, select_month, select_day);  //查询的日期
    QDate d2(year, month, day);  // 当前日期
    nDayNum = d1.daysTo(d2);
    QTime time1(select_hh,0,0);
    QTime time2(hh,0,0);
    nHour = 24*nDayNum + time1.secsTo(time2)/3600 +1;

    FRAME_DLT_645 frame_dlt_645;
    bool errFlag = false;
    uint8_t Dlt645_buff[200];    //写电表指令数组
    uint8_t recData[200];          //读到的电表数据
    uint8_t targetData[200];     //
    uint8_t DLT645_07_readType02[4] = {0x00,0x32,0x37,0x38};  //整点冻结数据块（共13个字节） 01-冻结上一次，共可查询上254次（FE）
    memset(Dlt645_buff,0x0,sizeof(Dlt645_buff));
    memset(recData,0x0,sizeof(recData));
    memset(targetData,0x0,sizeof(targetData));

    pLog_dlt64507->getLogPoint(_strLogName)->info("##############remoteReadAmmeterData_07-HourFreeze##################");
    pLog_dlt64507->getLogPoint(_strLogName)->info("Start to read ammeter ==: "+ConvertHex2Qstr((unsigned char *)ammeterId, 6));

    for(int i=0;i<4;i++)
        Dlt645_buff[i] =0xFE;

    write(g_ammeterHandle,Dlt645_buff,4);//发送四个前导字节，唤醒接收方
    usleep(500000);

    if((nHour <= 254)&&(nHour > 0))
    {
        pLog_dlt64507->getLogPoint(_strLogName)->info("hour_freee_energy_select time:" + select_time);
        pLog_dlt64507->getLogPoint(_strLogName)->info("hour_freee_energy_select the number of:"  + QString::number(nHour));
            DLT645_07_readType02[0] = (char)nHour + 0x33;
//                    DLT645_07_readType02[0] = 0x01 + 0x33;
//            for(int i = 0; i < 4;i ++)
//            {
//            }

        getBytes = 0;
        readCount =0;
        fixReadCMD(frame_dlt_645,ammeterId,Dlt645_buff,DLT645_07_readType02);   //组合读电表命令
        tcflush(g_ammeterHandle,TCIFLUSH);  //清空一次接收返回
        int wr = write(g_ammeterHandle,Dlt645_buff,16);
        pLog_dlt64507->getLogPoint(_strLogName)->info("02H-HOUR_FREEZE_ENERGY_DATA write count ===     "+QString::number(wr));
//        usleep(500000);
//        getBytes = read(g_ammeterHandle,recData, 212);
        while(1)
        {
            readCount++;
            if(readCount > 3)
                break;
            usleep(500000);
            memset(recData,0x0,sizeof(recData));
            getBytes = read(g_ammeterHandle,recData, 212);
            if(getBytes != 0 && getBytes != -1)
            {
                if(recData[10 + recData[9]] == verify(recData,10 + recData[9]))   //校验值相等
                {
                    break;
                }
                else
                {
                    pLog_dlt64507->getLogPoint(_strLogName)->info("Second Send cmd ==: "+ConvertHex2Qstr((unsigned char *)Dlt645_buff, 16));
                    tcflush(g_ammeterHandle,TCIFLUSH);  //清空一次接收返回
                    write(g_ammeterHandle,Dlt645_buff,16);
                }
            }
            else
            {
                pLog_dlt64507->getLogPoint(_strLogName)->info("Second Send cmd ==: "+ConvertHex2Qstr((unsigned char *)Dlt645_buff, 16));
                write(g_ammeterHandle,Dlt645_buff,16);
            }
        }
        pLog_dlt64507->getLogPoint(_strLogName)->info("02H-HOUR_FREEZE_ENERGY_DATA get count    ===     "+QString::number(getBytes));
        if(checkRecData(getBytes,recData,targetData,errFlag,ammeterId))   //校验接收到的数据
        {
            decodeRemoteRecDataHourFreeze(targetData,errFlag,info);  //解析远程抄表的接收数据
        }
        usleep(1000000);
    }
    else
    {
        pLog_dlt64507->getLogPoint(_strLogName)->info("select fail:More than 254 hourly!!!");
    }

}

///
/// \brief dlt645_07::remoteReadAmmeterData_07_DayFreezeEnergy
/// \param ammeterId
/// \param readingTime
/// \param info
///远程读07电表数据-日冻结电能
void dlt645_07::remoteReadAmmeterData_07_DayFreezeEnergy(unsigned char *ammeterId,unsigned char *readingTime, stAmmeterConfig &info)
{
    int getBytes;
    int index = 0;
    int readCount;
    QDateTime dt = QDateTime::currentDateTime();
    short year=dt.toString("yyyy").toInt();
    short month = dt.toString("MM").toInt();
    short day = dt.toString("dd").toInt();

    int select_year = ((readingTime[0]>>4) &0x0f)*1000 +(readingTime[0]&0x0f)*100 + ((readingTime[1]>>4) &0x0f)*10 +(readingTime[1]&0x0f);
    int select_month = ((readingTime[2]>>4) &0x0f)*10 +(readingTime[2]&0x0f);
    int select_day = ((readingTime[3]>>4) &0x0f)*10 +(readingTime[3]&0x0f);
    int select_hh = ((readingTime[4]>>4) &0x0f)*10 +(readingTime[4]&0x0f);
    int select_mm =((readingTime[5]>>4) &0x0f)*10 +(readingTime[5]&0x0f);
    QString select_time = QString::number(select_year) + "-" + QString::number(select_month) + "-" + QString::number(select_day) + " " +QString::number(select_hh) + ":" + QString::number(select_mm);

    int nDayNum;
    int nDay;   //整点冻结查询次数
    QDate d1(select_year, select_month, select_day);  //查询的日期
    QDate d2(year, month, day);  // 当前日期
    nDayNum = d1.daysTo(d2);
    nDay = nDayNum + 1;

    FRAME_DLT_645 frame_dlt_645;
    bool errFlag = false;
    uint8_t Dlt645_buff[200];    //写电表指令数组
    uint8_t recData[200];          //读到的电表数据
    uint8_t targetData[200];     //
    memset(Dlt645_buff,0x0,sizeof(Dlt645_buff));
    memset(recData,0x0,sizeof(recData));
    memset(targetData,0x0,sizeof(targetData));

    pLog_dlt64507->getLogPoint(_strLogName)->info("##############remoteReadAmmeterData_07-DayFreeze##################");
    pLog_dlt64507->getLogPoint(_strLogName)->info("Start to read ammeter ==: "+ConvertHex2Qstr((unsigned char *)ammeterId, 6));

    for(int i=0;i<4;i++)
        Dlt645_buff[i] =0xFE;

    write(g_ammeterHandle,Dlt645_buff,4);//发送四个前导字节，唤醒接收方
    usleep(500000);

    if((nDay <= 62)&&(nDay > 0))
    {
        pLog_dlt64507->getLogPoint(_strLogName)->info("day_freee_energy_select time:" + select_time);
        pLog_dlt64507->getLogPoint(_strLogName)->info("day_freee_energy_select the number of:"  + QString::number(nDay));
        for(index = 0;index < 9;index ++)
        {
            getBytes = 0;
            readCount = 0;
            DLT645_07_readType03[index][0] = (char)nDay + 0x33;
            fixReadCMD(frame_dlt_645,info.addr,Dlt645_buff,DLT645_07_readType03[index]);   //组合读电表命令

            tcflush(g_ammeterHandle,TCIFLUSH);  //清空一次接收返回
            int wr = write(g_ammeterHandle,Dlt645_buff,16);
            pLog_dlt64507->getLogPoint(_strLogName)->info("03H-DAY_FREEZE write count---NUM:" + QString::number(index) + "===    " + QString::number(wr));
//            usleep(500000);
            if((index == 7)||(index == 8))
            {
                while(1)
                {
                    readCount++;
                    if(readCount > 20)
                        break;
                    usleep(100000);
                    memset(recData,0x0,sizeof(recData));
                    getBytes = read(g_ammeterHandle,recData, 212);
                    if(getBytes != 0 && getBytes != -1)
                    {
                        break;
                    }
                }
            }
            else
            {
                while(1)
                {
                    readCount++;
                    if(readCount > 3)
                        break;
                    usleep(500000);
                    memset(recData,0x0,sizeof(recData));
                    getBytes = read(g_ammeterHandle,recData, 212);
                    if(getBytes != 0 && getBytes != -1)
                    {
                        if(recData[10 + recData[9]] == verify(recData,10 + recData[9]))   //校验值相等
                        {
                            break;
                        }
                        else
                        {
                            pLog_dlt64507->getLogPoint(_strLogName)->info("Second Send cmd ==: "+ConvertHex2Qstr((unsigned char *)Dlt645_buff, 16));
                            tcflush(g_ammeterHandle,TCIFLUSH);  //清空一次接收返回
                            write(g_ammeterHandle,Dlt645_buff,16);
                        }
                    }
                    else
                    {
                        pLog_dlt64507->getLogPoint(_strLogName)->info("Second Send cmd ==: "+ConvertHex2Qstr((unsigned char *)Dlt645_buff, 16));
                        write(g_ammeterHandle,Dlt645_buff,16);
                    }
                }
            }
//                usleep(500000);
//            getBytes = read(g_ammeterHandle,recData, 212);
            pLog_dlt64507->getLogPoint(_strLogName)->info("03H-DAY_FREEZE get count---NUM:" + QString::number(index) + "===    " + QString::number(getBytes));
            if(checkRecData(getBytes,recData,targetData,errFlag,ammeterId))   //校验接收到的数据
            {
                decodeRemoteRecDataDayFreeze(targetData,index, errFlag,info);  //解析远程抄表的接收数据
            }
            usleep(1000000);
        }
    }
    else
    {
        pLog_dlt64507->getLogPoint(_strLogName)->info("select fail:More than 62 days!!!");
    }
}

///
/// \brief dlt645_07::remoteReadAmmeterData_07_SettlementEnergy
/// \param ammeterId
/// \param readingTime
/// \param info
///远程读07电表数据-结算日电能
void dlt645_07::remoteReadAmmeterData_07_SettlementEnergy(unsigned char *ammeterId,unsigned char *readingTime, stAmmeterConfig &info)
{
    int getBytes;
    int index = 0;
    int readCount;
    QDateTime dt = QDateTime::currentDateTime();
    short year=dt.toString("yyyy").toInt();
    short month = dt.toString("MM").toInt();

    int select_year = ((readingTime[0]>>4) &0x0f)*1000 +(readingTime[0]&0x0f)*100 + ((readingTime[1]>>4) &0x0f)*10 +(readingTime[1]&0x0f);
    int select_month = ((readingTime[2]>>4) &0x0f)*10 +(readingTime[2]&0x0f);
    int select_day = ((readingTime[3]>>4) &0x0f)*10 +(readingTime[3]&0x0f);
    int select_hh = ((readingTime[4]>>4) &0x0f)*10 +(readingTime[4]&0x0f);
    int select_mm =((readingTime[5]>>4) &0x0f)*10 +(readingTime[5]&0x0f);
    QString select_time = QString::number(select_year) + "-" + QString::number(select_month) + "-" + QString::number(select_day) + " " +QString::number(select_hh) + ":" + QString::number(select_mm);

    int nMonth; //结算日查询次数


    FRAME_DLT_645 frame_dlt_645;
    bool errFlag = false;
    uint8_t Dlt645_buff[200];    //写电表指令数组
    uint8_t recData[200];          //读到的电表数据
    uint8_t targetData[200];     //
    memset(Dlt645_buff,0x0,sizeof(Dlt645_buff));
    memset(recData,0x0,sizeof(recData));
    memset(targetData,0x0,sizeof(targetData));

    pLog_dlt64507->getLogPoint(_strLogName)->info("##############remoteReadAmmeterData_07-Settlement##################");
    pLog_dlt64507->getLogPoint(_strLogName)->info("Start to read ammeter ==: "+ConvertHex2Qstr((unsigned char *)ammeterId, 6));

    for(int i=0;i<4;i++)
        Dlt645_buff[i] =0xFE;

    write(g_ammeterHandle,Dlt645_buff,4);//发送四个前导字节，唤醒接收方
    usleep(500000);

    if(year - select_year == 1)  //跨年
        nMonth = 12 - select_month +month + 1;
    else
        nMonth = month - select_month + 1;

    if((nMonth <= 12)&&(nMonth > 0))
    {
        pLog_dlt64507->getLogPoint(_strLogName)->info("settlement_freee_energy_select time:" + select_time);
        pLog_dlt64507->getLogPoint(_strLogName)->info("settlement_freee_energy_select the number of:"  + QString::number(nMonth));
        for(index = 0;index < 12;index ++)
        {
            getBytes = 0;
            readCount = 0;
            DLT645_07_readType04[index][0] = (char)nMonth + 0x33;
            fixReadCMD(frame_dlt_645,info.addr,Dlt645_buff,DLT645_07_readType04[index]);   //组合读电表命令

            tcflush(g_ammeterHandle,TCIFLUSH);  //清空一次接收返回
            int wr = write(g_ammeterHandle,Dlt645_buff,16);
            pLog_dlt64507->getLogPoint(_strLogName)->info("04H-SETTLEMENT write count---NUM:" + QString::number(index) + "===    " + QString::number(wr));
//            usleep(500000);
//            getBytes = read(g_ammeterHandle,recData, 212);
            while(1)
            {
                readCount++;
                if(readCount > 3)
                    break;
                usleep(500000);
                memset(recData,0x0,sizeof(recData));
                getBytes = read(g_ammeterHandle,recData, 212);
                if(getBytes != 0 && getBytes != -1)
                {
                    if(recData[10 + recData[9]] == verify(recData,10 + recData[9]))   //校验值相等
                    {
                        break;
                    }
                    else
                    {
                        pLog_dlt64507->getLogPoint(_strLogName)->info("Second Send cmd ==: "+ConvertHex2Qstr((unsigned char *)Dlt645_buff, 16));
                        tcflush(g_ammeterHandle,TCIFLUSH);  //清空一次接收返回
                        write(g_ammeterHandle,Dlt645_buff,16);
                    }
                }
                else
                {
                    pLog_dlt64507->getLogPoint(_strLogName)->info("Second Send cmd ==: "+ConvertHex2Qstr((unsigned char *)Dlt645_buff, 16));
                    write(g_ammeterHandle,Dlt645_buff,16);
                }
            }
            pLog_dlt64507->getLogPoint(_strLogName)->info("04H-SETTLEMENT get coun---NUM:" + QString::number(index) + "===    " + QString::number(getBytes));
            if(checkRecData(getBytes,recData,targetData,errFlag,ammeterId))   //校验接收到的数据
            {
                decodeRemoteRecDataSettlement(targetData,index, errFlag,info);  //解析远程抄表的接收数据
            }
            usleep(1000000);
        }
    }
    else
    {
        pLog_dlt64507->getLogPoint(_strLogName)->info("select fail:More than 12 monthly!!!");
    }
}

///
/// \brief dlt645_07::decodeRemoteRecDataCurrent
/// \param targetData
/// \param valueFlag
/// \param errFlag
/// \param info
///解析远程抄表的接收数据
void dlt645_07::decodeRemoteRecDataCurrent(uint8_t *targetData,int valueFlag, bool &errFlag,stAmmeterConfig &info)
{
    FRAME_CURRENT_ENERGY_MAX_DEMAND_DATA currentEnergyMaxDemandData;
    switch (valueFlag) {
    case 0:  //正向有功总电能
    case 1: //(当前)正向有功费率1电能
    case 2: //(当前)正向有功费率2电能
    case 3: //(当前)正向有功费率3电能
    case 4: //(当前)正向有功费率4电能
    case 5: //(当前)反向有功总电能
    case 6: //（当前）第一象限无功总电能
    case 7: //（当前）第二象限无功总电能
    case 8: //（当前）第三象限无功总电能
    case 9: //（当前）第四象限无功总电能
         parse_current_energy(valueFlag,targetData, errFlag,currentEnergyMaxDemandData,info.voltageRatio,info.currentRatio);
        break;
    case 10: //(当前)正向有功总最大需量及发生时间
    case 11: //(当前)反向有功总最大需量及发生时间
        parse_current_max_demand_time(valueFlag,targetData, errFlag,currentEnergyMaxDemandData,info.voltageRatio,info.currentRatio);
        break;
    default:
        break;
    }
}

///
/// \brief dlt645_07::decodeRemoteRecDataHourFreeze
/// \param targetData
/// \param errFlag
/// \param info
///解析数据-整点冻结
void dlt645_07::decodeRemoteRecDataHourFreeze(uint8_t *targetData,bool &errFlag,stAmmeterConfig &info)
{
    FRAME_HOUR_FREEZE_ENERGY_DATA hourFreezeEnergyData;
    parse_hour_freeze_energy_data(targetData, errFlag,hourFreezeEnergyData,info.voltageRatio,info.currentRatio);
}

///
/// \brief dlt645_07::decodeRemoteRecDataDayFreeze
/// \param targetData
/// \param valueFlag
/// \param errFlag
/// \param info
///解析数据-日冻结
void dlt645_07::decodeRemoteRecDataDayFreeze(uint8_t *targetData,int valueFlag, bool &errFlag,stAmmeterConfig &info)
{
    DAY_FREEZE_ENERGY_MAX_DEMAND_DATA dayFreezeEnergyMaxDemandData;
    switch (valueFlag) {
    case 1: //(上一次)日冻结正向有功电能数据
    case 2: //(上一次)日冻结反向有功电能数据
    case 3: //(上一次)日冻结正向无功电能数据-第一象限无功总电能
    case 4: //(上一次)日冻结正向无功电能数据-第二象限无功总电能
    case 5: //(上一次)日冻结反向无功电能数据-第三象限无功总电能
    case 6: //(上一次)日冻结反向无功电能数据-第四象限无功总电能
        parse_day_freeze_energy(valueFlag,targetData, errFlag,dayFreezeEnergyMaxDemandData,info.voltageRatio,info.currentRatio);
        break;
    case 0: //(上1次)日冻结时间
    case 7: //（上 1 次）日冻结正向有功最大需量及发生时间数据
    case 8: //（上 1 次）日冻结反向有功最大需量及发生时间数据
         parse_day_freeze_max_demand_time(valueFlag,targetData, errFlag,dayFreezeEnergyMaxDemandData,info.voltageRatio,info.currentRatio);
        break;
    default:
        break;
    }
}

///
/// \brief dlt645_07::decodeRemoteRecDataSettlement
/// \param targetData
/// \param valueFlag
/// \param errFlag
/// \param info
///解析数据-结算日
void dlt645_07::decodeRemoteRecDataSettlement(uint8_t *targetData,int valueFlag, bool &errFlag,stAmmeterConfig &info)
{
    SETTLEMENT_ENERGY_MAX_DEMAND_DATA settlementEnergyMaxDemandData;
    switch (valueFlag) {
    case 0:  //(上N结算日)正向有功总电能
    case 1: //(上N结算日)正向有功费率1电能
    case 2: //(上N结算日)正向有功费率2电能
    case 3: //(上N结算日)正向有功费率3电能
    case 4: //(上N结算日)正向有功费率4电能
    case 5: //(上N结算日)反向有功总电能
    case 6: //（上N结算日）第一象限无功总电能
    case 7: //（上N结算日）第二象限无功总电能
    case 8: //（上N结算日）第三象限无功总电能
    case 9: //（上N结算日）第四象限无功总电能
         parse_settlement_energy(valueFlag,targetData, errFlag,settlementEnergyMaxDemandData,info.voltageRatio,info.currentRatio);
        break;
    case 10: //(上N结算日)正向有功总最大需量及发生时间
    case 11: //(上N结算日)反向有功总最大需量及发生时间
        parse_settlement_max_demand_time(valueFlag,targetData, errFlag,settlementEnergyMaxDemandData,info.voltageRatio,info.currentRatio);
        break;
    default:
        break;
    }
}


///
/// \brief dlt645_07::parse_current_energy
/// \param valueFlag
/// \param targetData
/// \param errFlag
/// \param currentEnergyMaxDemandData
/// \param PT_value
/// \param CT_value
///(当前)电能解析函数
void dlt645_07::parse_current_energy(int valueFlag,uint8_t *targetData,bool & errFlag,FRAME_CURRENT_ENERGY_MAX_DEMAND_DATA &currentEnergyMaxDemandData,int &PT_value,int &CT_value)
{
    QByteArray qAmmeterByteArray;
    qAmmeterByteArray.clear();
    int lowByte,highByte;
    int elePower;
    lowByte =   ((targetData[1]>>4) &0x0f)*1000+(targetData[1]&0x0f)*100+((targetData[0]>>4) &0x0f)*10 +(targetData[0]&0x0f) ;
    highByte =  ((targetData[3]>>4) &0x0f)*10000000 +(targetData[3]&0x0f)*1000000+((targetData[2]>>4) &0x0f)*100000+ (targetData[2]&0x0f)*10000;
    if(!errFlag)
        elePower = (highByte+lowByte)*CT_value*PT_value;
    else
        elePower = 0x7FFFFFFF;
    switch (valueFlag) {
    case 0:  //正向有功总电能
    {
        pLog_dlt64507->getLogPoint(_strLogName)->info("current_active_absorb_energy :    "+QString::number(elePower));
        currentEnergyMaxDemandData.current_active_absorb_energy =  elePower;
        qAmmeterByteArray.append((char*)&currentEnergyMaxDemandData.current_active_absorb_energy,4);
        dataMap_645_07.insert (Addr_RemoteAmeterCurrentInfo_1_Adj,qAmmeterByteArray);
    }
        break;

    case 1: //(当前)正向有功费率1电能
        pLog_dlt64507->getLogPoint(_strLogName)->info("current_active_absorb_rate1_energy :    "+QString::number(elePower));
        currentEnergyMaxDemandData.current_active_absorb_rate1_energy =  elePower;
        qAmmeterByteArray = QByteArray((char*)&currentEnergyMaxDemandData.current_active_absorb_rate1_energy,4);
        dataMap_645_07.insert(Addr_RemoteAmeterCurrentInfo_2_Adj,qAmmeterByteArray);
        break;
    case 2: //(当前)正向有功费率2电能
        pLog_dlt64507->getLogPoint(_strLogName)->info("current_active_absorb_rate2_energy :    "+QString::number(elePower));
        currentEnergyMaxDemandData.current_active_absorb_rate2_energy =  elePower;
        qAmmeterByteArray = QByteArray((char*)&currentEnergyMaxDemandData.current_active_absorb_rate2_energy,4);
        dataMap_645_07.insert(Addr_RemoteAmeterCurrentInfo_3_Adj,qAmmeterByteArray);
        break;
    case 3: //(当前)正向有功费率3电能
        pLog_dlt64507->getLogPoint(_strLogName)->info("current_active_absorb_rate3_energy :    "+QString::number(elePower));
        currentEnergyMaxDemandData.current_active_absorb_rate3_energy =  elePower;
        qAmmeterByteArray = QByteArray((char*)&currentEnergyMaxDemandData.current_active_absorb_rate3_energy,4);
        dataMap_645_07.insert(Addr_RemoteAmeterCurrentInfo_4_Adj,qAmmeterByteArray);
        break;
    case 4: //(当前)正向有功费率4电能
        pLog_dlt64507->getLogPoint(_strLogName)->info("current_active_absorb_rate4_energy :    "+QString::number(elePower));
        currentEnergyMaxDemandData.current_active_absorb_rate4_energy =  elePower;
        qAmmeterByteArray = QByteArray((char*)&currentEnergyMaxDemandData.current_active_absorb_rate4_energy,4);
        dataMap_645_07.insert(Addr_RemoteAmeterCurrentInfo_5_Adj,qAmmeterByteArray);
        break;
    case 5: //(当前)反向有功总电能
        pLog_dlt64507->getLogPoint(_strLogName)->info("current_active_liberate_energy :    "+QString::number(elePower));
        currentEnergyMaxDemandData.current_active_liberate_energy =  elePower;
        qAmmeterByteArray = QByteArray((char*)&currentEnergyMaxDemandData.current_active_liberate_energy,4);
        dataMap_645_07.insert(Addr_RemoteAmeterCurrentInfo_6_Adj,qAmmeterByteArray);
        break;
    case 6: //（当前）第一象限无功总电能
        pLog_dlt64507->getLogPoint(_strLogName)->info("current_reactive_absorb_energy1 :    "+QString::number(elePower));
        currentEnergyMaxDemandData.current_reactive_absorb_energy1 =  elePower;
        uiCurrentReactiveLiberateEnergy = currentEnergyMaxDemandData.current_reactive_absorb_energy1;
        break;
    case 7: //（当前）第二象限无功总电能
        pLog_dlt64507->getLogPoint(_strLogName)->info("current_reactive_absorb_energy2 :    "+QString::number(elePower));
        currentEnergyMaxDemandData.current_reactive_absorb_energy2 =  elePower;
        qAmmeterByteArray.clear();
        currentEnergyMaxDemandData.current_reactive_absorb_energy = uiCurrentReactiveLiberateEnergy + currentEnergyMaxDemandData.current_reactive_absorb_energy2;
        qAmmeterByteArray = QByteArray((char*)&currentEnergyMaxDemandData.current_reactive_absorb_energy,4);
        dataMap_645_07.insert(Addr_RemoteAmeterCurrentInfo_7_Adj,qAmmeterByteArray);
        break;
    case 8: //（当前）第三象限无功总电能
        pLog_dlt64507->getLogPoint(_strLogName)->info("current_reactive_liberate_energy1 :    "+QString::number(elePower));
        currentEnergyMaxDemandData.current_reactive_liberate_energy1 = elePower;
        uiCurrentReactiveAbsortEnergy = currentEnergyMaxDemandData.current_reactive_liberate_energy1;
        break;
    case 9: //（当前）第四象限无功总电能
        pLog_dlt64507->getLogPoint(_strLogName)->info("current_reactive_liberate_energy2 :    "+QString::number(elePower));
        currentEnergyMaxDemandData.current_reactive_liberate_energy2 =  elePower;
        currentEnergyMaxDemandData.current_reactive_liberate_energy = uiCurrentReactiveAbsortEnergy + currentEnergyMaxDemandData.current_reactive_liberate_energy2;
        qAmmeterByteArray = QByteArray((char*)&currentEnergyMaxDemandData.current_reactive_liberate_energy,4);
        dataMap_645_07.insert(Addr_RemoteAmeterCurrentInfo_8_Adj,qAmmeterByteArray);
        break;
    default:
        break;
    }
    memset(targetData,0,sizeof(targetData));   //清空数据
}


///
/// \brief dlt645_07::parse_current_max_demand_time
/// \param valueFlag
/// \param targetData
/// \param errFlag
/// \param currentEnergyMaxDemandData
/// \param PT_value
/// \param CT_value
/////(当前)最大需量及发生时间解析函数
 void dlt645_07::parse_current_max_demand_time(int valueFlag,uint8_t *targetData,bool & errFlag,FRAME_CURRENT_ENERGY_MAX_DEMAND_DATA &currentEnergyMaxDemandData,int &PT_value,int &CT_value)
 {
     int byte;
     int demand;
     QByteArray qAmmeterByteArray;
     qAmmeterByteArray.clear();

     byte =   ((targetData[2]>>4) &0x0f)*100000+ (targetData[2]&0x0f)*10000 + ((targetData[1]>>4) &0x0f)*1000+(targetData[1]&0x0f)*100+((targetData[0]>>4) &0x0f)*10 +(targetData[0]&0x0f);
     if(!errFlag)
         demand = byte*CT_value*PT_value;
     else
         demand = 0x7FFFFFFF;
     switch(valueFlag){
     case 10: //(当前)正向有功总最大需量及发生时间
         pLog_dlt64507->getLogPoint(_strLogName)->info("current_active_absorb_max_demand :    "+QString::number(demand));
         currentEnergyMaxDemandData.current_active_absorb_max_demand = (float)demand/10000;
         ConvertDataFormat(&targetData[3],5);      //大小端转换
         memcpy(currentEnergyMaxDemandData.current_active_absorb_max_demand_time,&targetData[3],sizeof(currentEnergyMaxDemandData.current_active_absorb_max_demand_time));
//         for(int i = 0;i <5;i++)
//         {
//         }
         if(errFlag){
             memset(currentEnergyMaxDemandData.current_active_absorb_max_demand_time,0x00,5);
         }
         qAmmeterByteArray.append(QByteArray((char*)&currentEnergyMaxDemandData.current_active_absorb_max_demand,4));
         qAmmeterByteArray.append(QByteArray((char*)&currentEnergyMaxDemandData.current_active_absorb_max_demand_time,5));
         dataMap_645_07.insert(Addr_RemoteAmeterCurrentInfo_9_Adj, qAmmeterByteArray);
     break;
     case 11: //(当前)反向有功总最大需量及发生时间
         pLog_dlt64507->getLogPoint(_strLogName)->info("current_reactive_absorb_max_demand :    "+QString::number(demand));
         currentEnergyMaxDemandData.current_active_liberate_max_demand = (float)demand/10000;
         ConvertDataFormat(&targetData[3],5);      //大小端转换
         memcpy(currentEnergyMaxDemandData.current_active_liberate_max_demand_time,&targetData[3],sizeof(currentEnergyMaxDemandData.current_active_liberate_max_demand_time));

         if(errFlag){
             memset(currentEnergyMaxDemandData.current_active_liberate_max_demand_time,0x00,5);
         }
         qAmmeterByteArray.append(QByteArray((char*)&currentEnergyMaxDemandData.current_active_liberate_max_demand,4));
         qAmmeterByteArray.append(QByteArray((char*)&currentEnergyMaxDemandData.current_active_liberate_max_demand_time,5));
         dataMap_645_07.insert(Addr_RemoteAmeterCurrentInfo_10_Adj, qAmmeterByteArray);
         break;
     default:
         break;
     }
     memset(targetData,0,sizeof(targetData));   //清空数据
 }

 ///
 /// \brief dlt645_07::parse_hour_freeze_energy_data
 /// \param targetData
 /// \param errFlag
 /// \param hourFreezeEnergyData
 /// \param PT_value
 /// \param CT_value
 ///整点冻结电能解析函数
 void dlt645_07::parse_hour_freeze_energy_data(uint8_t *targetData,bool & errFlag,FRAME_HOUR_FREEZE_ENERGY_DATA &hourFreezeEnergyData,int &PT_value,int &CT_value)
 {
     QByteArray qAmmeterByteArray;
     qAmmeterByteArray.clear();

     int hour_freeze_active_absorb_energy_byte;
     int hour_freeze_active_absorb_energy;
     int hour_freeze_active_liberate_energy_byte;
     int hour_freeze_active_liberate_energy;
     for(int i = 0;i < (int)sizeof(targetData);i ++){  //校验此项是否启用
         if(targetData[i] == 0xaa)
         {
             errFlag = true;
             break;
         }
     }
     hour_freeze_active_absorb_energy_byte = ((targetData[8]>>4) &0x0f)*10000000+ (targetData[8]&0x0f)*1000000 + ((targetData[7]>>4) &0x0f)*100000+ (targetData[7]&0x0f)*10000 +
                                                                            ((targetData[6]>>4) &0x0f)*1000+(targetData[6]&0x0f)*100+((targetData[5]>>4) &0x0f)*10 +(targetData[5]&0x0f);
     hour_freeze_active_liberate_energy_byte = ((targetData[12]>>4) &0x0f)*10000000+ (targetData[12]&0x0f)*1000000 + ((targetData[11]>>4) &0x0f)*100000+ (targetData[11]&0x0f)*10000 +
                                                                            ((targetData[10]>>4) &0x0f)*1000+(targetData[10]&0x0f)*100+((targetData[9]>>4) &0x0f)*10 +(targetData[9]&0x0f);
     if(!errFlag){
         hour_freeze_active_absorb_energy = hour_freeze_active_absorb_energy_byte*CT_value*PT_value;
         hour_freeze_active_liberate_energy = hour_freeze_active_liberate_energy_byte*CT_value*PT_value;
     }
     else{
         hour_freeze_active_absorb_energy = 0x7FFFFFFF;
         hour_freeze_active_liberate_energy = 0x7FFFFFFF;
     }

     ConvertDataFormat(&targetData[0],5);      //大小端转换
     memcpy(hourFreezeEnergyData.hour_freeze_time,&targetData[0],sizeof(hourFreezeEnergyData.hour_freeze_time));
     qAmmeterByteArray = QByteArray((char*)&hourFreezeEnergyData.hour_freeze_time,5);
     dataMap_645_07.insert(Addr_RemoteAmeterHourFreezeInfo_1_Adj,qAmmeterByteArray);

     pLog_dlt64507->getLogPoint(_strLogName)->info("hour_freeze_active_absorb_energy :    "+QString::number(hour_freeze_active_absorb_energy));
     hourFreezeEnergyData.hour_freeze_active_absorb_energy = hour_freeze_active_absorb_energy;
     qAmmeterByteArray.clear();
     qAmmeterByteArray = QByteArray((char*)&hourFreezeEnergyData.hour_freeze_active_absorb_energy,4);
     dataMap_645_07.insert(Addr_RemoteAmeterHourFreezeInfo_2_Adj,qAmmeterByteArray);

     pLog_dlt64507->getLogPoint(_strLogName)->info("hour_freeze_active_liberate_energy :    "+QString::number(hour_freeze_active_liberate_energy));
     hourFreezeEnergyData.hour_freeze_active_liberate_energy = hour_freeze_active_liberate_energy;
     qAmmeterByteArray.clear();
     qAmmeterByteArray = QByteArray((char*)&hourFreezeEnergyData.hour_freeze_active_liberate_energy,4);
     dataMap_645_07.insert(Addr_RemoteAmeterHourFreezeInfo_3_Adj,qAmmeterByteArray);

     memset(targetData,0,sizeof(targetData));   //清空数据
 }

 ///
 /// \brief dlt645_07::parse_day_freeze_energy
 /// \param valueFlag
 /// \param targetData
 /// \param errFlag
 /// \param dayFreezeEnergyMaxDemandData
 /// \param PT_value
 /// \param CT_value
 ///日冻结电能解析函数
void dlt645_07::parse_day_freeze_energy(int valueFlag,uint8_t *targetData,bool & errFlag,DAY_FREEZE_ENERGY_MAX_DEMAND_DATA &dayFreezeEnergyMaxDemandData,int &PT_value,int &CT_value)
{
    QByteArray qAmmeterByteArray;
    qAmmeterByteArray.clear();

    int lowByte,highByte;
    int elePower;
    int rateLowByte[4],rateHighByte[4],rataElePower[4];
    lowByte =   ((targetData[1]>>4) &0x0f)*1000+(targetData[1]&0x0f)*100+((targetData[0]>>4) &0x0f)*10 +(targetData[0]&0x0f) ;
    highByte =  ((targetData[3]>>4) &0x0f)*10000000 +(targetData[3]&0x0f)*1000000+((targetData[2]>>4) &0x0f)*100000+ (targetData[2]&0x0f)*10000;
    if(!errFlag)
        elePower = (highByte+lowByte)*CT_value*PT_value;
    else
        elePower = 0x7FFFFFFF;
    switch (valueFlag) {
    case 1: //（上 1 次）日冻结正向有功电能数据
    {

        pLog_dlt64507->getLogPoint(_strLogName)->info("day_freeze_active_absorb_energy :    "+QString::number(elePower));
        dayFreezeEnergyMaxDemandData.day_freeze_active_absorb_energy =  elePower;
        for(int i = 0;i < 4;i ++)
        {
            rateLowByte[i] =   ((targetData[i*4 + 5]>>4) &0x0f)*1000+(targetData[i*4 + 5]&0x0f)*100+((targetData[i*4 + 4]>>4) &0x0f)*10 +(targetData[i*4 + 4]&0x0f) ;
            rateHighByte[i] =  ((targetData[i*4 + 7]>>4) &0x0f)*10000000 +(targetData[i*4 + 7]&0x0f)*1000000+((targetData[i*4 + 6]>>4) &0x0f)*100000+ (targetData[i*4 + 6]&0x0f)*10000;
            if(!errFlag)
                rataElePower[i] = (rateLowByte[i]+rateHighByte[i])*CT_value*PT_value;
            else
                rataElePower[i] = 0x7FFFFFFF;
        }
        pLog_dlt64507->getLogPoint(_strLogName)->info("day_freeze_active_absorb_rate1_energy :    "+QString::number(rataElePower[0]));
        dayFreezeEnergyMaxDemandData.day_freeze_active_absorb_rate1_energy =  rataElePower[0];

        pLog_dlt64507->getLogPoint(_strLogName)->info("day_freeze_active_absorb_rate2_energy :    "+QString::number(rataElePower[1]));
        dayFreezeEnergyMaxDemandData.day_freeze_active_absorb_rate2_energy =  rataElePower[1];

        pLog_dlt64507->getLogPoint(_strLogName)->info("day_freeze_active_absorb_rate3_energy :    "+QString::number(rataElePower[2]));
        dayFreezeEnergyMaxDemandData.day_freeze_active_absorb_rate3_energy =  rataElePower[2];

        pLog_dlt64507->getLogPoint(_strLogName)->info("day_freeze_active_absorb_rate4_energy :    "+QString::number(rataElePower[3]));
        dayFreezeEnergyMaxDemandData.day_freeze_active_absorb_rate4_energy =  rataElePower[3];

        qAmmeterByteArray.append((char*)&dayFreezeEnergyMaxDemandData.day_freeze_active_absorb_energy,4);
        qAmmeterByteArray.append((char*)&dayFreezeEnergyMaxDemandData.day_freeze_active_absorb_rate1_energy,4);
        qAmmeterByteArray.append((char*)&dayFreezeEnergyMaxDemandData.day_freeze_active_absorb_rate2_energy,4);
        qAmmeterByteArray.append((char*)&dayFreezeEnergyMaxDemandData.day_freeze_active_absorb_rate3_energy,4);
        qAmmeterByteArray.append((char*)&dayFreezeEnergyMaxDemandData.day_freeze_active_absorb_rate4_energy,4);
        dataMap_645_07.insert(Addr_RemoteAmeterDayFreezeInfo_2_Adj,qAmmeterByteArray);
    }
        break;
    case 2: //（上 1 次）日冻结反向有功电能数据
        pLog_dlt64507->getLogPoint(_strLogName)->info("day_freeze_reactive_absorb_energy :    "+QString::number(elePower));
        dayFreezeEnergyMaxDemandData.day_freeze_active_liberate_energy =  elePower;

        qAmmeterByteArray = QByteArray((char*)&dayFreezeEnergyMaxDemandData.day_freeze_active_liberate_energy,4);
        dataMap_645_07.insert(Addr_RemoteAmeterDayFreezeInfo_3_Adj,qAmmeterByteArray);
        break;
    case 3: //（上 1 次）日冻结第一象限无功电能数
        pLog_dlt64507->getLogPoint(_strLogName)->info("day_freeze_reactive_absorb_energy1 :    "+QString::number(elePower));
        dayFreezeEnergyMaxDemandData.day_freeze_reactive_absorb_energy1 =  elePower;
        uiDayFreezeReactiveLiberateEnergy = dayFreezeEnergyMaxDemandData.day_freeze_reactive_absorb_energy1;

        qAmmeterByteArray = QByteArray((char*)&dayFreezeEnergyMaxDemandData.day_freeze_reactive_absorb_energy1,4);
        dataMap_645_07.insert(Addr_Remote_day_freeze_reactive_absorb_energy1_Term,qAmmeterByteArray);
        break;
    case 4: //（上 1 次）日冻结第二象限无功电能数
        pLog_dlt64507->getLogPoint(_strLogName)->info("day_freeze_reactive_absorb_energy2 :    "+QString::number(elePower));
        dayFreezeEnergyMaxDemandData.day_freeze_reactive_absorb_energy2 =  elePower;

        qAmmeterByteArray = QByteArray((char*)&dayFreezeEnergyMaxDemandData.day_freeze_reactive_absorb_energy2,4);
        dataMap_645_07.insert(Addr_Remote_day_freeze_reactive_absorb_energy2_Term,qAmmeterByteArray);

        qAmmeterByteArray.clear();
        dayFreezeEnergyMaxDemandData.day_freeze_reactive_absorb_energy = uiDayFreezeReactiveLiberateEnergy + dayFreezeEnergyMaxDemandData.day_freeze_reactive_absorb_energy2;
        qAmmeterByteArray = QByteArray((char*)&dayFreezeEnergyMaxDemandData.day_freeze_reactive_absorb_energy,4);
        dataMap_645_07.insert(Addr_RemoteAmeterDayFreezeInfo_4_Adj,qAmmeterByteArray);
        break;
    case 5: //（上 1 次）日冻结第三象限无功电能数
        pLog_dlt64507->getLogPoint(_strLogName)->info("day_freeze_reactive_liberate_energy1 :    "+QString::number(elePower));
        dayFreezeEnergyMaxDemandData.day_freeze_reactive_liberate_energy1 =  elePower;
        uiDayFreezeReactiveAbsortEnergy = dayFreezeEnergyMaxDemandData.day_freeze_reactive_liberate_energy1;

        qAmmeterByteArray = QByteArray((char*)&dayFreezeEnergyMaxDemandData.day_freeze_reactive_liberate_energy1,4);
        dataMap_645_07.insert(Addr_Remote_day_freeze_reactive_liberate_energy1_Term,qAmmeterByteArray);
        break;
    case 6: //（上 1 次）日冻结第四象限无功电能数
        pLog_dlt64507->getLogPoint(_strLogName)->info("day_freeze_reactive_liberate_energy2 :    "+QString::number(elePower));
        dayFreezeEnergyMaxDemandData.day_freeze_reactive_liberate_energy2 =  elePower;

        qAmmeterByteArray = QByteArray((char*)&dayFreezeEnergyMaxDemandData.day_freeze_reactive_liberate_energy2,4);
        dataMap_645_07.insert(Addr_Remote_day_freeze_reactive_liberate_energy2_Term,qAmmeterByteArray);

        qAmmeterByteArray.clear();
        dayFreezeEnergyMaxDemandData.day_freeze_reactive_liberate_energy = uiDayFreezeReactiveAbsortEnergy + dayFreezeEnergyMaxDemandData.day_freeze_reactive_liberate_energy2;
        qAmmeterByteArray = QByteArray((char*)&dayFreezeEnergyMaxDemandData.day_freeze_reactive_liberate_energy,4);
        dataMap_645_07.insert(Addr_RemoteAmeterDayFreezeInfo_5_Adj,qAmmeterByteArray);
        break;
    default:
        break;
    }
    memset(targetData,0,sizeof(targetData));   //清空数据
}

///
/// \brief dlt645_07::parse_day_freeze_max_demand_time
/// \param valueFlag
/// \param targetData
/// \param errFlag
/// \param dayFreezeEnergyMaxDemandData
/// \param PT_value
/// \param CT_value
///日冻结最大需量及发生时间函数
void dlt645_07::parse_day_freeze_max_demand_time(int valueFlag,uint8_t *targetData,bool & errFlag,DAY_FREEZE_ENERGY_MAX_DEMAND_DATA &dayFreezeEnergyMaxDemandData,int &PT_value,int &CT_value)
{
    QByteArray qAmmeterByteArray;
    qAmmeterByteArray.clear();

//    QDateTime dt = QDateTime::currentDateTime();
//    QString year=dt.toString("yyyy").mid(0,2);
//    short ch = dt.toString("yyyy").mid(0,2).toInt();//截取年yy

    if(valueFlag == 0)
    {
        ConvertDataFormat(&targetData[0],5);      //大小端转换
        memcpy(dayFreezeEnergyMaxDemandData.day_freeze_time,&targetData[0],5);
//        for(int i = 0;i<6;i++)
//        {
//        }

        qAmmeterByteArray = QByteArray((char*)&dayFreezeEnergyMaxDemandData.day_freeze_time,5);
        dataMap_645_07.insert(Addr_RemoteAmeterDayFreezeInfo_1_Adj,qAmmeterByteArray);
    }
    else
    {
        int byte;
        int demand;
        byte =   ((targetData[2]>>4) &0x0f)*100000+ (targetData[2]&0x0f)*10000 + ((targetData[1]>>4) &0x0f)*1000+(targetData[1]&0x0f)*100+((targetData[0]>>4) &0x0f)*10 +(targetData[0]&0x0f);
        if(!errFlag)
            demand = byte*CT_value*PT_value;
        else
            demand = 0x7FFFFFFF;
        switch(valueFlag){
        case 7: //（上 1 次）日冻结正向有功最大需量及发生时间数据
            pLog_dlt64507->getLogPoint(_strLogName)->info("day_freeze_active_absorb_max_demand :    "+QString::number(demand));
            dayFreezeEnergyMaxDemandData.day_freeze_active_absorb_max_demand = (float)demand/10000;
            ConvertDataFormat(&targetData[3],5);      //大小端转换
            memcpy(dayFreezeEnergyMaxDemandData.day_freeze_active_absorb_max_demand_time,&targetData[3],sizeof(dayFreezeEnergyMaxDemandData.day_freeze_active_absorb_max_demand_time));

            if(errFlag){
                memset(dayFreezeEnergyMaxDemandData.day_freeze_active_absorb_max_demand_time,0x00,5);
            }
            qAmmeterByteArray.append(QByteArray((char*)&dayFreezeEnergyMaxDemandData.day_freeze_active_absorb_max_demand,4));
            qAmmeterByteArray.append(QByteArray((char*)&dayFreezeEnergyMaxDemandData.day_freeze_active_absorb_max_demand_time,5));
            dataMap_645_07.insert(Addr_RemoteAmeterDayFreezeInfo_6_Adj, qAmmeterByteArray);
            break;
        case 8: //（上 1 次）日冻结反向有功最大需量及发生时间数据
            pLog_dlt64507->getLogPoint(_strLogName)->info("day_freeze_active_liberate_max_demand :    "+QString::number(demand));
            dayFreezeEnergyMaxDemandData.day_freeze_active_liberate_max_demand = (float)demand/10000;
            ConvertDataFormat(&targetData[3],5);      //大小端转换
            memcpy(dayFreezeEnergyMaxDemandData.day_freeze_active_liberate_max_demand_time,&targetData[3],sizeof(dayFreezeEnergyMaxDemandData.day_freeze_active_liberate_max_demand_time));

            if(errFlag){
                memset(dayFreezeEnergyMaxDemandData.day_freeze_active_liberate_max_demand_time,0x00,5);
            }
            qAmmeterByteArray.append(QByteArray((char*)&dayFreezeEnergyMaxDemandData.day_freeze_active_liberate_max_demand,4));
            qAmmeterByteArray.append(QByteArray((char*)&dayFreezeEnergyMaxDemandData.day_freeze_active_liberate_max_demand_time,5));
            dataMap_645_07.insert(Addr_RemoteAmeterDayFreezeInfo_7_Adj, qAmmeterByteArray);
            break;
        default:
            break;
        }
    }
    memset(targetData,0,sizeof(targetData));   //清空数据
}

///
/// \brief dlt645_07::parse_settlement_energy
/// \param valueFlag
/// \param targetData
/// \param errFlag
/// \param settlementEnergyMaxDemandData
/// \param PT_value
/// \param CT_value
///结算日电能解析函数
void dlt645_07::parse_settlement_energy(int valueFlag,uint8_t *targetData,bool & errFlag,SETTLEMENT_ENERGY_MAX_DEMAND_DATA &settlementEnergyMaxDemandData,int &PT_value,int &CT_value)
{
    QByteArray qAmmeterByteArray;
    qAmmeterByteArray.clear();
    int lowByte,highByte;
    int elePower;
    lowByte =   ((targetData[1]>>4) &0x0f)*1000+(targetData[1]&0x0f)*100+((targetData[0]>>4) &0x0f)*10 +(targetData[0]&0x0f) ;
    highByte =  ((targetData[3]>>4) &0x0f)*10000000 +(targetData[3]&0x0f)*1000000+((targetData[2]>>4) &0x0f)*100000+ (targetData[2]&0x0f)*10000;
    if(!errFlag)
        elePower = (highByte+lowByte)*CT_value*PT_value;
    else
        elePower = 0x7FFFFFFF;
    switch (valueFlag) {
    case 0:  //正向有功总电能
        pLog_dlt64507->getLogPoint(_strLogName)->info("settlement_active_absorb_energy :    "+QString::number(elePower));
        settlementEnergyMaxDemandData.settlement_active_absorb_energy =  elePower;
        qAmmeterByteArray = QByteArray((char*)&settlementEnergyMaxDemandData.settlement_active_absorb_energy,4);
        dataMap_645_07.insert(Addr_RemoteAmeterSettlementInfo_1_Adj,qAmmeterByteArray);
        break;
    case 1: //(结算日)正向有功费率1电能
        pLog_dlt64507->getLogPoint(_strLogName)->info("settlement_active_absorb_rate1_energy :    "+QString::number(elePower));
        settlementEnergyMaxDemandData.settlement_active_absorb_rate1_energy =  elePower;
        qAmmeterByteArray = QByteArray((char*)&settlementEnergyMaxDemandData.settlement_active_absorb_rate1_energy,4);
        dataMap_645_07.insert(Addr_RemoteAmeterSettlementInfo_2_Adj,qAmmeterByteArray);
        break;
    case 2: //(结算日)正向有功费率2电能
        pLog_dlt64507->getLogPoint(_strLogName)->info("settlement_active_absorb_rate2_energy :    "+QString::number(elePower));
        settlementEnergyMaxDemandData.settlement_active_absorb_rate2_energy =  elePower;
        qAmmeterByteArray = QByteArray((char*)&settlementEnergyMaxDemandData.settlement_active_absorb_rate2_energy,4);
        dataMap_645_07.insert(Addr_RemoteAmeterSettlementInfo_3_Adj,qAmmeterByteArray);
        break;
    case 3: //(结算日)正向有功费率3电能
        pLog_dlt64507->getLogPoint(_strLogName)->info("settlement_active_absorb_rate3_energy :    "+QString::number(elePower));
        settlementEnergyMaxDemandData.settlement_active_absorb_rate3_energy =  elePower;
        qAmmeterByteArray = QByteArray((char*)&settlementEnergyMaxDemandData.settlement_active_absorb_rate3_energy,4);
        dataMap_645_07.insert(Addr_RemoteAmeterSettlementInfo_4_Adj,qAmmeterByteArray);
        break;
    case 4: //(结算日)正向有功费率4电能
        pLog_dlt64507->getLogPoint(_strLogName)->info("settlement_active_absorb_rate4_energy :    "+QString::number(elePower));
        settlementEnergyMaxDemandData.settlement_active_absorb_rate4_energy =  elePower;
        qAmmeterByteArray = QByteArray((char*)&settlementEnergyMaxDemandData.settlement_active_absorb_rate4_energy,4);
        dataMap_645_07.insert(Addr_RemoteAmeterSettlementInfo_5_Adj,qAmmeterByteArray);
        break;
    case 5: //(结算日)反向有功总电能
        pLog_dlt64507->getLogPoint(_strLogName)->info("settlement_active_liberate_energy :    "+QString::number(elePower));
        settlementEnergyMaxDemandData.settlement_active_liberate_energy =  elePower;
        qAmmeterByteArray = QByteArray((char*)&settlementEnergyMaxDemandData.settlement_active_liberate_energy,4);
        dataMap_645_07.insert(Addr_RemoteAmeterSettlementInfo_6_Adj,qAmmeterByteArray);
        break;
    case 6: //（结算日）第一象限无功总电能
        pLog_dlt64507->getLogPoint(_strLogName)->info("settlement_reactive_absorb_energy1 :    "+QString::number(elePower));
        settlementEnergyMaxDemandData.settlement_reactive_absorb_energy1 =  elePower;
        uiSettlementReactiveLiberateEnergy = settlementEnergyMaxDemandData.settlement_reactive_absorb_energy1;
        break;
    case 7: //（结算日）第二象限无功总电能
        pLog_dlt64507->getLogPoint(_strLogName)->info("settlement_reactive_absorb_energy2 :    "+QString::number(elePower));
        settlementEnergyMaxDemandData.settlement_reactive_absorb_energy2 =  elePower;
        qAmmeterByteArray.clear();
        settlementEnergyMaxDemandData.settlement_reactive_absorb_energy = uiSettlementReactiveLiberateEnergy + settlementEnergyMaxDemandData.settlement_reactive_absorb_energy2;
        qAmmeterByteArray = QByteArray((char*)&settlementEnergyMaxDemandData.settlement_reactive_absorb_energy,4);
        dataMap_645_07.insert(Addr_RemoteAmeterSettlementInfo_7_Adj,qAmmeterByteArray);
        break;
    case 8: //（结算日）第三象限无功总电能
        pLog_dlt64507->getLogPoint(_strLogName)->info("settlement_reactive_liberate_energy1 :    "+QString::number(elePower));
        settlementEnergyMaxDemandData.settlement_reactive_liberate_energy1 =  elePower;
        uiSettlementReactiveAbsortEnergy = settlementEnergyMaxDemandData.settlement_reactive_liberate_energy1;
        break;
    case 9: //（结算日）第四象限无功总电能
        pLog_dlt64507->getLogPoint(_strLogName)->info("settlement_reactive_liberate_energy2 :    "+QString::number(elePower));
        settlementEnergyMaxDemandData.settlement_reactive_liberate_energy2 =  elePower;
        settlementEnergyMaxDemandData.settlement_reactive_liberate_energy = settlementEnergyMaxDemandData.settlement_reactive_liberate_energy2 + uiSettlementReactiveAbsortEnergy;
        qAmmeterByteArray = QByteArray((char*)&settlementEnergyMaxDemandData.settlement_reactive_liberate_energy,4);
        dataMap_645_07.insert(Addr_RemoteAmeterSettlementInfo_8_Adj,qAmmeterByteArray);
        break;
    default:
        break;
    }
    memset(targetData,0,sizeof(targetData));   //清空数据
}

///
/// \brief dlt645_07::parse_settlement_max_demand_time
/// \param valueFlag
/// \param targetData
/// \param errFlag
/// \param settlementEnergyMaxDemandData
/// \param PT_value
/// \param CT_value
/// 结算日最大需量及发生时间函数
void dlt645_07::parse_settlement_max_demand_time(int valueFlag,uint8_t *targetData,bool & errFlag,SETTLEMENT_ENERGY_MAX_DEMAND_DATA &settlementEnergyMaxDemandData,int &PT_value,int &CT_value)
{
    QByteArray qAmmeterByteArray;
    qAmmeterByteArray.clear();
    int byte;
    int demand;
    byte =   ((targetData[2]>>4) &0x0f)*100000+ (targetData[2]&0x0f)*10000 + ((targetData[1]>>4) &0x0f)*1000+(targetData[1]&0x0f)*100+((targetData[0]>>4) &0x0f)*10 +(targetData[0]&0x0f);
    if(!errFlag)
        demand = byte*CT_value*PT_value;
    else
        demand = 0x7FFFFFFF;
    switch(valueFlag){
    case 10: //(结算日)正向有功总最大需量及发生时间
        pLog_dlt64507->getLogPoint(_strLogName)->info("settlement_active_absorb_max_demand :    "+QString::number(demand));
        settlementEnergyMaxDemandData.settlement_active_absorb_max_demand = (float)demand/10000;
        ConvertDataFormat(&targetData[3],5);      //大小端转换
        memcpy(settlementEnergyMaxDemandData.settlement_active_absorb_max_demand_time,&targetData[3],sizeof(settlementEnergyMaxDemandData.settlement_active_absorb_max_demand_time));

        if(errFlag){
            memset(settlementEnergyMaxDemandData.settlement_active_absorb_max_demand_time,0x00,5);
        }
        qAmmeterByteArray.append(QByteArray((char*)&settlementEnergyMaxDemandData.settlement_active_absorb_max_demand,4));
        qAmmeterByteArray.append(QByteArray((char*)&settlementEnergyMaxDemandData.settlement_active_absorb_max_demand_time,5));
        dataMap_645_07.insert(Addr_RemoteAmeterSettlementInfo_9_Adj, qAmmeterByteArray);
        break;
    case 11: //(结算)反向有功总最大需量及发生时间
        pLog_dlt64507->getLogPoint(_strLogName)->info("settlement_active_liberate_max_demand :    "+QString::number(demand));
        settlementEnergyMaxDemandData.settlement_active_liberate_max_demand = (float)demand/10000;
        ConvertDataFormat(&targetData[3],5);      //大小端转换
        memcpy(settlementEnergyMaxDemandData.settlement_active_liberate_max_demand_time,&targetData[3],sizeof(settlementEnergyMaxDemandData.settlement_active_liberate_max_demand_time));

        if(errFlag){
            memset(settlementEnergyMaxDemandData.settlement_active_absorb_max_demand_time,0x00,5);
        }
        qAmmeterByteArray.append(QByteArray((char*)&settlementEnergyMaxDemandData.settlement_active_liberate_max_demand,4));
        qAmmeterByteArray.append(QByteArray((char*)&settlementEnergyMaxDemandData.settlement_active_liberate_max_demand_time,5));
        dataMap_645_07.insert(Addr_RemoteAmeterSettlementInfo_10_Adj, qAmmeterByteArray);
        break;
    default:
        break;
    }
    memset(targetData,0,sizeof(targetData));   //清空数据
}

void dlt645_07::slot_stopRead(bool flag)
{
    stopReadFlag = flag;
}

///
/// \brief dlt645_07::slot_readRemoteAmmeter_dlt645_07
///远程抄表
void dlt645_07::slot_readRemoteAmmeter_dlt645_07(unsigned char * ammeterId,int readDataType ,unsigned char * readingTime,stAmmeterConfig info)
{
    if(!Init())//串口初始化
    {
        return;
    }

    if(remoteReadAmmeterData_07(ammeterId,readDataType,readingTime,info)){
       emit sigReadOver_645_07(4);//07协议电表-远程抄表结束
       pLog_dlt64507->getLogPoint(_strLogName)->info("#################Read Ammete End#######################");
    }
}

 //读取功率监测电表功率
 void dlt645_07::slot_readPowerMonitorAmmeter(stAmmeterConfig info)
 {
     if(!Init())//串口初始化
     {
         return;
     }

     uint8_t Dlt645_buff[200];
     memset(Dlt645_buff,0x0,sizeof(Dlt645_buff));

     pLog_dlt64507->getLogPoint(_strLogName)->info("#####################################################");
     pLog_dlt64507->getLogPoint(_strLogName)->info("Start to read ammeter ==: "+ConvertHex2Qstr((unsigned char *)info.addr, 6));

     for(int i=0;i<4;i++)
         Dlt645_buff[i] =0xFE;

     write(g_ammeterHandle,Dlt645_buff,4);//发送四个前导字节，唤醒接收方
     usleep(500000);

     ReadPowerMonitorAmmeter(info);
 }
