#ifndef			__JSON_INTERFACE_H__
#define			__JSON_INTERFACE_H__

#include <json-c/json.h>

/*库类型定义*/
enum
{
	DB_PROCESS_RECORD=0,
	DB_REAL_RECORD
};

/*过程数据库表定义*/
enum
{
	PROCESS_BMS_STATIC=0,      
	PROCESS_CSCU_REBOOT,
	PROCESS_TERMINAL_ONLINE,
	PROCESS_CHARGER_ORDER,     
	PROCESS_PULL_GUN,       
	PROCESS_CSCU_ONLINE,
	PROCESS_TERMINAL_FAULT,
	PROCESS_COUNT
};
/*实时数据库表定义*/
enum
{
	REAL_CHAREE_PROCESS=0,
	REAL_CHARGE_ENERGY,
	REAL_BMS_DYNAMIC,
	REAL_COUNT
};

struct json_object *json_pack_wohle(int db_type, int table_type, void *param);
char *get_json_point(struct json_object *object);
void free_json_point(struct json_object *object);

#endif
