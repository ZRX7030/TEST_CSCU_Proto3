#ifndef		__HTTP_INTERFACE_H__
#define		__HTTP_INTERFACE_H__

/**
 *http请求后返回的数据
 */
struct receive_buff_st
{
	char *rec_buff;
	int buff_size;
	
	int rec_size;
};

int sendHttpRequest(char *url, char *data, int len, void *in_param, void *stream);
//int dealRecvData(void *buffer, void *stream); 

#endif
