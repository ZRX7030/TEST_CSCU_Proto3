#ifndef PROTOCOLBASE_H
#define PROTOCOLBASE_H

/**
  *用于将界面产生的的数据发送到继承方
  */

#include "Common.h"

class ProtocolBase
{
public:
    ProtocolBase(){
    }

public:
    virtual int sendProtocolData(InfoProtocol, InfoAddrType)=0;
};
#endif // PROTOCOLBASE_H
