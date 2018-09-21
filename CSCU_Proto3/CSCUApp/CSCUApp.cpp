#include "CSCUApp.h"
#include "CSCUBus.h"
#include "GeneralData.h"
#include <QTextCodec>
#include <QSettings>
#include <QStringList>
#include <QLibrary>

#include "Bus.h"
#include "ModuleIO.h"
#include "Log.h"
#include "ParamSet.h"
#include "DBOperate.h"
#include "DevCache/DevCache.h"
#include "RealDataFilter.h"

bool CSCUApp::_terminate = false;
bool CSCUApp::_initFinish = false;

typedef CModuleIO* (*pCreateDevInstance)();

CSCUApp::CSCUApp(int &argc, char **argv) : QCoreApplication(argc, argv)
{
	//注册自定义类型
    qRegisterMetaType<InfoMap>("InfoMap");
    qRegisterMetaType<InfoAddrType>("InfoAddrType");

    //全局字符编码 UTF-8
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

	setVerion();
}

CSCUApp::~CSCUApp()
{

}

bool CSCUApp::onExit(int sig)
{
	if(_initFinish){
		_terminate = true;	
	}

	return true;
}

void CSCUApp::registeApp()
{
	QList<int> list;
	CBus::GetInstance()->RegistApp(this, list);
}

bool CSCUApp::setVerion(QString strVersion)
{
	QSettings setting(MAIN_CONFIG, QSettings::IniFormat);
	setting.setValue("CSCUSys/Version", MAIN_VERSION);
	return true;
}

void CSCUApp::slotFromBus(InfoMap mapInfo, InfoAddrType type)
{

}

int CSCUApp::exec()
{
	int ret, cnt;

	if(!loadModule())
		return -1;

	registeApp();

	_initFinish = true;

	qDebug()<<"CSCU run";

	while(!_terminate){
		QTimer::singleShot(1000, this, SLOT(quit()));

		QCoreApplication::exec();

		ret = ParamSet::GetInstance()->needReboot();
		switch(ret){
			case 1:
				freeModule();
				return 1;
			case 2:
				freeModule(true);
				return 1;
			default:
				break;
		}

		if(cnt++ / 60){
			for(int i = 0; i < listModule.count(); i++){
				listModule.at(i)->ModuleStatus();
			}
			cnt = 0;
		}
	}

	freeModule();

	return 0;
}

bool CSCUApp::loadModule()
{
	QSettings setting(LIB_CONFIG, QSettings::IniFormat);		

	for(int i = 0; i < setting.childGroups().count(); i++){
		setting.beginGroup(setting.childGroups().at(i));

		if(setting.contains("enable") && setting.value("enable").toString().toLower() == "true"){
			QLibrary lib(QString(LIB_PATH) + setting.value("library").toString());

			if(!lib.load()){
				qDebug()<<lib.errorString();
				return false;
			}

			pCreateDevInstance func;
			func = (pCreateDevInstance)lib.resolve("CreateDevInstance");
			if(!func){
				qDebug()<<lib.errorString();
				return false;
			}

			CModuleIO* pModule = func();
			if(!pModule){
				qDebug()<<lib.errorString();
				return false;
			}

			QThread* pThread = new QThread();
			//模块初始化
			pModule->InitModule(pThread);
			//模块注册到总线
			pModule->RegistModule();

			//指针保存至全局列表，以备访问及释放
			listModule.append(pModule);
			listThread.append(pThread);
		}

		setting.endGroup();
	}

	//启动所有模块
	for(int i = 0; i < listModule.count(); i++){
		listModule.at(i)->StartModule();
	}
	sleep(1);
	return true;
}

void CSCUApp::freeModule(bool reboot)
{
	for(int i = listModule.count() - 1; i >= 0; i--){
		CModuleIO* pModule = listModule.at(i);
        if(pModule)
            pModule->StopModule();
	}

	for(int i = listThread.count() - 1; i >= 0; i--){
		QThread* pThread = listThread.at(i);
		if(pThread){
            pThread->quit();
            pThread->wait();
			delete pThread;
		}
	}
	listThread.clear();

	for(int i = listModule.count() - 1; i >= 0; i--){
		CModuleIO* pModule = listModule.at(i);
		if(pModule)
			delete pModule;
	}
	listModule.clear();

	if(reboot)
        system("reboot;sleep 8");

	qDebug()<<"CSCU exit";

	exit(0);
}
