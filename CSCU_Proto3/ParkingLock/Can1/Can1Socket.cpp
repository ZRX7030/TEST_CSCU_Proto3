#include "Can1Socket.h"

cCan1Socket::cCan1Socket()
{
    ;
}

int cCan1Socket::SetCan1Bittiming(const char * pCan1Name , int iBitRate)
{
    //socket can setting
    struct can_bittiming {
        __u32 bitrate;		/* Bit-rate in bits/second */
        __u32 sample_point;	/* Sample point in one-tenth of a percent */
        __u32 tq;			/* Time quanta (TQ) in nanoseconds */
        __u32 prop_seg;		/* Propagation segment in TQs */
        __u32 phase_seg1;	/* Phase buffer segment 1 in TQs */
        __u32 phase_seg2;	/* Phase buffer segment 2 in TQs */
        __u32 sjw;			/* Synchronisation jump width in TQs */
        __u32 brp;			/* Bit-rate prescaler */
    };

    struct can_bittiming 	bt;
    char				 	str[256];

    memset( &bt, 0, sizeof(bt));
    bt.bitrate = iBitRate;
    switch( bt.bitrate )
    {
    case 1000000:
        bt.tq = 84;					// ns
        bt.prop_seg = 1;
        bt.phase_seg1 = 8;
        bt.phase_seg2 = 2;
        bt.sjw = 2;
        break;
    case 800000:
        bt.tq = 84;					//ns
        bt.prop_seg = 4;
        bt.phase_seg1 = 8;
        bt.phase_seg2 = 2;
        bt.sjw = 2;
        break;
    case 500000:
        bt.tq = 125;				//ns
        bt.prop_seg = 5;
        bt.phase_seg1 = 8;
        bt.phase_seg2 = 2;
        bt.sjw = 2;
        break;
    case 250000:
        bt.tq = 250;				//ns
        bt.prop_seg = 5;
        bt.phase_seg1 = 8;
        bt.phase_seg2 = 2;
        bt.sjw = 2;
        break;
    case 125000:
        bt.tq = 500;				//ns
        bt.prop_seg = 5;
        bt.phase_seg1 = 8;
        bt.phase_seg2 = 2;
        bt.sjw = 2;
        break;

    case 100000:
        bt.tq = 625;
        bt.prop_seg = 5;
        bt.phase_seg1 = 8;
        bt.phase_seg2 = 2;
        bt.sjw = 2;
        break;

    case 60000:
        bt.tq = 1041;
        bt.prop_seg = 5;
        bt.phase_seg1 = 8;
        bt.phase_seg2 = 2;
        bt.sjw = 2;
        break;

    case 50000:
        bt.tq = 1250;
        bt.prop_seg = 5;
        bt.phase_seg1 = 8;
        bt.phase_seg2 = 2;
        bt.sjw = 2;
        break;

    case 20000:
        bt.tq = 3125;
        bt.prop_seg = 5;
        bt.phase_seg1 = 8;
        bt.phase_seg2 = 2;
        bt.sjw = 2;
        break;

    case 10000:
        bt.tq = 100000 / 16;
        bt.prop_seg = 5;
        bt.phase_seg1 = 8;
        bt.phase_seg2 = 2;
        bt.sjw = 2;
        break;
    }

    memset(str,0,sizeof(str));
    sprintf(str,"ifconfig %s down",pCan1Name);
    system( str );

    memset(str,0,sizeof(str));
    sprintf( str, "ip link set %s type can tq %d prop-seg %d phase-seg1 %d phase-seg2 %d sjw %d restart-ms 100",
             pCan1Name,bt.tq, bt.prop_seg, bt.phase_seg1, bt.phase_seg2, bt.sjw );
    system( str );    

    return bt.bitrate;
}

int cCan1Socket::SocketCan1_Init(const char *pCan1Name, int iBitRate)
{
    int loopback = 0; /* 0 = disabled, 1 = enabled (default) */
    struct sockaddr_can addr;
    struct can_frame g_canFrame;
    struct ifreq ifr;
    memcpy(chCan1NameAttr,pCan1Name,strlen(pCan1Name));
    iBitRateAttr = iBitRate;

    char szTemp[256] = {0};
    int ret;
    //struct can_filter filter;
    sprintf(szTemp,"ifconfig %s down",pCan1Name);
    system( szTemp);

    //baudrate = 250000;

    //    Dprintf( "bitrate is %d\n", baudrate );
//    SetCanBittiming( pCanName, iBitRate);

    g_canHandle = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    //    Dprintf( "SOCK_RAW can sockfd:%d\n", g_canHandle );
    if( g_canHandle < 0 )
    {
        return -1;
    }

    //disbale loopback
    setsockopt(g_canHandle, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &loopback, sizeof(loopback));

    //set can error filter
    can_err_mask_t err_mask = ( CAN_ERR_TX_TIMEOUT | CAN_ERR_BUSOFF | CAN_ERR_BUSERROR );
    setsockopt(g_canHandle, SOL_CAN_RAW, CAN_RAW_ERR_FILTER, &err_mask, sizeof(err_mask));

    strcpy(ifr.ifr_name, pCan1Name );
    ret = ioctl(g_canHandle, SIOCGIFINDEX, &ifr);
    if( ret < 0 )
    {
        return -1;
    }

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    bind(g_canHandle, (struct sockaddr *)&addr, sizeof(addr));

    g_canFrame.can_id = 0x02 ;//  | CAN_EFF_FLAG;//远程帧，标准帧去掉CAN_EFF_FLAG
    g_canFrame.can_dlc = 8;//数据长度

    memset( g_canFrame.data, 0x32, g_canFrame.can_dlc );
    memset(szTemp,0,sizeof(szTemp));
    SetCan1Bittiming( pCan1Name, iBitRate);
    sprintf(szTemp,"ifconfig %s up",pCan1Name);
    system(szTemp);

    return 1;
}

int cCan1Socket::SocketCan1_Close()
{
    if(g_canHandle>0)
    {
        close(g_canHandle);
        g_canHandle = 0;
        return 1;
    }
    else
    {
        return -1;
    }
}

int cCan1Socket::SocketCan1_Read(struct can_frame *frame)
{
    int nbytes, nfds;
    fd_set rfds;
    struct timeval tv;

    tv.tv_sec = 0;
    tv.tv_usec = 200000;
    FD_ZERO(&rfds);
    FD_SET(g_canHandle, &rfds);

    nfds = select(g_canHandle+1, &rfds, NULL, NULL, &tv);
    if(nfds == 0)
    {
        return -1;
    }
    if(nfds < 0)
    {
        return -1;
    }
    nbytes = read(g_canHandle, frame, sizeof(struct can_frame));
    if (nbytes < 0)
    {
//        perror("can raw socket read");
        return -1;
    }
    if( nbytes < (int)sizeof(struct can_frame))
    {
        fprintf(stderr, "read: incomplete CAN frame\n");
        return -1;
    }
    if( (*frame).can_id & CAN_ERR_FLAG )
    {
    }
    return nbytes;
}

int cCan1Socket::SocketCan1_Write(struct can_frame *frame)
{
    int nbytes;

    nbytes = write(g_canHandle, frame, sizeof(struct can_frame));
#ifdef CAN_TEST_SWITCH
    Dprintf("[SocketCan_Write]=%d\n",nbytes);
#endif
    //gstr_CanLog->AddCommLog(2,0, sizeof(struct can_frame),(char *)frame);
    return nbytes;
}
