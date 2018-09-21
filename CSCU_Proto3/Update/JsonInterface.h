#ifndef			__JSON_INTERFACE_H__
#define			__JSON_INTERFACE_H__

#include <json-c/json.h>
#include <QMap>
#include <QString>

/**
 *请求升级传输的数据
 */
typedef struct __updateRequestData
{
	char cscuVer[30];
	char teuiVer[30];
	char stationAddr[30];
	char macAddr[30];
	char stationName[30];
	QMap<unsigned char, QString> chargerVer; 
}stUpdateRequestData;

/**
 *请求升级结果反馈
 */
typedef struct __updateRequestResultData
{
	unsigned char result;				//1 需要升级  2 不需要升级
	unsigned char curUpdateType;		//当前升级类型  1 cscu 2直流机
	char fileName[30];					//升级文件名
	char url[512];
	char md5[50];						//升级文件的md5值
	int updateTime;				//更新时间
	QMap<unsigned char, QString> chargerFile;	//充电机文件名
    char  CanStr[128];
}stUpdateRequestResultData;



struct json_object *jsonPackUpdateRequest(stUpdateRequestData requestData);
int jsonRequestResultParse(char *msg, stUpdateRequestResultData *result);
int jsonCmdTypeParse(char *msg);

char *getJsonPoint(struct json_object *object);
void freeJsonPoint(struct json_object *object);

#endif
