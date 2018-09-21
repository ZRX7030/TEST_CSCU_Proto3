#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "upload.h"
#include "config.h"
#include "signals.h"


int main(int argc, char *argv[])
{
	printf("upload start................ver 1.0\n");
	//signals_init();
	if(config_parse_args(argc, argv))
		exit(0);
#if 0
	strcpy(args.host_port, "D-BJ-3rdCOM.chinacloudapp.cn:9001");
	strcpy(args.station_name, "370212032110000");                          
    args.ac1_num=27;                          
    args.ac3_num=0;                          
    args.dc_num=3;
	args.max_count=1000;
#endif
	
	upload_init();
	upload_process_record();
	upload_real_record(args.ac1_num, args.ac3_num , args.dc_num);

	upload_deinit();
	printf("upload end................\n");
	return 0;
}
