#ifndef SWAPBASE_H
#define SWAPBASE_H

/**
 *用于接收bus发过来的数据
 */

#include "Common.h"

class SwapBase
{
public:
    SwapBase(){
    }

public:
    virtual void receiveFromBus(InfoMap, InfoAddrType)=0;
};

#endif
