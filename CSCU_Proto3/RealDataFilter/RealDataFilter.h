#ifndef			__REAL_DATA_FILTER_H__
#define			__REAL_DATA_FILTER_H__

#include <QTimer>
#include <QObject>

#include "DevCache/DevCache.h"
#include "Infotag/CSCUBus.h"
#include "DBOperate.h"
#include "Bus/Bus.h"
#include "Log.h"
#include "ParamSet/ParamSet.h"

//实时数据回调函数
typedef bool (*realDataFunc)(InfoMap, InfoAddrType);

#define MAX_ABSOLUTE_ERROR_ALLOWED_ACTIVE_ENERGY 20.0 * 100   //6秒内最大有功电能绝对误差允许值: 20kwh; energy=当前功率 * 6s * 分辨率
#define MAX_RELATIVE_ERROR_ALLOWED_ACTIVE_ENERGY 0.50  //最大有功电能相对误差允许值: 50%
//#define MAX_RANGE_ACTIVE_ENERGY 1000000 // 999950   //最大有功电能统计量程
/*突发数据定义*/
enum
{
	BURST_LINK_STATUS=0,
	BURST_CHARGE_STATUS,
	BURST_RELAY_STATUS,
	BURST_FAULT_STATUS,
    //BURST_CARLOCK_STATUS,//add by weiwb
    BURST_COUNT
};

#define BURST_FLAG_CHARGE_INTERFACE	0x001
#define BURST_FLAG_LINK_STATUS		0x002
#define BURST_FLAG_RELAY_STATUS		0x004
#define BURST_FLAG_PARKING_SPACE	0x008
#define BURST_FLAG_CHARGER_STATUS	0x010
#define BURST_FLAG_STATUS_FAULT		0x020
#define BURST_FLAG_BMS_FAULT		0x040
#define BURST_FLAG_STOP_RESULT		0x080
#define BURST_FLAG_QUNLUNCELUE		0x100
#define BURST_FLAG_CTRLMODE			0x200
#define BURST_FLAG_AUXPOWERTYPE		0x400

class Q_DECL_IMPORT RealDataFilter : public CModuleIO
{
	Q_OBJECT

public:
	~RealDataFilter(void);

	static RealDataFilter *GetInstance();
    
	void realDataUpdate(InfoMap TelecontrolMap, InfoAddrType enAddrType);
	bool isAllValid();							//所有设备数据是否有效
	void setRealDataCallBack(realDataFunc func);//设置实时数据回调函数

    //动态库接口moduleIO
    int InitModule(QThread* pThread);
    int RegistModule();
    int StartModule();
    int StopModule();
    int ModuleStatus();
private:
	QTimer timer;
	
	DevCache *devCache;
	CBus *bus;
	Log *log;
	DBOperate *db;
	ParamSet *param;
	realDataFunc realDataCallBack;
	QString _strLogName;

    //能效系统柜子信息
    CabinetAllDatasMap devIDMap_energyPlan;
    stCabinetData cabinetData;
    bool devMapChange;

	int vinFlag;		//1 vin  2 车牌号
    int energyFilterFlag ;    //电度过滤开关 1: 开启, 0:关闭
	int chargerDCNum; 
	int chargerAC1Num;
	int chargerAC3Num;

	FRAME_REMOTE_SINGLE *signalOldStatus;			//遥信旧的状态保存 

	stThreePhaseTypeConfig ThreePhaseTypeConfig;//三相相别全部参数

	int findPhaseType(unsigned char canAddr);
	void CSCURebootRecord(void);
	void onOffLineRecord(unsigned char oldStatus, unsigned char status, unsigned char canaddr);			//记录数据存储
	void gunInsertPullRecord(unsigned char oldLink, unsigned char newLink, unsigned char canaddr);
    void bmsInfoRecord(TerminalStatus Status);
	void faultDeal(unsigned char faultCode, unsigned char canAddr);				

	bool burstStatusCheck(int type, unsigned char currStatus, unsigned char canAddr, InfoMap &Map);		//突发检测
	void terminalOfflineCheck(void);	//离线检测
    bool activeEnergyCheck(TerminalStatus & Status, uint &uiNowEnergy);//有功电能过滤 add by XX 2017-03-21
    void CCUOffLineCheck();  //nihai add 2017-07-30 CCU离线

    void energyPlanDevOnlineCheck_DCDC_ES();    //能效设备在线检验  add by XX 2017-09-30

	/*数据更新接口*/
	void updateTelesignalData(InfoMap Map);
	void updateTelemeterData(InfoMap Map);
	void updateAmmeterData(InfoMap Map);
	void updateBMSData(InfoMap BMSMap);
	void updateAlarmInfo(InfoMap alarmMap);
	void updateTempHumi(InfoMap alarmMap);
    void updateTermSpecInfo(InfoMap CenterMap);

    //changed by XX 2017-08-15
//    void updateEnergyPlanEnvData(InfoMap Map);
    void updateEnergyStorageCabinetInfo(InfoMap centerMap);
    void updateEnergyStorageCabinetBatteryInfo(InfoMap centerMap);
    void updatePhotoVoltaicCabinetInfo(InfoMap centerMap);
    void updateSysControlCabinetInfo(InfoMap centerMap);
    void updateFourQuadrantCabinetInfo(InfoMap centerMap);
    void updateChargeDischargeCabinetInfo(InfoMap centerMap);
    void updateMainDistributionCabinetInfo(InfoMap centerMap);
    void updatePowerOptimizerInfo(InfoMap centerMap);
    void updateHygrothermographInfo(InfoMap centerMap);
    void updateSingleInverterCabinetInfo(InfoMap centerMap);
    void updateACDCInfo(InfoMap centerMap);
    void updateDCDCInfo_CD(InfoMap centerMap);
    void updateDCDCInfo_ES(InfoMap centerMap);
    void updateEMSInfo(InfoMap centerMap);

	void updateCCUSignalMeasure(InfoMap dcMap);
	void updatePDUSignalMeasure(InfoMap dcMap);
//	void updateBranchSignalMeasure(InfoMap dcMap);//(后期无用, 废弃)
	void updateDCModuleSignalMeasure(InfoMap dcMap);

    void updateDCCabFaultState(InfoMap dcMap);  //add by XX 2017-06-15

	int AutoDetect(const char *data, int &iValidLen);

    unsigned char CheckOldEnergy(unsigned int first_energy,unsigned int second_energy,TerminalStatus & Status);   //冻结电量两两比较  hd 2018-8-31

	void AcMeter(TerminalStatus &Status);
	bool IsValidInitEnergy(uchar cIndex);
	void UpdateVirtualMeter(uchar cIndex);
	void InitVirtualMeter();
	double m_fActivePower[ID_MaxDCCanID - ID_MinDCCanID + 1];
	double m_fNowEnergy[ID_MaxDCCanID - ID_MinDCCanID + 1];
	double m_fPreEnergy[ID_MaxDCCanID - ID_MinDCCanID + 1];
	double m_fVMeter[ID_MaxDCCanID - ID_MinDCCanID + 1];
	double m_fMaxMeterRange[ID_MaxDCCanID - ID_MinDCCanID + 1];
	double m_fMeterScale[ID_MaxDCCanID - ID_MinDCCanID + 1];
	int m_iMeterType;

	void loadCouple300KWSetting();
	void couple300KWGunState(int canAddr, bool link);
	QMap<int, QByteArray*> _mapGun;
	int _b300KWEnable;

private slots:
	void timeOut(void);
public slots:
	void slotFromBus(InfoMap Map, InfoAddrType enAddrType);			//接收到升级命令

signals:
	void sigToBus(InfoMap Map, InfoAddrType enAddrType);		//状态突发

protected:
	RealDataFilter();
};


#endif
