#include <stdio.h>
#include <stdlib.h>
#include "Card/Card.h"
#include "CommonFunc/commfunc.h"

unsigned char sWriteCpuCardNum[7] = {0x02,0x00,0x02,0x51,0x56,0x03,0x04};
//02 00 09 34 32 03 40 01 30 00 01 88 03 f5
unsigned char read_command_ic_check[14] = {0x02,0x00,0x09,0x34,0x32,0x03,0x40,0x01,0x30,0x00,0x01,0x88,0x03,0xf5};
//02 00 04 34 33 03 00 03 01
unsigned char read_command_ic_read[9] = {0x02,0x00,0x04,0x34,0x33,0x03,0x00,0x03,0x01};
//02 00 02 34 31 03 06
unsigned char read_command_ic_read_number[7] = {0x02,0x00,0x02,0x34,0x31,0x03,0x06};

///
/// \brief Card::Card 构造函数
/// \param pDepends
///
Card::Card()
{
    _strLogName = "card";
    pParamSet = ParamSet::GetInstance();   //获取配置模块句柄

    bCardSerialPort = false;
    count = 0; //定时器计数赋值0
    iCardType = 0;
}

///析构函数
/// \brief Card::~Card
///
Card::~Card()
{
}

///根据配置选项初始化函数
/// \brief Card::InitModule
/// \param pThread
/// \return
///
int  Card::InitModule (QThread *pThread)
{
    m_pWorkThread = pThread;

    this->moveToThread (m_pWorkThread);
    QObject::connect (m_pWorkThread,SIGNAL(started()),this,SLOT(slotThreadStartCard()));

    return  0;
}

///注册刷卡器设备到总线
/// \brief Card::RegistModule
/// \param pBus
/// \return
///
int Card::RegistModule ()
{
	QList<int> list;
	//-----------------刷卡远程充电相关主题--------------//
	list.append (AddrType_ScreenApplyReadCard); //主题一：显示屏申请读卡
	list.append(AddrType_ScreenApplyStopCard);  //主题一.一：显示屏申请结束读卡
	CBus::GetInstance()->RegistDev (this,list);    //模块标志号-刷卡器

	return 0;
}

///启动刷卡器模块
/// \brief Card::StartModule
/// \return
///
int Card::StartModule ()
{
    m_pWorkThread->start ();
    return 0;
}


///停止模块
/// \brief Card::StopModule
/// \return
///
int Card::StopModule ()
{
    if(pCardSecTimer.isActive()){
        pCardSecTimer.stop();
    }
    return 0;
}

///模块工作状态
/// \brief Card::ModuleStatus
/// \return
///
int Card::ModuleStatus ()
{
    return 0;
}



///SO库调用实现函数, 创建新实例返回
/// \brief CreateDevInstance
/// \param argc
/// \param pDepends
/// \return
///
CModuleIO* CreateDevInstance()
{
    return new Card();
}

///实例销毁
/// \brief DestroyDevInstance
/// \param pModule
///
void DestroyDevInstance(CModuleIO* pModule)
{

    if(pModule){
        delete pModule;
    }
}


///刷卡器开始工作
/// \brief Card::slotThreadStartCard
///
void Card::slotThreadStartCard ()
{
     connect(&pCardSecTimer, SIGNAL(timeout()), this, SLOT(readCardNum()));
}

///刷卡器串口初始化函数
/// \brief Card::SerialCardInit
///
void Card::SerialCardInit()
{
    stCSCUSysConfig cscuSysConfig;       //CSCU参数设置缓存

    //获取底板型号
    pParamSet->querySetting(&cscuSysConfig, PARAM_CSCU_SYS);
    //串口打开并初始化
    switch(cscuSysConfig.boardType)
    {
    case 1:
        bCardSerialPort = CardSerialPort.Open (CARD_SERIAL_NUM_3);        //1.0地板ttys5
        break;
    case 2:
        bCardSerialPort = CardSerialPort.Open (CARD_SERIAL_NUM_1);
        break;
    case 3:
        bCardSerialPort = CardSerialPort.Open (CARD_SERIAL_NUM_2);
        break;
    default:
        bCardSerialPort = CardSerialPort.Open (CARD_SERIAL_NUM_1);
        break;
    }
    CardSerialPort.SetParity (8,'N',1);
    CardSerialPort.SetSpeed (115200);

    if(bCardSerialPort == true)
    {
          CardWorkStartFlag = TRUE;
    }
}

//------------------------------------------------------CSCU BUS接口相关------------------------------------------------------//
///
/// \brief slotFromBusCard 接收BUS数据
/// \param qInfoMap
/// \param InfoType
///
void Card::slotFromBus(InfoMap CardMap, InfoAddrType CardType)
{
	switch (CardType) {
		case AddrType_ScreenApplyReadCard:  //主题一：显示屏申请读卡
			if(CardMap.contains(Addr_CanID_Comm)){
				ucCanID = CardMap[Addr_CanID_Comm].at(0);  //    //确定CAN地址
			}
			else{
				return;
			}

			if(CardMap.contains(Addr_CardType)){
				ucCardType = CardMap[Addr_CardType].at(0);  //    //确定卡类型
			}
			else{
				return;
			}

			switch (ucCardType) {  //不同卡类型开启定时器时间不同
				case 1://1-ID卡

					break;
				case 2://2-特来电CPU卡(默认值读CPU卡)  //20s
					if(!pCardSecTimer.isActive()){
						pCardSecTimer.start (400);  //读卡
					}
					break;
				case 3://3-智能出行卡
					if(!pCardSecTimer.isActive()){
						pCardSecTimer.start (400);  //读卡
					}
					break;
				case 4://4-招商银行卡
					if(!pCardSecTimer.isActive()){
						pCardSecTimer.start (400);  //读卡
					}
					break;
				case 5://5-车分享卡

					break;
				case 6://6-智能出行+CPU卡   (刷IC特来电号，（北京公车改革智能出行卡）)
					if(!pCardSecTimer.isActive()){
						pCardSecTimer.start (200);  //读卡100ms一次
					}
					break;
				case 7://7-招商银行+CPU卡    刷IC固件卡号，（北京公车改革工商银行卡）
					if(!pCardSecTimer.isActive()){
						pCardSecTimer.start (200);  //读卡100ms一次
					}
					break;
				case 8://8-车分享+CPU卡

					break;
				default:
					break;
			}

			readCardNum();   //读卡
			break;

		case AddrType_ScreenApplyStopCard:  //主题一.一：显示屏申请结束读卡
			if(pCardSecTimer.isActive ()){
				pCardSecTimer.stop ();
			}
			if(bCardSerialPort == true){
				CardSerialPort.Close ();  //关掉读卡器端口
				bCardSerialPort = false;
			}
			count = 0; //计数清0
			break;
		default:
			break;
	}
}

///
/// \brief Card::SendCardNumToBus 将取出的卡号与CAN ID发到Bus上
/// \return
///
bool Card::SendCardNumToBus()
{
    InfoMap CardMap;
    InfoAddrType CardType;

    if(PackageOutCardNum(CardMap, CardType)){
        emit sigToBus(CardMap,CardType);  //将CAN ID/卡号发到Bus
//        CardMap.clear ();
    }
    else{
        return false;
    }

    return true;
}

///
/// \brief PackageOutCardNum //将CAN ID 卡号打包
/// \param CardMap
/// \param CardType
/// \return
///
bool Card::PackageOutCardNum(InfoMap &CardMap, InfoAddrType &CardType)
{
    QByteArray qCardTempByteArray;

    CardType = AddrType_CenterReadCard;   //显示屏申请读卡主题

    qCardTempByteArray.append(ucCanID);
    CardMap.insert(Addr_CanID_Comm, qCardTempByteArray); //CAN地址

    qCardTempByteArray.clear();
//    qCardTempByteArray.append((char *)&sCardnumberBuf);
     if(iCardType == 3){ //招商银行卡
         qCardTempByteArray = QByteArray((const char *)sCardnumberBuf, 4);
     }
     else{
         qCardTempByteArray = QByteArray((const char *)sCardnumberBuf, 8);
    }
    CardMap.insert(Addr_CardAccount, qCardTempByteArray);  //卡号

    return true;
}

///
/// \brief Card::readCardNum        读卡片信息函数
/// \return
///
int Card::readCardNum ()
{
    int countnum;
       if(bCardSerialPort == false){
           SerialCardInit();   //串口初始化
       }
       switch (ucCardType) {
       case 1://1-ID卡

           break;
       case 2://2-特来电CPU卡(默认值读CPU卡)  //20s
           count ++;           
           if(readCardNumCpu() == 1){
               if(pCardSecTimer.isActive ()){
                   pCardSecTimer.stop ();
               }
               if(bCardSerialPort == true){
                   CardSerialPort.Close ();  //关掉读卡器端口
                   bCardSerialPort = false;
               }
               system("udisk_buzzer_out_one_time.sh &");
               SendCardNumToBus();  //将取出的卡号与CAN ID发到Bus上
               count = 0; //计数清0
               return 1;
           }

           if(count == 70){   //28s超时
               count = 0;
               if(pCardSecTimer.isActive ()){
                   pCardSecTimer.stop ();
               }
               if(bCardSerialPort == true){
                   CardSerialPort.Close ();  //关掉读卡器端口
                   bCardSerialPort = false;
               }
           }
           break;
       case 3://3-智能出行卡
           count ++;           
           if(readCardNumIC() == 1){
               if(pCardSecTimer.isActive ()){
                   pCardSecTimer.stop ();
               }
               if(bCardSerialPort == true){
                   CardSerialPort.Close ();  //关掉读卡器端口
                   bCardSerialPort = false;
               }
               system("udisk_buzzer_out_one_time.sh &");
               SendCardNumToBus();  //将取出的卡号与CAN ID发到Bus上
               count = 0; //计数清0
               return 1;
           }

           if(count == 70){   //28s超时
               count = 0;
               if(pCardSecTimer.isActive ()){
                   pCardSecTimer.stop ();
               }
               if(bCardSerialPort == true){
                   CardSerialPort.Close ();  //关掉读卡器端口
                   bCardSerialPort = false;
               }
           }
           break;
       case 4://4-招商银行卡
           count ++;           
           if(readCardNumICNumber() == 1){
               if(pCardSecTimer.isActive ()){
                   pCardSecTimer.stop ();
               }
               if(bCardSerialPort == true){
                   CardSerialPort.Close ();  //关掉读卡器端口
                   bCardSerialPort = false;
               }
               system("udisk_buzzer_out_one_time.sh &");
               SendCardNumToBus();  //将取出的卡号与CAN ID发到Bus上
               count = 0; //计数清0
               return 1;
           }

           if(count == 70){   //28s超时
               count = 0;
               if(pCardSecTimer.isActive ()){
                   pCardSecTimer.stop ();
               }
               if(bCardSerialPort == true){
                   CardSerialPort.Close ();  //关掉读卡器端口
                   bCardSerialPort = false;
               }
           }
           break;
       case 5://5-车分享卡

           break;
       case 6://6-智能出行+CPU卡   (刷IC特来电号，（北京公车改革智能出行卡）)
           count ++;
           countnum = count%2;

           if(countnum != 0){
              if(readCardNumIC() == 1){
                  if(pCardSecTimer.isActive ()){
                      pCardSecTimer.stop ();
                  }
                  if(bCardSerialPort == true){
                      CardSerialPort.Close ();  //关掉读卡器端口
                      bCardSerialPort = false;
                  }
                  system("udisk_buzzer_out_one_time.sh &");
                  SendCardNumToBus();  //将取出的卡号与CAN ID发到Bus上
                  count = 0; //计数清0
                  return 1;
              }
           }

           if(countnum == 0){
               if(readCardNumCpu() == 1){
                   if(pCardSecTimer.isActive ()){
                       pCardSecTimer.stop ();
                   }
                   if(bCardSerialPort == true){
                       CardSerialPort.Close ();  //关掉读卡器端口
                       bCardSerialPort = false;
                   }
                   system("udisk_buzzer_out_one_time.sh &");
                   SendCardNumToBus();  //将取出的卡号与CAN ID发到Bus上
                   count = 0; //计数清0
                   return 1;
               }
           }

           if(count == 140){   //28s超时
               count = 0;
               if(pCardSecTimer.isActive ()){
                   pCardSecTimer.stop ();
               }
               if(bCardSerialPort == true){
                   CardSerialPort.Close ();  //关掉读卡器端口
                   bCardSerialPort = false;
               }
           }
           break;
        case 7://7-招商银行+CPU卡    刷IC固件卡号，（北京公车改革工商银行卡）
           count ++;
           countnum = count%2;

           if(countnum != 0){
              if(readCardNumICNumber() == 1){
                  if(pCardSecTimer.isActive ()){
                      pCardSecTimer.stop ();
                  }
                  if(bCardSerialPort == true){
                      CardSerialPort.Close ();  //关掉读卡器端口
                      bCardSerialPort = false;
                  }
                  system("udisk_buzzer_out_one_time.sh &");
                  SendCardNumToBus();  //将取出的卡号与CAN ID发到Bus上
                  count = 0; //计数清0
                  return 1;
              }
           }

           if(countnum == 0){
               if(readCardNumCpu() == 1){
                   if(pCardSecTimer.isActive ()){
                       pCardSecTimer.stop ();
                   }
                   if(bCardSerialPort == true){
                       CardSerialPort.Close ();  //关掉读卡器端口
                       bCardSerialPort = false;
                   }
                   system("udisk_buzzer_out_one_time.sh &");
                   SendCardNumToBus();  //将取出的卡号与CAN ID发到Bus上
                   count = 0; //计数清0
                   return 1;
               }
           }

           if(count == 140){   //28s超时
               count = 0;
               if(pCardSecTimer.isActive ()){
                   pCardSecTimer.stop ();
               }
               if(bCardSerialPort == true){
                   CardSerialPort.Close ();  //关掉读卡器端口
                   bCardSerialPort = false;
               }
           }

            break;
        case 8://8-车分享+CPU卡

            break;
       default:
           break;
       }

       return 0;
}

///
/// \brief Card::readCardNumCpu读取CPU卡卡号
/// \return
///
int Card::readCardNumCpu(){
    int iDataNum = 0;
    int iDataWrite = 0;
    unsigned char sCardReadHeader[5];         //接收数据头
    unsigned char sCardReadData[64];           //接收数据

    memset(sCardReadHeader,0,sizeof(sCardReadHeader));
    memset(sCardReadData,0,sizeof(sCardReadData));
    memset(sCardnumberBuf,0,sizeof(sCardnumberBuf));

    iDataWrite = CardSerialPort.Write (sWriteCpuCardNum,sizeof(sWriteCpuCardNum));
    if(iDataWrite < 0){
        return 0;
    }
//    for(int i = 0;i < (int)sizeof(sWriteCpuCardNum); i ++)
//    {
//    }

    //每次读取数据时间间隔400ms
    iDataNum = CardSerialPort.Read (sCardReadData,64, 400);
   // if(iDataNum >= 0) //nihai modify
    if(iDataNum >=15)
    {
        if(sCardReadData[5] == 0x59){
            if(sCardReadData[6] == 0x71 && sCardReadData[7] == 0x72 && sCardReadData[8] == 0x73 && sCardReadData[9] == 0x74)
                return 0;
            else{
    //            sCardnumberBuf[0] = 0x08;
                iCardType = 1;
                memcpy(&sCardnumberBuf[0],&sCardReadData[7],8);
                return 1;
            }
        }
        return 0;
    }
    return 0;
}

///
/// \brief Card::readCardNumIC 读取IC卡片信息
/// \return
///
int Card::readCardNumIC()
{
    int iDataWrite = 0;
    unsigned char card_buf_header[5];         //接收数据
    unsigned char card_buf_data[64];          //接收数据

    memset(card_buf_header,0,sizeof(card_buf_header));
    memset(card_buf_data,0,sizeof(card_buf_data));
    memset(sCardnumberBuf,0,sizeof(sCardnumberBuf));

    iDataWrite = CardSerialPort.Write (read_command_ic_check,sizeof(read_command_ic_check));
    if(iDataWrite < 0){
        return 0;
    }

    if(CardSerialPort.Read (card_buf_header,3,100) < 3){
        return 0;
    }

    if(CardSerialPort.Read (card_buf_data,card_buf_header[2] + 2,100) < card_buf_header[2] + 2){
        return 0;
    }

    if(card_buf_data[3] == 0x59){

        memset(card_buf_header,0,sizeof(card_buf_header));
        memset(card_buf_data,0,sizeof(card_buf_data));

        if(CardSerialPort.Write(read_command_ic_read, (int)sizeof(read_command_ic_read)) < (int)sizeof(read_command_ic_read)){
            return 0;
        }

        usleep(10000);

        if(CardSerialPort.Read (card_buf_header,3,100) < 3){
            return 0;
        }

        if(CardSerialPort.Read(card_buf_data,card_buf_header[2] + 2,100) < card_buf_header[2] + 2){
            return 0;
        }

        if(card_buf_data[4] == 0x59){
             iCardType = 2;
//            card_number_buf[0] = 0x08;
            memcpy(&sCardnumberBuf[0],&card_buf_data[5],8);
//            for(int i = 0;i < (int)sizeof(sCardnumberBuf);i ++)
//            {
//            }
            return 1;
        }
    }
    return 0;
}

///
/// \brief Card::readCardNumICNumber 读取IC卡的固件卡号信息
/// \return
///
int Card::readCardNumICNumber()
{
    unsigned char card_buf_header[5];         //接收数据
    unsigned char card_buf_data[64];          //接收数据

    memset(card_buf_header,0,sizeof(card_buf_header));
    memset(card_buf_data,0,sizeof(card_buf_data));
    memset(sCardnumberBuf,0,sizeof(sCardnumberBuf));


    if(CardSerialPort.Write(read_command_ic_read_number, (int)sizeof(read_command_ic_read_number)) < (int)sizeof(read_command_ic_read_number)){
        return 0;
    }

    usleep(10000);

    if(CardSerialPort.Read(card_buf_header,5,100) < 5){
        return 0;
    }

    if(CardSerialPort.Read(card_buf_data,card_buf_header[2],100) < card_buf_header[2]){
        return 0;
    }

    if(card_buf_data[0] == 0x59){

        if(card_buf_data[1] == 0x71 && card_buf_data[2] == 0x72 && card_buf_data[3] == 0x73 && card_buf_data[4] == 0x74){
            memset(sCardnumberBuf,0,sizeof(sCardnumberBuf));
            return 0;
        }
        else{
            iCardType = 3;
            memcpy(&sCardnumberBuf[0],&card_buf_data[1],4);
            return 1;
        }
    }
    return 0;
}

