#include <stdio.h>
#include <stdlib.h>

#include "DevCache/DevCache.h"

DevCache::DevCache()
{
	int i = 0;
	TerminalStatus Status;
	unParamConfig config;

	defaultStatus.cCanAddr = 0xff;
    
	memset(&CSCURealData.realData, 0x0, sizeof(RealStatusData));

	paramSet = ParamSet::GetInstance();
	dbOperate = DBOperate::GetInstance();

	paramSet->querySetting(&config, PARAM_CSCU_SYS);

    chargerCCUNum = config.cscuSysConfig.CCUnum;
	chargerDCNum = config.cscuSysConfig.directCurrent;
	chargerAC1Num = config.cscuSysConfig.singlePhase;
	chargerAC3Num = config.cscuSysConfig.threePhase;

    memset(&Status, 0, sizeof(TerminalStatus));

    for(i=0; i< chargerAC1Num + chargerAC3Num + chargerDCNum +chargerCCUNum ; i++)
		DevStatusList.append(Status);

	for(i = 0; i < chargerAC1Num; i++)
		ResetTerminalStatus( i+1 );
	for(i = 0; i < chargerAC3Num; i++)
		ResetTerminalStatus( i+151 );
	for(i = 0; i < chargerDCNum; i++)
		ResetTerminalStatus( i+181 );
	/*数据库更新*/
	loadTerminalStatus();
	loadChargeStep();
//	QObject::connect(&timer, SIGNAL(timeout()), this, SLOT(update()));
//	timer.start(100);
}

DevCache::~DevCache (void)
{
	DevStatusList.clear();
	DevChargeStepList.clear();
}

DevCache *DevCache::GetInstance()
{
	static DevCache* ins = NULL;
	if(!ins){
		ins = new DevCache();
	}

	return ins;
}

/**
 *加载存储的充电状态数据
 */
void DevCache::loadChargeStep(void)
{
	struct db_blob_result_st blob_result;
	char cmd_buff[256];
	CHARGE_STEP Status;

	snprintf(cmd_buff, sizeof(cmd_buff), "select data from charge_save_table");
	dbOperate->DBSqlQueryBlob(cmd_buff, &blob_result, DB_DOWN_SAVE);
	
	for( int i=0; i<blob_result.blob_list.size(); i++ )
	{
		struct db_blob_st blob = blob_result.blob_list.at(i);

		if(blob.data && blob.size == sizeof(CHARGE_STEP))
		{
			memcpy((unsigned char *)&Status, blob.data, blob.size);
			
			int canaddr = Status.ucCanAddr;
			AddChargeStep(canaddr, Status);
		}
	}
	
	dbOperate->DBQueryFreeBlob(&blob_result); 
}

/**
 *加载存储的TerminalStatus数据
 */
void DevCache::loadTerminalStatus(void)
{
	struct db_blob_result_st blob_result;
	char cmd_buff[256];
	TerminalStatus Status;

	for(int k=0; k< 3; k++)
	{
		if(k == 0)
		{
			if(chargerAC1Num == 0)
				continue;
			snprintf(cmd_buff, sizeof(cmd_buff), "select data from status_save_table where canaddr > 0 and canaddr <= %d", chargerAC1Num);
		}
		else if(k == 1)
		{
			if(chargerAC3Num == 0)
				continue;
			snprintf(cmd_buff, sizeof(cmd_buff), "select data from status_save_table where canaddr > 150 and canaddr <= %d", 150+chargerAC3Num);
		}
		else if(k == 2)
		{
			if(chargerDCNum == 0)
				continue;
			snprintf(cmd_buff, sizeof(cmd_buff), "select data from status_save_table where canaddr > 180 and canaddr <= %d", 180+chargerDCNum);
		}
		dbOperate->DBSqlQueryBlob(cmd_buff, &blob_result, DB_DOWN_SAVE);
		for( int i=0; i<blob_result.blob_list.size(); i++ )
		{
			struct db_blob_st blob = blob_result.blob_list.at(i);

			if(blob.data && blob.size == sizeof(TerminalStatus))
			{
				memcpy((unsigned char *)&Status, blob.data, blob.size);
			
				Status.validFlag = 0;
				
				TerminalStatus &devStatus = GetUpdateTerminalStatus(Status.cCanAddr);
				if(devStatus.cCanAddr != 0xff)
				{
					devStatus = Status;
					FreeUpdateTerminalStatus();

					TerminalStatus st_TerminalStatus;
					QueryTerminalStatus(Status.cCanAddr, st_TerminalStatus);
				}
			}
		}
		dbOperate->DBQueryFreeBlob(&blob_result); 
	}
}

/**
 *通过can地址查找位置
 */
int DevCache::findPositionViaCanaddr(unsigned char ucCanAddr)
{
	int i = -1;
	if(ucCanAddr <= 110 && ucCanAddr > 0)
	{
		if(ucCanAddr <= chargerAC1Num)
			i = ucCanAddr - 1;
	}
	else if(ucCanAddr > 150 && ucCanAddr <= 180)
	{
		if(ucCanAddr <= (chargerAC3Num+150))
			i = chargerAC1Num + ucCanAddr - 151;
	}
	else if(ucCanAddr > 180 && ucCanAddr <= 230)
	{
		if(ucCanAddr <= (chargerDCNum+180))
			i = chargerAC1Num + chargerAC3Num + ucCanAddr - 181;
	}
    else if(ucCanAddr > 230 && ucCanAddr <= 240)
    {
        if(ucCanAddr > (240 - chargerCCUNum))
            i = chargerAC1Num + chargerAC3Num + chargerDCNum +  240 - ucCanAddr;
    }

	return i;
}

/**
 *查询状态数据
 */
bool DevCache::QueryTerminalStatus(unsigned char ucCanAddr, TerminalStatus &st_TerminalStatus)
{
	int Ret;
	bool backValue = false;
	Ret = findPositionViaCanaddr(ucCanAddr);
	if( Ret > -1 && !DevStatusList.isEmpty())	
	{
		mutexDevStatus.lock();	
		st_TerminalStatus = DevStatusList.at(Ret);
		//TerminalStatus Status = DevStatusList.at(Ret);
		//memcpy(&st_TerminalStatus, &Status, sizeof(st_TerminalStatus));
		mutexDevStatus.unlock();
		backValue = true;
	}

	return backValue;
}

/**
 *清空状态数据
 */
bool DevCache::ResetTerminalStatus(unsigned char ucCanAddr)
{
	int Ret;
	bool BackValue = false;
	TerminalStatus Status;

	Ret = findPositionViaCanaddr(ucCanAddr);
	if(Ret > -1)
	{
		memset(&Status, 0, sizeof(TerminalStatus));
		Status.cCanAddr = ucCanAddr;
		Status.stFrameRemoteSingle.charge_status = 6;		//默认离线离线
		Status.cStatus = CHARGE_STATUS_DISCONNECT;
		
		if(Ret < chargerAC1Num)
			Status.cType = 0;
		else if(Ret >= chargerAC1Num && Ret < (chargerAC1Num + chargerAC3Num))
			Status.cType = 1;
		else if( Ret >= (chargerAC1Num + chargerAC3Num) 
				&& Ret < (chargerDCNum + chargerAC1Num + chargerAC3Num) )
			Status.cType = 2;

		mutexDevStatus.lock();	
		DevStatusList.replace(Ret, Status);	
		mutexDevStatus.unlock();	

		BackValue = true;
	}

	return BackValue;
}

/**
 *保存变化的状态数据
 */
bool DevCache::SaveTerminalStatus(unsigned char ucCanAddr)
{
	bool ret = false;
	TerminalStatus Status;
	if(QueryTerminalStatus(ucCanAddr, Status))
	{
		char cmd_buff[256];
		struct db_result_st result;
		snprintf(cmd_buff, sizeof(cmd_buff), "select id from status_save_table where canaddr=\"%d\" limit 1", ucCanAddr);
		if(0 == dbOperate->DBSqlQuery((char *)cmd_buff, &result, DB_DOWN_SAVE))
		{
			if(result.row == 0 && result.column == 0)
				snprintf(cmd_buff, sizeof(cmd_buff), "insert into status_save_table (canaddr, data) values(\"%d\", ?)", ucCanAddr);
			else
				snprintf(cmd_buff, sizeof(cmd_buff), "update status_save_table set data=? where canaddr=\"%d\"", ucCanAddr);
			dbOperate->DBSqlExecBlob(cmd_buff, (unsigned char *)&Status, sizeof(TerminalStatus), DB_DOWN_SAVE);
			ret = true;
		}	
		dbOperate->DBQueryFree(&result);
	}

	return ret;
}

/**
 *获取更新, 获取失败时，返回的 参数中canaddr是0xff
 */
TerminalStatus &DevCache::GetUpdateTerminalStatus(unsigned char ucCanAddr)
{
	//TerminalStatus Status;
	//Status.cCanAddr = 0xff;

	int Ret = findPositionViaCanaddr(ucCanAddr);
	if(Ret == -1)
	{
		defaultStatus.cCanAddr = 0xff;
		return defaultStatus;
	}

	mutexDevStatus.lock();
	return  DevStatusList.operator[](Ret);	
}
/**
 * 释放更新
 */
void DevCache::FreeUpdateTerminalStatus()
{
	mutexDevStatus.unlock();
}
/**
 *根据can地址查找充电缓存区的位置
 */
int DevCache::FindChargeStepPosition(unsigned char canAddr)
{
	mutexDevChargeStep.lock();
	for(int i=0; i< DevChargeStepList.size(); i++)
	{
		CHARGE_STEP Status = DevChargeStepList.at(i);
		if(Status.ucCanAddr == canAddr)
		{
			mutexDevChargeStep.unlock();
			return i;
		}
	}
	mutexDevChargeStep.unlock();

	return -1;
}
/**
 * 添加充电状态缓存
 */
bool DevCache::AddChargeStep(unsigned char canAddr, CHARGE_STEP &st_ChargeStep)
{
	canAddr = canAddr;
	mutexDevChargeStep.lock();
	DevChargeStepList.append(st_ChargeStep);	
	mutexDevChargeStep.unlock();
	return true;
}

/**
 *删除充电状态缓存
 */
bool DevCache::DeleteChargeStep(unsigned char canAddr)
{
	int ret = FindChargeStepPosition(canAddr);
	if(ret == -1)
		return false;

	mutexDevChargeStep.lock();
	DevChargeStepList.removeAt(ret);	
	mutexDevChargeStep.unlock();

	char cmd_buff[256];
	snprintf(cmd_buff, sizeof(cmd_buff), "delete from charge_save_table where canaddr=\"%d\"", canAddr);
	dbOperate->DBSqlExec(cmd_buff, DB_DOWN_SAVE);

	return true;
}

/**
 *充电状态缓存查询
 */
bool DevCache::QueryChargeStep(unsigned char canAddr, CHARGE_STEP &st_ChargeStep)
{
	int Ret;
	bool backValue = false;

	Ret = FindChargeStepPosition(canAddr);
	if( Ret > -1 && !DevChargeStepList.isEmpty())	
	{
		mutexDevChargeStep.lock();	
		st_ChargeStep = DevChargeStepList.at(Ret);
		mutexDevChargeStep.unlock();

		backValue = true;
	}

	return backValue;
}
/**
 *更新充电状态数据
 */
bool DevCache::UpateChargeStep(unsigned char canAddr, CHARGE_STEP &st_ChargeStep)
{
	int Ret;
	Ret = FindChargeStepPosition(canAddr);
	if( Ret > -1 && !DevChargeStepList.isEmpty())	
	{
		mutexDevChargeStep.lock();	
		DevChargeStepList.replace(Ret, st_ChargeStep);
		mutexDevChargeStep.unlock();
		return true;
	}
	return false;
}
/**
 *充电过程数据保存
 */
bool DevCache::SaveChargeStep(unsigned char ucCanAddr)
{
	bool ret=false;
	CHARGE_STEP Status;
	if(QueryChargeStep(ucCanAddr, Status))
	{
		char cmd_buff[256];
		struct db_result_st result;
		
		snprintf(cmd_buff, sizeof(cmd_buff), "select id from charge_save_table where canaddr=\"%d\" limit 1", ucCanAddr);
		if(0 == dbOperate->DBSqlQuery((char *)cmd_buff, &result, DB_DOWN_SAVE))
		{
			if(result.row ==0 && result.column == 0)
				snprintf(cmd_buff, sizeof(cmd_buff), "insert into charge_save_table (canaddr, data) values(\"%d\", ?)", ucCanAddr);
			else
				snprintf(cmd_buff, sizeof(cmd_buff), "update charge_save_table set data=? where canaddr=\"%d\"", ucCanAddr);
			dbOperate->DBSqlExecBlob(cmd_buff, (unsigned char *)&Status, sizeof(CHARGE_STEP), DB_DOWN_SAVE);
			ret = true;
		}	
		dbOperate->DBQueryFree(&result);
#if 0
		
		char cmd_buff[256];
		snprintf(cmd_buff, sizeof(cmd_buff), "replace into charge_save_table (canaddr, data) values(%d, ?)", ucCanAddr);
		dbOperate->DBSqlExecBlob(cmd_buff, (unsigned char *)&Status, sizeof(CHARGE_STEP), DB_DOWN_SAVE);
		return true;
#endif
	}
	
	return ret;
}
/*
void DevCache::QueryDCDCModuleInfo(QByteArray key,QVariant &var)
{
    QByteArray id;
    if(key.at(1) == 1)
    {
        stDCDCInfo def;		//默认为空
        memset(&def, 0, sizeof(stDCDCInfo));
        id=key.mid(2,2);
        var.setValue( CSCURealData.dcDc.dcDcData.value(id, def));
    }
}

//ACDC四象限柜信息
void DevCache::QueryQuadrantCabinetInfo(QByteArray key,QVariant &var)
{
    QByteArray id;
    switch(key.at(1))
    {
    case 0://柜子开关量
    {
        stFourQuadrantCabinetInfo def;		//默认为空
        memset(&def, 0, sizeof(stFourQuadrantCabinetInfo));
        id=key.mid(0,1);
        var.setValue( CSCURealData.fourQuadrant.fourQuadrantData.value(id, def));
    }
        break;
    case 1://ACDC模块信息
    {
        stACDCInfo def;		//默认为空
        memset(&def, 0, sizeof(stACDCInfo));
        id=key.mid(2,2);
        var.setValue( CSCURealData.acDc.acDcData.value(id, def));
    }
        break;
    case 2://温湿度计
    {
        stHumitureInfo def;		//默认为空
        memset(&def, 0, sizeof(stACDCInfo));
        id=key.mid(2,2);
        var.setValue( CSCURealData.humiture.humitureData.value(id, def));
    }
        break;
    default:
        break;
    }
}

void DevCache::QueryMainDistributionCabinetInfo(QByteArray key,QVariant &var)
{
    QByteArray id;
    switch(key.at(1))
    {
    case 0://柜子开关量
    {
        stMainDistributionCabinetInfo def;		//默认为空
        memset(&def, 0, sizeof(stMainDistributionCabinetInfo));
        id = key.mid(0,1);
        var.setValue( CSCURealData.mainDis.mainDisData.value(id, def));
    }
        break;
    case 2://温湿度计
    {
        stHumitureInfo def;		//默认为空
        memset(&def, 0, sizeof(stACDCInfo));
        id = key.mid(2,2);
        var.setValue( CSCURealData.humiture.humitureData.value(id, def));
    }
        break;
    default:
        break;
    }
}

void DevCache::QuerySysControlCabinetInfo(QByteArray key,QVariant &var)
{
    QByteArray id;
    switch(key.at(1))
    {
    case 0://柜子开关量
    {
        stSysControlCabinetInfo def;		//默认为空
        memset(&def, 0, sizeof(stSysControlCabinetInfo));
        id = key.mid(0,1);
        var.setValue( CSCURealData.sysControl.sysControlData.value(id, def));
    }
        break;
    case 2://温湿度计
    {
        stHumitureInfo def;		//默认为空
        memset(&def, 0, sizeof(stACDCInfo));
        id = key.mid(2,2);
        var.setValue( CSCURealData.humiture.humitureData.value(id, def));
    }
        break;
    default:
        break;
    }
}
void DevCache::QueryPhotoVoltaicCabinetInfo(QByteArray key,QVariant &var)
{
    QByteArray id;
    switch(key.at(1))
    {
    case 0://柜子开关量
    {
        stPhotoVoltaicCabinetInfo def;		//默认为空
        memset(&def, 0, sizeof(stPhotoVoltaicCabinetInfo));
        id = key.mid(0,1);
        var.setValue( CSCURealData.photoVoltaic.photoVoltaicData.value(id, def));
    }
        break;
    case 1://功率优化器信息
    {
        stPowerOptimizerInfo def;		//默认为空
        memset(&def, 0, sizeof(stPowerOptimizerInfo));
        id = key.mid(2,2);
        var.setValue( CSCURealData.powerOptimizer.powerOptimizerData.value(id, def));
    }
    case 2://温湿度计
    {
        stHumitureInfo def;		//默认为空
        memset(&def, 0, sizeof(stACDCInfo));
        id = key.mid(2,2);
        var.setValue( CSCURealData.humiture.humitureData.value(id, def));
    }
        break;
    default:
        break;
    }
}
void DevCache::QueryEnergyStorageCabinetInfo(QByteArray key,QVariant &var)
{
    QByteArray id;
    switch(key.at(1))
    {
    case 0://柜子开关量
    {
        stEnergyStorageCabinetInfo def;		//默认为空
        memset(&def, 0, sizeof(stEnergyStorageCabinetInfo));
        id = key.mid(0,1);
        var.setValue( CSCURealData.energyStorage.energyStorageData.value(id, def));
    }
        break;
    case 1://电池信息
    {
        stEnergyStorageCabinetBatteryInfo def;		//默认为空
        memset(&def, 0, sizeof(stEnergyStorageCabinetBatteryInfo));
        id = key.mid(2,2);
        var.setValue( CSCURealData.battery.batteryData.value(id, def));
    }
    case 2://温湿度计
    {
        stHumitureInfo def;		//默认为空
        memset(&def, 0, sizeof(stACDCInfo));
        id = key.mid(2,2);
        var.setValue( CSCURealData.humiture.humitureData.value(id, def));
    }
        break;
    default:
        break;
    }
}
void DevCache::QueryEnergyPlanEnv(QVariant &var, QVariant &param)
{//查询柜子信息用：柜子can地址+信息类型+设备ID，其中设备ID为2字节can地址+设备地址，对于柜子信息只有can地址
 //比如ACDC四象限变化柜，有模块信息／开关量信息／温湿度信息
 //查询模块信息，用　柜子can地址＋信息类型＋模块id组合
    QByteArray key = param.value<QByteArray>();
    if(key.size() != 4)
        return;

     if(key.at(0)>=ID_MinFourQuadrantConverterCabinet && key.at(0)<=ID_MaxFourQuadrantConverterCabinet)
     {
         QueryQuadrantCabinetInfo(key,var);
     }
     else if(key.at(0)>=ID_MinACLoadDistributionCabinet && key.at(0)<=ID_MaxACLoadDistributionCabinet)
     {
         QueryMainDistributionCabinetInfo(key,var);
     }
     else if(key.at(0)>=ID_MinSystemDistributionCabinet && key.at(0)<=ID_MaxSystemDistributionCabinet)
     {
         QuerySysControlCabinetInfo(key,var);
     }
     else if(key.at(0)>=ID_MinDCPhotovoltaicControlCabinet && key.at(0)<=ID_MaxDCPhotovoltaicControlCabinet)
     {
         QueryPhotoVoltaicCabinetInfo(key,var);
     }
    else if(key.at(0)>=ID_MinDCEnergyStorageCabinet && key.at(0)<=ID_MaxDCEnergyStorageCabinet)
    {
         QueryEnergyStorageCabinetInfo(key,var);
    }
     else
     {
         QueryDCDCModuleInfo(key,var);
     }
}
*/


/**
 *查询cscu试试状态数据
 *参数：type 要查询的数据类型  param 要查询数据类型的参数
 *返回值： var 没有查到时为空
 *	       CACHE_STATUS				 返回RealStatusData类型值
 *	       CACHE_INLINE_AMMETER		 返回stAmmeterData类型值
 *	       CACHE_CCUDATA			 返回CCUAllRealData
 */
bool DevCache::QueryRealStatusMeter(QVariant &var, int type, QVariant &param)
{
	bool ret = true;
    stAmmeterData ammeterData;
	mutexCSCURealData.lock();
	switch(type)
	{
		case CACHE_STATUS:
			{
				var.setValue(CSCURealData.realData); 
			}break;
		case CACHE_INLINE_AMMETER:
			{
				stAmmeterData def;		//默认为空
				memset(&def, 0, sizeof(stAmmeterData));

                QByteArray key = param.value<QByteArray>();
				var.setValue( CSCURealData.inLineAmmeter.ammeterData.value(key, def));
                ammeterData = var.value<stAmmeterData>();
			}break;
        case CACHE_ENERGYPLAN_ENV:
        {
            ;//QueryEnergyPlanEnv(var,param);
        }break;
        case CACHE_CCUDATA:
        {
            var.setValue(CSCURealData.ccuAllData);
        }break;
		default: 
			ret = false;
			break;
	}
	mutexCSCURealData.unlock();

	return ret;
}
/**
 *更新cscu实时状态数据
 */
RealStatusMeterData &DevCache::GetUpdateRealStatusMeter(void)
{
	mutexCSCURealData.lock();	
	return CSCURealData;
}
/**
 *更新结束后释放
 */
void DevCache::FreeUpdateRealStatusMeter(void)
{
	mutexCSCURealData.unlock();
}

/**
 * 直流柜数据更新获取
 */
stDCCabinetDatas &DevCache::GetUpdateDCCabinetMeter(void)
{
    mutexDCCabinet.lock();
	return DCCabinetDatas;
}
/**
 *直流柜数据更新释放
 */
void DevCache::FreeUpdateDCCabinetMeter(void)
{
    mutexDCCabinet.unlock();
}

//能效系统数据更新获取
stEnergyPlanDatas &DevCache::GetUpdateEnergyPlanMeter(void)
{
    mutexEnergyPlan.lock();
    return energyPlanDatas;
}

//能效数据更新释放
void DevCache::FreeUpdateEnergyPlantMeter(void)
{
    mutexEnergyPlan.unlock();
}

/**
 *直流柜数据查询
 *参数 canAddr		can地址
 *	   id			保留
 *	   var          返回的结果， Map形式
 *	   type         查询的类型， 代表ccu、pdu、分支箱、直流模块、终端数据
 */
bool DevCache::QueryDCCabinetMeter(unsigned char canAddr, QByteArray id, QVariant &var, int type)
{
	id = id;
	bool ret = true;
	
	mutexDCCabinet.lock();
	switch(type)
	{
		case CACHE_DCCCUDATA:
            {
				if( true ==  DCCabinetDatas.ccuMap.contains(canAddr) )
                    var.setValue(DCCabinetDatas.ccuMap[canAddr]);
                else
                    ret = false;
			}break;
		case CACHE_DCPDUDATA:
			{
                if( true ==  DCCabinetDatas.pduMap.contains(canAddr) )
                {
                    PDUAllDatasMap::iterator it;
                    var.setValue(DCCabinetDatas.pduMap[canAddr]);
                }
				else
                {
					ret = false;
                }
			}break;
		case CACHE_DCBRANCHDATA:
			{
				if(true == DCCabinetDatas.branchMap.contains(canAddr))
					var.setValue(DCCabinetDatas.branchMap[canAddr]);
				else
					ret = false;
			}break;
		case CACHE_DCMODULEDATA:
			{
				if(true == DCCabinetDatas.dcmoduleMap.contains(canAddr))
					var.setValue(DCCabinetDatas.dcmoduleMap[canAddr]);
				else
					ret = false;
			}break;
		case CACHE_TERMDATA:   //终端数据不区分can地址，直接返回map
			{
				var.setValue(DCCabinetDatas.terminalMap);
			}break;
    case CACHE_DEVICESPECIFICATIONDATA:
            if(true == DCCabinetDatas.ccuDeviceMap.contains(canAddr))
                var.setValue(DCCabinetDatas.ccuDeviceMap[canAddr]);
            else
                ret = false;
        break;
		default: 
			ret = false;
			break;
	}
	mutexDCCabinet.unlock();

	return ret;
}

/**
 *能效数据查询
 *参数 canAddr		can地址
 *	   id			保留
 *	   var          返回的结果， Map形式
 *	   type         查询的类型， 代表ccu、pdu、分支箱、直流模块、终端数据
 */
bool DevCache::QueryEnergyPlanMeter(unsigned char canAddr, QByteArray id, QVariant &var, int type)
{
    id = id;
    bool ret = true;

    mutexEnergyPlan.lock();
    switch(type)
    {
    case CACHE_ES_CAB_DATA:
    {
        if( true ==  energyPlanDatas.esCabMap.contains(canAddr) )
            var.setValue(energyPlanDatas.esCabMap[canAddr]);
        else
            ret = false;
    }break;
    case CACHE_ES_BAT_DATA:
    {
        if( true ==  energyPlanDatas.esBatMap.contains(canAddr) )
            var.setValue(energyPlanDatas.esBatMap[canAddr]);
        else
            ret = false;
    }break;
    case CACHE_PH_CAB_DATA:
    {
        if( true ==  energyPlanDatas.phCabMap.contains(canAddr) )
            var.setValue(energyPlanDatas.phCabMap[canAddr]);
        else
            ret = false;
    }break;
    case CACHE_SC_CAB_DATA:
    {
        if( true ==  energyPlanDatas.scCabMap.contains(canAddr) )
            var.setValue(energyPlanDatas.scCabMap[canAddr]);
        else
            ret = false;
    }break;
    case CACHE_FQ_CAB_DATA:
    {
        if( true ==  energyPlanDatas.fqCabMap.contains(canAddr) )
            var.setValue(energyPlanDatas.fqCabMap[canAddr]);
        else
            ret = false;
    }break;
    case CACHE_CD_CAB_DATA:
    {
        if( true ==  energyPlanDatas.cdCabMap.contains(canAddr) )
            var.setValue(energyPlanDatas.cdCabMap[canAddr]);
        else
            ret = false;
    }break;
    case CACHE_TD_CAB_DATA:
    {
        if( true ==  energyPlanDatas.tdCabMap.contains(canAddr) )
            var.setValue(energyPlanDatas.tdCabMap[canAddr]);
        else
            ret = false;
    }break;
    case CACHE_PO_MOD_DATA:
    {
        if( true ==  energyPlanDatas.poModMap.contains(canAddr) )
            var.setValue(energyPlanDatas.poModMap[canAddr]);
        else
            ret = false;
    }break;
    case CACHE_HY_MOD_DATA:
    {
        if( true ==  energyPlanDatas.hyModMap.contains(canAddr) )
            var.setValue(energyPlanDatas.hyModMap[canAddr]);
        else
            ret = false;
    }break;
    case CACHE_SI_CAB_DATA:
    {
        if( true ==  energyPlanDatas.siCabMap.contains(canAddr) )
            var.setValue(energyPlanDatas.siCabMap[canAddr]);
        else
            ret = false;
    }break;
    case CACHE_ACDC_MOD_DATA:
    {
        if( true ==  energyPlanDatas.acdcModMap.contains(canAddr) )
            var.setValue(energyPlanDatas.acdcModMap[canAddr]);
        else
            ret = false;
    }break;
    case CACHE_DCDC_CD_MOD_DATA:
    {
        if( true ==  energyPlanDatas.dcdcModMap_cd.contains(canAddr) )
            var.setValue(energyPlanDatas.dcdcModMap_cd[canAddr]);
        else
            ret = false;
    }break;
    case CACHE_DCDC_ES_MOD_DATA:
    {
        if( true ==  energyPlanDatas.dcdcModMap_es.contains(canAddr) )
            var.setValue(energyPlanDatas.dcdcModMap_es[canAddr]);
        else
            ret = false;
    }break;
    case CACHE_EMS_CAB_DATA:
    {
        if( true ==  energyPlanDatas.emsMap.contains(canAddr) )
            var.setValue(energyPlanDatas.emsMap[canAddr]);
        else
            ret = false;
    }break;
    }
    mutexEnergyPlan.unlock();
    return ret;
}

/**
 *查询ccu、pdu、分支箱、直流模块个数
 */
int DevCache::QueryDCCabinetNum(int type)
{
	int value = 0;	
	mutexDCCabinet.lock();
	switch(type)
	{
		case CACHE_DCCCUDATA:
            {
				value = DCCabinetDatas.ccuMap.size();
			}break;
		case CACHE_DCPDUDATA:
			{
				value = DCCabinetDatas.pduMap.size();
			}break;
		case CACHE_DCBRANCHDATA:
			{
				value = DCCabinetDatas.branchMap.size();
			}break;
		case CACHE_DCMODULEDATA:
			{
				value = DCCabinetDatas.dcmoduleMap.size();
			}break;
		default: break;
	}
	mutexDCCabinet.unlock();

	return value;
}
////更新CCU 最后获取数据时间
void DevCache::UpdateCCUDataTime(unsigned char ucCanAddr)
{
    QDateTime time = QDateTime::currentDateTime();   //获取当前时间
    unsigned int ui_time = time.toTime_t();   //将当前时间转为时间戳
    if(!CCUOnlineMap.contains(ucCanAddr))
    {
        CCUStatusOnline ccustatus;
        ccustatus.uc_status =1; //在线
        ccustatus.ui_time = ui_time;
        CCUOnlineMap.insert(ucCanAddr,ccustatus);
    }else
    {
        CCUOnlineMap[ucCanAddr].uc_status =1 ;  //在线
        CCUOnlineMap[ucCanAddr].ui_time = ui_time;
    }
}
//查询CCU 状态
bool  DevCache::QueryCCUStatus(unsigned char ucCanAddr,CCUStatusOnline &ccustatus)
{
    if(!CCUOnlineMap.contains(ucCanAddr))
    {
        return false; //从未收到过CCU数据，判定离线。
    }else
    {
        ccustatus = CCUOnlineMap[ucCanAddr];
        return true;
    }
}
