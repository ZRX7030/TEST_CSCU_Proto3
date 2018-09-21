#include <QDebug>
#include <stdio.h>
#include <QList>
#include "Bus.h"

CBus::CBus()
{

}

CBus::~CBus()
{
	eventMap.clear();
}

CBus *CBus::GetInstance()
{
	static CBus *ins = NULL;
	if(!ins){
		ins = new CBus();
	}

	return ins;
}

bool CBus::RegistApp(QCoreApplication *pApp, QList<int> list)
{
	int listSize = list.size();
	int type=0;

	QObject::connect( pApp, SIGNAL(sigToBus(InfoMap, InfoAddrType)), this, SLOT(slotBusIn(InfoMap, InfoAddrType)) );

	if(listSize)
	{
		CModuleHelper* pHelper = new CModuleHelper();
		QObject::connect( pHelper, SIGNAL(sigToBus(InfoMap, InfoAddrType)), pApp, SLOT(slotFromBus(InfoMap, InfoAddrType)) );
		for( int i=0; i< listSize; i++)
		{
			type = list.at(i);
			eventMap.insert(type,pHelper);
		}
	}
}

bool CBus::RegistDev(CModuleIO *pModule, QList<int> list)
{
	int listSize = list.size();
	int type=0;

	QObject::connect( pModule, SIGNAL(sigToBus(InfoMap, InfoAddrType)), this, SLOT(slotBusIn(InfoMap, InfoAddrType)) );

	if(listSize)
	{
		CModuleHelper* pHelper = new CModuleHelper();
		QObject::connect( pHelper, SIGNAL(sigToBus(InfoMap, InfoAddrType)), pModule, SLOT(slotFromBus(InfoMap, InfoAddrType)) );
		for( int i=0; i< listSize; i++)
		{
			type = list.at(i);
			eventMap.insert(type,pHelper);
		}
	}

	return true;
}

/**
 *模块发送过来的信号，处理函数
 */
void CBus::slotBusIn(InfoMap mapInfo, InfoAddrType type)
{
	if(eventMap.contains(type))
	{
		QList<CModuleHelper *> rList;
		rList.clear();
		rList = eventMap.values(type);
		for(int i=0;i<rList.size();i++)
		{
			CModuleHelper * helper = rList.at(i);
			helper->sigHelper(mapInfo,type);
		}
	}
}
