/*
 * commfunc.h
 *
 * 类描述：公共函数类
 * 创建: LDM 20150414
 * 修改记录:
 * 见 commfunc.cpp
 */
#include "commfunc.h"
#include <ctype.h>
#include <stdio.h>
#include <QDebug>
#include <QDir>
#include <net/if.h>
#include <arpa/inet.h>
#include <netdb.h>

// 函数功能:
//     实现数据的大小端转换
// 函数参数:
// 输入:
//     pOriginal  —— 等待转化的数据
//     iLen   —— 等待转化的数据长度
// 函数返回值:
//     成功位ture，否则位FALSE
bool ConvertDataFormat( unsigned char* pOriginal, int iLen )
{
    int i, j;
    unsigned char tempChar;

    i = iLen / 2;
    for (j = 0; j < i; j++)
    {
        tempChar = pOriginal[j];
        pOriginal[j] = pOriginal[iLen - j - 1];
        pOriginal[iLen - j - 1] = tempChar;
    }
    return true;
}

unsigned char char_to_bin(unsigned char bchar)
{
    if ((bchar >= '0') && (bchar <= '9'))
        return (bchar - '0');
    else {
        if ((bchar >= 'A') && (bchar <= 'F'))
            return (bchar - 'A' + 10);
        if ((bchar >= 'a') && (bchar <= 'f'))
            return (bchar - 'a' + 10);
        else
            return(0);
    }
}

//16进制Asci字符串转化为bin数据
bool AsciToBin(unsigned char *psAsci,int iLen,unsigned char *psOut)
{
    if(iLen%2)
        return false;

    for(int i=0;i<iLen/2;i++)
    {
        psOut[i] = char_to_bin(psAsci[i*2])*16+char_to_bin(psAsci[i*2+1]);
    }
    return true;
}
//获得异或和校验值
unsigned char get_XOR(unsigned char * psBuf, int nCount  )
{
    unsigned char c=0;

    int i;
    for( i = 0; i<nCount; i++ )
    {
        c ^= psBuf[i];
    }

    return c;
}

// 函数功能:
//     实现十六进制数据向long数据的转化
// 函数参数:
// 输入:
//     pHex —— 等待转化的十六进制数据数组
//     iLen —— 等待转化的十六进制数据数组的长度
// 输出:
//     l —— 转化得到的long类型数据
// 函数返回值:
//     成功位ture，否则位FALSE
bool HexToLong( unsigned char* pHex, unsigned int iLen, unsigned long* l )
{
    unsigned int iTemp;

    if (iLen > (sizeof(float) * 2))
    {
        return true;
    }

    *l = 0;
    int iAscii;
    iTemp = 0;

    unsigned int j[] = {1, 256, 65536, 16777216};
    for (int i = 0; i < (int)iLen; i ++)
    {
        iAscii = toascii(pHex[i]);
        if (pHex[i] & 0x80)
        {
            iAscii += 128;
        }

        iTemp += iAscii * j[iLen - i - 1];
    }


    *l = (unsigned long)iTemp;

    return true;
}

// 函数功能:
//     实现long类型数据向十六进制数据的转化
// 函数参数:
// 输入:
//     l —— 等待转化的float类型数据
// 输出:
//     pHex —— 转化得到的十六进制数据
// 函数返回值:
//     成功位ture，否则位FALSE
bool LongToHex( long l, unsigned char* pHex )
{
    int len = sizeof(long)-1;
    for (int i = len; i >= 0; i--)
    {
        pHex[len-i] = (unsigned char)( (l & (0xff<<(i*8)))>>(i*8) );
    }

    return true;
}

// 函数功能:
//     实现字符类型数据向BCD的转化
// 函数参数:
// 输入:
//     pChar —— 等待转化的字符数组
//     iLen —— 等待转化的字符数据的长度
// 输出:
//     pBCD —— 转化得到的BCD码
// 函数返回值:
//     成功位ture，否则位FALSE
bool CharToBCD( unsigned char* pChar, int iLen, unsigned char* pBCD )
{
    int i;
    int iBCDNum = iLen / 2;

    for (i = 0; i < iBCDNum; i ++)
    {
        if ((pChar[2 * i]>='0') && (pChar[2 * i] <= '9'))
        {
            pBCD[i] = pChar[2 * i]-'0';
        }
        else if ((pChar[2 * i]>='a') && (pChar[2 * i] <= 'z'))
        {
            pBCD[i] = pChar[2 * i]-'a'+10;
        }
        else
            pBCD[i] = pChar[2 * i]-'A'+10;

        if ((pChar[2*i+1]>='0') && (pChar[2*i+1] <= '9'))
        {
            pBCD[i] = (pBCD[i]<<4) | (pChar[2*i+1]-'0');
        }
        else if ((pChar[2*i+1]>='a') && (pChar[2*i+1] <= 'z'))
        {
            pBCD[i] = (pBCD[i]<<4) | (pChar[2*i+1]-'a'+10);
        }
        else
            pBCD[i] = (pBCD[i]<<4) | (pChar[2 * i+1]-'A'+10);
    }
    return true;
}
///
/// \brief UnsignedCharToBCD将16进制的数据转换为10进制BCD码,每个字节表示的数据不能超过99
/// \param pChar
/// \param iLen
/// \param pBCD
/// \return
///
bool UnsignedCharToBCD( unsigned char* pUnChar, int iLen, unsigned char* pBCD)
{
    for(int i = 0;i < iLen;i++)
    {
        if(pUnChar[i] >99)
        {
            return FALSE;
        }
        else
        {
            pBCD[i] = pUnChar[i]/10*16+pUnChar[i]%10;
        }
    }
    return TRUE;
}

// 函数功能:
//     实现BCD向字符类型数据的转化
// 函数参数:
// 输入:
//     pBCD —— 等待转化的BCD码
//     iLen —— 等待转化的BCD码长度
// 输出:
//     pChar —— 转化得到的字符数组
// 函数返回值:
//     成功位ture，否则位FALSE
bool BCDToChar( unsigned char* pBCD, int iLen, unsigned char* pChar )
{
    int i = 0;
    unsigned char chHigh; // 保存BCD码中的高位
    unsigned char chLow; // 保存BCD码中的低位

    for (i = 0; i < iLen; i ++)
    {
        chLow = pBCD[i] & 0x0f;
        chHigh = (pBCD[i] & 0xf0) >> 4;

        if ((chHigh > 15) || (chLow > 15))
        {
            return FALSE;
        }

        // 转化得到第一个字符
        if (chHigh<=9)
        {
            pChar[i * 2] = chHigh + '0';
        }
        else
        {
            pChar[i * 2] = chHigh + 'A' - 10;
        }

        // 转化得到第二个字符
        if (chLow<=9)
        {
            pChar[i * 2+1] = chLow + '0';
        }
        else
        {
            pChar[i * 2+1] = chLow + 'A' - 10;
        }
    }

    return true;
}

QString LeftZeroStr(QString strOld,int iLen)
{
   QString sTemp = "000000000000000000";
   if(strOld.length() == iLen)
           return strOld;
   else
   {
       if(strOld.length()<iLen)
       {
           return sTemp.left(iLen - strOld.length()) + strOld;
       }
       else
       {
           return strOld.left(iLen);
       }
   }
}

QString RightZeroStr(QString strOld,int iLen)
{
    QString sTemp = "000000000000000000";

    if(strOld.length() == iLen)
            return strOld;
    else
    {
        if(strOld.length()<iLen)
            return strOld + sTemp.left(iLen - strOld.length());
        else
            return strOld.right(iLen);
    }
}
//字节1	字节2	字节3	字节4	字节5	字节6	     字节7  	      字节8
//BCD	    BCD	    BCD	     BCD	BCD	    ASCII码	  BCD	          BCD
//37	         02	      12	      00	      01	      0~Z	      11	               01
//区号	                            站号	               集控编号	 单桩编号	 单桩枪头
void AddBCDStationNo(QString sNo,unsigned char *psData)
{
    //unsigned char TempChar[15];

    QString sStationNo = LeftZeroStr(sNo,STATION_NO_LEN);
    //memcpy(TempChar,  (unsigned char *)sStationNo.toAscii().data(), 15);
    //CharToBCD((unsigned char *)sStationNo.toAscii().data(),STATION_NO_LEN-6,psData);//前10个字符
    CharToBCD((unsigned char *)sStationNo.toAscii().data(),STATION_NO_LEN,psData);//前10个字符
    //psData[5] = TempChar[10];//第11个字符,即站编码字节6
    //CharToBCD(&TempChar[11],4,psData);//后4个字符
}
void ShowBinMsg(const char *psTitle,unsigned char *psData,int iLen)
{
    Dprintf("%s DataLen = %d\n",psTitle,iLen);
    for (int k =0 ;k<iLen;k++) Dprintf(" %02x",psData[k]);
        Dprintf("\n");
}

// 函数功能:将16进制数组转换成qstring格式 YCZ
QString ConvertHex2Qstr(unsigned char *psData,int len){
    QString dst_str;
    for(int i = 0; i<len; i++){
        unsigned char outchar =* (psData + i);
        QString str = QString("%1 ").arg(outchar&0xFF, 2, 16, QLatin1Char('0'));
        dst_str += str;
    }
    return dst_str;
}
// 函数功能:将10进制数组转换成qstring格式
QString ConvertDec2Qstr(unsigned char *psData,int len){
    QString dst_str;
    for(int i = 0; i<len; i++){
        unsigned char outchar =* (psData + i);
        QString str = QString("%1 ").arg(outchar&0xFF, 2, 10, QLatin1Char('0'));
        dst_str += str;
    }
    return dst_str;
}
// 函数功能:将10进制,unsigned char串转换成qstring格式 XX
QString CoventUnsignedCharToQString(unsigned char * p,unsigned char p_length)
{
    QString ret;
    for(int i = 0;i < p_length;i++)
    {
        ret +=QString::number(p[i]);
    }
    return ret;
}

///
/// \brief ConvertQstr2Hex 将QString变量转换成16进制数组 (字符串格式：68 14 28 00 08 带空格） add by YCZ 2016-03-18
/// \param src_str 待转QString
/// \param dst_data 目标数组
/// \param max_data 目标数组最大支持数据长度
/// \return
///
bool ConvertQstr2Hex(QString src_str, unsigned char *dst_data, int max_data)
{
    src_str.replace(QString(" "),QString(""));//先去掉空格
    if(src_str.length()/2 > max_data){
        return false;
    }
    char * ps_data = src_str.toAscii().data();
    CharToBCD((unsigned char *)ps_data, src_str.length(), dst_data);
    return true;
}

///
/// \brief IntDivision 根据终端数量和一包最多包含的终端数量，计算所需包数
/// \param iDivisor 终端数量
/// \param iDividend 一包最多包含的终端数量
/// \return 所需包数
///
int IntDivision(int iDivisor,int iDividend)
{
    if(iDivisor<=0||iDividend<=0)
        return 0;
    int iModNum = iDivisor%iDividend;
    return (iDivisor - iModNum)/iDividend + (iModNum>0?1:0);
}

bool CreateDir(QString &sPath)
{
    QDir dir(sPath);
    if(dir.exists())
    {
        return true;
    }

    QString sCmd = "mkdir "+sPath;
    if(system(sCmd.toAscii().data())==0)
        return true;
    else
        return false;
}

bool isIP(char *psName)
{
    int   nAddr;
    bool   bSuccess = false;

    try
    {
        nAddr   =   inet_addr(psName);
        bSuccess   =   true;
    }
    catch(...)
    {

    }
    return   bSuccess&&nAddr!=-1;
}

bool getHostByName(char *psName,char *psHostIp)
{
    struct hostent *host=NULL;

    if(!isIP(psName))
    {
        host = gethostbyname(psName);
        if(host == NULL)
        {
            memcpy(psHostIp,psName,strlen(psName));
            return false;
        }
        else
        {
            switch( host->h_addrtype )
            {
                case AF_INET:
                    sprintf(psHostIp,"%s",inet_ntoa(*(struct in_addr*)host->h_addr_list[0]));
                    break;
                case AF_INET6:
                    return false;
                    break;
                default:
                    return false;
                    break;
            }
        }
    }
    else
    {
        memcpy(psHostIp,psName,strlen(psName));
    }
    return true;
}

QString getHostByName(QString sHostName)
{
    QString sRet;
    char psTemp[256];
    memset(psTemp,0x0,sizeof(psTemp));
    bool bRet = getHostByName(sHostName.toAscii().data(),psTemp);
    if(!bRet)
        sRet = sHostName;
    else
        sRet = QString(psTemp);
    return sRet;
}

//判断格式为yyyy-MM-dd HH:mm:ss字符串是否是合法的时间格式
bool isValidDate(char *psDate)
{
    if(strlen(psDate)!=19)
        return false;
    char psTemp[8];

    //年
    memset(psTemp,0x0,sizeof(psTemp));
    int iOffset = 0;
    memcpy(psTemp,&psDate[iOffset],4);
    int iTemp = atoi(psTemp);
    if(iTemp<2015)
        return false;
    iOffset +=4;
    if(psDate[iOffset]!='-')
        return false;
    iOffset ++;

    //月
    memset(psTemp,0x0,sizeof(psTemp));
    memcpy(psTemp,&psDate[iOffset],2);
    iTemp = atoi(psTemp);
    if(iTemp<1||iTemp>12)
        return false;
    iOffset +=2;
    if(psDate[iOffset]!='-')
        return false;
    iOffset ++;
    //日
    memset(psTemp,0x0,sizeof(psTemp));
    memcpy(psTemp,&psDate[iOffset],2);
    iTemp = atoi(psTemp);
    if(iTemp<1||iTemp>31)
        return false;
    iOffset +=2;
    if(psDate[iOffset]!='-')
        return false;
    iOffset ++;

    if(psDate[iOffset]!=' ')
        return false;
    iOffset ++;

    //时
    memset(psTemp,0x0,sizeof(psTemp));
    memcpy(psTemp,&psDate[iOffset],2);
    iTemp = atoi(psTemp);
    if(iTemp<1||iTemp>23)
        return false;
    iOffset +=2;
    if(psDate[iOffset]!=':')
        return false;
    iOffset ++;
    //时
    memset(psTemp,0x0,sizeof(psTemp));
    memcpy(psTemp,&psDate[iOffset],2);
    iTemp = atoi(psTemp);
    if(iTemp<0||iTemp>59)
        return false;
    iOffset +=2;
    if(psDate[iOffset]!=':')
        return false;
    iOffset ++;
    //时
    memset(psTemp,0x0,sizeof(psTemp));
    memcpy(psTemp,&psDate[iOffset],2);
    iTemp = atoi(psTemp);
    if(iTemp<0||iTemp>59)
        return false;

    return true;
}

int  GetStorageFree()
{    
    QString systotal,sysfree,prgtotal,prgfree="0";
    system("df -h > /tmp/dfret");
    QFile file("/tmp/dfret");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream rstr(&file);
        while(!rstr.atEnd())
        {
            QString line = rstr.readLine();
            if (line.startsWith("ubi0:rootfs"))
            {
                QStringList tmp = line.split(" ",QString::SkipEmptyParts);
                if (tmp.count() > 3)
                {
                    systotal = tmp.at(1);
                    sysfree = tmp.at(3);
                }
            }
            else if (line.startsWith("ubi1_0"))
            {
                QStringList tmp = line.split(" ",QString::SkipEmptyParts);
                if (tmp.count() > 3)
                {
                    prgtotal = tmp.at(1);
                    prgfree = tmp.at(3);
                }
            }
        }

        file.close();
    }
    return prgfree.remove("M").toInt();
}
bool SystemFuncResult(int ret_in)
{
    int ret1 = 0;
    int ret2 = 0;
    ret1 = WEXITSTATUS(ret_in);
    ret2 = WIFSIGNALED(ret_in);
    if((ret1 != 0)||(ret2 != 0))
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

int Verify_CRC16(unsigned char* pchMsg, int wDataLen)
{
    char chCRCHi = 0xFF; // 高CRC字节初始化
    char chCRCLo = 0xFF; // 低CRC字节初始化
    short wIndex;            // CRC循环中的索引
    while (wDataLen--)
    {
        // 计算CRC
        wIndex = chCRCLo ^ *pchMsg++ ;
        chCRCLo = chCRCHi ^ chCRCHTalbe[wIndex];
        chCRCHi = chCRCLTalbe[wIndex] ;
    }
    return ((chCRCHi << 8) | chCRCLo) ;
}


//----------------------------------------------时间转换----------------------------------------------//
///
/// \brief QDateTime2CharArray  QDateTime转["2016-11-18 10:40:41"]
/// \param dtQDateTime
/// \param dst 待转入数组指针,注意一定是20个字节长度
///
void QDateTime2CharArray(QDateTime &dtQDateTime, char * dst)
{
    memcpy(dst, dtQDateTime.toString("yyyy-MM-dd hh:mm:ss").toAscii().data(), 20);
}

///
/// \brief CharArray2QDateTime  ["2016-11-18 10:40:41"]转QDateTime
/// \param src 待转出数组指针,注意一定是20个字节长度
/// \param dtQDateTime
///
void CharArray2QDateTime(char * src, QDateTime &dtQDateTime)
{
    dtQDateTime = QDateTime::fromString(QString(src),"yyyy-MM-dd HH:mm:ss");
}

//检查终端类型,
//输入: ucCanID , 终端CAN地址
//返回 : 终端类型, 详见终端类型枚举
unsigned char CheckTermType(unsigned char ucCanID)
{
    //返回交流单相终端
    if((ucCanID >= ID_MinACSinCanID) && (ucCanID <= ID_MaxACSinCanID))
    {
        return TermType_ACSin;
    }
    //返回分支箱
//    if((ucCanID >= ID_MinBranchCanID) && (ucCanID <= ID_MaxBranchCanID))
//    {
//        return TermType_Branch;
//    }
    //保留
//    if((ucCanID >= ID_MinReserve1) && (ucCanID <= ID_MaxReserve1))
//    {
//        return TermType_Reserve1;
//    }
    //交流负载总配电柜
    if((ucCanID >= ID_MinACLoadDistributionCabinet) && (ucCanID <= ID_MaxACLoadDistributionCabinet))
    {
        return TermType_ACLoadDisBox;
    }
    //系统配电柜
    if((ucCanID >= ID_MinSystemDistributionCabinet) && (ucCanID <= ID_MaxSystemDistributionCabinet))
    {
        return TermType_SysDisBox;
    }
    //直流光伏控制柜
    if((ucCanID >= ID_MinDCPhotovoltaicControlCabinet) && (ucCanID <= ID_MaxDCPhotovoltaicControlCabinet))
    {
        return TermType_DCPVCtrlBox;
    }
    //四象限变换柜
    if((ucCanID >= ID_MinFourQuadrantConverterCabinet) && (ucCanID <= ID_MaxFourQuadrantConverterCabinet))
    {
        return TermType_4QuaConvertBox;
    }
    //独立逆变柜
    if((ucCanID >= ID_MinIndependentInverterCabinet) && (ucCanID <= ID_MaxIndependentInverterCabinet))
    {
        return TermType_IndeInventerBox;
    }
    //直流充放电柜
    if((ucCanID >= ID_MinDCChargeDischargeCabinet) && (ucCanID <= ID_MaxDCChargeDischargeCabinet))
    {
        return TermType_DCDischargeBox;
    }
    //直流充放电蓄能柜
    if((ucCanID >= ID_MinDCEnergyStorageCabinet) && (ucCanID <= ID_MaxDCEnergyStorageCabinet))
    {
        return TermType_DCDischargeSEBox;
    }
    //返回交流三相终端
    if((ucCanID >= ID_MinACThrCanID) && (ucCanID <= ID_MaxACThrCanID))
    {
        return TermType_ACThr;
    }
    //返回直流终端 = PDU
    if((ucCanID >= ID_MinDCCanID) && (ucCanID <= ID_MaxDCCanID))
    {
        return TermType_DC;
    }
    //返回CCU
    if((ucCanID > ID_MinCCUCanID) && (ucCanID < ID_MaxCCUCanID))
    {
        return TermType_CCU;
    }
    //集中控制器
    if((ucCanID > ID_MinControlCenterCanID) && (ucCanID < ID_MaxControlCenterCanID))
    {
        return TermType_CSCU;
    }
    return TermType_Undef;
}

//开始,结束充电返回代码
QByteArray GetCardResult(unsigned char ucRetCode, unsigned char ucType)
{
    QByteArray retArray;
    switch(ucRetCode)
    {
    case 1:
        retArray = "用户余额不足，请先进行充值";
        break;
    case 2:
        retArray = "用户卡未绑定";
        break;
    case 3:
        retArray = "您的用户已冻结，无法开启充电";
        break;
    case 4:
        retArray = "未找到用户卡";
        break;
    case 5:
        retArray = "未找到用户";
        break;
    case 6:
        retArray = "未找到终端";
        break;
    case 7:
        retArray = "通信故障";
        break;
    case 8:
        retArray = "已经充电，不能重复充电";
        break;
    case 9:
        retArray = "该电站尚未运营";
        break;
    case 10:
        retArray = "终端正在服务中,请选择其他终端";
        break;
    case 11:
        retArray = "充电枪未与车连接";
        break;
    case 12:
        retArray = "通信超时";
        break;
    case 13:
        retArray = "未找到用户账户";
        break;
    case 14:
        retArray = "未找到计费策略";
        break;
    case 15:
        retArray = "未找到充电业务信息";
        break;
    case 16:
        retArray = "已经结束充电，请勿重复操作";
        break;
    case 17:
        retArray = "开始充电异常";
        break;
    case 18:
        retArray = "时间段不能超过24小时，请重设";
        break;
    case 19:
        retArray = "终端离网，请选择其他终端";
        break;
    case 20:
        retArray = "您最多只能同时进行三个充电业务";
        break;
    case 21:
        retArray = "操作过于频繁，请稍后再试";
        break;
    case 22:
        retArray = "电站未运营不允许充电";
        break;
    case 23:
        retArray = "轮充设备不支持刷卡充电";
        break;
    case 24:
        retArray = "终端故障不允许充电";
        break;
    case 25:
        retArray = "此终端为调度充电，不允许app充电";
        break;
    case 26:
        retArray = "运营电站，不允许测试充电";
        break;
    case 27:
        retArray = "该电站不支持直接充";
        break;
    case 28:
        retArray = "终端没有初始电量";
        break;
    case 29:
        retArray = "获取正在消费的代金券时发生异常";
        break;
    case 30:
        retArray = "获取代金券余额时发生异常";
        break;
    case 31:
        retArray = "此代金卷不抵扣电费或者服务费";
        break;
    case 32:
        retArray = "此代金卷费用限制不正确";
        break;
    case 33:
        retArray = "获取正在消费的信用账户异常";
        break;
    case 34:
        retArray = "获取信用账户余额帐异常";
        break;
    case 35:
        retArray = "获取正在消费的现金账户异常";
        break;
    case 36:
        retArray = "获取现金余额账异常";
        break;
    case 37:
        retArray = "创建订单异常";
        break;
    case 38:
        retArray = "结束充电失败";
        break;
    case 39:
        retArray = "初始电量异常";
        break;
    case 40:
        retArray = "APP只支持收费电站的轮充充电";
    break;
    case 41:
        retArray = "该电站收费不允许直接充电";
        break;
    case 42:
        retArray = "该终端初始电量大于最大计量值";
        break;
    case 43:
        retArray = "终端未响应";
        break;
    case 44:
        retArray = "有未支付的充电订单,不允许开启充电";
        break;
    case 45:
        retArray = "北京公车改革刷卡充电中国移动身份确认失败";
        break;
    case 46:
        retArray = "获取电站电价高峰时段信息异常";
        break;
    case 47:
        retArray = "强制暂停充电失败";
        break;
    case 48:
        retArray = "强制开启充电失败";
        break;
    case 50:
        retArray = "终端不存在地锁";
        break;
    case 51:
        retArray = "下发降锁命令失败";
        break;
    case 52:
        retArray = "下发升锁命令失败";
        break;
    case 53:
        retArray = "强制创建订单异常";
        break;
    case 54:
        retArray = "充电机故障,请选择其他终端";
        break;
    case 55:
        retArray = "BMS故障,请选择其他终端";
        break;
    case 56:
        retArray = "连接故障,请选择其他终端";
        break;
    case 57:
        retArray = "APP定时充电不支持轮充设备充电";
        break;
    case 58:
        retArray = "易通卡刷卡申请创建电子钱包记录异常";
        break;
    case 59:
        retArray = "电子钱包支付未完成";
        break;
    case 60:
        retArray = "电子钱包补扣未支付失败";
        break;
    case 61:
        retArray = "未获取到车架号信息";
        break;
    case 62:
        retArray = "车牌号不匹配";
        break;
    case 63:
        retArray = "未获取到电子钱包记录";
        break;
    case 64:
        retArray = "未获取到运营商";
        break;
    case 65:
        retArray = "冻结优惠券失败";
        break;
    case 66:
        retArray = "检查终端是否含有正在充电订单异常";
        break;
    case 67:
        retArray = "更新充电明细异常";
        break;
    case 68:
        retArray = "结束充电通知执行充电业务结算异常";
        break;
    case 69:
        retArray = "结束充电通知执行逻辑异常";
        break;
    case 70:
        retArray = "您最多只能同时进行一个充电业务";
        break;
    case 71:
        retArray = "根据车牌号未获取到车辆信息";
        break;
    case 72:
        retArray = "检查更新未完全支付状态异常";
        break;
    case 73:
        retArray = "账号未绑定，请重新登录";
        break;
    case 74:
        retArray = "客户余额不足，请充值后再操作";
        break;
    case 75:
        retArray = "获取平台账户失败";
        break;
    case 76:
        retArray = "不能停止他人启动的订单";
        break;
    case 77:
        retArray = "检查客户未支付订单异常";
        break;
    case 78:
        retArray = "企业信用账户有未还款的账单";
        break;
    case 79:
        retArray = "检查企业信用账户还款情况异常";
        break;
    case 80:
        retArray = "该电站不支持限制充电";
        break;
    case 81:
        retArray = "限制金额不能小于零元";
        break;
    case 82:
        retArray = "限制电量不能小于零度";
        break;
    case 83:
        retArray = "此终端已被预约，请选择其他终端进行充电";
        break;
    case 84:
        retArray = "获取不到折扣策略";
        break;
    case 87:
        retArray = "创建客户信息失败";
        break;
    case 88:
        retArray = "创建客户信息异常";
        break;
    case 89:
        retArray = "创建客户卡信息失败";
        break;
    case 90:
        retArray = "创建客户卡信息异常";
        break;
    case 91:
        retArray = "获取用户卡信息异常";
        break;
    case 92:
        retArray = "枪未插好";
        break;
    case 93:
        retArray = "电池已充满";
        break;
    case 94:
        retArray = "车辆电池温度过高";
        break;
    case 95:
        retArray = "车辆电池温度过低";
        break;
    case 96:
        retArray = "车辆电池温度变化异常";
        break;
    case 97:
        retArray = "车辆BMS接触器故障";
        break;
    case 98:
        retArray = "车辆电池端口电压异常";
        break;
    case 99:
        retArray = "车辆控制器故障";
        break;
    case 100:
        retArray = "车辆电池过压";
        break;
    case 101:
        retArray = "充电机无可用模块";
        break;
    case 102:
        retArray = "车辆故障";
        break;
    case 103:
        retArray = "车辆BMS故障";
        break;
    case 104:
        retArray = "副枪不允许开启充电";
        break;
    case 105:
        retArray = "副枪不允许结束充电";
        break;
    case 106:
        retArray = "终端状态未知";
        break;
    case 107:
        retArray = "计费策略不完整";
        break;
    case 108:
        retArray = "应急模式时不能从云端开启充电";
        break;
    case 109:
        retArray = "该电站不支持经济充电";
        break;
    case 110:
        retArray = "该用户未交押金";
        break;
    case 111:
        retArray = "检查用户是否交押金异常";
        break;
    case 112:
        retArray = "经济充电不支持轮充";
        break;
    case 113:
        retArray = "您还未预约，无法开启充电";
        break;
    case 114:
        retArray = "客户不满足场站对于充电客户的限制";
        break;
    case 116:
        retArray = "该终端非营业时间不允许充电";
        break;
    case 117:
        retArray = "充电机切换中不允许充电";
        break;
    case 120:
        retArray = "该终端非营业时间不允许充电";
        break;
    case 121:
        retArray = "充电机切换中不允许充电";
        break;
    case 122:
        retArray = "车辆已充满";
        break;
    case 123:
        retArray = "此车位已被使用，请选择其他终端进行充电";
        break;
    case 124:
        retArray = "充电账户和预约账户不一致";
        break;
    case 125:
        retArray = "充电账户和使用地锁的账户不一致";
        break;
    case 126:
        retArray = "您的车已经被限制了充电范围,请到可以允许的电站充电";
        break;
    case 127:
        retArray = "终端不支持您的充电启动方式";
        break;
    case 128:
        retArray = "终端不支持定时充电";
        break;
    case 129:
        retArray = "支付账户限制的VIN不存在";
        break;
    case 130:
        retArray = "支付账户与VIN绑定的支付账户不一致";
        break;
    case 131:
        retArray = "第三方终端只允许通过APP扫码或调度充电";
        break;
    case 132:
        retArray = "第三方终端设备校验异常";
        break;
    case 133:
        retArray = "地锁处于升起状态不允许充电";
        break;
    case 134:
        retArray = "电站不允许使用芝麻信用";
        break;
    case 135:
        retArray = "获取终端信息异常";
        break;
    case 244:
        retArray = "二维码超时失效";
        break;
   case 245://248:
      retArray = "未获得分组确认或未获得自动充电属性";
        break;
    case 246://249:
        retArray = "未获得分组确认";
        break;
    case 247://250:
        retArray = "未收到VIN号，请稍后再试";
     break;
    case 248:
        retArray = "应急充电订单超过限制";
        break;
    case 249:
        retArray = "应急充电时间超过限制";
        break;
    case 250:
        retArray = "卡号校验失败";
        break;
    case 251:
        retArray = "没有找到订单";
        break;
    case 252:
        retArray = "设备启动中,请稍后";
        break;
	case 253:
        retArray = "本地鉴权失败";
		break;
	case 254:
        retArray = "启动充电失败";
		break;
    case 255:
        if(ucType == 1)
        {
            retArray = "申请开始充电成功";
        }
        else if(ucType == 2)
        {
            retArray = "申请结束充电成功";
        }
        break;
    default:
        retArray = "失败，未知错误";
		break;
    }

    return retArray;
}

QByteArray GetCardResultEnglish(unsigned char ucRetCode, unsigned char ucType)
{
    QByteArray retArray;
    switch(ucRetCode)
    {
    case 1:
        retArray = "Poor user balance";
        break;
    case 2:
        retArray = "Customer card is not bound";
        break;
    case 3:
        retArray = "Customer card is frozen";
        break;
    case 4:
        retArray = "No customer card found";
        break;
    case 5:
        retArray = "No customer found";
        break;
    case 6:
        retArray = "Terminal not found";
        break;
    case 7:
        retArray = "Communication failure";
        break;
    case 8:
        retArray = "The charging service has been started";
        break;
    case 9:
        retArray = "The station is not yet operational";
        break;
    case 10:
        retArray = "The terminal is in service";
        break;
    case 11:
        retArray = "No connect the gun";
        break;
    case 12:
        retArray = "Communication timeout";
        break;
    case 13:
        retArray = "No customer account found";
        break;
    case 14:
        retArray = "No billing strategy found";
        break;
    case 15:
        retArray = "No customer's charging business info";
        break;
    case 16:
        retArray = "Has finished charging";
        break;
    case 17:
        retArray = "An exception when charging began";
        break;
    case 18:
        retArray = "Timing interval can not exceed 24 h";
        break;
    case 19:
        retArray = "Terminal off the net";
        break;
    case 20:
        retArray = "You can only open three charging services";
        break;
    case 21:
        retArray = "Operation is too frequent";
        break;
    case 22:
        retArray = "Power is not allowed to operate";
        break;
    case 23:
        retArray = "Charging equipment does not support credit card charging";
        break;
    case 24:
        retArray = "Terminal failure";
        break;
    case 25:
        retArray = "Free charge device does not support app start";
        break;
    case 26:
        retArray = "The stations are not allowed to test charge";
        break;
    case 27:
        retArray = "Terminal does not support direct charging";
        break;
    case 28:
        retArray = "The terminal has no initial Electricity";
        break;
    case 29:
        retArray = "Obtain an exception when the voucher is being consumed";
        break;
    case 30:
        retArray = "An exception occurred when obtained the balance";
        break;
    case 31:
        retArray = "This voucher can not be deducted for electricity";
        break;
    case 32:
        retArray = "The fee limit for this voucher is incorrect";
        break;
    case 33:
        retArray = "An exception occurred when consumed credit account";
        break;
    case 34:
        retArray = "An exception occurred";
        break;
    case 35:
        retArray = "An exception occurred";
        break;
    case 36:
        retArray = "An exception occurred";
        break;
    case 37:
        retArray = "An exception occurred while creating the order";
        break;
    case 38:
        retArray = "End charge failed";
        break;
    case 39:
        retArray = "An exception occurred";
        break;
    case 41:
        retArray = "The station charges are not allowed to charge directly";
        break;
    case 42:
        retArray = "The initial charge is greater than the maximum";
        break;
    case 43:
        retArray = "The terminal is not responding";
        break;
    case 44:
        retArray = "You have an unpaid charge order";
        break;
    case 46:
        retArray = "An exception occurred";
        break;
    case 47:
        retArray = "Forced pause charging failed";
        break;
    case 48:
        retArray = "Forced to start charging failed";
        break;
    case 50:
        retArray = "Terminal is not associated with the lock";
        break;
    case 51:
        retArray = "The drop lock command failed";
        break;
    case 52:
        retArray = "The lock command failed";
        break;
    case 53:
        retArray = "An exception occurred when the order was created";
        break;
    case 54:
        retArray = "Charger failure";
        break;
    case 55:
        retArray = "BMS failure";
        break;
    case 56:
        retArray = "Connection failure";
        break;
    case 57:
        retArray = "Timed charging failure";
        break;
    case 58:
        retArray = "An exception occurred while applying for an e-wallet";
        break;
    case 59:
        retArray = "E-wallet has unpaid orders";
        break;
    case 60:
        retArray = "E-wallet charges fail";
        break;
    case 61:
        retArray = "No frame number information";
        break;
    case 62:
        retArray = "License plate number does not match";
        break;
    case 63:
        retArray = "No wallet records received";
        break;
    case 64:
        retArray = "No operator information is available";
        break;
    case 65:
        retArray = "Frozen coupon failed";
        break;
    case 66:
        retArray = "An exception occurred";
        break;
    case 67:
        retArray = "An exception occurred";
        break;
    case 68:
        retArray = "An exception occurred when the order was settled";
        break;
    case 69:
        retArray = "An exception occurred while the charge was sent";
        break;
    case 70:
        retArray = "You can only carry out a charging business";
        break;
    case 71:
        retArray = "The vehicle information is not obtained";
        break;
    case 72:
        retArray = "An exception occurred";
        break;
    case 73:
        retArray = "Account is not bound";
        break;
    case 74:
        retArray = "Customer balance is insufficient";
        break;
    case 75:
        retArray = "Failed to get platform account";
        break;
    case 76:
        retArray = "Can not stop others from starting the order";
        break;
    case 78:
        retArray = "Corporate credit accounts have unpaid bills";
        break;
    case 79:
        retArray = "Check the corporate credit account repayment exception";
        break;
    case 251:
        retArray = "Did not find the order";
        break;
    case 252:
        retArray = "Please start later";
        break;
    case 253:
        retArray = "Local authentication failed";
        break;
    case 254:
        retArray = "Start charging failed";
        break;
    case 255:
        if(ucType == 1)
        {
            retArray = "Start charging successfully";
        }
        else if(ucType == 2)
        {
            retArray = "Application is successful";
        }
        break;
    default:
        retArray = "Failure, unknown error";
        break;
    }

    return retArray;
}

bool QueryCarLisenceName(char * strCarLisence, QString &StringName)
{
    char Sheng[][7] = {
        {"未知"}, {"京"},{"沪"},{"津"},
        {"渝"},{"冀"},{"晋"},{"蒙"},
        {"辽"},{"吉"},{"黑"},{"苏"},
        {"浙"},{"皖"},{"闽"},{"赣"},
        {"鲁"},{"豫"},{"鄂"},{"湘"},
        {"粤"},{"桂"},{"琼"},{"川"},
        {"贵"},{"云"},{"藏"},{"陕"},
        {"甘"},{"青"},{"宁"},{"新"}
    };
    QString qstrTemp;
    char ctemp[2] = {0};
    unsigned char ucTempCarLisence;
    //取省代号
    StringName = QString::fromUtf8(Sheng[(unsigned char)strCarLisence[0]]);
    //循环取其他几位
    for(int i = 1; i < 7; i++){
        ucTempCarLisence = *(strCarLisence+i);
        if(ucTempCarLisence <= 36){
            if( ucTempCarLisence <= 9){
                ctemp[0] = '0' + ucTempCarLisence;
                qstrTemp = QString(QLatin1String((char *)ctemp));
                StringName.append(qstrTemp);
            }
            else if(ucTempCarLisence >= 11 && ucTempCarLisence <= 36){
                ctemp[0] = 'A' + (ucTempCarLisence - 11);
                qstrTemp = QString(QLatin1String((char *)ctemp));
                StringName.append(qstrTemp);
            }
            else{
                return false;
            }
        }
        else{
            return false;
        }
    }
    return true;
}
