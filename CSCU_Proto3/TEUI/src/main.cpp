#include <QApplication>
#include <QTextCodec>
#include <QDebug>
#include <QWidget>


#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <errno.h>  
#include <QWSServer>

#include "TeuiMainWindow.h"
#include "Bus.h"
#include "Sockets.h"
#include "TeuiProtocol.h"
#include "Common.h"


struct stConfigParam
{
	char host[30];			//server host
};

struct stConfigParam configParam;
stTeuiParam  teuiParam;

void paramParse(int argc, char *argv[])
{
	for(int i = 1; i< argc; i++)
	{
		if(strcmp(argv[i], "-host") == 0)
		{
			struct in_addr addr;  

			if(inet_pton(AF_INET, argv[i+1], &addr) > 0) 
				snprintf(configParam.host, sizeof(configParam.host), "%s", argv[i+1]);
            //qDebug () << "parse host is :" << QString(configParam.host);
		}
	}
}
/**
 *信号捕获，捕获到 用户长时间无动作
 */
void SignalsProgram(int signum)
{
    //printf("catch stop signal................\n");

	teuiParam.capSignal = 1;
}

int SignalsInit(void)
{
	struct sigaction sigact;        

	memset(&sigact, 0, sizeof(struct sigaction));

	sigact.sa_handler = SignalsProgram;

	if (sigaction(10, &sigact, NULL) == -1) {                                                                                    
		fprintf(stderr, "signal int error: %s\n", strerror(errno));
		return -1; 
	}   

	return 0;
}

int main(int argc, char *argv[])
{
	
	snprintf(configParam.host, sizeof(configParam.host), "%s", "127.0.0.1");
	paramParse(argc, argv);

	/*参数初始化*/
	teuiParam.showTermWin = true;
	teuiParam.capSignal = 0;

    QApplication a(argc, argv);
    qRegisterMetaType<QVariant>("QVariant");
    qRegisterMetaType<InfoMap>("InfoMap");
    qRegisterMetaType<InfoAddrType>("InfoAddrType");

    QWSServer::setCursorVisible(false);
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

	SignalsInit();

    CBus *bus = new CBus();
    qDebug() << "main bus" << bus;
    TeuiProtocol *teuiProtocol = new TeuiProtocol(1, bus);
    Sockets *teuiSocket = new Sockets(configParam.host, 7000);
    //Sockets *teuiSocket = new Sockets("192.168.1.100", 7000);

    TeuiMainWindow w(NULL, bus, teuiProtocol, &teuiParam);

    QObject::connect(teuiSocket, SIGNAL(receiveDatas(QVariant)), teuiProtocol, SLOT(receivePackageDatas(QVariant)));
    QObject::connect(teuiProtocol, SIGNAL(sendPackageDatas(unsigned char *, int)), teuiSocket, SLOT(sendDatas(unsigned char *, int)));
    QObject::connect(teuiSocket, SIGNAL(socketConnected()), &w, SLOT(slotConnected()));
    QObject::connect(teuiSocket, SIGNAL(socketClosed()), &w, SLOT(slotClosed()));
    QObject::connect(&w, SIGNAL(sigReconnect()), teuiSocket, SLOT(reconnect()));

    teuiSocket->startRun();

    w.resize(800,600);
    w.show();

    a.exec();
    return 0;
}
