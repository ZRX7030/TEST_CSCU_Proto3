#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <QDebug>
#include <QSettings>
#include <QStringList>
#include <QFile>
#include "RealDataFilter/RealDataFilter.h"
#include "Infotag/Ammeter.h"

#define VIRTUAL_METER_CONF	"/mnt/nandflash/database/virtual_meter.db"

RealDataFilter::RealDataFilter()
{
	_strLogName = "event";

    devCache = DevCache::GetInstance();
    bus = CBus::GetInstance();
    log = Log::GetInstance();
    db = DBOperate::GetInstance();
    param = ParamSet::GetInstance();

    realDataCallBack = NULL;

    CSCURebootRecord();			//记录启动记录

    /*读配置*/
    unParamConfig config;
    param->querySetting(&config, PARAM_CSCU_SYS);
    chargerDCNum = config.cscuSysConfig.directCurrent;
    chargerAC1Num = config.cscuSysConfig.singlePhase;
    chargerAC3Num = config.cscuSysConfig.threePhase;

    param->querySetting(&config, PARAM_CHARGE);
    vinFlag = config.chargeConfig.vinType;
    energyFilterFlag = config.chargeConfig.energyFilter;
    m_iMeterType = config.chargeConfig.meterType;

    param->querySetting(&config, PARAM_EXTRA);
    _b300KWEnable = config.extraConfig.coupleGun300KW;

    param->querySetting(&ThreePhaseTypeConfig, PARAM_PHASE_TYPE);

    InitVirtualMeter();

    signalOldStatus = new FRAME_REMOTE_SINGLE[chargerAC1Num+chargerAC3Num+chargerDCNum];
    /*加载旧的数据*/
    TerminalStatus status;
    int offset, num, canaddr;
    for(int k=0; k<3; k++)
    {
        if(k == 0)
        {
            offset = 0;
            canaddr = 0;
            num = chargerAC1Num;
        }
        else if(k == 1)
        {
            offset = chargerAC1Num;
            canaddr = 150;
            num = chargerAC3Num;
        }
        else if(k == 2)
        {
            offset = chargerAC1Num + chargerAC3Num;
            canaddr = 180;
            num = chargerDCNum;
        }

        for(int i=0; i<num; i++)
        {
            devCache->QueryTerminalStatus(canaddr+i+1, status);
            if(status.cCanAddr != 0xff)
            {
                signalOldStatus[offset+i] = status.stFrameRemoteSingle;
            }
        }
    }
    /*离线检测*/
    timer.setInterval(5000);
    QObject::connect(&timer, SIGNAL(timeout()), this, SLOT(timeOut()));
    timer.start();

	loadCouple300KWSetting();

    QList<int> list;
    list.append(AddrType_ParamChange);
	list.append(AddrType_LogicState);
    bus->RegistDev(this, list);
}

RealDataFilter::~RealDataFilter(void)
{
    delete[] signalOldStatus;
}

RealDataFilter *RealDataFilter::GetInstance()
{
	static RealDataFilter *ins = NULL;
	if(!ins){
		ins = new RealDataFilter();
	}

	return ins;
}

/**
 *终端离线判断
 */
void RealDataFilter::terminalOfflineCheck(void)
{
    TerminalStatus status;
    int num, canaddr;
    QDateTime currDate = QDateTime::currentDateTime();
    for(int k=0; k<3; k++)
    {
        if(k == 0)
        {
            canaddr = 0;
            num = chargerAC1Num;
        }
        else if(k == 1)
        {
            canaddr = 150;
            num = chargerAC3Num;
        }
        else if(k == 2)
        {
            canaddr = 180;
            num = chargerDCNum;
        }

        for(int i=0; i<num; i++)
        {
            devCache->QueryTerminalStatus(canaddr+i+1, status);
            QDateTime lastTime = QDateTime::fromString(QString(status.lastActiveTime), "yyyyMMddhhmmss");
            QString timeDate = currDate.toString("yyyyMMddhhmmss");

            int timeValue = lastTime.secsTo(currDate);
            //            if(timeValue > 45 && timeValue < 300)
            if(timeValue > 180 && timeValue < 300)//应直流机要求修改离线判断时间
            {
                unsigned char oldStatus = 0;

                TerminalStatus &update = devCache->GetUpdateTerminalStatus(canaddr+i+1);
                oldStatus = update.stFrameRemoteSingle.charge_status;
                //离线, 遥测置零, BMS信息置零
                //                memset(&update.stFrameBmsInfo, 0x00, sizeof(update.stFrameBmsInfo));
                //                memset(&update.stFrameRemoteMeSurement1, 0x00, sizeof(update.stFrameRemoteMeSurement1));
                //                memset(&update.stFrameRemoteMeSurement2, 0x00, sizeof(update.stFrameRemoteMeSurement2));

                update.stFrameRemoteSingle.charge_status = 6;		//离线
                snprintf(update.lastActiveTime, sizeof(update.lastActiveTime), "%s", timeDate.toAscii().data());
                devCache->FreeUpdateTerminalStatus();

                InfoMap Map;
                unsigned char real_can = canaddr+i+1;
                if(burstStatusCheck(BURST_CHARGE_STATUS, 6, real_can, Map))
                {
                    Map.insert(Addr_CanID_Comm, QByteArray((char *)&real_can, 1));
                    emit sigToBus(Map, AddrType_TermSignal);
                }

                if(oldStatus != 6)
                    onOffLineRecord(oldStatus, 6, canaddr+i+1);
            }
            else if(timeValue >300 || timeValue < 0)			//超过5分钟
            {
                TerminalStatus &update = devCache->GetUpdateTerminalStatus(canaddr+i+1);
                snprintf(update.lastActiveTime, sizeof(update.lastActiveTime), "%s", timeDate.toAscii().data());
                devCache->FreeUpdateTerminalStatus();
            }
        }
    }
}

//有功电能过滤
//void RealDataFilter::activeEnergyCheck(TerminalStatus & Status, uint &uiNowEnergy)
//{
//    InfoMap map;
//    QString logStr;

//    //充电中, 进行电度过滤
//    if(Status.cStatus == CHARGE_STATUS_CHARGING){
//        //累计原始电度记录点
//        if(Status.stFrameRemoteMeSurement2.ucRecordNum < 3){
//            Status.stFrameRemoteMeSurement2.active_electric_energy_old[Status.stFrameRemoteMeSurement2.ucRecordNum] = uiNowEnergy;
//            Status.stFrameRemoteMeSurement2.ucRecordNum++;
//        }else{
//            //线性判断
//            if(Status.stFrameRemoteMeSurement2.active_electric_energy_old[2] > uiNowEnergy){   //上次记录的电量值大于本次电量值
//                double dAmmeterRange = param->getAmmeterRange(Status.cCanAddr);  //获取电表最大量程

//                //判断电表是否已经过最大值, 累计原始电度记录点清0, 重新统计//如果一个异常跳变点符合此条件将会被误判为超量程--------
//                if(Status.stFrameRemoteMeSurement2.active_electric_energy_old[2] > (int)(dAmmeterRange * 100) - MAX_ABSOLUTE_ERROR_ALLOWED_ACTIVE_ENERGY){
//                    Status.stFrameRemoteMeSurement2.ucRecordNum = 0;
//                    for(int i = 0; i < 3; i++){
//                        Status.stFrameRemoteMeSurement2.active_electric_energy_old[i] = 0;
//                    }
//                }else{
//                    //没有超过电表最大值, 报警 ---//如果old[2]中数值是异常的大数值，则将会丢弃当前正常的小数值------
//                    map.insert(Addr_CanID_Comm, QByteArray((char *)&Status.cCanAddr, 1));
//                    map.insert(Addr_AbnormalActiveEnergy, QByteArray((char *)&uiNowEnergy, 4));
//                    emit sigToBus(map, AddrType_ActiveEnergyFault_Term);

//                    logStr.sprintf("电度过滤异常 线性判断 can地址: %d, 异常电能: %f, 原始电能:%f, %f, %f ",
//                            Status.cCanAddr,
//                            (double)uiNowEnergy / 100.0,
//                            (double)Status.stFrameRemoteMeSurement2.active_electric_energy_old[0] / 100.0,
//                            (double)Status.stFrameRemoteMeSurement2.active_electric_energy_old[1] / 100.0,
//                            (double)Status.stFrameRemoteMeSurement2.active_electric_energy_old[2] / 100.0);

//                    log->getLogPoint(_strLogName)->info(logStr);
//                    //扔掉错误数据
//                    uiNowEnergy = Status.stFrameRemoteMeSurement2.active_electric_energy_old[2];
//                }
//            }

//            //功率判断 允许绝对误差+当前电量+(1+允许相对误差)*功率*时间 > 当前电量 为正常//old[0],active_power的值不能保证是正确的值
//            //old[0]+1.5*(kwh*6s)=理论上当前电量 +20kwh(最大功率下6秒可能产生的偏差电量) 理论上应该>当前电量
//            if( (MAX_ABSOLUTE_ERROR_ALLOWED_ACTIVE_ENERGY
//                 + Status.stFrameRemoteMeSurement2.active_electric_energy_old[0]
//                 + (1+MAX_RELATIVE_ERROR_ALLOWED_ACTIVE_ENERGY)*(Status.stFrameRemoteMeSurement1.active_power * 6/3600.0 * 100))
//                    <= uiNowEnergy ){

//                map.insert(Addr_CanID_Comm, QByteArray((char *)&Status.cCanAddr, 1));
//                map.insert(Addr_AbnormalActiveEnergy, QByteArray((char *)&uiNowEnergy, 4));
//                emit sigToBus(map, AddrType_ActiveEnergyFault_Term);

//                logStr.sprintf("电度过滤异常 功率判断 can地址: %d, 异常电能: %f, 原始电能:%f, %f, %f ",
//                        Status.cCanAddr,
//                        (double)uiNowEnergy / 100.0,
//                        (double)Status.stFrameRemoteMeSurement2.active_electric_energy_old[0] / 100.0,
//                        (double)Status.stFrameRemoteMeSurement2.active_electric_energy_old[1] / 100.0,
//                        (double)Status.stFrameRemoteMeSurement2.active_electric_energy_old[2] / 100.0);

//                log->getLogPoint(_strLogName)->info(logStr);
//                //扔掉错误数据
//                uiNowEnergy = Status.stFrameRemoteMeSurement2.active_electric_energy_old[2];
//            }
//        }
//    }else{
//        Status.stFrameRemoteMeSurement2.ucRecordNum = 0;
//    }
//    //更换记录值
//    for(int i = 1; i < 3; i++){
//        Status.stFrameRemoteMeSurement2.active_electric_energy_old[i-1] = Status.stFrameRemoteMeSurement2.active_electric_energy_old[i];
//    }

//    Status.stFrameRemoteMeSurement2.active_electric_energy_old[2] = uiNowEnergy;

//    //判断
//    /*
//    if(CheckOldEnergy(Status.stFrameRemoteMeSurement2.active_electric_energy_old[1],Status.stFrameRemoteMeSurement2.active_electric_energy_old[2],Status)==3)
//    {
//        Status.stFrameRemoteMeSurement2.active_electric_energy_old[1] =0;
//        Status.stFrameRemoteMeSurement2.ucRecordNum>0? Status.stFrameRemoteMeSurement2.ucRecordNum--:Status.stFrameRemoteMeSurement2.ucRecordNum=0;
//    }
//    */

//}
//有功电能过滤
bool RealDataFilter::activeEnergyCheck(TerminalStatus & Status, uint &uiNowEnergy)
{
    InfoMap map;
    QString logStr;
    bool flag = false;

    //充电中, 进行电度过滤
    if(Status.cStatus == CHARGE_STATUS_CHARGING || Status.stFrameRemoteSingle.charge_status == CHARGE_STATUS_REALTIME_CHARGING){
        //累计原始电度记录点
         if(Status.stFrameRemoteMeSurement2.ucRecordNum < 1){   //n=0
            Status.stFrameRemoteMeSurement2.ucRecordNum++;

        }else{
            //线性判断
            if(Status.stFrameRemoteMeSurement2.active_electric_energy_old[2] > uiNowEnergy){   //上次记录的电量值大于本次电量值
                map.insert(Addr_CanID_Comm, QByteArray((char *)&Status.cCanAddr, 1));
                map.insert(Addr_AbnormalActiveEnergy, QByteArray((char *)&uiNowEnergy, 4));
                emit sigToBus(map, AddrType_ActiveEnergyFault_Term);

                logStr.sprintf("电度过滤异常 线性判断 can地址: %d, 异常电能: %f, 原始电能:%f, %f, %f ",
                        Status.cCanAddr,
                        (double)uiNowEnergy / 100.0,
                        (double)Status.stFrameRemoteMeSurement2.active_electric_energy_old[0] / 100.0,
                        (double)Status.stFrameRemoteMeSurement2.active_electric_energy_old[1] / 100.0,
                        (double)Status.stFrameRemoteMeSurement2.active_electric_energy_old[2] / 100.0);

                log->getLogPoint(_strLogName)->info(logStr);
                //扔掉错误数据
                uiNowEnergy = Status.stFrameRemoteMeSurement2.active_electric_energy_old[2];
            }
            if(Status.stFrameRemoteMeSurement2.ucRecordNum < 3)   //n=1,2
            {
                Status.stFrameRemoteMeSurement2.ucRecordNum++;
                if(CheckOldEnergy(Status.stFrameRemoteMeSurement2.active_electric_energy_old[2],uiNowEnergy,Status)==3)   //比较异常则丢弃之前的值。
                {
                    Status.stFrameRemoteMeSurement2.active_electric_power_old[2] =Status.stFrameRemoteMeSurement2.active_electric_power_old[1];
                    Status.stFrameRemoteMeSurement2.ucRecordNum>0? Status.stFrameRemoteMeSurement2.ucRecordNum--:Status.stFrameRemoteMeSurement2.ucRecordNum=0;
                    map.insert(Addr_CanID_Comm, QByteArray((char *)&Status.cCanAddr, 1));
                    map.insert(Addr_AbnormalActiveEnergy, QByteArray((char *)&uiNowEnergy, 4));
                    emit sigToBus(map, AddrType_ActiveEnergyFault_Term);

                    logStr.sprintf("电度过滤异常 功率判断 can地址: %d, 异常电能: %f, 原始电能:%f, %f, %f ",
                            Status.cCanAddr,
                            (double)uiNowEnergy / 100.0,
                            (double)Status.stFrameRemoteMeSurement2.active_electric_energy_old[0] / 100.0,
                            (double)Status.stFrameRemoteMeSurement2.active_electric_energy_old[1] / 100.0,
                            (double)Status.stFrameRemoteMeSurement2.active_electric_energy_old[2] / 100.0);

                    log->getLogPoint(_strLogName)->info(logStr);
                     uiNowEnergy = Status.stFrameRemoteMeSurement2.active_electric_energy_old[2];
                }
            }else//n=3
            {
            //功率判断 允许绝对误差+当前电量+(1+允许相对误差)*功率*时间 > 当前电量 为正常//old[0],active_power的值不能保证是正确的值
            //old[0]+1.5*(kwh*6s)=理论上当前电量 +20kwh(最大功率下6秒可能产生的偏差电量) 理论上应该>当前电量
                Status.stFrameRemoteMeSurement2.active_electric_power_old[2] = Status.stFrameRemoteMeSurement1.active_power * 2/3600.0 * 100;
                if( (MAX_ABSOLUTE_ERROR_ALLOWED_ACTIVE_ENERGY
                     + Status.stFrameRemoteMeSurement2.active_electric_energy_old[0]
                     //+ (1+MAX_RELATIVE_ERROR_ALLOWED_ACTIVE_ENERGY)*(Status.stFrameRemoteMeSurement1.active_power * 6/3600.0 * 100))
                     + (1+MAX_RELATIVE_ERROR_ALLOWED_ACTIVE_ENERGY)*(Status.stFrameRemoteMeSurement2.active_electric_power_old[2]+Status.stFrameRemoteMeSurement2.active_electric_power_old[1]+Status.stFrameRemoteMeSurement2.active_electric_power_old[0]))
                        <= uiNowEnergy ){

                    map.insert(Addr_CanID_Comm, QByteArray((char *)&Status.cCanAddr, 1));
                    map.insert(Addr_AbnormalActiveEnergy, QByteArray((char *)&uiNowEnergy, 4));
                    emit sigToBus(map, AddrType_ActiveEnergyFault_Term);

                    logStr.sprintf("电度过滤异常 功率判断 can地址: %d, 异常电能: %f, 原始电能:%f, %f, %f ",
                            Status.cCanAddr,
                            (double)uiNowEnergy / 100.0,
                            (double)Status.stFrameRemoteMeSurement2.active_electric_energy_old[0] / 100.0,
                            (double)Status.stFrameRemoteMeSurement2.active_electric_energy_old[1] / 100.0,
                            (double)Status.stFrameRemoteMeSurement2.active_electric_energy_old[2] / 100.0);

                    log->getLogPoint(_strLogName)->info(logStr);
                    //扔掉错误数据
                    uiNowEnergy = Status.stFrameRemoteMeSurement2.active_electric_energy_old[2];
                }
            }
        }
        flag = true;
    }else{
        Status.stFrameRemoteMeSurement2.ucRecordNum = 0;

    }
    //更换记录值
    for(int i = 1; i < 3; i++){
        Status.stFrameRemoteMeSurement2.active_electric_energy_old[i-1] = Status.stFrameRemoteMeSurement2.active_electric_energy_old[i];
        Status.stFrameRemoteMeSurement2.active_electric_power_old[i-1] = Status.stFrameRemoteMeSurement2.active_electric_power_old[i];
    }

    Status.stFrameRemoteMeSurement2.active_electric_energy_old[2] = uiNowEnergy;
    return flag;
}

//如果相邻的两个电判断为异常,则启动异常数据报警。
unsigned char RealDataFilter::CheckOldEnergy(unsigned int first_energy,unsigned int second_energy,TerminalStatus & Status)
{
    if(first_energy==0 ||
            second_energy==0)
        return 1;
    if(( (MAX_ABSOLUTE_ERROR_ALLOWED_ACTIVE_ENERGY/3)
         + first_energy
         + (1+MAX_RELATIVE_ERROR_ALLOWED_ACTIVE_ENERGY)*(Status.stFrameRemoteMeSurement1.active_power * 2/3600.0 * 100))
            > second_energy )
    {
        Status.stFrameRemoteMeSurement2.active_electric_power_old[2] = Status.stFrameRemoteMeSurement1.active_power * 2/3600.0 * 100;
        return 2;
    }
    else
        return 3;
}

void RealDataFilter::slotFromBus(InfoMap Map, InfoAddrType enAddrType)
{
    if(enAddrType == AddrType_ParamChange)
    {
        if(Map.contains(Addr_Param_Change))
        {
            int type = *((int *)Map.value(Addr_Param_Change).data());
            unParamConfig config;
            if(type == PARAM_CSCU_SYS)
            {
                param->querySetting(&config, type);
                chargerDCNum = config.cscuSysConfig.directCurrent;
                chargerAC1Num = config.cscuSysConfig.singlePhase;
                chargerAC3Num = config.cscuSysConfig.threePhase;
            }
            else if(type == PARAM_CHARGE)
            {
                param->querySetting(&config, type);
                vinFlag = config.chargeConfig.vinType;
            }
            else if(type == PARAM_PHASE_TYPE)
            {
                ThreePhaseTypeConfig.phaseTypeConfig.clear();
                param->querySetting(&ThreePhaseTypeConfig, type);
            }
        }
    }else if(enAddrType == AddrType_LogicState){
		if(!Map.contains(Addr_LogicState_Burst))
			return;
		if(!Map.contains(Addr_CanID_Comm))
			return;
		char status, canAddr;

		status = Map[Addr_LogicState_Burst].at(0);
		canAddr = Map[Addr_CanID_Comm].at(0);

		if(status == CHARGE_STATUS_FREE){
			//清理BMS信息
			TerminalStatus & status = devCache->GetUpdateTerminalStatus(canAddr);
			memset(&status.stFrameBmsInfo, 0, sizeof(status.stFrameBmsInfo));//BMS信息 充电中
			memset(&status.stFrameBmsHand, 0, sizeof(status.stFrameBmsHand));//BMS信息 握手
			memset(&status.stFrameBmsParam, 0, sizeof(status.stFrameBmsParam));//BMS信息 参数配置
			memset(&status.stFrameBmsChargeTerm, 0, sizeof(status.stFrameBmsChargeTerm));//BMS信息 充电终止
			devCache->FreeUpdateTerminalStatus();
		}
	}
}

/**
 *定时器超时
 */
void RealDataFilter::timeOut(void)
{
    terminalOfflineCheck();
    CCUOffLineCheck();  //nihai add 2017-07-30 检测CCU是否离线
    energyPlanDevOnlineCheck_DCDC_ES();
}

/**
 *状态变化突发
 */
bool RealDataFilter::burstStatusCheck(int type, unsigned char currStatus, unsigned char canAddr, InfoMap &Map)
{
    /*根据canaddr查找数据*/
    FRAME_REMOTE_SINGLE *signal = NULL;
    if(canAddr > 0 && canAddr <= 150)
        signal = &signalOldStatus[canAddr - 1];
    else if(canAddr > 150 && canAddr <= 180)
        signal = &signalOldStatus[canAddr - 151 + chargerAC1Num];
    else if(canAddr > 180 && canAddr <= 250)
        signal = &signalOldStatus[canAddr - 181 + chargerAC1Num + chargerAC3Num];
    else
        return false;

    /*查找突发类型*/
    int burstKey=0, oldKey=0;
    char *oldChangeStatus;
    switch(type)
    {
    case BURST_LINK_STATUS:
        burstKey = Addr_LinkState_Sudden;
        oldKey = Addr_LinkState_Term;
        oldChangeStatus = &signal->link_status;
        break;
        //case BURST_CARLOCK_STATUS:
        //   burstKey = Addr_ParkingSpaceFreeFlag_Sudden;
        //   oldKey = Addr_ParkingSpaceFreeFlag_Term;
        //   oldChangeStatus = &signal->parking_space;
        //   break;
    case BURST_CHARGE_STATUS:
        burstKey = Addr_WorkState_Sudden;
        oldKey = Addr_WorkState_Term;
        oldChangeStatus = &signal->charge_status;
        break;
    case BURST_RELAY_STATUS:
        burstKey = Addr_RelyState_Sudden;
        oldKey = Addr_RelyState_Term;
        oldChangeStatus = &signal->relay_status;
        break;
    case BURST_FAULT_STATUS:
        burstKey = Addr_FaultCode_Sudden;
        oldKey = Addr_FaultCode_Term;
        oldChangeStatus = &signal->status_fault;
        break;
    default:
        return false;
    }

    /*突发判断*/
    if(currStatus != *oldChangeStatus)
    {
        Map.insert(burstKey, QByteArray((char *)&currStatus, 1));
        Map.insert(oldKey, QByteArray((char *)oldChangeStatus, 1));

        *oldChangeStatus = currStatus;

        return true;
    }
    return false;
}
/**
 *系统重启记录
 */
void RealDataFilter::CSCURebootRecord(void)
{
    char cmd_buff[256];

    snprintf(cmd_buff, sizeof(cmd_buff), "insert into cscu_reboot_table (record_time, flag) values(\"%s\", 1)", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toAscii().data());
    db->DBSqlExec(cmd_buff, DB_PROCESS_RECORD);
}
/**
 *终端插拔枪记录
 */
void RealDataFilter::gunInsertPullRecord(unsigned char oldLink, unsigned char newLink, unsigned char canAddr)
{
    char cmd_buff[256];

    if( oldLink == 0 && newLink == 1)   //插枪
    {
        snprintf(cmd_buff, sizeof(cmd_buff), "insert into plug_gun_table (canaddr, insert_record_time, pull_record_time) values(\"%d\", \"%s\", \"\")", canAddr, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toAscii().data());
        db->DBSqlExec(cmd_buff, DB_PROCESS_RECORD);
    }
    else if(oldLink == 1 && newLink == 0)   //拔枪
    {
        snprintf(cmd_buff, sizeof(cmd_buff), "update plug_gun_table set pull_record_time = \"%s\" where canaddr=\"%d\" and pull_record_time=\"\"", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toAscii().data(), canAddr);
        db->DBSqlExec(cmd_buff, DB_PROCESS_RECORD);
    }
}

/**
 *BMS数据记录
 * 内部无加锁,若外部调用,则需要在函数外加锁.
 */
void RealDataFilter::bmsInfoRecord(TerminalStatus Status)
{
    QString todo;
    QString endFrame1,endFrame2;
    char * pF = (char * )&(Status.stFrameBmsChargeTerm.BMSErrorFrame);
    endFrame1.sprintf("%02x %02x %02x %02x",pF[0], pF[1], pF[2], pF[3]);
    pF = (char * )&(Status.stFrameBmsChargeTerm.ChargerErrorFrame);
    endFrame2.sprintf("%02x %02x %02x %02x",pF[0], pF[1], pF[2], pF[3]);
    //BMS数据写入数据库并清空缓存内容
    todo.sprintf("insert into bms_static_table (  \
                 canAddr, guid_number, charager_id, record_time, gbt27930_version, \
                 whole_max_charge_voltage, battery_type, battery_capacity, battery_total_voltage, manufactor_name, \
                 battery_group_serial_number, battery_group_datetime, battery_group_charge_count, battery_group_property, car_vin, \
                 bms_version, battery_single_voltage, battery_single_current, battary_total_energy, max_charge_voltage, \
                 max_allow_tempeture, whole_soc, whole_current_voltage, max_out_voltage, min_out_voltage, \
                 max_out_current, min_out_current, bms_stop_reson, bms_stop_fault, bms_stop_error, \
                 charger_stop_reason, charger_stop_fault, charger_stop_error, stop_soc, min_single_volatge, \
                 max_single_volatge, min_tempeture, max_tempeture, bms_error_package, charger_error_package) \
            values(    \
                \"%d\",\"%s\", \"%d\",\"%s\",\"%s\",          \
                \"%f\",\"%d\",\"%f\",\"%f\",\"%s\",   \
                \"%d\",\"%s\",\"%d\",\"%d\",\"%s\",   \
                \"%s\",\"%f\",\"%f\",\"%f\",\"%f\",   \
                \"%f\",\"%f\",\"%f\",\"%f\",\"%f\",   \
                \"%f\",\"%f\",\"%d\",\"%d\",\"%d\",   \
                \"%d\",\"%d\",\"%d\",\"%f\",\"%f\",    \
                \"%f\",\"%f\",\"%f\",\"%s\",\"%s\"    \
                )", \
            Status.cCanAddr,"000000", Status.stFrameBmsHand.BatteryOwnerFlag, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toAscii().data(), Status.stFrameBmsHand.BMSProtocolVer,\
            Status.stFrameBmsHand.MaxAllowedVoltage, Status.stFrameBmsHand.BatteryType, Status.stFrameBmsHand.BatteryRatedCapacity, Status.stFrameBmsHand.BatteryRatedVoltage, Status.stFrameBmsHand.BatteryManufacturer, \
            Status.stFrameBmsHand.BatterySerialNum, Status.stFrameBmsHand.BatteryProduceDate, Status.stFrameBmsHand.BatteryChargeTime, Status.stFrameBmsHand.BatteryOwnerFlag, Status.stFrameBmsInfo.BMS_car_VIN,\
            Status.stFrameBmsHand.BMSSoftwareVer, Status.stFrameBmsParam.SingleBatteryMaxAllowedVoltage, Status.stFrameBmsParam.MaxAllowedCurrent, Status.stFrameBmsParam.BatteryTotalEnergy, Status.stFrameBmsParam.MaxParamAllowedVoltage, \
            Status.stFrameBmsParam.MaxtAllowedTemp, Status.stFrameBmsParam.ParamSOC, Status.stFrameBmsParam.BatteryVoltage, Status.stFrameBmsParam.MaxOutputVoltage, Status.stFrameBmsParam.MinOutputVoltage, \
            Status.stFrameBmsParam.MaxOutputCurrent, Status.stFrameBmsParam.MinOutputCurrent, Status.stFrameBmsChargeTerm.BMSStopReason, Status.stFrameBmsChargeTerm.BMSFaultReason, Status.stFrameBmsChargeTerm.BMSErrorReason, \
            Status.stFrameBmsChargeTerm.ChargerStopReason, Status.stFrameBmsChargeTerm.ChargerFaultReason, Status.stFrameBmsChargeTerm.ChargerErrorReason, Status.stFrameBmsChargeTerm.ChargeEndSOC, Status.stFrameBmsChargeTerm.MinSingleVoltage, \
            Status.stFrameBmsChargeTerm.MaxSingleVoltage, Status.stFrameBmsChargeTerm.MinTemp, Status.stFrameBmsChargeTerm.MaxTemp, endFrame1.toAscii().data(), endFrame2.toAscii().data());

    db->DBSqlExec(todo.toAscii().data(), DB_PROCESS_RECORD);

}

/**
 *终端上下线记录
 */
void RealDataFilter::onOffLineRecord(unsigned char oldStatus, unsigned char newStatus, unsigned char canAddr)
{
    char cmd_buff[256];

    if(newStatus == 6 && oldStatus != 6)
    {
        snprintf(cmd_buff, sizeof(cmd_buff), "insert into terminal_online_table (canaddr, charger_offline_time, charger_online_time) values(\"%d\", \"%s\", \"\")", canAddr, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toAscii().data());
        db->DBSqlExec(cmd_buff, DB_PROCESS_RECORD);
    }
    else if(oldStatus == 6 && newStatus != 6)
    {
        snprintf(cmd_buff, sizeof(cmd_buff), "update terminal_online_table set charger_online_time=\"%s\" where canaddr=%d and charger_online_time=\"\"", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toAscii().data(), canAddr);
        db->DBSqlExec(cmd_buff, DB_PROCESS_RECORD);
    }
}

/**
 *故障数据存储，只有当故障数据发生变化时才处理
 */
void RealDataFilter::faultDeal(unsigned char faultCode, unsigned char canAddr)
{
    char cmd_buff[256];
    struct db_result_st result;

    /*更新上一次的故障停止时间*/
    snprintf(cmd_buff, sizeof(cmd_buff), "select id from terminal_fault_table where fault_stop_time=\"\" and canaddr=\"%d\" limit 1", canAddr);
    if(0 == db->DBSqlQuery((char *)cmd_buff, &result, DB_PROCESS_RECORD))
    {
        if(result.row != 0 && result.column != 0)
        {
            unsigned int id = atoi(result.result[0]);
            snprintf(cmd_buff, sizeof(cmd_buff), "update terminal_fault_table set fault_stop_time=\"%s\" where id=\"%d\"",QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toAscii().data(),id);
            db->DBSqlExec(cmd_buff, DB_PROCESS_RECORD);
        }

        db->DBQueryFree(&result);
    }

    /*添加新的记录*/
    if(faultCode > 0)
    {
        snprintf(cmd_buff, sizeof(cmd_buff), "insert into terminal_fault_table (canaddr, fault_start_time, fault_stop_time, fault_code) values(\"%d\", \"%s\", \"\",\"%d\")", canAddr, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toAscii().data(), faultCode);
        db->DBSqlExec(cmd_buff, DB_PROCESS_RECORD);
    }
}

/**
 *对外的数据更新接口
 */
void RealDataFilter::realDataUpdate(InfoMap Map, InfoAddrType enAddrType)
{
	//在新数据更新前进行回调，以处理数据突发。
    if(realDataCallBack){
        realDataCallBack(Map, enAddrType);
    }

    switch(enAddrType)
    {
    case AddrType_TermSignal:
        updateTelesignalData(Map);
        break;
    case AddrType_TermMeasure:
        updateTelemeterData(Map);
        break;
    case AddrType_TermBMS:
        updateBMSData(Map);
        break;
    case AddrType_Ammeter:
        updateAmmeterData(Map);
        break;
        //    case AddrType_EnergyPlanEnvSignal:
        //        updateEnergyPlanEnvData(Map);
        //        break;
    case AddrType_Alarm_Report:
        updateAlarmInfo(Map);
        break;
        //case AddrType_CCUMeasure_DCCab:
        //	updateCCUSignalMeasure(Map);
        //	break;
    case AddrType_TempHumi:
        updateTempHumi(Map);
        break;
    case AddrType_CCUSignal_DCCab:		//CCU遥信
    case AddrType_CCUMeasure_DCCab:		//CCU遥测
        updateCCUSignalMeasure(Map);
        break;
    case AddrType_PDUSignal_DCCab:		//PDU遥信
    case AddrType_PDUMeasure_DCCab:		//PDU遥测
        updatePDUSignalMeasure(Map);
        break;
    case AddrType_BOXSignal_DCCab:		//分支箱遥信
    case AddrType_BOXMeasure_DCCab:		//分支箱遥测
        //        updateBranchSignalMeasure(Map);
        break;
    case AddrType_MODSignal_DCCab:		//模块遥信
    case AddrType_MODMeasure_DCCab:		//模块遥
        updateDCModuleSignalMeasure(Map);
        break;
    case AddrType_FaultState_DCcab: //直流柜故障状态
        updateDCCabFaultState(Map);
        break;
    case AddrType_ES_Cab_Info:
        updateEnergyStorageCabinetInfo(Map);
        break;
    case AddrType_ES_Bat_Info:
        updateEnergyStorageCabinetBatteryInfo(Map);
        break;
    case AddrType_PH_Cab_Info:
        updatePhotoVoltaicCabinetInfo(Map);
        break;
    case AddrType_SC_Cab_Info:
        updateSysControlCabinetInfo(Map);
        break;
    case AddrType_FQ_Cab_Info:
        updateFourQuadrantCabinetInfo(Map);
        break;
    case AddrType_CD_Cab_Info:
        updateChargeDischargeCabinetInfo(Map);
        break;
    case AddrType_TD_Cab_Info:
        updateMainDistributionCabinetInfo(Map);
        break;
    case AddrType_PO_Mod_Info:
        updatePowerOptimizerInfo(Map);
        break;
    case AddrType_HY_Mod_Info:
        updateHygrothermographInfo(Map);
        break;
    case AddrType_SI_Cab_Info:
        updateSingleInverterCabinetInfo(Map);
        break;
    case AddrType_ACDC_Mod_Info:
        updateACDCInfo(Map);
        break;
    case AddrType_DCDC_Mod_CD_Info:
        updateDCDCInfo_CD(Map);
        break;
    case AddrType_DCDC_Mod_ES_Info:
        updateDCDCInfo_ES(Map);
        break;
    case AddrType_EMS_Info:
        updateEMSInfo(Map);
        break;
    case AddrType_TempContorlInfo:
        break;
    case AddrType_ModuleSpecInfo: //设备规格信息 9F
        //updateTermSpecInfo(Map);
        break;
    default:
        break;
    }
}

/**
 *所有设备有效数据上传完毕判断
 */
bool RealDataFilter::isAllValid()
{
    int validCount = 0;
    unsigned char startAddr[3]={1,151,181};
    unsigned char count[3];
    count[0] = chargerAC1Num;
    count[1] = chargerAC3Num;
    count[2] = chargerDCNum;

    for(int i=0; i<3; i++)
    {
        for(int k=0; k<count[i]; k++ )
        {
            TerminalStatus Status;

            devCache->QueryTerminalStatus(startAddr[i]+k, Status);
            if(Status.cCanAddr != 0xff)
            {
                if(Status.validFlag == ALL_VALID_NUMBER)
                    validCount++;
            }
        }
    }

    if(validCount == (chargerAC1Num + chargerAC3Num + chargerDCNum))
        return true;
    else
        return false;
}

//更新  设备规格信息
void RealDataFilter::updateTermSpecInfo(InfoMap CenterMap)
{
    char tmpValue[40];
    unsigned char canAddr;
    QByteArray Value, id;

    if(CenterMap.isEmpty())
        return;

    Value = CenterMap.value(Addr_CanID_Comm);
    canAddr = CenterMap[Addr_CanID_Comm].at(0);

    id = CenterMap.value(Addr_DevID_DC_Comm);

    QMap<unsigned int, QByteArray>::iterator it;

    stDCCabinetDatas &Status = devCache->GetUpdateDCCabinetMeter();
    if(!Status.ccuDeviceMap.contains(canAddr))
    {
        return ;
    }
    stCCUADviceSpecificationsMap &CCUADviceSpecifications = Status.ccuDeviceMap[canAddr].mapSingle.operator[](id);
    devCache->UpdateCCUDataTime(canAddr); //更新CCU接收报文的时间 nihai add

    //模块ID
    CCUADviceSpecifications.ModuleID = CenterMap[Addr_DevID_DC_Comm].at(0);
    //添加设备规格信息传输完成标志
    CCUADviceSpecifications.DeviceSpecificationsFlag = CenterMap[Addr_SepcEndFlag_Term].at(0);
    //槽位号
    CCUADviceSpecifications.SlotNum = CenterMap[Addr_SlotNum_Term].at(0);
    //模块序列号
    CCUADviceSpecifications.SerialNumber = CenterMap[Addr_SerialNumber_Term];
    //软件1版本
    CCUADviceSpecifications.SoftwareVer = CenterMap[Addr_SoftwareVer_Term];
    //软件2版本
    CCUADviceSpecifications.SoftwareVer1 = CenterMap[Addr_SoftwareVer1_Term];
    //软件3版本
    CCUADviceSpecifications.SoftwareVer2 = CenterMap[Addr_SoftwareVer2_Term];
    //硬件版本
    CCUADviceSpecifications.HardwareVer = CenterMap[Addr_HardwareVer_Term];
    //   }
    devCache->FreeUpdateDCCabinetMeter();
}

/**
 *遥信数据更新
 *2017-12-6为适应南京新协议对所有遥信数据项做了突发
 */
void RealDataFilter::updateTelesignalData(InfoMap TeleindicationMap)
{
    char tmpValue[30];

    if(TeleindicationMap.isEmpty())
        return;
    int burstFlag = 0;
    QMap<unsigned int, QByteArray>::iterator it;
    QByteArray Value;
    unsigned char canAddr;

    Value = TeleindicationMap.value(Addr_CanID_Comm);
    if(Value.size())
        canAddr = *((unsigned char *)Value.data());
    else
        return;

    /*终端map更新，用于记录实际收到的终端数据个数，目前只记录一个can地址*/
    if(  ((canAddr>= ID_MinACSinCanID) &&(canAddr <= ID_MaxACSinCanID))
         ||((canAddr>= ID_MinACThrCanID) &&(canAddr <= ID_MaxACThrCanID))
         ||((canAddr>= ID_MinDCCanID) &&(canAddr <= ID_MaxDCCanID))
         )
    {
        stDCCabinetDatas &termStatus = devCache->GetUpdateDCCabinetMeter();
        unsigned char &terminalData = termStatus.terminalMap.operator[](canAddr);
        terminalData = canAddr;
        devCache->FreeUpdateDCCabinetMeter();
    }

    /*终端数据更新*/
    TerminalStatus & Status = devCache->GetUpdateTerminalStatus(canAddr);
    if(Status.cCanAddr == 0xff)
        return;

    InfoMap Map;
    bool dealFault = false;

    unsigned char faultCode=0;			//当前故障码
    unsigned char oldLinkStatus=0;		//上次连接状态
    unsigned char newLinkStatus=0;
    unsigned char oldChargeStatus=0;		//上次充电状态
    unsigned char newChargeStatus=0;

    /*记录收到遥信数据的时间*/
    QString strTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
    memcpy(Status.lastActiveTime, strTime.toAscii().data(), strTime.length());

    for(it = TeleindicationMap.begin(); it != TeleindicationMap.end(); ++it)
    {
        if((unsigned int)it.value().size() > (sizeof(tmpValue)-1))
            continue;

        unsigned int Key = it.key();
        memcpy(tmpValue, it.value().data(), it.value().size());
        tmpValue[it.value().size()] = 0;

        if( Key == Addr_ChargeGunNum_Term ){
            if(tmpValue[0] != Status.stFrameRemoteSingle.charge_interface_type){
                Map.insert(Addr_CanID_Comm, QByteArray(1, canAddr));
                Map.insert(Addr_ChargeGunNum_Term, QByteArray(1, Status.stFrameRemoteSingle.charge_interface_type));
                Map.insert(Addr_ChargeGunNum_Sudden, QByteArray(1, tmpValue[0]));
                burstFlag |= BURST_FLAG_CHARGE_INTERFACE;
            }
            Status.stFrameRemoteSingle.charge_interface_type = tmpValue[0];
        }else if( Key == Addr_LinkState_Term ){
            Status.validFlag |= 0x01;

            oldLinkStatus = Status.stFrameRemoteSingle.link_status;
            newLinkStatus = tmpValue[0];
            Status.stFrameRemoteSingle.link_status = tmpValue[0];

            if(burstStatusCheck(BURST_LINK_STATUS, tmpValue[0], canAddr, Map))
            {
                //插枪拔枪,清空BMS内存数据
                burstFlag |= BURST_FLAG_LINK_STATUS;
                //拔枪,将BMS信息记录数据库
                if(oldLinkStatus == 1 && newLinkStatus == 0)   //拔枪
                {
					couple300KWGunState(canAddr, false);
                    bmsInfoRecord(Status);
                }
				if(oldLinkStatus == 0 && newLinkStatus == 1){	//插枪
					couple300KWGunState(canAddr, true);
				}
				/*BMS信息清理放到充电服务逻辑工作状态后进行清理
                memset(&Status.stFrameBmsInfo, 0x00, sizeof(Status.stFrameBmsInfo));//BMS信息 充电中
                memset(&Status.stFrameBmsHand, 0x00, sizeof(Status.stFrameBmsHand));//BMS信息 握手
                memset(&Status.stFrameBmsParam, 0x00, sizeof(Status.stFrameBmsParam));//BMS信息 参数配置
                memset(&Status.stFrameBmsChargeTerm, 0x00, sizeof(Status.stFrameBmsChargeTerm));//BMS信息 充电终止
				*/
                Map.insert(Addr_WorkState_Sudden,QByteArray((char *)&Status.stFrameRemoteSingle.charge_status, 1));
                Map.insert(Addr_WorkState_Term,QByteArray((char *)&Status.stFrameRemoteSingle.charge_status, 1));
            }
        }else if( Key == Addr_RelyState_Term ){
            Status.validFlag |= 0x02;

            Status.stFrameRemoteSingle.relay_status = tmpValue[0];

            if(burstStatusCheck(BURST_RELAY_STATUS, tmpValue[0], canAddr, Map))
               burstFlag |= BURST_FLAG_RELAY_STATUS;
        }else if( Key == Addr_ParkingSpaceFreeFlag_Term ){
            if(tmpValue[0] != Status.stFrameRemoteSingle.parking_space){
                Map.insert(Addr_CanID_Comm, QByteArray(1, canAddr));
                Map.insert(Addr_ParkingSpaceFreeFlag_Term, QByteArray(1, Status.stFrameRemoteSingle.parking_space));
                Map.insert(Addr_ParkingSpaceFreeFlag_Sudden, QByteArray(1, tmpValue[0]));
                burstFlag |= BURST_FLAG_PARKING_SPACE;
            }
            Status.stFrameRemoteSingle.parking_space = tmpValue[0];
        }else if( Key == Addr_WorkState_Term ){
            Status.validFlag |= 0x04;
            if(canAddr < 150)   //单项交流老版本兼容性
            {
                if( ( tmpValue[0] == CHARGE_STATUS_REALTIME_CHARGING) && (Status.stFrameRemoteSingle.relay_status == 0) )
                {
                    switch(Status.stFrameRemoteSingle.charge_status)
                    {
                    case CHARGE_STATUS_REALTIME_CHARGING:   //继电器断开,  当前充电中, 上一个状态为充电中, 该状态置为待机
                    case CHARGE_STATUS_REALTIME_FAULT:   //继电器断开,  当前充电中, 上一个状态为故障, 该状态置为待机
                    case CHARGE_STATUS_REALTIME_OFFLINE:    //继电器断开,  当前充电中, 上一个状态为离线, 该状态置为待机
                    case CHARGE_STATUS_REALTIME_SWITCH:     //继电器断开,  当前充电中, 上一个状态为切换中, 该状态置为待机
                        tmpValue[0] = CHARGE_STATUS_REALTIME_STANDBY;
                        break;
                    default:    //继电器断开, 当前充电中, 默认, 充电机工作状态为充电中之前的工作状态
                        tmpValue[0] = Status.stFrameRemoteSingle.charge_status;
                        break;
                    }
                }
            }

            oldChargeStatus = Status.stFrameRemoteSingle.charge_status;
            newChargeStatus = tmpValue[0];
            Status.stFrameRemoteSingle.charge_status = tmpValue[0];

//            //add by FJC
//            CHARGE_STEP stChargeStep;//存放从缓存中读出的状态机
//            if(devCache->QueryChargeStep(canAddr, stChargeStep))
//            {
//                if(stChargeStep.ucCmdValue == CHARGE_CMD_TYPE_PAUSH_CHARGE)
//                {
//                    if(tmpValue[0] ==CHARGE_STATUS_REALTIME_STANDBY)
//                    {
//                        Status.stFrameRemoteSingle.charge_status = CHARGE_STATUS_REALTIME_PAUSE;//暂停
//                    }
//                }
//            }

            if(burstStatusCheck(BURST_CHARGE_STATUS, tmpValue[0], canAddr, Map)){
                burstFlag |= BURST_FLAG_CHARGER_STATUS;
                uint uiNowEnergy = Status.stFrameRemoteMeSurement2.active_electric_energy;
                if(newChargeStatus ==CHARGE_STATUS_REALTIME_CHARGING && energyFilterFlag == 1){
                    activeEnergyCheck(Status, uiNowEnergy);
                        //burstFlag |=0x01;
                }
			}
        }else if (Key == Addr_FaultCode_Term ){
            Status.stFrameRemoteSingle.status_fault = tmpValue[0];
            if(burstStatusCheck(BURST_FAULT_STATUS, tmpValue[0], canAddr, Map))
            {
                faultCode = tmpValue[0];
                dealFault = true;
                burstFlag |= BURST_FLAG_STATUS_FAULT;
            }
        }else if( Key == Addr_BMSFaultCode_Term ){
            if(tmpValue[0] != Status.stFrameRemoteSingle.BMS_fault){
                Map.insert(Addr_CanID_Comm, QByteArray(1, canAddr));
                Map.insert(Addr_BMSFaultCode_Term, QByteArray(1, Status.stFrameRemoteSingle.BMS_fault));
                Map.insert(Addr_BMSFaultCode_Sudden, QByteArray(1, tmpValue[0]));
                burstFlag |= BURST_FLAG_BMS_FAULT;
            }
            Status.stFrameRemoteSingle.BMS_fault = tmpValue[0];
        }else if( Key == Addr_ChargeEndCode_Term ){ 
             char stopreasontemp=tmpValue[0];
             //集控发起的终止，屏幕停止按钮，错峰停止，异常电量过滤停止  2018-8-1 hd
             if(stopreasontemp==2)
             {
                 CHARGE_STEP stChargeStep;//存放从缓存中读出的状态机
                 if(devCache->QueryChargeStep(canAddr, stChargeStep))
                 {
                     if( stChargeStep.ucStopReasonCSCU == 104)
                     {
                         stopreasontemp= 91;
                     }else if( stChargeStep.ucStopReasonCSCU == 114)
                     {
                         stopreasontemp = 93;
                     }else if( stChargeStep.ucStopReasonCSCU == 110)
                     {
                         stopreasontemp= 94;
                     }
                    //add by wbw 2018-08-22
                    else if( stChargeStep.ucStopReasonCSCU == 115)
                    {
                        Status.stFrameRemoteSingle.Stop_Result = 95;
                    }
                 }
             }
            if(stopreasontemp != Status.stFrameRemoteSingle.Stop_Result){
                Map.insert(Addr_CanID_Comm, QByteArray(1, canAddr));
                Map.insert(Addr_ChargeEndCode_Term, QByteArray(1, Status.stFrameRemoteSingle.Stop_Result));
                Map.insert(Addr_ChargeEndCode_Sudden, QByteArray(1, stopreasontemp));
                burstFlag |= BURST_FLAG_STOP_RESULT;
            }
            Status.stFrameRemoteSingle.Stop_Result = stopreasontemp;
        }else if( Key == Addr_CtrlModeFlag_Term ){//控制模式
            char oldValue = Status.stFrameRemoteSingle.QunLunCeLue & 0x0f;
            char newValue = tmpValue[0] & 0x0f;

            if(newValue != oldValue){
                Map.insert(Addr_CanID_Comm, QByteArray(1, canAddr));
                Map.insert(Addr_CtrlModeFlag_Term, QByteArray(1, oldValue));
                Map.insert(Addr_CtrlModeFlag_Sudden, QByteArray(1, newValue));
                burstFlag |= BURST_FLAG_CTRLMODE;
            }

            Status.stFrameRemoteSingle.QunLunCeLue &= 0xf0;
            Status.stFrameRemoteSingle.QunLunCeLue |= newValue;
        }else if( Key == Addr_GroupModeFlag_Term ){//群充策略
            char oldValue = (Status.stFrameRemoteSingle.QunLunCeLue & 0xf0) >> 4;
            char newValue = tmpValue[0];

            if(newValue != oldValue){
                Map.insert(Addr_CanID_Comm, QByteArray(1, canAddr));
                Map.insert(Addr_GroupModeFlag_Term, QByteArray(1, oldValue));
                Map.insert(Addr_GroupQueueFlag_Sudden, QByteArray(1, newValue));
                burstFlag |= BURST_FLAG_QUNLUNCELUE;
            }
            Status.stFrameRemoteSingle.QunLunCeLue &= 0x0f;
            Status.stFrameRemoteSingle.QunLunCeLue |= (newValue << 4);
        }else if( Key == Addr_AuxPowerType_Term ){
            if(tmpValue[0] != Status.stFrameRemoteSingle.AuxPowerType){
                Map.insert(Addr_CanID_Comm, QByteArray(1, canAddr));
                Map.insert(Addr_AuxPowerType_Term, QByteArray(1, Status.stFrameRemoteSingle.AuxPowerType));
                Map.insert(Addr_AuxPowerType_Sudden, QByteArray(1, tmpValue[0]));
                burstFlag |= BURST_FLAG_AUXPOWERTYPE;
            }
            Status.stFrameRemoteSingle.AuxPowerType = tmpValue[0];
        }else if( Key == Addr_SoftwareVer_Term){
            snprintf((char *)Status.psTermianlVer, sizeof(Status.psTermianlVer), "%s", (char *)tmpValue);
        }
    }

    devCache->FreeUpdateTerminalStatus();

    /*突发状态信息、记录状态变化*/
    if(burstFlag)
    {
        Map.insert(Addr_CanID_Comm, QByteArray((char *)&canAddr, 1));
        emit sigToBus(Map, AddrType_TermSignal);

        if(burstFlag & BURST_FLAG_LINK_STATUS ||
                burstFlag & BURST_FLAG_RELAY_STATUS ||
                burstFlag & BURST_FLAG_CHARGER_STATUS ||
                burstFlag & BURST_FLAG_STATUS_FAULT){
            //按照老流程，只有连接开关、继电器、充电机、整机故障等遥信变化进行保存
            devCache->SaveTerminalStatus(canAddr);
        }

        if(burstFlag & BURST_FLAG_LINK_STATUS)
            gunInsertPullRecord(oldLinkStatus, newLinkStatus, canAddr);
        if(burstFlag & BURST_FLAG_CHARGER_STATUS)
            onOffLineRecord(oldChargeStatus, newChargeStatus, canAddr);
    }

    /*处理故障*/
    if(dealFault)
        faultDeal(faultCode, canAddr);
}

/**
 *根据can地址查找相别参数
 */
int RealDataFilter::findPhaseType(unsigned char canAddr)
{
    QList<stPhaseTypeConfig> list = ThreePhaseTypeConfig.phaseTypeConfig;

    stPhaseTypeConfig phaseType;
    for(int i=0; i <list.size(); i++)
    {
        phaseType = list.at(i);
        if(phaseType.canaddr == canAddr)
            return phaseType.type;
    }

    return 0;
}
/**
 *遥测数据更新
 */
void RealDataFilter::updateTelemeterData(InfoMap TelemeterMap)
{
    char tmpValue[10];
    int burstFlag = 0;
    if(TelemeterMap.isEmpty())
        return;

    QMap<unsigned int, QByteArray>::iterator it;
    QByteArray Value;
    unsigned char canAddr = 0xff;

    Value = TelemeterMap.value(Addr_CanID_Comm);
    if(Value.size())
        canAddr = *((unsigned char *)Value.data());
    else
        return;

    TerminalStatus & Status = devCache->GetUpdateTerminalStatus(canAddr);
    if(Status.cCanAddr == 0xff)
        return;

    QString strTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
    memcpy(Status.lastActiveTime, strTime.toAscii().data(), strTime.length());

    for(it = TelemeterMap.begin(); it != TelemeterMap.end(); ++it)
    {
        if((unsigned int)it.value().size() > sizeof(tmpValue))
            continue;

        unsigned int Key = it.key();
        memcpy(tmpValue, it.value().data(), it.value().size());

        if(Key == Addr_AVoltage_Term)
            Status.stFrameRemoteMeSurement1.A_voltage = *((float *)tmpValue);
        if( Key == Addr_BVoltage_Term)
            Status.stFrameRemoteMeSurement1.B_voltage = *((float *)tmpValue);
        else if(Key == Addr_CVoltage_Term)
            Status.stFrameRemoteMeSurement1.C_voltage = *((float *)tmpValue);
        else if(Key == Addr_ACurrent_Term)
            Status.stFrameRemoteMeSurement1.A_current = *((float *)tmpValue);
        else if(Key == Addr_BCurrent_Term)
            Status.stFrameRemoteMeSurement1.B_current = *((float *)tmpValue);
        else if(Key == Addr_CCurrent_Term)
            Status.stFrameRemoteMeSurement1.C_current = *((float *)tmpValue);
        else if(Key == Addr_TotalActivePower_Term){
            Status.stFrameRemoteMeSurement1.active_power = *((float *)tmpValue);
            if(m_iMeterType == 1 && Status.cCanAddr <= ID_MaxDCCanID && Status.cCanAddr >= ID_MinDCCanID){
                m_fActivePower[Status.cCanAddr - ID_MinDCCanID] = *((float *)tmpValue);
                AcMeter(Status);
            }
        }
        else if(Key == Addr_TotalReactivePower_Term)
            Status.stFrameRemoteMeSurement1.reactive_power = *((float *)tmpValue);
        else if(Key == Addr_TotalPowerFactor_Term)
            Status.stFrameRemoteMeSurement1.power_factor = *((float *)tmpValue);
        else if(Key == Addr_ZeroLineCurrent_Term)
            Status.stFrameRemoteMeSurement1.neutralLine_current = *((float *)tmpValue);
        else if(Key == Addr_VoltageUnbalanceRate_Term)
            Status.stFrameRemoteMeSurement1.voltage_unbalance_rate = *((float *)tmpValue);
        else if(Key == Addr_CurrentUnbalanceRate_Term)
            Status.stFrameRemoteMeSurement1.current_unbalance_rate = *((float *)tmpValue);
        else if(Key == Addr_DCChargeVoltage_Term){
            Status.stFrameRemoteMeSurement1.voltage_of_dc = *((float *)tmpValue);
            AcMeter(Status);
        }
        else if(Key == Addr_DCChargeCurrent_Term){
            Status.stFrameRemoteMeSurement1.current_of_dc = *((float *)tmpValue);
            AcMeter(Status);
        }
        else if(Key == Addr_TotalActiveEnergy_Term)
        {
            Status.validFlag |= 0x08;
            uint uiNowEnergy = *((uint *)tmpValue);
            if(energyFilterFlag == 1){
                if(activeEnergyCheck(Status, uiNowEnergy))
                    burstFlag |=0x01;
            }
			Status.stFrameRemoteMeSurement2.active_electric_energy = uiNowEnergy;

			if(m_iMeterType == 1 && Status.cCanAddr <= ID_MaxDCCanID && Status.cCanAddr >= ID_MinDCCanID){
				m_fNowEnergy[Status.cCanAddr - ID_MinDCCanID] = (double)uiNowEnergy / 100.0;
				AcMeter(Status);
			}
        }
        else if(Key == Addr_TotalReactiveEnergy_Term)
            Status.stFrameRemoteMeSurement2.reactive_electric_energy = *((uint *)tmpValue);
        else if(Key == Addr_TotalReverseActiveEnergy_Term)
            Status.stFrameRemoteMeSurement2.ReverseActiveEnergy = *((uint *)tmpValue);
        else if(Key == Addr_TotalReverseReactiveEnergy_Term)
            Status.stFrameRemoteMeSurement2.ReverseReactiveEnergy = *((uint *)tmpValue);
        else if(Key == Addr_ACSinVoltage_Term)		//根据相别处理
        {
            int ret = findPhaseType(canAddr);
            Status.stFrameRemoteMeSurement1.A_voltage = 0;
            Status.stFrameRemoteMeSurement1.B_voltage = 0; //nihai modify,单项时，其余两相数据为0xffff表示无效
            Status.stFrameRemoteMeSurement1.C_voltage = 0;
            if((ret == 1) || (ret == 0) )
            {
                Status.stFrameRemoteMeSurement1.A_voltage = *((float *)tmpValue);
            }
            else if(ret == 2)
            {
                Status.stFrameRemoteMeSurement1.B_voltage = *((float *)tmpValue);
            }
            else if(ret == 3)
            {
                Status.stFrameRemoteMeSurement1.C_voltage = *((float *)tmpValue);
            }
        }
        else if(Key == Addr_ACSinCurrent_Term)
        {
            int ret = findPhaseType(canAddr);
            Status.stFrameRemoteMeSurement1.A_current = 0;
            Status.stFrameRemoteMeSurement1.B_current = 0; //nihai modify 20170528,单项交流时，其余亮相显示为非法值0xffff
            Status.stFrameRemoteMeSurement1.C_current = 0;
            if((ret == 1) || (ret == 0))
            {
                Status.stFrameRemoteMeSurement1.A_current = *((float *)tmpValue);
            }
            else if(ret == 2)
            {
                Status.stFrameRemoteMeSurement1.B_current = *((float *)tmpValue);
            }
            else if(ret == 3)
            {
                Status.stFrameRemoteMeSurement1.C_current = *((float *)tmpValue);
            }
        }else if(Key == Addr_Qiang_Temp )
        {
            Status.Qiangtoutemp = *((float*)tmpValue);
        }
    }

    devCache->FreeUpdateTerminalStatus();
    if(burstFlag & 0x01){
            //如果有电量上传而且开启了异常电镀数过滤则保存电量
            devCache->SaveTerminalStatus(canAddr);
    }
}
/**
 *更新bms信息
 */
void RealDataFilter::updateBMSData(InfoMap BMSMap)
{
    char tmpValue[20] = {0};
    if(BMSMap.isEmpty())
    {
        return;
    }

    QMap<unsigned int, QByteArray>::iterator it;
    QByteArray Value;
    unsigned char canAddr = 0xff;

    Value = BMSMap.value(Addr_CanID_Comm);
    if(Value.size())
        canAddr = *((unsigned char *)Value.data());
    else
        return;


    TerminalStatus & Status = devCache->GetUpdateTerminalStatus(canAddr);
    if(Status.cCanAddr == 0xff)
        return;

    QString strTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
    memcpy(Status.lastActiveTime, strTime.toAscii().data(), strTime.length());

    for(it = BMSMap.begin(); it != BMSMap.end(); ++it)
    {
        if((unsigned int)it.value().size() > sizeof(tmpValue))
            continue;

        int size = it.value().size();
        unsigned int Key = it.key();
        memcpy(tmpValue, it.value().data(), it.value().size());

        if(Key == Addr_NeedVoltage_BMS)
            Status.stFrameBmsInfo.BMS_need_voltage = *((float *)tmpValue);				//BMS需求电压
        else if(Key == Addr_NeedCurrent_BMS)
            Status.stFrameBmsInfo.BMS_need_current = *((float *)tmpValue);				//BMS需求电流
        else if(Key == Addr_NowSOC_BMS)
        {
            if(*((unsigned char *)tmpValue) !=0)
                Status.stFrameBmsInfo.batery_SOC = *((unsigned char *)tmpValue);			//当前SOC
        }
        else if(Key == Addr_MaxBatteryTemp_BMS)
            Status.stFrameBmsInfo.max_batery_temperature = *((short *)tmpValue);        //最高电池温度
        else if(Key == Addr_MaxSingleBatteryVoltage_BMS)
            Status.stFrameBmsInfo.max_batery_voltage = *((float *)tmpValue);			//最高电池电压
        else if(Key == Addr_MinBatteryTemp_BMS)
            Status.stFrameBmsInfo.lowest_battery_temperature = *((short *)tmpValue);	//最低电池温度
        else if(Key == Addr_MinSingleVoltage_BMS)
            Status.stFrameBmsInfo.lowest_charge_voltage = *((float *)tmpValue);			//最低电池电压
        else if(Key == Addr_BatteryVIN_BMS)
        {
            InfoMap map;
            if(vinFlag == 1)
            {
                memcpy(Status.stFrameBmsInfo.BMS_car_VIN, tmpValue, size);
                map.insert(Addr_CanID_Comm, QByteArray((char *)&canAddr, 1));
                map.insert(Addr_BatteryVIN_BMS, QByteArray((char *)tmpValue, size));

                emit sigToBus(map, AddrType_VinNum);
            }
            else if(vinFlag == 2)
            {
                memcpy(Status.stFrameBmsInfo.car_license_plate, tmpValue+7, 7);
                map.insert(Addr_CanID_Comm, QByteArray((char *)&canAddr, 1));
                map.insert(Addr_CarLicense, QByteArray((char *)tmpValue+7, 7));

                emit sigToBus(map, AddrType_CarLicence);
            }else if(vinFlag == 3){
                int iRet = AutoDetect(tmpValue, size);
                if(iRet == 1){
                    memcpy(Status.stFrameBmsInfo.BMS_car_VIN, tmpValue, size);
                    map.insert(Addr_CanID_Comm, QByteArray((char *)&canAddr, 1));
                    map.insert(Addr_BatteryVIN_BMS, QByteArray((char *)tmpValue, size));
                    emit sigToBus(map, AddrType_VinNum);
                }else if(iRet == 2){
                    memcpy(Status.stFrameBmsInfo.car_license_plate, tmpValue + 7, 7);
                    map.insert(Addr_CanID_Comm, QByteArray((char *)&canAddr, 1));
                    map.insert(Addr_CarLicense, QByteArray((char *)tmpValue + 7, 7));
                    emit sigToBus(map, AddrType_CarLicence);
                }else{
                }
            }
        }
        else if(Key == Addr_ChargeType_BMS)//充电模式
            Status.stFrameBmsInfo.ChargeType =  tmpValue[0];
        else if(Key == Addr_ChargeVoltageMeasured_BMS)//充电电压测量值
            Status.stFrameBmsInfo.ChargeVoltageMeasured = *((float *)tmpValue);
        else if(Key == Addr_ChargeCurrentMeasured_BMS)//充电电流测量值
            Status.stFrameBmsInfo.ChargeCurrentMeasured = *((float *)tmpValue);
        // else if(Key == Addr_MaxSingleBatteryVoltageSerial_BMS)//最高单体动力蓄电池电压所在组号
        else if(Key==Addr_MaxSingleBatterySerialNum_BMS)  //最高单体电池组号
            Status.stFrameBmsInfo.MaxSingleBatteryVoltageSerial = *((short *)tmpValue);
        // else if(Key == Addr_MaxSingleBatterySerialNum_BMS)     //nihai modify 20170523
        //    Status.stFrameBmsInfo.MaxSingleBatterySerialNum= tmpValue[0];
        else if(Key == Addr_LeftTime)//估算剩余充电时间（min）
            Status.stFrameBmsInfo.LeftTime = *((short *)tmpValue);
        else if(Key == Addr_SingleBatteryNum_BMS)//最高单体动力蓄电池电压所在编号
            Status.stFrameBmsInfo.SingleBatteryNum = tmpValue[0];
        else if(Key == Addr_MaxTempPointNum_BMS)//最高温度检测点编号
            Status.stFrameBmsInfo.MaxTempPointNum = tmpValue[0];
        else if(Key == Addr_MinTempPointNum_BMS)//最低动力蓄电池温度检测点编号
            Status.stFrameBmsInfo.MaxTempPointNum = tmpValue[0];
        else if(Key == Addr_ChargePermitFlag_BMS)//BMS充电允许标志
            Status.stFrameBmsInfo.ChargePermitFlag = tmpValue[0];
        //握手阶段
        else if(Key == Addr_BMSProtocolVer_BMS)//BMS协议版本号
        {
            strncpy(Status.stFrameBmsHand.BMSProtocolVer, tmpValue, sizeof(Status.stFrameBmsHand.BMSProtocolVer));
        }
        else if(Key == Addr_MaxAllowedVoltage_BMS)//最高允许充电总电压
        {
            Status.stFrameBmsHand.MaxAllowedVoltage = *((float *)tmpValue);
        }
        else if(Key == Addr_BatteryType_BMS)//电池类型
        {
            Status.stFrameBmsHand.BatteryType = *((unsigned char *)tmpValue);
        }
        else if(Key == Addr_BatteryRatedCapacity_BMS)//整车动力蓄电池系统额定容量（AH）
        {
            Status.stFrameBmsHand.BatteryRatedCapacity = *((float *)tmpValue);

        }
        else if(Key == Addr_BatteryRatedVoltage_BMS)//整车动力蓄电池额定总电压
        {
            Status.stFrameBmsHand.BatteryRatedVoltage = *((float *)tmpValue);

        }
        else if(Key == Addr_BatteryManufacturer_BMS)//电池生产厂商名称
        {
            strncpy(Status.stFrameBmsHand.BatteryManufacturer, tmpValue, sizeof(Status.stFrameBmsHand.BatteryManufacturer));

        }
        else if(Key == Addr_BatterySerialNum_BMS)//电池组序号
        {
            Status.stFrameBmsHand.BatterySerialNum = *(int *)tmpValue;

        }
        else if(Key == Addr_BatteryProduceDate_BMS)//电池组生产日期
        {
            strncpy(Status.stFrameBmsHand.BatteryProduceDate, tmpValue, sizeof(Status.stFrameBmsHand.BatteryProduceDate));

        }
        else if(Key == Addr_BatteryChargeTime_BMS)//电池组充电次数
        {
            Status.stFrameBmsHand.BatteryChargeTime = *((int *)tmpValue);

        }
        else if(Key == Addr_BatteryOwnerFlag_BMS)//电池组产权标识
        {
            Status.stFrameBmsHand.BatteryOwnerFlag = tmpValue[0];

        }
        else if(Key == Addr_BMSSoftwareVer_BMS)//BMS软件版本号
        {
            strncpy(Status.stFrameBmsHand.BMSSoftwareVer,tmpValue, sizeof(Status.stFrameBmsHand.BMSSoftwareVer));
        }
        //参数配置阶段
        else if(Key == Addr_SingleBatteryMaxAllowedVoltage_BMS)//单体动力蓄电池最高允许充电电压
            Status.stFrameBmsParam.SingleBatteryMaxAllowedVoltage = *((float *)tmpValue);
        else if(Key == Addr_MaxAllowedCurrent_BMS)//最高允许充电电流
            Status.stFrameBmsParam.MaxAllowedCurrent = *((float *)tmpValue);
        else if(Key == Addr_BatteryTotalEnergy_BMS)//动力蓄电池标称总能
            Status.stFrameBmsParam.BatteryTotalEnergy = *((float *)tmpValue);
        else if(Key == Addr_MaxParamAllowedVoltage_BMS)//最高允许充电总
            Status.stFrameBmsParam.MaxParamAllowedVoltage = *((float *)tmpValue);
        else if(Key == Addr_MaxtAllowedTemp_BMS)//最高允许温度
            Status.stFrameBmsParam.MaxtAllowedTemp = *((float *)tmpValue);
        else if(Key == Addr_ParamSOC_BMS)//整车动力蓄电池荷电状态（SOC）(参数配置阶段)
            Status.stFrameBmsParam.ParamSOC = *((float *)tmpValue);
        else if(Key == Addr_BatteryVoltage_BMS)//整车动力蓄电池当前电池电压
            Status.stFrameBmsParam.BatteryVoltage = *((float *)tmpValue);
        else if(Key == Addr_MaxOutputVoltage_BMS)//最大输出电压
            Status.stFrameBmsParam.MaxOutputVoltage = *((float *)tmpValue);
        else if(Key == Addr_MinOutputVoltage_BMS)//最小输出电压
            Status.stFrameBmsParam.MinOutputVoltage = *((float *)tmpValue);
        else if(Key == Addr_MaxOutputCurrent_BMS)//最大输出电流
            Status.stFrameBmsParam.MaxOutputCurrent = *((float *)tmpValue);
        else if(Key == Addr_MinOutputCurrent_BMS)//最小输出电流
            Status.stFrameBmsParam.MinOutputCurrent = *((float *)tmpValue);
        //BMS中止充电阶段
        else if(Key == Addr_BMSStopReason_BMS)//BMS中止充电原因
            Status.stFrameBmsChargeTerm.BMSStopReason = *(int *)tmpValue;
        else if(Key == Addr_BMSFaultReason_BMS)//BMS中止充电故障原因
            Status.stFrameBmsChargeTerm.BMSFaultReason = *(int *)tmpValue;
        else if(Key == Addr_BMSErrorReason_BMS)//BMS中止充电错误原因
            Status.stFrameBmsChargeTerm.BMSErrorReason = *(int *)tmpValue;
        else if(Key == Addr_ChargerStopReason_BMS)//充电机中止充电原因
            Status.stFrameBmsChargeTerm.ChargerStopReason = *(int *)tmpValue;
        else if(Key == Addr_ChargerFaultReason_BMS)//充电机中止充电故障原因
            Status.stFrameBmsChargeTerm.ChargerFaultReason = *(int *)tmpValue;
        else if(Key == Addr_ChargerErrorReason_BMS)//充电机中止充电错误原因
            Status.stFrameBmsChargeTerm.ChargerErrorReason = *(int *)tmpValue;
        else if(Key == Addr_ChargeEndSOC_BMS)//中止荷电状态SOC（%）
            Status.stFrameBmsChargeTerm.ChargeEndSOC = *(float *)tmpValue;
        else if(Key == Addr_MinSingleVoltage_BMS_END)//动力蓄电池单体最低电压
            Status.stFrameBmsChargeTerm.MinSingleVoltage = *((float *)tmpValue);
        else if(Key == Addr_MaxSingleVoltage_BMS)//动力蓄电池单体最高电压
            Status.stFrameBmsChargeTerm.MaxSingleVoltage = *((float *)tmpValue);
        else if(Key == Addr_MinTemp_BMS)//动力蓄电池最低温度
            Status.stFrameBmsChargeTerm.MinTemp = *((float *)tmpValue);
        else if(Key == Addr_MaxTemp_BMS)//动力蓄电池最高温度
            Status.stFrameBmsChargeTerm.MaxTemp = *((float *)tmpValue);
        else if(Key == Addr_BMSErrorFrame_BMS)//BMS错误报文
            Status.stFrameBmsChargeTerm.BMSErrorFrame = *(unsigned int *)tmpValue;
        else if(Key == Addr_ChargerErrorFrame_BMS)//充电机错误报文
            Status.stFrameBmsChargeTerm.ChargerErrorFrame = *(unsigned int *)tmpValue;

        memset(tmpValue, 0x00, sizeof(tmpValue));
    }
    devCache->FreeUpdateTerminalStatus();
}
/**
 *进线侧电表数据更新
 */
void RealDataFilter::updateAmmeterData(InfoMap TelemeterMap)
{
    char tmpValue[10];
    float oldPower = 0;
    if(TelemeterMap.isEmpty())
        return;

    QByteArray Value = TelemeterMap.value(Addr_Ammeter_ID);		//电表地址
    if(Value.size() != 6)
        return;

    RealStatusMeterData &Status = devCache->GetUpdateRealStatusMeter();
    stAmmeterData &inLineAmmeter = Status.inLineAmmeter.ammeterData.operator[](Value);

    QMap<unsigned int, QByteArray>::iterator it;
    for(it = TelemeterMap.begin(); it != TelemeterMap.end(); ++it)
    {
        if((unsigned int)it.value().size() > sizeof(tmpValue))
            continue;

        unsigned int key = it.key();
        memcpy(tmpValue, it.value().data(), it.value().size());

        if(key==Addr_Ammeter_ID)
            memcpy(inLineAmmeter.addr,tmpValue,6);
        else if(key == Addr_Vol_A_Term)
            inLineAmmeter.Vol_A = *((float *)(tmpValue));
        else if(key == Addr_Vol_B_Term)
            inLineAmmeter.Vol_B = *((float *)(tmpValue));
        else if(key ==	Addr_Vol_C_Term)
            inLineAmmeter.Vol_C = *((float *)(tmpValue));
        else if(key ==	Addr_Cur_A_Term)
            inLineAmmeter.Cur_A = *((float *)(tmpValue));
        else if(key ==	Addr_Cur_B_Term)
            inLineAmmeter.Cur_B = *((float *)(tmpValue));
        else if(key ==	Addr_Cur_C_Term)
            inLineAmmeter.Cur_C = *((float *)(tmpValue));
        else if(key ==	Addr_Power_Term)
        {
            oldPower = inLineAmmeter.TotalPower;
            inLineAmmeter.TotalPower = *((float *)(tmpValue));
        }
        else if(key ==	Addr_rePower_Term)
            inLineAmmeter.TotalRePower = *((float *)(tmpValue));
        else if(key ==	Addr_PowerFactor_Term)
            inLineAmmeter.PowerFactor = *((float *)(tmpValue));
        else if(key ==	Addr_Cur_0_Term)
            inLineAmmeter.Cur_0= *((float *)(tmpValue));
        else if(key ==	Addr_Vol_unbalance_Term)
            inLineAmmeter.VolUnbalance = *((float *)(tmpValue));
        else if(key ==	Addr_Cur_unbalance_Term)
            inLineAmmeter.CurUnbalance = *((float *)(tmpValue));
        else if(key == Addr_harm_distortion_Term)
            inLineAmmeter.HarmDistortion = *((float *)(tmpValue));
        else if(key ==	Addr_Power_A_Term)
            inLineAmmeter.Power_A = *((float *)(tmpValue));
        else if(key ==	Addr_Power_B_Term)
            inLineAmmeter.Power_B = *((float *)(tmpValue));
        else if(key ==	Addr_Power_C_Term)
            inLineAmmeter.Power_C = *((float *)(tmpValue));
        else if(key ==	Addr_rePower_A_Term)
            inLineAmmeter.RePower_A = *((float *)(tmpValue));
        else if(key ==	Addr_rePower_B_Term)
            inLineAmmeter.RePower_B = *((float *)(tmpValue));
        else if(key ==	Addr_rePower_C_Term)
            inLineAmmeter.RePower_C = *((float *)(tmpValue));
        else if(key ==	Addr_PowerFactor_A_Term)
            inLineAmmeter.PowerFactor_A = *((float *)(tmpValue));
        else if(key ==	Addr_PowerFactor_B_Term)
            inLineAmmeter.PowerFactor_B = *((float *)(tmpValue));
        else if(key ==	Addr_PowerFactor_C_Term)
            inLineAmmeter.PowerFactor_C = *((float *)(tmpValue));
        else if(key ==	Addr_active_absorb_energy_Term)
            inLineAmmeter.ActiveAbsorbEnergy = *((float *)(tmpValue));
        else if(key ==	Addr_active_liberate_energy_Term)
            inLineAmmeter.ActiveLiberateEnergy = *((float *)(tmpValue));
        else if(key ==	Addr_reactive_sensibility_energy_Term)
            inLineAmmeter.ReactiveSensibilityEnergy = *((float *)(tmpValue));
        else if(key ==	Addr_reactive_capacity_energy_Term)
            inLineAmmeter.ReactiveCapacityEnergy = *((float *)(tmpValue));
    }
    //突发有功功率
    if(oldPower != inLineAmmeter.TotalPower)
    {
        InfoMap tempMap;
        QByteArray tempArray;
        //电表ID
        tempArray.append((char *)inLineAmmeter.addr, 6);
        tempMap.insert(Addr_Ammeter_ID, tempArray);
        tempArray.clear();
        //功率数值
        tempArray.append((char *)&inLineAmmeter.TotalPower, 4);
        tempMap.insert(Addr_Power_Term, tempArray);
        tempArray.clear();

        emit sigToBus(tempMap,AddrType_EnergyPlan_Signal_Burst);
    }

    devCache->FreeUpdateRealStatusMeter();
}

/**
 *储能柜数据更新
 */
void RealDataFilter::updateEnergyStorageCabinetInfo(InfoMap centerMap)
{
    char tmpValue[10];
    unsigned char canAddr;
    QByteArray Value;
    if(centerMap.isEmpty())
        return;

    Value = centerMap.value(Addr_CanID_Comm);		//柜子地址
    if(Value.size())
        canAddr = *((unsigned char *)Value.data());
    else
        return;

    stEnergyPlanDatas &Status = devCache->GetUpdateEnergyPlanMeter();
    if(!Status.esCabMap.contains(canAddr))
    {
        InfoMap tempMap;
        TDevice device;
        QByteArray array;
        memset(&device, 0x00, sizeof(device));
        device.enDeviceType = DEV_STORAGE_CABINET;
        device.enSubDeviceType = SUB_DEV_UNKNOWN;
        device.iCanID = canAddr;
        device.iParentID = ID_DefaultControlCenterCanID;
        array.append((char *)&device, sizeof(device));
        tempMap.insert(Addr_EnergyPlan_Dev,array);
        emit sigToBus(tempMap, AddrType_EnergyPlanDevChange);
    }
    stEnergyStorageCabinetInfo &esCabData = Status.esCabMap.operator [](canAddr);
    QMap<unsigned int, QByteArray>::iterator it;

    for(it = centerMap.begin(); it != centerMap.end(); ++it)
    {
        if((unsigned int)it.value().size() > sizeof(tmpValue))
            continue;

        unsigned int key = it.key();
        memcpy(tmpValue, it.value().data(), it.value().size());

        switch(key)
        {
        case Addr_mainBreaker_EnergyStore:
            esCabData.mainBreaker = *((char *)(&tmpValue));
            break;
        case Addr_slaveBreaker_EnergyStore:
            esCabData.slaveBreaker = *((char *)(&tmpValue));
            break;
        case Addr_DCBreaker1_EnergyStore:
            esCabData.DCBreaker1 = *((char *)(&tmpValue));
            break;
        case Addr_DCBreaker2_EnergyStore:
            esCabData.DCBreaker2 = *((char *)(&tmpValue));
            break;
        case Addr_fireExtinguisherStatus_EnergyStore:
            esCabData.fireExtinguisher_1 = *((char *)(&tmpValue));
            break;
        case Addr_DCBreaker3_EnergyStore:
            esCabData.DCBreaker3 = *((char *)(&tmpValue));
            break;
        case Addr_DCBreaker4_EnergyStore:
            esCabData.DCBreaker4 = *((char *)(&tmpValue));
            break;
        case Addr_travelSwitch_EnergyStore:
            esCabData.tripFeedback = *((char *)(&tmpValue));
            break;
        case Addr_fireExtinguisher_EnergyStore:
            esCabData.fireExtinguisher_2 = *((char *)(&tmpValue));
            break;
        case Addr_founder_EnergyStore:
            esCabData.waterIn = *((char *)(&tmpValue));
            break;
        default:
            break;
        }
    }
    devCache->FreeUpdateEnergyPlantMeter();
}

/**
 *储能电池数据更新
 */
void RealDataFilter::updateEnergyStorageCabinetBatteryInfo(InfoMap centerMap)
{
    char tmpValue[10];
    unsigned char canAddr;
    QByteArray Value;
    if(centerMap.isEmpty())
        return;

    Value = centerMap.value(Addr_CanID_Comm);		//柜子地址
    if(Value.size())
        canAddr = *((unsigned char *)Value.data());
    else
        return;

    stEnergyPlanDatas &Status = devCache->GetUpdateEnergyPlanMeter();
    if(!Status.esBatMap.contains(canAddr))
    {
        InfoMap tempMap;
        TDevice device;
        QByteArray array;
        memset(&device, 0x00, sizeof(device));
        device.enDeviceType = DEV_STORAGE_BATTERY;
        device.enSubDeviceType = SUB_DEV_UNKNOWN;
        device.iCanID = canAddr * 1000 + canAddr;
        device.iParentID = canAddr;
        array.append((char *)&device, sizeof(device));
        tempMap.insert(Addr_EnergyPlan_Dev,array);
        emit sigToBus(tempMap, AddrType_EnergyPlanDevChange);
    }
    stEnergyStorageBatteryInfo &esBatData = Status.esBatMap.operator [](canAddr);
    QMap<unsigned int, QByteArray>::iterator it;

    for(it = centerMap.begin(); it != centerMap.end(); ++it)
    {
        if((unsigned int)it.value().size() > sizeof(tmpValue))
            continue;

        unsigned int key = it.key();
        memcpy(tmpValue, it.value().data(), it.value().size());

        switch(key)
        {
        case Addr_B2C_STATUS_CRC:
            esBatData.B2C_STATUS.CRC = *((char *)(&tmpValue));
            break;
        case Addr_B2C_STATUS_bmsHeartBeat:
            esBatData.B2C_STATUS.bmsHeartBeat = *((char *)(&tmpValue));
            break;
        case Addr_B2C_STATUS_test:
            esBatData.B2C_STATUS.test = *((char *)(&tmpValue));
            break;
        case Addr_B2C_STATUS_tankSwitch:
            esBatData.B2C_STATUS.tankSwitch = *((char *)(&tmpValue));
            break;
        case Addr_B2C_STATUS_singleOverVolAlarm:
            esBatData.B2C_STATUS.singleOverVolAlarm = *((char *)(&tmpValue));
            break;
        case Addr_B2C_STATUS_singleLowVolAlarm:
            esBatData.B2C_STATUS.singleLowVolAlarm = *((char *)(&tmpValue));
            break;
        case Addr_B2C_STATUS_OverTempAlarm:
            esBatData.B2C_STATUS.OverTempAlarm = *((char *)(&tmpValue));
            break;
        case Addr_B2C_STATUS_BelowTempAlarm:
            esBatData.B2C_STATUS.BelowTempAlarm = *((char *)(&tmpValue));
        case Addr_B2C_STATUS_insulationAlarm:
            esBatData.B2C_STATUS.insulationAlarm = *((char *)(&tmpValue));
            break;
        case Addr_B2C_STATUS_BMScommuFault:
            esBatData.B2C_STATUS.BMScommuFault = *((char *)(&tmpValue));
            break;
        case Addr_B2C_STATUS_BMScontrolPower:
            esBatData.B2C_STATUS.BMScontrolPower = *((char *)(&tmpValue));
            break;
        case Addr_B2C_STATUS_BMSfullPowerON:
            esBatData.B2C_STATUS.BMSfullPowerON = *((char *)(&tmpValue));
            break;
        case Addr_B2C_STATUS_BMSsysStatus:
            esBatData.B2C_STATUS.BMSsysStatus = *((char *)(&tmpValue));
            break;
        case Addr_B2C_STATUS_ESSfullEnergy:
            esBatData.B2C_STATUS.ESSfullEnergy = *((char *)(&tmpValue));
            break;
        case Addr_B2C_STATUS_ESSfullDisCharge:
            esBatData.B2C_STATUS.ESSfullDisCharge = *((char *)(&tmpValue));
            break;
        case Addr_B2C_STATUS_ApplyACInfo:
            esBatData.B2C_STATUS.ApplyACInfo = *((char *)(&tmpValue));
            break;
        case Addr_B2C_STATUS_ApplySysInfo:
            esBatData.B2C_STATUS.ApplySysInfo = *((char *)(&tmpValue));
            break;
        case Addr_B2C_STATUS_SOC:
            esBatData.B2C_STATUS.SOC = *((float *)(&tmpValue));
            break;

        case Addr_B2C_SUMDATA1_CRC:
            esBatData.B2C_SUMDATA1.CRC = *((char *)(&tmpValue));
            break;
        case Addr_B2C_SUMDATA1_tankNum:
            esBatData.B2C_SUMDATA1.tankNum = *((char *)(&tmpValue));
            break;
        case Addr_B2C_SUMDATA1_BMShighVol:
            esBatData.B2C_SUMDATA1.BMShighVol = *((float *)(&tmpValue));
            break;
        case Addr_B2C_SUMDATA1_BMScur:
            esBatData.B2C_SUMDATA1.BMScur = *((float *)(&tmpValue));
            break;
        case Addr_B2C_SUMDATA2_CRC:
            esBatData.B2C_SUMDATA2.CRC = *((char *)(&tmpValue));
            break;
        case Addr_B2C_SUMDATA2_BMSchargeEnergy:
            esBatData.B2C_SUMDATA2.BMSchargeEnergy = *((float *)(&tmpValue));
            break;
        case Addr_B2C_SUMDATA2_BMSdisChargeEnergy:
            esBatData.B2C_SUMDATA2.BMSdisChargeEnergy = *((float *)(&tmpValue));
            break;
        case Addr_B2C_SUMDATA2_SOH:
            esBatData.B2C_SUMDATA2.SOH = *((char *)(&tmpValue));
            break;
        case Addr_B2C_SUMDATA3_sysHumidity:
            esBatData.B2C_SUMDATA3.sysHumidity = *((char *)(&tmpValue));
            break;
        case Addr_B2C_SUMDATA3_singleMaxVol:
            esBatData.B2C_SUMDATA3.singleMaxVol = *((float *)(&tmpValue));
            break;
        case Addr_B2C_SUMDATA3_singleMinVol:
            esBatData.B2C_SUMDATA3.singleMinVol = *((float *)(&tmpValue));
            break;
        case Addr_B2C_SUMDATA3_singleMaxTem:
            esBatData.B2C_SUMDATA3.singleMaxTem = *((float *)(&tmpValue));
            break;
        case Addr_B2C_SUMDATA3_singleMinTem:
            esBatData.B2C_SUMDATA3.singleMinTem = *((float *)(&tmpValue));
            break;
        case Addr_B2C_SUMDATA3_sysTemp:
            esBatData.B2C_SUMDATA3.sysTemp = *((float *)(&tmpValue));
            break;
        case Addr_B2C_LIMIT_BMSlimitDischargeCur:
            esBatData.B2C_LIMIT.BMSlimitDischargeCur = *((float *)(&tmpValue));
            break;
        case Addr_B2C_LIMIT_BMSlimitChargeCur:
            esBatData.B2C_LIMIT.BMSlimitChargeCur = *((float *)(&tmpValue));
            break;
        case Addr_B2C_LIMIT_BMSlimitChargeVol:
            esBatData.B2C_LIMIT.BMSlimitChargeVol = *((float *)(&tmpValue));
            break;
        case Addr_B2C_LIMIT_BMSlimitDisChargeVol:
            esBatData.B2C_LIMIT.BMSlimitDisChargeVol = *((float *)(&tmpValue));
            break;
        case Addr_BatteryOutVol:
            esBatData.outVol = *((float *)(&tmpValue));
            break;
        case Addr_BatteryFuseVol:
            esBatData.fuseVol = *((float *)(&tmpValue));
            break;
        case Addr_BatteryBreakVol:
            esBatData.breakerVol = *((float *)(&tmpValue));
            break;
        case Addr_BatteryCur:
            esBatData.cur = *((float *)(&tmpValue));
            break;
        case Addr_BatteryDcVol:
            esBatData.dcVol = *((float *)(&tmpValue));
            break;
        case Addr_BatteryDcCur:
            esBatData.dcCur = *((float *)(&tmpValue));
            break;
        case Addr_BatteryDcPower:
            esBatData.dcPower = *((float *)(&tmpValue));
            break;
        case Addr_BatteryDcPositiveEnergy:
            esBatData.dcPositiveEnergy = *((float *)(&tmpValue));
            break;
        case Addr_BatteryDcDisPositiveEnergy:
            esBatData.dcDisPositiveEnergy = *((float *)(&tmpValue));
            break;
        case Addr_BatteryDcPT:
            esBatData.dcPT = *((unsigned short *)(&tmpValue));
            break;
        case Addr_BatteryDcCT:
            esBatData.dcCT = *((unsigned short *)(&tmpValue));
            break;
        default:
            break;
        }
    }
    devCache->FreeUpdateEnergyPlantMeter();
}


/**
 *光伏柜数据更新
 */
void RealDataFilter::updatePhotoVoltaicCabinetInfo(InfoMap centerMap)
{
    char tmpValue[10];
    unsigned char canAddr;
    QByteArray Value;
    if(centerMap.isEmpty())
        return;

    Value = centerMap.value(Addr_CanID_Comm);		//柜子地址
    if(Value.size())
        canAddr = *((unsigned char *)Value.data());
    else
        return;

    stEnergyPlanDatas &Status = devCache->GetUpdateEnergyPlanMeter();
    if(!Status.phCabMap.contains(canAddr))
    {
        InfoMap tempMap;
        TDevice device;
        QByteArray array;
        memset(&device, 0x00, sizeof(device));
        device.enDeviceType = DEV_PV_CONTROL_CABINET;
        device.enSubDeviceType = SUB_DEV_UNKNOWN;
        device.iCanID = canAddr;
        device.iParentID = ID_DefaultControlCenterCanID;
        array.append((char *)&device, sizeof(device));
        tempMap.insert(Addr_EnergyPlan_Dev,array);
        emit sigToBus(tempMap, AddrType_EnergyPlanDevChange);
    }
    stPhotoVoltaicCabinetInfo &phCabData = Status.phCabMap.operator [](canAddr);
    QMap<unsigned int, QByteArray>::iterator it;

    for(it = centerMap.begin(); it != centerMap.end(); ++it)
    {
        if((unsigned int)it.value().size() > sizeof(tmpValue))
            continue;

        unsigned int key = it.key();
        memcpy(tmpValue, it.value().data(), it.value().size());

        switch(key)
        {
        case Addr_fireExtinguisherVoltaicCabinet1:
            phCabData.smokeSensor = *((char *)(&tmpValue));
            break;
        case Addr_breaker1VoltaicCabinet1:
            phCabData.breaker1 = *((char *)(&tmpValue));
            break;
        case Addr_breaker2VoltaicCabinet1:
            phCabData.breaker2 = *((char *)(&tmpValue));
            break;
        case Addr_breaker3VoltaicCabinet1:
            phCabData.breaker3 = *((char *)(&tmpValue));
            break;
        case Addr_breaker4VoltaicCabinet1:
            phCabData.breaker4 = *((char *)(&tmpValue));
            break;
        case Addr_breaker5VoltaicCabinet1:
            phCabData.breaker5 = *((char *)(&tmpValue));
            break;
        case Addr_breaker6VoltaicCabinet1:
            phCabData.breaker6 = *((char *)(&tmpValue));
            break;
        case Addr_breaker7VoltaicCabinet1:
            phCabData.breaker7 = *((char *)(&tmpValue));
            break;
        default:
            break;
        }
    }
    devCache->FreeUpdateEnergyPlantMeter();
}

/**
 *系统控制柜数据更新
 */
void RealDataFilter::updateSysControlCabinetInfo(InfoMap centerMap)
{
    char tmpValue[10];
    unsigned char canAddr;
    QByteArray Value;
    if(centerMap.isEmpty())
        return;

    Value = centerMap.value(Addr_CanID_Comm);		//柜子地址
    if(Value.size())
        canAddr = *((unsigned char *)Value.data());
    else
        return;

    stEnergyPlanDatas &Status = devCache->GetUpdateEnergyPlanMeter();
    if(!Status.scCabMap.contains(canAddr))
    {
        InfoMap tempMap;
        TDevice device;
        QByteArray array;
        memset(&device, 0x00, sizeof(device));
        device.enDeviceType = DEV_SYSTEM_CONTROL_CAB;
        device.enSubDeviceType = SUB_DEV_UNKNOWN;
        device.iCanID = canAddr;
        device.iParentID = ID_DefaultControlCenterCanID;
        array.append((char *)&device, sizeof(device));
        tempMap.insert(Addr_EnergyPlan_Dev,array);
        emit sigToBus(tempMap, AddrType_EnergyPlanDevChange);
    }
    stSystemControlCabinetInfo &scCabData = Status.scCabMap.operator [](canAddr);
    QMap<unsigned int, QByteArray>::iterator it;

    for(it = centerMap.begin(); it != centerMap.end(); ++it)
    {
        if((unsigned int)it.value().size() > sizeof(tmpValue))
            continue;

        unsigned int key = it.key();
        memcpy(tmpValue, it.value().data(), it.value().size());

        switch(key)
        {
        case Addr_inEmergencyStop:
            scCabData.centerContorlEmergencyStop = *((char *)(&tmpValue));
            break;
        case Addr_outEmergencyStop:
            scCabData.outEmergencyStop = *((char *)(&tmpValue));
            break;
        case Addr_lowVolTravelSwitch:
            scCabData.lowVolTravelSwitch = *((char *)(&tmpValue));
            break;
        case Addr_highVolTravelSwitch:
            scCabData.highVolTravelSwitch = *((char *)(&tmpValue));
            break;
        case Addr_dormTravelSwitch:
            scCabData.dormTravelSwitch = *((char *)(&tmpValue));
            break;
        case Addr_lowVolSmokeSensor:
            scCabData.lowVolSmokeSensor = *((char *)(&tmpValue));
            break;
        case Addr_highVolSmokeSensor:
            scCabData.highVolSmokeSensor = *((char *)(&tmpValue));
            break;
        case Addr_transformerSmokeSensor:
            scCabData.transformerSmokeSensor = *((char *)(&tmpValue));
            break;
        case Addr_transformerOverTemp:
            scCabData.transformerOverTemp = *((char *)(&tmpValue));
            break;
        case Addr_transformerTempControlerFault:
            scCabData.transformerTempControlerFault = *((char *)(&tmpValue));
            break;
        case Addr_waterIn_SysControl2:
            scCabData.waterIn = *((char *)(&tmpValue));
            break;
        default:
            break;
        }
    }
    devCache->FreeUpdateEnergyPlantMeter();
}

/**
 *ACDC四象限柜数据更新
 */
void RealDataFilter::updateFourQuadrantCabinetInfo(InfoMap centerMap)
{
    char tmpValue[10];
    unsigned char canAddr;
    QByteArray Value;
    if(centerMap.isEmpty())
        return;

    Value = centerMap.value(Addr_CanID_Comm);		//柜子地址
    if(Value.size())
        canAddr = *((unsigned char *)Value.data());
    else
        return;

    stEnergyPlanDatas &Status = devCache->GetUpdateEnergyPlanMeter();
    if(!Status.fqCabMap.contains(canAddr))
    {
        InfoMap tempMap;
        TDevice device;
        QByteArray array;
        memset(&device, 0x00, sizeof(device));
        device.enDeviceType = DEV_FOUR_QUADRANT_CHARGE_CAB;
        device.enSubDeviceType = SUB_DEV_UNKNOWN;
        device.iCanID = canAddr;
        device.iParentID = ID_DefaultControlCenterCanID;
        array.append((char *)&device, sizeof(device));
        tempMap.insert(Addr_EnergyPlan_Dev,array);
        emit sigToBus(tempMap, AddrType_EnergyPlanDevChange);
    }
    stFourQuadrantCabinetInfo &fqCabData = Status.fqCabMap.operator [](canAddr);
    QMap<unsigned int, QByteArray>::iterator it;

    for(it = centerMap.begin(); it != centerMap.end(); ++it)
    {
        if((unsigned int)it.value().size() > sizeof(tmpValue))
            continue;

        unsigned int key = it.key();
        memcpy(tmpValue, it.value().data(), it.value().size());

        switch(key)
        {
        case Addr_ACbreaker_Cabinet2:
            fqCabData.ACbreaker2 = *((char *)(&tmpValue));
            break;
        case Addr_DCbreaker_Cabinet2:
            fqCabData.DCbreaker2 = *((char *)(&tmpValue));
            break;
        case Addr_ACbreaker_Cabinet3:
            fqCabData.ACbreaker3 = *((char *)(&tmpValue));
            break;
        case Addr_DCbreaker_Cabinet3:
            fqCabData.DCbreaker3 = *((char *)(&tmpValue));
            break;
        case Addr_surgeFeedback_Cabinet2:
            fqCabData.surgeFeedback = *((char *)(&tmpValue));
            break;
        case Addr_fireExtinguisher_Cabinet2:
            fqCabData.fireExtinguisher2 = *((char *)(&tmpValue));
            break;
        case Addr_fireExtinguisher_Cabinet3:
            fqCabData.fireExtinguisher3 = *((char *)(&tmpValue));
            break;
        default:
            break;
        }
    }
    devCache->FreeUpdateEnergyPlantMeter();
}

/**
 *充放电柜数据更新
 */
void RealDataFilter::updateChargeDischargeCabinetInfo(InfoMap centerMap)
{
    char tmpValue[10];
    unsigned char canAddr;
    QByteArray Value;
    if(centerMap.isEmpty())
        return;

    Value = centerMap.value(Addr_CanID_Comm);		//柜子地址
    if(Value.size())
        canAddr = *((unsigned char *)Value.data());
    else
        return;

    stEnergyPlanDatas &Status = devCache->GetUpdateEnergyPlanMeter();
    if(!Status.cdCabMap.contains(canAddr))
    {
        InfoMap tempMap;
        TDevice device;
        QByteArray array;
        memset(&device, 0x00, sizeof(device));
        device.enDeviceType = DEV_DC_CHARGE_DISCHARGE_CAB;
        device.enSubDeviceType = SUB_DEV_UNKNOWN;
        device.iCanID = canAddr;
        device.iParentID = ID_DefaultControlCenterCanID;
        array.append((char *)&device, sizeof(device));
        tempMap.insert(Addr_EnergyPlan_Dev,array);
        emit sigToBus(tempMap, AddrType_EnergyPlanDevChange);
    }
    stChargeDischargeCabinetInfo &cdCabData = Status.cdCabMap.operator [](canAddr);
    QMap<unsigned int, QByteArray>::iterator it;

    for(it = centerMap.begin(); it != centerMap.end(); ++it)
    {
        if((unsigned int)it.value().size() > sizeof(tmpValue))
            continue;

        unsigned int key = it.key();
        memcpy(tmpValue, it.value().data(), it.value().size());

        switch(key)
        {
        case Addr_DCDC1breaker:
            cdCabData.DCDC1breaker = *((char *)(&tmpValue));
            break;
        case Addr_DCDC2breaker:
            cdCabData.DCDC2breaker = *((char *)(&tmpValue));
            break;
        case Addr_DCDC1fireExtinguisher:
            cdCabData.DCDC1fireExtinguisher = *((char *)(&tmpValue));
            break;
        case Addr_DCDC2fireExtinguisher:
            cdCabData.DCDC2fireExtinguisher = *((char *)(&tmpValue));
            break;
        case Addr_DCDCsumVol:
            cdCabData.DCDCsumVol = *((float *)(&tmpValue));
            break;
        case Addr_DCDCsumCur:
            cdCabData.DCDCsumCur = *((float *)(&tmpValue));
            break;
        default:
            break;
        }
    }
    devCache->FreeUpdateEnergyPlantMeter();
}

/**
 *总配电柜数据更新
 */
void RealDataFilter::updateMainDistributionCabinetInfo(InfoMap centerMap)
{
    char tmpValue[10];
    unsigned char canAddr;
    QByteArray Value;
    if(centerMap.isEmpty())
        return;

    Value = centerMap.value(Addr_CanID_Comm);		//柜子地址
    if(Value.size())
        canAddr = *((unsigned char *)Value.data());
    else
        return;

    stEnergyPlanDatas &Status = devCache->GetUpdateEnergyPlanMeter();
    if(!Status.tdCabMap.contains(canAddr))
    {
        InfoMap tempMap;
        TDevice device;
        QByteArray array;
        memset(&device, 0x00, sizeof(device));
        device.enDeviceType = DEV_TOTAL_DISTRIBUTION_CAB;
        device.enSubDeviceType = SUB_DEV_UNKNOWN;
        device.iCanID = canAddr;
        device.iParentID = ID_DefaultControlCenterCanID;
        array.append((char *)&device, sizeof(device));
        tempMap.insert(Addr_EnergyPlan_Dev,array);
        emit sigToBus(tempMap, AddrType_EnergyPlanDevChange);
    }
    stTotalDistributionCabinetInfo &tdCabMap = Status.tdCabMap.operator [](canAddr);
    QMap<unsigned int, QByteArray>::iterator it;

    for(it = centerMap.begin(); it != centerMap.end(); ++it)
    {
        if((unsigned int)it.value().size() > sizeof(tmpValue))
            continue;

        unsigned int key = it.key();
        memcpy(tmpValue, it.value().data(), it.value().size());

        switch(key)
        {
        case Addr_sumBreaker:
            tdCabMap.sumBreaker = *((char *)(&tmpValue));
            break;
        case Addr_loadBreaker1:
            tdCabMap.loadBreaker1 = *((char *)(&tmpValue));
            break;
        case Addr_loadBreaker2:
            tdCabMap.loadBreaker2 = *((char *)(&tmpValue));
            break;
        case Addr_loadBreaker3:
            tdCabMap.loadBreaker3 = *((char *)(&tmpValue));
            break;
        case Addr_loadBreaker4:
            tdCabMap.loadBreaker4 = *((char *)(&tmpValue));
            break;
        case Addr_acBreaker:
            tdCabMap.acBreaker = *((char *)(&tmpValue));
            break;
        case Addr_fireExtinguisherDisCabinet:
            tdCabMap.fireExtinguisher = *((char *)(&tmpValue));
            break;
        default:
            break;
        }
    }
    devCache->FreeUpdateEnergyPlantMeter();
}

/**
 *功率优化器数据更新
 */
void RealDataFilter::updatePowerOptimizerInfo(InfoMap centerMap)
{
    char tmpValue[10];
    unsigned char canAddr;
    QByteArray Value, id;
    if(centerMap.isEmpty())
        return;

    Value = centerMap.value(Addr_CanID_Comm);		//柜子地址
    id = centerMap.value(Addr_DevID_DC_Comm);
    if(Value.size())
        canAddr = *((unsigned char *)Value.data());
    else
        return;

    stEnergyPlanDatas &Status = devCache->GetUpdateEnergyPlanMeter();
    if(!Status.poModMap[canAddr].mapSingle.contains(id))
    {
        InfoMap tempMap;
        TDevice device;
        QByteArray array;
        memset(&device, 0x00, sizeof(device));
        device.enDeviceType = DEV_POWER_OPTIMIZER;
        device.enSubDeviceType = SUB_DEV_UNKNOWN;
        device.iCanID = *(unsigned short *)id.data() + canAddr * 1000;
        device.iParentID = canAddr;
        array.append((char *)&device, sizeof(device));
        tempMap.insert(Addr_EnergyPlan_Dev,array);
        emit sigToBus(tempMap, AddrType_EnergyPlanDevChange);
    }
    stPowerOptimizerSingle &singleData = Status.poModMap[canAddr].mapSingle.operator [](id);
    QMap<unsigned int, QByteArray>::iterator it;

    for(it = centerMap.begin(); it != centerMap.end(); ++it)
    {
        if((unsigned int)it.value().size() > sizeof(tmpValue))
            continue;

        unsigned int key = it.key();
        memcpy(tmpValue, it.value().data(), it.value().size());

        switch(key)
        {
        case Addr_PowerOptimizerID:
            singleData.PowerOptimizerID = *((unsigned short *)(&tmpValue));
            break;
        case Addr_inVol1:
            singleData.inVol1 = *((float *)(&tmpValue));
            break;
        case Addr_inVol2:
            singleData.inVol2 = *((float *)(&tmpValue));
            break;
        case Addr_inVol3:
            singleData.inVol3 = *((float *)(&tmpValue));
            break;
        case Addr_inVol4:
            singleData.inVol4 = *((float *)(&tmpValue));
            break;
        case Addr_curBranch1:
            singleData.curBranch1 = *((float *)(&tmpValue));
            break;
        case Addr_curBranch2:
            singleData.curBranch2 = *((float *)(&tmpValue));
            break;
        case Addr_curBranch3:
            singleData.curBranch3 = *((float *)(&tmpValue));
            break;
        case Addr_curBranch4:
            singleData.curBranch4 = *((float *)(&tmpValue));
            break;
        case Addr_curBranch5:
            singleData.curBranch5 = *((float *)(&tmpValue));
            break;
        case Addr_curBranch6:
            singleData.curBranch6 = *((float *)(&tmpValue));
            break;
        case Addr_curBranch7:
            singleData.curBranch7 = *((float *)(&tmpValue));
            break;
        case Addr_curBranch8:
            singleData.curBranch8 = *((float *)(&tmpValue));
            break;
        case Addr_curBranch9:
            singleData.curBranch9 = *((float *)(&tmpValue));
            break;
        case Addr_curBranch10:
            singleData.curBranch10 = *((float *)(&tmpValue));
            break;
        case Addr_curBranch11:
            singleData.curBranch11 = *((float *)(&tmpValue));
            break;
        case Addr_curBranch12:
            singleData.curBranch12 = *((float *)(&tmpValue));
            break;
        case Addr_curBranch13:
            singleData.curBranch13 = *((float *)(&tmpValue));
            break;
        case Addr_curBranch14:
            singleData.curBranch14 = *((float *)(&tmpValue));
            break;
        case Addr_curBranch15:
            singleData.curBranch15 = *((float *)(&tmpValue));
            break;
        case Addr_curBranch16:
            singleData.curBranch16 = *((float *)(&tmpValue));
            break;
        case Addr_realPower:
            singleData.realPower = *((float *)(&tmpValue));
            break;
        case Addr_radiatorTemp:
            singleData.radiatorTemp = *((float *)(&tmpValue));
            break;
        case Addr_fault1_bit0:
            singleData.fault1.bit0 = *((char *)(&tmpValue));
            break;
        case Addr_fault1_bit1:
            singleData.fault1.bit1 = *((char *)(&tmpValue));
            break;
        case Addr_fault1_bit2:
            singleData.fault1.bit2 = *((char *)(&tmpValue));
            break;
        case Addr_fault1_bit3:
            singleData.fault1.bit3 = *((char *)(&tmpValue));
            break;
        case Addr_fault1_bit4:
            singleData.fault1.bit4 = *((char *)(&tmpValue));
            break;
        case Addr_fault1_bit5:
            singleData.fault1.bit5 = *((char *)(&tmpValue));
            break;
        case Addr_fault1_bit6:
            singleData.fault1.bit6 = *((char *)(&tmpValue));
            break;
        case Addr_fault1_reserve:
            singleData.fault1.bit7 = *((char *)(&tmpValue));
            break;
        case Addr_reserve1:
            singleData.fault1.reserve = *((char *)(&tmpValue));
            break;
        case Addr_fault2_bit0:
            singleData.fault2.bit0 = *((char *)(&tmpValue));
            break;
        case Addr_fault2_bit1:
            singleData.fault2.bit1 = *((char *)(&tmpValue));
            break;
        case Addr_fault2_bit2:
            singleData.fault2.bit2 = *((char *)(&tmpValue));
            break;
        case Addr_fault2_bit3:
            singleData.fault2.bit3 = *((char *)(&tmpValue));
            break;
        case Addr_fault2_bit4:
            singleData.fault2.bit4 = *((char *)(&tmpValue));
            break;
        case Addr_fault2_bit5:
            singleData.fault2.bit5 = *((char *)(&tmpValue));
            break;
        case Addr_fault2_bit6:
            singleData.fault2.bit6 = *((char *)(&tmpValue));
            break;
        case Addr_fault2_bit7:
            singleData.fault2.bit7 = *((char *)(&tmpValue));
            break;
        case Addr_fault2_bit8:
            singleData.fault2.bit8 = *((char *)(&tmpValue));
            break;
        case Addr_fault2_bit9:
            singleData.fault2.bit9 = *((char *)(&tmpValue));
            break;
        case Addr_fault2_bit10:
            singleData.fault2.bit10 = *((char *)(&tmpValue));
            break;
        case Addr_fault2_bit11:
            singleData.fault2.bit11 = *((char *)(&tmpValue));
            break;
        case Addr_fault2_bit12:
            singleData.fault2.bit12 = *((char *)(&tmpValue));
            break;
        case Addr_fault2_bit13:
            singleData.fault2.bit13 = *((char *)(&tmpValue));
            break;
        case Addr_fault2_bit14:
            singleData.fault2.bit14 = *((char *)(&tmpValue));
            break;
        case Addr_fault2_bit15:
            singleData.fault2.bit15 = *((char *)(&tmpValue));
            break;

        case Addr_warning_bit0:
            singleData.warning.bit0 = *((char *)(&tmpValue));
            break;
        case Addr_warning_bit1:
            singleData.warning.bit1 = *((char *)(&tmpValue));
            break;
        case Addr_warning_bit2:
            singleData.warning.bit2 = *((char *)(&tmpValue));
            break;
        case Addr_warning_bit3:
            singleData.warning.bit3 = *((char *)(&tmpValue));
            break;
        case Addr_warning_bit4:
            singleData.warning.bit4 = *((char *)(&tmpValue));
            break;
        case Addr_warning_bit5:
            singleData.warning.bit5 = *((char *)(&tmpValue));
            break;
        case Addr_warning_bit6:
            singleData.warning.bit6 = *((char *)(&tmpValue));
            break;
        case Addr_warning_bit7:
            singleData.warning.bit7 = *((char *)(&tmpValue));
            break;
        case Addr_warning_bit8:
            singleData.warning.bit8 = *((char *)(&tmpValue));
            break;
        case Addr_warning_bit9:
            singleData.warning.bit9 = *((char *)(&tmpValue));
            break;
        case Addr_warning_bit10:
            singleData.warning.bit10 = *((char *)(&tmpValue));
            break;
        case Addr_warning_bit11:
            singleData.warning.bit11 = *((char *)(&tmpValue));
            break;
        case Addr_warning_bit12:
            singleData.warning.bit12 = *((char *)(&tmpValue));
            break;
        case Addr_warning_bit13:
            singleData.warning.bit13 = *((char *)(&tmpValue));
            break;
        case Addr_warning_bit14:
            singleData.warning.bit14 = *((char *)(&tmpValue));
            break;
        case Addr_warning_bit15:
            singleData.warning.bit15 = *((char *)(&tmpValue));
            break;

        case Addr_combinerStatus:
            singleData.combinerStatus = *((char *)(&tmpValue));
            break;
        case Addr_SoftVer_PO:
            singleData.softVer = *((unsigned int *)(&tmpValue));
            break;
        case Addr_sysRequestStatus_bit0:
            singleData.sysRequestStatus.bit0 = *((char *)(&tmpValue));
            break;
        case Addr_sysRequestStatus_bit1:
            singleData.sysRequestStatus.bit1 = *((char *)(&tmpValue));
            break;

        case Addr_inVol5:
            singleData.inVol5 = *((float *)(&tmpValue));
            break;
        case Addr_inVol6:
            singleData.inVol6 = *((float *)(&tmpValue));
            break;
        case Addr_inVol7:
            singleData.inVol7 = *((float *)(&tmpValue));
            break;
        case Addr_inVol8:
            singleData.inVol8 = *((float *)(&tmpValue));
            break;
        case Addr_outVol:
            singleData.outVol = *((float *)(&tmpValue));
            break;
        default:
            break;
        }
    }
    devCache->FreeUpdateEnergyPlantMeter();
}

/**
 *温湿度计数据更新
 */
void RealDataFilter::updateHygrothermographInfo(InfoMap centerMap)
{
    char tmpValue[10];
    unsigned char canAddr;
    QByteArray Value, id;
    if(centerMap.isEmpty())
        return;

    Value = centerMap.value(Addr_CanID_Comm);		//柜子地址
    id = centerMap.value(Addr_DevID_DC_Comm);
    if(Value.size())
        canAddr = *((unsigned char *)Value.data());
    else
        return;

    stEnergyPlanDatas &Status = devCache->GetUpdateEnergyPlanMeter();
    if(!Status.hyModMap[canAddr].mapSingle.contains(id))
    {
        InfoMap tempMap;
        TDevice device;
        QByteArray array;
        memset(&device, 0x00, sizeof(device));
        device.enDeviceType = DEV_TEMP_HUMI_SENSOR;
        device.enSubDeviceType = SUB_DEV_UNKNOWN;
        device.iCanID = *(unsigned short *)id.data() + canAddr * 1000;
        device.iParentID = canAddr;
        array.append((char *)&device, sizeof(device));
        tempMap.insert(Addr_EnergyPlan_Dev,array);
        emit sigToBus(tempMap, AddrType_EnergyPlanDevChange);
    }
    stHygrothermographSingle &singleData = Status.hyModMap[canAddr].mapSingle.operator [](id);
    QMap<unsigned int, QByteArray>::iterator it;

    for(it = centerMap.begin(); it != centerMap.end(); ++it)
    {
        if((unsigned int)it.value().size() > sizeof(tmpValue))
            continue;

        unsigned int key = it.key();
        memcpy(tmpValue, it.value().data(), it.value().size());

        switch(key)
        {
        case Addr_envTemperature:
            singleData.tempurature = *((float *)(&tmpValue));
            break;
        case Addr_envHumidity:
            singleData.humility = *((float *)(&tmpValue));
            break;
        default:
            break;
        }
    }

    devCache->FreeUpdateEnergyPlantMeter();
}

/**
 *独立逆变器数据更新
 */
void RealDataFilter::updateSingleInverterCabinetInfo(InfoMap centerMap)
{
    char tmpValue[10];
    unsigned char canAddr;
    QByteArray Value;
    if(centerMap.isEmpty())
        return;

    Value = centerMap.value(Addr_CanID_Comm);		//柜子地址
    if(Value.size())
        canAddr = *((unsigned char *)Value.data());
    else
        return;

    stEnergyPlanDatas &Status = devCache->GetUpdateEnergyPlanMeter();
    if(!Status.siCabMap.contains(canAddr))
    {
        InfoMap tempMap;
        TDevice device;
        QByteArray array;
        memset(&device, 0x00, sizeof(device));
        device.enDeviceType = DEV_ACDC_INDEPENDENT_INVERTER_CABINET;
        device.enSubDeviceType = SUB_DEV_UNKNOWN;
        device.iCanID = canAddr;
        device.iParentID = ID_DefaultControlCenterCanID;
        array.append((char *)&device, sizeof(device));
        tempMap.insert(Addr_EnergyPlan_Dev,array);
        emit sigToBus(tempMap, AddrType_EnergyPlanDevChange);
    }
    stSingleInverterCabInfo &singleInfo = Status.siCabMap.operator [](canAddr);
    QMap<unsigned int, QByteArray>::iterator it;

    for(it = centerMap.begin(); it != centerMap.end(); ++it)
    {
        if((unsigned int)it.value().size() > sizeof(tmpValue))
            continue;

        unsigned int key = it.key();
        memcpy(tmpValue, it.value().data(), it.value().size());

        switch(key)
        {
        case Addr_ACbreaker_Inverter:
            singleInfo.ACbreaker = *((char *)(&tmpValue));
            break;
        case Addr_DCbreaker_Inverter:
            singleInfo.DCbreaker = *((char *)(&tmpValue));
            break;
        case Addr_surgeFeedback_Cabinet2_Inverter:
            singleInfo.surgeFeedback_Cabinet2 = *((char *)(&tmpValue));
            break;
        case Addr_fireExtinguisher_Cabinet3_Inverter:
            singleInfo.fireExtinguisher_Cabinet3 = *((char *)(&tmpValue));
            break;
        default:
            break;
        }
    }
    devCache->FreeUpdateEnergyPlantMeter();
}

/**
 *ACDC数据更新
 */
void RealDataFilter::updateACDCInfo(InfoMap centerMap)
{
    char tmpValue[10];
    unsigned char canAddr;
    QByteArray Value, id;
    if(centerMap.isEmpty())
        return;

    Value = centerMap.value(Addr_CanID_Comm);		//柜子地址
    id = centerMap.value(Addr_DevID_DC_Comm);
    if(Value.size())
        canAddr = *((unsigned char *)Value.data());
    else
        return;

    stEnergyPlanDatas &Status = devCache->GetUpdateEnergyPlanMeter();
    if(!Status.acdcModMap[canAddr].mapSingle.contains(id))
    {
        InfoMap tempMap;
        TDevice device;
        QByteArray array;
        memset(&device, 0x00, sizeof(device));
        device.enDeviceType = DEV_ACDC_MODULE;
        device.enSubDeviceType = SUB_DEV_UNKNOWN;
        device.iCanID = *(unsigned short *)id.data() + canAddr * 1000;
        device.iParentID = canAddr;
        array.append((char *)&device, sizeof(device));
        tempMap.insert(Addr_EnergyPlan_Dev,array);
        emit sigToBus(tempMap, AddrType_EnergyPlanDevChange);
    }
    stACDCModuleSingle &singleData = Status.acdcModMap[canAddr].mapSingle.operator [](id);
    QMap<unsigned int, QByteArray>::iterator it;

    for(it = centerMap.begin(); it != centerMap.end(); ++it)
    {
        if((unsigned int)it.value().size() > sizeof(tmpValue))
            continue;

        unsigned int key = it.key();
        memcpy(tmpValue, it.value().data(), it.value().size());

        switch(key)
        {
        case Addr_vol_U:
            singleData.vol_A = *((float *)(&tmpValue));
            break;
        case Addr_cur_U:
            singleData.cur_A = *((float *)(&tmpValue));
            break;
        case Addr_vol_V:
            singleData.vol_B = *((float *)(&tmpValue));
            break;
        case Addr_cur_V:
            singleData.cur_B = *((float *)(&tmpValue));
            break;
        case Addr_vol_W:
            singleData.vol_C = *((float *)(&tmpValue));
            break;
        case Addr_cur_W:
            singleData.cur_C = *((float *)(&tmpValue));
            break;
        case Addr_frequency:
            singleData.frequency = *((float *)(&tmpValue));
            break;

        case Addr_sysActivePower:
            singleData.sysActivePower = *((float *)(&tmpValue));
            break;
        case Addr_sysReActivePower:
            singleData.sysReActivePower = *((float *)(&tmpValue));
            break;
        case Addr_sysApparentPower:
            singleData.sysApparentPower = *((float *)(&tmpValue));
            break;
        case Addr_PF:
            singleData.PF = *((float *)(&tmpValue));
            break;
        case Addr_DCpositiveCur:
            singleData.DCpositiveCur = *((float *)(&tmpValue));
            break;
        case Addr_DCnegativeCur:
            singleData.DCnegativeCur = *((float *)(&tmpValue));
            break;
        case Addr_DCpositiveBusBarVol:
            singleData.DCpositiveBusBarVol = *((float *)(&tmpValue));
            break;
        case Addr_DCnegativeBusBarVol:
            singleData.DCnegativeBusBarVol = *((float *)(&tmpValue));
            break;
        case Addr_DCbilateralBusBarVol:
            singleData.DCbilateralBusBarVol = *((float *)(&tmpValue));
            break;
        case Addr_DCpower:
            singleData.DCpower = *((float *)(&tmpValue));
            break;
        case Addr_devStatus0:
            singleData.devStatus = *((unsigned short *)(&tmpValue));
            break;
        case Addr_warningStatus0:
            singleData.warningStatus = *((unsigned short *)(&tmpValue));
            break;
        case Addr_faultStatus0:
            singleData.faultStatus = *((unsigned short *)(&tmpValue));
            break;
        case Addr_HWVersion:
            singleData.HWVersion = *((unsigned int *)(&tmpValue));
            break;
        case Addr_SWVersion:
            singleData.SWVersion = *((unsigned int *)(&tmpValue));
            break;
        case Addr_tmp_IGBT1:
            singleData.tmp_IGBT1 = *((float *)(&tmpValue));
            break;
        case Addr_tmp_IGBT2:
            singleData.tmp_IGBT2 = *((float *)(&tmpValue));
            break;
        case Addr_tmp_IGBT3:
            singleData.tmp_IGBT3 = *((float *)(&tmpValue));
            break;
        case Addr_tmp_IGBT4:
            singleData.tmp_IGBT4 = *((float *)(&tmpValue));
            break;
        case Addr_tmp_IGBT5:
            singleData.tmp_IGBT5 = *((float *)(&tmpValue));
            break;
        case Addr_tmp_IGBT6:
            singleData.tmp_IGBT6 = *((float *)(&tmpValue));
            break;
        case Addr_tmp_IN:
            singleData.tmp_IN = *((float *)(&tmpValue));
            break;
        case Addr_tmp_OUT:
            singleData.tmp_OUT = *((float *)(&tmpValue));
            break;
        case Addr_inductance1_cur:
            singleData.inductance1_cur = *((float *)(&tmpValue));
            break;
        case Addr_inductance2_cur:
            singleData.inductance2_cur = *((float *)(&tmpValue));
            break;
        case Addr_inductance3_cur:
            singleData.inductance3_cur = *((float *)(&tmpValue));
            break;
        case Addr_inductance4_cur:
            singleData.inductance4_cur = *((float *)(&tmpValue));
            break;
        case Addr_inductance5_cur:
            singleData.inductance5_cur = *((float *)(&tmpValue));
            break;
        case Addr_inductance6_cur:
            singleData.inductance6_cur = *((float *)(&tmpValue));
            break;
        default:
            break;
        }
    }

    devCache->FreeUpdateEnergyPlantMeter();
}

/**
 *DCDC数据更新(充放电柜)
 */
void RealDataFilter::updateDCDCInfo_CD(InfoMap centerMap)
{
    char tmpValue[10];
    unsigned char canAddr;
    QByteArray Value, id;
    if(centerMap.isEmpty())
        return;

    Value = centerMap.value(Addr_CanID_Comm);		//柜子地址
    id = centerMap.value(Addr_DevID_DC_Comm);
    if(Value.size())
        canAddr = *((unsigned char *)Value.data());
    else
        return;

    stEnergyPlanDatas &Status = devCache->GetUpdateEnergyPlanMeter();
    if(!Status.dcdcModMap_cd[canAddr].mapSingle.contains(id))
    {
        InfoMap tempMap;
        TDevice device;
        QByteArray array;
        memset(&device, 0x00, sizeof(device));
        device.enDeviceType = DEV_CHARGE_DISCHARGE_CAB_DCDC_MODULE;
        device.enSubDeviceType = SUB_DEV_UNKNOWN;
        device.iCanID = id.at(0) + canAddr * 1000;
        device.iParentID = canAddr;
        array.append((char *)&device, sizeof(device));
        tempMap.insert(Addr_EnergyPlan_Dev,array);
        emit sigToBus(tempMap, AddrType_EnergyPlanDevChange);
    }
    stDCDC_CDCModuleSingle &singleData = Status.dcdcModMap_cd[canAddr].mapSingle.operator [](id);
    QMap<unsigned int, QByteArray>::iterator it;

    for(it = centerMap.begin(); it != centerMap.end(); ++it)
    {
        if((unsigned int)it.value().size() > sizeof(tmpValue))
            continue;

        unsigned int key = it.key();
        memcpy(tmpValue, it.value().data(), it.value().size());

        switch(key)
        {
        case Addr_boardID:
            singleData.boardID = *((char *)(&tmpValue));
            break;
        case Addr_moduleMode:
            singleData.status = *((char *)(&tmpValue));
            break;
        case Addr_outVol:
            singleData.outVol = *((float *)(&tmpValue));
            break;
        case Addr_outCur:
            singleData.outCur = *((float *)(&tmpValue));
            break;
        case Addr_inVol:
            singleData.inVol = *((float *)(&tmpValue));
            break;
        case Addr_inCur:
            singleData.inCur = *((float *)(&tmpValue));
            break;
        case Addr_boardTmp_M1:
            singleData.boardTemp_M1 = *((float *)(&tmpValue));
            break;
        case Addr_envTmp:
            singleData.envTemp = *((float *)(&tmpValue));
            break;
        case Addr_runTime:
            singleData.runTime = *((float *)(&tmpValue));
            break;
        case Addr_chargeDischargeTimes:
            singleData.chargeDisChargeTimes = *((float *)(&tmpValue));
            break;

        case Addr_moduleStatus0:
            singleData.alarm0 = *((char *)(&tmpValue));
            break;
        case Addr_moduleStatus1:
            singleData.alarm1 = *((char *)(&tmpValue));
            break;
        case Addr_moduleStatus2:
            singleData.alarm2 = *((char *)(&tmpValue));
            break;
        case Addr_moduleStatus3:
            singleData.alarm3 = *((char *)(&tmpValue));
            break;
        case Addr_moduleStatus4:
            singleData.alarm4 = *((char *)(&tmpValue));
            break;
        case Addr_moduleStatus5:
            singleData.alarm5 = *((char *)(&tmpValue));
            break;
        case Addr_moduleStatus6:
            singleData.alarm6 = *((char *)(&tmpValue));
            break;
        case Addr_moduleStatus7:
            singleData.alarm7 = *((char *)(&tmpValue));
            break;
        case Addr_moduleStatus8:
            singleData.alarm8 = *((char *)(&tmpValue));
            break;
        case Addr_moduleStatus9:
            singleData.alarm9 = *((char *)(&tmpValue));
            break;
        case Addr_moduleStatus10:
            singleData.alarm10 = *((char *)(&tmpValue));
            break;
        case Addr_moduleStatus11:
            singleData.alarm11 = *((char *)(&tmpValue));
            break;
        case Addr_moduleStatus12:
            singleData.alarm12 = *((char *)(&tmpValue));
            break;
        case Addr_moduleStatus13:
            singleData.alarm13 = *((char *)(&tmpValue));
            break;
        case Addr_moduleStatus14:
            singleData.alarm14 = *((char *)(&tmpValue));
            break;
        case Addr_moduleStatus15:
            singleData.alarm15 = *((char *)(&tmpValue));
            break;
        case Addr_moduleStatus16:
            singleData.alarm16 = *((char *)(&tmpValue));
            break;
        case Addr_moduleStatus17:
            singleData.alarm17 = *((char *)(&tmpValue));
            break;
        case Addr_moduleStatus18:
            singleData.alarm18 = *((char *)(&tmpValue));
            break;
        case Addr_moduleStatus19:
            singleData.alarm19 = *((char *)(&tmpValue));
            break;
        case Addr_moduleStatus20:
            singleData.alarm20 = *((char *)(&tmpValue));
            break;
        case Addr_moduleStatus21:
            singleData.alarm21 = *((char *)(&tmpValue));
            break;
        case Addr_moduleStatus22:
            singleData.alarm22 = *((char *)(&tmpValue));
            break;
        case Addr_moduleStatus23:
            singleData.alarm23 = *((char *)(&tmpValue));
            break;
        case Addr_moduleStatus24:
            singleData.alarm24 = *((char *)(&tmpValue));
            break;
        case Addr_moduleStatus25:
            singleData.alarm25 = *((char *)(&tmpValue));
            break;
        case Addr_moduleStatus26:
            singleData.alarm26 = *((char *)(&tmpValue));
            break;
        case Addr_moduleStatus27:
            singleData.alarm27 = *((char *)(&tmpValue));
            break;
        case Addr_moduleStatus28:
            singleData.alarm28 = *((char *)(&tmpValue));
            break;
        case Addr_moduleStatus29:
            singleData.alarm29 = *((char *)(&tmpValue));
            break;
        case Addr_moduleStatus30:
            singleData.alarm30 = *((char *)(&tmpValue));
            break;
        case Addr_moduleStatus31:
            singleData.alarm31 = *((char *)(&tmpValue));
            break;
        default:
            break;
        }
    }
    devCache->FreeUpdateEnergyPlantMeter();
}

/**
 *DCDC数据更新(储能柜)
 */
void RealDataFilter::updateDCDCInfo_ES(InfoMap centerMap)
{
    char tmpValue[10];
    unsigned char canAddr = 0;
    unsigned char onlineState = 0;
    QByteArray Value, id;
    if(centerMap.isEmpty())
        return;

    Value = centerMap.value(Addr_CanID_Comm);		//柜子地址
    id = centerMap.value(Addr_DevID_DC_Comm);
    if(Value.size())
        canAddr = *((unsigned char *)Value.data());
    else
        return;

    stEnergyPlanDatas &Status = devCache->GetUpdateEnergyPlanMeter();
    if(!Status.dcdcModMap_es[canAddr].mapSingle.contains(id))
    {
        InfoMap tempMap;
        TDevice device;
        QByteArray array;
        memset(&device, 0x00, sizeof(device));
        device.enDeviceType = DEV_ENERGY_STORAGE_CAB_DCDC_MODULE;
        device.enSubDeviceType = SUB_DEV_UNKNOWN;
        device.iCanID = *(unsigned short*)id.data() + canAddr * 1000;
        device.iParentID = canAddr;
        array.append((char *)&device, sizeof(device));
        tempMap.insert(Addr_EnergyPlan_Dev,array);
        emit sigToBus(tempMap, AddrType_EnergyPlanDevChange);
    }
    stEnergyStorageDCDCSingle &singleData = Status.dcdcModMap_es[canAddr].mapSingle.operator [](id);
    singleData.devStatus.reserve = 0;
    singleData.devStatus.reserve1 = 0;
    singleData.warningStatus.reserve = 0;
    singleData.warningStatus.reserve1 = 0;
    singleData.onlineCounter = 0;

    QMap<unsigned int, QByteArray>::iterator it;
    for(it = centerMap.begin(); it != centerMap.end(); ++it)
    {
        if((unsigned int)it.value().size() > sizeof(tmpValue))
            continue;

        unsigned int key = it.key();
        memcpy(tmpValue, it.value().data(), it.value().size());

        switch(key)
        {
        case Addr_inVol_EnergyStorage:
            singleData.vol_in = *((float *)(&tmpValue));
            break;
        case Addr_outVol_EnergyStorage:
            singleData.vol_out = *((float *)(&tmpValue));
            break;
        case Addr_inCur_EnergyStorage:
            singleData.cur_in = *((float *)(&tmpValue));
            break;
        case Addr_outCur_EnergyStorage:
            singleData.cur_out = *((float *)(&tmpValue));
            break;
        case Addr_batteryVol_EnergyStorage:
            singleData.vol_battery = *((float *)(&tmpValue));
            break;
        case Addr_dcPower_EnergyStorage:
            singleData.power_dc = *((float *)(&tmpValue));
            break;
        case Addr_devStatus0_EnergyStorage:
            singleData.devStatus.warning = *((char *)(&tmpValue));
            break;
        case Addr_devStatus1_EnergyStorage:
            singleData.devStatus.run = *((char *)(&tmpValue));
            break;
        case Addr_devStatus2_EnergyStorage:
            singleData.devStatus.fault = *((char *)(&tmpValue));
            break;
        case Addr_devStatus3_EnergyStorage:
            onlineState = singleData.devStatus.offline;
            singleData.devStatus.offline = *((char *)(&tmpValue));
            break;
        case Addr_warningStatus0_EnergyStorage:
            singleData.warningStatus.fun1 = *((char *)(&tmpValue));
            break;
        case Addr_warningStatus1_EnergyStorage:
            singleData.warningStatus.fun2 = *((char *)(&tmpValue));
            break;
        case Addr_warningStatus2_EnergyStorage:
            singleData.warningStatus.fun3 = *((char *)(&tmpValue));
            break;
        case Addr_faultStatus0_EnergyStorage:
            singleData.faultStatus.fault0 = *((char *)(&tmpValue));
            break;
        case Addr_faultStatus1_EnergyStorage:
            singleData.faultStatus.fault1 = *((char *)(&tmpValue));
            break;
        case Addr_faultStatus2_EnergyStorage:
            singleData.faultStatus.fault2 = *((char *)(&tmpValue));
            break;
        case Addr_faultStatus3_EnergyStorage:
            singleData.faultStatus.fault3 = *((char *)(&tmpValue));
            break;
        case Addr_faultStatus4_EnergyStorage:
            singleData.faultStatus.fault4 = *((char *)(&tmpValue));
            break;
        case Addr_faultStatus5_EnergyStorage:
            singleData.faultStatus.fault5 = *((char *)(&tmpValue));
            break;
        case Addr_faultStatus6_EnergyStorage:
            singleData.faultStatus.fault6 = *((char *)(&tmpValue));
            break;
        case Addr_faultStatus7_EnergyStorage:
            singleData.faultStatus.fault7 = *((char *)(&tmpValue));
            break;

        case Addr_faultStatus8_EnergyStorage:
            singleData.faultStatus.fault8 = *((char *)(&tmpValue));
            break;
        case Addr_faultStatus9_EnergyStorage:
            singleData.faultStatus.fault9 = *((char *)(&tmpValue));
            break;
        case Addr_faultStatus10_EnergyStorage:
            singleData.faultStatus.fault10 = *((char *)(&tmpValue));
            break;
        case Addr_faultStatus11_EnergyStorage:
            singleData.faultStatus.fault11 = *((char *)(&tmpValue));
            break;
        case Addr_faultStatus12_EnergyStorage:
            singleData.faultStatus.fault12 = *((char *)(&tmpValue));
            break;
        case Addr_faultStatus13_EnergyStorage:
            singleData.faultStatus.fault13 = *((char *)(&tmpValue));
            break;
        case Addr_faultStatus14_EnergyStorage:
            singleData.faultStatus.fault14 = *((char *)(&tmpValue));
            break;
        case Addr_faultStatus15_EnergyStorage:
            singleData.faultStatus.fault15 = *((char *)(&tmpValue));
            break;

        case Addr_HWVersion:
            singleData.HWVersion = *((unsigned int *)(&tmpValue));
            break;
        case Addr_SWVersion:
            singleData.SWVersion = *((unsigned int *)(&tmpValue));
            break;
        case Addr_tmp_IGBT1:
            singleData.tmp_IGBT1 = *((float *)(&tmpValue));
            break;
        case Addr_tmp_IGBT2:
            singleData.tmp_IGBT2 = *((float *)(&tmpValue));
            break;
        case Addr_tmp_IGBT3:
            singleData.tmp_IGBT3 = *((float *)(&tmpValue));
            break;
        case Addr_tmp_IGBT4:
            singleData.tmp_IGBT4 = *((float *)(&tmpValue));
            break;
        case Addr_tmp_IGBT5:
            singleData.tmp_IGBT5 = *((float *)(&tmpValue));
            break;
        case Addr_tmp_IGBT6:
            singleData.tmp_IGBT6 = *((float *)(&tmpValue));
            break;
        case Addr_tmp_IN:
            singleData.tmp_IN = *((float *)(&tmpValue));
            break;
        case Addr_tmp_OUT:
            singleData.tmp_OUT = *((float *)(&tmpValue));
            break;
        default:
            break;
        }
    }

    if(onlineState != singleData.devStatus.offline)
    {
        InfoMap centerMap;
        QByteArray tempArray;
        unsigned char ucOnlineFlag = singleData.devStatus.offline;
        int iEPID = *(unsigned short*)id.data() + canAddr * 1000;
        //增加能效ID
        tempArray.append((char *)&iEPID, sizeof(iEPID));
        centerMap.insert(Addr_EnergyPlan_ID_Comm, tempArray);
        tempArray.clear();
        //增加离线变化状态突发
        tempArray.append((char *)&ucOnlineFlag, sizeof(ucOnlineFlag));
        centerMap.insert(Addr_devStatus3_EnergyStorage, tempArray);
        tempArray.clear();


    }
    devCache->FreeUpdateEnergyPlantMeter();
}

/**
 *EMS数据更新
 */
void RealDataFilter::updateEMSInfo(InfoMap centerMap)
{
    char tmpValue[10];
    unsigned char ucGridState_old = 0;
    unsigned char canAddr;
    QByteArray Value;
    if(centerMap.isEmpty())
        return;

    Value = centerMap.value(Addr_CanID_Comm);		//柜子地址
    if(Value.size())
        canAddr = *((unsigned char *)Value.data());
    else
        return;

    stEnergyPlanDatas &Status = devCache->GetUpdateEnergyPlanMeter();
    if(!Status.emsMap.contains(canAddr))
    {
        InfoMap tempMap;
        TDevice device;
        QByteArray array;
        memset(&device, 0x00, sizeof(device));
        device.enDeviceType = DEV_EMS_CAB;
        device.enSubDeviceType = SUB_DEV_UNKNOWN;
        device.iCanID = canAddr;
        device.iParentID = ID_DefaultControlCenterCanID;
        array.append((char *)&device, sizeof(device));
        tempMap.insert(Addr_EnergyPlan_Dev,array);
        emit sigToBus(tempMap, AddrType_EnergyPlanDevChange);
    }
    stEMSInfo &singleInfo = Status.emsMap.operator [](canAddr);
    QMap<unsigned int, QByteArray>::iterator it;

    for(it = centerMap.begin(); it != centerMap.end(); ++it)
    {
        if((unsigned int)it.value().size() > sizeof(tmpValue))
            continue;

        unsigned int key = it.key();
        memcpy(tmpValue, it.value().data(), it.value().size());

        switch(key)
        {
        case Addr_tmpEMSHighVol:
            singleInfo.tempHighVol = *((float *)(&tmpValue));
            break;
        case Addr_humityEMSHighVol:
            singleInfo.humidityHighVol = *((float *)(&tmpValue));
            break;
        case Addr_tmpEMSLowVol:
            singleInfo.tempLowVol_in = *((float *)(&tmpValue));
            break;
        case Addr_humityEMSLowVol:
            singleInfo.humidityLowVol_in = *((float *)(&tmpValue));
            break;
        case Addr_tmpEMSLowVol_out:
            singleInfo.tempLowVol_out = *((float *)(&tmpValue));
            break;
        case Addr_humityEMSLowVol_out:
            singleInfo.humidityLowVol_out = *((float *)(&tmpValue));
            break;
        case Addr_smokeSensor_lowVolIn:
            singleInfo.smokeSensor_lowVolIn = *((char *)(&tmpValue));
            break;
        case Addr_frameFeedback:
            singleInfo.frameFeedback = *((char *)(&tmpValue));
            break;
        case Addr_minorLoadbreaker_630A1:
            singleInfo.minorLoadbreaker_630A1 = *((char *)(&tmpValue));
            break;
        case Addr_minorLoadbreaker_630A2:
            singleInfo.minorLoadbreaker_630A2 = *((char *)(&tmpValue));
            break;
        case Addr_smokeSensor_lowVolOut:
            singleInfo.smokeSensor_lowVolOut = *((char *)(&tmpValue));
            break;
        case Addr_minorLoadbreaker_400A1:
            singleInfo.minorLoadbreaker_400A1 = *((char *)(&tmpValue));
            break;
        case Addr_minorLoadbreaker_400A2:
            singleInfo.minorLoadbreaker_400A2 = *((char *)(&tmpValue));
            break;
        case Addr_minorLoadbreaker_400A3:
            singleInfo.minorLoadbreaker_400A3 = *((char *)(&tmpValue));
            break;
        case Addr_acdcBreaker:
            singleInfo.acdcBreaker = *((char *)(&tmpValue));
            break;
        case Addr_importBreaker1:
            singleInfo.importBreaker1 = *((char *)(&tmpValue));
            break;
        case Addr_importBreaker2:
            singleInfo.importBreaker2 = *((char *)(&tmpValue));
            break;
        case Addr_emergncyStop:
            singleInfo.emergncyStop = *((char *)(&tmpValue));
            break;

        case Addr_mainModuleSwitchCMD:
            singleInfo.SmoothSwitch_acdc_FQ = *((char *)(&tmpValue));
            break;
        case Addr_mainLoad_ACDCModuleSwitchCMD:
            singleInfo.SmoothSwitch_acdc_SI = *((char *)(&tmpValue));
            break;
        case Addr_storageUnit1_DCDCModuleSwitchCMD:
            singleInfo.SmoothSwitch_unit1_ES = *((char *)(&tmpValue));
            break;
        case Addr_storageUnit2_DCDCModuleSwitchCMD:
            singleInfo.SmoothSwitch_unit2_ES = *((char *)(&tmpValue));
            break;
        case Addr_Grid_State_EMS:
            ucGridState_old = singleInfo.GridState;
            singleInfo.GridState = *((char *)(&tmpValue));
            break;
        default:
            break;
        }
    }
    //EMS并网状态突发
    if(ucGridState_old != singleInfo.GridState)
    {
        InfoMap centerMap;
        QByteArray tempArray;
        int iID = canAddr;
        //添加CAN地址
        tempArray.append((char *)&iID, sizeof(iID));
        centerMap.insert(Addr_EnergyPlan_ID_Comm, tempArray);
        tempArray.clear();
        //添加电网变化值
        tempArray.append((char *)&singleInfo.GridState);
        centerMap.insert(Addr_Grid_State_EMS, tempArray);
        tempArray.clear();

        emit sigToBus(centerMap, AddrType_EnergyPlan_Signal_Burst);
    }
    devCache->FreeUpdateEnergyPlantMeter();
}

/**
 *报警数据更新
 */
void RealDataFilter::updateAlarmInfo(InfoMap alarmMap)
{
    char tmpValue[10];

    if(alarmMap.isEmpty())
        return;

    RealStatusMeterData &Status = devCache->GetUpdateRealStatusMeter();
    RealStatusData &Realdata = Status.realData;

    QMap<unsigned int, QByteArray>::iterator it;
    for(it = alarmMap.begin(); it != alarmMap.end(); ++it)
    {
        if((unsigned int)it.value().size() > sizeof(tmpValue))
            continue;

        unsigned int key = it.key();
        memcpy(tmpValue, it.value().data(), it.value().size());

        if(key == Addr_Alarm_Type1)
            Realdata.alarm1 = *((unsigned short *)tmpValue);
        else if(key == Addr_Alarm_Type1)
            Realdata.alarm2 = *((unsigned short *)tmpValue);
    }
    devCache->FreeUpdateRealStatusMeter();
}

/**
 *温湿度数据更新
 */
void RealDataFilter::updateTempHumi(InfoMap envMap)
{
    //short data = 0;
    char tmpValue[10];

    if(envMap.isEmpty())
        return;
    RealStatusMeterData &Status = devCache->GetUpdateRealStatusMeter();
    RealStatusData &Realdata = Status.realData;

    QMap<unsigned int, QByteArray>::iterator it;

    for(it = envMap.begin(); it != envMap.end(); ++it)
    {
        if((unsigned int)it.value().size() > sizeof(tmpValue))
            continue;
        unsigned int key = it.key();
        memcpy(tmpValue, it.value().data(), it.value().size());
        if(key == Addr_StationEnvTemp)
            Realdata.temperature = *((short *)tmpValue);
        else if(key == Addr_StationEnvHumi)
            Realdata.humidity = *((short *)tmpValue);
    }
    devCache->FreeUpdateRealStatusMeter();
}
/**
 *直流柜CCU遥测数据
 */
void RealDataFilter::updateCCUSignalMeasure(InfoMap dcMap)
{
    char tmpValue[10];
    unsigned char canAddr;
    QByteArray Value;

    if(dcMap.isEmpty())
        return;
    Value = dcMap.value(Addr_CanID_Comm);
    if(Value.size())
        canAddr = *((unsigned char *)Value.data());
    else
        return;
    QMap<unsigned int, QByteArray>::iterator it;

    //	RealStatusMeterData &Status = devCache->GetUpdateRealStatusMeter();
    //CCURealData &ccuData = Status.ccuAllData.ccuData.operator[](canAddr);
    stDCCabinetDatas &Status = devCache->GetUpdateDCCabinetMeter();
    stCCUDatasItem &ccuData = Status.ccuMap.operator[](canAddr);
    //CCU 报文接收
    devCache->UpdateCCUDataTime(canAddr); //nihai add


    for(it = dcMap.begin(); it != dcMap.end(); ++it)
    {
        if((unsigned int)it.value().size() > sizeof(tmpValue))
            continue;

        unsigned int key = it.key();
        memcpy(tmpValue, it.value().data(), it.value().size());

        if(key == Addr_CCUTotalRunTime_DCcab)
            ccuData.ccutotalruntime = *((unsigned int *)tmpValue);
        else if( key == Addr_DCcabFaultCode)		//告警状态
            ccuData.warning_status = *((unsigned char *)tmpValue);
        else if(key == Addr_CCUEnvTemp_DCcab)
            ccuData.ccuenvtemp = *((short *)tmpValue);
        else if(key == Addr_CabRatedPower_DCCab)
            ccuData.cabratedpower = *((float *)tmpValue);
        else if(key == Addr_CabOutPower_DCCab)
            ccuData.cabnowpower = *((float *)tmpValue);
        else if(key == Addr_CCUWorkState_DCcab)                //CCU运行状态
            ccuData.runStatus = *((char *)tmpValue);
        else if(key == Addr_CCUInputContactor_DCcab)			//输入接触器状态
            ccuData.inRelayStatus = *((char *)tmpValue);
        else if(key == Addr_CCULinkageContactor_DCcab)			//联动接触器状态
            ccuData.linkageRelayStatus = *((char *)tmpValue);
        else if(key == Addr_CCUSystemType_DCcab)				//系统机型
            ccuData.sysType = *((char *)tmpValue);
    }
    devCache->FreeUpdateDCCabinetMeter();
}
/**
 *直流柜pdu 遥信遥测数据
 */
void RealDataFilter::updatePDUSignalMeasure(InfoMap dcMap)
{
    char tmpValue[10];
    unsigned char canAddr;
    QByteArray Value, id;

    if(dcMap.isEmpty())
        return;

    Value = dcMap.value(Addr_CanID_Comm);
    if(Value.size())
        canAddr = *((unsigned char *)Value.data());
    else
        return;

    id = dcMap.value(Addr_DevID_DC_Comm);
    QMap<unsigned int, QByteArray>::iterator it;

    stDCCabinetDatas &Status = devCache->GetUpdateDCCabinetMeter();
    if(!Status.pduMap.contains(canAddr))
    {
        InfoMap tempMap;
        TDevice device;
        QByteArray array;
        memset(&device, 0x00, sizeof(device));
        device.enDeviceType = DEV_PDU;
        device.enSubDeviceType = SUB_DEV_UNKNOWN;
        device.iCanID = id.at(0) + canAddr * 1000;
        device.iParentID = canAddr;
        array.append((char *)&device, sizeof(device));
        tempMap.insert(Addr_EnergyPlan_Dev,array);
        emit sigToBus(tempMap, AddrType_EnergyPlanDevChange);
    }
    stPDUDatasItem &pduData = Status.pduMap[canAddr].mapSingle.operator[](id);

    for(it = dcMap.begin(); it != dcMap.end(); ++it)
    {
        if((unsigned int)it.value().size() > sizeof(tmpValue))
            continue;

        unsigned int key = it.key();
        memcpy(tmpValue, it.value().data(), it.value().size());

        if(key == Addr_PDUWorkState_DCcab)
            pduData.runStatus = *((unsigned char *)tmpValue);
        else if( key == Addr_DCcabFaultCode)		//告警状态
            pduData.warning_status = *((unsigned char *)tmpValue);
        else if(key == Addr_PDUGreenLight_DCcab)    //绿灯
            pduData.greenLight = *((unsigned char *)tmpValue);
        else if(key == Addr_PDUYellowLight_DCcab)    //黄灯
            pduData.yellowLight = *((unsigned char *)tmpValue);
        else if(key == Addr_PDURedLight_DCcab)    //红灯
            pduData.redLight = *((unsigned char *)tmpValue);
        else if(key == Addr_PDUOutVoltage_DCcab)		//PDU输出电压
            pduData.outVolatge= *((float *)tmpValue);
        else if(key == Addr_PDUOutCurrent_DCcab)		//PDU输出电流
            pduData.outCurrent = *((float *)tmpValue);
        else if(key == Addr_PDUEnvTemp_DCcab)			//PDU环境温度
            pduData.stationTemp = *((short *)tmpValue);
        else if(key == Addr_PDURadTemp_DCcab)			//PDU散热器温度
            pduData.coolingTemp = *((short *)tmpValue);
        else if(key == Addr_PDUTotalRunTime_DCcab)		//PDU总运行时间
            pduData.totalRuntime = *((unsigned int *)tmpValue);
        else if(key == Addr_PDUSwitchTime_DCcab)		//PDU切换次数
            pduData.switchNum = *((unsigned int *)tmpValue);
        else if(key == Addr_PDUResistanceMinus_DCcab)			//负对地阻值
            pduData.resistanceMinus = *((unsigned short *)tmpValue);
        else if(key == Addr_PDUResistancePlus_DCcab)			//正对地阻值
            pduData.resistancePlus = *((unsigned short *)tmpValue);
        else if(key == Addr_PDUSetVol_DCcab)		//PDU设模块输出电压
            pduData.setVoltage = *((float *)tmpValue);
        else if(key == Addr_PDUSetCur_DCcab)		//PDU设模块限流点
            pduData.setCurrent = *((float *)tmpValue);
        else if(key == Addr_PDUEnergy_DCcab)    //PDU电表度数
            pduData.energy = *((float *)tmpValue);

    }
    devCache->FreeUpdateDCCabinetMeter();
}
/**
 *直流柜分支箱遥信遥测数据, (后期无用, 废弃)
 */
/*
void RealDataFilter::updateBranchSignalMeasure(InfoMap dcMap)
{
    char tmpValue[10];
    unsigned char canAddr;
    QByteArray Value, id;

    if(dcMap.isEmpty())
        return;

    Value = dcMap.value(Addr_CanID_Comm);
    if(Value.size())
        canAddr = *((unsigned char *)Value.data());
    else
        return;

    id = dcMap.value(Addr_DevID_DC_Comm);

    QMap<unsigned int, QByteArray>::iterator it;

    stDCCabinetDatas &Status = devCache->GetUpdateDCCabinetMeter();
    stBranchDatasItem &branchData = Status.branchMap[canAddr].mapSingle.operator[](id);

    for(it = dcMap.begin(); it != dcMap.end(); ++it)
    {
        if((unsigned int )it.value().size() > sizeof(tmpValue))
            continue;

        unsigned int key = it.key();
        memcpy(tmpValue, it.value().data(), it.value().size());

        if( key == Addr_BOXWorkState_DCcab)				//分支箱运行状态
            branchData.runStatus = *((unsigned char *)tmpValue);
        else if( key == Addr_BOXOutVoltage_DCcab)		//分支箱输出电压
            branchData.outVolatge = *((float *)tmpValue);
        else if( key == Addr_BOXOutCurrent_DCcab)		//分支箱输出电流
            branchData.outCurrent = *((float *)tmpValue);
        else if( key == Addr_BOXEnvTemp_DCcab)			//分支箱环境温度
            branchData.stationTemp = *((short *)tmpValue);
        else if( key == Addr_BOXM1Temp_DCcab)			//分支箱M1板温度
            branchData.m1Temp = *((short *)tmpValue);
        else if( key == Addr_BOXTotalRunTime_DCcab)		//分支箱总运行时间
            branchData.totalRuntime = *((unsigned int *)tmpValue);
        else if( key == Addr_BOXSwitchTime_DCcab)		//分支箱切换次数
            branchData.switchNum = *((unsigned int *)tmpValue);
    }
    devCache->FreeUpdateDCCabinetMeter();
}
*/

/**
 *直流柜直流模块遥信遥测数据
 */
void RealDataFilter::updateDCModuleSignalMeasure(InfoMap dcMap)
{
    char tmpValue[10];
    unsigned char canAddr;
    QByteArray Value, id;

    if(dcMap.isEmpty())
        return;

    Value = dcMap.value(Addr_CanID_Comm);
    if(Value.size())
        canAddr = *((unsigned char *)Value.data());
    else
        return;

    id = dcMap.value(Addr_DevID_DC_Comm);

    QMap<unsigned int, QByteArray>::iterator it;

    stDCCabinetDatas &Status = devCache->GetUpdateDCCabinetMeter();
    if(!Status.dcmoduleMap.contains(canAddr))
    {
        InfoMap tempMap;
        TDevice device;
        QByteArray array;
        memset(&device, 0x00, sizeof(device));
        device.enDeviceType = DEV_MODULE;
        device.enSubDeviceType = SUB_DEV_UNKNOWN;
        device.iCanID = id.at(0) + canAddr * 1000;
        device.iParentID = canAddr;
        array.append((char *)&device, sizeof(device));
        tempMap.insert(Addr_EnergyPlan_Dev,array);
        emit sigToBus(tempMap, AddrType_EnergyPlanDevChange);
    }
    stDCModuleDatasItem &dcmoduleData = Status.dcmoduleMap[canAddr].mapSingle.operator[](id);
    devCache->UpdateCCUDataTime(canAddr); //更新CCU接收报文的时间 nihai add

    for(it = dcMap.begin(); it != dcMap.end(); ++it)
    {
        if((unsigned int)it.value().size() > sizeof(tmpValue))
            continue;

        unsigned int key = it.key();
        memcpy(tmpValue, it.value().data(), it.value().size());

        if( key == Addr_DCModuleWorkState_DCcab)		//模块运行状态
            dcmoduleData.runStatus = *((unsigned char *)tmpValue);
        else if( key == Addr_DCcabFaultCode)		//告警状态
            dcmoduleData.warning_status = *((unsigned char *)tmpValue);
        else if( key == Addr_DCModuleGroupNum_DCcab)		//模块所属分组
            dcmoduleData.group = *((unsigned char *)tmpValue);
        else if( key == Addr_DCModuleInAVoltage_DCcab)		//直流模块输入A相电压
            dcmoduleData.inVoloatgeA = *((float *)tmpValue);
        else if( key == Addr_DCModuleInBVoltage_DCcab)		//直流模块输入B相电压
            dcmoduleData.inVoloatgeB = *((float *)tmpValue);
        else if( key == Addr_DCModuleInCVoltage_DCcab)		//直流模块输入C相电压
            dcmoduleData.inVoloatgeC = *((float *)tmpValue);
        else if( key == Addr_DCModuleInCurrent_DCcab)		//直流模块输入电流
            dcmoduleData.inCurrent = *((float *)tmpValue);
        else if( key == Addr_DCModuleOutVoltage_DCcab)		//直流模块输出电压
            dcmoduleData.outVolatge = *((float *)tmpValue);
        else if( key == Addr_DCModuleOutCurrent_DCcab)		//直流模块输出电流
            dcmoduleData.outCurrent = *((float *)tmpValue);
        else if( key == Addr_DCModuleEnvTemp_DCcab)			//直流模块环境温度
            dcmoduleData.stationTemp = *((short *)tmpValue);
        else if( key == Addr_DCModuleM1Temp_DCcab)			//直流模块M1温度
            dcmoduleData.m1Temp = *((short *)tmpValue);
        else if( key == Addr_DCModuleTotalRunTime_DCcab)	//直流模块总运行时间
            dcmoduleData.totalRuntime = *((unsigned int *)tmpValue);
        else if( key == Addr_DCModuleSwitchTime_DCcab)		//直流模块切换次数
            dcmoduleData.switchNum = *((unsigned int *)tmpValue);
    }
    devCache->FreeUpdateDCCabinetMeter();
}

/*
 * 更新直流柜模块故障状态
 */
void RealDataFilter::updateDCCabFaultState(InfoMap dcMap)
{
    unsigned char canAddr, innerid, faultState;
    QByteArray Value, id;

    if(dcMap.isEmpty())
        return;

    Value = dcMap.value(Addr_CanID_Comm);
    if(Value.size())
        canAddr = *((unsigned char *)Value.data());
    else
        return;
    id = dcMap.value(Addr_DevID_DC_Comm);
    innerid = id.at(0);
    faultState = dcMap[Addr_DCcabFaultState].at(0);
    stDCCabinetDatas &Status = devCache->GetUpdateDCCabinetMeter();
    //模块故障
    if((innerid <= InnerID_MaxMod)&&(innerid >= InnerID_MinMod))
    {
        stDCModuleDatasItem &dcmoduleData = Status.dcmoduleMap[canAddr].mapSingle.operator[](id);
        if(faultState == 0x55)
        {
            dcmoduleData.warning_status = 1;
        }
        else
        {
            dcmoduleData.warning_status = 0;
        }
    }
    //CCU故障
    if((innerid <= ID_MaxCCUCanID)&&(innerid >= ID_MinCCUCanID))
    {
        stCCUDatasItem &ccuData = Status.ccuMap.operator[](canAddr);
        if(faultState == 0x55)
            ccuData.warning_status = 1;
        else
            ccuData.warning_status = 0;
    }
    //PDU故障
    if((innerid <= InnerID_MaxPDU)&&(innerid >= InnerID_MinPDU))
    {
        stPDUDatasItem &pduData = Status.pduMap[canAddr].mapSingle.operator[](id);
        if(faultState == 0x55)
            pduData.warning_status = 1;
        else
            pduData.warning_status = 0;
    }

    devCache->FreeUpdateDCCabinetMeter();
}

/*
 * 车牌号校验从第8字节开始的7位长度，且格式符合鲁B23PL5
 * VIN校验所有字符都应在0-9或A-Z范围内
 */
int RealDataFilter::AutoDetect(const char *data, int &iValidLen)
{
    enum{
        INVALID = 0,
        VIN_DETECT,
        CAR_DETECT,
    };

    QString str, strLog;
    int iBuffLen, iLen;
    signed char c;

    for(int i = 0; i < 17; i++){
        str.sprintf("%d ", data[i]);
        strLog += str;
    }
    log->getLogPoint(_strLogName)->info(strLog);

    //校验是否符合宇通车牌第一及第二位，如果符合再校验后续长度是否为5
    //符合以上两个条件则认为是车牌号
    if((data[7] >= 1 && data[7] <= 31) && (data[8] >= 11 && data[8] <= 36)){
        iLen = 0;
        for(int i = 9; i < 14; i++){
            if(data[i] == 0xFF)
                break;
            c = data[i];
            if((c >= 0 && c <= 36) && c != 10)
                iLen++;
        }

        if(iLen == 5){
            log->getLogPoint(_strLogName)->info("CAR Detected");
            iValidLen = 7;
            return CAR_DETECT;
        }
    }

    //不符合车牌号的空字符串，必定不是VIN
    if(data[0] == '\0' || data[0] == 0xFF){
        log->getLogPoint(_strLogName)->info("VIN Invalid");
        return INVALID;
    }

    //符合VIN的字符范围为0-9或A-Z
    iBuffLen = 0;
    iLen = 0;
    for(int i = 0; i < 17; i++){
        if(data[i] == '\0' || data[i] == 0xFF)
            break;
        c = data[i];
        iBuffLen++;
        if((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z'))
            iLen++;
    }

    //小于14字节的认为是无效VIN
    if(iLen < 17){
        log->getLogPoint(_strLogName)->info(QString().sprintf("VIN Too Short valid len = %d", iLen));
        return INVALID;
    }
    //符合条件的VIN字符长度与实际长度相等，则VIN有效
    if(iLen == iBuffLen){
        log->getLogPoint(_strLogName)->info("VIN Detected");
        iValidLen = iLen;
        return VIN_DETECT;
    }

    log->getLogPoint(_strLogName)->info("ALL Invalid");
    return INVALID;
}

/*
 * 对电量进行修正，以直流功率除总有功功率，计算损耗比S，取连续两次
 * 电量E1，E2的差值E = E2 - E1，则该时间段内电量为E * S，虚拟电表电
 * 量Ev = Ev + E * S。此时总有功功率被修改为直流功率，电量值被修改为Ev。
 */
void RealDataFilter::AcMeter(TerminalStatus & Status)
{
	double fScale = 1.0, fDcPower, fEnergy = 0.0;
	int cIndex;

	if(m_iMeterType != 1 || 
		Status.cCanAddr > ID_MaxDCCanID || 
		Status.cCanAddr < ID_MinDCCanID ||
		!(Status.validFlag & 0x08)){
		return;
	}

	cIndex = Status.cCanAddr - ID_MinDCCanID;

	if(m_fVMeter[cIndex] <= 0.0 || m_fPreEnergy[cIndex] <= 0.0){
		if(!IsValidInitEnergy(cIndex))
			return;

		m_fVMeter[cIndex] = m_fPreEnergy[cIndex];

		log->getLogPoint(_strLogName)->info(
				QString().sprintf("CAN=%d Init Meter Real=%f Virtual=%f", 
					Status.cCanAddr, m_fPreEnergy[cIndex], m_fVMeter[cIndex]));
	}

	do{
		fDcPower = fabs(Status.stFrameRemoteMeSurement1.voltage_of_dc * Status.stFrameRemoteMeSurement1.current_of_dc * 0.001);
		if(fDcPower <= 0.0)
			break;

		Status.stFrameRemoteMeSurement1.active_power = fDcPower;
		if(m_fActivePower[cIndex] <= 0.0)
			break;

		if(m_fMeterScale[cIndex] > 0.0)
			fScale = m_fMeterScale[cIndex];
		else
			fScale = fDcPower / m_fActivePower[cIndex];

		if(fScale <= 0.0)
			break;

		if(fScale > 1.0)
			fScale = 1.0;

		if(m_fNowEnergy[cIndex] > m_fMaxMeterRange[cIndex] || m_fNowEnergy[cIndex] < 0.0){
			//记录电量异常，不进行处理
			log->getLogPoint(_strLogName)->info(
					QString().sprintf("CAN=%d [Range Error] NowEnergy=%f PreEnergy=%f", 
						Status.cCanAddr, m_fNowEnergy[cIndex], m_fPreEnergy[cIndex]));

			break;
		}

		if(m_fNowEnergy[cIndex] < m_fPreEnergy[cIndex]){
			//超过电表量程
			if(m_fNowEnergy[cIndex] < 10.0 && m_fPreEnergy[cIndex] > (m_fMaxMeterRange[cIndex] - 10.0)){
				fEnergy = m_fMaxMeterRange[cIndex] - m_fPreEnergy[cIndex];	
				fEnergy += m_fNowEnergy[cIndex];
				if(fEnergy > 0.0 && fEnergy < 20.0){
					m_fVMeter[cIndex] = m_fNowEnergy[cIndex];
					m_fPreEnergy[cIndex] = m_fNowEnergy[cIndex];

					UpdateVirtualMeter(cIndex);
				}

				log->getLogPoint(_strLogName)->info(
						QString().sprintf("CAN=%d OutRange D-Energy=%f NowEnergy=%f PreEnergy=%f", 
							Status.cCanAddr, fEnergy, m_fNowEnergy[cIndex], m_fPreEnergy[cIndex]));
			}else{
				//记录电量异常，不进行处理
				log->getLogPoint(_strLogName)->info(
						QString().sprintf("CAN=%d [Less Error] D-Energy=%f NowEnergy=%f PreEnergy=%f", 
							Status.cCanAddr, fEnergy, m_fNowEnergy[cIndex], m_fPreEnergy[cIndex]));
			}
		}else{
			fEnergy = m_fNowEnergy[cIndex] - m_fPreEnergy[cIndex];
			if(fEnergy > 0.0 && fEnergy < 20.0){
				m_fVMeter[cIndex] += fEnergy * fScale;
				m_fPreEnergy[cIndex] = m_fNowEnergy[cIndex];

				UpdateVirtualMeter(cIndex);

				log->getLogPoint(_strLogName)->info(
						QString().sprintf("CAN=%d Meter Scale=%f D-Energy=%f Real=%f Virtual=%f", 
							Status.cCanAddr, fScale, fEnergy, m_fNowEnergy[cIndex], m_fVMeter[cIndex]));
			}

			if(fEnergy >= 20.0){
				//记录电量异常，不进行处理
				log->getLogPoint(_strLogName)->info(
						QString().sprintf("CAN=%d [Large Error] Meter Scale=%f D-Energy=%f Real=%f Virtual=%f", 
							Status.cCanAddr, fScale, fEnergy, m_fNowEnergy[cIndex], m_fVMeter[cIndex]));
			}
		}
	}while(false);

	if(Status.cStatus != CHARGE_STATUS_CHARGING){
		fEnergy = m_fNowEnergy[cIndex] - m_fPreEnergy[cIndex];	
		if(fEnergy > 0.0){
			m_fVMeter[cIndex] += fEnergy * fScale;
			m_fPreEnergy[cIndex] = m_fNowEnergy[cIndex];
			UpdateVirtualMeter(cIndex);

			log->getLogPoint(_strLogName)->info(
					QString().sprintf("CAN=%d [Correct V-Meter] Meter Scale=%f D-Energy=%f Now=%f Virtual=%f", 
						Status.cCanAddr, fScale, fEnergy, m_fNowEnergy[cIndex], m_fVMeter[cIndex]));
		}
	}

	Status.stFrameRemoteMeSurement2.active_electric_energy = (int)(m_fVMeter[cIndex] * 100.0);
}

/*
 * 确定电量是否有效，连续两个电量的电量差大于0小于20认为电量有效，
 * 无效时丢弃前一个电量，直到符合以上条件认为电量有效。
 * 无效时虚拟电表不开始计量，所有电量都是实际电量，所以也不需要考虑
 * 量程问题，此时量程由上层处理。
 */
bool RealDataFilter::IsValidInitEnergy(uchar cIndex)
{
    double fEnergy;

    if(m_fNowEnergy[cIndex] >= m_fMaxMeterRange[cIndex] || m_fNowEnergy[cIndex] < 0.0){
        return false;
    }

    if(m_fPreEnergy[cIndex] <= 0.0){
        m_fPreEnergy[cIndex] = m_fNowEnergy[cIndex];
        return false;
    }

    fEnergy = m_fNowEnergy[cIndex] - m_fPreEnergy[cIndex];

    if(fEnergy > 0.0 && fEnergy < 20.0){
        log->getLogPoint(_strLogName)->info(
                    QString().sprintf("CAN=%d Init Energy NowEnergy=%f PreEnergy=%f D-Energy=%f",
                                      cIndex + ID_MinDCCanID, m_fNowEnergy[cIndex], m_fPreEnergy[cIndex], fEnergy));
        return true;
    }

    m_fPreEnergy[cIndex] = m_fNowEnergy[cIndex];
    return false;
}

/*
 * 记录VirtualMeter计量值，记录真实电表值
 */
void RealDataFilter::UpdateVirtualMeter(uchar cIndex)
{
    QSettings setting(VIRTUAL_METER_CONF, QSettings::IniFormat);
    QString strKey, strValue;

    strKey.sprintf("%d/VirtualMeter", cIndex + ID_MinDCCanID);
    strValue.sprintf("%f", m_fVMeter[cIndex]);
    setting.setValue(strKey, strValue);

    strKey.sprintf("%d/RealMeter", cIndex + ID_MinDCCanID);
    strValue.sprintf("%f", m_fPreEnergy[cIndex]);
    setting.setValue(strKey, strValue);
}

/*
 * 交流计量开关关闭后，在数据无效的时候重置MeterState为0，
 * 防止集控器异常退出导致的数据记录错误
 */
void RealDataFilter::InitVirtualMeter()
{
    QSettings setting(VIRTUAL_METER_CONF, QSettings::IniFormat);
    QString strGroup;

    log->getLogPoint(_strLogName)->info(
                QString().sprintf("Load MeterType=%d", m_iMeterType));

    if(m_iMeterType == 0){
        //开关关闭需清除配置，防止开关再打开时数据不一致
        for(int i = 0; i < setting.childGroups().count(); i++){
            strGroup = setting.childGroups().at(i);
            setting.beginGroup(strGroup);
            setting.setValue("VirtualMeter", "0.0");
            setting.setValue("RealMeter", "0.0");
            setting.endGroup();

            log->getLogPoint(_strLogName)->info(QString().sprintf("CAN=%s Reset Virtual-Meter=0.0", strGroup.toAscii().data()));
        }
    }else{
        uchar cIndex;
        for(int i = ID_MinDCCanID; i <= ID_MaxDCCanID; i++){
            cIndex = i - ID_MinDCCanID;
            strGroup.sprintf("%d", i);

            m_fActivePower[cIndex] = 0.0;
            m_fNowEnergy[cIndex] = 0.0;

            setting.beginGroup(strGroup);
            m_fVMeter[cIndex] = setting.value("VirtualMeter", "0.0").toFloat();
            m_fPreEnergy[cIndex] = setting.value("RealMeter", "0.0").toFloat();
            m_fMaxMeterRange[cIndex] = param->getAmmeterRange(i);
            m_fMeterScale[cIndex] = param->getAcMeterScale(i) / 100.0;
            setting.endGroup();

            log->getLogPoint(_strLogName)->info(QString().sprintf("CAN=%d Load Virtual-Meter=%f Real-Meter=%f MaxRange=%f Scale=%f",
                                                                       i, m_fVMeter[cIndex], m_fPreEnergy[cIndex], m_fMaxMeterRange[cIndex], m_fMeterScale[cIndex]));
        }
    }
}
//判断CCU是否离线
void RealDataFilter::CCUOffLineCheck()
{
    QDateTime time = QDateTime::currentDateTime();   //获取当前时间
    unsigned int ui_time = time.toTime_t();   //将当前时间转为时间戳
    unsigned int ui_interval=0;
    CCUStatusOnline ccuStatus;

    for(int i=240;i>230;i--)
    {
        if(devCache->QueryCCUStatus(i,ccuStatus))
        {
            ui_interval = ui_time - ccuStatus.ui_time;
            if(ui_interval > 40) //40s没有接收到数据，认为CCU离线
            {
                ccuStatus.ui_time = 0;
                //告警CCU离线
                stDCCabinetDatas &dccab = devCache->GetUpdateDCCabinetMeter(); //GetUpdateDCCabinetMeter(void);
                dccab.ccuMap.remove(i); //删除CCU数据
                dccab.dcmoduleMap.remove(i);
                dccab.pduMap.remove(i);
                dccab.branchMap.remove(i);
                dccab.terminalMap.remove(i);
                devCache->FreeUpdateDCCabinetMeter();
            }
        }
    }
}


void RealDataFilter::energyPlanDevOnlineCheck_DCDC_ES()
{
    DCDC_ESAllDataMap::iterator mapit;
    QMap<QByteArray, stEnergyStorageDCDCSingle>::iterator it, itbak;

    stEnergyPlanDatas &Status = devCache->GetUpdateEnergyPlanMeter();
    DCDC_ESAllDataMap &map = Status.dcdcModMap_es;
    //counter add
    for(mapit = map.begin(); mapit != map.end(); ++mapit)
    {
        for(it = mapit.value().mapSingle.begin(); it != mapit.value().mapSingle.end(); ++it)
        {
            it.value().onlineCounter++;
        }
    }

    //online check
    for(mapit = map.begin(); mapit != map.end(); ++mapit)
    {
        if(mapit.value().mapSingle.isEmpty())
        {
            continue;
        }

        for(it = mapit.value().mapSingle.begin(); it != mapit.value().mapSingle.end(); ++it)
        {
            if(it.value().onlineCounter > 8)
            {
                //发送设备离线主题
                InfoMap tempMap;
                TDevice device;
                QByteArray array;
                memset(&device, 0x00, sizeof(device));
                device.enDeviceType = DEV_ENERGY_STORAGE_CAB_DCDC_MODULE;
                device.enSubDeviceType = SUB_DEV_UNKNOWN;
                device.iCanID = *(unsigned short*)(it.key().data()) + mapit.key() * 1000;
                device.iParentID =  mapit.key();
                array.append((char *)&device, sizeof(device));
                tempMap.insert(Addr_EnergyPlan_Dev,array);
                emit sigToBus(tempMap, AddrType_EnergyPlanDevOffline);
                //缓存中删除该节点
                itbak = it;
                it++;

                mapit.value().mapSingle.erase(itbak);

                //                mapit.value().mapSingle.remove(it.key());
                //                if(mapit.value().mapSingle.isEmpty())
                //                {
                //                    break;
                //                }
                //                else
                //                {
                //                    it = mapit.value().mapSingle.begin();
                //                }
            }
        }
    }

    devCache->FreeUpdateEnergyPlantMeter();

}

/*
 *南京新协议要求遥测数据突发。所有的遥测数据采用总线通知会造成总线传输压力，
 *同时还需要保证数据的实时性，采用回调函数的方式传输实时数据。
 *该函数只有协议模块使用。
 */
void RealDataFilter::setRealDataCallBack(realDataFunc func)
{
    if(func){
        realDataCallBack = func;
    }
}

/*
 * 动态库接口函数，初始化模块
 * pThread	输入 模块将要运行的线程
 * 返回值 	0表示无错误，-1表示有错误
 */
int RealDataFilter::InitModule(QThread* pThread)
{
    return 0;
}

/*
 * 动态库接口函数，向BUS注册模块
 * pBus		输入 BUS模块指针
 * 返回值 	0表示无错误，-1表示有错误
 */
int RealDataFilter::RegistModule()
{
    return 0;
}

/*
 * 动态库接口函数，启动模块
 * 返回值 	0表示无错误，-1表示有错误
 */
int RealDataFilter::StartModule()
{
    return 0;
}

/*
 * 动态库接口函数，停止模块
 * 返回值 	0表示无错误，-1表示有错误
 */
int RealDataFilter::StopModule()
{
    return 0;
}

/*
 * 动态库接口函数，模块工作状态
 * 返回值 	0表示无错误，-1表示有错误
 */
int RealDataFilter::ModuleStatus()
{
    return 0;
}

void RealDataFilter::loadCouple300KWSetting()
{
    struct db_result_st result;
	QString strSql;

	if(!_b300KWEnable)
		return;

	strSql.sprintf("SELECT canaddr, autochargenum FROM terminal_autocharge_set_table ORDER BY canaddr ASC;");

    if(db->DBSqlQuery(strSql.toAscii().data(), &result, DB_PARAM) != 0)
		return;

	int canAddr, gunNum, linkNum;
	for(int i = 0; i < result.row; i = i + 2){
		canAddr = atoi(result.result[result.column * i]);
		gunNum = atoi(result.result[result.column * i + 1]);
		if(gunNum != 2)
			continue;

		QByteArray *ar = new QByteArray();
		ar->append(gunNum);
		for(int j = 0; j < gunNum; j++){
			ar->append(canAddr + j);
		}

		linkNum = 0;
		ar->append(linkNum);
		for(int j = 0; j < gunNum; j++){
			TerminalStatus status;
			memset(&status, 0, sizeof(TerminalStatus));
            devCache->QueryTerminalStatus(canAddr + j, status);
			if(status.stFrameRemoteSingle.link_status == 1)
				linkNum++;

			ar->remove(gunNum + 1, 1);
			ar->append(linkNum);

			_mapGun[canAddr + j] = ar;
		}
	}

    db->DBQueryFree(&result);
}

void RealDataFilter::couple300KWGunState(int canAddr, bool link)
{
	if(!_b300KWEnable || !_mapGun.contains(canAddr))
		return;

	QString str;
	QByteArray *ar;
	int gunNum = 0, linkNum = 0, maxIndex = 0;

	ar = _mapGun[canAddr];
	gunNum = ar->at(0);
	maxIndex = gunNum + 1;

	str.sprintf("300KW双枪 CAN=%d 连接状态=%d", canAddr, link);
    log->getLogPoint(_strLogName)->info(str);

	if(ar->count() > maxIndex){
		linkNum = ar->at(maxIndex);
	}else{
		ar->append(linkNum);
	}

	if(link){
		linkNum++;
	}else{
		if(linkNum > 0)
			linkNum--;
	}

	ar->remove(maxIndex, 1);
	ar->append(linkNum);

	str.sprintf("300KW双枪 设置枪数量=%d 已插枪数量=%d", gunNum, linkNum);
    log->getLogPoint(_strLogName)->info(str);

	if(gunNum == linkNum){

		InfoMap map;
		map[Addr_ChargeGunType] = QByteArray(1, gunNum); 
		map[Addr_CanID_Comm] = QByteArray(1, ar->at(1)); 
		map[Addr_ChargeGun_Master] = QByteArray(1, ar->at(1));
		map[Addr_ChargeGun_Slave1] = QByteArray(1, ar->at(2));
		map[Addr_300kw_WorkMode] = QByteArray(1, 2);
		
		emit sigToBus(map, AddrType_CheckChargeManner);
		emit sigToBus(map, AddrType_DoubleSys300kwSetting_Publish);
	}
}
