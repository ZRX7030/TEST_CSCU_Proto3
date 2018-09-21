#ifndef		__DBOperate_H__
#define		__DBOperate_H__

#include "Database/database.h"


#define			DB_PROCESS_RECORD_PATH			"/mnt/nandflash/database/process_record.db"
#define			DB_REAL_RECORD_PATH				"/mnt/nandflash/database/real_record.db"
#define			DB_PARAM_PATH					"/mnt/nandflash/database/param_config.db"
#define			DB_DOWN_SAVE_PATH				"/mnt/nandflash/database/down_save.db"
#define			DB_AUTHENTICATION_PATH			"/mnt/nandflash/database/authentication.db"

/*数据库类型定义*/
enum
{
	DB_PROCESS_RECORD,			//充电过程记录库
	DB_DOWN_SAVE,				//掉电保存库
	DB_REAL_RECORD,				//充电实时数据库
	DB_PARAM,					//参数配置数据库
	DB_AUTHENTICATION			//本地鉴权数据库
};

class DBOperate
{
private:
	database *db_process_record;
	database *db_down_save;
	database *db_real_record;
	database *db_param;
	database *db_authentication;

protected:
	DBOperate();

public:
	~DBOperate();

	static DBOperate *GetInstance();

	int DBSqlQuery(char *, void *result, int db_type);
	int DBSqlExec(char *sql, int db_type);
	int DBQueryFree(void *result);
	
	int DBSqlQueryBlob(char *, void *result, int db_type);
	int DBSqlExecBlob(char *sql, const unsigned char *data, int size, int db_type);
	int DBQueryFreeBlob(void *result);

	int DBCopy(const char *dst, int db_type);
};


#endif
