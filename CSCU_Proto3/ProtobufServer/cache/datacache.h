#ifndef DATACACHE_H
#define DATACACHE_H

#include "Infotag/CSCUBus.h"
#include <QList>
#include <pthread.h>

typedef struct _DataPoint
{
	QString canAddr;	//设备can地址
	QString devType;	//设备类型
	QString devName;	//设备名称
	QString dataName;	//数据名称
	QString dataType;	//数据类型
	QString value;		//数据值
	QString valueDesc;	//数据值描述
	QString status;		//数据品质
}DataPoint;

typedef QList<DataPoint> QDataPointList;
Q_DECLARE_METATYPE(QDataPointList)

class DataCache
{
public:
	DataCache();
	~DataCache();

	//创建数据点表
	static bool createTable();
	//销毁数据点表
	static void destroyTable();
	//运行数据查询
	static bool query(QString strKey, QDataPointList &list, QString strQuery);
	//告警信息
	static bool alarm(InfoMap &map, QDataPointList &burst);
	//数据更新
	static bool update(InfoMap &map, QDataPointList &burst);
	//保存设备规格信息
	static bool saveDevSpec(InfoMap &map);
	//保存缓存数据
	static bool saveCache();

private:
	static pthread_mutex_t mutex;
	static QVariantMap mapModel;
	static QVariantMap mapPoint;
	static QVariantMap mapDevSpec;
	static QVariantMap mapRealSpec;
};

#endif
