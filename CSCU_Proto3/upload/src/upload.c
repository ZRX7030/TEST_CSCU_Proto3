#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "database.h"
#include "upload.h"
#include "http_interface.h"
#include "json_interface.h"
#include "compress.h"
#include "config.h"
#include "log.h"

#define		PROCESS_RECORD_DB_NAME			"/mnt/nandflash/database/process_record.db"
#define		REAL_RECORD_DB_NAME				"/mnt/nandflash/database/real_record.db"

struct receive_buff_st http_rcv_buff;

void upload_init(void)
{
	http_rcv_buff.rec_buff = malloc(2*1024);
	http_rcv_buff.buff_size = 2*1024;
}

void upload_deinit(void)
{
	free(http_rcv_buff.rec_buff);
}

/**
 *过程记录数据库内容上传
 */
void upload_process_record(void)
{
	const char *table_name[] =		{"bms_static_table","cscu_reboot_table","terminal_online_table","charge_order","plug_gun_table","cscu_online_table","terminal_fault_table"};				//表名
	const char *condition_field[]=	{"reocrd_time","record_time","charger_online_time","EndTime","pull_record_time","terminal_offline_time","fault_stop_time"};				//判断条件字段
	const int condition_var[] =     {2,2,2,2,2,2,2};					   //0 无字段 1 内容不为空 2 判断时间
	const int table_type[] = {PROCESS_BMS_STATIC, PROCESS_CSCU_REBOOT, PROCESS_TERMINAL_ONLINE, PROCESS_CHARGER_ORDER, PROCESS_PULL_GUN, PROCESS_CSCU_ONLINE, PROCESS_TERMINAL_FAULT};

	sqlite3 *fdb =NULL;
	struct db_result_st query_result;
	int exit_flag = 0, k=0, count_num = 0;
	char cmd_buff[512];
	char time_buff[20];

	time_t s;
	struct tm *old_time;
	s = time((time_t*)NULL);
	s -= 259200;			//减三天的时间
	old_time = gmtime(&s); 
	snprintf(time_buff, sizeof(time_buff), "%04d-%02d-%02d %02d:%02d:%02d", 
			old_time->tm_year+1900, old_time->tm_mon+1, old_time->tm_mday,  old_time->tm_hour, old_time->tm_min, old_time->tm_sec);
	sqlite_database_open(&fdb, PROCESS_RECORD_DB_NAME);
	for(k =0; k<PROCESS_COUNT; k++)
	{
		exit_flag = 0;
		count_num = 0;	
		while(!exit_flag)
		{
			int ret = 0;
			//int compress_len = 0;
			unsigned char *compress_buff = NULL;
			
			/*单张表超过了最大上传条数*/
			if(count_num > args.max_count)
			{
				exit_flag = 1;
				continue;
			}

			/*查询*/
			if(condition_var[k] == 0)
				snprintf(cmd_buff,sizeof(cmd_buff),"select * from %s order by id limit 50", table_name[k]);
			else if(condition_var[k] == 1)
				snprintf(cmd_buff,sizeof(cmd_buff),"select * from %s where %s!=\"\" order by id limit 50",table_name[k], condition_field[k]);
			else if(condition_var[k] == 2)
				snprintf(cmd_buff,sizeof(cmd_buff),"select * from %s where %s!=\"\" and %s <\"%s\" order by id limit 50",table_name[k], condition_field[k], condition_field[k], time_buff);

			ret = sqlite_database_query(fdb, cmd_buff, &query_result);
			if(ret ==-1 || query_result.row == 0)
			{
				char str_buff[200];
				exit_flag = 1;
				snprintf(str_buff, sizeof(str_buff),"sql=%s ret=%d row=%d",  cmd_buff, ret, query_result.row);
				write_syslog("process record, query faild or empty: ", str_buff);
				continue;
			}
			printf("upload table name: %s\n", table_name[k]);	
			count_num += query_result.row;

			/*压缩上传*/
			struct json_object *object=json_pack_wohle(DB_PROCESS_RECORD, table_type[k], &query_result);
			char *value = get_json_point(object);
			int value_len = strlen(value);

			ret = -1;
			int real_len=0;
			compress_buff = malloc(value_len);
			if(compress_buff)
				ret = def((unsigned char *)value, value_len, compress_buff, &real_len);
			//free_json_point(object);
			if(ret == 0)
			{
				snprintf(cmd_buff, sizeof(cmd_buff), "http://%s/cscu-bin", args.host_port);
				ret = send_http_request(cmd_buff, (char *)compress_buff, real_len, "upload", &http_rcv_buff);
				if(-1 == deal_recv_data(&http_rcv_buff, value))
					write_syslog("upload_faild, table_name: ", table_name[k]);
				//printf("back code =%d value=%s\n", ret, value);
			}
			free(compress_buff);
			free_json_point(object);

			/*删除已经查询*/
			snprintf(cmd_buff, sizeof(cmd_buff), "delete from %s where id >=%d and id <=%d",
					table_name[k], atoi(query_result.result[0]), atoi(query_result.result[(query_result.row - 1)*query_result.column]));
			if(-1 == sqlite_database_exec(fdb, cmd_buff))
				write_syslog("detlet error, sql: ", cmd_buff);

			/*判断是否继续查询*/	
			if(query_result.row < 50)
				exit_flag = 1;

			/*释放查询资源*/
			sqlite_query_free(&query_result);
			usleep(500000);
		}	
	}
	sqlite_database_close(fdb);
}

/**
 *实时数据库内容上传
 */
void upload_real_record(int ac1_num, int ac3_num, int dc_num)
{
	const char *table_name[] =		{"charge_energy","charge_process","bms_dynamic"};				//表名
	const char *condition_field[]=	{"NowTime","record_time","record_time"};					//判断条件字段
	const int condition_var[] =     {2,2,2};						//0 无字段 1 内容不为空 2 判断时间
	const int table_type[] = {REAL_CHARGE_ENERGY, REAL_CHAREE_PROCESS, REAL_BMS_DYNAMIC};
	
	int start_canaddr[3]={1,151,181};
	int can_num[3];
	can_num[0] = ac1_num;
	can_num[1] = ac3_num;
	can_num[2] = dc_num;


	sqlite3 *fdb =NULL;
	struct db_result_st query_result;
	int exit_flag = 0, count_num;
	int k=0, i=0, m=0;
	char cmd_buff[512];
	char time_buff[30];

	time_t s;
	struct tm *old_time;
	s = time((time_t*)NULL);
	s -= 259200;			//减三天的时间
	old_time = gmtime(&s); 
	snprintf(time_buff, sizeof(time_buff), "%04d-%02d-%02d %02d:%02d:%02d", 
			old_time->tm_year+1900, old_time->tm_mon+1, old_time->tm_mday,  old_time->tm_hour, old_time->tm_min, old_time->tm_sec);

	sqlite_database_open(&fdb, REAL_RECORD_DB_NAME);
	for(k =0; k<REAL_COUNT; k++)			//表的种类
	{
		for(i=0 ;i<3; i++)			//终端的类型
		{
			for(m=0; m<can_num[i]; m++)			//每种类型终端的数量
			{
				exit_flag = 0;
				count_num = 0;

				while(!exit_flag)
				{
					int ret = 0;
					int compress_len = 0;
					unsigned char *compress_buff = NULL;

					if( k == 2 && (i == 0 || i == 1) )		//过滤交流动态bms数据
					{
						exit_flag = 1;
						continue;
					}

					/*单张表超过了最大上传条数*/
					if(count_num > args.max_count)
					{
						exit_flag = 1;
						continue;
					}

					/*查询*/
					if(condition_var[k] == 0)
						snprintf(cmd_buff,sizeof(cmd_buff),"select * from %s_%d_table order by id limit 50", table_name[k],start_canaddr[i]+m);
					else if(condition_var[k] == 1)
						snprintf(cmd_buff,sizeof(cmd_buff),"select * from %s_%d_table where %s!=\"\" order by id limit 50",table_name[k], start_canaddr[i]+m, condition_field[k]);
					else if(condition_var[k] == 2)
						snprintf(cmd_buff,sizeof(cmd_buff),"select * from %s_%d_table where %s!=\"\" and %s <\"%s\" order by id limit 50",table_name[k], start_canaddr[i]+m, condition_field[k], condition_field[k], time_buff);

					ret = sqlite_database_query(fdb, cmd_buff, &query_result);
					if(ret ==-1 || query_result.row == 0)
					{
						char str_buff[200];
						exit_flag = 1;
						snprintf(str_buff, sizeof(str_buff),"sql=%s ret=%d row=%d", cmd_buff, ret, query_result.row);
						write_syslog("real record, query faild or empty: ", str_buff);
						continue;
					}
					printf("upload table name: %s_%d_table\n", table_name[k], start_canaddr[i]+m);	
					//	printf("mytable   %s_%d\n", table_name[k], start_canaddr[i]+m);
					count_num += query_result.row;

					/*压缩上传*/
					struct json_object *object=json_pack_wohle(DB_REAL_RECORD, table_type[k], &query_result);
					char *value = get_json_point(object);
					int value_len = strlen(value);

					ret = -1;
					compress_buff = malloc(value_len);
					if(compress_buff)
						ret = def((unsigned char *)value, value_len, compress_buff, &compress_len);
					//free_json_point(object);

					if(ret == 0)
					{
						snprintf(cmd_buff, sizeof(cmd_buff), "http://%s/cscu-bin", args.host_port);
						ret = send_http_request(cmd_buff, (char *)compress_buff, compress_len, "upload", &http_rcv_buff);
						if(-1 == deal_recv_data(&http_rcv_buff, value))
						{
							char str_buff[100];
							snprintf(str_buff, sizeof(str_buff), "upload table name: %s_%d_table\n", table_name[k], start_canaddr[i]+m);	
							write_syslog("upload_faild, table_name: ", str_buff);
						}
					}
					free(compress_buff);
					free_json_point(object);

					/*删除已经查询*/
					snprintf(cmd_buff, sizeof(cmd_buff), "delete from %s_%d_table where id >=%d and id <=%d",
							table_name[k], start_canaddr[i]+m, atoi(query_result.result[0]),  atoi(query_result.result[(query_result.row - 1)*query_result.column]));                                                             
					if (-1 == sqlite_database_exec(fdb, cmd_buff))
						write_syslog("delete error, sql=", cmd_buff);

					/*判断是否继续查询*/	
					if(query_result.row < 50)
						exit_flag = 1;

					/*释放查询资源*/
					sqlite_query_free(&query_result);
					usleep(500000);
				}
			}
		}
	}
	sqlite_database_close(fdb);
}

