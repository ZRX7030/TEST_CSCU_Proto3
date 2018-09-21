#include "CSCUApp.h"
#include <signal.h>

void sigExit(int sig)
{
	CSCUApp::onExit(sig);
}

int main(int argc, char *argv[])
{
    CSCUApp a(argc, argv);

	signal(SIGTERM, sigExit);

	return a.exec();
}
