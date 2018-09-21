#include <QFile>
#include "powerLimit.h"

#define POWER_INCREASE_ITEM 2
#define RESET_POWERLIMIT_INTERVAL 300 //5分钟
PowerLimit::PowerLimit(DevCache* cache,Log *log,ParamSet *par)
{
	_strLogName = "powerlimit";
    actionTimer =  new QTimer;
    actionTimer->setInterval(1000);
    connect(actionTimer,SIGNAL(timeout()),this,SLOT(ProcTimeOut()));

    oldPower = 0;
    downFlag = false;
    upFlag = false;
    powerNoChangeMonitor = 0;
    readAmmeterFailCount = 0;

    getParamSetResult = true;

    pPowerLimitLog = log;
    m_pDevCache = cache;
    pParamSet = par;

    memset(limitPowerGroup, 0x0, 40);
    memset(fullPowerGroup, 0x0, 40);
    memset(requirePowerGroup, 0x0, 40);
    memset(getResponse,0x0,40);

}

PowerLimit::~PowerLimit()
{

}

void PowerLimit::slot_readAmmeterSuccess()
{
    readAmmeterFailCount = 0;
    savePower2Log();
}

void PowerLimit::slot_readAmmeterFail()
{
    readAmmeterFailCount++;
    if(readAmmeterFailCount < 2)//连续两次读功率失败，以安全功率充电
    {
        return;
    }
    else
    {
        readAmmeterFailCount = 0;
    }
    pPowerLimitLog->getLogPoint(_strLogName)->info("电表读取功率失败，以子站安全功率充电！！");
    //以安全功率充电
    unsigned int newLimit = config.SAFE_CHARGE_POWER/config.sCCUcount;
    for(unsigned int i=0;i<config.sCCUcount;i++)//平均下发重新分配功率
    {
        //CCU地址从2３１开始计算
//        setLimitPower(config.SAFE_CHARGE_POWER/config.sCCUcount,ID_MinCCUCanID + i);
//        limitPowerGroup[i] = config.SAFE_CHARGE_POWER/config.sCCUcount;

        //CCU地址从240开始计算
        if(abs(limitPowerGroup[9-i] - newLimit) > 5)//和上次限制功率相比，变动大于5则再次下发
        {
            if(newLimit >= fullPowerGroup[9-i])
            {
                limitPowerGroup[9-i] = fullPowerGroup[9-i];
                newLimit = fullPowerGroup[9-i];
            }
            else
            {
                limitPowerGroup[9-i] = newLimit;
            }
            setLimitPower(newLimit,ID_MaxCCUCanID - i);
        }
    }
}

void PowerLimit::init()
{
    pPowerLimitLog->getLogPoint(_strLogName)->info("负荷约束功能开启，以子站安全功率充电！！");
    //上电时以安全功率充电
    for(unsigned int i=0;i<config.sCCUcount;i++)//平均分配功率
    {
        //CCU地址从231开始计算
//        setLimitPower(config.SAFE_CHARGE_POWER/config.sCCUcount,ID_MinCCUCanID + i);
//        limitPowerGroup[i] = config.SAFE_CHARGE_POWER/config.sCCUcount;
        //CCU地址从240开始计算
        setLimitPower(config.SAFE_CHARGE_POWER/config.sCCUcount,ID_MaxCCUCanID - i);
        limitPowerGroup[9-i] = config.SAFE_CHARGE_POWER/config.sCCUcount;

        QString str;
        for(int i=0;i<10;i++)
            str+=" "+QString::number(limitPowerGroup[i]);
        pPowerLimitLog->getLogPoint(_strLogName)->info("各个CCU限制功率："+str);

    }

    if(config.sSUMPower_Ammeter_Enable)//开启电表动态负荷约束
    {
        pPowerLimitLog->getLogPoint(_strLogName)->info("电表动态读取计算限制功率功能开启，读取功率！！");
        pParamSet->querySetting(&conf_allAmmeter, PARAM_AMMETER);
        if(conf_allAmmeter.ammeterConfig.length()>0)
        {
            conf_ammeter = conf_allAmmeter.ammeterConfig.at(0);//暂定取第一个电表
            vKey.setValue(QByteArray((char *)conf_ammeter.addr));
            vValue.setValue(stAmmeter);

            m_pDevCache->QueryRealStatusMeter(vValue, CACHE_INLINE_AMMETER, vKey);
            stAmmeter = vValue.value<stAmmeterData>();

            oldPower = stAmmeter.TotalPower;
        }
    }

}

void PowerLimit::checkSetPowerLimitResult()
{
    //检验设置CCU 功率是否３S设置完成
    waitParamSetResultCount++;
    if(waitParamSetResultCount >= 4)//根据CCU建议为3S，为防误判集控限制４S
    {
        bool getFlag = true;
        waitParamSetResultCount = 0;
        unsigned int i,canid;
        char cmd = CHARGE_CMD_TYPE_STOP_CHARGE;
        for(i=0;i<config.sCCUcount;i++)
        {
            if(getResponse[9-i])//未收到响应，停止该CCU的充电
            {
                pPowerLimitLog->getLogPoint(_strLogName)->info("CCU "+QString::number(ID_MaxCCUCanID-i)+"未收到响应，下发停止充电命令");
                getFlag = false;
                InfoMap qInfoMap;
                canid = ID_MaxCCUCanID-i;

                qInfoMap.insert(Addr_CanID_Comm, QByteArray((char*)&canid,4));
                qInfoMap.insert(Addr_ChargeCmd_Ctrl, QByteArray((char*)&cmd,1));

                 emit sig_stopCharge(qInfoMap,AddrType_CmdCtrl);
            }
        }
        if(getFlag)
        {
            pPowerLimitLog->getLogPoint(_strLogName)->info("收到所有响应！！！！！！！！！！！！！！！！！！");
            getParamSetResult = true;//收到所有设置响应
            waitParamSetResultCount = 0;
        }
    }
}
//获取CCU 功率信息:额定功率／需求功率
void PowerLimit::getPowerInfoFromCCU()
{
    QByteArray id;
//    CCUAllDatasMap ccuDataItem;
    stCCUDatasItem ccuDataItem;;

    for(unsigned int i=0; i<config.sCCUcount; i++)
    {
        //CCU地址从231开始计算
//        const  char canID= i+ID_MinCCUCanID;
//        if(ccuDataMap.ccuData.contains(canID))
//        {
//            tmp = ccuDataMap.ccuData.find(canID).value();
//            requirePowerGroup[i] = tmp.cabnowpower;//cabnowPower理解为CCU需求功率
//            fullPowerGroup[i] = tmp.cabratedpower;//额定功率
//        }
//        else//不能确保每个CCU都能上传功率信息给集控，没收到到按0处理
//        {
//            fullPowerGroup[i] = 0;
//            requirePowerGroup[i] = 0;
//        }

//CCU地址从240开始计算
        unsigned  char canID= ID_MaxCCUCanID - i;
        if(m_pDevCache->QueryDCCabinetMeter(canID,id,vValue, CACHE_DCCCUDATA))
        {
            ccuDataItem = vValue.value<stCCUDatasItem>();
            requirePowerGroup[9-i] = ccuDataItem.cabnowpower;//cabnowPower理解为CCU需求功率
            fullPowerGroup[9-i] = ccuDataItem.cabratedpower;//额定功率

            //        ccuDataItem = vValue.value<CCUAllDatasMap>();
//            tmp = ccuDataItem.find(canID).value();
//            requirePowerGroup[9-i] = tmp.cabnowpower;//cabnowPower理解为CCU需求功率
//            fullPowerGroup[9-i] = tmp.cabratedpower;//额定功率
        }
        else//不能确保每个CCU都能上传功率信息给集控，没收到到按0处理
        {
            fullPowerGroup[9-i] = 0;
            requirePowerGroup[9-i] = 0;
        }
        QString str;
        for(int i=0;i<10;i++)
            str+="  "+QString::number(requirePowerGroup[i]);
//        pPowerLimitLog->getLogPoint(_strLogName)->info("实时CCU需求功率："+str);
        str = "";
        for(int i=0;i<10;i++)
            str+="  "+QString::number(fullPowerGroup[i]);
//        pPowerLimitLog->getLogPoint(_strLogName)->info("实时CCU额定功率："+str);
    }
}

void PowerLimit::savePower2File(float sumPower, float chargePower)
{
    QString  date = QDate::currentDate().toString("yyyy-MM-dd");
//    QString fileName = date + ".txt";
    QString fileName = "/mnt/nandflash/"+date + ".txt";

    QString time = QTime::currentTime().toString("hh:mm:ss") ;
    QFile file (fileName);
    if (!file.open(QFile::Append))
          return;

    int limitPower,reqPower;
    limitPower = reqPower = 0;
    for(int i=0;i<10;i++)
    {
        limitPower+=limitPowerGroup[i];
        reqPower+=requirePowerGroup[i];
    }

    QTextStream out(&file);
    //时间　电表功率　充电功率　限制功率　需求功率
//    out << time <<"  "<<QString("%1").arg(sumPower/10000,10)<<QString("%1").arg(chargePower/100,10)<<QString("%1").arg(limitPowerGroup[9],10)<<endl;
    out << time <<"  "<<QString("%1").arg(sumPower,10)<<QString("%1").arg(chargePower,10);
    out <<QString("%1").arg(limitPower,10)<<QString("%1").arg(reqPower,10)<<endl;
      file.close();
}

bool PowerLimit::queryMonitorPower(float & sumPower)
{
    stAllAmmeterConfig AllAmmeterConfig;  //全部电表参数
    QByteArray AmmeterAddr;

    pParamSet->querySetting(&AllAmmeterConfig,PARAM_AMMETER);
    if(!AllAmmeterConfig.ammeterConfig.isEmpty())
    {
        //默认取第一块电表
        AmmeterAddr.append((char *)AllAmmeterConfig.ammeterConfig.at(0).addr, 6);

        stAmmeterData ammeterData;
        QVariant Var;
        QVariant Param;
        Param.setValue(AmmeterAddr);
        if(!m_pDevCache->QueryRealStatusMeter(Var, CACHE_INLINE_AMMETER, Param))
        {
            pPowerLimitLog->getLogPoint(_strLogName)->info("获取负荷约束电表数据失败！！");
            return false;
        }
        else
        {
           ammeterData = Var.value<stAmmeterData>();
           sumPower = ammeterData.TotalPower;
//           pPowerLimitLog->getLogPoint(_strLogName)->info("获取负荷约束电表功率为："+QString::number(sumPower));
           return true;
        }
    }
    return false;
}

void PowerLimit::savePower2Log()
{
    float chargePower = 0;//当前充电功率
    float sumPower = 0;

    unParamConfig para;
    TerminalStatus status;

    pParamSet->querySetting(&para, PARAM_CSCU_SYS);

    for(unsigned char canID = ID_MinDCCanID;canID<ID_MinDCCanID+para.cscuSysConfig.directCurrent;canID++)
    {
        m_pDevCache->QueryTerminalStatus(canID,status);
        if(status.cStatus == CHARGE_STATUS_CHARGING)
            chargePower += status.stFrameRemoteMeSurement1.active_power;
    }
    if(queryMonitorPower(sumPower))
    {
        savePower2File(sumPower,chargePower);
    }
}

void PowerLimit::saveKeyPoint2File(QString str)
{
    QString  date = QDate::currentDate().toString("yyyy-MM-dd");
    QString fileName = "KeyPoint_"+date + ".txt";

    QString time = QTime::currentTime().toString("hh:mm:ss") ;

    QFile file (fileName);
    if (!file.open(QFile::Append))
          return;

    QTextStream out(&file);
    out << time <<"  "<<str<< endl;

      file.close();
}

void PowerLimit::ProcTimeOut()
{
    int hour = QTime::currentTime().hour();
    int minute = QTime::currentTime().minute();
    int second = QTime::currentTime().second();
    if(hour == 0 && minute ==0 && second ==0)
        system("find /mnt/nandflash -mtime +15 -name \"20*.txt\"| xargs rm -fr");
    //      system("find /mnt/nandflash -mtime +15 -name \"20*.txt\" -exec rm -rf {} \\;");

    getPowerInfoFromCCU();//获取CCU额定功率／需求功率

    if(config.sSUMPower_Ammeter_Enable)//电表动态计算功率
    {//每秒钟读取电表功率，动态计算限制功率
//        m_pDevCache->QueryRealStatusMeter(vValue, CACHE_INLINE_AMMETER, vKey);
//        stAmmeter = vValue.value<stAmmeterData>();
        if(!queryMonitorPower(sumPower) || sumPower >= config.STATION_LIMT_POWER)
        {//不能查到场站功率或查到场站功率超负荷
            pPowerLimitLog->getLogPoint(_strLogName)->info("不能查到场站功率或查到场站功率超负荷，以子站安全功率充电！！");
            for(unsigned int i=0;i<config.sCCUcount;i++)//平均下发重新分配功率
            {  //CCU地址从240开始
                setLimitPower(config.SAFE_CHARGE_POWER/config.sCCUcount,ID_MaxCCUCanID - i);
                limitPowerGroup[9-i] = config.SAFE_CHARGE_POWER/config.sCCUcount;
            }
            return;
        }

        if(sumPower - POWER_INCREASE_ITEM > oldPower)//相邻点对比，功率上升
        {
            upFlag = true;
            powerNoChangeMonitor = 0;
            dynamicSetLimitPower_up(sumPower,oldPower);
        }
        else if(sumPower < oldPower - POWER_INCREASE_ITEM)//功率下降
        {
            downFlag = true;
            powerNoChangeMonitor = 0;
            dynamicSetLimitPower_down(sumPower,oldPower);
        }
        else//功率变化平缓
        {
            upFlag = false;
            downFlag = false;
            powerUpMonitor.clear();
            powerDownMonitor.clear();

            powerNoChangeMonitor++;
            if(powerNoChangeMonitor >= RESET_POWERLIMIT_INTERVAL)
            {
                pPowerLimitLog->getLogPoint(_strLogName)->info("超过５分钟功率变动平缓！！");
                powerNoChangeMonitor = 0;
                getPowerLimit_ammeter(sumPower);
                reAssignPower();
            }
        }
        oldPower = sumPower;
    }
    else
    {
       reAssignPower();
    }

    //下发限制功率应答
    if(!getParamSetResult)
    {
        checkSetPowerLimitResult();
    }
}
///
/// \brief PowerLimit::getPowerLimit_ammeter
/// \param newPower
///获取动态计算功率　sumPower_Ammeter
void PowerLimit::getPowerLimit_ammeter(int newPower)
{
    float chargePower = 0;//当前总充电功率
    TerminalStatus stTerminalStatus;

    for(int id=ID_MinDCCanID; id <= ID_MaxDCCanID; id++)
    {
        if(m_pDevCache->QueryTerminalStatus(id, stTerminalStatus))
        {
            if(stTerminalStatus.stFrameRemoteSingle.charge_status == 1)//正在充电的直流终端
            {
                chargePower += stTerminalStatus.stFrameRemoteMeSurement1.active_power;
            }
        }
    }
    //限制为:  场站限制功率－（当前功率－充电功率）
        sumPower_Ammeter = config.STATION_LIMT_POWER  - (newPower - chargePower);

       unsigned int sumFullPower = 0;//需要重新分配功率的CCU的额定功率之合
        for(unsigned int i=0;i<config.sCCUcount;i++)
        {
            sumFullPower += fullPowerGroup[9-i];
        }

        if(sumPower_Ammeter >= sumFullPower)
        {
            sumPower_Ammeter = sumFullPower;
        }
        pPowerLimitLog->getLogPoint(_strLogName)->info("动态计算限制功率为："+QString::number(sumPower_Ammeter));
}


bool PowerLimit::checkChange(unsigned int requirePower)
{
    unsigned int newLimit = 0;
    for(unsigned int i=0;i<config.sCCUcount;i++)
    {
        newLimit = requirePowerGroup[9-i] * SumLimitPower/requirePower;
        if(abs(limitPowerGroup[9-i] - newLimit) >= 5)//和上次限制功率相比，变动大于5则再次下发
        {
            pPowerLimitLog->getLogPoint(_strLogName)->info("有CCU功率变动大于５，全部重新下发！！");
            return true;
        }
    }
    return false;
}

void PowerLimit::reAssignPower()//按需求功率占比分配给CCU
{
    pPowerLimitLog->getLogPoint(_strLogName)->info("重新分配功率！！！！");

    unsigned int requirePower = 0;
    unsigned int newLimit = 0;

    QString str;
    for(int i=0;i<10;i++)
        str+="  "+QString::number(requirePowerGroup[i]);

    pPowerLimitLog->getLogPoint(_strLogName)->info("各个CCU需求功率："+str);

    //获取当前需求功率    
    for(unsigned int i=0;i<config.sCCUcount;i++)
    {
        requirePower += requirePowerGroup[9-i];
    }

    if(requirePower == 0)//需求功率上传前，不改变功率限制
    {
        return;
    }

    getSumLimitPower();//获取集控限制功率 SumLimitPower

    //按需求比例分配可用功率功率,重新分配功率有一个CCU功率变化大于５时全部重新下发
    if(checkChange(requirePower))
    {
       for(unsigned int i=0;i<config.sCCUcount;i++)
       {
           newLimit = requirePowerGroup[9-i] * SumLimitPower/requirePower;

           if(newLimit >= fullPowerGroup[9-i] * 10)
           {
               limitPowerGroup[9-i] = fullPowerGroup[9-i] * 10;
               newLimit = fullPowerGroup[9-i] * 10;
           }
           else
           {
               limitPowerGroup[9-i] = newLimit;
           }
           setLimitPower(newLimit,ID_MaxCCUCanID - i);
       }
	}
	else
	{
		pPowerLimitLog->getLogPoint(_strLogName)->info("没有CCU 功率变动大于５，无动作！！！！");
	}

}
void PowerLimit::dynamicSetLimitPower_up(float newPower,float tmpPower)
{

    if(downFlag)//之前是下降
    {
        pPowerLimitLog->getLogPoint(_strLogName)->info("之前功率在下降　！！！！！");
        powerDownMonitor.clear();
        downFlag = false;
    }

    pPowerLimitLog->getLogPoint(_strLogName)->info("功率上升！！！！　　当前功率:  "+QString::number((int)newPower));
    powerUpMonitor.append(newPower-tmpPower);//新的记录点

    //连续三个点增长，或有一个点增长大于10
    if((powerUpMonitor.last() > 10) || (powerUpMonitor.length()==3))
        {
            //找到记录点的最大值
            tmpPower = powerUpMonitor.at(0);
            for(int i=0;i<powerUpMonitor.length()-1;i++)
            {
                if(tmpPower<powerUpMonitor.at(i+1))
                    tmpPower = powerUpMonitor.at(i+1);
            }

            QString str =" 功率上升：\n";
            str+="当前记录点：";
            for(int j=0;j<powerUpMonitor.length();j++)
            {
                str+="  "+QString("%1").arg(powerUpMonitor.at(j));
            }
            str+="\n预测值："+QString("%1").arg(newPower + 3*tmpPower);
            pPowerLimitLog->getLogPoint(_strLogName)->info(str);

            if(newPower + 3*tmpPower > config.STATION_LIMT_POWER)//预测3S
            {
                pPowerLimitLog->getLogPoint(_strLogName)->info("预测３S后超过场站限制功率，以安全功率充电！！");
                //预测功率超过场站限制功率时，以安全功率充电
                for(unsigned int i=0;i<config.sCCUcount;i++)//平均下发重新分配功率
                {
                    //CCU地址从231开始
//                    setLimitPower(config.SAFE_CHARGE_POWER/config.sCCUcount,ID_MinCCUCanID + i);
//                    limitPowerGroup[i] = config.SAFE_CHARGE_POWER/config.sCCUcount;
                    //CCU地址从240开始
                    setLimitPower(config.SAFE_CHARGE_POWER/config.sCCUcount,ID_MaxCCUCanID - i);
                    limitPowerGroup[9-i] = config.SAFE_CHARGE_POWER/config.sCCUcount;
                }
                powerUpMonitor.clear();
            }
            else
            {
                pPowerLimitLog->getLogPoint(_strLogName)->info("预测３S后不会超过场站限制功率，以重新动态计算功率充电！！");
                getPowerLimit_ammeter(newPower);
                reAssignPower();//下发重新分配功率
                powerUpMonitor.removeFirst();
            }
        }
}

void PowerLimit::dynamicSetLimitPower_down(float newPower,float tmpPower)
{
    if(upFlag)//之前是上升
    {
        pPowerLimitLog->getLogPoint(_strLogName)->info("之前功率在上升　！！！！！");
        powerUpMonitor.clear();
        upFlag = false;
    }
    pPowerLimitLog->getLogPoint(_strLogName)->info("功率下降！！！！！　当前功率："+QString::number((int)newPower));

    powerDownMonitor.append(tmpPower-newPower);

    //有一个点下降超过10，或连续５个点下降，重新计算下发约束
     if((powerDownMonitor.last() > 10) || (powerDownMonitor.length()==5))
     {
         QString str =" 功率下降：\n";
         str+="当前记录点：";
         for(int j=0;j<powerDownMonitor.length();j++)
         {
             str+="  "+QString("%1").arg(powerDownMonitor.at(j));
         }
         pPowerLimitLog->getLogPoint(_strLogName)->info(str);
         getPowerLimit_ammeter(newPower);
         reAssignPower();//下发重新分配功率
         powerDownMonitor.clear();
     }
}

///
/// \brief PowerLimit::getSumLimitPower
///获取限制功率　SumLimitPower
void PowerLimit::getSumLimitPower()
{
    QList<unsigned int> minPowerList;
    unsigned int sumLimitPower = 0;

//    minPowerList.append(config.SAFE_CHARGE_POWER);

    if(config.sSUMPower_Server_Enable)
        minPowerList.append(config.sSUMPower_Server);
    if(config.sSUMPower_Manual_Enable)
        minPowerList.append(config.sSUMPower_Manual);
    if(config.sSUMPower_Ammeter_Enable)
        minPowerList.append(sumPower_Ammeter);

    //限制功率取三种方式的最小值
    if(minPowerList.length() > 0)
    {
        sumLimitPower = minPowerList.at(0);
    }
    for(int i=0;i<minPowerList.count()-1;i++)
    {
        if(sumLimitPower<minPowerList.at(i+1))
            continue;
        else
            sumLimitPower = minPowerList.at(i+1);
    }
    SumLimitPower = sumLimitPower;

    pPowerLimitLog->getLogPoint(_strLogName)->info("综合计算限制功率为："+QString::number(SumLimitPower));
}
/*给设备下发功率参数
*/
void PowerLimit::setLimitPower(unsigned int TempGridPower,unsigned short cCanAddr)
{
    if(TempGridPower == 0)
    {
        TempGridPower = 2;
        limitPowerGroup[9-(ID_MaxCCUCanID - cCanAddr)] = 2;
    }
    TempGridPower = TempGridPower*10;//协议精度0.1
    pPowerLimitLog->getLogPoint(_strLogName)->info("下发功率，CAN "+QString::number((int)cCanAddr)+"，　值："+QString::number(TempGridPower));
    InfoMap qInfoMap;
    qInfoMap.insert(Addr_CanID_Comm, QByteArray((char*)&cCanAddr,2));
    qInfoMap.insert(Addr_CabMaxPower_Adj, QByteArray((char*)&TempGridPower,4));
    emit sig_setLimitPower(qInfoMap,AddrType_TermAdjustment);
    getParamSetResult = false;
    getResponse[cCanAddr-ID_MinCCUCanID] = 1;//1为未收到响应
    waitParamSetResultCount = 0;
}

//CCU设置功率结果
void PowerLimit::slot_setCCUResult(InfoMap qInfoMap)
{
    int canID,result;
    if(qInfoMap.contains(Addr_CanID_Comm))
    {
        canID = qInfoMap[Addr_CanID_Comm].at(0);
    }
    else
    {
        return;
    }
    if(qInfoMap.contains(Addr_ArgSetResult))
    {
        result = qInfoMap[Addr_ArgSetResult].at(0);
        if(result == 0xff)
        {
        getResponse[canID-ID_MinCCUCanID] = 0;
        }
    }
    else
    {
        return;
    }

}

void PowerLimit::slot_paraChange(stPowerLimitConfig powerLimitConfig)
{
    config = powerLimitConfig;
    if(config.sPowerLimit_Enable && !actionTimer->isActive())
    {
        actionTimer->start();
        init();//功能模块初始化
    }
    else if(!config.sPowerLimit_Enable && actionTimer->isActive())
    {
        actionTimer->stop();
    }
}
