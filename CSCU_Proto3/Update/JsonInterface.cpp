#include <stdio.h>
#include <QDebug>

#include "JsonInterface.h"

/**
 *获取整个完整的json通信格式
 */
struct json_object *jsonPackUpdateRequest(stUpdateRequestData requestData)
{
	char version[512]={0};
	char addr[512] = {0};
	char *version_point = version;
	char *addr_point = addr;

	struct json_object *value_array = json_object_new_array();
	struct json_object *param_array = json_object_new_array();
//	struct json_object *charge_version_array = json_object_new_array();
	struct json_object *rpc_object =  json_object_new_object(); 
	
	//json_object_put(value_array);
	/*充电机版本信息*/
	QMap<unsigned char, QString>::iterator it;
	for( it = requestData.chargerVer.begin(); it != requestData.chargerVer.end(); ++it) 
	{ 
	/*	struct json_object *obj = json_object_new_object(); 
		json_object_object_add(obj, QString::number(it.key(), 10).toAscii().data(), json_object_new_string(it.value().toAscii().data()));
		json_object_array_add(charge_version_array, obj);*/

        if(it.value().size() == 0)
            continue;

		snprintf(version_point, sizeof(version) - (version_point - version),"%s-",it.value().toAscii().data());
		snprintf(addr_point, sizeof(addr) - (addr_point - addr),"%d-",(int)it.key());
		version_point = version + strlen(version);	
		addr_point = addr + strlen(addr);	
	}
	*(version_point-1) = 0;
	*(addr_point-1) = 0;

	struct json_object *array_param_obj = json_object_new_object(); 
	json_object_object_add(array_param_obj, "station_name", json_object_new_string(requestData.stationName));
	json_object_object_add(array_param_obj, "mac", json_object_new_string(requestData.macAddr));
	json_object_object_add(array_param_obj, "station1", json_object_new_string(requestData.stationAddr));
	json_object_object_add(array_param_obj, "station2", json_object_new_string(requestData.stationAddr));
	json_object_object_add(array_param_obj, "station3", json_object_new_string(requestData.stationAddr));
	json_object_array_add(param_array, array_param_obj);
	

    struct json_object *array_data_obj = json_object_new_object();
    json_object_object_add(array_data_obj, "terminal_version", json_object_new_string(version));
    json_object_object_add(array_data_obj, "terminal_addr", json_object_new_string(addr));
	json_object_object_add(array_data_obj, "cscu_version", json_object_new_string(requestData.cscuVer));
	json_object_object_add(array_data_obj, "teui_version", json_object_new_string(requestData.teuiVer));
	json_object_array_add(value_array, array_data_obj);

	json_object_object_add(rpc_object, "cmd", json_object_new_string("update_detect"));
	json_object_object_add(rpc_object, "param", param_array);
	json_object_object_add(rpc_object, "data", value_array);

	return rpc_object;
}
/**
 *接收命令类型解析
 */
int jsonRequestResultParse(char *msg, stUpdateRequestResultData *result)
{
	struct json_object *rpc = json_tokener_parse(msg);
	if(!rpc)
		return 0;
	struct json_object *result_obj = NULL;
	char *value = NULL;

	/*命令解析*/
	json_object_object_get_ex(rpc, "cmd", &result_obj);	
	if(result_obj)
	{
		value = (char *)json_object_get_string(result_obj);
		if(strcmp(value, "update_detect") != 0)
			return 0;
	}

	/*data域解析*/
	json_object_object_get_ex(rpc, "data", &result_obj);	
	if(result_obj && json_type_array == json_object_get_type(result_obj))
	{
		struct json_object *parse_obj = NULL; 
		struct json_object *data_obj = json_object_array_get_idx(result_obj, 0 );
		json_object_object_get_ex(data_obj, "status", &parse_obj);	
		if(parse_obj)
		{
			result->result = json_object_get_int(parse_obj);
		}
		if(result->result == 1)
		{
			json_object_object_get_ex(data_obj, "update_device", &parse_obj);	
			if(parse_obj)
			{
				result->curUpdateType = json_object_get_int(parse_obj);
			}
			
			json_object_object_get_ex(data_obj, "update_time", &parse_obj);	
			if(parse_obj)
			{
				result->updateTime = json_object_get_int(parse_obj);
			}

			json_object_object_get_ex(data_obj, "download_url", &parse_obj);	
			if(parse_obj)
			{
				value = (char *)json_object_get_string(parse_obj);
				snprintf(result->url, sizeof(result->url) ,"%s", value);
			}

			json_object_object_get_ex(data_obj, "file_md5", &parse_obj);	
			if(parse_obj)
			{
				value = (char *)json_object_get_string(parse_obj);
				snprintf(result->md5, sizeof(result->md5) ,"%s", value);
			}
			
			json_object_object_get_ex(data_obj, "can_list", &parse_obj);	
			if(parse_obj)
			{
                value = (char *)json_object_get_string(parse_obj);
                snprintf(result->CanStr, sizeof(result->CanStr) ,"%s", value);
            }
        }
	}
	return 1;
}

/*
 * json格式命令类型解析
 */
int jsonCmdTypeParse(char *msg)
{
	int ret = 0;
	struct json_object *rpc = json_tokener_parse(msg);
	if(!rpc)
		return 0;

	struct json_object *cmd_obj = NULL;
	json_object_object_get_ex(rpc, "cmd", &cmd_obj);
	if(cmd_obj ) //&& json_object_is_type(cmd_obj, json_type_object))
	{
		char *cmd_value = (char *)json_object_get_string(cmd_obj);
		if(strcmp(cmd_value, "repond_update") == 0)
			ret = 1;
	}

	json_object_put(rpc);

	return ret;
}
/**
 *
 */
char *getJsonPoint(struct json_object *object)
{
	return (char *)json_object_to_json_string(object);
}

/**
 *
 */
void freeJsonPoint(struct json_object *object)
{
	json_object_put(object);
}
