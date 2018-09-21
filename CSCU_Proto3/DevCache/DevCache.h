#ifndef DEVCACHE_H
#define DEVCACHE_H

#include <QTimer>
#include <QMutex>
#include <QMap>
#include <QVariant>

#include "GeneralData/104_info_struct.h"
#include "GeneralData/GeneralData.h"
#include "Infotag/CSCUBus.h"
#include "ParamSet/ParamSet.h"
#include "Database/DBOperate.h"

//add by XX 2016-03-26
//数据分类
typedef enum _DataType_Cache
{
    TYPE_REAL_DATA_CSCU, //CSCU实时数据
    TYPE_REAL_DATA_CAB, //直流柜实时数据
    TYPE_REAL_DATA_AMMETER, //电表实时数据
    TYPE_REAL_DATA_ENERGY_PLAN, //能效实时数据
    TYPE_STEP_DATA, //过程数据
    TYPE_TERM_DATA  //终端数据

}DataType_Cache;

//过程数据枚举
typedef enum _StepData_Cache
{
    CACHE_CHARGE_STEP   //充电过程数据
}StepData_Cache;

//终端数据枚举
typedef enum _TermData_Cache
{
    CACHE_TERM_DATA //终端数据
}TermData_Cache;
//end add

//能效计划相关结构数据枚举增加 add by XX 2017-07-24
//typedef enum _EnergyPlanData
//{
//    CACHE_TELSINGLE_EP = 0,
//    CACHE_ACDC_EP = 1,
//    CACHE_TEMP_EP = 2
//}_EnergyPlanData;
//end add

/*实时状态、测量数据查询*/
typedef enum _RealData_Cache
{
	CACHE_STATUS,				//状态数据
	CACHE_INLINE_AMMETER,		//进线侧电表
    CACHE_REMOTE_AMMETER,		//远程电表数据
	CACHE_CCUDATA,				//ccu数据, 已废弃

    CACHE_ENERGYPLAN_ENV,//能效系统环境量

    CACHE_ES_CAB_DATA,  //储能柜
    CACHE_ES_BAT_DATA,  //储能电池
    CACHE_PH_CAB_DATA,  //光伏柜
    CACHE_SC_CAB_DATA,  //系统控制柜
    CACHE_FQ_CAB_DATA,  //四象限柜
    CACHE_CD_CAB_DATA,  //充放电柜
    CACHE_TD_CAB_DATA,  //总配电柜
    CACHE_PO_MOD_DATA,  //功率优化器
    CACHE_HY_MOD_DATA,  //温湿度计
    CACHE_SI_CAB_DATA,  //独立逆变柜
    CACHE_ACDC_MOD_DATA,    //ACDC模块
    CACHE_DCDC_CD_MOD_DATA, //充放电柜DCDC模块
    CACHE_DCDC_ES_MOD_DATA, //储能柜DCDC模块
    CACHE_EMS_CAB_DATA, //EMS数据

	
	CACHE_DCCCUDATA,			//直流柜ccu数据
	CACHE_DCPDUDATA,			//直流柜pdu数据
	CACHE_DCBRANCHDATA,			//直流柜分支箱数据
	CACHE_DCMODULEDATA,			//直流柜直流模块数据
	CACHE_TERMDATA,				//终端数据
    CACHE_DEVICESPECIFICATIONDATA,			//设备规格信息
	CACHE_COUNT
}RealData_Cache;


class DevCache : public QObject
{
    Q_OBJECT

public:
    ~DevCache();

	static DevCache *GetInstance();

	//cscu实时状态数据、测量数据操作接口
	bool QueryRealStatusMeter(QVariant &var, int type, QVariant &param);	  //Parma 为电表地址参数，QByteArray 类型的数据	
	RealStatusMeterData  &GetUpdateRealStatusMeter(void);
	void FreeUpdateRealStatusMeter(void);

    //对终端状态缓存的操作接口
    bool QueryTerminalStatus(unsigned char ucCanAddr, TerminalStatus &st_TerminalStatus);
	TerminalStatus &GetUpdateTerminalStatus(unsigned char ucCanAddr);
	void FreeUpdateTerminalStatus();
	bool ResetTerminalStatus(unsigned char ucCanAddr);
    bool SaveTerminalStatus(unsigned char ucCanAddr);
	void loadTerminalStatus(void);

    //对终端状态机缓存操作接口
	bool AddChargeStep(unsigned char ucCanAddr, CHARGE_STEP &st_ChargeStep);
	bool DeleteChargeStep(unsigned char ucCanAddr);
	bool QueryChargeStep(unsigned char ucCanAddr, CHARGE_STEP &st_ChargeStep);
	bool UpateChargeStep(unsigned char ucCanAddr, CHARGE_STEP &st_ChargeStep);    
	bool SaveChargeStep(unsigned char ucCanAddr);
	void loadChargeStep(void);

    //更新CCU 最后接收数据时间
    void UpdateCCUDataTime(unsigned char ucCanAddr); //更新CCU 最后获取报文时间
    bool QueryCCUStatus(unsigned char ucCanAddr ,CCUStatusOnline &CCUStatus);  //获取CCU是否在线


	/*直流柜数据操作接口*/
	stDCCabinetDatas &GetUpdateDCCabinetMeter(void);
	void FreeUpdateDCCabinetMeter(void);
	bool QueryDCCabinetMeter(unsigned char canAddr, QByteArray id, QVariant &var, int type);

	int QueryDCCabinetNum(int type);

    //能效数据操作接口
    stEnergyPlanDatas &GetUpdateEnergyPlanMeter(void);
    void FreeUpdateEnergyPlantMeter(void);
    bool QueryEnergyPlanMeter(unsigned char canAddr, QByteArray id, QVariant &var, int type);

    /*能效系统柜子数据操作接口*/
//    void QueryEnergyStorageCabinetInfo(QByteArray key,QVariant &var);
//    void QueryQuadrantCabinetInfo(QByteArray key,QVariant &var);
//    void QueryMainDistributionCabinetInfo(QByteArray key,QVariant &var);
//    void QuerySysControlCabinetInfo(QByteArray key,QVariant &var);
//    void QueryPhotoVoltaicCabinetInfo(QByteArray key,QVariant &var);
//    void QueryEnergyPlanEnv(QVariant &var, QVariant &param);
//    void QueryDCDCModuleInfo(QByteArray key,QVariant &var);
protected:
	DevCache();

private:
	int chargerAC1Num;
	int chargerAC3Num;
	int chargerDCNum;
    int chargerCCUNum;

	TerminalStatus defaultStatus;		//查找失败返还

	QMutex mutexDevStatus;                   
    QMutex mutexDevChargeStep;  
    QMutex mutexCSCURealData;  
    QMutex mutexDCCabinet;  
    QMutex mutexEnergyPlan;
    stEnergyPlanDatas energyPlanDatas;  //能效数据
	stDCCabinetDatas DCCabinetDatas;		//直流柜数据
	RealStatusMeterData CSCURealData;		//ccu实时数据
	QList<CHARGE_STEP> DevChargeStepList;	//充电数据
    QList<TerminalStatus> DevStatusList;    //终端数据
    QMap<unsigned char,CCUStatusOnline> CCUOnlineMap;  //CCU在线状态  nihai ，增加判断CCU 是否在线，用于记录最后一次接收CCU数据的时间戳

	ParamSet *paramSet;
	DBOperate *dbOperate;
	//QTimer timer;	
	int findPositionViaCanaddr(unsigned char ucCanAddr);
	int FindChargeStepPosition(unsigned char canAddr);
};

#endif // DEVCACHE_H
