#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"

struct args_config args = {
	//.host_port="D-BJ-3rdCOM.chinacloudapp.cn:8080",
	.host_port="",
	.station_name="",                          
    .ac1_num=1,                          
    .ac3_num=1,                          
    .dc_num=1,
	.max_count=800
};

void usage(void)
{
	printf("Usage: upload [OPTION]...\n");
	printf("upload db info data to server\n");
	printf("    -H host and port name         \n");
	printf("    -a station name               \n");
    printf("    -s single phase ac number     \n");
    printf("    -t three phase ac number      \n");
    printf("    -d dc number				  \n");
    printf("    -m max count each table       \n");
    printf("    -h help                       \n");
}

int config_parse_args(int argc, char *argv[])
{
	int opt;

	while((opt = getopt(argc, argv, "H:a:s:t:d:m:h")) != -1) {
		switch (opt) {
		case 'H':
            snprintf(args.host_port, sizeof(args.host_port), "%s", optarg);
			break;
        case 'a':
            snprintf(args.station_name, sizeof(args.station_name), "%s", optarg);
            break;
        case 's':
            args.ac1_num = atoi(optarg);
            break;
        case 't':
            args.ac3_num = atoi(optarg);
            break;
		case 'd':
            args.dc_num = atoi(optarg);
            break;
		case 'm':
            args.max_count = atoi(optarg);
            break;
        case 'h':
            usage();
            exit(0);
            break;
		default:
            usage();
            exit(0);
            break;
		}	
	}
	
	if(strlen(args.host_port) == 0)
	{
		printf("http host is null.\n");
		return 1;
	}

    return 0;
}

