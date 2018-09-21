#ifndef CSCUAPP_H
#define CSCUAPP_H

#include <QCoreApplication>
#include <QList>
#include "ModuleIO.h"

class CSCUApp : public QCoreApplication
{
	Q_OBJECT
public:
	CSCUApp(int &argc, char **argv);
	~CSCUApp();

	static bool onExit(int sig);

	int exec();

signals:
    void sigToBus(InfoMap mapInfo, InfoAddrType type);
public slots:
    void slotFromBus(InfoMap mapInfo, InfoAddrType type);

private:
	bool setVerion(QString strVersion = "");
	bool loadModule();
	void freeModule(bool reboot = false);
	void registeApp();

	QList<CModuleIO*> listModule;
	QList<QThread*> listThread;

	static bool _initFinish;
	static bool _terminate;
};

#endif
