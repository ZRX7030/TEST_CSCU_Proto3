#include "peakshaving.h"

//#define STOP_CHARGE_CMD_RSN_VIN_APPLY 110//临时写为110，待正式定义后修改
PeakShaving::PeakShaving(stAllTPFVConfig * TPFVconfig, DevCache*cache,ParamSet *param,Log *log)
{
	_strLogName = "smartcharge";
    actionTimer =  new QTimer;
//    actionTimer->setInterval(60000);//正式用
//    actionTimer->setInterval(5000);//测试用
//    connect(actionTimer,SIGNAL(timeout()),this,SLOT(ProcTimeOut()));

    memset(TimeFlag, 0x0,INTERVAL_COUNT);

    pLog_smartCharge = log;

    pDevCache = cache;
    config = TPFVconfig;
    pParamSet = param;

     stCSCUSysConfig para;
     pParamSet->querySetting(&para, PARAM_CSCU_SYS);
    ucNumTerminal[0] = para.singlePhase;
    ucNumTerminal[1] = para.threePhase;
    ucNumTerminal[2] = para.directCurrent;
    LogOut(QString("Config Terminal Num: AC=%1, 3AC=%2, DC=%3")\
           .arg(ucNumTerminal[0]).arg(ucNumTerminal[1]).arg(ucNumTerminal[2]), 2);
}


PeakShaving::~PeakShaving()
{
    ;
}

///
/// \brief ChargeService::LogOut 日志记录
/// \param str 日志串
/// \param Level 级别
///
inline void  PeakShaving::LogOut(QString str,int Level)
{
    switch (Level) {
    case 1:
        pLog_smartCharge->getLogPoint(_strLogName)->debug(str);
        break;
    case 2:
        pLog_smartCharge->getLogPoint(_strLogName)->info(str);
        break;
    case 3:
        pLog_smartCharge->getLogPoint(_strLogName)->warn(str);
        break;
    case 4:
        pLog_smartCharge->getLogPoint(_strLogName)->error(str);
        break;
    default:
        break;
    }
}

//flag   １ :收到经济充电命令时即时处理;　已完成/插枪状态终端可以充电
//           0 :  对之前经济充电命令到后续处理; 只有插枪状态终端可以充电
//暂定limit_soc为100允许充电，为 0不允许充电
void PeakShaving::SmartChargeAction(TerminalStatus &status, int index,unsigned char canID,unsigned char flag)
{
    InfoMap qInfoMap;
    unsigned char ack=1;

    LogOut(QString("soc : %1  flag %2 ").arg(QString::number(config->tpfvConfig.at(index).limit_soc)).arg(QString::number(flag)), 2);
    if( config->tpfvConfig.at(index).limit_soc ==0)     //不允许充电
    {
        if(status.cStatus == CHARGE_STATUS_CHARGING)//充电中的要停止充电／返回充电失败
        {
            LogOut("削峰填谷申请停止充电！！！", 2);
            qInfoMap.insert(Addr_CanID_Comm, QByteArray(1,canID));
            emit sig_ChargeApply(qInfoMap,AddrType_InPeakApplyStopCharge);
        }
        else if(status.cStatus == CHARGE_STATUS_GUN_STANDBY)   //只有待机才允许到达时间集控自动启动
        {
            ack = 255;
        }
    }
    else if(config->tpfvConfig.at(index).limit_soc == 100)    //允许充电
    {
//        CHARGE_STEP stChargeStep;//存放从缓存中读出的状态机
//        if(pDevCache->QueryChargeStep(canID, stChargeStep))
//        {

        if((((status.cStatus == CHARGE_STATUS_GUN_STANDBY) || (status.cStatus == CHARGE_STATUS_PAUSH)) && (!flag) )|| \
                (flag && ((status.cStatus == CHARGE_STATUS_GUN_STANDBY) || (status.cStatus == CHARGE_STATUS_FINISH))) )//插枪
        {
            LogOut("削峰填谷申请开始充电！！！"+QString::number((int)status.cStatus), 2);
            ack = 255;

            qInfoMap.insert(Addr_CanID_Comm, QByteArray(1,canID));
            qInfoMap.insert(Addr_AckResult_Ctrl, QByteArray(1,ack));
            qInfoMap.insert(Addr_ChargeCmd_Ctrl, QByteArray(1,2));
            if(flag)
            {
                emit sig_ChargeApply(qInfoMap,AddrType_InPeakApplyStartChargeACK);   //返回开始充电的ACK
                 LogOut(QString("削峰填谷申请开始充电返回结果 %1").arg(QString::number(255)),1);
            }
            emit sig_ChargeApply(qInfoMap,AddrType_InPeakApplyStartCharge);

           return ;
        }
//        }
    }

    if(flag)
    {
        qInfoMap.clear();
        qInfoMap.insert(Addr_CanID_Comm, QByteArray(1,canID));
        qInfoMap.insert(Addr_AckResult_Ctrl, QByteArray(1,ack));
        qInfoMap.insert(Addr_ChargeCmd_Ctrl, QByteArray(1,2));
        emit sig_ChargeApply(qInfoMap,AddrType_InPeakApplyStartChargeACK);   //返回开始充电的ACK
        LogOut(QString("削峰填谷申请开始充电返回结果 %1").arg(QString::number(ack)),1);
    }
}

///
/// \brief SmartCharge::identifyTimeSection
/// \param config
/// \return
///判断当前时间是否在某个时间段
bool PeakShaving::identifyTimeSection(stTPFVConfig config)
{
    QDateTime dt = QDateTime::currentDateTime();
    minute=dt.toString("mm").toInt();
    hour = dt.toString("hh").toInt();

    int time,startTime,endTime;
    time = hour*100 + minute;

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

void PeakShaving::ProcTimeOut()
{
    QDateTime dt = QDateTime::currentDateTime();
    __u8 ucCanAddr = 0;
    __u8 ucIndexID = 0;//起始地址

    LogOut(QString("当前时间：　%1") .arg(dt.toString()),2);
    for(int i=0;i<config->tpfvConfig.count();i++)
    {
        if(!identifyTimeSection(config->tpfvConfig.at(i)))//当前时间是否在本时段
        {
            continue;
        }
        LogOut(QString("当前所在时段：　%1") .arg(QString::number(i)),2);

        ucCanAddr = 0;
        ucIndexID = 0;//起始地址

        for(int termType = 0; termType< 3 ; termType++)
        {
            switch (termType)
            {
            case 0://单相
                ucIndexID  = ID_MinACSinCanID;
                break;
            case 1://三相
                ucIndexID  = ID_MinACThrCanID;
                break;
            case 2://直流
                ucIndexID  = ID_MinDCCanID;
                break;
            default:
                break;
            }
            if(ucNumTerminal[termType] > 0)
            {
                for(int j = 0; j < ucNumTerminal[termType]; j++)
                {
                    ucCanAddr = ucIndexID+j;
                    LogOut(QString("枪编号＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝%1") .arg(QString::number(ucCanAddr)),2);

                    TerminalStatus status;
                    pDevCache->QueryTerminalStatus(ucCanAddr,status);
                    LogOut(QString("终端状态：　%1") .arg(QString::number((int)status.cStatus)),2);

                    CHARGE_STEP stChargeStep;//存放从缓存中读出的状态机
                    if(!pDevCache->QueryChargeStep(ucCanAddr, stChargeStep))
                    {
                        continue;
                    }

                    LogOut(QString("充电指令类型：　%1") .arg(QString::number((int)stChargeStep.ucCmdValue)),2);
                    if(stChargeStep.ucCmdValue == CHARGE_CMD_TYPE_START_CHARGE_ECONOMIC ||
                        stChargeStep.ucCmdValue ==  CHARGE_CMD_TYPE_PAUSH_CHARGE )    //经济充电   DEBUG
                    {
                        SmartChargeAction(status, i,ucCanAddr,0);
                    }
                }
            }
        }
        break;
    }
}
void PeakShaving::slot_chargeFromEnergyPlan(unsigned char canID,unsigned char cmd)
{
    InfoMap qInfoMap;
    unsigned char ack= 255;
LogOut(QString("削峰填谷申请canid  %1").arg(canID),2);
LogOut(QString("削峰填谷申请cmd  %1").arg(cmd),2);
    qInfoMap.insert(Addr_CanID_Comm, QByteArray(1,canID));
    qInfoMap.insert(Addr_AckResult_Ctrl, QByteArray(1,ack));
    qInfoMap.insert(Addr_ChargeCmd_Ctrl, QByteArray(1,cmd));
    emit sig_ChargeApply(qInfoMap,AddrType_ToPeakChargeCMD_Ack);   //返回开始充电的ACK给能效后台
    LogOut(QString("削峰填谷申请开始充电返回结果 %1").arg(QString::number(255)),1);


    if(cmd == CHARGE_CMD_TYPE_STOP_DISCHARGE || cmd == CHARGE_CMD_TYPE_STOP_CHARGE)
        emit sig_ChargeApply(qInfoMap,AddrType_InPeakApplyStopCharge);//停止充电／放电
    else if(cmd == CHARGE_CMD_TYPE_START_CHARGE_NOW || cmd == CHARGE_CMD_TYPE_START_CHARGE_ECONOMIC || cmd == CHARGE_CMD_TYPE_START_DISCHARGE)
        emit sig_ChargeApply(qInfoMap,AddrType_InPeakApplyStartCharge);//开始充电／放电
}
//收到云平台经济充电，只应答
void PeakShaving::slot_StartChargingCMD(unsigned char canID)
{
    InfoMap qInfoMap;
    unsigned char ack=255;
    qInfoMap.insert(Addr_CanID_Comm, QByteArray(1,canID));
    qInfoMap.insert(Addr_AckResult_Ctrl, QByteArray(1,ack));
    qInfoMap.insert(Addr_ChargeCmd_Ctrl, QByteArray(1,2));
    emit sig_ChargeApply(qInfoMap,AddrType_InPeakApplyStartChargeACK);   //返回开始充电的ACK
//        QDateTime dt = QDateTime::currentDateTime();

//        LogOut(QString("slot_StartChargingCMD 当前时间：　%1") .arg(dt.toString()),2);

//        for(int i=0;i<config->tpfvConfig.count();i++)//tpfvConfig每条记录为一个时段
//        {
//            if(!identifyTimeSection(config->tpfvConfig.at(i)))//当前时间是否在本时段
//            {
//                continue;
//            }
//             LogOut(QString("当前所在时段：　%1") .arg(QString::number(i)),2);
//            LogOut(QString("枪编号＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝%1") .arg(QString::number(canID)),2);
//           // CHARGE_STEP stChargeStep;//存放从缓存中读出的状态机
//            TerminalStatus status;

//            pDevCache->QueryTerminalStatus(canID,status);
//            LogOut(QString("终端状态：　%1") .arg(QString::number((int)status.cStatus)),2);
//            SmartChargeAction(status, i,canID,1);
//            break;
//        }
}
///
/// \brief SmartCharge::slot_paraChange
/// \param TPFVconfig
///更新错峰充电时段配置
void PeakShaving::slot_paraChange(stAllTPFVConfig * TPFVconfig)
{
    configLock.lock();
   config = TPFVconfig;
   configLock.unlock();
}

///
/// \brief SmartCharge::slot_smartChargeSwitch
/// \param flag
///根据设置启停定时器
void PeakShaving::slot_smartChargeSwitch(bool flag)
{

//    if(actionTimer->isActive() && !flag)
//    {
//        actionTimer->stop();
//    }
//    else if(!actionTimer->isActive() && flag)
//    {
//        actionTimer->start();
//    }
}
