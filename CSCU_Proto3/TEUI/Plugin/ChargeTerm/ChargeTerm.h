#ifndef CHARGETERM_H
#define CHARGETERM_H

#include <QWidget>
#include <QTimer>

/*链接状态图标索引*/
enum
{
    LABEL_INDEX_SLEEP = 0,
    LABEL_INDEX_LINK = 1,
    LABEL_INDEX_FAULT = 2,
    LABEL_INDEX_CHARGEING1 = 3,
    LABEL_INDEX_CHARGEING2 = 4,
    LABEL_INDEX_CHARGEING3 = 5,
    LABEL_INDEX_CHARGEING4 = 6,
    LABEL_INDEX_CHARGEING5 = 7,
    LABEL_INDEX_PROCESSBAR = 8,
    LABEL_INDEX_FINSH = 9,
    LABEL_INDEX_QUEUE = 10,
    LABEL_INDEX_OFFLINE = 11
};
/*充电状态定义*/
enum
{
    CHARGE_STATUS_STARTING = 0,//0启动中 A
    CHARGE_STATUS_GUN_STANDBY,//1待机-枪已连接，等待充电 B
    CHARGE_STATUS_LIMIT,//2充电-限制 A
    CHARGE_STATUS_PAUSH,//3充电-暂停 A
    CHARGE_STATUS_CHARGING,//4充电-充电中 A
    CHARGE_STATUS_SWITCH,//5待机-切换中 B
    CHARGE_STATUS_FREE,//6待机-空闲 B
    CHARGE_STATUS_DISCONNECT,//7离线-未通信 C
    CHARGE_STATUS_FINISH,//8待机-已完成 A
    CHARGE_STATUS_FAULT,//9故障 B
    CHARGE_STATUS_DISCHARGING,//10放电 A
    CHARGE_STATUS_SLAVEGUN,//11副枪
    CHARGE_STATUS_COUPLE_ERR,//12配对错误
    CHARGE_STATUS_FULL=13,//13待机-车已充满 A
    CHARGE_STATUS_FINISH1,//14待机-手动断开 B
    CHARGE_STATUS_QUEUE1,//15充电-排队1 B
    CHARGE_STATUS_QUEUE2,//16充电-排队2 B
    CHARGE_STATUS_QUEUE3,//17充电-排队3 B
    CHARGE_STATUS_QUEUE4,//18充电-排队3 B
    CHARGE_STATUS_QUEUE5,//19充电-排队5 B
    CHARGE_STATUS_QUEUE6,//20充电-排队6 B
    CHARGE_STATUS_QUEUE7//21充电-排队7  B
};

typedef struct __ChargeTermData
{
    unsigned char canAddr;              //can地址
    unsigned char soc;                  //当前soc
    int status;           //当前连接状态
    int linkStatus;
    char name[20];                      //终端名字
}stChargeTermData;

namespace Ui {
class ChargeTerm;
}

class ChargeTerm : public QWidget
{
    Q_OBJECT

public:
    explicit ChargeTerm(QWidget *parent = 0);
    ~ChargeTerm();
    void singleChargeTermSet();
    void updateChargeStatus(stChargeTermData chargeData);
signals:
    void signalChargerClicked(unsigned char canAddr, int Status);
protected:
    void mousePressEvent(QMouseEvent *event);
private:
    Ui::ChargeTerm *ui;
    stChargeTermData chargeStatus;

    QTimer *flashTimer;
    int timerCount;
private slots:
    void flashOverTimer();
};

#endif // CHARGETERM_H
