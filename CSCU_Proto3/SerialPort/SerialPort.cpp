#include "SerialPort.h"

int speed_arr[] = { B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200};
int name_arr[] = { 115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200};

cSerialPort::cSerialPort()
{
    m_devname.clear();
    fd = -1;
    m_speed = 115200;
    m_databits = 8;
    m_parity = 'n';
    m_stopbits = 1;
}
cSerialPort::~cSerialPort()
{
    ;
}

bool cSerialPort::SetSpeed(const int speed)
{
    unsigned int i;
    int status;
    struct termios Opt;
    tcgetattr(fd, &Opt);
    for (i= 0; i < sizeof(speed_arr) / sizeof(int); i++)
    {
        if (speed == name_arr[i])
        {
            tcflush(fd, TCIOFLUSH);
            cfsetispeed(&Opt, speed_arr[i]);
            cfsetospeed(&Opt, speed_arr[i]);
            status = tcsetattr(fd, TCSANOW, &Opt);
            if (status != 0)
            {
                perror("tcsetattr fd1");
                return FALSE;
            }
            tcflush(fd, TCIOFLUSH);
        }
    }
    m_speed = speed;
    return TRUE;
}

bool cSerialPort::SetParity(int databits,int parity,int stopbits)
{
    struct termios options;
    if (tcgetattr(fd, &options) != 0)
    {
        perror("SetupSerial 1");
        return (FALSE);
    }
    options.c_cflag &= ~CSIZE;
    switch (databits)
    {
        case 7:
            options.c_cflag |= CS7;
            break;
        case 8:
            options.c_cflag |= CS8;
            break;
        default:
            fprintf(stderr, "Unsupported data size\n");
            return (FALSE);
    }
    switch (parity)
    {
        case 'n':
        case 'N':
            options.c_cflag &= ~PARENB;
            options.c_iflag &= ~INPCK;
            break;
        case 'o':
        case 'O':
            options.c_cflag |= (PARODD | PARENB);
            options.c_iflag |= INPCK;
            break;
        case 'e':
        case 'E':
            options.c_cflag |= PARENB;
            options.c_cflag &= ~PARODD;
            options.c_iflag |= INPCK;
            break;
        case 'S':
        case 's':
            options.c_cflag &= ~PARENB;
            options.c_cflag &= ~CSTOPB;
            break;
        default:
            fprintf(stderr, "Unsupported parity\n");
            return (FALSE);
    }

    switch (stopbits)
    {
        case 1:
            options.c_cflag &= ~CSTOPB;
            break;
        case 2:
            options.c_cflag |= CSTOPB;
            break;
        default:
            fprintf(stderr, "Unsupported stop bits\n");
            return (FALSE);
    }
    /* Set input parity option */
    if (parity != 'n')
    {
        options.c_iflag |= INPCK;
    }
    tcflush(fd, TCIFLUSH);

//  options.c_iflag &= ~(IXON | IXOFF | IXANY);

    options.c_cc[VTIME] = 1; // 1 seconds
    options.c_cc[VMIN] = 0;
    if (tcsetattr(fd, TCSANOW, &options) != 0)
    {
        perror("SetupSerial faild!");
        return (FALSE);
    }
    m_stopbits = stopbits;
    m_databits = databits;
    m_parity   = parity;
    return (TRUE);
}

bool cSerialPort::OpenSlic(const QString &dev)
{
    if((fd = open(dev.toAscii().data(),O_RDWR))>0)
    {
        if(fd!=-1)
        {
            m_devname = dev;
            return TRUE;
        }
    }
    return FALSE;
}

bool cSerialPort::Open(const QString &name)
{
    if ((fd = open(name.toAscii().data(),O_RDWR)) > 0)
    {
        struct termios init_settings,new_settings;
        tcgetattr(fd,&init_settings);
        new_settings = init_settings;
        new_settings.c_lflag &= ~(ICANON|ISIG);
        new_settings.c_lflag &= ~ECHO;
        new_settings.c_iflag &= ~ICRNL;
        new_settings.c_iflag &= ~INLCR;
        new_settings.c_iflag &= ~IXON;
        new_settings.c_iflag &= ~IXANY;
        new_settings.c_iflag &= ~IXOFF;
        new_settings.c_iflag &= ~IGNBRK;
        new_settings.c_oflag &= ~ONLCR;
        new_settings.c_oflag &= ~OCRNL;
        if (tcsetattr(fd, TCSANOW, &new_settings) != 0)
        {
            perror("SetupSerial faild!");
        }
        else
        {
            m_devname = name;
            return TRUE;
        }
    }

    return FALSE;
}

int cSerialPort::Close()
{
    if (fd > 0)
    {
        return close(fd);
    }
    return -1;
}

bool cSerialPort::Reset()
{
    if (fd > 0)
    {
        Close();
    }
    if (Open(m_devname) && SetSpeed(m_speed) && SetParity(m_databits,m_parity,m_stopbits))
    {
        return TRUE;
    }
    return FALSE;
}

//int cSerialPort::Read(unsigned char *data,const int &len)
//{
//    int ret = read(fd, data, len);
//    return ret;
//}

//实际使用串口读函数------(使用)
int cSerialPort::Read(unsigned char *data,const int &len)
{
    int ret = 0;
    ret = read(fd, data, len);
    return len;
//    int ret = 0,retry = 3,left = len;
//    if (len > 20) retry = 10;
//    unsigned char *pdata = data;
//    do{
//        if (left == 0)
//            break;
//        ret = read(fd, pdata, left);
//        if (ret > 0)
//        {
//            left -= ret;
//            pdata += ret;
//        }
//    }while(retry--);
//    if (len != left)
//    {
////      for(int i=0;i<len-left;i++)
////          {
////              Dprintf(stderr,"%02x ",*(pdata+i));
////              if (i > 0 && i%15 == 0)Dprintf(stderr,"\n");
////          }
//    }
//    return len-left;
}
int cSerialPort::Read(unsigned char *pdata, const int len, const int waitms)
{
    int result;
    fd_set inputs;
    struct timeval timeout;
    FD_ZERO(&inputs);
    FD_SET(fd,&inputs);
    timeout.tv_sec = waitms/1000;
    timeout.tv_usec = (waitms%1000)*1000;
    result = select(FD_SETSIZE,&inputs,(fd_set *)NULL,(fd_set *)NULL,&timeout);
    if (result == 0)
    {
        return -1;
    }
    if (result == -1)
    {
        return -2;
    }
    int ret = read(fd,pdata,len);
    return ret;
}
//int cSerialPort::Read(unsigned char * pdata,const int &len,const int &waitms)
//{
//    memset(pdata,0,len);

//    int ret = Read(pdata,len,waitms);

//    return ret;

//}

int cSerialPort::Write(const QByteArray &data)
{
    int iWrite=0,iOne=0;
    int iretyr=0;
    char *psData = (char *)data.data();
    while(1)
    {
        iOne = write(fd, &psData[iWrite], data.length()-iWrite);
        if(iOne>0)
            iWrite +=iOne;
        if(iWrite==data.length())
            break;
        iretyr ++;
        if(iretyr>=5)
            break;
    }
    return iWrite;
   // return write(fd, data.data(), data.length());
}
int cSerialPort::Write(const char *data,const int len)
{
    return write(fd, data, len);
}
int cSerialPort::Write(const unsigned char *data,const int len)
{
//    for(int i = 0; i < len; i++)
//    {
//    }

    return write(fd, data, len);
}
