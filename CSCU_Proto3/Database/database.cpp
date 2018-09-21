#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "database.h"

#define sqlite_debug(format, ...) fprintf (stdout, "[%s:%d]: "format, __FILE__,__LINE__,## __VA_ARGS__)


database::database(char *db_name)
{
	pthread_mutex_init(&mutex_db,NULL);

	if(sqlite_database_open(&fd_db, db_name) != SQLITE_OK){
		printf("database \"%s\" open failed.",db_name);
	}
}

database::~database()
{
	sqlite_database_close(fd_db);
}
#if 0
static int sqlite_busy_callback(void *ptr, int count)
{
	if( count > 20 ){			//最多200ms
		return 0;    
	}

	usleep(10000); //延时10ms 
	return 1; 
}
#endif
/*
 *函数名称：sqlite_database_open
 *函数功能：打开sqlite数据库
 *输入参数：无
 *输出参数：db
 *返回值：  0 成功 -1 失败
 */
int database::sqlite_database_open(sqlite3 **db, char *db_path)
{
	int nresult = 0;

	nresult = sqlite3_open(db_path, db);	
	if(nresult != SQLITE_OK){
		return nresult;
	}
//	sqlite3_busy_handler(*db, sqlite_busy_callback, *db);
	return nresult;
}	

/*
 *函数名称：database_query_result
 *函数功能: 数据库查询结果的返回
 *输入参数：
 *输出参数：
 *返回值：  1 成功 0 失败
 */
int database::database_query_result(char **row_data, int row_num, int field_num, void *result)
{
	struct db_result_st *db_result = (struct db_result_st *)result;

	db_result->result = new char*[row_num*field_num];
	if(db_result->result == NULL)
		return 0;

	db_result->row = row_num;
	db_result->column = field_num;

	char **dst_point = db_result->result;
    for(int row=0; row<row_num; row++)
    {
        for(int cloumn=0; cloumn<field_num; cloumn++)
        {
            if(row_data[field_num+row*field_num+cloumn])
            {
				dst_point[row*field_num+cloumn] = new char[strlen(row_data[field_num+row*field_num+cloumn])+1];
				if(dst_point[row*field_num+cloumn])
					strcpy(dst_point[row*field_num+cloumn], row_data[field_num+row*field_num+cloumn]);
			}
			else
				dst_point[row*field_num+cloumn] = NULL;
		}
	}

	return 1;
}

int database::sqlite_query_free(void *param)
{
	struct db_result_st *db_result = (struct db_result_st *)param;

	if(!db_result->result)
		return 1;

    for(int row=0; row< db_result->row; row++)
    {
        for(int column=0; column<db_result->column; column++)
        {
			if(db_result->result[row*db_result->column+column])
				delete db_result->result[row*db_result->column+column];
		}
	}
	delete[] db_result->result;

	return 1;
}
/*
 *函数名称：sqlite_database_query
 *函数功能：数据库查询
 *输入参数：
 *输出参数：无
 *返回值：  0 成功 -1失败
 */
int database::sqlite_database_query(char *sql, void *result)
{
	char *errmsg = NULL;
	int nresult = 0;
	int nrow = 0;
	int ncolumn = 0;
	char **presult = NULL;

	if(!fd_db || !sql){
		return -1;
	}
	
	memset(result, 0, sizeof(struct db_result_st));
	
	pthread_mutex_lock(&mutex_db);
	nresult = sqlite3_get_table(fd_db, sql, &presult, &nrow, &ncolumn, &errmsg);
	pthread_mutex_unlock(&mutex_db);
	if(nresult != SQLITE_OK){
		fprintf(stderr, "%s:  %s\n", sql, errmsg);
		sqlite3_free(errmsg);
		return -1;
	}

	database_query_result(presult, nrow, ncolumn, result);

	sqlite3_free(errmsg);
	sqlite3_free_table(presult);

	return 0;
}

/*
 *函数名称：sqlite_database_exec
 *函数功能: 执行sql语句
 *输入参数：
*输出参数：无
*返回值：  0 成功 -1失败
*/
int database::sqlite_database_exec(char *sql)
{
	int nresult = 0;
	char *errmsg = NULL;

	if(!fd_db || !sql){
		return -1;
	}

	pthread_mutex_lock(&mutex_db);
	nresult = sqlite3_exec(fd_db, sql, NULL, NULL, &errmsg);
	pthread_mutex_unlock(&mutex_db);
	if(nresult != SQLITE_OK){
		fprintf(stderr, "%s:  %s\n", sql, errmsg);
		sqlite3_free(errmsg);
		return -1;
	}

	sqlite3_free(errmsg);
	return 0;
}

/*
*函数名称：sqlite_database_begin_transaction
*函数功能：数据库开启事务函数
*输入参数：
*输出参数：无
*返回值：  0 成功 -1 失败
*/
int database::sqlite_database_begin_transaction()
{
	if(!fd_db){
		return -1;
	}
	
	int nresult = sqlite3_exec(fd_db, "begin", 0, 0, 0);

	return nresult;
}

/*
*函数名称：sqlite_database_commit_transaction
*函数功能：数据库提交事务函数
*输入参数： 
*输出参数：无
*返回值：  0 成功 -1 失败
*/
int database::sqlite_database_commit_transaction()
{
	if(!fd_db){
		return -1;
	}
	
	int nresult = sqlite3_exec(fd_db, "commit", 0, 0, 0);

	return nresult;
}

/*
*函数名称：sqlite_insert_blob
*函数功能：插入blob数据
*输入参数： 
*输出参数：无
*返回值：  0 成功 -1 失败
*/
int database::sqlite_exec_blob(const char *sql, const unsigned char *zBlob, int nBlob)
{
	//const char *zSql = "INSERT INTO blobs(key, value) VALUES(?, ?)";
	sqlite3_stmt *pStmt;
	int rc;

	do {
		pthread_mutex_lock(&mutex_db);
		rc = sqlite3_prepare(fd_db, sql, -1, &pStmt, 0);
		if( rc!=SQLITE_OK ){
			fprintf(stderr, "sqlite_exec_blob error:%s\n", sql);
			pthread_mutex_unlock(&mutex_db);
			return rc;
		}
		sqlite3_bind_blob(pStmt, 1, zBlob, nBlob, SQLITE_STATIC);
		rc = sqlite3_step(pStmt);
		rc = sqlite3_finalize(pStmt);
		pthread_mutex_unlock(&mutex_db);
	}while( rc==SQLITE_SCHEMA );

	return rc;
}

/*
*函数名称：sqlite_query_blob
*函数功能：插入blob数据
*输入参数： 
*输出参数：无
*返回值：  0 成功 -1 失败
*/
int database::sqlite_query_blob( const char *sql, void *result)
{
	struct db_blob_result_st *db_result = (struct db_blob_result_st *)result;
	
	//const char *zSql = "SELECT value FROM blobs WHERE key = ?";
	sqlite3_stmt *pStmt;
	int rc;
	
	//int tmp_size = 0;
	//unsigned char *pdata = NULL;
	//int row = 0;

	pthread_mutex_lock(&mutex_db);
	rc = sqlite3_prepare(fd_db, sql, -1, &pStmt, 0);
	if( rc!=SQLITE_OK ){
		fprintf(stderr, "sqlite_query_blob error:%s\n", sql);
		pthread_mutex_unlock(&mutex_db);
		return rc;
	}
	
	while(SQLITE_ROW  == sqlite3_step(pStmt))
	{
		struct db_blob_st db_blob;

		db_blob.size = sqlite3_column_bytes(pStmt, 0);
		if(db_blob.size)
		{
			db_blob.data = new unsigned char[db_blob.size+1];
			memcpy(db_blob.data, sqlite3_column_blob(pStmt, 0), db_blob.size);
			db_result->blob_list.append(db_blob);
		}
	}

	rc = sqlite3_finalize(pStmt);
	pthread_mutex_unlock(&mutex_db);

	return rc;
}
/*
 *函数名称：sqlite_blob_free
 *函数功能: 查询数据blob数据后释放函数
 *输入参数： 
 *输出参数：
 *返回值：  
 */
int database::sqlite_blob_free(void *result)
{
	struct db_blob_result_st *db_result = (struct db_blob_result_st *)result;
	for(int i=0; i< db_result->blob_list.size(); i++)	
	{
		struct db_blob_st blob = db_result->blob_list.at(i);
		if(blob.data)
			delete blob.data;
	}

	db_result->blob_list.clear();
	return 1;
}
/*
 *函数名称：sqlite_database_close
 *函数功能：数据库关闭函数
 *输入参数：db:数据库 
 *输出参数：无
 *返回值：  无
 */
void database::sqlite_database_close(sqlite3 *db)
{
	if(!db){
		return;
	}

	sqlite3_close(db);
}
	
int database::sqlite_dbcopy(const char *src, const char *dst)
{
	int ret = 0;
	char cmd[256];

	snprintf(cmd, sizeof(cmd), "cp -f %s %s", src, dst);

	pthread_mutex_lock(&mutex_db);
	ret = system(cmd);
	pthread_mutex_unlock(&mutex_db);

	return ret;
}
