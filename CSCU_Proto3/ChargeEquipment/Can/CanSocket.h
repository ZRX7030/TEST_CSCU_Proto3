#ifndef CANSOCKET_H
#define CANSOCKET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/can/error.h>
#include <linux/netlink.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <net/if.h>
#include <fcntl.h>
#include <sys/un.h>
#include <sys/time.h>
#include <time.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <string.h>

class cCanSocket
{
private:
    int g_canHandle;
    int iBitRateAttr;
    char chCanNameAttr[10];

public:
    cCanSocket();
    int SocketCan_Init(const char *pCanName, int iBitRate);
    int SocketCan_Close();
    int SocketCan_Read(struct can_frame *frame);
    int SocketCan_Write(struct can_frame *frame);

private:
    int SetCanBittiming(const char *pCanName, int iBitRate);

};

#endif // CANCOMMUNICATION_H
