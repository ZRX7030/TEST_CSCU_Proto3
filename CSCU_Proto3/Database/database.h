#ifndef _DATABASE_H_
#define _DATABASE_H_

#include <QList>
#include <pthread.h>
#include <unistd.h>
#include "sqlite3.h"

/**
 *数据库查询结果返回
 */
struct db_result_st
{
	char **result;
	int row;
	int column;
};

struct db_blob_st
{
	unsigned char *data;
	int size;
};

struct db_blob_result_st
{
	QList<struct db_blob_st> blob_list;
};


class database
{

private:
	pthread_mutex_t mutex_db;
	sqlite3 *fd_db;

	int sqlite_database_open(sqlite3 **, char *);
	void sqlite_database_close(sqlite3 *);
	int database_query_result(char **, int, int, void*);

public:
	database(char *dbname);
	~database();

	int sqlite_database_query(char *, void *);
	static int sqlite_query_free(void *);

	int sqlite_exec_blob(const char *, const unsigned char *, int);
	int sqlite_query_blob(const char *, void *);
	static int sqlite_blob_free(void *);
	
	//int sqlite_busy_callback(void *, int);

	int sqlite_database_exec(char *);
	int sqlite_database_begin_transaction();
	int sqlite_database_commit_transaction();

	int sqlite_dbcopy(const char *src, const char *dst);
};

#endif

