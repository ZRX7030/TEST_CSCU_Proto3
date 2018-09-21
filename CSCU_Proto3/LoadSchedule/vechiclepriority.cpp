#include "vechiclepriority.h"
#include <QDebug>

vechiclepriority::vechiclepriority( DevCache*cache,Log *log,DBOperate* pDatabase,ParamSet *param)
{
	_strLogName = "smartcharge";
    m_pSetting = param;
    m_pDevCache =cache;
    m_pLog = log;
    m_pDatabase =  pDatabase;

    m_CcuIndex.clear();
    m_TerminalMap.clear();
    m_VINlist.clear();

#if 0
     unsigned char CCUtotalnum = 0;  //直流柜个数
    //从直流机获取CCU相关信息，需要直流机同时支持此功能
    CCUtotalnum = m_pDevCache->QueryDCCabinetNum(CACHE_DCCCUDATA);

    ccumessgae messagetemp;
    unsigned char canid=ID_MinDCCanID;
    unsigned char gunnum_old=0;
    for(int i =0 ;i < CCUtotalnum ;i++)
    {
        messagetemp.ModuleNum = m_pDevCache->QueryDCCabinetNum(CACHE_DCMODULEDATA);
        messagetemp.GunNum =  m_pDevCache->QueryDCCabinetNum(CACHE_DCPDUDATA);


        messagetemp.StartCandid = canid + gunnum_old;
        m_CcuIndex.insert(i,messagetemp);
        canid = messagetemp.StartCandid;
        gunnum_old = messagetemp.GunNum;
    }
#endif


#if 1
    //从配置文件中读取
    unParamConfig *paramConfig = new unParamConfig;
    m_pSetting->querySetting(paramConfig,PARAM_SMARTCAR);
    stSmartCarConfig config_smartCar = paramConfig->smartCarConfig;
    unsigned char CCUtotalnum = 0;  //直流柜个数

    CCUtotalnum = config_smartCar.CCUtotalnum;

    ccumessgae messagetemp;
    unsigned char canid=ID_MinDCCanID;
    unsigned char gunnum_old=0;
    for(int i =0 ;i < CCUtotalnum ;i++)
    {
        messagetemp.ModuleNum =config_smartCar.ModuleNum[i];
        messagetemp.GunNum =  config_smartCar.gunnum[i];
        messagetemp.StartCandid = canid + gunnum_old;
        m_pLog->getLogPoint(_strLogName)->info("ModuleNum =   "+QString::number(messagetemp.ModuleNum));
        m_pLog->getLogPoint(_strLogName)->info( QString("GunNum = %1 StartCandid = %2").arg(messagetemp.GunNum).arg(messagetemp.StartCandid)  );
        m_CcuIndex.insert(i,messagetemp);
        canid = messagetemp.StartCandid;
        gunnum_old = messagetemp.GunNum;
    }

#endif

}

vechiclepriority::~vechiclepriority()
{
    m_CcuIndex.clear();
    m_TerminalMap.clear();
    m_VINlist.clear();

}

/*读取数据库中的高优先级VIN组
*/
bool vechiclepriority::GetHighPriorityVinList()
{
    struct db_result_st result;

    QString todo = QString("select car_vin, priority from table_car_authentication where is_delete = 0");

    int ret = m_pDatabase->DBSqlQuery(todo.toAscii().data(), &result, DB_AUTHENTICATION);
    if(0 == ret)
    {
        m_pLog->getLogPoint(_strLogName)->info(QString("VIN有效记录条数%1!").arg(result.row));
        if(result.column == 2)
        {
                m_VINlist.clear();    // 清空原来的VIN列表，重新填充
                for(int i=0; i<result.row ;i++)
                {
                    if(((unsigned char)atoi(result.result[i*result.column+1])) ==HighPriority )
                    {
                        QByteArray tem;
                        tem.append((char *)result.result[i*result.column],17);
                        m_VINlist.append(tem);
                    }
                }//end of loop 每一行赋值data
        }//end of 判断几列
        m_pDatabase->DBQueryFree(&result);
    }
    else
    {
        m_pLog->getLogPoint(_strLogName)->info("Query table table_car_authentication false!");
        return false;
    }

        return true;
}

/*根据给胡VIN号比对数据库中的高优先级的VIN组，确认指定VIN的优先级
*/

bool vechiclepriority::CompareVINPriority(char *sVIN)
{
    struct db_result_st result;

    QString todo ;
    todo.sprintf("select car_vin from table_car_authentication where is_delete = 0 and priority=1 and car_vin=\'%s\'",
                sVIN);

    m_pLog->getLogPoint(_strLogName)->info(QString("%1").arg(todo.toAscii().data()));

    int ret = m_pDatabase->DBSqlQuery(todo.toAscii().data(), &result, DB_AUTHENTICATION);
    if((0 == ret) && (result.row > 0))
    {
        m_pLog->getLogPoint(_strLogName)->info(QString("VIN有效记录条数%1!").arg(result.row));
        m_pDatabase->DBQueryFree(&result);
        return true;
    }
    else
    {
        m_pLog->getLogPoint(_strLogName)->info("Query table table_car_authentication false!");
        return false;
    }
    return false;
}


/*根据需求电压电流计算需求模块数
*/
unsigned char vechiclepriority::NeedVolatytoModouleNum(float BMS_need_voltage,float BMS_need_current)
{
    float power_temp=0;
    int integer=0,decimal=0;

   power_temp =  BMS_need_voltage  * BMS_need_current;
   integer = power_temp /ModuleMAXPower;
   decimal = (int)(power_temp+0.5) %ModuleMAXPower;    //取余使用整数，将浮点数作一个四舍五入


   if((decimal>0) || ((decimal==0) && (integer ==0)))
       integer +=1;


   return integer;
}

/*根据模块数计算限制电流
*/
float vechiclepriority::ModouleNumtoNeedVolaty(unsigned char canaddr)
{
    modulemessage terminaltemp;
    TerminalStatus stStatus;
    float current_temp=0;

    memset(&stStatus, 0, sizeof(TerminalStatus));
    memset(&terminaltemp, 0, sizeof(modulemessage));

    if(m_pDevCache)
        m_pDevCache->QueryTerminalStatus(canaddr, stStatus);

  terminaltemp =  m_TerminalMap.value(canaddr);
    current_temp = terminaltemp.DistributionNUM *ModuleMAXPower /stStatus.stFrameBmsInfo.BMS_need_voltage;

   m_pLog->getLogPoint(_strLogName)->info("needcurrunt =   "+QString::number(stStatus.stFrameBmsInfo.BMS_need_current));
   m_pLog->getLogPoint(_strLogName)->info("limitcurrunt =   "+QString::number(current_temp));
   return current_temp;
}

/*计算高优先级和低优先级的个数
 *UsedModuleNum :已经使用的模块个数。非调度开始的充电车辆占用的模块数
*/
void vechiclepriority::CheckPriorityNum(const ccumessgae ccumessagetemp,unsigned char& HighPrioritynum,unsigned char &LowPrioritynum,unsigned char &UsedModuleNum)
{
    int pdunum =0;
    modulemessage modulemessagetemp;
    TerminalStatus status;
    CHARGE_STEP stChargeStep;//存放从缓存中读出的状态机
    unsigned char yetsendnum=0;   //使用模块数

    for(pdunum=0;pdunum< ccumessagetemp.GunNum;pdunum++)
    {
         memset(&status,0,sizeof(TerminalStatus));
        m_pDevCache->QueryTerminalStatus(ccumessagetemp.StartCandid+pdunum,status);

        if(status.cStatus == CHARGE_STATUS_CHARGING)//充电中并且是VIN开始的充电hd 2017-4-5
        {
             memset(&stChargeStep,0,sizeof(CHARGE_STEP));
              m_pDevCache->QueryChargeStep(ccumessagetemp.StartCandid+pdunum, stChargeStep);

              if(stChargeStep.ucStartReason == START_CHARGE_CMD_RSN_SMART_CHARGE_VIN ||       //VIN申请开始充电会是这个值
                      stChargeStep.ucStartReason == START_CHARGE_CMD_RSN_DEV_VIN_REMOTE)           //插枪充电设备上传VIN申请开始充电
              {
                  memset(&modulemessagetemp,0,sizeof(modulemessage));
                  modulemessagetemp.NeedNUM = NeedVolatytoModouleNum(status.stFrameBmsInfo.BMS_need_voltage  , abs(status.stFrameBmsInfo.BMS_need_current));
                   if(CompareVINPriority(stChargeStep.sVIN))
                  {

                       modulemessagetemp.Priority =HighPriority;
                       HighPrioritynum ++;
                  }
                  else
                  {
                       modulemessagetemp.Priority =LowPriority;
                       LowPrioritynum++;
                  }
                  m_TerminalMap.insert(ccumessagetemp.StartCandid+pdunum,modulemessagetemp);
              }
              else
              {
                  yetsendnum = NeedVolatytoModouleNum(status.stFrameRemoteMeSurement1.voltage_of_dc , abs(status.stFrameRemoteMeSurement1.current_of_dc));
                  UsedModuleNum +=yetsendnum ;
              }
        }
    }

}

/*计算每个枪的预分配模块数
 *Priority :优先级
 *Prioritynum :相应优先级的枪个数
 *CanUsedMoudleNum ：可分配的总模块数
 *startcadnid :本CCU下的最小PDU地址
*/
void vechiclepriority::GunModuleNum(unsigned char Priority,unsigned char& Prioritynum,unsigned char &CanUsedMoudleNum,unsigned char startcadnid)
{
    modulemessage modulemessagetemp;
   QMap <unsigned int , modulemessage> ::iterator it;

    for(it = m_TerminalMap.begin(); it != m_TerminalMap.end(); ++it)
    {
        modulemessagetemp = it.value();
        if((modulemessagetemp.Priority == Priority)  && (it.key() >(startcadnid - 1)))
        {
            if( modulemessagetemp.NeedNUM >  modulemessagetemp.DistributionNUM)
            {
                if(CanUsedMoudleNum)
                {
                    CanUsedMoudleNum --;
                    modulemessagetemp.DistributionNUM ++;
                    if(modulemessagetemp.NeedNUM ==  modulemessagetemp.DistributionNUM)
                    {
                        if(Prioritynum)
                             Prioritynum --;
                    }
                     it.value() = modulemessagetemp;
                }
                else
                {
                    break;
                }
            }
        }
         m_pLog->getLogPoint(_strLogName)->info( QString("canid = %1 NeedNUM = %2  DistributionNUM= %3").arg(it.key()).arg(modulemessagetemp.NeedNUM).arg(modulemessagetemp.DistributionNUM)  );
    }
}

void vechiclepriority::ClearDistributionModuleNum(unsigned int canid)
{
    m_pLog->getLogPoint(_strLogName)->info( QString("ClearDistributionModuleNum canid = %1  ").arg(canid));

    modulemessage modulemessage_temp;
    if(m_TerminalMap.contains(canid))
    {
       modulemessage_temp =  m_TerminalMap.value(canid);
       modulemessage_temp.DistributionNUM =0;
        m_TerminalMap.insert(canid,modulemessage_temp);
        m_pLog->getLogPoint(_strLogName)->info( QString("DistributionModuleNum DistributionNUM= %2").arg(m_TerminalMap.value(canid).DistributionNUM) );
    }

}

/*计算每个枪的预分配模块数
*/
void vechiclepriority::DistributionModuleNum(void)
{
    ccumessgae ccumessagetemp;
    QMap <unsigned int , ccumessgae>::iterator it;
    unsigned char HighPrioritynum=0,LowPrioritynum=0;//高优先级个数  低优先级个数
    unsigned char CanUsedMoudleNum=0;     //可以分配的模块数
    unsigned char UsedNum =0;     //非VIN启动的车辆占用的总模块数
    int temp=0;

    memset(&ccumessagetemp,0,sizeof(ccumessgae));

    //GetHighPriorityVinList();
    if(m_CcuIndex.size() < 1)
        return;
    m_TerminalMapOld = m_TerminalMap;
    m_TerminalMap.clear();
    for(it = m_CcuIndex.begin(); it != m_CcuIndex.end(); ++it)
    {
        HighPrioritynum=0;
        LowPrioritynum=0;
        UsedNum =0;
        ccumessagetemp = it.value();//CscuMap.find(ccunum+1);
        CheckPriorityNum(ccumessagetemp,HighPrioritynum,LowPrioritynum,UsedNum);
        temp = ccumessagetemp.ModuleNum - UsedNum -LowPrioritynum; //将低优先级需要分的基本个数（1个）预留，剩余的高优先级的分配
        CanUsedMoudleNum = ((temp > 0) ? temp :0);//ccumessagetemp.ModuleNum - LowPrioritynum;      //防止出现负值
        m_pLog->getLogPoint(_strLogName)->info( QString("ModuleNum = %1 alreadused = %2").arg(ccumessagetemp.ModuleNum).arg(UsedNum)  );
        while(CanUsedMoudleNum && HighPrioritynum)
        {
            GunModuleNum(HighPriority,HighPrioritynum,CanUsedMoudleNum,ccumessagetemp.StartCandid);
        }
        CanUsedMoudleNum +=LowPrioritynum;     //高优先级分配完成后剩余的个数加给低优先级预留的个数，总个数给低优先级的分
        while(CanUsedMoudleNum && LowPrioritynum)
        {
            GunModuleNum(LowPriority,LowPrioritynum,CanUsedMoudleNum,ccumessagetemp.StartCandid);
        }
    }
}

/*计算是否下发限制电流，如果需要将限制电流下发
 *flag :1 需要下发限制电流  0 不需要下发限制电流
 *limitcurrunt ：当需要下发限制电流时，这里存储限制电流值
*/
void vechiclepriority::SendLimitCurrunt(unsigned char canid,unsigned char &flag,float &limitcurrunt)
{

    if(((m_TerminalMapOld.value(canid).DistributionNUM != m_TerminalMap.value(canid).DistributionNUM) ||(m_TerminalMap.contains(canid) && !m_TerminalMapOld.contains(canid))) \
            && (m_TerminalMap.value(canid).DistributionNUM !=0) )   //新旧分配数量不同，或者旧的没有，新的有。新的分配个数非0
    {
        flag =1;
        limitcurrunt = ModouleNumtoNeedVolaty(canid);
    }
    else
    {
        flag = 0;
        limitcurrunt=0;
    }
}
