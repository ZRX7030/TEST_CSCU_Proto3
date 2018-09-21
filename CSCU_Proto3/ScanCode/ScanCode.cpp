#include <stdio.h>
#include <stdlib.h>
#include <QDebug>
#include "ScanCode/ScanCode.h"

int cScanCode::readScanCode()
{
    int len = 0;
    char send_to[64] = {0};
    char rece_to[4096] = {0};
    int res = 0;
    char buf[4096] = {0};

    count++;
    //open the HID-POS
    int ret = init();
    if(ret == 0)
    {
        if(pCardSecTimer->isActive ())
        {
            pCardSecTimer->stop ();
            comm_close(fd);
        }
    }
        comm_clear(fd);

//        the trigger command into message
        len = hid_send((char *)"\x1b\x31",2,send_to);
        res = comm_write(fd,send_to,len);
        if(res != len)
        {
            return -1;
        }
        res = comm_read(fd,buf,sizeof(buf),2000);

        if(res>0)
        {
            res = hid_rece(buf,res,rece_to);
//            showHEX((unsigned char*)rece_to,res);
            if(res == 1 && *rece_to == 0x06)
            {
                res = comm_read(fd,buf,sizeof(buf),1000);//  1000:超时时间
//                showHEX((unsigned char*)buf,res);
                if(res > 0)
                {
                    res = hid_rece(buf,res,rece_to);
                    dataMapScanCode.clear();
                    dataMapScanCode.insert(Addr_ScanCode,QByteArray(rece_to,res + 24));
                    emit sendScanCode(dataMapScanCode);
                    dataMapScanCode.clear();
                    if(pCardSecTimer->isActive ())
                    {
                        pCardSecTimer->stop ();
                        comm_close(fd);
                    }
                }
            }
        }
        if(count >25)
        {
            if(pCardSecTimer->isActive ())
            {
                pCardSecTimer->stop ();
                comm_close(fd);
            }
        }
    return 0;
}

//print the string by HEX
void cScanCode::showHEX(unsigned char *str,int len)
{
    int i = 0;
    for(i=0;i<len;i++)
    {
        str++;
    }
}

void cScanCode::comm_close(int fd)
{
    if (fd >= 0)
    {
       // tcsetattr(fd, TCSANOW, &old_tio); //还原串口配置
        close(fd);
    }
}

int cScanCode::hid_rece(char *from,int len,char *to)
{
    int packlen = 0;
    int curlen = 0;
    char *cur = from;
    char *cur_to = to;

    if(len<=0 || !(len%64 == 0 || len%65 == 0))
    {
        return -1;
    }
    while(1)
    {
        if(*cur != 0x02)
        {
            cur++;
            continue;
        }
        curlen = *(cur+1);
        memcpy(cur_to+packlen,cur+2,curlen);
        packlen += curlen;
        if(*(cur+63) != 0x01)
        {
            break;
        }
        cur += 64;
    }
    *(cur_to+packlen) = 0;
    return packlen;
}

int cScanCode::comm_read(int fd,void *buf, int buf_size,int timeout)
{
    int ret = 0,len = 0;
    char* tmp=(char* )buf;
    fd_set rset;
    struct timeval tv;
    memset((char*)&tv, 0, sizeof(struct timeval));

    if (fd < 0)
    {
        return -1;
    }

    FD_ZERO(&rset);
    FD_SET(fd, &rset);
    if(timeout == -1){
        //一直等待
        ret = select(fd+1, &rset, NULL, NULL, NULL);
    }
    else if (timeout == 0){
        //检查完立即返回
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        ret = select(fd+1, &rset, NULL, NULL, &tv);
    }else if (timeout > 0){
        //等待指定时间
        tv.tv_sec = timeout /1000;
        tv.tv_usec = (timeout %1000) * 1000;
        ret = select(fd+1, &rset, NULL, NULL, &tv);
    }
    if(ret < 0){
        return ret;
    }
    else if(ret == 0){
        return ret;
    }
    else if(ret > 0){
        if(FD_ISSET(fd, &rset))
        {
            do
            {
                usleep(20000);
                ret=read(fd, tmp+len, buf_size-len);
                if(ret>0)
                {
                    len+=ret;
                }

            }while(ret>0);
            return len;
        }

    }
    return 0;


}

int cScanCode::comm_write(int fd, void *buf, int len)
{
    int ret=0;
    char *pchar=(char *)buf;
    unsigned int iSize=len;
    if(fd<0)
        return 0;
    while(iSize != 0){
        if (iSize > 256)
            ret=write(fd, pchar, 256);
        else
            ret=write(fd, pchar, iSize);
        if(ret <=0)
            return -1;
        iSize = iSize-ret;
        pchar = &((char *)buf)[len-iSize];

    }
    return len;
}

int cScanCode::hid_send(char *from,int len,char *to)
{
    if(len<=0 || len>62)
    {
        return -1;
    }
//	memset(to,0,64);
    to[0] = 0x04;
    to[1] = len;
    memcpy(to+2,from,len);

    return len+2;
}

void cScanCode::comm_clear(int fd)
{
    if (fd >= 0)
    {
       tcflush(fd, TCIOFLUSH);
    }
}

int cScanCode::comm_open(char* portName)
{
    //打开串口

    int fd = open(portName, O_RDWR|O_NOCTTY|O_NDELAY);
    if (fd == -1)
    {
        return -1;

    }

    set_nonblock_flag(fd,1);

    if (!comm_Set(fd,9600,0,8,1,'n'))
    {
        return -1;
    }

    return fd;
}

int cScanCode::set_comm_bps(int fd,int bps)
{
    int   i;
    //	int   status;
    int   speed_arr[] = {B115200, B57600,B38400, B19200, B9600, B4800, B2400, B1200, B300 };
    int   name_arr[] = {115200,57600,38400,  19200,  9600,  4800,  2400,  1200, 300};

    struct termios options;

    if  ( tcgetattr( fd,&options)  !=  0)//得到与fd指向对象的相关参数，并将它们保存于options,该函数,还可以测试配置是否正确，该串口是否可用等。若调用成功，函数返回值为0，若调用失败，函数返回值为1.
    {
        return -1;
    }
    //设置串口输入波特率和输出波特率
    for ( i= 0;  (unsigned int)i < sizeof(speed_arr) / sizeof(int);  i++)
    {
        if  (bps == name_arr[i])
        {
            cfsetispeed(&options, speed_arr[i]);
            cfsetospeed(&options, speed_arr[i]);
        }
    }
    //激活配置 (将修改后的termios数据设置到串口中）
    if (tcsetattr(fd,TCSANOW,&options) != 0)
    {
        return -1;
    }

    return 0;
}

/*******************************************************************
* 名称：                comm_Set
* 功能：                设置串口数据位，停止位和效验位
* 入口参数：        fd         串口文件描述符
*                              speed      串口速度
*                              flow_ctrl  数据流控制
*                           databits   数据位   取值为 7 或者8
*                           stopbits   停止位   取值为 1 或者2
*                           parity     效验类型 取值为N,E,O,,S
*出口参数：              正确返回为1，错误返回为-1
*******************************************************************/

int cScanCode::comm_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity)
{
    flow_ctrl = 0;
    databits = 0;
    stopbits = 0;
    parity = 0;
    set_comm_bps(fd,speed);

    struct termios options;

    if  ( tcgetattr( fd,&options)  !=  0)//得到与fd指向对象的相关参数，并将它们保存于options,该函数,还可以测试配置是否正确，该串口是否可用等。若调用成功，函数返回值为0，若调用失败，函数返回值为1.
    {
        return -1;
    }
    //设置串口输入波特率和输出波特率
    //     for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
    // 	{
    // 		if  (speed == name_arr[i])
    // 		{
    // 			cfsetispeed(&options, speed_arr[i]);
    // 			cfsetospeed(&options, speed_arr[i]);
    // 		}
    // 	}


    //设置等待时间和最小接收字符
    options.c_cc[VTIME] = 1; // 读取一个字符等待1*(1/10)s
    options.c_cc[VMIN] = 1; // 读取字符的最少个数为1
    //如果发生数据溢出，接收数据，但是不再读取

    // Set non-canonical(raw) mode
    options.c_lflag &= ~(ICANON|ECHO|ECHOE|ISIG); // Set raw input and output

    options.c_cflag |= CLOCAL|CREAD;
    options.c_cflag &= ~CRTSCTS; // Disable hardware flow control

    options.c_iflag |= IGNBRK; // ignore break condition (-BRKINT)
    options.c_iflag &= ~(IXON|IXOFF); // Disable software flow control (-IXANY)
    options.c_iflag &= ~INPCK; // enable parity checking
    options.c_iflag &= ~(IGNPAR|PARMRK); //Read parity error byte as 0x00
    //#ifdef IUCLC
    options.c_iflag &= ~(ISTRIP|INLCR|ICRNL|IGNCR|IUCLC);
    // #else // MAC OS X 10.4/10.5 don't have IUCLC
    // 	tioa.c_iflag &= ~(ISTRIP|INLCR|ICRNL|IGNCR);
    // #endif

    options.c_oflag &= ~OPOST; // Disable all output processing
    // OPOST will cause an LF to become CR LF .


    tcflush(fd,TCIFLUSH);
    //激活配置 (将修改后的termios数据设置到串口中）
    if (tcsetattr(fd,TCSANOW,&options) != 0)
    {
        return -1;
    }
    //	    cur_tio = options;
    return 1;

}

//设置非阻塞模式
int cScanCode::set_nonblock_flag (int fp, int value)
{
    int oldflags = fcntl (fp, F_GETFL, 0);
    /* If reading the flags failed, return error indication now. */
    if (oldflags == -1)
        return -1;
    /* Set just the flag we want to set. */
    if (value != 0)
        oldflags |= O_NONBLOCK;
    else
        oldflags &= ~O_NONBLOCK;

    /* Store modified flag word in the descriptor. */
    return fcntl (fp, F_SETFL, oldflags);
}

//------------------------------------------------------CSCU BUS接口相关------------------------------------------------------//
///
/// \brief slotFromBusCard 接收BUS数据
/// \param qInfoMap
/// \param InfoType
///
void cScanCode::slotFromBus(InfoMap CardMap, InfoAddrType CardType)
{
    switch (CardType)
    {
    case AddrType_ScreenApplyReadCard:  //主题一：显示屏申请读卡
        if(CardMap.contains(Addr_CanID_Comm))
        {
             pDealScanCode->ucCanID = CardMap[Addr_CanID_Comm].at(0);  //    //确定CAN地址
        }
        if(!pCardSecTimer->isActive())
        {
            count = 0;
            pCardSecTimer->start (400);  //读卡
        }
        break;
    case AddrType_ScreenApplyStopCard:  //主题一.一：显示屏申请结束读卡
        if(pCardSecTimer->isActive ())
        {
            pCardSecTimer->stop ();
            comm_close(fd);
        }
        break;
    default:
        break;
    }
}

int cScanCode::init()
{
    //open the HID-POS
    fd = comm_open((char *)"/dev/hidraw0");

    if(fd == -1)
    {
        fd = comm_open((char *)"/dev/hidraw1");
        if(fd == -1)
        {
            fd = comm_open((char *)"/dev/hidraw2");
            if(fd == -1)
            {
                return 0;
            }
        }
    }
    return 1;
}

void cScanCode::slotFormDealScanCode(InfoMap CardMap, InfoAddrType CardType)
{
    emit sigToBus(CardMap,CardType);
}

cScanCode::cScanCode()
{
    pDES = new cDES();
    pDealScanCode = new cDealScanCode();

    connect(this,SIGNAL(sendScanCode(InfoMap)),pDES,SLOT(ProcDES(InfoMap)));
    connect(pDES,SIGNAL(sendDES(char *)),pDealScanCode,SLOT(ProcDealScanCode(char *)));
    connect(pDealScanCode,SIGNAL(sendToScanCode(InfoMap,InfoAddrType)),this,SLOT(slotFormDealScanCode(InfoMap,InfoAddrType)));
}

cScanCode::~cScanCode()
{
    delete pDealScanCode;
    delete pDES;
}

//启动模块
void cScanCode::ProcStartWork()
{
//    init();
    pCardSecTimer = new QTimer();
    connect(pCardSecTimer, SIGNAL(timeout()), this, SLOT(readScanCode()));
//    pCardSecTimer->start(1000);
}


//根据配置选项初始化
int cScanCode::InitModule( QThread* pThread)
{
    m_pWorkThread = pThread;

    this->moveToThread(m_pWorkThread);
    QObject::connect(m_pWorkThread, SIGNAL(started()), this, SLOT(ProcStartWork()));
    return 0;
}

//注册设备到总线
int cScanCode::RegistModule()
{
	QList<int> list;

	//-----------------刷卡远程充电相关主题--------------//
	list.append (AddrType_ScreenApplyReadCard); //主题一：显示屏申请读卡
	list.append(AddrType_ScreenApplyStopCard);  //主题一.一：显示屏申请结束读卡

	CBus::GetInstance()->RegistDev (this,list);    //模块标志号-刷卡器

    return 0;
}

//启动模块
int cScanCode::StartModule()
{
    m_pWorkThread->start();
    return 0;
}

//停止模块
int cScanCode::StopModule()
{
    return 0;
}

//模块工作状态
int cScanCode::ModuleStatus()
{
    return 0;
}

CModuleIO* CreateDevInstance()
{
    return new cScanCode();
}

void DestroyDevInstance(CModuleIO* pModule)
{

    if(pModule)
    {
        delete pModule;
    }
}
