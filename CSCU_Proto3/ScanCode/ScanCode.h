#ifndef SCANCODE_H
#define SCANCODE_H

#include <QObject>
#include <QTimer>
#include <QThread>
#include <fcntl.h>
#include <termios.h>
#include"ScanCode/DES.h"
#include "Infotag/CSCUBus.h"
#include "GeneralData/ModuleIO.h"
#include"ScanCode/DealScanCode.h"

class cScanCode : public CModuleIO
{
    Q_OBJECT
   public:

        cScanCode();
       ~cScanCode();
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
        int fd;
        int count;
        InfoMap dataMapScanCode;

        int init();
        int comm_open(char* portName);
        int set_nonblock_flag (int fp, int value);
        int hid_send(char *from,int len,char *to);
        int comm_write(int fd, void *buf, int len);
        int hid_rece(char *from,int len,char *to);
        int set_comm_bps(int fd,int bps);
        int comm_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity);
        int comm_read(int fd,void *buf, int buf_size,int timeout);
        void comm_clear(int fd);
        void comm_close(int fd);
        void showHEX(unsigned char *str,int len);


        QTimer * pCardSecTimer;
        cDealScanCode *pDealScanCode;
        cDES *pDES;
public slots:
        void slotFromBus(InfoMap CardMap, InfoAddrType CardType);

private slots:
    void ProcStartWork();
    int  readScanCode();     //
    void slotFormDealScanCode(InfoMap CardMap, InfoAddrType CardType);

signals:
    void sendScanCode(InfoMap);
    void sigToBus(InfoMap CardMap, InfoAddrType CardType);             // 向BUS发送数据

};

#endif // SCANCODE_H
