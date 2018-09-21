#ifndef _ACTIVEDEFEND_H
#define _ACTIVEDEFEND_H

#include "DevCache.h"
#include "RealDataFilter.h"
#include "ParamSet.h"
#include "Log.h"
#include "DBOperate.h"
#include "ModuleIO.h"
#include "../defense/Defense.h"

class ActiveDefend: public CModuleIO
{
    Q_OBJECT
public:
    ActiveDefend();
	~ActiveDefend();

	//动态库接口
    int InitModule(QThread* pThread);
    int RegistModule();
    int StartModule();
    int StopModule();
    int ModuleStatus();

	static void cmdToBus(InfoMap map, InfoAddrType type);
	static void writeLog(QString strLog, int iLevel = 2);

	static DevCache *getCache();
	static RealDataFilter *getFilter();
	static ParamSet *getSetting();
	static Log *getLog();
	static DBOperate *getDb();

	static ActiveDefend *m_pDefend;

signals:
	//总线信号
    void sigToBus(InfoMap mapInfo, InfoAddrType type);
public slots:
	//总线信号槽
    void slotFromBus(InfoMap mapInfo, InfoAddrType type);

private slots:
	void slot_onThreadRun();

private:
	void timerEvent(QTimerEvent *event);
	QList<Defense *> createDF(uchar canAddr);
	void destroyDF(uchar canAddr);
	TriggeredInfo *createTG(uchar canAddr);
	void destroyTG(uchar canAddr);

	uchar m_cCanRange[3][2];//单相、三相、直流can地址范围
	int m_iTimer;

	QMap< uchar, TriggeredInfo * > m_mapTriggeredDF;
	QMap< uchar, QList<Defense *> > m_mapDF;

	static DevCache* m_pDevCache;		//数据缓存
	static RealDataFilter* m_pFilter;	//数据过滤
	static ParamSet* m_pSetting;		//配置
	static Log* m_pLog;					//日志
	static DBOperate* m_pDatabase;		//数据库
};

#endif
