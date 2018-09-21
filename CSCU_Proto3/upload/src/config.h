#ifndef _CONFIG_H_
#define _CONFIG_H_

struct args_config 
{
	char host_port[50];			//http连接地址
	char station_name[50];                            
    int ac1_num;                          
    int ac3_num;                          
    int dc_num;
	int max_count;			//每一个表上传的最大条数
};

extern struct args_config args;

int config_parse_args(int argc, char *argv[]);

#endif /* config.h */
