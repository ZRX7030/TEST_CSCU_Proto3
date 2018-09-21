#include "smartcharge.h"
#include <QDebug>

#define STOP_CHARGE_CMD_RSN_VIN_APPLY 110//临时写为110，待正式定义后修改

SmartCharge::SmartCharge(stAllTPFVConfig * TPFVconfig, DevCache*cache,ParamSet *param,Log *log,DBOperate* DBOperate)
{
	_strLogName = "smartcharge";
    actionTimer =  new QTimer;
    actionTimer->setInterval(60000);//正式用
//    actionTimer->setInterval(5000);//测试用
    connect(actionTimer,SIGNAL(timeout()),this,SLOT(ProcTimeOut()));

    memset(TimeFlag, 0x0,INTERVAL_COUNT);

    pLog_smartCharge = log;

    pDevCache = cache;
    config = TPFVconfig;
    pParamSet = param;
    pDBOperate = DBOperate;

    //车辆优先调度

     unParamConfig *paramConfig = new unParamConfig;
    pParamSet->querySetting(paramConfig,PARAM_SMARTCAR);
    stSmartCarConfig config_smartCar = paramConfig->smartCarConfig;
    vechiclePriority = NULL;
    if(config_smartCar.sSmartCar_Enable)//车辆优先级调度开启
    {
        vechiclePriority = new vechiclepriority(pDevCache,pLog_smartCharge,pDBOperate,pParamSet);
        connect(this,SIGNAL(sig_timeout()),vechiclePriority,SLOT(DistributionModuleNum()));
        connect(this,SIGNAL(sig_clearcurruntnum(unsigned int)),vechiclePriority,SLOT(ClearDistributionModuleNum(unsigned int)));

    }
    delete paramConfig;
}

SmartCharge::~SmartCharge()
{
    if(vechiclePriority)
    {
        delete vechiclePriority;
        vechiclePriority=NULL;
    }
}
/*比较本时段内设置胡SOC和充电电流是否符合设置条件，并且调整到设置条件内
*/
void SmartCharge::SmartChargeAction(TerminalStatus &status, int index,unsigned char canID,CHARGE_STEP &stChargeStep)
{
     InfoMap qInfoMap;
     QDateTime dt = QDateTime::currentDateTime();
     minute=dt.toString("mm").toInt();
     hour = dt.toString("hh").toInt();

     int time,startTime;
     time = hour*100 + minute;

     startTime = config->tpfvConfig.at(index).start_hour*100 + config->tpfvConfig.at(index).start_minute;

    if(status.stFrameBmsInfo.batery_SOC <= config->tpfvConfig.at(index).limit_soc)
    {//“当前SOC值”<= “目标SOC值”
        if(time == startTime)//时段的起点，全速充电

        {

            pLog_smartCharge->getLogPoint(_strLogName)->info("当前SOC值”<= “目标SOC值，起始点，全速充电");

            qInfoMap.insert(Addr_CanID_Comm, QByteArray(1,canID));
            float cur = 400;
            qInfoMap.insert(Addr_AdjustCurrent_Adj,QByteArray((char*)&cur, 4));      //调整充电电流 400=全速充
            emit sig_adjChargeCur(qInfoMap,AddrType_LimitChargeCurrent);
            emit sig_clearcurruntnum(canID);  //全速充电之后清除原来分配的模块数，以便重新进入车辆调度后重新分配
        }
        else
        {

            //车辆优先级调度
            unParamConfig para;
            unsigned char sendlimitcurruntflag=0;
            float sendlimitcurrunt=0;


            pParamSet->querySetting(&para,PARAM_SMARTCAR);
            stSmartCarConfig config_smartCar = para.smartCarConfig;
            if(config_smartCar.sSmartCar_Enable)//车辆优先级调度充电开启
            {
                vechiclePriority->SendLimitCurrunt(canID, sendlimitcurruntflag, sendlimitcurrunt);
                if(sendlimitcurruntflag == 1)  //需要下发限制电流
                {
                     pLog_smartCharge->getLogPoint(_strLogName)->info("当前SOC值”<=“目标SOC值，车辆调度调整充电电流");
                     pLog_smartCharge->getLogPoint(_strLogName)->info("Limit current ==   "+QString::number(sendlimitcurrunt));
                     pLog_smartCharge->getLogPoint(_strLogName)->info("charge current ==   "+QString::number(abs(status.stFrameRemoteMeSurement1.current_of_dc)));
                    // if(config->tpfvConfig.at(index).limit_current < abs(status.stFrameRemoteMeSurement1.current_of_dc))
                         qInfoMap.insert(Addr_CanID_Comm, QByteArray(1,canID));
                        // float limitCur = (float)(config->tpfvConfig.at(index).limit_current);
                         char *data =(char*) &sendlimitcurrunt;
                         QByteArray termArray;
                         termArray.append(data,4);
                         qInfoMap.insert(Addr_AdjustCurrent_Adj,termArray);
                         emit sig_adjChargeCur(qInfoMap,AddrType_LimitChargeCurrent);
                    // }
                }
            }
        }
    }
    else//“当前SOC值”>“目标SOC值”
    {
        if(config->tpfvConfig.at(index).limit_current == 0)//限制电流为０，停止充电
        {
pLog_smartCharge->getLogPoint(_strLogName)->info("当前SOC值”>“目标SOC值，限制电流为０，停止充电");

            qInfoMap.insert(Addr_CanID_Comm, QByteArray(1,canID));
            QByteArray tem;
            tem.append((char *)stChargeStep.sVIN,LENGTH_VIN_NO);
            //m_VINlist.append(tem);
            //qInfoMap.insert(Addr_BatteryVIN_BMS, QByteArray::fromRawData(stChargeStep.sVIN, LENGTH_VIN_NO));//VIN号
            qInfoMap.insert(Addr_BatteryVIN_BMS, tem);//VIN号
            pLog_smartCharge->getLogPoint(_strLogName)->info("vin ==   "+QString(qInfoMap[Addr_BatteryVIN_BMS]));
            emit sig_stopCharge(qInfoMap,AddrType_InVinApplyStopCharge);
        }
        else
        {//限制电流不为０，充电电流大于限制电流时，限制电流
            pLog_smartCharge->getLogPoint(_strLogName)->info("当前SOC值”>“目标SOC值，限制电流不为０，调整充电电流");
            pLog_smartCharge->getLogPoint(_strLogName)->info("Limit current ==   "+QString::number(config->tpfvConfig.at(index).limit_current));
            pLog_smartCharge->getLogPoint(_strLogName)->info("charge current ==   "+QString::number(abs(status.stFrameRemoteMeSurement1.current_of_dc)));
            if(config->tpfvConfig.at(index).limit_current < abs(status.stFrameRemoteMeSurement1.current_of_dc))
            {
                qInfoMap.insert(Addr_CanID_Comm, QByteArray(1,canID));
                float limitCur = (float)(config->tpfvConfig.at(index).limit_current);
                char *data =(char*) &limitCur;
                QByteArray termArray;
                termArray.append(data,4);
                qInfoMap.insert(Addr_AdjustCurrent_Adj,termArray);
                emit sig_adjChargeCur(qInfoMap,AddrType_LimitChargeCurrent);
            }
        }
    }
}

///
/// \brief SmartCharge::getStopTimeSection
/// \param dt
/// \return
///计算停止时间属于哪个时间段
int SmartCharge::getStopTimeSection(QDateTime dt)
{
    int time,startTime,endTime;
    for(int i=0;i<config->tpfvConfig.count();i++)
    {
        minute=dt.toString("mm").toInt();
        hour = dt.toString("hh").toInt();

        time = hour*100 + minute;
        startTime = config->tpfvConfig.at(i).start_hour*100 + config->tpfvConfig.at(i).start_minute;
        endTime = config->tpfvConfig.at(i).stop_hour*100 + config->tpfvConfig.at(i).stop_minute;
        if(startTime < endTime)
        {
            if(time>=startTime && time<endTime)
            {
                return i;
            }
        }
        else
        {
//            if(time>=startTime && time<endTime+2400)
            if((time>=startTime && time<=2400) ||
                                (time<endTime))
            {
                return i;
            }
        }
    }
    return 0xFF;
}

///
/// \brief SmartCharge::identifyTimeSection
/// \param config
/// \return
///判断当前时间是否在某个时间段
bool SmartCharge::identifyTimeSection(stTPFVConfig config)
{
    QDateTime dt = QDateTime::currentDateTime();
    minute=dt.toString("mm").toInt();
    hour = dt.toString("hh").toInt();

    int time,startTime,endTime;
    time = hour*100 + minute;

    if(config.start_hour == 24)
    {
        config.start_hour = 0;
    }
    startTime = config.start_hour*100 + config.start_minute;
    endTime = config.stop_hour*100 + config.stop_minute;
    if(startTime < endTime)
    {
        if(time>=startTime && time<endTime)
        {
            return true;
        }
    }
    else
    {
        if((time>=startTime && time<=2400) ||
                (time<endTime))
        {
            return true;
        }
    }

    return false;
}

void SmartCharge::ProcTimeOut()
{
    QDateTime dt = QDateTime::currentDateTime();
//     pLog_smartCharge->getLogPoint(_strLogName)->info("当前时间：　"+dt.toString());
    for(int i=0;i<config->tpfvConfig.count();i++)
    {
        if(!identifyTimeSection(config->tpfvConfig.at(i)))//当前时间是否在本时段
        {
            continue;
        }
        pLog_smartCharge->getLogPoint(_strLogName)->info("当前所在时段：　"+QString::number(i));
        unParamConfig para;

        pParamSet->querySetting(&para,PARAM_SMARTCAR);
        stSmartCarConfig config_smartCar = para.smartCarConfig;
        if(config_smartCar.sSmartCar_Enable)//车辆优先级调度充电开启
        {
            emit sig_timeout();
        }

        pParamSet->querySetting(&para, PARAM_CSCU_SYS);
        for(unsigned char canID = ID_MinDCCanID;canID<ID_MinDCCanID+para.cscuSysConfig.directCurrent;canID++)
        {
            pLog_smartCharge->getLogPoint(_strLogName)->info("枪编号＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝　"+QString::number(canID));

            CHARGE_STEP stChargeStep;//存放从缓存中读出的状态机
            TerminalStatus status;

            pDevCache->QueryTerminalStatus(canID,status);
            pLog_smartCharge->getLogPoint(_strLogName)->info("终端状态：　"+QString::number((int)status.cStatus));

            if(!pDevCache->QueryChargeStep(canID, stChargeStep))
            {
                continue;
            }



            if(status.cStatus == CHARGE_STATUS_CHARGING)//充电中
            {
                //vin申请
                pLog_smartCharge->getLogPoint(_strLogName)->info("充电开始原因：　"+QString::number((int)stChargeStep.ucStartReason));
                if(stChargeStep.ucStartReason == START_CHARGE_CMD_RSN_SMART_CHARGE_VIN ||
                    stChargeStep.ucStartReason == START_CHARGE_CMD_RSN_DEV_VIN_REMOTE  ||
                    stChargeStep.ucStartReason == START_CHARGE_CMD_RSN_EMERGENCY_VIN ||
                    stChargeStep.ucStartReason == START_CHARGE_CMD_RSN_COUPLE_CHARGE_VIN)   //多枪充电启动方式也加入错峰调度
                {
                    SmartChargeAction(status, i,canID,stChargeStep);     //判断条件是否符合并且调整到符合条件
                }
                else
                {
                    pLog_smartCharge->getLogPoint(_strLogName)->info("充电不是VIN申请开始的，被干预!!!!!!!!!!!");
                }
            }
            else if(status.cStatus == CHARGE_STATUS_FINISH)//已完成
            {
                if(!pDevCache->QueryChargeStep(canID, stChargeStep))
                {
                    continue;
                }
                pLog_smartCharge->getLogPoint(_strLogName)->info("充电结束原因：　"+QString::number((int)stChargeStep.ucStopReasonCSCU));

                if(stChargeStep.ucStopReasonCSCU == 110)//错峰充电申请结束
                {
                    QDateTime dt;
                    CharArray2QDateTime(stChargeStep.sEndTime,dt);

                        pLog_smartCharge->getLogPoint(_strLogName)->info("停止充电时间：　"+dt.toString());
                        pLog_smartCharge->getLogPoint(_strLogName)->info("所在时段：　"+QString::number(getStopTimeSection(dt)));

                    if(getStopTimeSection(dt) != i)//不是在本时段停止的充电，在本时段重新启动
                    {
                        pLog_smartCharge->getLogPoint(_strLogName)->info("不是在本时段停止的充电，在本时段重新启动");
                        InfoMap qInfoMap;
                        qInfoMap.insert(Addr_CanID_Comm, QByteArray(1,canID));
                        QByteArray tem;
                        tem.append((char *)stChargeStep.sVIN,LENGTH_VIN_NO);
                        //qInfoMap.insert(Addr_BatteryVIN_BMS,QByteArray::fromRawData(stChargeStep.sVIN, LENGTH_VIN_NO));
                        qInfoMap.insert(Addr_BatteryVIN_BMS,tem);
                        qInfoMap.insert(Addr_VINApplyStartChargeType,QByteArray(1,1));//VIN申请开始充电类型,默认1,充满为止
                        emit sig_adjChargeCur(qInfoMap,AddrType_InVinApplyStartCharge);
                    }
                }
                else
                {
                    pLog_smartCharge->getLogPoint(_strLogName)->info("终端不是VIN申请结束的，记录被干预!!!!!!!!!!!!!!!!!!!");
                }
            }

//            if(status.cStatus != CHARGE_STATUS_FINISH && status.cStatus != CHARGE_STATUS_CHARGING)
//            {
//                pLog_smartCharge->getLogPoint(_strLogName)->info("终端　"+QString::number(canID)+" 不在充电中或已完成状态，不使用策略");
//            }

        }

        break;
    }
}

///
/// \brief SmartCharge::slot_paraChange
/// \param TPFVconfig
///更新错峰充电时段配置
void SmartCharge::slot_paraChange(stAllTPFVConfig * TPFVconfig)
{
    configLock.lock();
   config = TPFVconfig;
   configLock.unlock();
}

///
/// \brief SmartCharge::slot_smartChargeSwitch
/// \param flag
///根据设置启停定时器
void SmartCharge::slot_smartChargeSwitch(bool flag)
{

    if(actionTimer->isActive() && !flag)
    {
        actionTimer->stop();
    }
    else if(!actionTimer->isActive() && flag)
    {
        actionTimer->start();
    }
}
