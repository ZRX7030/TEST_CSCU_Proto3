#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <QObject>
#include <QMap>
#include <QTimerEvent>
#include "HttpRequest.h"
#include "GeneralData/ModuleIO.h"
#include "DBOperate.h"
#include "ParamSet.h"
#include "Log.h"

typedef enum{
	IF_TOKEN = 0,
	IF_CARD,
	IF_CAR,
	IF_ORDER,
	IF_COUNT	
}INTERFACE_TYPE;

enum{
	LOG_DEBUG = 1,
	LOG_INFO,
	LOG_WARN,
	LOG_ERROR
};

class CWebServer : public CModuleIO
{
    Q_OBJECT
public:
    CWebServer();
    ~CWebServer();

    //根据配置选项初始化
    int InitModule( QThread* pThread);
    //注册设备到总线
    int RegistModule();
    //启动模块
    int StartModule();
    //停止模块
    int StopModule();
    //模块工作状态
    int ModuleStatus();

signals:
    void sigToBus(InfoMap, InfoAddrType);

public slots:
	void slotFromBus(InfoMap mapInfo, InfoAddrType type);

private slots:
	void slot_onWorkThreadRun();
	void slot_onHttpFinished(CHttpRequest *request);

private:
	void timerEvent(QTimerEvent *event);
	void request(QByteArray &arData, INTERFACE_TYPE type);

	void aesEncrypt(QByteArray arData, QByteArray &arEncrypt, bool enc);
	void signatureData(QString key, QString secret, QByteArray &arSig);
	void createRequestJson(QByteArray &arData, QByteArray &arJson);
	bool parseResponseJson(QByteArray &arJson, QByteArray &arData);

	void requestToken();
	bool parseToken(QByteArray &arJson, QString &strToken, int &iAlive);
	void requestCard();
	int parseCard(QByteArray &arJson);
	void requestCar();
	int parseCar(QByteArray &arJson);
	void sendOrder();
	int parseOrder(QByteArray &arJson);
	void WriteLog(QString strLog, int iLevel = LOG_INFO);
	void ParseWebUrl(InfoMap &mapInfo);

	QString m_strOperateId;
	QString m_strOperateSecret;
	QString m_strDataSecret;
	QString m_strKey;
	QString m_strIv;
	QString m_strUrl;
	int 	m_iPort;
	QUrl m_url[IF_COUNT];
	QString m_strStation;

	bool m_bTerminate;
	int m_iServerTimer;
	QString m_strToken;
	int m_iTokenAlive; 
	int m_iRequest;
	int m_iCardPage;
	int m_iCarPage;
	int m_iCardTimeout;
	int m_iCarTimeout;
	int m_iOrderTimeout;
	int m_iSendTimeout;

	DBOperate *m_pDatabase;
	ParamSet *m_pSetting;
    Log *m_pLog;
};

#endif // WEBSERVER_H
