#ifndef CARD_H
#define CARD_H

#include <QObject>
#include <QTimer>
#include <QThread>
#include "Infotag/CSCUBus.h"
#include "DevCache/DevCache.h"
#include "GeneralData/GeneralData.h"
#include "GeneralData/ModuleIO.h"
#include "ParamSet/ParamSet.h"
#include "SerialPort/SerialPort.h"

class Card : public CModuleIO    //继承main的类
{
    Q_OBJECT
    public:
        Card();
        ~Card();

    private:
        int InitModule (QThread *pThread);     //根据配置选项初始化
        int RegistModule();             //注册设备到总线
        int StartModule ();                                   //启动模块
        int StopModule ();                                   //停止模块
        int ModuleStatus ();                                //模块工作状态
    signals:
        void sigToBus(InfoMap CardMap, InfoAddrType CardType);             // 向BUS发送数据
    public slots:
        void slotFromBus(InfoMap CardMap, InfoAddrType CardType);       // 接收BUS数据-显示屏申请读卡
        void slotThreadStartCard();                                                  //card开始工作
        int readCardNum();//读卡片信息函数
	private:
        //内部定义数据
        int  gCardSerial;
        bool CardWorkStartFlag;                     //开始工作标识位
        cSerialPort  CardSerialPort;               //串口类
        bool bCardSerialPort;
        int iCardType; //1=CPU卡  2=智能出行卡  3=招商银行卡

        unsigned char sCardnumberBuf[8];         //接收卡号

        unsigned char ucCanID;
        int ucCardType;//卡类型
        int count; //定时器计数
        //外部输入参数
        ParamSet * pParamSet;
    private:
        QTimer pCardSecTimer;
        void SerialCardInit();                                              //刷卡器串口初始化函数
        int readCardNumCpu();  //读CPU卡卡号
        int readCardNumIC();   //读ID卡
        int readCardNumICNumber();   //读取IC卡的固件卡号信息

        bool SendCardNumToBus();//将取出的卡号与CAN ID发到Bus上
        bool PackageOutCardNum(InfoMap &CardMap, InfoAddrType &CardType);  //将CAN ID/卡号打包
};

#endif // CARD_H
