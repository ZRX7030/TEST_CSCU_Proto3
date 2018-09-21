#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <QObject>
#include <QUrl>
#include "curl/curl.h"

class CHttpRequest : public QObject
{
    Q_OBJECT

public:
	CHttpRequest(QString strToken = "");
	~CHttpRequest();

	void post(QUrl url, QByteArray arPost);
	QByteArray readAll();
	QString errorString();
	int error();
	QUrl url();

signals:
	void finished(CHttpRequest *p);

private:
	static size_t recv(void *buffer, size_t size, size_t nmemb, void *stream);

	QByteArray 	m_arData;
	QUrl 		m_url;
	QString 	m_strToken;
	QString 	m_strError;
	CURLcode 	m_curlCode;
	CURL		*m_curl;
};

#endif
