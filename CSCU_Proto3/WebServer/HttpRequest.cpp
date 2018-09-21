#include "HttpRequest.h"
#include <QDebug>

CHttpRequest::CHttpRequest(QString strToken)
{
	m_strToken = strToken;
	m_curlCode = CURLE_OK; 

	curl_global_init(CURL_GLOBAL_DEFAULT);
	m_curl = curl_easy_init();
}

CHttpRequest::~CHttpRequest()
{
    curl_easy_cleanup(m_curl);   
	curl_global_cleanup();
}

size_t CHttpRequest::recv(void *buffer, size_t size, size_t nmemb, void *stream)
{
	CHttpRequest *p = (CHttpRequest *)stream;
	p->m_arData.append((char *)buffer, size * nmemb);

    return size * nmemb;
}

void CHttpRequest::post(QUrl url, QByteArray arPost)
{
	struct curl_slist *list = NULL; 
	QString strAuth;

	m_url = url;

	if(m_strToken.isEmpty())
		strAuth = "Authorization;";
	else
		strAuth.sprintf("Authorization: %s", m_strToken.toAscii().data());

    list = curl_slist_append(list, strAuth.toAscii().data());
    list = curl_slist_append(list, "Content-Type: application/json; charset=utf-8");
    list = curl_slist_append(list, "Connection: Keep-Alive");
    list = curl_slist_append(list, "Accept-Language: en,*");
    list = curl_slist_append(list, "User-Agent: Mozilla/5.0");
    list = curl_slist_append(list, "Accept: ");
    list = curl_slist_append(list, "Expect: ");

	if(url.scheme() == "https"){
		curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0);
		curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 0);
		//curl_easy_setopt(m_curl,CURLOPT_CAPATH,"/mnt/nandflash/");  
	}
	/*
	curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 1);
	curl_easy_setopt(m_curl,CURLOPT_SSLCERT,"client-cert.pem");  
	curl_easy_setopt(m_curl,CURLOPT_SSLCERTPASSWD,"password");  
	curl_easy_setopt(m_curl,CURLOPT_SSLCERTTYPE,"PEM");  
	curl_easy_setopt(m_curl,CURLOPT_SSLKEY,"client-key.pem");  
	curl_easy_setopt(m_curl,CURLOPT_SSLKEYPASSWD,"password");  
	curl_easy_setopt(m_curl,CURLOPT_SSLKEYTYPE,"PEM");  
	*/
	curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, list);
    curl_easy_setopt(m_curl, CURLOPT_POST, 1); 
	curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, arPost.data());
	curl_easy_setopt(m_curl, CURLOPT_POSTFIELDSIZE, arPost.length());
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, recv);
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(m_curl, CURLOPT_URL, url.toString().toAscii().data());
	curl_easy_setopt(m_curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(m_curl, CURLOPT_MAXREDIRS, 0);
	curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, 120);
	curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT, 5);
	//curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1);

    m_curlCode = curl_easy_perform(m_curl);

 	curl_slist_free_all(list);

	emit finished(this);
}

QByteArray CHttpRequest::readAll()
{
	return m_arData;
}

int CHttpRequest::error()
{
	return m_curlCode;
}

QString CHttpRequest::errorString()
{
	if(m_curlCode != CURLE_OK)
		return curl_easy_strerror(m_curlCode);
	else
		return "";
}

QUrl CHttpRequest::url()
{
	return m_url;
}
