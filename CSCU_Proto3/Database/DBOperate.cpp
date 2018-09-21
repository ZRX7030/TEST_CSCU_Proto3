#include <iostream> 

#include "DBOperate.h"

DBOperate::DBOperate()
{
	db_process_record = new database((char *)DB_PROCESS_RECORD_PATH);
	db_real_record = new database((char *)DB_REAL_RECORD_PATH);
	db_down_save = new database((char *)DB_DOWN_SAVE_PATH);
	db_param = new database((char *)DB_PARAM_PATH);
	db_authentication = new database((char *)DB_AUTHENTICATION_PATH);
}

DBOperate::~DBOperate()
{
	delete db_process_record;
	delete db_down_save;
	delete db_real_record;
	delete db_param;
	delete db_authentication;
}

DBOperate *DBOperate::GetInstance()
{
	static DBOperate *ins = NULL;
	if(!ins){
		ins = new DBOperate();
	}

	return ins;
}

/*获取整个表的数据
*/
int DBOperate::DBSqlQuery(char *sql, void *result, int db_type)
{
	int ret = 0;
	switch(db_type)
	{
		case DB_PROCESS_RECORD:
			ret = db_process_record->sqlite_database_query(sql, result);
			break;
		case DB_DOWN_SAVE:
			ret = db_down_save->sqlite_database_query(sql, result);
			break;
		case DB_REAL_RECORD:
			ret = db_real_record->sqlite_database_query(sql, result);
			break;
		case DB_PARAM:
			ret = db_param->sqlite_database_query(sql, result);
			break;
		case DB_AUTHENTICATION:
			ret = db_authentication->sqlite_database_query(sql, result);
			break;
		default: break;
	}

	return ret;
}

/*执行一条指令
*/
int DBOperate::DBSqlExec(char *sql, int db_type)
{
	int ret = 0;
	switch(db_type)
	{
		case DB_PROCESS_RECORD:
			ret = db_process_record->sqlite_database_exec(sql);
			break;
		case DB_DOWN_SAVE:
			ret = db_down_save->sqlite_database_exec(sql);
			break;
		case DB_REAL_RECORD:
			ret = db_real_record->sqlite_database_exec(sql);
			break;
		case DB_PARAM:
			ret = db_param->sqlite_database_exec(sql);
			break;
		case DB_AUTHENTICATION:
			ret = db_authentication->sqlite_database_exec(sql);
			break;
		default: break;
	}

	return ret;
}

int DBOperate::DBQueryFree(void *result)
{
	return database::sqlite_query_free(result);
}

/*查询一块数据
*/
int DBOperate::DBSqlQueryBlob(char *sql, void *result, int db_type)
{
	int ret = 0;
	switch(db_type)
	{
		case DB_PROCESS_RECORD:
			ret = db_process_record->sqlite_query_blob(sql, result);
			break;
		case DB_DOWN_SAVE:
			ret = db_down_save->sqlite_query_blob(sql, result);
			break;
		case DB_REAL_RECORD:
			ret = db_real_record->sqlite_query_blob(sql, result);
			break;
		case DB_PARAM:
			ret = db_param->sqlite_query_blob(sql, result);
			break;
		case DB_AUTHENTICATION:
			ret = db_authentication->sqlite_query_blob(sql, result);
			break;
		default: break;
	}

	return ret;
}

/*插入一块数据
*/
int DBOperate::DBSqlExecBlob(char *sql, const unsigned char *data, int size, int db_type)
{
	int ret = 0;
	switch(db_type)
	{
		case DB_PROCESS_RECORD:
			ret = db_process_record->sqlite_exec_blob(sql, data, size);
			break;
		case DB_DOWN_SAVE:
			ret = db_down_save->sqlite_exec_blob(sql,data, size);
			break;
		case DB_REAL_RECORD:
			ret = db_real_record->sqlite_exec_blob(sql, data, size);
			break;
		case DB_PARAM:
			ret = db_param->sqlite_exec_blob(sql, data, size);
			break;
		case DB_AUTHENTICATION:
			ret = db_authentication->sqlite_exec_blob(sql, data, size);
			break;
		default: break;
	}

	return ret;
}

int DBOperate::DBQueryFreeBlob(void *result)
{
	return database::sqlite_blob_free(result);
}
/**
 *数据库复制操作  
 * 返回值：0 成功  非0值失败
 */
int DBOperate::DBCopy(const char *dst, int db_type)
{
	int ret=0;
	switch(db_type)
	{
		case DB_PROCESS_RECORD:
			ret = db_process_record->sqlite_dbcopy(DB_PROCESS_RECORD_PATH, 	dst);
			break;
		case DB_DOWN_SAVE:
			ret = db_down_save->sqlite_dbcopy(DB_DOWN_SAVE_PATH, dst);
			break;
		case DB_REAL_RECORD:
			ret = db_real_record->sqlite_dbcopy(DB_REAL_RECORD_PATH,dst);
			break;
		case DB_PARAM:
			ret = db_param->sqlite_dbcopy(DB_PARAM_PATH, dst);
			break;
		case DB_AUTHENTICATION:
			ret = db_authentication->sqlite_dbcopy(DB_AUTHENTICATION_PATH, dst);
			break;
		default: break;
	}
	return ret;
}
