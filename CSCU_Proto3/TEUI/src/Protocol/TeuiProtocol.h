#ifndef		__TEUI_PROTOCOL_H__
#define		__TEUI_PROTOCOL_H__

#include <QObject>
#include <QList>
#include <QVariant>
#include <QByteArray>

#include "ProtocolBase.h"
#include "Bus.h"

/*数据单元标示、内容结构*/
typedef struct __InfoData
{
    unsigned char cmdMaster;
    unsigned short cmdSlave;
    QByteArray arrayData;
}stInfoData;
Q_DECLARE_METATYPE(stInfoData)

/*数据单元标示、内容结构*/
typedef struct __ParseData
{
    unsigned char cmdMaster;
    unsigned short cmdSlave;
    int len;
    unsigned char *dst;
}stParseData;


class TeuiProtocol : public QObject, public ProtocolBase
{
	Q_OBJECT
private:

    unsigned char stationAddr;
    unsigned char sendBuff[2048];

    CBus *bus;
    stParseData paraseData;

    unsigned char getSum(unsigned char *src, int len);
    int organizeWholePack(unsigned char *data, int max_len,
                  unsigned char master, unsigned short slave, unsigned char *info_data, int info_len);
    int rcvBaseCheck(unsigned char *src_data, int src_len, stParseData *data_area, unsigned char **);
    void dealParaseDatas(stParseData parseData);

public:
    TeuiProtocol(unsigned char, CBus *bus);
    ~TeuiProtocol();
    
	int sendProtocolData(InfoProtocol Info, InfoAddrType);

signals:
    int sendPackageDatas(unsigned char *data, int len);		//发送数据到socket

public slots:
    void receivePackageDatas(QVariant var);                   //接收socket数据
};

#endif
