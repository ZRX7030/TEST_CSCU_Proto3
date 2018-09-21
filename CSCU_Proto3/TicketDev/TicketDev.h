#ifndef TICKETDEV_H
#define TICKETDEV_H

#include <QObject>
#include <QTimer>
#include <QThread>
#include "Infotag/CSCUBus.h"
#include "DevCache/DevCache.h"
#include "GeneralData/GeneralData.h"
#include "GeneralData/ModuleIO.h"
#include "ParamSet/ParamSet.h"
#include "SerialPort/SerialPort.h"
#include <QTextCodec>

class TicketDev : public CModuleIO   //继承main的类
{
    Q_OBJECT
    public:
        TicketDev();
        ~TicketDev();

    private:
        int InitModule (QThread *pThread);  //根据配置选项初始化
        int RegistModule ();         //注册设备到总线
        int StartModule ();                                //启动模块
        int StopModule ();                                //停止模块
        int ModuleStatus ();                             //模块工作状态

        ParamSet *pParamSet;
        DBOperate *pDBOperate;

        //部分显示数据存储
        stServer0Config ServerConfig;        //服务器参数设置缓存
        stCSCUSysConfig cscuConfig;      //CSCU参数设置缓存
    signals:
        void sigToBus(InfoMap TicketDevMap, InfoAddrType TicketDevMapType);

    public slots:
        //----------------------------------------------From BUS-----------------------------------------------//
        void slotFromBus(InfoMap TicketDevMap, InfoAddrType TicketDevMapType);       // 接收BUS数据-显示屏申请打印小票
        //开始工作
        void ProcStartWork();
    private:
        //内部定义数据
        int  gTicketDevSerial;
        bool TicketDevWorkStartFlag;                     //开始工作标识位
        cSerialPort * TicketDevSerialPort;               //串口类
        bool bTicketDevSerialPort;

    private:
        void SerialTicketDevInit();                                              //小票机串口初始化函数
        void PrintTicket(unsigned char ucCanID);     //打印小票
        void PrintTicketInfo(unsigned char ucCanID);   //打印小票信息
};

/*
 * 串口屏通信协议定义
 *
 * 打印小票信息的数据结构
 *	实例：
 */
typedef struct{
        unsigned char pdu_header_1;					    //帧头_1 0x03
        unsigned char pdu_header_2;					    //帧头_2 0x55
        unsigned char pdu_command_high;			//出纸方向高字节 0x1B
        unsigned char pdu_command_low;				//出纸方向低字节 0x63

        unsigned char pdu_command_num;           //出纸方向赋值-禁止反向打印0x00(1-允许反向打印)[7个字节]
        unsigned char pdu_return_1_1;
        unsigned char pdu_return_1_2;

        /***************1:青岛特来电充电系统充电详单[33个字节]*****************/
         char line_1_word[29];

        unsigned short line_1_return_1;                     //换行 0B0A
        unsigned short line_1_return_2;                     //换行 0B0A

        /***************2:流水号：201512160001 [24个字节]*****************/
       // char *line_2_word[7];
        char line_2_word[7];
        char line_2_sEventNo[18];
        unsigned short line_2_return;                     //换行 0B0A


        /***************3:站名称：厦门刷卡充电系统 [28个字节]*****************/
        char line_3_word[7];
        unsigned char ucStationName[40];   //站名称----GBK
        unsigned short pdu_3_byte_27_return;

        /***************4:站地址：110106018810000 [24个字节]*****************/
        char line_4_word[7];
        char line_4_station_address[15];
        unsigned short  line_4_return;

         /***************5:终端号：181号充电终端 [24个字节]*****************/
        char line_4_terminaWord[7];
        char line_4_terminaNum[3];
        char line_4_terminaWord_2[10];
        unsigned short  line_4_termina_return;

        /***************5:日  期：2015-12-16 18:00:42 [30个字节]*****************/
        char line_5_word[7];
        char line_5_date_now[20];
        unsigned short line_5_return;

        /***************6:分割线---------------------------------[32个字节]*****************/
        /* 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D*/
         char line_6_word[32];


        /***************7:开始充电时间：2015-12-16 16:00:22[32个字节]*****************/
        /**开始充电时间：2015-12-16 16:00:22   BFAA CABC B3E4 B5E7 CAB1 BCE4 3A 32303135 2D 3132 2D 3136 3136 3A 3030 3A 3232**/
        char line_7_word[13];
        char line_7_date_start_charge[20];

        /***************8:结束充电时间：2015-12-16 18:00:38[32个字节]*****************/
        /**结束充电时间：2015-12-16 18:00:38   BDE1 CAF8 B3E4 B5E7 CAB1 BCE4 3A 32303135 2D 3132 2D 3136 3138 3A 3030 3A 3338**/
        char line_8_word[13];
        char line_7_date_end_charge[20];

         /***************9:充电所用时长：2时00分16秒[28个字节]*****************/
        /*充电所用时长：02时00分16秒  B3E4 B5E7 CBF9 D3C3 CAB1 B3A4 3A 3032 CAB1 3030 B7D6 3136 C3EB*/
        char line_9_word[13];
        char line_9_charge_time[12];

        unsigned short  pdu_9_byte_27_return;      //换行 0B0A

        /***************10:充电    电量：002.12度[24个字节]*****************/
       /*充电    电量：2.12度  B3E4 B5E7 2020 2020 B5E7 C1BF 3A 0000322E3132 B6C8*/
       char line_10_word[13];
       char line_10_energy[6];
       unsigned short  pdu_10_byte_21_word_5;
       unsigned short  line_10_return;      //换行 0B0A

       /***************6:分割线---------------------------------[32个字节]*****************/
       /* 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D 2D2D*/
        char line_14_word[32];

       /***************15:欢迎光临，谢谢惠顾！[28个字节]*****************/
       /*欢迎光临，谢谢惠顾！ 2020 2020 2020 BBB6 D3AD B9E2 C1D9 2C D0BB D0BB BBDD B9CB 21 0B0A*/
       char line_15_word[26];
       unsigned short  pdu_end1_byte_27_return;      //换行 0B0A

       /***************16:青岛特来电新能源有限公司[32个字节]*****************/
       /*青岛特来电新能源有限公司   2020 2020 C7E0 B5BA CCD8 C0B4 B5E7 D0C2 C4DC D4B4 D3D0 CFDE B9AB CBBE 0B0A*/
       char line_16_word[28];
       unsigned short line_16_return;

       /***************17:QINGDAO TELD NEW ENERGY OD . LTD[32个字节]*****************/
       /*QINGDAO TELD NEW ENERGY OD . LTD    51494E4744414F 20 54454C44 20 4E4557 20 454E45524759 20 4F 20 2E 20 4C5444 0B0A*/
       char line_17_word[32];

       /***************18:网    址：http://www.teld.cn[28个字节]*****************/
       /*网    址：http://www.teld.cn	 202020 CDF8 D6B7 3A 687474703A2F2F 7777772E74656C642E636E 0B0A*/
       char line_18_word[30];
       unsigned short pdu_end4_byte_27_return;

        /***************19:客服电话：4001-300-001[32个字节]*****************/
       /*客服电话：4001300001	2020 2020 2020  BFCD B7FE B5E7 BBB0 3A 34303031 2D 333030 2D 303031*/
       char line_19_word[27];
       unsigned short pdu_end5_byte_31_return;


        /***************结束：换行+帧尾[12个字节]*****************/
        unsigned short pdu_printf_end_return_1;      //换行 0B0A
        unsigned short pdu_printf_end_return_2;      //换行 0B0A
        unsigned short pdu_printf_end_return_3;      //换行 0B0A
        unsigned short pdu_printf_end_return_4;      //换行 0B0A
        unsigned short pdu_printf_end_return_5;      //换行 0B0A
        unsigned short pdu_printf_end_return_6;      //换行 0B0A
        unsigned short pdu_printf_end_return_7;      //换行 0B0A
        unsigned char pdu_end_1;                          //帧尾_1 0x03
        unsigned char pdu_end_2;                          //帧尾_1 0xAA
}__attribute__ ((packed))UART_WR_PRINTF_INFORMATION_PDU;

#endif // TICKETDEV_H
