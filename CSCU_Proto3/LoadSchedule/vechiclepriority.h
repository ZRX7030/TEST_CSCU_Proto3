#ifndef VECHICLEPRIORITY_H
#define VECHICLEPRIORITY_H

#include <QObject>
#include "ParamSet/ParamSet.h"
#include "DevCache/DevCache.h"
#include "GeneralData/ModuleIO.h"
#include "RealDataFilter.h"
#include "Log.h"
#include "DBOperate.h"

typedef struct _ccumessage
{
    unsigned char ModuleNum;     //每个CCU下的模块数量
    unsigned char GunNum;        //每个CCU下的枪数量
    unsigned char StartCandid;      //此CCU的CAN地址开始值   EndCanid = StartCanid +GunNum -1 ;
}ccumessgae;
typedef QMap <unsigned int , ccumessgae> CscuMap;

typedef struct _modulemessage
{
    unsigned char NeedNUM;     // 需求模块数
    unsigned char DistributionNUM;        //预分配模块数
    unsigned char Priority;      //优先级 ;
}modulemessage;
typedef QMap <unsigned int , modulemessage> TerminalMap;    //每个枪的信息

typedef enum
{
    HighPriority = 1,
    LowPriority = 2
}prioritynum;

#define ModuleMAXPower 15000      //每个模块的额定输出功率
class vechiclepriority : public QObject
{
    Q_OBJECT
private:
    stAllTPFVConfig *config;

    DevCache* m_pDevCache;	//数据缓存
    RealDataFilter* m_pFilter;//数据过滤
    ParamSet* m_pSetting;	//配置
    Log* m_pLog;			//日志
    DBOperate* m_pDatabase;	//数据库
	QString _strLogName;

    //ID_MinCCUCanID = 231,
   // ID_MaxCCUCanID = 240,
    CscuMap m_CcuIndex;	//CCU相关信息存储
   // unsigned char CCUtotalnum;  //直流柜个数
   // unsigned char HighPriorityMun;   //高优先级VIN个数
    QList<QByteArray> m_VINlist;
    TerminalMap m_TerminalMap;
    TerminalMap m_TerminalMapOld;    //存放已分配模块数


    bool GetHighPriorityVinList();
    unsigned char NeedVolatytoModouleNum(float BMS_need_voltage,float BMS_need_current);
    float ModouleNumtoNeedVolaty(unsigned char canaddr);
    bool CompareVINPriority(char *sVIN);
    void CheckPriorityNum(const ccumessgae ccumessagetemp,unsigned char& HighPrioritynum,unsigned char &LowPrioritynum,unsigned char &UsedModuleNum);
    void GunModuleNum(unsigned char Priority,unsigned char &Prioritynum,unsigned char &CanUsedMoudleNum,unsigned char startcadnid);

public:
    explicit vechiclepriority(DevCache* ,Log*,DBOperate*,ParamSet*);

    ~vechiclepriority();

    void SendLimitCurrunt(unsigned char canid,unsigned char &flag,float &limitcurrunt);
    
signals:
    
public slots:
    void DistributionModuleNum(void);
    void ClearDistributionModuleNum(unsigned int canid);
    
};

#endif // VECHICLEPRIORITY_H
