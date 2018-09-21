#ifndef _DATABASE_H_
#define _DATABASE_H_

//#include <QList>
#include <pthread.h>

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
	//QList<struct db_blob_st> blob_list;
};


int sqlite_database_open(sqlite3 **, char *);
void sqlite_database_close(sqlite3 *);
int database_query_result(char **, int, int, void*);


int sqlite_database_query(sqlite3 *, char *, void *);
int sqlite_query_free(void *);

int sqlite_exec_blob(sqlite3 *, const char *, const unsigned char *, int);
int sqlite_query_blob(sqlite3 *, const char *, void *);
int sqlite_blob_free(void *);

int sqlite_database_exec(sqlite3 *, char *);
int sqlite_database_begin_transaction(sqlite3 *);
int sqlite_database_commit_transaction(sqlite3 *);

#endif

