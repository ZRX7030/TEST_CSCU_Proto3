#include <QUuid>
#include"ScanCode/DealScanCode.h"

cDealScanCode::cDealScanCode()
{
}

cDealScanCode::~cDealScanCode()
{
}

void cDealScanCode::ProcDealScanCode(char * qchar)
{
    struct json_object *rpc_tmp;
    rpc_tmp = json_tokener_parse((const char *)qchar);
    jrpc_parse_params(rpc_tmp);
    json_object_put(rpc_tmp);
}

int cDealScanCode::jrpc_parse_params(struct json_object *obj)
{
    InfoMap mapInfo;
    InfoAddrType InfoType;
    ScanCode_frame strFrame;
    struct json_object *arry_obj = NULL;

    memset((char *)&strFrame, 0x00, sizeof(strFrame));
    /*参数解析*/
    if(!json_object_is_type(obj, json_type_object))
        return 0;

    int count = json_object_array_length(obj);
    if(!count)
        return 0;

    /*读取数据,判断class、action的值*/
    json_object_object_get_ex(obj, "timestamp", &arry_obj);
    strFrame.timestamp = json_object_get_string(arry_obj);
    json_object_object_get_ex(obj, "userID", &arry_obj);
    strFrame.userID = json_object_get_string(arry_obj);
    json_object_object_get_ex(obj, "customerID", &arry_obj);
    strFrame.customerID = json_object_get_string(arry_obj);
    json_object_object_get_ex(obj, "balance", &arry_obj);
    strFrame.balance = json_object_get_double(arry_obj);

    QString str_customerID_tmp = QString(QByteArray(strFrame.customerID));
    str_customerID_tmp.remove('-');
    QString str_customerID = str_customerID_tmp.toLower();

    int ret = timeout(strFrame.timestamp);
//    int ret = 1;
    if(ret == 1)
    {
        mapInfo.insert(Addr_CanID_Comm, QByteArray(1, (char)ucCanID));
        mapInfo.insert(Addr_ScanCode_customerID, str_customerID.toAscii());

        //     mapInfo.insert(Addr_CardApplyCharge_Result, QByteArray(1, 0xff));
        //m_remain转QByteArray
        QString str_m_remain = QString("%1").arg(strFrame.balance);
        QByteArray qba_m_remain = str_m_remain.toLatin1();
        mapInfo.insert(Addr_CardAccountList, qba_m_remain);
        emit sendToScanCode(mapInfo,AddrType_CenterReadCard);  //将customerID 发到Bus
    }
    else
    {
        InfoType = AddrType_OutApplyStartChargeResult_ToScreen;
        mapInfo.insert(Addr_CanID_Comm, QByteArray(1, (char)ucCanID));
        mapInfo.insert(Addr_CardApplyCharge_Result, QByteArray(1, (char)0xf4));
        mapInfo.insert(Addr_ScanCode_customerID, str_customerID.toAscii());
        emit sendToScanCode(mapInfo,InfoType);  //将二维码超时结果 发到Bus
    }
    if(str_customerID.length() < 16)
    {
        InfoType = AddrType_OutApplyStartChargeResult_ToScreen;
        mapInfo.insert(Addr_CanID_Comm, QByteArray(1, (char)ucCanID));
        mapInfo.insert(Addr_CardApplyCharge_Result, QByteArray(1, (char)0x04));
        mapInfo.insert(Addr_ScanCode_customerID, str_customerID.toAscii());
        emit sendToScanCode(mapInfo,InfoType);  //客户卡id少于16个字节  发到Bus
    }

    return 0;
}

//接收控制中心数据
void cDealScanCode::slotFromBus(InfoMap mapInfo, InfoAddrType type)
{
    QByteArray arData;

    //刷卡申请帐户信息
    switch(type){
    case AddrType_ChargeServicApplyAccountInfo:
        arData.fill(0xFF, 11);
        if(mapInfo.contains(Addr_CardAccount)){
            arData[0] = mapInfo[Addr_CardAccount].length();
            if(arData.at(0) > 0 && arData.at(0) <= 10)
                arData.replace(1, arData.at(0), mapInfo[Addr_CardAccount]);
        }
        if(mapInfo.contains(Addr_CanID_Comm))
        //            list.append(mapInfo.find(Addr_CanID_Comm));
        //        list.append(mapInfo.insert(Addr_CardAccount, arData));
        if(mapInfo.contains(Addr_ScanCode_customerID))
        //            list.append(mapInfo.find(Addr_CardAccountType));
//        cCmd = CMD_CARD_ACCOUNT;
//        sReason = REASON_BURST;
        break;
    default:
        break;

    }
}

int cDealScanCode::timeout(const char * timestamp)
{
    QString str_ScanCode;

    str_ScanCode = QString(QByteArray(timestamp));

    //二维码生成时间
    QDateTime dt = QDateTime::fromString(str_ScanCode,"yyyyMMddHHmmss");
    short year=dt.toString("yyyy").toInt();
    short month = dt.toString("MM").toInt();
    short day = dt.toString("dd").toInt();
    short hh = dt.toString("hh").toInt();
    short mm=dt.toString("mm").toInt();
    short ss = dt.toString("ss").toInt();

    //当前时间
    QDateTime dt_now = QDateTime::currentDateTime();
    short year_now = dt_now.toString("yyyy").toInt();
    short month_now = dt_now.toString("MM").toInt();
    short day_now = dt_now.toString("dd").toInt();
    short mm_now = dt_now.toString("mm").toInt();
    short hh_now = dt_now.toString("hh").toInt();
    short ss_now = dt_now.toString("ss").toInt();

    if(year == year_now && month == month_now && day == day_now)
    {
        int time = hh * time_count *time_count + mm * time_count + ss;
        int time_now = hh_now * time_count *time_count + mm_now * time_count + ss_now;

        if((time_now - time) < time_count && (time_now - time) >= 0)
            return 1;
        else
            return  -1;
    }

    if((year == (year_now - 1)) || (month == (month_now - 1)) || (day ==( day_now - 1)) ||(month == 12 && month_now == 1) || (day == 30 || day == 31 && day_now == 1))
    {
        int time = hh * time_count *time_count + mm * time_count + ss;
        int time_now = 24 * time_count *time_count + mm_now * time_count + ss_now;
        if((time_now - time) < time_count && (time_now - time) >= 0)
            return 1;
        else
            return  -1;
    }

    return 0;
}
