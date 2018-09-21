#ifndef BUS_H
#define BUS_H

#include <QObject>
#include <QCoreApplication>
#include <QMultiMap>
#include "CModuleHelper.h"
#include "ModuleIO.h"

class CModuleIO;
typedef QMultiMap<int,CModuleHelper*> EventMap;

//模块信号 槽定义 sigToBus  slotFromBus
class Q_DECL_IMPORT CBus : public QObject
{
	Q_OBJECT
private:
    EventMap eventMap;

public:
	~CBus();

	static CBus *GetInstance();
	
    bool RegistApp(QCoreApplication *pApp, QList<int> list);
    bool RegistDev(CModuleIO *pModule, QList<int> list);

public slots:
    void slotBusIn(InfoMap mapInfo, InfoAddrType type);

protected:
	CBus();
};

#endif
