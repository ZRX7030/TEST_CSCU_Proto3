#include <QDebug>

#include "Bus.h"

CBus::CBus()
{

}

CBus::~CBus()
{
	reserveTable.clear();
}

/**
 *list 里的值可以为空(只注册数据)
 */
bool CBus::registDev(SwapBase *swapBase, QList<InfoAddrType> list)
{
	mutexTable.lock();
    reserveTable.insert(swapBase, list);
	mutexTable.unlock();
    return true;
}
/**
 *取消注册
 */
void CBus::cancelRegistDev( SwapBase *swapBase)
{
	mutexTable.lock();
    reserveTable.remove(swapBase);
	mutexTable.unlock();
}
/**
 * 根据订阅的主题发送到目标SwapBase
 */
void CBus::sendTodest(InfoMap mapInfo, InfoAddrType type)
{
	mutexTable.lock();
	BusReserveTable::iterator it;
//	qDebug() << "BusReserveTable size is " <<reserveTable.size();
	for( it = reserveTable.begin(); it !=  reserveTable.end(); ++it) 
	{ 
		QList<InfoAddrType> resList = it.value();
	//	qDebug() << "sendTodst list size is " << resList.size();
		for( int i=0; i< resList.size(); i++)
		{
			if(resList.at(i) == type)
			{
			//	qDebug() <<"send to dist................";
                SwapBase *swapBase = (SwapBase *)it.key();
                swapBase->receiveFromBus(mapInfo, type);
				mutexTable.unlock();
				break;
			}
		}
	}
	mutexTable.unlock();
//	qDebug() <<"send to dist eixt ok................";
}
