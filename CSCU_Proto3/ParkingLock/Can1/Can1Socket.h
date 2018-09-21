#ifndef CAN1SOCKET_H
#define CAN1SOCKET_H

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

class cCan1Socket
{
private:
    int g_canHandle;
    int iBitRateAttr;
    char chCan1NameAttr[10];

public:
    cCan1Socket();
    int SocketCan1_Init(const char *pCan1Name, int iBitRate);
    int SocketCan1_Close();
    int SocketCan1_Read(struct can_frame *frame);
    int SocketCan1_Write(struct can_frame *frame);

private:
    int SetCan1Bittiming(const char *pCan1Name, int iBitRate);

};

#endif // CANCOMMUNICATION_H
