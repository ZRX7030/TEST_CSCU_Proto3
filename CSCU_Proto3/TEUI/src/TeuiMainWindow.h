#ifndef TEUIMAINWINDOW_H
#define TEUIMAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QTimer>
#include <QVariant>
#include <QTranslator>

#include "Maintain.h"
#include "SwapBase.h"
#include "Bus.h"
#include "ProtocolBase.h"
#include "InfoData.h"
#include "Common.h"

namespace Ui {
class TeuiMainWindow;
}

class TeuiMainWindow : public QMainWindow, public SwapBase
{
    Q_OBJECT

public:
    explicit TeuiMainWindow(QWidget *parent = 0,CBus *bus = 0, ProtocolBase *protocol = 0, void *param = 0);
    ~TeuiMainWindow();
    void receiveFromBus(InfoMap, InfoAddrType);   //获取填充的位置

signals:
   void sigFromBus(InfoMap, InfoAddrType);
   void  sigCloseWidget(void);
   void  sigReconnect(void);
   void sigClickLogo();

private slots:
    //void on_buttonConnectStatus_clicked();
    void slotBusToOwn(InfoMap, InfoAddrType);
    void on_buttonLogo_clicked();
    void slotDisplayTime();
	void slotChargeClicked(unsigned char, int);
	void slotChargeSwitch(int oldStatus, int newStatus, QVariant);
	void slotUpdateExportSwitch(int oldStatus, int newStatus, QVariant var);

public slots:
    void slotConnected(void);
    void slotClosed(void);

private:
    Ui::TeuiMainWindow *ui;

	bool maintainLock;
    
	QWidget * widget;

    QWidget *maintainWidget;
    QWidget *updateExportWidget;
    QWidget *chargeManageWidget;

    QWidget *chargeStandby;              //刷卡开始充电
    QWidget *chargeVINStart;             //VIN后6位开始充电
    QWidget *chargeFinishReport;        //充电完成报告
    QWidget *chargingReport;            //充电中报告
    QWidget *mainChargeWidget;
    QWidget *chargeFault;               //故障报告
    QWidget *flashWait;                 //flash
    QWidget *chargeSelect;
    QWidget *chargeBase;
    QWidget *printPaper;                 //打印小票
    QWidget *exportFlash;                  //导出数据升级版本
	QStackedWidget *stackWidget;

	bool needCheckReconnect;

	int heartCount;				//心跳发送计数
	bool heartRespond;			//收到心跳标志

    int clickConnect;
    int currentPage;
	unsigned char canAddr;
    unsigned char emergencyCharge;
    int termStatus;
    CBus *bus;
	ProtocolBase *protocol;
    int password;
    QTimer dispTime;

    stSpecialFunc vinparam;

    stTeuiParam *teuiParam;
    QTranslator  *translator;
	//void SignalsProgram(int);
	//int SignalsInit();
    void readfile();
    void setLanguage();
};

#endif // TEUIMAINWINDOW_H
