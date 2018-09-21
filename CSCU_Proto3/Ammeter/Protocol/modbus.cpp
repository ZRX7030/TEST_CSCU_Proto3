#include "modbus.h"
#include <QDebug>
#include <qcoreapplication.h>




modbus::modbus(Log * log)
{
	_strLogName = "ammeter_modbus";
    //exitFlag = false;
     g_ammeterHandle = -1;
     memset(Dlt645_buff,0x0,sizeof(Dlt645_buff));
     memset(recData,0x0,sizeof(recData));
     memset(targetData,0x0,sizeof(targetData));

     syncTimeFlag = false;
     getDataFlag = true;
     tmpAddr = 0;

     pLog_modbus = log;
     ammeterData = new FRAME_SUB_STATION_INFO;
}

modbus::~modbus()
{
    if(g_ammeterHandle != -1)
       close(g_ammeterHandle);

    delete ammeterData;
}

void modbus::slot_getBoardType(int getBoardType)
{
   boardType = getBoardType;
}

bool modbus::Init()//串口通信初始化,默认波特率4800
{
    struct termios tio;

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

    if(g_ammeterHandle <= 0)
    {
        return false;
    }
    else
    {
        cfsetispeed(&tio,B4800);
        cfsetospeed(&tio,B4800);

        //无校验
        tio.c_cflag &= ~PARENB;
        tio.c_cflag &= ~CSTOPB;
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
        //fcntl(g_ModbusHandle,F_SETFL,0);//阻塞
        fcntl(g_ammeterHandle,F_SETFL,FNDELAY);//非阻塞

    }
    return true;
}

//int modbus::Verify_CRC16(unsigned char* pchMsg, int wDataLen)
//{
//    char chCRCHi = 0xFF; // 高CRC字节初始化
//    char chCRCLo = 0xFF; // 低CRC字节初始化
//    short wIndex;            // CRC循环中的索引
//    while (wDataLen--)
//    {
//        // 计算CRC
//        wIndex = chCRCLo ^ *pchMsg++ ;
//        chCRCLo = chCRCHi ^ chCRCHTalbe[wIndex];
//        chCRCHi = chCRCLTalbe[wIndex] ;
//    }
//    return ((chCRCHi << 8) | chCRCLo) ;
//}

 void modbus::read_modbus_register(short registerAddr,int length)
 {
     unsigned char sendData[10];
     memset(sendData,0x0,10);
     int iOffset = 0;
     int crc16 = 0;

//     sendData[0] = MODBUS_AMMETER_DEFAULT_ADDRESS;
     sendData[0] = tmpAddr;
     iOffset ++;

     sendData[1] = MODBUS_READ_DATA;
     iOffset ++;

     //寄存器地址
     sendData[2] = registerAddr / 256;//
     sendData[3] = registerAddr & 0xFF;
     iOffset = iOffset +2;

     //length
     sendData[4] = length / 256;
     sendData[5] = length & 0xFF;
     iOffset = iOffset +2;

     crc16 = Verify_CRC16(sendData,iOffset);
     sendData[6] = crc16 & 0xFF;
     sendData[7] = crc16 >> 8;
     iOffset = iOffset +2;

//     for(int i=0;i<iOffset;i++){
//     }

     pLog_modbus->getLogPoint(_strLogName)->info("send CMD == "+ConvertHex2Qstr((unsigned char *)sendData, iOffset));

     Modbus_Write(sendData,iOffset);
 }

 int modbus::Modbus_Read(unsigned char *recData)
 {
     int nbytes = -1;

     if(recData != NULL)
     {
         nbytes = read(g_ammeterHandle,recData, 100);
     }
     for(int i=0;i<nbytes;i++)
     pLog_modbus->getLogPoint(_strLogName)->info("receice result count == "+QString::number(nbytes));
     return nbytes;
 }

 int modbus::Modbus_Write(unsigned char *sendData,int num)
 {
     int nbytes = -1;

     if(sendData != NULL)
         nbytes = write(g_ammeterHandle,sendData, num);

     return nbytes;
 }

 void modbus::getVoltage_Current()
 {
     int A_voltage,B_voltage,C_voltage,A_current,B_current,C_current;

     unsigned char m_modbusRecvBuf[100];
     int crc16,crc16_rec = 0;
     int length = 0;
     unsigned short DPT,DCT;
     DPT=DCT=1;

     //读取DPT/DCT，计算电压电流用
     read_modbus_register(0x0023,1);//

     usleep(300000);
     getDataFlag=false;
     if((length = Modbus_Read(m_modbusRecvBuf))>2)
     {
         getDataFlag=true;
         //CRC校验
         crc16 = Verify_CRC16(m_modbusRecvBuf,length-2);
         crc16_rec = (m_modbusRecvBuf[length-1] * 256) + m_modbusRecvBuf[length-2];
         if(crc16 != crc16_rec){
             pLog_modbus->getLogPoint(_strLogName)->error("Get voltage ------- modbus data crc error!");
             return ;
         }
         DPT = m_modbusRecvBuf[3];
         DCT = m_modbusRecvBuf[4];
     }

     //读取电压电流
     read_modbus_register(0x0025,9);//
     usleep(900000);
     getDataFlag=false;
     if((length = Modbus_Read(m_modbusRecvBuf))>2)
     {
         getDataFlag=true;
         //CRC校验
         crc16 = Verify_CRC16(m_modbusRecvBuf,length-2);
         crc16_rec = (m_modbusRecvBuf[length-1] * 256) + m_modbusRecvBuf[length-2];
         if(crc16 != crc16_rec){

             pLog_modbus->getLogPoint(_strLogName)->error("Get voltage ------- modbus data crc error!");
             return ;
         }

         //m_logger_modbus->info("Modbus_Read m_StationStatus.A_voltage " + ConvertHex2Qstr(m_modbusRecvBuf,length));
          A_voltage =  ((m_modbusRecvBuf[3]*256+m_modbusRecvBuf[4]) * pow(10,DPT-4))*10;
         B_voltage =  ((m_modbusRecvBuf[5]*256+m_modbusRecvBuf[6]) * pow(10,DPT-4))*10;
         C_voltage =  ((m_modbusRecvBuf[7]*256+m_modbusRecvBuf[8]) * pow(10,DPT-4))*10;
         A_current = ((m_modbusRecvBuf[15]*256+m_modbusRecvBuf[16])*pow(10,DCT-4))*100;
         B_current = ((m_modbusRecvBuf[17]*256+m_modbusRecvBuf[18])*pow(10,DCT-4))*100;
         C_current = ((m_modbusRecvBuf[19]*256+m_modbusRecvBuf[20])*pow(10,DCT-4))*100;

         pLog_modbus->getLogPoint(_strLogName)->info("Vol A:    "+QString::number(A_voltage));
         pLog_modbus->getLogPoint(_strLogName)->info("Vol B:    "+QString::number(B_voltage));
         pLog_modbus->getLogPoint(_strLogName)->info("Vol C:    "+QString::number(C_voltage));
         pLog_modbus->getLogPoint(_strLogName)->info("Cur A:    "+QString::number(A_current));
         pLog_modbus->getLogPoint(_strLogName)->info("Cur B:    "+QString::number(B_current));
         pLog_modbus->getLogPoint(_strLogName)->info("Cur C:    "+QString::number(C_current));

     if(A_voltage != 0)
         ammeterData->A_voltage = (float)A_voltage/10;
     if(B_voltage != 0)
         ammeterData->B_voltage = B_voltage/10;
     if(C_voltage != 0)
         ammeterData->C_voltage = C_voltage/10;
     if(A_current != 0)
         ammeterData->A_current = A_current/10;
     if(B_current != 0)
         ammeterData->B_current = B_current/10;
     if(C_current != 0)
         ammeterData->C_current = C_current/10;
         dataMap_modbus.insert(Addr_Vol_A_Term,QByteArray((char*)&ammeterData->A_voltage,4));
         dataMap_modbus.insert(Addr_Vol_B_Term,QByteArray((char*)&ammeterData->B_voltage,4));
         dataMap_modbus.insert(Addr_Vol_C_Term,QByteArray((char*)&ammeterData->C_voltage,4));
         dataMap_modbus.insert(Addr_Cur_A_Term,QByteArray((char*)&ammeterData->A_current,4));
         dataMap_modbus.insert(Addr_Cur_B_Term,QByteArray((char*)&ammeterData->B_current,4));
         dataMap_modbus.insert(Addr_Cur_C_Term,QByteArray((char*)&ammeterData->C_current,4));

     }

 }


 void modbus::getActive_reactivePower()
 {
    unsigned int active_power,active_powerA,active_powerB,active_powerC;
    unsigned int reactive_power,reactive_powerA,reactive_powerB,reactive_powerC;
    unsigned int power_factor,power_factorA,power_factorB,power_factorC;//总有功功率

     unsigned char m_modbusRecvBuf[100];
     int crc16,crc16_rec = 0;
     int length = 0;
     unsigned short DPQ, DPQ1;
     DPQ = DPQ1 = 0;

     //DPQ
     read_modbus_register(0x0024,1);
     usleep(300000);
     getDataFlag=false;
     if((length = Modbus_Read(m_modbusRecvBuf))>2)
     {
         getDataFlag=true;
         crc16 = Verify_CRC16(m_modbusRecvBuf,length-2);
         crc16_rec = ( m_modbusRecvBuf[length-1] *256) + m_modbusRecvBuf[length-2];

         if(crc16 != crc16_rec){
             return ;
         }
         //memcpy(&gstr_pcnetThread->m_StationStatus.active_power,&m_modbusRecvBuf[3],2);
         DPQ = m_modbusRecvBuf[3];
         DPQ1 = m_modbusRecvBuf[4];//高位－低位:Q/Qc/Qb/Qa/P/Pc/Pb/Pa　０为正　１为负

     }

     //有功功率  无功功率　功率因数
     read_modbus_register(0x002E,12);
     usleep(1200000);
     getDataFlag=false;
     if((length = Modbus_Read(m_modbusRecvBuf))>2)
     {
         getDataFlag=true;
         crc16 = Verify_CRC16(m_modbusRecvBuf,length-2);
         crc16_rec = ( m_modbusRecvBuf[length-1] *256) + m_modbusRecvBuf[length-2];
         if(crc16 != crc16_rec){
             return ;
         }
         //memcpy(&gstr_pcnetThread->m_StationStatus.active_power,&m_modbusRecvBuf[3],2);
         active_powerA = m_modbusRecvBuf[3]*256+m_modbusRecvBuf[4];

         active_powerA *=pow(10,DPQ-4);

         if(DPQ1&0x01)
              active_powerA=- active_powerA;
          active_powerB = m_modbusRecvBuf[5]*256+m_modbusRecvBuf[6];
          active_powerB *=pow(10,DPQ-4);
         if(DPQ1&0x02)
              active_powerB=- active_powerB;
          active_powerC = m_modbusRecvBuf[7]*256+m_modbusRecvBuf[8];
          active_powerC *=pow(10,DPQ-4);
         if(DPQ1&0x04)
              active_powerC=- active_powerC;
          active_power = m_modbusRecvBuf[9]*256+m_modbusRecvBuf[10];
          active_power *=pow(10,DPQ-4);
         if(DPQ1&0x08)
              active_power=- active_power;


          active_power *=10;//计算值为W



          reactive_powerA = m_modbusRecvBuf[11]*256+m_modbusRecvBuf[12];
          reactive_powerA *=pow(10,DPQ-4);
         if(DPQ1&0x10)
              reactive_powerA=- reactive_powerA;
          reactive_powerB = m_modbusRecvBuf[13]*256+m_modbusRecvBuf[14];
          reactive_powerB *=pow(10,DPQ-4);
         if(DPQ1&0x20)
              reactive_powerB=- reactive_powerB;
          reactive_powerC = m_modbusRecvBuf[15]*256+m_modbusRecvBuf[16];
          reactive_powerC *=pow(10,DPQ-4);
         if(DPQ1&0x40)
              reactive_powerC=- reactive_powerC;
          reactive_power = m_modbusRecvBuf[17]*256+m_modbusRecvBuf[18];
          reactive_power *=pow(10,DPQ-4);
         if(DPQ1&0x80)
              reactive_power=- reactive_power;

         reactive_power *=10;


          power_factorA = m_modbusRecvBuf[19]*256+m_modbusRecvBuf[20];
          power_factorB = m_modbusRecvBuf[21]*256+m_modbusRecvBuf[22];
          power_factorC = m_modbusRecvBuf[23]*256+m_modbusRecvBuf[24];
          power_factor = m_modbusRecvBuf[25]*256+m_modbusRecvBuf[26];

         //零线电流/电压不平衡率/电流不平衡率，协议没有采集　默认为０

         active_powerA *=10;
         active_powerB *=10;
         active_powerC *=10;
         reactive_powerA *=10;
         reactive_powerB *=10;
         reactive_powerC *=10;

         ammeterData->active_power = active_power/100;
         ammeterData->active_powerA = active_powerA/100;
         ammeterData->active_powerB = active_powerB/100;
         ammeterData->active_powerC = active_powerC/100;
         ammeterData->reactive_power = reactive_power/100;
         ammeterData->reactive_powerA = reactive_powerA/100;
         ammeterData->reactive_powerB = reactive_powerB/100;
         ammeterData->reactive_powerC = reactive_powerC/100;
         ammeterData->power_factor = power_factor/1000;
         ammeterData->power_factorA = power_factorA/1000;
         ammeterData->power_factorB = power_factorB/1000;
         ammeterData->power_factorC = power_factorC/1000;

         dataMap_modbus.insert(Addr_Power_Term,QByteArray((char*)&ammeterData->A_voltage,4));
         dataMap_modbus.insert(Addr_Power_A_Term,QByteArray((char*)&ammeterData->A_voltage,4));
         dataMap_modbus.insert(Addr_Vol_B_Term,QByteArray((char*)&ammeterData->B_voltage,4));
         dataMap_modbus.insert(Addr_Vol_C_Term,QByteArray((char*)&ammeterData->C_voltage,4));
         dataMap_modbus.insert(Addr_Cur_A_Term,QByteArray((char*)&ammeterData->A_current,4));
         dataMap_modbus.insert(Addr_Cur_B_Term,QByteArray((char*)&ammeterData->B_current,4));
         dataMap_modbus.insert(Addr_Cur_C_Term,QByteArray((char*)&ammeterData->C_current,4));
         pLog_modbus->getLogPoint(_strLogName)->info("Power A:    "+QString::number(active_powerA));
         pLog_modbus->getLogPoint(_strLogName)->info("Power B:    "+QString::number(active_powerB));
         pLog_modbus->getLogPoint(_strLogName)->info("Power C:    "+QString::number(active_powerC));
         pLog_modbus->getLogPoint(_strLogName)->info("SUM Power:    "+QString::number(active_power));
         pLog_modbus->getLogPoint(_strLogName)->info("rePower A:    "+QString::number(reactive_powerA));
         pLog_modbus->getLogPoint(_strLogName)->info("rePower B:    "+QString::number(reactive_powerB));
         pLog_modbus->getLogPoint(_strLogName)->info("rePower C:    "+QString::number(reactive_powerC));
         pLog_modbus->getLogPoint(_strLogName)->info("re SUM Power:    "+QString::number(reactive_power));
         pLog_modbus->getLogPoint(_strLogName)->info("Power factor A:    "+QString::number(power_factorA));
         pLog_modbus->getLogPoint(_strLogName)->info("Power factor B:    "+QString::number(power_factorB));
         pLog_modbus->getLogPoint(_strLogName)->info("Power factor C:    "+QString::number(power_factorC));
         pLog_modbus->getLogPoint(_strLogName)->info("SUM Power factor:    "+QString::number(power_factor));
     }

 }

 void modbus::getEnergy()
 {
     unsigned char m_modbusRecvBuf[100];
     int crc16,crc16_rec = 0;
     int length = 0;
     unsigned int absorbEnergy,liberateEnergy,capacityEnergy,sensibilityEnergy;
     absorbEnergy=liberateEnergy=capacityEnergy=sensibilityEnergy=0;

     read_modbus_register(0x003F,8);
     usleep(1200000);
     getDataFlag=false;
     if((length = Modbus_Read(m_modbusRecvBuf))>2)
     {
         getDataFlag=true;
         crc16 = Verify_CRC16(m_modbusRecvBuf,length-2);
         crc16_rec = ( m_modbusRecvBuf[length-1] *256) + m_modbusRecvBuf[length-2];
         if(crc16 != crc16_rec){
             return ;
         }

         unsigned short us_ernergy_high = 0;
         unsigned short us_ernergy_low = 0;

         //吸收有功电能
         us_ernergy_high = m_modbusRecvBuf[3]*256+m_modbusRecvBuf[4];
         us_ernergy_low = m_modbusRecvBuf[5]*256+m_modbusRecvBuf[6];
         absorbEnergy = us_ernergy_high*65536+us_ernergy_low;

         //释放有功电能
         us_ernergy_high = m_modbusRecvBuf[7]*256+m_modbusRecvBuf[8];
         us_ernergy_low = m_modbusRecvBuf[9]*256+m_modbusRecvBuf[10];
         liberateEnergy = us_ernergy_high*65536+us_ernergy_low;

         //感性无功电能
         us_ernergy_high = m_modbusRecvBuf[11]*256+m_modbusRecvBuf[12];
         us_ernergy_low = m_modbusRecvBuf[13]*256+m_modbusRecvBuf[14];
          sensibilityEnergy= us_ernergy_high*65536+us_ernergy_low;

         //容性无功电能
         us_ernergy_high = m_modbusRecvBuf[15]*256+m_modbusRecvBuf[16];
         us_ernergy_low = m_modbusRecvBuf[17]*256+m_modbusRecvBuf[18];
          capacityEnergy= us_ernergy_high*65536+us_ernergy_low;
     }

     read_modbus_register(0x0003,1);//PT变比
     usleep(300000);
     getDataFlag=false;
     unsigned short PT_value = 0;
     if((length = Modbus_Read(m_modbusRecvBuf))>2)
     {
         getDataFlag=true;
         crc16 = Verify_CRC16(m_modbusRecvBuf,length-2);
         crc16_rec = ( m_modbusRecvBuf[length-1] *256) + m_modbusRecvBuf[length-2];
         if(crc16 != crc16_rec){
             return ;
         }
         PT_value=m_modbusRecvBuf[3]*256+m_modbusRecvBuf[4];
     }

     read_modbus_register(0x0004,1);//CT变比
     usleep(300000);
     getDataFlag=false;
     unsigned short CT_value = 0;
     if((length = Modbus_Read(m_modbusRecvBuf))>2)
     {
         getDataFlag=true;
         crc16 = Verify_CRC16(m_modbusRecvBuf,length-2);
         crc16_rec = ( m_modbusRecvBuf[length-1] *256) + m_modbusRecvBuf[length-2];
         if(crc16 != crc16_rec){
             return ;
         }

         CT_value=m_modbusRecvBuf[3]*256+m_modbusRecvBuf[4];
     }

     //PT及CT值由配置文件获取
     //    CT_value=gstr_paramSet->CT;
     //    PT_value=gstr_paramSet->PT;

     unsigned int tmpTime = PT_value*CT_value/1000;//避免出现计算过程中到溢出
     absorbEnergy= absorbEnergy*tmpTime*100;
     liberateEnergy=liberateEnergy *tmpTime*100;

     sensibilityEnergy=sensibilityEnergy * tmpTime*100;
     capacityEnergy=capacityEnergy * tmpTime*100;


     pLog_modbus->getLogPoint(_strLogName)->info("SUM absorbEnergy :    "+QString::number(absorbEnergy));
     pLog_modbus->getLogPoint(_strLogName)->info("SUM liberateEnergy :    "+QString::number(liberateEnergy));
     pLog_modbus->getLogPoint(_strLogName)->info("SUM sensibilityEnergy :    "+QString::number(sensibilityEnergy));
     pLog_modbus->getLogPoint(_strLogName)->info("SUM capacityEnergy :    "+QString::number(capacityEnergy));
     ammeterData->active_absorb_energy = absorbEnergy;
     ammeterData->active_liberate_energy = liberateEnergy;
     ammeterData->reactive_sensibility_energy = sensibilityEnergy;
     ammeterData->reactive_capacity_energy = capacityEnergy;

     dataMap_modbus.insert(Addr_active_absorb_energy_Term,QByteArray((char*)&ammeterData->active_absorb_energy,4));
     dataMap_modbus.insert(Addr_active_liberate_energy_Term,QByteArray((char*)&ammeterData->active_liberate_energy,4));
     dataMap_modbus.insert(Addr_reactive_sensibility_energy_Term,QByteArray((char*)&ammeterData->reactive_sensibility_energy,4));
     dataMap_modbus.insert(Addr_reactive_capacity_energy_Term,QByteArray((char*)&ammeterData->reactive_capacity_energy,4));

 }

 void modbus::readAmmeterData_modbus(stAmmeterConfig info)
 {
     pLog_modbus->getLogPoint(_strLogName)->info("#####################################################");
     pLog_modbus->getLogPoint(_strLogName)->info("Start to read ammeter ==: "+ConvertHex2Qstr((unsigned char *)info.addr, 6));

     tmpAddr = info.addr[0];//modbus电表规定电表地址从１开始排

     getVoltage_Current();
     getActive_reactivePower();
     getEnergy();

     dataMap_modbus.insert(Addr_Ammeter_Type,QByteArray('3',1));
     dataMap_modbus.insert(Addr_Ammeter_ID,QByteArray((const char *)info.addr,6));

     emit sendAmmeterData_modbus(dataMap_modbus);
     dataMap_modbus.clear();
 }


 void modbus::slot_readAmmeter_modbus(QList<stAmmeterConfig> infoList)
 {
     if(!Init())
     {
         return;
     }

     for(int i=0;i<infoList.length();i++)
     {
         readAmmeterData_modbus(infoList.at(i));
     }
     emit sigReadOver_modbus(3);//modbus协议电表

     close(g_ammeterHandle);
    // syncTime();//电表对时
 }

