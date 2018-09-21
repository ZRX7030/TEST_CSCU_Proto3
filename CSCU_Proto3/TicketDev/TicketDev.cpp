#include <stdio.h>
#include <stdlib.h>
#include <qcoreapplication.h>
#include <netinet/in.h>
#include "TicketDev/TicketDev.h"
#include "QDebug"
#include "CommonFunc/commfunc.h"

///
/// \brief TicketDev::TicketDev 构造函数
/// \param pDepends
///
TicketDev::TicketDev()
{
    bTicketDevSerialPort = false;
    pParamSet = ParamSet::GetInstance();
    pDBOperate = DBOperate::GetInstance();
}

///
/// \brief TicketDev::~TicketDev  析构函数
///
TicketDev::~TicketDev()
{
}

///根据配置选项初始化
/// \brief TicketDev::InitModule
/// \param pThread
/// \return
///
int TicketDev::InitModule (QThread *pThread)
{
    m_pWorkThread = pThread;

    this->moveToThread (m_pWorkThread);
    QObject::connect (m_pWorkThread,SIGNAL(started()),this,SLOT(ProcStartWork()));


    return  0;
}

///注册设备到总线
/// \brief TicketDev::RegistModule
/// \param pBus
/// \return
///
int TicketDev::RegistModule ()
{
	QList<int> List;//

	List.append(AddrType_MakePrintTicket);//执行打印小票动作。显示屏发布，小票机订阅

	CBus::GetInstance()->RegistDev(this, List);

    return 0;
}

///启动模块
/// \brief TicketDev::StartModule
/// \return
///
int TicketDev::StartModule ()
{
    m_pWorkThread->start();

    return 0;
}

///停止模块
/// \brief TicketDev::StopModule
/// \return
///
int TicketDev::StopModule ()
{

}

///模块工作状态
/// \brief TicketDev::ModuleStatus
/// \return
///
int TicketDev::ModuleStatus ()
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
    return new TicketDev();
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


///开始工作
/// \brief TicketDev::ProcStartWork
///
void TicketDev::ProcStartWork()
{
//    PrintTicket();
}

///接收BUS数据
/// \brief TicketDev::slotFromBus
/// \param TicketDevMap
/// \param TicketDevMapType
///
void TicketDev::slotFromBus(InfoMap TicketDevMap, InfoAddrType TicketDevMapType)
{
    unsigned char ucCanID;
    if(TicketDevMapType == AddrType_MakePrintTicket)
    {
        if(TicketDevMap.contains(Addr_CanID_Comm)){
             ucCanID = TicketDevMap[Addr_CanID_Comm].at(0);  //    //确定CAN地址
        }
        PrintTicket(ucCanID);
    }
}

///小票机串口初始化函数
/// \brief TicketDev::SerialTicketDevInit
///
void TicketDev::SerialTicketDevInit()
{
    stCSCUSysConfig cscuSysConfig;       //CSCU参数设置缓存
    TicketDevSerialPort = new cSerialPort();    //对应串口类

    //获取底板型号
    pParamSet->querySetting(&cscuSysConfig, PARAM_CSCU_SYS);
    //串口打开并初始化
    switch(cscuSysConfig.boardType)
    {
    case 2:
        bTicketDevSerialPort = TicketDevSerialPort->Open(TicketDev_SERIAL_NUM);
        break;
    case 3:
        bTicketDevSerialPort = TicketDevSerialPort->Open(TicketDev_SERIAL_NUM_1);
        break;
    default:
        bTicketDevSerialPort = TicketDevSerialPort->Open(TicketDev_SERIAL_NUM);
        break;
    }

    TicketDevSerialPort->SetParity(8,'N',1);
    TicketDevSerialPort->SetSpeed(9600);   //波特率9600
    if(bTicketDevSerialPort == true)
    {
        TicketDevWorkStartFlag = TRUE;
    }
}

///打印小票
/// \brief PrintTicket
///打印机的查询命令为1B 76 不受03 55命令控制，当打印机缺纸时，打印机返回一个字节状态00；
///当打印机工作正常时，返回一个字节状态01.如返回其他状态字节表示是无效字节。
void TicketDev::PrintTicket(unsigned char ucCanID)
{
    unsigned char value = 0;    //1-打印；2-否
    int iDataRead = 0;
    int iDataWrite = 0;
    unsigned char sWritePrintCmd[2] = {0x1B,0x76};
    unsigned char sReadData[10];           //接收数据
    if(bTicketDevSerialPort == false){
        SerialTicketDevInit();   //串口初始化
    }
    iDataWrite = TicketDevSerialPort->Write (sWritePrintCmd,sizeof(sWritePrintCmd));
    if(iDataWrite < 0){
        return;
    }
    iDataRead = TicketDevSerialPort->Read (sReadData,10, 400);
    if(iDataRead >= 0)
    {
        if(sReadData[0] == 0x01){    //允许打印
            PrintTicketInfo(ucCanID);
            value = 1;
        }
        else if(sReadData[0] == 0x00){   //提示缺纸状态
            value = 2;
        }
        InfoMap ToCenterMap;
        QByteArray qTempByteArray;
        qTempByteArray.append((char)ucCanID);
        ToCenterMap.insert(Addr_CanID_Comm, qTempByteArray);//can地址
        ToCenterMap.insert(Addr_TicketPrint_Result, QByteArray((char *)&value,1));

        //发送
        emit sigToBus(ToCenterMap, AddrType_MakePrintTicketResult);
    }
    else
    {
        value = 3;

        InfoMap ToCenterMap;
        QByteArray qTempByteArray;
        qTempByteArray.append((char)ucCanID);
        ToCenterMap.insert(Addr_CanID_Comm, qTempByteArray);//can地址
        ToCenterMap.insert(Addr_TicketPrint_Result, QByteArray((char *)&value,1));

        //发送
        emit sigToBus(ToCenterMap, AddrType_MakePrintTicketResult);
    }
}

///打印小票信息
/// \brief TicketDev::PrintTicketInfo
///
void TicketDev::PrintTicketInfo(unsigned char ucCanID)
{
    //字符编码转换
    QTextCodec * pGBK = QTextCodec::codecForName("GBK");
    QString tempString;
    QByteArray tempArray;
    //充电时间转换
    QDateTime startTime, stopTime;

    int seconds = 0;
    unsigned short usH, usM, usS;

//    uchar ucCanID = 0xb5;
    struct db_result_st result;
    QString todo = QString("select EventNo,StartTime,EndTime,TotalChargeTime,TotalChargeEnergy from charge_order where CanAddr = " + QString::number(ucCanID, 10));

    UART_WR_PRINTF_INFORMATION_PDU * pPrintTicket;

    unsigned char * pData = new unsigned char[sizeof(UART_WR_PRINTF_INFORMATION_PDU)];
    pPrintTicket = (UART_WR_PRINTF_INFORMATION_PDU *)pData;

    pPrintTicket->pdu_header_1 = 0x03;
    pPrintTicket->pdu_header_2 = 0x55;
    pPrintTicket->pdu_command_high = 0x1B;
    pPrintTicket->pdu_command_low = 0x63;
    pPrintTicket->pdu_command_num = 0x00;   //出纸方向赋值-禁止反向打印0x00(1-允许反向打印)[7个字节]
    pPrintTicket->pdu_return_1_1 = 0x0B;
    pPrintTicket->pdu_return_1_2 = 0x0A;



    /***************1:青岛特来电充电系统充电详单[33个字节]*****************/
    tempArray.clear();
    tempArray = pGBK->fromUnicode(QString::fromUtf8("   青岛特来电充电系统充电详单:"));
    memset((unsigned char *)&pPrintTicket->line_1_word,0x00,sizeof(pPrintTicket->line_1_word));
    memcpy((unsigned char *)&pPrintTicket->line_1_word, tempArray.data(), tempArray.length());


    pPrintTicket->line_1_return_1 = htons(0x0B0A);
    pPrintTicket->line_1_return_2 = htons(0x0B0A);

    /***************2:流水号：201512160001 [24个字节]*****************/
    tempArray.clear();
    tempArray = pGBK->fromUnicode(QString::fromUtf8("流水号:"));
    memset((unsigned char *)&pPrintTicket->line_2_word,0x00,sizeof(pPrintTicket->line_2_word));
    memcpy((unsigned char *)&pPrintTicket->line_2_word, tempArray.data(), tempArray.length());

    pPrintTicket->line_2_return = htons(0x0B0A);

         /***************3:站名称：厦门刷卡充电系统 [28个字节]*****************/
        /**站  名：厦门刷卡充电系统D5BE 2020 C3FB 3A CFC3 C3C5 CBA2 BFA8 B3E4 B5E7 CFB5 CDB3**/
    tempArray.clear();
    tempArray = pGBK->fromUnicode(QString::fromUtf8("站名称:"));
    memset((unsigned char *)&pPrintTicket->line_3_word,0x00,sizeof(pPrintTicket->line_3_word));
    memcpy((unsigned char *)&pPrintTicket->line_3_word, tempArray.data(), tempArray.length());

    if(pParamSet->querySetting(&cscuConfig,PARAM_CSCU_SYS))
    {
        tempArray.clear();
        tempArray.append((char *)cscuConfig.stationName);
        tempString = tempArray;
        if(tempString == NULL){
            tempString = "特来电充电站";
        }
        tempArray = pGBK->fromUnicode(tempString);
        memset((unsigned char *)&pPrintTicket->ucStationName, 0x00, sizeof(pPrintTicket->ucStationName));
        memcpy((unsigned char *)&pPrintTicket->ucStationName, tempArray.data(), tempArray.length());
    }
    pPrintTicket->pdu_3_byte_27_return = htons (0x0B0A);

        /***************4:站地址：110106018810000 [24个字节]*****************/
       /**站地址：110106018810000  D5BE B5D8 D6B7 3A  313130313036303138383130303030**/
    tempArray.clear();
    tempArray = pGBK->fromUnicode(QString::fromUtf8("站地址:"));
    memset((unsigned char *)&pPrintTicket->line_4_word,0x00,sizeof(pPrintTicket->line_4_word));
    memcpy((unsigned char *)&pPrintTicket->line_4_word, tempArray.data(), tempArray.length());

    pParamSet->querySetting(&ServerConfig,PARAM_SERVER0);
//    QTextCodec *gbk_line_4_station_address=QTextCodec::codecForName ("GBK");
    QString sLine_4_station_address = ServerConfig.stationNo;
//    QByteArray bb_4=gbk_line_4_station_address->fromUnicode (sLine_4_station_address.data (),sLine_4_station_address.length (),0);
//    strcpy (pPrintTicket->line_4_station_address,bb_4.data ());
    tempArray.clear();
    tempArray = pGBK->fromUnicode(sLine_4_station_address);
    memset((unsigned char *)&pPrintTicket->line_4_station_address,0x00,sizeof(pPrintTicket->line_4_station_address));
    memcpy((unsigned char *)&pPrintTicket->line_4_station_address, tempArray.data(), tempArray.length());
//        int i;
//        for(i=0;i<20;i++)
//        {
//        }
        pPrintTicket->line_4_return = htons(0x0B0A);

    /***************5:终端号：181号充电终端 [24个字节]*****************/
   tempArray.clear();
   tempArray = pGBK->fromUnicode(QString::fromUtf8("终端号:"));
   memset((unsigned char *)&pPrintTicket->line_4_terminaWord,0x00,sizeof(pPrintTicket->line_4_terminaWord));
   memcpy((unsigned char *)&pPrintTicket->line_4_terminaWord, tempArray.data(), tempArray.length());

   QString qsCanID = QString::number((int)ucCanID,10);
   tempArray.clear();
   tempArray = pGBK->fromUnicode(qsCanID.data(),qsCanID.length(),0);
   memset((unsigned char *)&pPrintTicket->line_4_terminaNum,0x00,sizeof(pPrintTicket->line_4_terminaNum));
   memcpy((unsigned char *)&pPrintTicket->line_4_terminaNum, tempArray.data(), tempArray.length());

   tempArray.clear();
   tempArray = pGBK->fromUnicode(QString::fromUtf8("号充电终端:"));
   memset((unsigned char *)&pPrintTicket->line_4_terminaWord_2,0x00,sizeof(pPrintTicket->line_4_terminaWord_2));
   memcpy((unsigned char *)&pPrintTicket->line_4_terminaWord_2, tempArray.data(), tempArray.length());
   pPrintTicket->line_4_termina_return = htons(0x0B0A);

        /***************5:日  期：2015-12-16 18:00:42 [30个字节]*****************/
//        QTextCodec *gbk_line_5_word = QTextCodec::codecForName("GBK");
//        QString sLine_5_word = "日  期:";
//        QByteArray ba_5 =  gbk_line_5_word->fromUnicode(sLine_5_word.data(),sLine_5_word.length(),0);
//        strcpy(pPrintTicket->line_5_word,ba_5.data());

//        QTextCodec *gbk_line_5_date_now = QTextCodec::codecForName("GBK");
        QDateTime time;
        time = QDateTime::currentDateTime();
        QString sLine_5_date_now = time.toString ("yyyy-MM-dd hh:mm:ss"); //设置显示格式
//        QByteArray bb_5 =  gbk_line_5_date_now->fromUnicode(sLine_5_date_now.data(),sLine_5_date_now.length(),0);
//        strcpy(pPrintTicket->line_5_date_now,bb_5.data());

        tempArray.clear();
        tempArray = pGBK->fromUnicode(QString::fromUtf8("日  期:"));
        memset((unsigned char *)&pPrintTicket->line_5_word,0x00,sizeof(pPrintTicket->line_5_word));
        memcpy((unsigned char *)&pPrintTicket->line_5_word, tempArray.data(), tempArray.length());

        tempArray.clear();
        tempArray = pGBK->fromUnicode(sLine_5_date_now);
        memset((unsigned char *)&pPrintTicket->line_5_date_now,0x00,sizeof(pPrintTicket->line_5_date_now));
        memcpy((unsigned char *)&pPrintTicket->line_5_date_now, tempArray.data(), tempArray.length());

        pPrintTicket->line_5_return =  htons (0x0B0A);

        /***************6:分割线---------------------------------[32个字节]*****************/
        /* 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 0B0A*/
        QTextCodec *gbk_line_6_word = QTextCodec::codecForName("GBK");
        QString sLine_6_word = "--------------------------------";
        QByteArray ba_6 =  gbk_line_6_word->fromUnicode(sLine_6_word.data(),sLine_6_word.length(),0);
        strcpy(pPrintTicket->line_6_word,ba_6.data());

        /***************7:开始充电时间：2015-12-16 16:00:22[32个字节]*****************/
        /**开始充电时间：2015-12-16 16:00:22   BFAA CABC B3E4 B5E7 CAB1 BCE4 3A 32303135 2D 3132 2D 3136 3136 3A 3030 3A 3232**/      
        tempArray.clear();
        tempArray = pGBK->fromUnicode(QString::fromUtf8("开始充电时间:"));
        memset((unsigned char *)&pPrintTicket->line_7_word,0x00,sizeof(pPrintTicket->line_7_word));
        memcpy((unsigned char *)&pPrintTicket->line_7_word, tempArray.data(), tempArray.length());

        /***************8:结束充电时间：2015-12-16 18:00:38[32个字节]*****************/
        /**结束充电时间：2015-12-16 18:00:38   BDE1 CAF8 B3E4 B5E7 CAB1 BCE4 3A 32303135 2D 3132 2D 3136 3138 3A 3030 3A 3338**/
        tempArray.clear();
        tempArray = pGBK->fromUnicode(QString::fromUtf8("结束充电时间:"));
        memset((unsigned char *)&pPrintTicket->line_8_word,0x00,sizeof(pPrintTicket->line_8_word));
        memcpy((unsigned char *)&pPrintTicket->line_8_word, tempArray.data(), tempArray.length());

        /***************9:充电所用时长：2时00分16秒[28个字节]*****************/
       /*充电所用时长：02时00分16秒  B3E4 B5E7 CBF9 D3C3 CAB1 B3A4 3A 3032 CAB1 3030 B7D6 3136 C3EB*/
        tempArray.clear();
        tempArray = pGBK->fromUnicode(QString::fromUtf8("充电所用时长:"));
        memset((unsigned char *)&pPrintTicket->line_9_word,0x00,sizeof(pPrintTicket->line_9_word));
        memcpy((unsigned char *)&pPrintTicket->line_9_word, tempArray.data(), tempArray.length());

        pPrintTicket->pdu_9_byte_27_return = htons(0x0B0A);

        /***************10:充电    电量：002.12度[24个字节]*****************/
       /*充电    电量：2.12度  B3E4 B5E7 2020 2020 B5E7 C1BF 3A 0000322E3132 B6C8*/
        tempArray.clear();
        tempArray = pGBK->fromUnicode(QString::fromUtf8("充电    电量:"));
        memset((unsigned char *)&pPrintTicket->line_10_word,0x00,sizeof(pPrintTicket->line_10_word));
        memcpy((unsigned char *)&pPrintTicket->line_10_word, tempArray.data(), tempArray.length());

        pPrintTicket->pdu_10_byte_21_word_5 = htons(0xB6C8);
        pPrintTicket->line_10_return = htons(0x0B0A);

       /***************14:分割线---------------------------------[32个字节]*****************/
       /* 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 0B0A*/
       tempArray.clear();
       tempArray = pGBK->fromUnicode(QString::fromUtf8("--------------------------------"));
       memset((unsigned char *)&pPrintTicket->line_14_word,0x00,sizeof(pPrintTicket->line_14_word));
       memcpy((unsigned char *)&pPrintTicket->line_14_word, tempArray.data(), tempArray.length());


       /***************15:欢迎光临，谢谢惠顾！[28个字节]*****************/
       /*欢迎光临，谢谢惠顾！ 2020 2020 2020 BBB6 D3AD B9E2 C1D9 2C D0BB D0BB BBDD B9CB 21 0B0A*/
       tempArray.clear();
       tempArray = pGBK->fromUnicode(QString::fromUtf8("      欢迎光临，谢谢惠顾!"));
       memset((unsigned char *)&pPrintTicket->line_15_word,0x00,sizeof(pPrintTicket->line_15_word));
       memcpy((unsigned char *)&pPrintTicket->line_15_word, tempArray.data(), tempArray.length());

       pPrintTicket->pdu_end1_byte_27_return = htons(0x0B0A);

       /***************16:青岛特来电新能源有限公司[32个字节]*****************/
       /*青岛特来电新能源有限公司   2020 2020 C7E0 B5BA CCD8 C0B4 B5E7 D0C2 C4DC D4B4 D3D0 CFDE B9AB CBBE 0B0A*/
       tempArray.clear();
       tempArray = pGBK->fromUnicode(QString::fromUtf8("    青岛特来电新能源有限公司"));
       memset((unsigned char *)&pPrintTicket->line_16_word,0x00,sizeof(pPrintTicket->line_16_word));
       memcpy((unsigned char *)&pPrintTicket->line_16_word, tempArray.data(), tempArray.length());
       pPrintTicket->line_16_return=htons (0x0B0A);

       /***************17:QINGDAO TELD NEW ENERGY OD . LTD[32个字节]*****************/
       /*QINGDAO TELD NEW ENERGY OD . LTD    51494E4744414F 20 54454C44 20 4E4557 20 454E45524759 20 4F 20 2E 20 4C5444 0B0A*/
       tempArray.clear();
       tempArray = pGBK->fromUnicode(QString::fromUtf8("QINGDAO TELD NEW ENERGY CD.,LTD. "));
       memset((unsigned char *)&pPrintTicket->line_17_word,0x00,sizeof(pPrintTicket->line_17_word));
       memcpy((unsigned char *)&pPrintTicket->line_17_word, tempArray.data(), tempArray.length());

       /***************18:网    址：http://www.teld.cn[28个字节]*****************/
       /*网    址：http://www.teld.cn	 202020 CDF8 D6B7 3A 687474703A2F2F 7777772E74656C642E636E 0B0A*/
       tempArray.clear();
       tempArray = pGBK->fromUnicode(QString::fromUtf8("  网    址：http://www.teld.cn"));
       memset((unsigned char *)&pPrintTicket->line_18_word,0x00,sizeof(pPrintTicket->line_18_word));
       memcpy((unsigned char *)&pPrintTicket->line_18_word, tempArray.data(), tempArray.length());
       pPrintTicket->pdu_end4_byte_27_return = htons (0x0B0A);

        /***************19:客服电话：4001-300-001[32个字节]*****************/
       /*客服电话：4001300001	2020 2020 2020  BFCD B7FE B5E7 BBB0 3A 34303031 2D 333030 2D 303031*/
       tempArray.clear();
       tempArray = pGBK->fromUnicode(QString::fromUtf8("     客服电话：4001-300-001"));
       memset((unsigned char *)&pPrintTicket->line_19_word,0x00,sizeof(pPrintTicket->line_19_word));
       memcpy((unsigned char *)&pPrintTicket->line_19_word, tempArray.data(), tempArray.length());
       pPrintTicket->pdu_end5_byte_31_return = htons (0x0B0A);


    /***************结束：换行+帧尾[12个字节]*****************/
    //pPrintTicket->pdu_return_4=htons(0x0B0A);
    pPrintTicket->pdu_printf_end_return_1 = htons(0x0B0A);
    pPrintTicket->pdu_printf_end_return_2 = htons (0x0B0A);
    pPrintTicket->pdu_printf_end_return_3 = htons (0x0B0A);
    pPrintTicket->pdu_printf_end_return_4 = htons (0x0B0A);
    pPrintTicket->pdu_printf_end_return_5 = htons (0x0B0A);
    pPrintTicket->pdu_printf_end_return_6 = htons (0x0B0A);
    pPrintTicket->pdu_printf_end_return_7 = htons (0x0B0A);
    pPrintTicket->pdu_end_1=0x03;
    pPrintTicket->pdu_end_2=0xAA;

    int ret = pDBOperate->DBSqlQuery(todo.toAscii().data(), &result, DB_PROCESS_RECORD);
    if(ret == 0)//查询成功
    {
        if(result.column > 0)
        {
//            QString todo = QString("select EventNo,StartTime,EndTime,TotalChargeTime,TotalChargeEnergy from charge_order where CanAddr = " + QString::number(ucCanID, 10));

            tempArray.clear();
            tempArray = pGBK->fromUnicode(QString::fromUtf8(result.result[(result.row - 1) * result.column]));
            memset((unsigned char *)&pPrintTicket->line_2_sEventNo,0x00,sizeof(pPrintTicket->line_2_sEventNo));
            memcpy((unsigned char *)&pPrintTicket->line_2_sEventNo[0], tempArray.data(), tempArray.length());


            tempArray.clear();
            tempArray = pGBK->fromUnicode(QString::fromUtf8(result.result[(result.row - 1) * result.column + 1]));
            memset((unsigned char *)&pPrintTicket->line_7_date_start_charge,0x00,sizeof(pPrintTicket->line_7_date_start_charge));
            memcpy((unsigned char *)&pPrintTicket->line_7_date_start_charge, tempArray.data(), tempArray.length());

            tempArray.clear();
            tempArray = pGBK->fromUnicode(QString::fromUtf8(result.result[(result.row - 1) * result.column + 2]));
            memset((unsigned char *)&pPrintTicket->line_7_date_end_charge,0x00,sizeof(pPrintTicket->line_7_date_end_charge));
            memcpy((unsigned char *)&pPrintTicket->line_7_date_end_charge, tempArray.data(), tempArray.length());

            tempArray.clear();
            float chargeEnergy = 0;
            char Energy[100];
            chargeEnergy = atof(result.result[(result.row - 1) * result.column + 4])*0.01;
            sprintf(Energy, "%.2f", chargeEnergy);
            tempArray = pGBK->fromUnicode(QString::fromUtf8(Energy));
            //tempArray = pGBK->fromUnicode(QString::fromUtf8((result.result[(result.row - 1) * result.column + 4])));
            memset((unsigned char *)&pPrintTicket->line_10_energy,0x00,sizeof(pPrintTicket->line_10_energy));
            memcpy((unsigned char *)&pPrintTicket->line_10_energy, (tempArray.data()), tempArray.length());

            //开始充电时间
            startTime = QDateTime::fromString(QString::fromUtf8(result.result[(result.row - 1) * result.column + 1]),"yyyy-MM-dd HH:mm:ss");
            //结束充电时间
            stopTime = QDateTime::fromString(QString::fromUtf8(result.result[(result.row - 1) * result.column + 2]),"yyyy-MM-dd HH:mm:ss");
            //充电时间
            seconds = startTime.secsTo(stopTime);
            //时间错误,则充电时间显示0
            if(seconds < 0)
            {
                seconds = 0;
            }

            usH = seconds/3600;
            usM = seconds/60 -usH*60;
            usS = seconds%60;
            QString sChargeTimeHour = QString::number((int)usH,10);
            QString sChargeTimeMin = QString::number((int)usM,10);
            QString sChargeTimeSecond = QString::number((int)usS,10);
            QString sChargeTime = sChargeTimeHour + "时";
            sChargeTime += sChargeTimeMin + "分";
            sChargeTime += sChargeTimeSecond + "秒";
            tempArray.clear();
            tempArray = pGBK->fromUnicode(sChargeTime);
            memset((unsigned char *)&pPrintTicket->line_9_charge_time,0x00,sizeof(pPrintTicket->line_9_charge_time));
            memcpy((unsigned char *)&pPrintTicket->line_9_charge_time, tempArray.data(), tempArray.length());
        }
    }

//    for(int i =0;i<26;i++){
//    }
    //发送帧
    TicketDevSerialPort->Write(pData, sizeof(* pPrintTicket));
    if(pData == NULL)
    {
    }
    else
    {
        delete pData;
    }
}
