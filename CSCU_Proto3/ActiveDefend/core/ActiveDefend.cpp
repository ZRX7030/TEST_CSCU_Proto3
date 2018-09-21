#include "ActiveDefend.h"
#include "factory/DefenseFactory.h"
#include <QTimerEvent>

DevCache* ActiveDefend::m_pDevCache = NULL;
RealDataFilter* ActiveDefend::m_pFilter = NULL;
ParamSet* ActiveDefend::m_pSetting = NULL;
Log* ActiveDefend::m_pLog = NULL;
DBOperate* ActiveDefend::m_pDatabase = NULL;
ActiveDefend *ActiveDefend::m_pDefend = NULL;

ActiveDefend::ActiveDefend()
{
	m_pSetting = ParamSet::GetInstance();
	m_pDevCache = DevCache::GetInstance();
	m_pFilter = RealDataFilter::GetInstance();
    m_pLog = Log::GetInstance();
	m_pDatabase = DBOperate::GetInstance();

	stCSCUSysConfig confSys;

	m_pSetting->querySetting(&confSys, PARAM_CSCU_SYS);
	m_cCanRange[0][0] = ID_MinACSinCanID;
	m_cCanRange[0][1] = ID_MinACSinCanID + (uchar)confSys.singlePhase;
	m_cCanRange[1][0] = ID_MinACThrCanID;
	m_cCanRange[1][1] = ID_MinACThrCanID + (uchar)confSys.threePhase;
	m_cCanRange[2][0] = ID_MinDCCanID;
	m_cCanRange[2][1] = ID_MinDCCanID + (uchar)confSys.directCurrent;

	m_iTimer = -1;
}

ActiveDefend::~ActiveDefend()
{
	if(m_iTimer > 0){
		killTimer(m_iTimer);
		m_iTimer = -1;
	}

	for(QMap<uchar, TriggeredInfo*>::Iterator it = m_mapTriggeredDF.begin(); it != m_mapTriggeredDF.end(); ++it){
		destroyTG(it.key());
	}

	for(QMap< uchar, QList<Defense *> >::Iterator it = m_mapDF.begin(); it != m_mapDF.end(); ++it){
		destroyDF(it.key());
	}
}

/*
 * 线程启动槽函数
 *
 * 参数		无
 * 返回值	无
 */
void ActiveDefend::slot_onThreadRun()
{
	writeLog(QString("ActiveDefend Thread Run"));

    m_iTimer = startTimer(3 * 1000);
}

/*
 * 接收其它模块消息
 * mapInfo 	输入 信息体集合
 * type    	输入 消息类型
 * 返回值	无
 */
void ActiveDefend::slotFromBus(InfoMap mapInfo, InfoAddrType type)
{

}

/*
 * 记录模块执行情况日志
 * strLog	输入 需记录的内容
 * iLevel	输入 日志输出等级
 * 返回值	无
 */
void ActiveDefend::writeLog(QString strLog, int iLevel)
{
	QString strLogName = "activedefend";
	switch (iLevel) {
		case 1:
			m_pLog->getLogPoint(strLogName)->debug(strLog);
			break;
		case 2:
			m_pLog->getLogPoint(strLogName)->info(strLog);
			break;
		case 3:
			m_pLog->getLogPoint(strLogName)->warn(strLog);
			break;
		case 4:
			m_pLog->getLogPoint(strLogName)->error(strLog);
			break;
		default:
			break;
	}
}

/*
 * 动态库接口函数，初始化模块
 * pThread	输入 模块将要运行的线程
 * 返回值 	0表示无错误，-1表示有错误
 */
int ActiveDefend::InitModule(QThread* pThread)
{
	m_pWorkThread = pThread;
	return 0;
}

/*
 * 动态库接口函数，向BUS注册模块
 * pBus		输入 BUS模块指针
 * 返回值 	0表示无错误，-1表示有错误
 */
int ActiveDefend::RegistModule()
{
	QList<int> list;

	list.append(AddrType_LocalEmergency_Result);

	CBus::GetInstance()->RegistDev(this, list);

	return 0;
}

/*
 * 动态库接口函数，启动模块
 * 返回值 	0表示无错误，-1表示有错误
 */
int ActiveDefend::StartModule()
{
	this->moveToThread(m_pWorkThread);
	QObject::connect(m_pWorkThread, SIGNAL(started()), this, SLOT(slot_onThreadRun()));
	m_pWorkThread->start();

	return 0;
}

/*
 * 动态库接口函数，停止模块
 * 返回值 	0表示无错误，-1表示有错误
 */
int ActiveDefend::StopModule()
{
	return 0;
}

/*
 * 动态库接口函数，模块工作状态
 * 返回值 	0表示无错误，-1表示有错误
 */
int ActiveDefend::ModuleStatus()
{
	return 0;
}

/*
 * 动态库接口函数，创建模块实例
 * pDepends 输入 公共模块指针列表
 * 返回值 	成功返回模块实例，失败返回NULL
 */
CModuleIO* CreateDevInstance()
{
	ActiveDefend::m_pDefend = new ActiveDefend();
	
	return ActiveDefend::m_pDefend ? ActiveDefend::m_pDefend : NULL;
}

/*
 * 动态库接口函数，销毁模块实例
 * pModule 输入 模块实例
 * 返回值  无	
 */
void DestroyDevInstance(CModuleIO* pModule)
{
	if(pModule){
		delete pModule;
	}
}

void ActiveDefend::cmdToBus(InfoMap map, InfoAddrType type)
{
	emit m_pDefend->sigToBus(map, type);
}

DevCache *ActiveDefend::getCache()
{
	return m_pDevCache;
}

RealDataFilter *ActiveDefend::getFilter()
{
	return m_pFilter;
}

ParamSet *ActiveDefend::getSetting()
{
	return m_pSetting;
}

Log *ActiveDefend::getLog()
{
	return m_pLog;
}

DBOperate *ActiveDefend::getDb()
{
	return m_pDatabase;
}

void ActiveDefend::timerEvent(QTimerEvent *event)
{
	if(event->timerId() == m_iTimer){
		TriggeredInfo *tgInfo = NULL;
		DefendLevelInfo *dfLevel = NULL;
		Defense *df = NULL;
		QString strTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

		for(int i = 0; i < 3; i++){
			for(uchar canAddr = m_cCanRange[i][0]; canAddr < m_cCanRange[i][1]; canAddr++){
				TerminalStatus status;
				bool chargeStopped = false;//是否停止充电状态

				if(!m_pDevCache->QueryTerminalStatus(canAddr, status))
					continue;
            	if(status.validFlag != ALL_VALID_NUMBER)
					continue;

				if(m_mapTriggeredDF.contains(canAddr)){
					tgInfo = m_mapTriggeredDF[canAddr];
					df = tgInfo->df;
					
					//继电器断开、充电机非工作状态为停止充电状态
					chargeStopped = status.stFrameRemoteSingle.relay_status == 0 && status.stFrameRemoteSingle.charge_status != 1;

					if(tgInfo->level < LEVEL_1){
						dfLevel = tgInfo->dfLevel[tgInfo->level];
						dfLevel->result = chargeStopped ? 1 : 0;

						writeLog(QString().sprintf("[CAN=%3d][防护检测][%s]检测是否充电中[%s]", 
									canAddr, df->getDesc().toAscii().data(),
									dfLevel->result == 0 ? "是" : "否"));

						if(dfLevel->result <= 0){
							memcpy(dfLevel->defendTime, strTime.toAscii().data(), strTime.length());
							df->alarm(canAddr, 1);
							df->stopCharge(canAddr);
							tgInfo->level = LEVEL_1;

							writeLog(QString().sprintf("[CAN=%3d][主动防护][%s]防护级别[%d]触发", 
									canAddr, df->getDesc().toAscii().data(), tgInfo->level));
							continue;
						}
					}if(tgInfo->level == LEVEL_1){
						dfLevel = tgInfo->dfLevel[tgInfo->level - 1];
						dfLevel->result = chargeStopped ? 1 : 0;
						df->dfLog(tgInfo);
						writeLog(QString().sprintf("[CAN=%3d][主动防护][%s]防护级别[%d]防护结果[%s]", 
									canAddr, df->getDesc().toAscii().data(), tgInfo->level, 
									dfLevel->result == 1 ? "成功" : "失败"));

						if(dfLevel->result <= 0){
							dfLevel = tgInfo->dfLevel[tgInfo->level];
							memcpy(dfLevel->triggerTime, strTime.toAscii().data(), strTime.length());
							memcpy(dfLevel->defendTime, strTime.toAscii().data(), strTime.length());
							tgInfo->level = LEVEL_2;
							df->alarm(canAddr, 2);
							df->trip();

							writeLog(QString().sprintf("[CAN=%3d][主动防护][%s]防护级别[%d]触发", 
										canAddr, df->getDesc().toAscii().data(), tgInfo->level));
							continue;
						}
					}else if(tgInfo->level == LEVEL_2){
						dfLevel = tgInfo->dfLevel[tgInfo->level - 1];
						dfLevel->result = chargeStopped ? 1 : 0;
						df->dfLog(tgInfo);

						writeLog(QString().sprintf("[CAN=%3d][主动防护][%s]防护级别[%d]防护结果[%s]", 
									canAddr, df->getDesc().toAscii().data(), tgInfo->level,
									dfLevel->result == 1 ? "成功" : "失败"));
					}

					destroyTG(canAddr);
					destroyDF(canAddr);
				}else{
					//兼容老版交流，交流只检测继电器状态，继电器闭合认为充电中
					//对于三相与直流，判断继电器闭合或充电机工作中则认为时候充电中
					if((canAddr < 150 && status.stFrameRemoteSingle.relay_status == 1) || 
						(canAddr > 150 && (status.stFrameRemoteSingle.relay_status == 1 || status.stFrameRemoteSingle.charge_status == 1))){
						QList<Defense *> listDF = createDF(canAddr);
						for(int i = 0; i < listDF.count(); i++){
							df = listDF.at(i);
							if(df->isTriggered(status) && !m_mapTriggeredDF.contains(canAddr)){
								tgInfo = createTG(canAddr);
								tgInfo->df = df;
								memcpy(tgInfo->dfLevel[tgInfo->level]->triggerTime, strTime.toAscii().data(), strTime.length());

								writeLog(QString().sprintf("[CAN=%3d][防护检测][%s]触发", 
											canAddr, df->getDesc().toAscii().data()));
								break;
							}
						}
					}else{
						if(!m_mapTriggeredDF.contains(canAddr))
							destroyDF(canAddr);
					}
				}

			}
		}
	}
}

QList<Defense *> ActiveDefend::createDF(uchar canAddr)
{
	QList<Defense *> listDF;

	if(!m_mapDF.contains(canAddr)){
		listDF = DefenseFactory::getDefenseList();
		m_mapDF[canAddr] = listDF;
	}else{
		listDF = m_mapDF[canAddr];	
	}
	return listDF;
}

void ActiveDefend::destroyDF(uchar canAddr)
{
	QList<Defense *> listDF;

	if(!m_mapDF.contains(canAddr))
		return;

	listDF = m_mapDF[canAddr];
	for(int i = 0; i < listDF.count(); i++){
		delete listDF.at(i);	
	}

	listDF.clear();

	m_mapDF.remove(canAddr);
}

TriggeredInfo *ActiveDefend::createTG(uchar canAddr)
{
	TriggeredInfo *tgInfo;

	tgInfo = new TriggeredInfo;
	memset(tgInfo, 0, sizeof(TriggeredInfo));

	tgInfo->canAddr = canAddr;

	tgInfo->dfLevel[0] = new DefendLevelInfo;
	memset(tgInfo->dfLevel[0], 0, sizeof(DefendLevelInfo));
	tgInfo->dfLevel[1] = new DefendLevelInfo;
	memset(tgInfo->dfLevel[1], 0, sizeof(DefendLevelInfo));

	m_mapTriggeredDF[canAddr] = tgInfo;

	return tgInfo;
}

void ActiveDefend::destroyTG(uchar canAddr)
{
	TriggeredInfo *tgInfo;

	if(!m_mapTriggeredDF.contains(canAddr))
		return;

	tgInfo = m_mapTriggeredDF[canAddr];

	for(int i = 0; i < LEVEL_COUNT; i++){
		if(tgInfo->dfLevel[i])
			delete tgInfo->dfLevel[i];
	}

	delete tgInfo;
	m_mapTriggeredDF.remove(canAddr);
}
