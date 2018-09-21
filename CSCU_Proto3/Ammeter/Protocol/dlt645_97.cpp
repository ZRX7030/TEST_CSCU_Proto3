#include "dlt645_97.h"
#include <QDebug>
#include <qcoreapplication.h>

#define ItemCount_97  24

//uint8_t DLT645_97_item[24][2]={{0x44,0xE9},{0x45,0xE9},{0x46,0xE9},{0x54,0xE9},{0x55,0xE9},{0x56,0xE9},{0x63,0xE9},{0x73,0xE9},
//                               {0x83,0xE9}, {0x64,0xE9},{0x65,0xE9},{0x66,0xE9},{0x74,0xE9},{0x75,0xE9},{0x76,0xE9},
//                               {0x84,0xE9},{0x85,0xE9},{0x86,0xE9},{0x43,0xC4},{0x53,0xC4},{0x63,0xC4},{0x73,0xC4},{0x83,0xC4},{0x93,0xC4}};
//ABC电压　ABC电流 有功功率　无功功率
//总功率因数　ABC有功功率　ABC无功功率
//ABC功率因数 正向有功总电能　反向有功总电能　一二三四象限无功
uint8_t DLT645_97_item[24][2]={{0x44,0xE9},{0x45,0xE9},{0x46,0xE9},{0x54,0xE9},{0x55,0xE9},{0x56,0xE9},{0x63,0xE9},{0x73,0xE9},
                               {0x83,0xE9}, {0x64,0xE9},{0x65,0xE9},{0x66,0xE9},{0x74,0xE9},{0x75,0xE9},{0x76,0xE9},
                               {0x84,0xE9},{0x85,0xE9},{0x86,0xE9},{0x43,0xC3},{0x53,0xC3},{0x63,0xC4},{0x83,0xC4},{0x93,0xC4},{0x73,0xC4}};

dlt645_97::dlt645_97(Log *log)
{
	_strLogName = "ammeter97";
     g_ammeterHandle = -1;
     memset(Dlt645_buff,0x0,sizeof(Dlt645_buff));
     memset(recData,0x0,sizeof(recData));
     memset(targetData,0x0,sizeof(targetData));

     syncTimeFlag = false;
     pLog_dlt64597 = log;
      ammeterData = new FRAME_SUB_STATION_INFO;
}

dlt645_97::~dlt645_97()
{
    if(g_ammeterHandle != -1)
       close(g_ammeterHandle);

    delete ammeterData;
}

void dlt645_97::slot_getBoardType(int getBoardType)
{
   boardType = getBoardType;
}

bool dlt645_97::Init()//串口通信初始化
{
    struct termios tio;

    //临时屏蔽
    if(boardType == 1)//1.0硬件
    {
        g_ammeterHandle=open(AMMETER_SERIAL_NUM_1,O_RDWR|O_NOCTTY);
    }
    else if(boardType == 2)//2.0硬件
    {
        g_ammeterHandle=open(AMMETER_SERIAL_NUM_2,O_RDWR|O_NOCTTY);
    }
    else if(boardType == 3)//模块化集控硬件
    {
        g_ammeterHandle = open(AMMETER_SERIAL_NUM_3,O_RDWR|O_NOCTTY);
    }

    if(g_ammeterHandle<0)
    {
        return false;
    }
    else
    {
        cfsetispeed(&tio,B1200 );
        cfsetospeed(&tio,B1200);

        tio.c_cflag |= PARENB;
        tio.c_cflag &= ~PARODD;
        tio.c_cflag &= ~CSIZE;
        tio.c_cflag |= CS8;
        tio.c_cflag |= CLOCAL | CREAD;
        tio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
        tio.c_oflag &= ~OPOST;
        tio.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
        tio.c_iflag &= ~ (INLCR | ICRNL | IGNCR);
        tio.c_oflag &= ~(ONLCR | OCRNL);
        tio.c_cc[VTIME]=1;
        tio.c_cc[VMIN]=1;

        tcflush(g_ammeterHandle,TCIFLUSH);
        tcsetattr(g_ammeterHandle,TCSANOW,&tio);
        //fcntl(fileHandle,F_SETFL,0);//阻塞FNDELAY
        fcntl(g_ammeterHandle,F_SETFL,FNDELAY);//非阻塞FNDELAY

    }
    return true;
}

void dlt645_97::syncTime()
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
        //pLog_dlt64597->getLogPoint(_strLogName)->info("sync time with ammeter ");

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


uint8_t dlt645_97::verify(uint8_t *data,uint32_t length)//计算校验位
{
    uint32_t i;
    uint8_t result=0;
    for(i=0;i<length;i++)
    {
        result+=data[i];
    }
    return (result &0x00FF);
}


void dlt645_97::decodeRecData(uint8_t *targetData,int valueFlag, bool &errFlag, FRAME_SUB_STATION_INFO&ammeterData,stAmmeterConfig &info)
{
    switch(valueFlag)
    {
       case 0:
        case 1:
        case 2:
        parseVol(targetData, errFlag,ammeterData,valueFlag,info.voltageRatio);
        break;

    case 3:
     case 4:
     case 5:
        parseCur(targetData, errFlag,ammeterData,valueFlag,info.currentRatio);
        break;
    case 6:
        case 9:
        case 10:
        case 11:
        parsePower(targetData, errFlag,ammeterData,valueFlag,info.voltageRatio,info.currentRatio);
        break;
    case 7:
    case 12:
    case 13:
    case 14:
        parseRePower(targetData, errFlag,ammeterData,valueFlag,info.voltageRatio,info.currentRatio);
        break;
    case 8:
    case 15:
    case 16:
    case 17:
        parsePowerFactor(targetData, errFlag,ammeterData,valueFlag);
        break;
    case 18:
        parseActive_absorb_energy(targetData, errFlag,ammeterData,info.voltageRatio,info.currentRatio);
        break;
    case 19:
        parseActive_liberate_energy(targetData, errFlag,ammeterData,info.voltageRatio,info.currentRatio);
        break;
    case 20:
        parsereactive_sensibility_energy1(targetData, errFlag,info.voltageRatio,info.currentRatio);
        break;
    case 21:
        parsreactive_capacity_energy2(targetData, errFlag,info.voltageRatio,info.currentRatio);
        break;
    case 22:
        parsereactive_sensibility_energy3(targetData, errFlag,info.voltageRatio,info.currentRatio);
        break;
    case 23:
        parsereactive_capacity_energy4(targetData, errFlag,info.voltageRatio,info.currentRatio);
        break;
    }
}

void dlt645_97::parseVol(uint8_t *targetData, bool &errFlag,FRAME_SUB_STATION_INFO&ammeterData, int index,int &PT_value)
{
    short lowByte,highByte;
    int tmpValue;

    //电压
    lowByte = ((targetData[0]>>4) &0x0f)*10 +(targetData[0]&0x0f) ;
    highByte = (targetData[1]&0x0f)*100 ;
    if(!errFlag)
        tmpValue = (highByte+lowByte)*PT_value;
    else
        tmpValue = 0x7FFFFFFF;
    switch(index)
    {
       case 0:
        ammeterData.A_voltage = tmpValue;
        dataMap_645_97.insert(Addr_Vol_A_Term,QByteArray((char*)&ammeterData.A_voltage,4));
        pLog_dlt64597->getLogPoint(_strLogName)->info("Vol A:    "+QString::number(ammeterData.A_voltage));
        break;
    case 1:
        ammeterData.B_voltage = tmpValue;
        dataMap_645_97.insert(Addr_Vol_B_Term,QByteArray((char*)&ammeterData.B_voltage,4));
        pLog_dlt64597->getLogPoint(_strLogName)->info("Vol B:    "+QString::number(ammeterData.B_voltage));
        break;
    case 2:
        ammeterData.C_voltage = tmpValue;
        dataMap_645_97.insert(Addr_Vol_C_Term,QByteArray((char*)&ammeterData.C_voltage,4));
        pLog_dlt64597->getLogPoint(_strLogName)->info("Vol C:    "+QString::number(ammeterData.C_voltage));
        break;
    default:
        break;
    }

}

void dlt645_97::parseCur(uint8_t *targetData, bool &errFlag,FRAME_SUB_STATION_INFO&ammeterData,int index,int & CT_value)
{
   short lowByte,highByte;
   int tmpValue;

   lowByte =   (targetData[1]&0x0f)*100+((targetData[0]>>4) &0x0f)*10 +(targetData[0]&0x0f) ;
   highByte = ((targetData[1]>>4) &0x0f)*1000 ;
   if(!errFlag)
       tmpValue =(highByte+lowByte)*CT_value;
   else
       tmpValue = 0x7FFFFFFF;

   switch (index)
   {
   case 3:
       ammeterData.A_current = tmpValue/100;
       dataMap_645_97.insert(Addr_Cur_A_Term,QByteArray((char*)&ammeterData.A_current,4));
       pLog_dlt64597->getLogPoint(_strLogName)->info("Cur A:    "+QString::number(ammeterData.A_current));
       break;
   case 4:
       ammeterData.B_current = tmpValue/100;
       dataMap_645_97.insert(Addr_Cur_B_Term,QByteArray((char*)&ammeterData.B_current,4));
       pLog_dlt64597->getLogPoint(_strLogName)->info("Cur B:    "+QString::number(ammeterData.B_current));
       break;
   case 5:
       ammeterData.C_current = tmpValue/100;
       dataMap_645_97.insert(Addr_Cur_C_Term,QByteArray((char*)&ammeterData.C_current,4));
       pLog_dlt64597->getLogPoint(_strLogName)->info("Cur C:    "+QString::number(ammeterData.C_current));
       break;
   default:
       break;
   }

}


void dlt645_97::parsePower(uint8_t *targetData, bool &errFlag,FRAME_SUB_STATION_INFO&ammeterData,int index,int &PT_value,int &CT_value)
{
    short lowByte,highByte;
    int tmpValue;//activePowerA,activePowerB,activePowerC;

    lowByte = ((targetData[1]>>4) &0x0f)*1000 +(targetData[1]&0x0f)*100 + ((targetData[0]>>4) &0x0f)*10 +(targetData[0]&0x0f);
    highByte =  ((targetData[2]>>4) &0x0f)*10 +(targetData[2]&0x0f) ;
    if(!errFlag)
        tmpValue = (highByte*10000+lowByte)*CT_value*PT_value;
    else
        tmpValue = 0x7FFFFFFF;
    switch(index)
    {
    case 6:
        ammeterData.active_power = tmpValue/10000;
        pLog_dlt64597->getLogPoint(_strLogName)->info("SUM Power:    "+QString::number(tmpValue));
        dataMap_645_97.insert(Addr_Power_Term,QByteArray((char*)&ammeterData.active_power,4));
        break;
    case 9:
        ammeterData.active_powerA = tmpValue/10000;
        pLog_dlt64597->getLogPoint(_strLogName)->info("SUM Power:    "+QString::number(tmpValue));
        dataMap_645_97.insert(Addr_Power_A_Term,QByteArray((char*)&ammeterData.active_powerA,4));
        break;
    case 10:
        ammeterData.active_powerB = tmpValue/10000;
        pLog_dlt64597->getLogPoint(_strLogName)->info("SUM Power:    "+QString::number(tmpValue));
        dataMap_645_97.insert(Addr_Power_B_Term,QByteArray((char*)&ammeterData.active_powerB,4));
        break;
    case 11:
        ammeterData.active_powerC = tmpValue/10000;
        pLog_dlt64597->getLogPoint(_strLogName)->info("SUM Power:    "+QString::number(tmpValue));
        dataMap_645_97.insert(Addr_Power_C_Term,QByteArray((char*)&ammeterData.active_powerC,4));
        break;
    default:
        break;
    }
}

void dlt645_97::parseRePower(uint8_t *targetData, bool &errFlag,FRAME_SUB_STATION_INFO&ammeterData,int index,int &PT_value,int &CT_value)
{
    short lowByte;
    int tmpValue;//reactivePowerA,reactivePowerB,reactivePowerC;

    lowByte = ((targetData[1]>>4) &0x0f)*1000 +(targetData[1]&0x0f)*100 + ((targetData[0]>>4) &0x0f)*10 +(targetData[0]&0x0f);
    //highByte =  ((targetData[2]>>4) &0x0f)*10 +(targetData[2]&0x0f) ;
    if(!errFlag)
        tmpValue = (lowByte)*CT_value*PT_value;
    else
        tmpValue = 0x7FFFFFFF;

    switch(index)
    {
       case 7:
        ammeterData.reactive_power = tmpValue/100;
        dataMap_645_97.insert(Addr_rePower_Term,QByteArray((char*)&ammeterData.reactive_power,4));
         pLog_dlt64597->getLogPoint(_strLogName)->info("SUM re-Power :    "+QString::number(tmpValue));
        break;
    case 12:
        ammeterData.reactive_powerA = tmpValue/100;
        dataMap_645_97.insert(Addr_rePower_A_Term,QByteArray((char*)&ammeterData.reactive_powerA,4));
        pLog_dlt64597->getLogPoint(_strLogName)->info("re-Power A:    "+QString::number(tmpValue));
        break;
    case 13:
        ammeterData.reactive_powerB = tmpValue/100;
        dataMap_645_97.insert(Addr_rePower_B_Term,QByteArray((char*)&ammeterData.reactive_powerB,4));
        pLog_dlt64597->getLogPoint(_strLogName)->info("re-Power B:    "+QString::number(tmpValue));
        break;
    case 14:
        ammeterData.reactive_powerC = tmpValue/100;
        dataMap_645_97.insert(Addr_rePower_C_Term,QByteArray((char*)&ammeterData.reactive_powerC,4));
        pLog_dlt64597->getLogPoint(_strLogName)->info("re-Power C:    "+QString::number(tmpValue));
        break;
    default:
        break;
    }

}


void dlt645_97::parsePowerFactor(uint8_t *targetData, bool &errFlag,FRAME_SUB_STATION_INFO&ammeterData,int index)
{
    short lowByte,highByte;
    int tmpValue;//PowerFactorA,PowerFactorB,PowerFactorC;

    lowByte =  (targetData[1]&0x0f)*100+ ((targetData[0]>>4) &0x0f)*10 +(targetData[0]&0x0f);
    highByte =  (targetData[1]>>4) &0x0f;
    if(!errFlag)
        tmpValue=highByte*1000+lowByte;
    else
        tmpValue = 0x7FFFFFFF;

    switch(index)
    {
      case 8:
        ammeterData.power_factor = tmpValue/1000;
        dataMap_645_97.insert(Addr_PowerFactor_Term,QByteArray((char*)&ammeterData.power_factor,4));
        pLog_dlt64597->getLogPoint(_strLogName)->info("SUM Power factor :    "+QString::number(tmpValue));
        break;
    case 15:
        ammeterData.power_factorA = tmpValue/1000;
        dataMap_645_97.insert(Addr_PowerFactor_A_Term,QByteArray((char*)&ammeterData.power_factorA,4));
        pLog_dlt64597->getLogPoint(_strLogName)->info("SUM Power factor :    "+QString::number(tmpValue));
        break;
    case 16:
        ammeterData.power_factorB = tmpValue/1000;
        dataMap_645_97.insert(Addr_PowerFactor_B_Term,QByteArray((char*)&ammeterData.power_factorB,4));
        pLog_dlt64597->getLogPoint(_strLogName)->info("SUM Power factor :    "+QString::number(tmpValue));
        break;
    case 17:
        ammeterData.power_factorC = tmpValue/1000;
        dataMap_645_97.insert(Addr_PowerFactor_C_Term,QByteArray((char*)&ammeterData.power_factorC,4));
        pLog_dlt64597->getLogPoint(_strLogName)->info("SUM Power factor :    "+QString::number(tmpValue));
        break;
    default:
        break;
    }

}

void dlt645_97::parseActive_absorb_energy(uint8_t *targetData,bool & errFlag,FRAME_SUB_STATION_INFO&ammeterData,int &PT_value,int &CT_value)
{
    int lowByte,highByte;
    int elePower;

    lowByte =   ((targetData[1]>>4) &0x0f)*1000+(targetData[1]&0x0f)*100+((targetData[0]>>4) &0x0f)*10 +(targetData[0]&0x0f) ;
    highByte =  ((targetData[3]>>4) &0x0f)*10000000 +(targetData[3]&0x0f)*1000000+((targetData[2]>>4) &0x0f)*100000+ (targetData[2]&0x0f)*10000;
    if(!errFlag)
        elePower = (highByte+lowByte)*CT_value*PT_value;
    else
        elePower = 0x7FFFFFFF;

    pLog_dlt64597->getLogPoint(_strLogName)->info("SUM Energy :    "+QString::number(elePower));
    ammeterData.active_absorb_energy = elePower;
    dataMap_645_97.insert(Addr_active_absorb_energy_Term,QByteArray((char*)&ammeterData.active_absorb_energy,4));
}

void dlt645_97::parseActive_liberate_energy(uint8_t *targetData,bool & errFlag,FRAME_SUB_STATION_INFO&ammeterData,int &PT_value,int &CT_value)
{
    int lowByte,highByte;
    int elePower;

    lowByte =   ((targetData[1]>>4) &0x0f)*1000+(targetData[1]&0x0f)*100+((targetData[0]>>4) &0x0f)*10 +(targetData[0]&0x0f) ;
    highByte =  ((targetData[3]>>4) &0x0f)*10000000 +(targetData[3]&0x0f)*1000000+((targetData[2]>>4) &0x0f)*100000+ (targetData[2]&0x0f)*10000;
    if(!errFlag)
        elePower = (highByte+lowByte)*CT_value*PT_value;
    else
        elePower = 0x7FFFFFFF;

    pLog_dlt64597->getLogPoint(_strLogName)->info("re SUM Energy :    "+QString::number(elePower));
    ammeterData.active_liberate_energy = elePower;
    dataMap_645_97.insert(Addr_active_liberate_energy_Term,QByteArray((char*)&ammeterData.active_liberate_energy,4));
}

void dlt645_97::parsereactive_sensibility_energy1(uint8_t *targetData,bool & errFlag,int &PT_value,int &CT_value)
{
    int lowByte,highByte;
    int elePower;

    lowByte =   ((targetData[1]>>4) &0x0f)*1000+(targetData[1]&0x0f)*100+((targetData[0]>>4) &0x0f)*10 +(targetData[0]&0x0f) ;
    highByte =  ((targetData[3]>>4) &0x0f)*10000000 +(targetData[3]&0x0f)*1000000+((targetData[2]>>4) &0x0f)*100000+ (targetData[2]&0x0f)*10000;

    if(!errFlag)
       elePower = (highByte+lowByte)*CT_value*PT_value;
    else
        elePower = 0;

    if(elePower != 0)
        ammeterData->reactive_sensibility_energy = elePower;
    else
        ammeterData->reactive_sensibility_energy = 0;

}


void dlt645_97::parsereactive_sensibility_energy3(uint8_t *targetData, bool & errFlag,int &PT_value,int &CT_value)
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
    pLog_dlt64597->getLogPoint(_strLogName)->info("gan SUM Energy :    "+QString::number(ammeterData->reactive_sensibility_energy));
    dataMap_645_97.insert(Addr_reactive_sensibility_energy_Term,QByteArray((char*)&ammeterData->reactive_sensibility_energy,4));
}


void dlt645_97::parsreactive_capacity_energy2(uint8_t *targetData, bool & errFlag,int &PT_value,int &CT_value)
{
    int lowByte,highByte;
    int elePower;

    lowByte =   ((targetData[1]>>4) &0x0f)*1000+(targetData[1]&0x0f)*100+((targetData[0]>>4) &0x0f)*10 +(targetData[0]&0x0f) ;
    highByte =  ((targetData[3]>>4) &0x0f)*10000000 +(targetData[3]&0x0f)*1000000+((targetData[2]>>4) &0x0f)*100000+ (targetData[2]&0x0f)*10000;

        if(!errFlag)
             elePower = (highByte+lowByte)*CT_value*PT_value;
        else
            elePower = 0;

        if(elePower != 0)
             ammeterData->reactive_capacity_energy = elePower;
        else
            ammeterData->reactive_capacity_energy = 0;
}


void dlt645_97::parsereactive_capacity_energy4(uint8_t *targetData, bool & errFlag,int &PT_value,int &CT_value)
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
    pLog_dlt64597->getLogPoint(_strLogName)->info("rong SUM Energy :    "+QString::number(ammeterData->reactive_capacity_energy));
    dataMap_645_97.insert(Addr_reactive_capacity_energy_Term,QByteArray((char*)&ammeterData->reactive_capacity_energy,4));
}


bool dlt645_97::checkRecData(int getBytes, uint8_t * recData,uint8_t *targetData,bool & errFlag, unsigned char *addr)
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
                    pLog_dlt64597->getLogPoint(_strLogName)->info("Data not form target ammeter !!!! ");
                    return false;
                }

                if(recData[index+8]!=0xC1)
                {
                    startIndex=recData[index+9]-2;
                    //pLog_dlt64597->getLogPoint(_strLogName)->info("############################################ ");
                    for (int i=0;i<startIndex;i++)//获取数据域(不包括数据标识部分)
                    {
                        targetData[i]=recData[index+12+i]- OFFSET;
                    }
                    pLog_dlt64597->getLogPoint(_strLogName)->info("Get value ==: "+ConvertHex2Qstr((unsigned char *)targetData, startIndex));
                }
                else//异常应答
                {
                    errFlag = true;
                    pLog_dlt64597->getLogPoint(_strLogName)->info("Get fault response !!!!!!!!!!!!!!!!!!!!!! ");
                }
                break;       //找到数据，退出循环
            }
        }
        if(index == getBytes-7)//未读到合法数据
        {
            errFlag = true;
            pLog_dlt64597->getLogPoint(_strLogName)->info("Get wrong format data !!!!!!!!!!!!!!!!!!!!! ");
            pLog_dlt64597->getLogPoint(_strLogName)->info(ConvertHex2Qstr((unsigned char *)recData, getBytes));
        }

            return true;
    }
    else//未读到数据
    {
        pLog_dlt64597->getLogPoint(_strLogName)->info("NO get data !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ");
        return false;
    }
}

 void dlt645_97::fixReadCMD(FRAME_DLT_645  &frame_dlt_645,unsigned char * addr,uint8_t * Dlt645_buff,uint8_t * cmdFrame)
 {

     frame_dlt_645.frame_start = FRAME_START;//帧起始符
     memcpy(frame_dlt_645.addr,addr,6);

     frame_dlt_645.frame_start2 = FRAME_START;//帧起始符
     frame_dlt_645.controlCode =0x01;//控制码(读数据)
     frame_dlt_645.data_length = 0x02;//数据域长度(仅含数据标识，长度固定为4字节)

     memcpy(Dlt645_buff,(unsigned char *)&frame_dlt_645,10);
     memcpy(&Dlt645_buff[10],cmdFrame,frame_dlt_645.data_length);//数据域(仅含数据标识)
     Dlt645_buff[frame_dlt_645.data_length+10] = verify(Dlt645_buff,frame_dlt_645.data_length+10);//校验码
     Dlt645_buff[frame_dlt_645.data_length+11] = FRAME_END;//结束符
 }


 void dlt645_97::readAmmeterData_97(stAmmeterConfig info)
 {
     FRAME_DLT_645 frame_dlt_645;
     FRAME_SUB_STATION_INFO ammeterData;
     bool errFlag = false;
     uint8_t Dlt645_buff[200];
     uint8_t recData[200];
     uint8_t targetData[200];
     memset(Dlt645_buff,0x0,sizeof(Dlt645_buff));
     memset(recData,0x0,sizeof(recData));
     memset(targetData,0x0,sizeof(targetData));

     pLog_dlt64597->getLogPoint(_strLogName)->info("#####################################################");
     pLog_dlt64597->getLogPoint(_strLogName)->info("Start to read ammeter ==: "+ConvertHex2Qstr((unsigned char *)info.addr, 6));


     for(int i=0;i<4;i++)
     {
         Dlt645_buff[i] =0xFE;
     }
     write(g_ammeterHandle,Dlt645_buff,4);//发送四个前导字节，唤醒接收方

     for(int index = 0; index<24;index++)
     {
         int getBytes = 0;

         fixReadCMD(frame_dlt_645,info.addr,DLT645_97_item[index],Dlt645_buff);

         write(g_ammeterHandle,Dlt645_buff,16);
         usleep(500000);

         getBytes = read(g_ammeterHandle,recData, 212);

         if(checkRecData(getBytes,recData,targetData,errFlag,info.addr))
         {
             decodeRecData(targetData,index, errFlag,ammeterData,info);
         }
     }

     dataMap_645_97.insert(Addr_Ammeter_Type,QByteArray('1',1));
     dataMap_645_97.insert(Addr_Ammeter_ID,QByteArray((const char *)info.addr,6));

     emit sendAmmeterData_645_97(dataMap_645_97);
     dataMap_645_97.clear();
 }


 void dlt645_97::slot_readAmmeter_dlt645_97(QList<stAmmeterConfig> infoList)
 {
     if(!Init())//串口初始化
     {
         return;
     }

     for(int i=0;i<infoList.length();i++)
     {
         readAmmeterData_97(infoList.at(i));
     }
     emit sigReadOver_645_97(1);//97协议电表

     close(g_ammeterHandle);
     //syncTime();//电表对时
 }

