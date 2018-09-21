#ifndef			__UPDATE_H__
#define			__UPDATE_H__

#include <QThread>
#include <QObject>
#include <QTimer>
#include <QFile>
#include <QLibrary>
#include <QStringList>

#include "ParamSet/ParamSet.h"
#include "Infotag/CSCUBus.h"
#include "GeneralData/GeneralData.h"
#include "Bus/Bus.h"
#include "GeneralData/ModuleIO.h"
#include "Log/Log.h"
#include "DevCache/DevCache.h"

#include "Update/HttpInterface.h"
#include "Update/JsonInterface.h"

extern "C"
{
#include "et299/et299api.h"
#include "et299/ET299_data.h"
}

#define ELEC_LOCK_LIB_NAME "libet299.so"

typedef int (*pFunElecLock)(ET_Lock *ET_Data);

class Q_DECL_IMPORT Update : public CModuleIO
{
    Q_OBJECT
public:
    Update();
    ~Update();

    int InitModule(QThread* pThread);
    int RegistModule();
    int StartModule();
    int StopModule();
    int ModuleStatus();

private:
    Log *log;
	DevCache *devcache;
    ParamSet *paramSet;
	char hostPort[30];
    unsigned char cmdSource, cmdTypeMaster, cmdTypeSlave, canAddr;
    stCSCUSysConfig cscuSysConfig;       //CSCU参数设置缓存
	stServer0Config server0Config;
	QMap<unsigned char, QString>	canUpdateMap;	//记录有多少个can设备需要升级	
    QString CanaddrStr;

	int updateTime;				//升级时间 1 立刻 2 空闲

    QTimer mountCheckTimer;
	int autoTimerCount;						

	QString resolveVersion(void);
	bool queryCanUpdateFile(unsigned char &canAddr, QString & fileName);
	void udiskInsertCheck(void);
	void autoUpdateRequest(void);
	int termChargingCheck(void);
	int checkDownloadResult(void);
	void dealDownloadResult(void);
	void queryRequestInfo(stUpdateRequestData *requestData);

signals:
    void sigToBus(InfoMap TelecontrolMap, InfoAddrType enAddrType);	//发送升级的结果
    void sigRunUpdate(QString);										//发送执行升级命令

private slots:
    void timeOut(void);
public slots:
    void slotFromBus(InfoMap Map, InfoAddrType enAddrType);			//接收到升级命令
    void procRunUpdate(QString);									//运行
    void procStartWork(void);										//线程运行，外部关联
};

#endif
