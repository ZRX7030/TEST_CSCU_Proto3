#include <stdio.h>

#include "json_interface.h"
#include "database.h"
#include "config.h"

const char *db_type_name[]={"process","real"};
const char *process_table_name[PROCESS_COUNT]={"bms", "reboot","term_online","order","gun","cscu_online","fault"};
const char *real_table_name[REAL_COUNT]={"process", "energy","bms"};

/**
 *组织json格式数据内容组织
 */
struct json_object *json_pack_value(void *param)
{
	struct db_result_st *in_param= (struct db_result_st *)param;

	struct json_object *value_array = json_object_new_array();
	if(!value_array)
		return NULL;
	
	int i,k;	
	for(i=0; i<in_param->row; i++)
	{
		struct json_object *array = json_object_new_array();
		if(!array)
			continue;
		for(k=0; k < in_param->column; k++)
			json_object_array_add(array, json_object_new_string(in_param->result[i*in_param->column+k]));

		json_object_array_add(value_array, array);
	}

	return value_array;
}
/**
 *获取整个完整的json通信格式
 */
struct json_object *json_pack_wohle(int db_type, int table_type, void *param)
{
	struct json_object *value_array = NULL;
	value_array =  json_pack_value(param);
	if(NULL == value_array)
		return NULL;

	struct json_object *rpc_object = NULL;  
	rpc_object = json_object_new_object();  
	if (NULL == rpc_object)  
	{  
		json_object_put(value_array);
		printf("new json object failed.\n");  
		return NULL;  
	}  

	json_object_object_add(rpc_object, "db_type", json_object_new_string(db_type_name[db_type]));
	if(db_type == DB_PROCESS_RECORD)
		json_object_object_add(rpc_object, "table_type", json_object_new_string(process_table_name[table_type]));  
	else if(db_type == DB_REAL_RECORD)
		json_object_object_add(rpc_object, "table_type", json_object_new_string(real_table_name[table_type]));  

	json_object_object_add(rpc_object, "station", json_object_new_string(args.station_name));

	json_object_object_add(rpc_object, "value", value_array);

	return rpc_object;
}

/**
 *
 */
char *get_json_point(struct json_object *object)
{
	return json_object_to_json_string(object);
}

/**
 *
 */
void free_json_point(struct json_object *object)
{
	json_object_put(object);
}
