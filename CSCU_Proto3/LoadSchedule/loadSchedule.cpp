#include "loadSchedule.h"


LoadSchedule::LoadSchedule()
{
	pParamSet = ParamSet::GetInstance();
	pRealDataFilter = RealDataFilter::GetInstance();
	m_pDevCache = DevCache::GetInstance(); 
    pLog_loadSchedule = Log::GetInstance();
	m_pDatabase = DBOperate::GetInstance();

	newConfig = NULL;
	powerLimit = NULL;
	smartCharge = NULL;
	peakShaving = NULL;

    newConfig = new stAllTPFVConfig;
}

LoadSchedule::~LoadSchedule()
{
    delete newConfig;
    if(powerLimit)
    {
        delete powerLimit;
        powerLimit= NULL;
    }
    if(smartCharge)
    {
        delete smartCharge;
        smartCharge= NULL;
    }
    if(peakShaving)
    {
        delete peakShaving;
        peakShaving= NULL;
    }
}
//获取配置信息
int LoadSchedule::getLoadScheduleConfig(ParamSet * &pPara)
{

    if(pPara)
    {
        unParamConfig *paramConfig = new unParamConfig;

        //负荷约束
        pParamSet->querySetting(paramConfig,PARAM_POWERLIMIT);
        stPowerLimitConfig config_powerLimit = paramConfig->powerLimitConfig;
        powerLimit = new PowerLimit(m_pDevCache,pLog_loadSchedule,pParamSet);
        connect(this,SIGNAL(sig_readAmmeterFail()),powerLimit,SLOT(slot_readAmmeterFail()));
        connect(this,SIGNAL(sig_readAmmeterSucess()),powerLimit,SLOT(slot_readAmmeterSuccess()));
        connect(this,SIGNAL(sig_paraChange_powerLimit(stPowerLimitConfig)),powerLimit,SLOT(slot_paraChange(stPowerLimitConfig)));
        connect(this,SIGNAL(sig_setCCUResult(InfoMap)),powerLimit,SLOT(slot_setCCUResult(InfoMap)));
        connect(powerLimit,SIGNAL(sig_setLimitPower(InfoMap,InfoAddrType)),this,SLOT(slot_setPowerLimit(InfoMap,InfoAddrType)));
        connect(powerLimit,SIGNAL(sig_stopCharge(InfoMap,InfoAddrType)),this,SIGNAL(sigToBus(InfoMap,InfoAddrType)));

        if(config_powerLimit.sPowerLimit_Enable)//初次进入模块，负荷约束功能开启
        {
            emit sig_paraChange_powerLimit(config_powerLimit);
        }

        //错峰充电
        pParamSet->querySetting(paramConfig,PARAM_SMARTCHARGE);
        stSmartChargeConfig configs_smartCharge = paramConfig->smartChargeConfig;
        stAllTPFVConfig *configs = new stAllTPFVConfig;
        pParamSet->queryTPFVConfig(configs);
        smartCharge = new SmartCharge(configs, m_pDevCache,pParamSet,pLog_loadSchedule,m_pDatabase);
        connect(this,SIGNAL(sig_paraChange_smartCharge(stAllTPFVConfig *)),smartCharge,SLOT(slot_paraChange(stAllTPFVConfig *)));
        connect(smartCharge,SIGNAL(sig_stopCharge(InfoMap,InfoAddrType)),this,SIGNAL(sigToBus(InfoMap,InfoAddrType)));
        connect(smartCharge,SIGNAL(sig_adjChargeCur(InfoMap,InfoAddrType)),this,SIGNAL(sigToBus(InfoMap,InfoAddrType)));
        connect(this,SIGNAL(sig_paraChange_smartChargeSwitch(bool)),smartCharge,SLOT(slot_smartChargeSwitch(bool)));

        if(configs_smartCharge.sSmartCharge_Enable)//错峰充电开启
        {
            emit sig_paraChange_smartChargeSwitch(paramConfig->smartChargeConfig.sSmartCharge_Enable);
        }

        //削峰填谷
        pParamSet->querySetting(paramConfig,PARAM_SMARTCHARGE);
        stSmartChargeConfig config_smartCharge = paramConfig->smartChargeConfig;
        stAllTPFVConfig *config = new stAllTPFVConfig;
        pParamSet->queryTPFVConfig(config);
        peakShaving = new PeakShaving(config, m_pDevCache,pParamSet,pLog_loadSchedule);

        connect(this,SIGNAL(sig_paraChange_smartCharge(stAllTPFVConfig *)),peakShaving,SLOT(slot_paraChange(stAllTPFVConfig *)));
        connect(peakShaving,SIGNAL(sig_ChargeApply(InfoMap,InfoAddrType)),this,SIGNAL(sigToBus(InfoMap,InfoAddrType)));
        connect(this,SIGNAL(sig_paraChange_smartChargeSwitch(bool)),peakShaving,SLOT(slot_smartChargeSwitch(bool)));
        connect(this,SIGNAL(sig_StartCharge(unsigned char)),peakShaving,SLOT(slot_StartChargingCMD(unsigned char)));
        connect(this,SIGNAL(sig_chargeFromEnergyPlan(unsigned char,unsigned char)),peakShaving,SLOT(slot_chargeFromEnergyPlan(unsigned char,unsigned char)));

        if(config_smartCharge.sSmartCharge_Enable)//临时借用错峰充电开关
        {
            emit sig_paraChange_smartChargeSwitch(paramConfig->smartChargeConfig.sSmartCharge_Enable);
        }

        delete paramConfig;

    }
    else
    {
    }

return 0;
}

//根据配置选项初始化
int LoadSchedule::InitModule( QThread* pThread)
{
    m_pWorkThread = pThread;
    this->moveToThread(m_pWorkThread);
    QObject::connect(m_pWorkThread, SIGNAL(started()), this, SLOT(ProcStartWork()));

    return 0;
}

//启动模块
void LoadSchedule::ProcStartWork()
{
    getLoadScheduleConfig(pParamSet);
}

//注册模块到总线
int LoadSchedule::RegistModule()
{
	QList<int> list;

	list.append(AddrType_ParamChange);
	list.append(AddrType_Ammeter_Disable);
	list.append(AddrType_Ammeter_Enable);
	list.append(AddrType_CCUArgResult);
	list.append(AddrType_SmartChargeSet);
	//        list.append(AddrType_CmdCtrl_Apply);
	list.append(AddrType_ToPeakApplyCharge);//主题：充电服务收到经济充电指令后，询问削峰填谷是否允许充电
	list.append(AddrType_ToPeakChargeCMD);

	CBus::GetInstance()->RegistDev(this, list);

    return 0;
}
//启动模块
int LoadSchedule::StartModule()
{
    m_pWorkThread->start();
    return 0;
}
int LoadSchedule::StopModule()
{
    return 0;
}
//模块工作状态
int LoadSchedule::ModuleStatus()
{
    return 0;
}
//校验一条设置是否与其他条设置首尾相接
bool LoadSchedule::checkTimeSection(chargeSet item1,QList<chargeSet> itemList)
{
    chargeSet item2;
    bool startFlag,endFlag;
    unsigned int startTime1,endTime1,startTime2,endTime2;

    startTime1 = item1.startHour*100+item1.startMinute;
    endTime1 = item1.endHour*100+item1.endMinute;
    startFlag = endFlag = false;

    //校验开始时间是否时某时段的结束时间
    for(int i=0;i<itemList.length();i++)
    {
        item2=itemList.at(i);
        endTime2 = item2.endHour*100+item2.endMinute;
        if(startTime1 == endTime2)
        {
            startFlag = true;
            break;
        }
    }
    //校验结束时间是否是某时段到开始时间
    for(int j=0;j<itemList.length();j++)
    {
        item2=itemList.at(j);
        startTime2 = item2.startHour*100+item2.startMinute;
        if(endTime1 == startTime2)
        {
            endFlag = true;
            break;
        }
    }

    return startFlag&&endFlag;
}
//
bool LoadSchedule::paraCheck_smartCharge(InfoMap &RecvBusDataMap)
{
    if(RecvBusDataMap.count() == 0)
        return false;

    QList<chargeSet> itemList;
    chargeSet item;
    InfoMap::iterator itTarget;

    //校验输入数据格式
    for(itTarget = RecvBusDataMap.begin(); itTarget != RecvBusDataMap.end(); itTarget++)
    {
        item = *(chargeSet*)(itTarget.value().data());
        itemList.append(item);
        //小时不能大于24;　等于24时分钟必须是０
        //分钟为0~59,时间能串起来等于24小时
        if(item.startHour>24 ||item.endHour>24 ||item.startMinute>59 ||item.startMinute<0||
           item.endMinute>59 ||item.endMinute<0 || (item.endHour==24 &&item.endMinute>0) ||
           (item.startHour==24 &&item.startMinute>0) || (item.startHour==item.endHour && item.startMinute==item.endMinute))//
            return false;
//        if(item.startHour==24)//对于24点系统并没有，因此保存为开始时间时要转化为0
//        {
//            item.startHour = 0;
//            RecvBusDataMap[itTarget.key()] = QByteArray((char*)&item,sizeof(chargeSet));
//        }
    }
    unsigned int startTime,endTime;
    unsigned int minuteSum,minuteCount;
    startTime=endTime=minuteCount=minuteSum=0;
    for(int i=0;i<itemList.length();i++)
    {
        item = itemList.at(i);
        if(itemList.length() == 1)//只有一条记录时,必须是0:00~24:00
        {
            if(item.startHour != 0 || item.startMinute!=0 || item.endHour!=24 || item.endMinute!=0)
            {
                return false;
            }
        }
        else
        {
        if(!checkTimeSection(item,itemList))//检查时间是否首尾相接
            return false;
        }

        startTime = item.startHour*100+item.startMinute;
        endTime =  item.endHour*100+item.endMinute;
        if(startTime>endTime)
        {
            endTime =  endTime + 2400;
            item.endHour = item.endHour + 24;
        }

        if(item.startHour==item.endHour)
        {
            minuteCount = endTime-startTime;
        }
        else//开始小时小于结束小时
        {
            if(item.startMinute>item.endMinute)
            {
                minuteCount = (item.endHour-1-item.startHour)*60+(item.endMinute+60-item.startMinute);
            }
            else
            {
                minuteCount = (item.endHour-item.startHour)*60+(item.endMinute-item.startMinute);
            }
        }

        minuteSum+=minuteCount;
    }
    if(minuteSum != 24*60)//分钟累加为24小时
        return false;

    return true;
}

/*分时段有效性检测
*/
//bool LoadSchedule::paraCheck_smartCharge(InfoMap &RecvBusDataMap)
//{

//    if(RecvBusDataMap.count() == 0)
//        return false;

//    int sectionCount = 0;
//    chargeSet item;
//        InfoMap::iterator itTarget;
//        for(itTarget = RecvBusDataMap.begin(); itTarget != RecvBusDataMap.end(); itTarget++)
//        {
//            item = *(chargeSet*)(itTarget.value().data());

//            if(item.startHour==24 && item.startMinute==0)//起始时间不能设置为２４点
//                break;

//            if(item.startHour==0 && item.startMinute==0 && item.endHour==0 && item.endMinute==0)
//            {//忽略全0的记录
//               continue;
//            }
//            else
//            {
//                if((item.endHour < item.startHour) || ((item.endHour == item.startHour)&&(item.endMinute <= item.startMinute)))
//                    item.endHour += 24;//跨越零点的记录，结束时间在计算时加２４

//                sectionCount += (item.endHour-item.startHour)*2 ;
//                if(item.endMinute > item.startMinute)
//                    sectionCount++;
//                else if(item.endMinute < item.startMinute)
//                    sectionCount--;
//            }
//        }

//        if(sectionCount != 48)//总时区数应为48个半小时
//        {
//            return false;
//        }

//        return true;
//}
///
/// \brief LoadSchedule::parseSmartChargeConfig
/// \param newConfig
///解析错峰充电配置信息到结构体，以备更新数据库
void LoadSchedule::parseSmartChargeConfig(stAllTPFVConfig *newConfig, InfoMap &RecvBusDataMap)
{
    stTPFVConfig item;
    InfoMap::iterator itTarget;

    newConfig->tpfvConfig.clear();
    for(itTarget = RecvBusDataMap.begin(); itTarget != RecvBusDataMap.end(); itTarget++)
    {
        item = *(stTPFVConfig*)(itTarget.value().data());
        newConfig->tpfvConfig.append(item);
    }
}

///
/// \brief LoadSchedule::procParamChange
/// \param RecvBusDataMap
///模块相关参数变化
void LoadSchedule::procParamChange(InfoMap &RecvBusDataMap)
{
    if(RecvBusDataMap.contains(Addr_Param_Change))
    {
        unParamConfig *paramConfig = new unParamConfig;
        switch(*(char*)(RecvBusDataMap[Addr_Param_Change].data()))
        {
        case PARAM_POWERLIMIT:
        {
            pParamSet->querySetting(paramConfig,PARAM_POWERLIMIT);
            emit sig_paraChange_powerLimit(paramConfig->powerLimitConfig);
        }
            break;
        case PARAM_SMARTCHARGE:
        {
            pParamSet->querySetting(paramConfig,PARAM_SMARTCHARGE);
            emit sig_paraChange_smartChargeSwitch(paramConfig->smartChargeConfig.sSmartCharge_Enable);
        }
            break;
        default:
            break;
        }
        delete paramConfig;
    }
}
/*本地设置错峰充电参数，屏幕参数传递
*/
void LoadSchedule::procSmartChargeSet(InfoMap qGetMap)
{
    unsigned char setResult;

    if(paraCheck_smartCharge(qGetMap))//错峰充电设置校验成功
    {
        parseSmartChargeConfig(newConfig,qGetMap);
        if(pParamSet->updateSetting(newConfig,PARAM_TPFV))//加解析信息到newConfig
        {
            setResult = 0xFF;//成功
            emit sig_paraChange_smartCharge(newConfig);
        }
        else
        {
            setResult = 0x0;//失败
        }
    }
    else
    {
        setResult = 0x0;//失败
    }
   InfoMap qInfoMap;
    qInfoMap.insert(Addr_SmartChargeSet_Result, QByteArray(1,setResult));

    emit sigToBus(qInfoMap,AddrType_SmartChargeSet_Result);

}
/*BUS总线订阅信息接收槽函数
*/
void LoadSchedule::slotFromBus(InfoMap qInfoMap, InfoAddrType InfoType)
{
    switch(InfoType)
    {
        case AddrType_ParamChange:
             procParamChange(qInfoMap);
        break;
    case AddrType_Ammeter_Disable:
        emit sig_readAmmeterFail();
        break;
    case AddrType_Ammeter_Enable:
        emit sig_readAmmeterSucess();
        break;
//    case AddrType_TermAdjustmentAck:
    case AddrType_CCUArgResult:
    {
        if(qInfoMap.contains(Addr_CanID_Comm))//CCU输出功率设置响应
        {
            if(qInfoMap[Addr_CanID_Comm].at(0)>= ID_MinCCUCanID &&
                qInfoMap[Addr_CanID_Comm].at(0)<= ID_MaxCCUCanID   )
            emit sig_setCCUResult(qInfoMap);
        }
    }
        break;
    case AddrType_SmartChargeSet:
    {
          procSmartChargeSet( qInfoMap);
    }
        break;
    case AddrType_ToPeakApplyCharge://主题：收到经济充电指令后，询问削峰填谷是否允许充电
         if(qInfoMap[Addr_ChargeCmd_Ctrl].at(0) == CHARGE_CMD_TYPE_START_CHARGE_ECONOMIC)//经济充电
         {
                 emit sig_StartCharge(qInfoMap[Addr_CanID_Comm].at(0));
         }
         break;
    case AddrType_ToPeakChargeCMD://主题：收到经济充电指令后，询问削峰填谷是否允许充电
         if(qInfoMap.contains(Addr_ChargeCmd_Ctrl))//经济充电
         {
               emit sig_chargeFromEnergyPlan(qInfoMap[Addr_CanID_Comm].at(0),qInfoMap[Addr_ChargeCmd_Ctrl].at(0));
         }
         break;

    default:
        break;
    }
}

/*遥调 给充电设备下发遥调参数，限制功率
*/
void LoadSchedule::slot_setPowerLimit(InfoMap infoMap,InfoAddrType infoType)
{
    emit sigToBus(infoMap,infoType);
}
//SO库调用实现函数, 创建新实例返回
CModuleIO* CreateDevInstance()
{
    return new LoadSchedule();
}

//实例销毁
void DestroyDevInstance(CModuleIO* pModule)
{
    if(pModule){
        delete pModule;
    }
}
