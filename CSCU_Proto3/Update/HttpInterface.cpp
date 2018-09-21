#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

#include "HttpInterface.h"
//#include "log.h"



/**
 *处理发送请求后接收到的数据
 */
size_t recv_data(void *buffer, size_t size, size_t nmemb, void *stream)  
{
	struct receive_buff_st *buff=( struct receive_buff_st * )stream;
	
	if( !buff->rec_buff )
		return  size*nmemb;

	int len=size * nmemb;
	if( (buff->rec_size + len ) >= buff->buff_size )
			len=buff->buff_size-buff->rec_size-1;

	memcpy(buff->rec_buff+ buff->rec_size, buffer, len);
	buff->rec_size+=len;
	
	buff->rec_buff[buff->rec_size]=0;
	
	return  size*nmemb;
}  
#if 0
/**
 *处理接收到到的http返回的数据
 */
int dealRecvData(void *buffer, void *stream)  
{
	struct receive_buff_st *buff = (struct receive_buff_st *)buffer;

	if(buff->rec_size == 0)
		return 0;
	
	buff->rec_size = 0;

	return  -1;
} 
#endif

/**
 * 失败返回 -1，成功返回正值
 */
int sendHttpRequest(char *url, char *data, int len, void *in_param, void *stream)
{
	CURL *curl=0;  
	int ret_code=-1;

	struct curl_httppost *post=NULL;//*last = NULL;
	struct curl_slist *header  = NULL;
	
	curl = curl_easy_init();  
	if (!curl)
		return -1;

#if 0
	curl_formadd(&post, &last,
			CURLFORM_COPYNAME, "data",
			CURLFORM_BUFFERPTR, data,
			CURLFORM_BUFFERLENGTH,len,
			CURLFORM_END);
	curl_formadd(&post, &last,
			CURLFORM_COPYNAME, "flag",
			CURLFORM_COPYCONTENTS, (char *)in_param,
			CURLFORM_END);
	header  = curl_slist_append(header , "content-type: multipart/form-data; charset=UTF-8;");
	header  = curl_slist_append(header , "Expect:");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
#endif
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_HEADER, 0);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, len);
	//curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);		
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);				//下载超时时间
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	//curl_easy_setopt(curl, CURLOPT_ENCODING, "gzip");
	//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);  //打开调试
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, recv_data);   //用于接收返回的数据  
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, stream);

	if(curl_easy_perform(curl)==CURLE_OK ) //开始数据传输,阻塞
		ret_code=1;
	//else
		//curl_easy_getinfo(url, CURLINFO_RESPONSE_CODE, &ret_code); //获取返回信息

	//释放资源
	curl_formfree(post);
	curl_slist_free_all(header);
	curl_easy_cleanup(curl);  

	return ret_code;
}


