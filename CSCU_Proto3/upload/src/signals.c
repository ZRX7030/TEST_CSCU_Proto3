#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>


volatile int sig_quit = 0;

static void signals_stop_program(int signum)
{
    printf("catch stop signal\n");

    sig_quit = 1;
}

int signals_init(void)
{
	struct sigaction sigact;	

	memset(&sigact, 0, sizeof(struct sigaction));

	sigact.sa_handler = signals_stop_program;

	if (sigaction(SIGINT, &sigact, NULL) == -1) {
		fprintf(stderr, "signal int error: %s\n", strerror(errno));
		return -1;
	}

	return 0;
}
