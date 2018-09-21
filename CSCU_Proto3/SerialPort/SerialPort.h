#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <termios.h>

#include <QDebug>

class cSerialPort
{
public:
    cSerialPort();
    ~cSerialPort();

    bool Open(const QString &dev);
    bool OpenSlic(const QString &dev);
    bool SetSpeed(const int baudrate);
    bool SetParity(int databits, int parity,int stopbits);
    bool Reset();
//    int Read(unsigned char *data,const int &len);
    int  Read(unsigned char *data, const int &len);
    int  Read(unsigned char *pdata, const int len, const int waitms);
//    int  Read(unsigned char *pdata, const int &len, const int &waitms);
    int  Write(const QByteArray &data);
    int  Write(const char *data,const int len);
    int  Write(const unsigned char *data,const int len);
    int  Close();

public:
    QString m_devname;
    int fd;
    int m_speed;
    int m_databits;
    int m_stopbits;
    int m_parity;
};

#endif // SERIALPORT_H
