#include "DeviceManage.h"

//SO库调用实现函数, 创建新实例返回
CModuleIO* CreateDevInstance()
{
    return new cDeviceManage();
}

//实例销毁
void DestroyDevInstance(CModuleIO* pModule)
{
    if(pModule)
    {
        delete pModule;
    }
}

cDeviceManage::cDeviceManage()
{
	_strLogName = "event";

    pDevCache = DevCache::GetInstance(); 
    pDBOperate = DBOperate::GetInstance();
    pParamSet = ParamSet::GetInstance();
    pLog =  Log::GetInstance();

    bWorkFlag = FALSE;
    pOneSecTimer = NULL;
    iFaultSerialNum = 1;
}

cDeviceManage::~cDeviceManage()
{
    if(bWorkFlag == TRUE)
    {
        StopModule();
    }
}

//根据配置选项初始化
int cDeviceManage::InitModule( QThread* pThread)
{
    m_pWorkThread = pThread;

    this->moveToThread(m_pWorkThread);
    QObject::connect(m_pWorkThread, SIGNAL(started()), this, SLOT(ProcStartWork()));

    return 0;
}

//注册设备到总线
int cDeviceManage::RegistModule()
{
	QList<int> List;
	List.append(AddrType_FaultState_DCcab); //直流柜故障状态
	List.append(AddrType_ModuleSpecInfo); //模块规格信息

	List.append(AddrType_ActiveProtectApply);   //主动防护功能申请
	List.append(AddrType_ActiveProtectQueryResult); //主动防护功能查询结果
	List.append(AddrType_FlexibleChargeApply);  //柔性充电功能申请
	List.append(AddrType_FlexibleChargeQueryResult);  //柔性充电功能查询结果
	List.append(AddrType_GeneralStaticArgApply);    //通用静态参数设置申请
	List.append(AddrType_GeneralStaticArgQueryResult);   //通用静态参数设置查询结果
	List.append(AddrType_GeneralDynamicArgApply);   //通用动态参数设置申请
	List.append(AddrType_GeneralDynamicArgQueryResult);   //通用动态参数设置结果

	List.append(AddrType_GeneralDynamicArgRenew_DB);   //通用动态参数更新数据库请求
	List.append(AddrType_GeneralStaticArgRenew_DB);   //通用静态参数更新数据库请求

	List.append(AddrType_CCUArgApply);      //CCU参数设置申请, 其他模块发布, 设备管理模块订阅
	List.append(AddrType_CCUQueryResult);   //CCU参数设置查询结果

	CBus::GetInstance()->RegistDev(this, List);

	return 0;
}

int cDeviceManage::StartModule()
{
    m_pWorkThread->start();

    return 0;
}

//停止模块
int cDeviceManage::StopModule()
{
    bWorkFlag = FALSE;
    pOneSecTimer->stop();
    if(pOneSecTimer!=NULL)
    {
        delete pOneSecTimer;
        pOneSecTimer = NULL;
    }
    return 0;
}

//模块工作状态
int cDeviceManage::ModuleStatus()
{
    return 0;
}

//接收控制中心数据
void cDeviceManage::slotFromBus(InfoMap RecvCenterMap, InfoAddrType enType)
{
    switch(enType)
    {
    case AddrType_FaultState_DCcab:
        ParseFaultState(RecvCenterMap);
        break;
    case AddrType_ModuleSpecInfo:
        ParseTermSpecInfo(RecvCenterMap);
        break;
    case AddrType_ActiveProtectApply:
    case AddrType_ActiveProtectQueryResult:
        ParseTermActiveArg(RecvCenterMap, enType);
        break;
    case AddrType_FlexibleChargeApply:
    case AddrType_FlexibleChargeQueryResult:
        ParseTermFlexArg(RecvCenterMap, enType);
        break;
    case AddrType_GeneralStaticArgApply:
        SendCenterData(RecvCenterMap, AddrType_GeneralStaticArgSet);
        break;
    case AddrType_GeneralStaticArgQueryResult:
        ParseTermStaticArg(RecvCenterMap, enType);
        break;
    case AddrType_GeneralDynamicArgApply:
        SendCenterData(RecvCenterMap, AddrType_GeneralDynamicArgSet);
        break;
    case AddrType_GeneralDynamicArgQueryResult:
        ParseTermDynamicArg(RecvCenterMap, enType);
        break;
    case AddrType_GeneralStaticArgRenew_DB:
        ParseRenewGeneralStaticArgDB();
        break;
    case AddrType_GeneralDynamicArgRenew_DB:
        ParseRenewGeneralDynamicArgDB();
        break;
    case AddrType_CCUArgApply:
        SendCenterData(RecvCenterMap, AddrType_CCUArgSet);
        break;
    case AddrType_CCUQueryResult:
        ParseCCUArg(RecvCenterMap);
        break;
    default:
        break;
    }
}

//检查  故障持续时间
void cDeviceManage::CheckFaultDuration()
{
}

//创建 设备故障记录
void cDeviceManage::CreatFaultRecord(const FaultRecord_DCcab &ListRecord)
{
    QString stTodo = "INSERT INTO dc_cabinet_fault_table (canaddr, module_id, fault_code, min_pdu_id, max_pdu_id, start_time, serialnum, record_state) VALUES("
            + QString::number(ListRecord.ucCCUAddr, 10) + ", "
            + QString::number(ListRecord.ucDevID, 10) + ", "
            + QString::number(ListRecord.ucFaultCode, 10) + ", "
            + QString::number(ListRecord.ucMinPDUID, 10) + ", "
            + QString::number(ListRecord.ucMaxPDUID, 10) + ", "
            +  '\'' + ListRecord.StartTime +  "\' ,"
            + QString::number(ListRecord.iSerialNum, 10) + ", "
            + QString::number(ListRecord.ucFaultState, 10) + ") ";
    pDBOperate->DBSqlExec(stTodo.toAscii().data(), DB_PROCESS_RECORD);
    iFaultSerialNum++;
}

//创建 设备规格信息记录
void cDeviceManage::CreatSpecInfoRecord(const SpecificInfo_DCcab &ListRecord)
{
    QString stTodo = "INSERT INTO format_data_table (canaddr, interid, slotnum, serialnum, softversion1, softversion2, softversion3, hdwversion) VALUES("
            + QString::number(ListRecord.ucCCUAddr, 10) + ", "
            + QString::number(ListRecord.ucDevID, 10) + ", "
            + QString::number(ListRecord.ucSlotNum, 10) + ", "
            +  '\'' + ListRecord.SerialNumber + '\'' + ", "
            +  '\'' + ListRecord.SoftwareVer +  '\'' + ", "
            +  '\'' + ListRecord.SoftwareVer1 + '\'' + ", "
            +  '\'' + ListRecord.SoftwareVer2 + '\'' + ", "
            +  '\'' + ListRecord.HardwareVer + '\'' + ", "
            +  '\'' + ListRecord.HardwareVer +  '\'' + ")";
    pDBOperate->DBSqlExec(stTodo.toAscii().data(), DB_PROCESS_RECORD);
}

//加载 告警数据库中内容到故障列表(程序启动运行1次)
void cDeviceManage::LoadFaultDBtoList()
{
    db_result_st dbst;
    FaultRecord_DCcab tempRecord;
    QList <FaultRecord_DCcab> tempFaultList;
    pDBOperate->DBSqlQuery("SELECT canaddr, module_id, fault_code, min_pdu_id, max_pdu_id, start_time, serialnum FROM dc_cabinet_fault_table where record_state = 85", &dbst, DB_PROCESS_RECORD);
    for(int i = 0; i < dbst.row; i++)
    {
        tempRecord.ucCCUAddr = atoi(dbst.result[i * dbst.column]);
        tempRecord.ucDevID = atoi(dbst.result[i * dbst.column +1]);
        tempRecord.ucFaultCode = atoi(dbst.result[i * dbst.column + 2]);
        tempRecord.ucMinPDUID = atoi(dbst.result[i * dbst.column + 3]);
        tempRecord.ucMaxPDUID = atoi(dbst.result[i * dbst.column + 4]);
        tempRecord.StartTime.clear();
        tempRecord.StartTime.append(dbst.result[i * dbst.column + 5]);
        tempRecord.iSerialNum = atoi(dbst.result[i * dbst.column + 6]);
        tempRecord.ucFaultState = 0x00;
        tempFaultList.append(tempRecord);
    }
    pDBOperate->DBQueryFree(&dbst);

    for(int i = 0; i < tempFaultList.count(); i++)
    {
        if(iFaultSerialNum < tempFaultList.at(i).iSerialNum)
        {
            iFaultSerialNum = tempFaultList.at(i).iSerialNum+1;
        }
        UpdateFaultRecord(tempFaultList.at(i));
        SendFaultStateChange(tempFaultList.at(i));
    }
}

//查询 流量统计
void cDeviceManage::QueryTrafficState(QByteArray Devname)
{
    TrafficState_DevMng stTraff;
    pClient->write(Devname);
    if(pClient->isReadable())
    {
        Devname = pClient->readAll();
        if(Devname.isEmpty())
        {
            return;
        }
        memset((char *)&stTraff, 0x00, sizeof(stTraff));
        memcpy((char *)&stTraff, Devname.data(), Devname.length());
    }
}

//查询 GPS信息, 0 失败, 1 成功, st 查询结果返回结构体
int cDeviceManage::QueryGPSInfo(QString FileName, GPS_DevMng &st )
{
    int time1,time2;
    QString data[15]={""};
    QByteArray ba;
    QString strBuffer,tmp;
    int n=0,data_n=0,count=0;
    int hour=0,min=0,sec=0,day=0,mon=0,year=0;

    QFile file(FileName);


    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return 0;
    };

    QTextStream in(&file);
    QString line = in.readLine();
    while((n=line.indexOf(","))!=-1)
    {

        tmp=line.mid(0,n);
        line = line.mid(n+1,(line.length()-n));
        count++;
        data[data_n++]=tmp;
    }

    data[count] = line;

    ba = data[1].toLatin1();
    char *data1 = ba.data();
    if(*data1=='V')
    {
        return 0;
    }
    else
    {
        st.dw = *data1;
    }


    int i = data[2].indexOf(".");
    data[2] = data[2].mid(0,i);
    int data2 = data[2].toInt();
    double data2_du1 = (data2 % 100)/60.0;
    double data2_du2 = data2 / 100;
    double tmp1 = data2_du1+data2_du2;
    ba = data[3].toLatin1();
    char *data3 = ba.data();
    if(*data3 == 'S')
        st.Latitude = tmp1*(-1);
    else
        st.Latitude = tmp1;



    int j = data[4].indexOf(".");
    data[4] = data[4].mid(0,j);
    int data4=data[4].toInt();
    double data4_du1 = (data4%100)/60.0;
    double data4_du2 = data4/100;
    double tmp2 = data4_du1+data4_du2;
    ba = data[5].toLatin1();
    char *data5 = ba.data();
    if(*data5 == 'W')
        st.Longitude = tmp2*(-1);
    else
        st.Longitude = tmp2;

    ba = data[10].toLatin1();
    char *data10 = ba.data();
    if(*data10 == 'W'){
        st.M_vt = data[9].toDouble()*(-1);

    }
    else
        st.M_vt = data[9].toDouble();

    st.e_rate = data[6].toDouble();
    st.e_course = data[7].toDouble();

    ba = data[11].toLatin1();
    char *data11 = ba.data();
    st.M_id = *data11;

    int m = data[0].indexOf(".");
    data[0] = data[0].mid(0,m);

    time1 = data[0].toInt();
    sec = time1%100;
    min = (time1%10000)/100;
    hour = time1/10000;


    time2 = data[8].toInt();
    day = time2/10000;
    mon = (time2%10000)/100;
    year = time2%100;
    year = 2000+year;
    strBuffer = "year-mon-day hour:min:sec";
    strBuffer.sprintf("%d-%02d-%02d %02d:%02d:%02d",year,mon,day,hour,min,sec);

    st.utc = QDateTime::fromString(strBuffer,"yyyy-MM-dd HH:mm:ss");
    return 1;
}

//发送  总线数据
void cDeviceManage::SendCenterData(InfoMap &ToCenterMap, InfoAddrType enType)
{
    emit sigToBus(ToCenterMap, enType);
}

//发送  故障状态, 并更新故障列表
void cDeviceManage::SendFaultStateChange(const FaultRecord_DCcab &stRecord)
{
    QByteArray tempArray;
    InfoMap ToCenterMap;//

    //赋值
    tempArray.append((char *)&stRecord.iSerialNum, 4);
    ToCenterMap.insert(Addr_DCAlarmNo, tempArray);
    tempArray.clear();

    tempArray.append(stRecord.ucCCUAddr);
    ToCenterMap.insert(Addr_CanID_Comm, tempArray);
    tempArray.clear();

    tempArray.append(stRecord.ucDevID);
    ToCenterMap.insert(Addr_DevID_DC_Comm, tempArray);
    tempArray.clear();

    tempArray.append(stRecord.ucFaultCode);
    ToCenterMap.insert(Addr_DCcabFaultCode, tempArray);
    tempArray.clear();

    tempArray.append(stRecord.ucFaultState);
    ToCenterMap.insert(Addr_DCcabFaultState, tempArray);
    tempArray.clear();

    tempArray.append(stRecord.ucMaxPDUID);
    ToCenterMap.insert(Addr_DCcabMaxPDU_ID, tempArray);
    tempArray.clear();

    tempArray.append(stRecord.ucMinPDUID);
    ToCenterMap.insert(Addr_DCcabMinPDU_ID, tempArray);
    tempArray.clear();

    SendCenterData(ToCenterMap, AddrType_FaultStateChange_DCcab);


    //鉴定会临时添加, 储能柜DCDC模块离线 短信
    if( ( stRecord.ucDevID >= ID_MinDCEnergyStorageCabinet) &&( stRecord.ucDevID <= ID_MaxDCEnergyStorageCabinet) )
    {
        if(stRecord.ucDevID == 0x01)
        {
            InfoMap map_temp;
            int iEnergyID_temp = (int)stRecord.ucCCUAddr * 1000 + (int)stRecord.ucDevID;
            int iDevStatus = 8;
            //添加能效ID
            tempArray.append((char *)&iEnergyID_temp, sizeof(iEnergyID_temp));
            map_temp.insert(Addr_EnergyPlan_ID_Comm, tempArray);
            tempArray.clear();
            //添加储能柜DCDC模块工作状态
            tempArray.append((char *)&iDevStatus, sizeof(iDevStatus));
            map_temp.insert(Addr_devStatus3_EnergyStorage, tempArray);
            tempArray.clear();

            SendCenterData(map_temp, AddrType_EnergyPlan_Signal_Burst);
        }

    }

}

//解析  故障状态, 并更新故障列表
void cDeviceManage::ParseFaultState(InfoMap &CenterMap)
{
    bool bFindRecord = FALSE;
    FaultRecord_DCcab stRecord;
    stRecord.ucCCUAddr = CenterMap[Addr_CanID_Comm].at(0);
    stRecord.ucDevID = CenterMap[Addr_DevID_DC_Comm].at(0);
    stRecord.ucFaultCode = CenterMap[Addr_DCcabFaultCode].at(0);
    stRecord.ucFaultState = CenterMap[Addr_DCcabFaultState].at(0);
    stRecord.ucMaxPDUID = CenterMap[Addr_DCcabMaxPDU_ID].at(0);
    stRecord.ucMinPDUID = CenterMap[Addr_DCcabMinPDU_ID].at(0);

    for(unsigned short i = 0; i < FaultRecordList.count(); i++)
    {
        if( (FaultRecordList.at(i).ucCCUAddr == stRecord.ucCCUAddr)
                &&(FaultRecordList.at(i).ucDevID == stRecord.ucDevID)
                &&(FaultRecordList.at(i).ucFaultCode == stRecord.ucFaultCode)
                )
        {
            bFindRecord = TRUE;
            //故障状态相同, 故障计数器清0
            if(FaultRecordList.at(i).ucFaultState == stRecord.ucFaultState)
            {
                FaultRecordList.at(i).ucCounter = 0;
            }
            else
            {
                FaultRecordList.at(i).ucFaultState = stRecord.ucFaultState;
//                SendFaultStateChange(stRecord);
                //故障状态变化, 故障状态由有故障变为无故障
                UpdateFaultRecord(FaultRecordList.at(i));
                SendFaultStateChange(FaultRecordList.at(i));
                FaultRecordList.removeAt(i);
            }
        }
    }
    //新产生故障
    if(bFindRecord == FALSE)
    {
        stRecord.StartTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toAscii();
        stRecord.iSerialNum = iFaultSerialNum;
        FaultRecordList.append(stRecord);
        CreatFaultRecord(stRecord);
        SendFaultStateChange(stRecord);
    }
}

//解析  设备规格信息, 并更新设备规格列表
void cDeviceManage::ParseTermSpecInfo(InfoMap &CenterMap)
{
    InfoMap tempMap;
    SpecificInfo_DCcab tempNode;
    tempNode.ucCCUAddr = CenterMap[Addr_CanID_Comm].at(0);
    tempNode.ucDevID = CenterMap[Addr_DevID_DC_Comm].at(0);
    tempNode.ucSlotNum = CenterMap[Addr_SlotNum_Term].at(0);
    tempNode.SoftwareVer = CenterMap[Addr_SoftwareVer_Term];
    tempNode.SoftwareVer1 = CenterMap[Addr_SoftwareVer1_Term];
    tempNode.SoftwareVer2 = CenterMap[Addr_SoftwareVer2_Term];
    tempNode.HardwareVer = CenterMap[Addr_HardwareVer_Term];
    tempNode.SerialNumber = CenterMap[Addr_SerialNumber_Term];

    if(CenterMap[Addr_SepcEndFlag_Term].at(0) == 0x00)  //首次传输未完成,记录列表
    {
        //nihai add 20170822 ,防止传送的模块信息重复，将重复的模块使用最新的信息, 集空存在掉电检测功能，因此在停电后，还能维持正常工作，如果CCU在未传完时掉电，再上电就会导致传送两次。
        for(int i=0;i<SpecificInfoList.count();i++)
        {
            if(SpecificInfoList.at(i).ucCCUAddr ==tempNode.ucCCUAddr && SpecificInfoList.at(i).ucDevID ==tempNode.ucDevID )
            {
                SpecificInfoList.removeAt(i);
                i--;
            }
        }
        // nihai end
        SpecificInfoList.append(tempNode);
        return;
    }
    else if(CenterMap[Addr_SepcEndFlag_Term].at(0) == 0x01)//首次传输完成
    {
        for(int i=0;i<SpecificInfoList.count();i++)
        {
            if(SpecificInfoList.at(i).ucCCUAddr ==tempNode.ucCCUAddr && SpecificInfoList.at(i).ucDevID ==tempNode.ucDevID )
            {
                SpecificInfoList.removeAt(i);
                i--;
            }
        }
        // nihai end
        SpecificInfoList.append(tempNode);
        //删除当前数据库下目的CCU CAN地址所有的记录
        QString todo = "delete from format_data_table where canaddr = " + QString::number(tempNode.ucCCUAddr, 10);
        pDBOperate->DBSqlExec(todo.toAscii().data(), DB_PROCESS_RECORD);
        for(int i = 0; i < SpecificInfoList.count(); i++)
        {
            if(SpecificInfoList.at(i).ucCCUAddr == tempNode.ucCCUAddr)
            {
                todo = "insert into format_data_table(canaddr, interid, slotnum, serialnum, softversion1, softversion2, softversion3, hdwversion) values('"
                        + QString::number(SpecificInfoList.at(i).ucCCUAddr, 10) +  "', '"
                        + QString::number(SpecificInfoList.at(i).ucDevID, 10) +  "', '"
                        + QString::number(SpecificInfoList.at(i).ucSlotNum, 10) +  "', '"
                        + SpecificInfoList.at(i).SerialNumber +  "', '"
                        + SpecificInfoList.at(i).SoftwareVer +  "', '"
                        + SpecificInfoList.at(i).SoftwareVer1 + "', '"
                        + SpecificInfoList.at(i).SoftwareVer2 +  "', '"
                        + SpecificInfoList.at(i).HardwareVer + "') " ;
                pDBOperate->DBSqlExec(todo.toAscii().data(), DB_PROCESS_RECORD);
                SpecificInfoList.removeAt(i);
                i--;
            }
        }

        SendCenterData(tempMap, AddrType_FormatChange);
    }
    else if(CenterMap[Addr_SepcEndFlag_Term].at(0) == 0x02)//单个信息插入
    {
        for(int i=0;i<SpecificInfoList.count();i++)
        {
            if(SpecificInfoList.at(i).ucCCUAddr ==tempNode.ucCCUAddr && SpecificInfoList.at(i).ucDevID ==tempNode.ucDevID )
            {
                SpecificInfoList.removeAt(i);
                i--;
            }
        }
        //删除当前数据库下 某个CCU的CAN地址下内部ID的记录, 并重新插入
        //        QString todo = "delete from format_data_table where canaddr = "+ QString::number(tempNode.ucCCUAddr, 16) + "and interid = " + + QString::number(tempNode.ucDevID, 16);
        QString todo;
        todo.sprintf("delete from format_data_table where canaddr = %d and interid = %d",tempNode.ucCCUAddr, tempNode.ucDevID);
        pDBOperate->DBSqlExec(todo.toAscii().data(), DB_PROCESS_RECORD);
        todo = "insert into format_data_table(canaddr, interid, slotnum, serialnum, softversion1, softversion2, softversion3, hdwversion) values( '"
                + QString::number(tempNode.ucCCUAddr, 10) + "', '"
                + QString::number(tempNode.ucDevID, 10) +  "', '"
                + QString::number(tempNode.ucSlotNum, 10) +  "', '"
                + tempNode.SerialNumber +  "', '"
                + tempNode.SoftwareVer +  "', '"
                + tempNode.SoftwareVer1 +  "', '"
                + tempNode.SoftwareVer2 +  "', '"
                + tempNode.HardwareVer + "') " ;
        pDBOperate->DBSqlExec(todo.toAscii().data(), DB_PROCESS_RECORD);

        SendCenterData(tempMap, AddrType_FormatChange);
    }
}

//解析  动态参数设置, 并更新动态参数设置表
void cDeviceManage::ParseTermDynamicArg(InfoMap &CenterMap, unsigned int uiType)
{
    unsigned char ucTermWorkMode = 0;
    unsigned char ucTermWorkState = 0;
    unsigned char ucGroupStrategy = 0;
    unsigned char ucPriorityLevel = 0;
    unsigned char ucCanID = 0;
    db_result_st dbst;
    QString todo;
    ucCanID = CenterMap[Addr_CanID_Comm].at(0);
    //数据库操作----查询动态参数
    todo = "SELECT work_type, work_status, strategy, priority_level FROM dcdynamic_param_table WHERE canaddr = "+ QString::number(ucCanID, 10);
    if(0 != pDBOperate->DBSqlQuery(todo.toAscii().data(), &dbst, DB_PARAM))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! ParseTermDynamicArg Query Error");
        return;
    }
    if(dbst.row != 0)
    {
        ucTermWorkMode = (unsigned char) atoi(dbst.result[0]);
        ucTermWorkState = (unsigned char) atoi(dbst.result[1]);
        ucGroupStrategy = (unsigned char) atoi(dbst.result[2]);
        ucPriorityLevel = (unsigned char) atoi(dbst.result[3]);
    }
    else
    {
        todo = "INSERT INTO dcdynamic_param_table (canaddr, work_type, work_status, strategy, priority_level) VALUES( "
                + QString::number(ucCanID, 10) + ", "
                + QString::number(ucTermWorkMode, 10) + ", "
                + QString::number(ucTermWorkState, 10) + ", "
                + QString::number(ucGroupStrategy, 10) + ", "
                + QString::number(ucPriorityLevel, 10) + ") " ;
        pDBOperate->DBSqlExec(todo.toAscii().data(), DB_PARAM);
    }
    pDBOperate->DBQueryFree(&dbst);

    if(CenterMap.contains(Addr_WorkMode_GDA))   //充电终端工作模式
    {
        ucTermWorkMode = CenterMap[Addr_WorkMode_GDA].at(0);
    }
    if(CenterMap.contains(Addr_WorkState_GDA))   //充电终端工作状态
    {
        ucTermWorkState = CenterMap[Addr_WorkState_GDA].at(0);
    }
    if(CenterMap.contains(Addr_GroupStrategy_GDA))   //群充策略
    {
        ucGroupStrategy = CenterMap[Addr_GroupStrategy_GDA].at(0);
    }
    if(CenterMap.contains(Addr_PriorityLevel_GDA))   //充电优先等级
    {
        ucPriorityLevel = CenterMap[Addr_PriorityLevel_GDA].at(0);
    }
    if(CenterMap.contains(Addr_ChargeEndTime_GDA))   //充电完成时间
    {
        ;
    }

    todo = "UPDATE dcdynamic_param_table SET work_type =" + QString::number(ucTermWorkMode, 10) + ", "
            + "work_status = " + QString::number(ucTermWorkState, 10) + ", "
            + "strategy = " + QString::number(ucGroupStrategy, 10) + ", "
            + "priority_level = " + QString::number(ucPriorityLevel, 10) + "  "
            + "where canaddr = " + QString::number(ucCanID, 10);
    pDBOperate->DBSqlExec(todo.toAscii().data(), DB_PARAM);

    if(uiType == AddrType_GeneralDynamicArgApply)
    {
        SendCenterData(CenterMap, AddrType_GeneralDynamicArgSet);
    }
    else if(uiType == AddrType_GeneralDynamicArgQueryResult)
    {
        bool bFind = FALSE;
        for(int i = 0 ; i < DynamaicArgList.count(); i ++)
        {
            if(DynamaicArgList.at(i).ucCanID == ucCanID)
            {
                bFind = TRUE;
                if(DynamaicArgList.at(i).ucArgNum>= 3)  //全部参数已获取
                {
                    SendCenterData(CenterMap, AddrType_GeneralDynamicArgQueryEnd);
                    DynamaicArgList.removeAt(i);
                    break;
                }
                else
                {
                    DynamaicArgList.at(i).ucArgNum++;
                }
            }
        }
        if(bFind == FALSE)
        {
            DynamicArgResult stResult;
            stResult.ucCanID = ucCanID;
            stResult.ucArgNum = 1;
            DynamaicArgList.append(stResult);
        }
    }
}

//解析  终端静态参数设置, 并更新终端静态参数设置表
void cDeviceManage::ParseTermStaticArg(InfoMap CenterMap, unsigned int uiType)
{
    unsigned char ucElecLockType = 0;
    unsigned char ucVINEnable = 0;
    unsigned char ucAuxPowerType = 0;
    unsigned char ucElecLockEnable = 0;
    unsigned char ucBMSProType = 0;
    unsigned char ucTermIDToSet = 0;
    float fMaxGunCur = 0;
    unsigned char ucCanID = 0;
    db_result_st dbst;
    QString todo;
    ucCanID = CenterMap[Addr_CanID_Comm].at(0);

    //数据库操作----查询静态参数
    todo = "SELECT eleclock_type, vin_enable, auxpower_type, eleclock_enable, max_gun_current, bms_pro_type FROM dcstatic_param_table WHERE canaddr = "+ QString::number(ucCanID, 10);
    if(0 != pDBOperate->DBSqlQuery(todo.toAscii().data(), &dbst, DB_PARAM))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! ParseTermStaticArg Query Error");
        return;
    }

    if(dbst.row != 0)
    {
        ucElecLockType = (unsigned char) atoi(dbst.result[0]);
        ucVINEnable = (unsigned char) atoi(dbst.result[1]);
        ucAuxPowerType = (unsigned char) atoi(dbst.result[2]);
        ucElecLockEnable = (unsigned char) atoi(dbst.result[3]);
        fMaxGunCur = atof(dbst.result[4]);
        ucBMSProType = (unsigned char) atoi(dbst.result[5]);
    }
    else
    {
        todo = "INSERT INTO dcstatic_param_table (canaddr, eleclock_type, vin_enable, auxpower_type, eleclock_enable, max_gun_current, bms_pro_type) VALUES( "
                + QString::number(ucCanID, 10) + ", "
                + QString::number(ucElecLockType, 10) + ", "
                + QString::number(ucVINEnable, 10) + ", "
                + QString::number(ucAuxPowerType, 10) + ", "
                + QString("%1").arg(fMaxGunCur) + ", "
                + QString::number(ucElecLockEnable, 10) + ", "
                + QString::number(ucBMSProType, 10) + ") " ;
        pDBOperate->DBSqlExec(todo.toAscii().data(), DB_PARAM);
    }
    pDBOperate->DBQueryFree(&dbst);

    if(CenterMap.contains(Addr_ElecLockType_GSA))   //电子锁类型
    {
        ucElecLockType = CenterMap[Addr_ElecLockType_GSA].at(0);
    }
    if(CenterMap.contains(Addr_VINEnable_GSA))   //VIN使能
    {
        ucVINEnable = CenterMap[Addr_VINEnable_GSA].at(0);
    }
    if(CenterMap.contains(Addr_AuxPowerType_GSA))   //设置辅源类型
    {
        ucAuxPowerType = CenterMap[Addr_AuxPowerType_GSA].at(0);
    }
    if(CenterMap.contains(Addr_ElecLockEnable_GSA))   //电子锁使能
    {
        ucElecLockEnable = CenterMap[Addr_ElecLockEnable_GSA].at(0);
    }
    if(CenterMap.contains(Addr_MaxGunCur_GSA))   //设置枪头最大电流
    {
        fMaxGunCur = *((float *)(CenterMap[Addr_MaxGunCur_GSA].data()));
    }
    if(CenterMap.contains(Addr_BMSProTypeSet_GSA))   //BMS新老国标设置
    {
        ucBMSProType = CenterMap[Addr_BMSProTypeSet_GSA].at(0);
    }

    if(CenterMap.contains(Addr_TermIDSet_GSA))   //终端ID
    {
        ucTermIDToSet = CenterMap[Addr_TermIDSet_GSA].at(0);
    }
    else
    {
        ucTermIDToSet = ucCanID;
    }

    todo = "UPDATE dcstatic_param_table SET eleclock_type =" + QString::number(ucElecLockType, 10) + ", "
            + "vin_enable = " + QString::number(ucVINEnable, 10) + ", "
            + "auxpower_type = " + QString::number(ucAuxPowerType, 10) + ", "
            + "eleclock_enable = " + QString::number(ucElecLockEnable, 10) + ", "
            + "max_gun_current = " + QString("%1").arg(fMaxGunCur) + ", "
            + "bms_pro_type = " + QString::number(ucBMSProType, 10) + "  "
            + "where canaddr = " + QString::number(ucCanID, 10);
    pDBOperate->DBSqlExec(todo.toAscii().data(), DB_PARAM);
    if(uiType == AddrType_GeneralStaticArgApply)
    {
        SendCenterData(CenterMap, AddrType_GeneralStaticArgSet);
    }
}

//解析  终端主动防护参数设置,并更新终端主动防护参数表
void cDeviceManage::ParseTermActiveArg(InfoMap &CenterMap, unsigned int uiType)
{
    //    UpdateActiveRecord(CenterMap);
    if(uiType == AddrType_ActiveProtectApply)
    {
        SendCenterData(CenterMap, AddrType_ActiveProtectSet);
    }
    else    //仅在获取查询结果的时候更新数据库
    {
        UpdateActiveRecord(CenterMap);
    }
}

//解析  终端柔性充电参数设置,并更新终端柔性充电参数表
void cDeviceManage::ParseTermFlexArg(InfoMap &CenterMap, unsigned int uiType)
{
    UpdateFlexRecord(CenterMap);
    if(uiType == AddrType_FlexibleChargeApply)
    {
        SendCenterData(CenterMap, AddrType_FlexibleChargeSet);
    }
}

//解析  CCU参数设置, 并更新数据库CCU参数表
void cDeviceManage::ParseCCUArg(InfoMap &CenterMap)
{
    unsigned char ucCanID = 0;
    unsigned char ucCCUID = 0;
    unsigned char ucTermStartID = 0;
    float fCabMaxPower = 0;

    db_result_st dbst;
    QString todo;
    ucCanID = CenterMap[Addr_CanID_Comm].at(0);

    //数据库操作----查询CCU参数
    todo = "SELECT ccu_set_addr, start_addr, max_power FROM ccu_param_table WHERE canaddr = "+ QString::number(ucCanID, 10);
    if(0 != pDBOperate->DBSqlQuery(todo.toAscii().data(), &dbst, DB_PARAM))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! ParseTermDynamicArg Query Error");
        return;
    }
    if(dbst.row != 0)
    {
        ucCCUID = (unsigned char) atoi(dbst.result[0]);
        ucTermStartID = (unsigned char) atoi(dbst.result[1]);
        fCabMaxPower = atof(dbst.result[2]);
    }
    else
    {
        todo = "INSERT INTO ccu_param_table (canaddr, ccu_set_addr, start_addr, max_power) VALUES( "
                + QString::number(ucCanID, 10) + ", "
                + QString::number(ucCCUID, 10) + ", "
                + QString::number(ucTermStartID, 10) + ", "
                + QString("%1").arg(fCabMaxPower) +  ") " ;

        pDBOperate->DBSqlExec(todo.toAscii().data(), DB_PARAM);
    }
    pDBOperate->DBQueryFree(&dbst);

    if(CenterMap.contains(Addr_CCUidSet_CCUArg))   //设置CCU的ID
    {
        ucCCUID = CenterMap[Addr_CCUidSet_CCUArg].at(0);
    }
    if(CenterMap.contains(Addr_TermStartID_CCUArg))   //设置直流柜终端的起始地址
    {
        ucTermStartID = CenterMap[Addr_TermStartID_CCUArg].at(0);
    }
    if(CenterMap.contains(Addr_CabMaxPower_CCUArg))   //直流柜最大输出功率
    {
        fCabMaxPower = *(float *)CenterMap[Addr_CabMaxPower_CCUArg].data();
    }

    todo = "UPDATE ccu_param_table SET ccu_set_addr =" + QString::number(ucCCUID, 10) + ", "
            + "start_addr = " + QString::number(ucTermStartID, 10) + ", "
            + "max_power = " + QString("%1").arg(fCabMaxPower)  + "  "
            + "where canaddr = " + QString::number(ucCanID, 10);
    pDBOperate->DBSqlExec(todo.toAscii().data(), DB_PARAM);
}

//解析 更新终端动态参数表----根据缓存中设备实时状态刷新
void cDeviceManage::ParseRenewGeneralDynamicArgDB()
{
    InfoMap ToCenterMap;
    RenewGeneralDynamicArgDB();
    SendCenterData(ToCenterMap, AddrType_GeneralDynamicArgRenewAck_DB);
}

//解析 更新静端动态参数表----根据缓存中设备实时状态刷新
void cDeviceManage::ParseRenewGeneralStaticArgDB()
{
    InfoMap ToCenterMap;
    RenewGeneralStaticArgDB();
    SendCenterData(ToCenterMap, AddrType_GeneralStaticArgRenewAck_DB);
}

//更新 终端动态参数设置表----根据缓存中设备实时状态刷新
void cDeviceManage::RenewGeneralDynamicArgDB()
{
    unsigned char ucTermWorkMode = 0;
    unsigned char ucTermWorkState = 0;
    unsigned char ucGroupStrategy = 0;
    unsigned char ucPriorityLevel = 0;
    unsigned char ucCanID = 0;
    stCSCUSysConfig cscuSysConfig;       //CSCU参数设置缓存
    pParamSet->querySetting(&cscuSysConfig, PARAM_CSCU_SYS);
    TerminalStatus stTemp;
    db_result_st dbst;
    QString todo;
    for(unsigned char i = 0; i < cscuSysConfig.directCurrent; i++)
    {
        ucCanID = i + ID_MinDCCanID;
        if(!pDevCache->QueryTerminalStatus(ucCanID, stTemp))
        {
            pLog->getLogPoint(_strLogName)->info("ERROR! SendMainPageData  QueryRealStatusMeter Error!");
        }
        ucTermWorkState = (stTemp.stFrameRemoteSingle.QunLunCeLue & 0x0F);
        ucGroupStrategy = (stTemp.stFrameRemoteSingle.QunLunCeLue >> 4);

        //数据库操作----查询动态参数
        todo = "SELECT work_type, work_status, strategy, priority_level FROM dcdynamic_param_table WHERE canaddr = "+ QString::number(ucCanID, 10);
        if(0 != pDBOperate->DBSqlQuery(todo.toAscii().data(), &dbst, DB_PARAM))
        {
            pLog->getLogPoint(_strLogName)->info("ERROR! RenewGeneralDynamicArgDB Query Error");
            return;
        }
        if(dbst.row == 0)
        {
            todo = "INSERT INTO dcdynamic_param_table (canaddr, work_status, strategy ) VALUES( "
                    + QString::number(ucCanID, 10) + ", "
                    + QString::number(ucTermWorkState, 10) + ", "
                    + QString::number(ucGroupStrategy, 10) +  ") ";
            pDBOperate->DBSqlExec(todo.toAscii().data(), DB_PARAM);
        }
        pDBOperate->DBQueryFree(&dbst);

        todo = "UPDATE dcdynamic_param_table SET work_type =" + QString::number(ucTermWorkMode, 10) + ", "
                + "work_status = " + QString::number(ucTermWorkState, 10) + ", "
                + "strategy = " + QString::number(ucGroupStrategy, 10) + ", "
                + "priority_level = " + QString::number(ucPriorityLevel, 10) + "  "
                + "where canaddr = " + QString::number(ucCanID, 10);
        pDBOperate->DBSqlExec(todo.toAscii().data(), DB_PARAM);
    }
}

//更新 终端静态参数设置表----根据缓存中设备实时状态刷新
void cDeviceManage::RenewGeneralStaticArgDB()
{
    unsigned char ucElecLockType = 0;
    unsigned char ucVINEnable = 0;
    unsigned char ucAuxPowerType = 0;
    unsigned char ucElecLockEnable = 0;
    unsigned char ucBMSProType = 0;
    float fMaxGunCur = 0;
    unsigned char ucCanID = 0;
    stCSCUSysConfig cscuSysConfig;       //CSCU参数设置缓存
    pParamSet->querySetting(&cscuSysConfig, PARAM_CSCU_SYS);
    TerminalStatus stTemp;
    db_result_st dbst;
    QString todo;

    for(unsigned char i = 0; i < cscuSysConfig.directCurrent; i++)
    {
        ucCanID = i + ID_MinDCCanID;
        if(!pDevCache->QueryTerminalStatus(ucCanID, stTemp))
        {
            pLog->getLogPoint(_strLogName)->info("ERROR! SendMainPageData  QueryRealStatusMeter Error!");
        }
        ucAuxPowerType = stTemp.stFrameRemoteSingle.AuxPowerType;

        //数据库操作----查询静态参数
        todo = "SELECT eleclock_type, vin_enable, auxpower_type, eleclock_enable, max_gun_current FROM dcstatic_param_table WHERE canaddr = "+ QString::number(ucCanID, 10);
        if(0 != pDBOperate->DBSqlQuery(todo.toAscii().data(), &dbst, DB_PARAM))
        {
            pLog->getLogPoint(_strLogName)->info("ERROR! ParseTermStaticArg Query Error");
            return;
        }
        if(dbst.row == 0)
        {
            todo = "INSERT INTO dcstatic_param_table (canaddr, auxpower_type) VALUES( "
                    + QString::number(ucCanID, 10) + ", "
                    + QString::number(ucAuxPowerType, 10) +  ") ";
            pDBOperate->DBSqlExec(todo.toAscii().data(), DB_PARAM);
        }
        pDBOperate->DBQueryFree(&dbst);

        todo = "UPDATE dcstatic_param_table SET eleclock_type =" + QString::number(ucElecLockType, 10) + ", "
                + "vin_enable = " + QString::number(ucVINEnable, 10) + ", "
                + "auxpower_type = " + QString::number(ucAuxPowerType, 10) + ", "
                + "eleclock_enable = " + QString::number(ucElecLockEnable, 10) + ", "
                + "max_gun_current = " + QString("%1").arg(fMaxGunCur) + ",  "
                + "bms_pro_type = " + QString::number(ucBMSProType, 10) + "  "
                + "where canaddr = " + QString::number(ucCanID, 10);
        pDBOperate->DBSqlExec(todo.toAscii().data(), DB_PARAM);
    }
}

//更新 设备故障数据库
void cDeviceManage::UpdateFaultRecord(const FaultRecord_DCcab &ListRecord)
{
    QString stTodo = "UPDATE dc_cabinet_fault_table set stop_time = \'"
            + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
            + "\' , record_state = " + QString::number(ListRecord.ucFaultState, 10)
            + " WHERE canaddr = " + QString::number(ListRecord.ucCCUAddr, 10)
            + " and module_id = " + QString::number(ListRecord.ucDevID, 10)
            + " and fault_code = " + QString::number(ListRecord.ucFaultCode, 10)
            + " and start_time = \'" + ListRecord.StartTime + '\'';
    pDBOperate->DBSqlExec(stTodo.toAscii().data(), DB_PROCESS_RECORD);
}

//更新 主动防护库
void cDeviceManage::UpdateActiveRecord(InfoMap &CenterMap)
{
    db_result_st dbst;
    QString todo;
    unsigned char ucCanID = 0;
    FrameActiveProtectionSet stFrame;
    ucCanID = CenterMap[Addr_CanID_Comm].at(0);
    if(!CenterMap.contains(Addr_ArgData))
    {
        return;
    }
    memcpy(&stFrame, CenterMap[Addr_ArgData].data(), sizeof(stFrame));

    //数据库操作----查询主动防护参数
    todo = "SELECT * FROM activeprotection_param_table WHERE canaddr = "+ QString::number(ucCanID, 10);
    if(0 != pDBOperate->DBSqlQuery(todo.toAscii().data(), &dbst, DB_PARAM))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! ParseTermActiveProArg Query Error");
        return;
    }
    if(dbst.row != 0)
    {
        todo = "DELETE FROM activeprotection_param_table WHERE canaddr = " + QString::number(ucCanID, 10);
        pDBOperate->DBSqlExec(todo.toAscii().data(), DB_PARAM);
//        return;
    }
    pDBOperate->DBQueryFree(&dbst);
    todo.sprintf("INSERT INTO activeprotection_param_table\
                 (canaddr, equstage_current, batteryequ_time, heatout_thresold, heatout_time, heatout_disable, \
                  single_overvolatge_thresold, single_overvolatge_suretime, single_overvolatge_disable, \
                  all_overvolatge_thresold, all_overvolatge_suretime, all_overvolatge_disable, \
                  overcurrent_thresold, overcurrent_suretime, overcurrent_disable, \
                  overtemp_thresold, overtemp_suretime, overtemp_disable, \
                  lowtemp_thresold, lowtemp_suretime, lowtemp_disable, \
                  bmsrelay_linkvol_thresold, bmsrelay_link_suretime, bmsrelay_link_disable, \
                  bmsrelay_openvol_thresold, bmsrelay_opencur_thresold, bmsrelay_open_suretime, bmsrelay_open_disable, \
                  overcharge_ratio, overcharge_energy, overcharge_suretime, overcharge_disable, \
                  bmsrepeat_suretime, bmsrepeat_disable, bmscheck_disable, \
                  overvoltage_single_1, overvoltage_single_2, overvoltage_single_3, \
                  overtemp_single_1, overtemp_single_2, overtemp_single_3) \
                 VALUES(%d, %f, %d,   %d, %d, %d,   %f, %d, %d, \
                        %f, %d, %d,   %f, %d, %d,   %d, %d, %d, %d, %d, %d, \
                        %f, %d, %d,   %f, %f, %d, %d,   %f, %f, %d, %d,   %d, %d, %d, \
                        %f, %f, %f, %d, %d, %d)",
                 (int)ucCanID,
                 (float)(stFrame.usBalanceCurrentCoefficient*0.01),
                 (int)(stFrame.ucBalanceTime),

                 (int)(stFrame.ucTempThreshold - 50),
                 (int)(stFrame.ucTempRiseEnsureTime),
                 (int)(stFrame.ucNoTempProtect),

                 (float)(stFrame.usSingleOverVoltageThreshold*0.01),
                 (int)(stFrame.ucSingleOverVoltageEnsureTime),
                 (int)(stFrame.ucNoSingleOverVoltageProtect),
                 //line 1 end
                 (float)(stFrame.usTotalOverVoltageThreshold*0.1),
                 (int)(stFrame.ucOverVoltageEnsureTime),
                 (int)(stFrame.ucNoOverVoltageProtect),

                 (float)(stFrame.usOverCurrentThreshold*0.1 - 2000),
                 (int)(stFrame.ucOverCurrentEnsureTime),
                 (int)(stFrame.ucNoOverCurrentProtect),

                 (int)(stFrame.ucOverTempThreshold - 50),
                 (int)(stFrame.ucOverTempEnsureTime),
                 (int)(stFrame.ucNoOverTempProtect),

                 (int)(stFrame.ucBelowTempThreshold - 50),
                 (int)(stFrame.ucBelowTempEnsureTime),
                 (int)(stFrame.ucNoBelowTempProtect),
                 //line 2 end
                 (float)(stFrame.usBMSRelayAdhesionVoltageThreshold*0.1),
                 (int)(stFrame.ucBMSRelayAdhesionEnsureTime),
                 (int)(stFrame.ucNoBMSRelayAdhesion),

                 (float)(stFrame.usBMSRelayBreakOffVoltageThreshold*0.1),
                 (float)(stFrame.usBMSRelayBreakOffCurrentThreshold*0.1 - 2000),
                 (int)(stFrame.ucBMSRelayBreakOffEnsureTime),
                 (int)(stFrame.ucNoBMSRelayBreakOff),

                 (float)(stFrame.ucOverChargeJudgemetCoefficient*0.1),
                 (float)(stFrame.ucOverChargeEnergyReferanceValue*0.1),
                 (int)(stFrame.ucOverChargeEnergyEnsureTime),
                 (int)(stFrame.ucNoOverCharge),

                 (int)(stFrame.ucBMSDataRepetTime),
                 (int)(stFrame.ucNoBMSDataRepet),
                 (int)(stFrame.ucNoBMSCheck),
                 //line 3 end
                 (float)(stFrame.usOVTh_LNCM*0.01),
                 (float)(stFrame.usOVTh_LTO*0.01),
                 (float)(stFrame.usOVTh_LMO*0.01),
                 (int)(stFrame.ucOTTh_LNCM - 50),
                 (int)(stFrame.ucOTTh_LTO - 50),
                 (int)(stFrame.ucOTTh_LMO - 50)
                 );

    pDBOperate->DBSqlExec(todo.toAscii().data(), DB_PARAM);
}

//更新 柔性充电库
void cDeviceManage::UpdateFlexRecord(InfoMap &CenterMap)
{
    QByteArray dataArray;
    FlexChargeArg stArg;
    QString todo;
    unsigned char ucCanID;
    unsigned char ucType = 0;
    if(CenterMap.contains(Addr_CanID_Comm))
    {
        ucCanID = CenterMap[Addr_CanID_Comm].at(0);
    }
    else
    {
        return;
    }
    if(CenterMap.contains(Addr_ArgNo))
    {
        ucType = CenterMap[Addr_ArgNo].at(0);
    }
    if( (CenterMap.contains(Addr_SOCCoefficient_FC)) || (ucType == 1))
    {
        if(CenterMap.contains(Addr_SOCCoefficient_FC))
            dataArray = CenterMap[Addr_SOCCoefficient_FC];
        else
            dataArray = CenterMap[Addr_ArgData].right(1);
        memcpy(&stArg.stType1, dataArray.data(), dataArray.length());
        todo.sprintf("DELETE from dcflexcharge_param_table where canaddr = %d and arg_type = 1", ucCanID );
        pDBOperate->DBSqlExec(todo.toAscii().data(), DB_PARAM);
        for(int i = 0 ; i < 20; i++)
        {
            if(*((unsigned int *)&(stArg.stType1[i])) == 0xFFFFFFFF )
            {
                continue;
            }
            todo.sprintf("INSERT into dcflexcharge_param_table (canaddr,  arg_type, cur_coef, soc_start, soc_stop) , VALUES(%d, %d, %d, %d, %d)",
                         ucCanID ,1, stArg.stType1[i].usCurCoefficent, stArg.stType1[i].ucStartSOC, stArg.stType1[i].ucStopSOC);
            pDBOperate->DBSqlExec(todo.toAscii().data(), DB_PARAM);
        }
    }
    else if( (CenterMap.contains(Addr_TempCoefficient_FC)) || (ucType == 2))
    {
        if(CenterMap.contains(Addr_TempCoefficient_FC))
            dataArray = CenterMap[Addr_TempCoefficient_FC];
        else
            dataArray = CenterMap[Addr_ArgData].right(1);
        memcpy(&stArg.stType2, dataArray.data(), dataArray.length());
        todo.sprintf("DELETE from dcflexcharge_param_table where canaddr = %d and arg_type = 2", ucCanID );
        pDBOperate->DBSqlExec(todo.toAscii().data(), DB_PARAM);
        for(int i = 0 ; i < 20; i++)
        {
            if(*((unsigned int *)&(stArg.stType2[i])) == 0xFFFFFFFF )
            {
                continue;
            }
            todo.sprintf("INSERT into dcflexcharge_param_table (canaddr, arg_type, cur_coef, temp_start, temp_stop) , VALUES(%d, %d, %d, %d, %d)",
                         ucCanID ,2, stArg.stType2[i].usCurCoefficent, stArg.stType2[i].ucTempStart, stArg.stType2[i].ucTempStop);
            pDBOperate->DBSqlExec(todo.toAscii().data(), DB_PARAM);
        }
    }
    else if( (CenterMap.contains(Addr_TimeCoefficient_FC)) || (ucType == 3))
    {
        if(CenterMap.contains(Addr_TimeCoefficient_FC))
            dataArray = CenterMap[Addr_TimeCoefficient_FC];
        else
            dataArray = CenterMap[Addr_ArgData].right(1);
        memcpy(&stArg.stType3, dataArray.data(), dataArray.length());
        todo.sprintf("DELETE from dcflexcharge_param_table where canaddr = %d and arg_type = 3", ucCanID );
        pDBOperate->DBSqlExec(todo.toAscii().data(), DB_PARAM);
        for(int i = 0 ; i < 20; i++)
        {
            if(*((unsigned int *)&(stArg.stType3[i])) == 0xFFFFFFFF )
            {
                continue;
            }
            todo.sprintf("INSERT into dcflexcharge_param_table (canaddr, arg_type, cur_coef, time) , VALUES(%d, %d, %d, %d)",
                         ucCanID ,3, stArg.stType2[i].usCurCoefficent, stArg.stType3[i].usTimeLength);
            pDBOperate->DBSqlExec(todo.toAscii().data(), DB_PARAM);
        }
    }
    else
    {
        return;
    }
}

//流量统计 建立连接
void cDeviceManage::ProcTrafficStateConnected()
{
    bTrafficConnectFlag = TRUE;
}

//流量统计 断开连接
void cDeviceManage::ProcTrafficStateDisConnected()
{
    bTrafficConnectFlag = FALSE;
}

//开始工作
void cDeviceManage::ProcStartWork()
{
    bWorkFlag = TRUE;
    bTrafficConnectFlag = FALSE;
    pOneSecTimer = new QTimer();
    pClient = new QTcpSocket();
    pClient->connectToHost(QHostAddress::LocalHost, PortNum_TrafficState);
    connect(pClient, SIGNAL(connected()), this, SLOT(ProcTrafficStateConnected()));
    connect(pClient, SIGNAL(disconnected()), this, SLOT(ProcTrafficStateDisConnected()));
    connect(pOneSecTimer, SIGNAL(timeout()), this, SLOT(ProcOneSecTimeOut()));
    pOneSecTimer->start(1000);
    LoadFaultDBtoList();
}

void cDeviceManage::ProcOneSecTimeOut()
{

    if(bTrafficConnectFlag == FALSE)
    {
        pClient->connectToHost(QHostAddress::LocalHost, PortNum_TrafficState);
    }
    else
    {
        ;
    }
}
