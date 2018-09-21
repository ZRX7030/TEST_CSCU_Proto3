#ifndef DEALSCANCODE_H
#define DEALSCANCODE_H

#include <QObject>
#include <QDebug>
#include "GeneralData/GeneralData.h"
#include "Infotag/CSCUBus.h"
#include "DevCache/DevCache.h"
#include "ParamSet/ParamSet.h"
#include "Infotag/CSCUBus.h"
#include "Log/Log.h"
#include <fcntl.h>
#include <termios.h>
#include <json-c/json.h>
#include <json-c/json_object.h>

#define time_count  60


//扫描二维码所得数据
struct ScanCode_frame
{
    const char *timestamp;
    const char *userID;
    const char *customerID;
    float balance;
};

class cDealScanCode :public QObject
{
    Q_OBJECT
   public:

        cDealScanCode();
       ~cDealScanCode();

    unsigned char ucCanID;

private:
    int jrpc_parse_params(struct json_object *obj);
    int timeout(const char *);

private slots:
    void ProcDealScanCode(char *);
    //接收控制中心数据
    void slotFromBus(InfoMap mapInfo, InfoAddrType type);
signals:
    void sendToScanCode(InfoMap, InfoAddrType);
};

#endif // DEALSCANCODE_H
