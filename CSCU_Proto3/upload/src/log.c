#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "log.h"

int write_syslog(char *head, char *info)
{
	openlog("upload", LOG_PID|LOG_CONS, LOG_USER);
	syslog(LOG_INFO, "%s %s", head, info);
	closelog();
	return 1;
}

