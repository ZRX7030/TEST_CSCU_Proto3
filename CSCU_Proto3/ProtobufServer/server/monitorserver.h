#ifndef MONITORSERVER_H
#define MONITORSERVER_H

#include "ProtobufServer/server/protobufserver.h"

class MonitorServer : public ProtobufServer
{
    Q_OBJECT
public:
    MonitorServer();
	~MonitorServer();

	//实时数据
	static bool onRealData(InfoMap map, InfoAddrType type);
	//网络配置
	void getNetSetting(string &host, ushort &port, int &encrypt);
	//日志
	void writeLog(QString strLog, int iLevel = 2);

public:
	//模块导出接口
    int RegistModule();

signals:
	void signalBurst(QDataPointList burst);

private slots:
	void slot_onThreadRun();
	void slot_onDataBurst(QDataPointList burst);
    void slotFromBus(InfoMap mapInfo, InfoAddrType type);
};

#endif // MONITORSERVER_H
