#ifndef MAINCHARGE_H
#define MAINCHARGE_H

#include <QWidget>
#include <QMap>
#include <QTimer>

#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"
#include "InfoData.h"
#include "ChargeTerm.h"

//#define		TEUI_GROUP

namespace Ui {
class MainCharge;
}

class MainCharge : public QWidget, public SwapBase
{
    Q_OBJECT
    
public:
    explicit MainCharge(QWidget *parent = 0, CBus *bus = 0, ProtocolBase *protocol = 0);
    ~MainCharge();
    void receiveFromBus(InfoMap, InfoAddrType);
    void queryConfig();
    void queryConfigSpecialFeatureSet();//merge by yanwei 20171011
#if 0
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
#endif
signals:
   void sigFromBus(InfoMap, InfoAddrType);
   void sigChargerClicked(unsigned char, int);

private slots:
    void slotBusToOwn(InfoMap, InfoAddrType);

	void on_pushButtonFrontPage_clicked();
	void on_pushButtonNextPage_clicked();

	void slotQueryTerminal();
    void slotChargerClicked(unsigned char canAddr, int chargeStatus);
    //void slotTimerLock();

private:
    Ui::MainCharge *ui;

    QTimer timerQuery;
    QTimer *lockTimer;
    CBus *bus;
    ProtocolBase *protocol;

    //int pressX;
    //int pressY;
    //bool timerLockFlag;

    int currentPage;
    int totalNum;
    int totalPage;
    int leftNum;
    int groupsingleflag;

    QMap<int, unsigned char> mapCanAddr;            //个数与can地址的对应关系;
    QMap<unsigned char, ChargeTerm *> mapChargerm; //can地址和终端插件对应关系
    QMap<unsigned char, ChargeTerm *> mapTotalCharge; //can地址和终端插件对应关系
    QList<unsigned char> queryCanList;              //定时查询状态数据

    int currentPosition;

    void currentPageInit(int page);
    void chargeTermGroup();
    void chargeTermSingle();
    void readfile();
    stSpecialFunc special_set;  //nihai add
};
#endif // MAINCHARGE_H
