#ifndef BUS_H
#define BUS_H

#include <QMap>
#include <QList>
#include <QMutex>

#include "SwapBase.h"

typedef  QMap<SwapBase*, QList<InfoAddrType> > BusReserveTable;			//模块订阅bus

class CBus
{
private:
	BusReserveTable reserveTable;
	QMutex mutexTable;
public:
	CBus();
	~CBus();
	
    bool registDev(SwapBase *swapBase, QList<InfoAddrType> list);
    void cancelRegistDev(SwapBase *swapBase);

    void sendTodest(InfoMap mapInfo, InfoAddrType type);         //发送到目标
};

#endif
