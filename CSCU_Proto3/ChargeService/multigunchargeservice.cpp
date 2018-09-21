#include "multigunchargeservice.h"

MultigunChargeService::MultigunChargeService()
{
	_strLogName = "charge";

    mpDBOperate = DBOperate::GetInstance();
    mgpDevCache = DevCache::GetInstance();
    mgpParamSet = ParamSet::GetInstance();
    mgpLog = Log::GetInstance();
}

MultigunChargeService::~MultigunChargeService()
{}


///
/// \brief ChargeService::LogOut 日志记录
/// \param str 日志串
/// \param Level 级别
///
inline void  MultigunChargeService::LogOut(QString str,int Level)
{
    switch (Level) {
    case 1:
        mgpLog->getLogPoint(_strLogName)->debug(str);
        break;
    case 2:
        mgpLog->getLogPoint(_strLogName)->info(str);
        break;
    case 3:
        mgpLog->getLogPoint(_strLogName)->warn(str);
        break;
    case 4:
        mgpLog->getLogPoint(_strLogName)->error(str);
        break;
    default:
        break;
    }
}

///
/// \brief ChargeService::MultigunProcTerminalChargeMannerInfo
/// \param qInfoMap
/// \param InfoType
///收到直流机上传单双枪信息，更新状态机中充电方式及枪分组信息
void MultigunChargeService::MultigunProcTerminalChargeMannerInfo(InfoMap &qInfoMap, bool m_bServiceOffline ,bool m_bNetOffline )
{
    LogOut(QString("收到总线-单双枪充电信息!"), 2);

    InfoAddrType InfoType;
    stChargeConfig charge;
    unsigned char temp[7];
    unsigned char chargeManner ;
    unsigned char canID_master ;
    memset(temp,0,7);
    if(qInfoMap.contains(Addr_ChargeGunType))
    {
        chargeManner = qInfoMap.value(Addr_ChargeGunType).at(0);
    }else
    {
        //PackageChargetypeReault2ChargeEquipment(canID_master,1);
        return;
    }
    if(qInfoMap.contains(Addr_ChargeGun_Master))
    {
        canID_master = qInfoMap.value(Addr_ChargeGun_Master).at(0);
        temp[0] = canID_master;
    }
    else
    {
        //PackageChargetypeReault2ChargeEquipment(canID_master,1);
        return;
    }
    if(qInfoMap.contains(Addr_ChargeGun_Slave1))
        temp[1] = qInfoMap.value(Addr_ChargeGun_Slave1).at(0);
    if(qInfoMap.contains(Addr_ChargeGun_Slave2))
        temp[2] = qInfoMap.value(Addr_ChargeGun_Slave2).at(0);
    if(qInfoMap.contains(Addr_ChargeGun_Slave3))
        temp[3] = qInfoMap.value(Addr_ChargeGun_Slave3).at(0);
    if(qInfoMap.contains(Addr_ChargeGun_Slave4))
        temp[4] = qInfoMap.value(Addr_ChargeGun_Slave4).at(0);
    if(qInfoMap.contains(Addr_ChargeGun_Slave5))
        temp[5] = qInfoMap.value(Addr_ChargeGun_Slave5).at(0);
    if(qInfoMap.contains(Addr_ChargeGun_Slave6))
        temp[6] = qInfoMap.value(Addr_ChargeGun_Slave6).at(0);


    memset(&charge, 0, sizeof(stChargeConfig));
    mgpParamSet->querySetting(&charge, PARAM_CHARGE);
    if(charge.coupleGun == 0)
    {
         PackageChargetypeReault2ChargeEquipment(canID_master,1);
         return;
    }
     if(CheckChargeMannerInfo(qInfoMap))
     {
         if(chargeManner>1)
            SetTermNameDB(1,temp);//hd ++++++分组信息有更新需要同步更新终端名称列表（和数据库） 1
         else
             SetTermNameDB(0,temp);//hd ++++++分组信息有更新需要同步更新终端名称列表（和数据库） 1
        //校验通过
         PackageChargetypeReault2ChargeEquipment(canID_master,0);
         //更新主机
         UpdateTerminalChargeManner(canID_master,chargeManner,MASTER_GUN);//主枪
         if(chargeManner > SINGLE_CHARGE)//多枪充电
         {
             for(int i=1;i<chargeManner;i++)
             {
                UpdateTerminalChargeManner( temp[i],chargeManner,SLAVE_GUN);//副枪
             }
         }
         if(m_bServiceOffline || m_bNetOffline)    //本地应急充电
             ProcChargeMannerResponseResult_offline(qInfoMap);
         else
         {
             //发送给平台
             InfoType = AddrType_CheckChargeManner_Success;
             emit sigToInBus(qInfoMap, InfoType);
         }
     }
     else
     {
         //更新主机
         UpdateTerminalChargeManner(canID_master,chargeManner,COUPLE_ERR);//配对错误
         if(chargeManner > SINGLE_CHARGE)//多枪充电
         {
             for(int i=1;i<chargeManner;i++)
             {
                UpdateTerminalChargeManner( temp[i],chargeManner,COUPLE_ERR);//配对错误
             }
         }
         //UpdateTerminalChargeManner(canID_slave,chargeManner,COUPLE_ERR);
         PackageChargetypeReault2ChargeEquipment(canID_master,1);
     }
}

/// \brief ChargeService::MultigunProcChargeMannerResponseResult
/// \param qInfoMap
/// \param InfoType
///解析平台响应结果，与上传一致则设置终端可响应充电命令
///检查：是否成功／充电方式／主副枪号
void MultigunChargeService::MultigunProcChargeMannerResponseResult(InfoMap &qInfoMap)
{
    //解析响应结果
    int count_slave;
    unsigned char result = 0;
    unsigned char temp[7];
    unsigned char chargeManner = 0;
    unsigned char canID_master = 0;
    memset(temp,0,7);
    if(qInfoMap.contains(Addr_ChargeGunType_Reault))
        result = qInfoMap.value(Addr_ChargeGunType_Reault).at(0);
    if(qInfoMap.contains(Addr_ChargeGunType))
        chargeManner = qInfoMap.value(Addr_ChargeGunType).at(0);
    if(qInfoMap.contains(Addr_ChargeGun_Master))
    {
        canID_master = qInfoMap.value(Addr_ChargeGun_Master).at(0);
        temp[0] = canID_master;
    }
    if(qInfoMap.contains(Addr_ChargeGun_Slave1))
        temp[1] = qInfoMap.value(Addr_ChargeGun_Slave1).at(0);
    if(qInfoMap.contains(Addr_ChargeGun_Slave2))
        temp[2] = qInfoMap.value(Addr_ChargeGun_Slave2).at(0);
    if(qInfoMap.contains(Addr_ChargeGun_Slave3))
        temp[3] = qInfoMap.value(Addr_ChargeGun_Slave3).at(0);
    if(qInfoMap.contains(Addr_ChargeGun_Slave4))
        temp[4] = qInfoMap.value(Addr_ChargeGun_Slave4).at(0);
    if(qInfoMap.contains(Addr_ChargeGun_Slave5))
        temp[5] = qInfoMap.value(Addr_ChargeGun_Slave5).at(0);
    if(qInfoMap.contains(Addr_ChargeGun_Slave6))
        temp[6] = qInfoMap.value(Addr_ChargeGun_Slave6).at(0);

    stChargeConfig charge;
    LogOut(QString("充电方式   ")+QString::number(chargeManner), 2);
    LogOut(QString("主枪　　　   ")+QString::number(canID_master), 2);
    LogOut(QString("结果　　　   ")+QString::number(result), 2);

    memset(&charge, 0, sizeof(stChargeConfig));
    mgpParamSet->querySetting(&charge, PARAM_CHARGE);
    if(charge.coupleGun == 0)
        return;
    if(result != 0xFF)
    {
        LogOut(QString("平台确认失败！！！！ "), 2);
        return;
    }
    else
    {
        TerminalStatus stTerminalStatus_master ;//= GetstEmptyTerminalStatus();//存放缓存中的数据

        if(mgpDevCache->QueryTerminalStatus(canID_master, stTerminalStatus_master)== false){
            LogOut(QString("获取终端状态失败2!!!!!!!!!!"), 3);
            return;
        }
        if((stTerminalStatus_master.chargeManner == chargeManner) && (stTerminalStatus_master.gunType == MASTER_GUN ))
        {//主枪
            if(chargeManner >1)
            {
                for( count_slave=1;count_slave<chargeManner;count_slave++)
                {
                    TerminalStatus stTerminalStatus_slave ;//= stEmptyTerminalStatus;//存放缓存中的数据
                    if(mgpDevCache->QueryTerminalStatus(temp[count_slave], stTerminalStatus_slave)== false){
                        LogOut(QString("获取终端状态失败3!!!!!!!!!!"), 3);
                        return;
                    }
                     if((stTerminalStatus_slave.chargeManner != chargeManner) || (stTerminalStatus_slave.gunType != SLAVE_GUN))
                     {
                         //break;
                         return;
                     }
                }
                if(count_slave ==chargeManner )
                {
                    ///更新主枪
                    TerminalStatus & master = mgpDevCache->GetUpdateTerminalStatus(canID_master);
                    master.chargeResponseFlag = true;
                    master.gunnum = chargeManner;
                    mgpDevCache->FreeUpdateTerminalStatus();
                    if(mgpDevCache->SaveTerminalStatus(canID_master)  == false)
                    {
                        LogOut(QString("更新终端状态失败!!!!!!!!!!"), 3);
                        return;
                    }
                }
                else
                    return;
            }
            else
            {
                TerminalStatus & master = mgpDevCache->GetUpdateTerminalStatus(canID_master);
                master.chargeResponseFlag = true;
                master.gunnum = chargeManner;
                mgpDevCache->FreeUpdateTerminalStatus();
                if(mgpDevCache->SaveTerminalStatus(canID_master) == false)
                {
                    LogOut(QString("更新终端状态失败!!!!!!!!!!"), 3);
                    return;
                }
            }

            //增加VIN出发使能的判断，避免所有直流设备都直接发起VIN申请开始充电  hd 2017-6-19
            //修改从配置文件中获取自动充电的枪数，如果枪数相同才启动自动充电  hd 2017-7-10
            //修改从数据库中查找出本CANID对应的设置属性，允许自动充电才启动自动充电  hd 2017-7-11
            if((canID_master >ID_MaxACThrCanID) && (canID_master < ID_MinCCUCanID)  && (charge.vinAuto ==1)
                    //&&(chargeManner == charge.coupleGunNum)       //直流机设备才立刻使用VIN去申请充电，交流设备不需要 hd
                //只有充电枪个数跟配置文件中设置的允许自动充电的枪个数相同才允许启动自动充电
                    && (GetAutochargeSet(temp,chargeManner)))     //分组内的所有枪全部设置为允许自动充电
            {
                 //收到确认后，取vin申请充电
                 CHARGE_STEP stChargeStep;//存放从缓存中读出的状态机
                 if(mgpDevCache->QueryChargeStep(canID_master, stChargeStep) == false)      //hd 2017-6-19 判断有真实有效的VIN才申请开始充电
                 {
                      LogOut(QString("QueryChargeStep false1 "),2);
                 }
                 else
                 {
                     InfoMap qInfoMap;
                     qInfoMap.insert(Addr_CanID_Comm, QByteArray(1,canID_master));
                     qInfoMap.insert(Addr_BatteryVIN_BMS,QByteArray(stChargeStep.sVIN, LENGTH_VIN_NO));
                     LogOut(QString("接收到平台响应后集控自动开始申请充电     :%1").arg(QString(qInfoMap[Addr_BatteryVIN_BMS])),2);
                     emit sigInVinApplyStartCharge(qInfoMap);
                 }
            }
        }
    }
}

///
/// \brief ChargeService::MultigunProcChargeGunGroupInfo
/// \param qInfoMap
///收到平台下发的多枪分组信息后，下发给终端
void MultigunChargeService::MultigunProcChargeGunGroupInfo(InfoMap qInfoMap)
{
    LogOut(QString("收到总线-多枪分组信息!"), 2);
    QByteArray qTempByteArray;
    InfoAddrType InfoType;
    unsigned char CANID;

    InfoType = AddrType_ChargeGunGroup_Info_IN;
    CANID = ID_BroadCastCanID;
    qTempByteArray.append(CANID);
    qInfoMap.insert(Addr_CanID_Comm, qTempByteArray);

    emit sigToInBus(qInfoMap, InfoType);
}

//双抢返回结果 result  0成功，1 失败
void MultigunChargeService::PackageChargetypeReault2ChargeEquipment(unsigned char CANID, unsigned char RESULT)
{
    QByteArray qTempByteArray;
    InfoMap qInfoMap;
    InfoAddrType InfoType;

    InfoType = AddrType_Response_Result_IN;
    qTempByteArray.append(CANID);
    qInfoMap.insert(Addr_CanID_Comm, qTempByteArray);

    qTempByteArray.clear();
    qTempByteArray.append(RESULT);
    qInfoMap.insert(Addr_DCChargeMannerSetResult_Term, qTempByteArray);
    LogOut(QString("Send CoupleGun Type can = %1 Reault = %2!").arg((__u8 )qInfoMap[Addr_CanID_Comm].at(0)).arg((__u8 )qInfoMap[Addr_DCChargeMannerSetResult_Term].at(0)), 1);
    emit sigToInBus(qInfoMap, InfoType);
}
//校验单双枪充电信息
bool MultigunChargeService::CheckChargeMannerInfo(InfoMap &qInfoMap)
{
    //存放解析出的数据
    unsigned char CANID;
    unsigned char temp[7];
    unsigned char chargeManner;
    unsigned char canID_master;
    if(qInfoMap.contains(Addr_CanID_Comm))
    {
        CANID = qInfoMap.value(Addr_CanID_Comm).at(0);
    }else
        return false;
    if(qInfoMap.contains(Addr_ChargeGunType))
    {
        chargeManner = qInfoMap.value(Addr_ChargeGunType).at(0);
    }else
        return false;
    if(qInfoMap.contains(Addr_ChargeGun_Master))
    {
        canID_master = qInfoMap.value(Addr_ChargeGun_Master).at(0);
        temp[0] = canID_master;
    }else
        return false;
    if(qInfoMap.contains(Addr_ChargeGun_Slave1))
        temp[1] = qInfoMap.value(Addr_ChargeGun_Slave1).at(0);
    if(qInfoMap.contains(Addr_ChargeGun_Slave2))
        temp[2] = qInfoMap.value(Addr_ChargeGun_Slave2).at(0);
    if(qInfoMap.contains(Addr_ChargeGun_Slave3))
        temp[3] = qInfoMap.value(Addr_ChargeGun_Slave3).at(0);
    if(qInfoMap.contains(Addr_ChargeGun_Slave4))
        temp[4] = qInfoMap.value(Addr_ChargeGun_Slave4).at(0);
    if(qInfoMap.contains(Addr_ChargeGun_Slave5))
        temp[5] = qInfoMap.value(Addr_ChargeGun_Slave5).at(0);
    if(qInfoMap.contains(Addr_ChargeGun_Slave6))
        temp[6] = qInfoMap.value(Addr_ChargeGun_Slave6).at(0);
    LogOut(QString("CheckChargeMannerInfo 模式%1 主枪 %2  副枪1 %3  副枪2 %4  副枪3 %5  副枪4 %6  副枪5 %7  副枪6 %8")\
           .arg(QString::number(chargeManner)).arg(QString::number(canID_master)).arg(QString::number(temp[1]))\
           .arg(QString::number(temp[2])).arg(QString::number(temp[3])).arg(QString::number(temp[4])).arg(QString::number(temp[5]))\
           .arg(QString::number(temp[6])), 2);
    if(chargeManner == SINGLE_CHARGE)//单枪充电
    {//校验主枪与源地址是否相同
        if(canID_master == CANID)
        {
            return true;
        }
    }
    else
    {
        return (CheckChargeGunGroupInfo(chargeManner,temp, CANID));
    }

    return false;
}
/*
 *校验多枪组
*/
bool MultigunChargeService::CheckChargeGunGroupInfo( unsigned char chargeManner, unsigned char* group, unsigned char canID)
{
    struct db_result_st result;
    QString strSql;
    int iCount=0;
    unsigned char temp[7];

    memset(temp,0x00,7);
    memcpy(temp,group,7);

    //CAN地址是否处于多枪分组中，如果存在则获取组编号
    strSql.sprintf("SELECT * FROM chargegun_group_table WHERE gun1=%d OR gun2=%d OR gun3=%d OR gun4=%d OR gun5=%d OR gun6=%d OR gun7=%d;",
            canID, canID, canID, canID, canID, canID, canID);
    LogOut(strSql,1);
    if(mpDBOperate->DBSqlQuery(strSql.toAscii().data(), &result, DB_PARAM) != 0)
        return false;
    LogOut(QString("DBSqlQuery reault=%1").arg(result.row),1);
    if(result.row > 0 &&result.row<2 )    //1个CANID只能包含在一组信息内，如果多组内都包含则认为校验失败hd 2017-6-23
    {
        LogOut(QString("数据库中记录: 枪1 %1  枪2 %2  枪3 %3  枪4 %4  枪5 %5  枪6 %6  枪7 %7")\
               .arg(QString::number(atoi(result.result[2]))).arg(QString::number(atoi(result.result[3])))\
               .arg(QString::number(atoi(result.result[4]))).arg(QString::number(atoi(result.result[5]))).arg(QString::number(atoi(result.result[6])))\
                .arg(QString::number(atoi(result.result[7]))).arg(QString::number(atoi(result.result[8]))), 2);
        //数据库中有
        iCount=0;
        for(int num=0, cmp;num<7;num++)
        {
            if(temp[num])     //直流机上传的CANID非0
            {
                 for( cmp =0;cmp<7;cmp++)
                 {
                     if(temp[num] == atoi(result.result[2+cmp]))
                     {
                         break;
                     }
                 }
                 if(cmp==7)
                 {
                       mpDBOperate->DBQueryFree(&result);
                       LogOut(QString("数据库中校验失败！"), 2);
                     return false;
                 }
                 iCount ++;
            }
        }
          mpDBOperate->DBQueryFree(&result);
        if(iCount == chargeManner)
             return true;      //组内自由组合，组外不许充电
        else
        {
             LogOut(QString("上传的分组数和实际不符！！"), 2);
            return false;
        }
    }
    else
    {
        if(result.row>1)
        {
            LogOut(QString("数据库中同一个CANID包含在多组内  %1 %2 ！！").arg(QString::number(atoi(result.result[1])))\
                    .arg(QString::number(atoi(result.result[1+ result.column]))), 2);
        }
        else
        {
             mpDBOperate->DBQueryFree(&result);
            //数据库中没有
            if(chargeManner == COUPLE_CHARGE)
            {
                if(qAbs(temp[0] - temp[1]) == 1)
                {
                     return true;
                }
                else
                {
                     LogOut(QString("双枪CAN地址连续性校验失败"), 2);
                     return false;
                }
            }
            else
            {
                for (int count =0,y;count < chargeManner;count ++)
                {
                    for( y =0 ;y<chargeManner;y++)
                    {
                        if(qAbs(temp[count] - temp[y]) == 1)
                        {
                            break;
                        }
                    }
                    if(y ==chargeManner )
                    {
                        LogOut(QString("多枪CAN地址连续性校验失败"), 2);
                        return false;
                    }
                }
                return true;
            }
        }
         mpDBOperate->DBQueryFree(&result);
    }
     LogOut(QString("校验失败"), 2);
    return false;
}
#if 0
//多枪分组信息更改后需要通知屏幕显示模块更改显示的分组信息
//flag == 1 添加
//flag ==0 删除
//*group  [0]mastID  [...]slaveID
//void MultigunChargeService::SetTermNameDB(unsigned char flag,unsigned char *group)
//{
//    QString TempName, TempID,GunNamedis;
//    stChargeConfig charge;
//    InfoMap qInfoMap;
//    InfoAddrType InfoType;
//    struct db_result_st result;
//    unsigned char gunnamedis_temp=0;
//    mgpParamSet->querySetting(&charge, PARAM_CHARGE);
//    if(flag && (charge.coupleGun != 0))    //多枪模式
//    {
//        for(int i =0 ;i<7;i++)
//        {
//            if(group[i] )    //can地址非0
//            {
//                if(i==0)   //主枪
//                {
//                    gunnamedis_temp = GetCoupleGunNamedis(group[i]);
//                    GunNamedis = QString::number(gunnamedis_temp,10);
//                }
//                TempID = QString::number(group[i], 10);
//                TempName = "组"+ GunNamedis+ "-"+TempID  ;
//                QString todo = QString("SELECT id FROM terminal_name_show_table  where canaddr = '%1'").arg(TempID);
//                if(mpDBOperate->DBSqlQuery(todo.toAscii().data(), &result, DB_PARAM) == 0)
//                {
//                    if(result.row>0)
//                        todo = QString("UPDATE terminal_name_show_table SET name = '%1' where canaddr = '%2'").arg(TempName).arg(TempID);
//                    else
//                        todo = QString("INSERT INTO terminal_name_show_table ( canaddr, name)  VALUES( " + TempID + " , " + " '" + TempName + "' " + " ) ");
//                    mpDBOperate->DBQueryFree(&result);
//                    mpDBOperate->DBSqlExec(todo.toAscii().data(), DB_PARAM);
//                }
//            }
//        }
//    }else    //单枪模式
//    {
//        TempID = QString::number(group[0], 10);
//        if(group[0] <=ID_MaxACSinCanID)
//        {
//            TempName = TempID + "号交流" ;
//        }else if(group[0] < ID_MinDCCanID)
//        {
//            TempName = TempID + "号三相" ;
//        }else if(group[0] <ID_MinCCUCanID )
//        {
//            TempName = TempID + "号直流" ;
//        }
//        QString todo = QString("SELECT id FROM terminal_name_show_table  where canaddr = '%1'").arg(TempID);
//        if(mpDBOperate->DBSqlQuery(todo.toAscii().data(), &result, DB_PARAM) == 0)
//        {
//            if(result.row>0)
//            {
//                todo = QString("UPDATE  terminal_name_show_table set name ='%1'  where canaddr = '%2'").arg(TempName).arg(TempID);
//            }
//            else
//            {
//                todo = QString("INSERT INTO terminal_name_show_table ( canaddr, name)  VALUES( " + TempID + " , " + " '" + TempName + "' " + " ) ");
//            }
//            mpDBOperate->DBQueryFree(&result);
//            mpDBOperate->DBSqlExec(todo.toAscii().data(), DB_PARAM);
//        }
//    }
//    InfoType = AddrType_Change_ChargeGunGroup_Info;
//    LogOut(QString("AddrType_Change_ChargeGunGroup_Info !"), 3);
//    emit sigToInBus(qInfoMap, InfoType);
//}


//多枪分组信息更改后需要通知屏幕显示模块更改显示的分组信息
//flag == 1 添加
//flag ==0 删除
//*group  [0]mastID  [...]slaveID
//2018-3-9每个图标表示一个终端，重写分组 hd
void MultigunChargeService::SetTermNameDB(unsigned char flag,unsigned char *group)
{
    QString TempName, TempID,GunNamedis;
    stChargeConfig charge;
    InfoMap qInfoMap;
    InfoAddrType InfoType;
    struct db_result_st result;
    unsigned char gunnamedis_temp=0;
    mgpParamSet->querySetting(&charge, PARAM_CHARGE);
    if(flag && (charge.coupleGun != 0))    //多枪模式
    {
        for(int i =0 ;i<7;i++)
        {
            if(group[i] )    //can地址非0
            {
                if(i==0)   //主枪
                {
                    gunnamedis_temp = GetCoupleGunNamedis(group[i]);
                    GunNamedis = QString::number(gunnamedis_temp,10);
                }
                TempID = QString::number(group[i], 10);
<<<<<<< HEAD
                //TempName = "组"+ GunNamedis+ "-"+TempID  ;
                QString todo = QString("SELECT id,multitype,name FROM terminal_name_multi_table  where canaddr = '%1'").arg(TempID);
                LogOut(todo,1);
=======
                if(charge.languageType == 1)
                    TempName = "组"+ GunNamedis+ "-"+TempID  ;
                else if(charge.languageType == 2)
                    TempName = "group"+ GunNamedis+ "-"+TempID  ;
                QString todo = QString("SELECT id FROM terminal_name_show_table  where canaddr = '%1'").arg(TempID);
>>>>>>> dev_English_G0
                if(mpDBOperate->DBSqlQuery(todo.toAscii().data(), &result, DB_PARAM) == 0)
                {
                    if(result.row>0 && atoi(result.result[1]) !=2) //非充电弓    多模块充电枪或单模块充电枪
                    {
                        if(!(QString(result.result[2]).contains("组",Qt::CaseSensitive)))
                        {
                            TempName = "组"+ GunNamedis+ "-"+result.result[2]  ;
                            todo = QString("UPDATE terminal_name_multi_table SET name = '%1' where canaddr = '%2'").arg(TempName).arg(TempID);
                            LogOut(todo,1);
                            mpDBOperate->DBSqlExec(todo.toAscii().data(), DB_PARAM);
                        }
                        else
                            LogOut("terminal_name_multi_table name has contains 组",1);
                    }
                    mpDBOperate->DBQueryFree(&result);
                }
            }
        }
    }else    //单枪模式
    {
        TempID = QString::number(group[0], 10);
<<<<<<< HEAD
        //查询multi_gun_param_tablel如果包含此地址则不需要更新
        QString todo = QString("SELECT id,multitype,name FROM terminal_name_multi_table  where canaddr = '%1'").arg(TempID);
        LogOut(todo,1);
=======
        if(group[0] <=ID_MaxACSinCanID)
        {
            if(charge.languageType == 1)
                TempName = TempID + "号交流" ;
            else if(charge.languageType == 2)
                TempName = TempID + "AC" ;
        }else if(group[0] < ID_MinDCCanID)
        {
            if(charge.languageType == 1)
                TempName = TempID + "号三相" ;
            else if(charge.languageType == 2)
                TempName = TempID + "Three-AC" ;
        }else if(group[0] <ID_MinCCUCanID )
        {
            if(charge.languageType == 1)
                TempName = TempID + "号直流" ;
            else if(charge.languageType == 2)
                TempName = TempID + "DC" ;
        }
        QString todo = QString("SELECT id FROM terminal_name_show_table  where canaddr = '%1'").arg(TempID);
>>>>>>> dev_English_G0
        if(mpDBOperate->DBSqlQuery(todo.toAscii().data(), &result, DB_PARAM) == 0)
        {
            if(result.row>0 && (atoi(result.result[1])) !=2)  //不是充电弓
            {
                if(QString(result.result[2]).contains("组",Qt::CaseSensitive))
                {
                    //TempName = QString(result.result[2]).mid(3) ;
                    int ret =QString(result.result[2]).indexOf("-");
                    TempName = QString(result.result[2]).mid(ret+1);
                }else
                    TempName = QString(result.result[2]);
                todo = QString("UPDATE  terminal_name_multi_table set name ='%1'  where canaddr = '%2'").arg(TempName).arg(TempID);
                LogOut(todo,1);
                 mpDBOperate->DBSqlExec(todo.toAscii().data(), DB_PARAM);
            }
            mpDBOperate->DBQueryFree(&result);
        }
    }
    InfoType = AddrType_Change_ChargeGunGroup_Info;
    LogOut(QString("AddrType_Change_ChargeGunGroup_Info !"), 3);
    emit sigToInBus(qInfoMap, InfoType);
}
#endif
//多枪分组信息更改后需要通知屏幕显示模块更改显示的分组信息
//flag == 1 添加
//flag ==0 删除
//*group  [0]mastID  [...]slaveID
//2018-3-9每个图标表示一个终端，重写分组 hd
void MultigunChargeService::SetTermNameDB(unsigned char flag,unsigned char *group)
{
    QString TempName, TempID,GunNamedis;
    stChargeConfig charge;
    InfoMap qInfoMap;
    InfoAddrType InfoType;
    struct db_result_st result;
    unsigned char gunnamedis_temp=0;
    mgpParamSet->querySetting(&charge, PARAM_CHARGE);
    if(flag && (charge.coupleGun != 0))    //多枪模式
    {
        for(int i =0 ;i<7;i++)
        {
            if(group[i] )    //can地址非0
            {
                if(i==0)   //主枪
                {
                    gunnamedis_temp = GetCoupleGunNamedis(group[i]);
                    GunNamedis = QString::number(gunnamedis_temp,10);
                }
                TempID = QString::number(group[i], 10);
                //TempName = "组"+ GunNamedis+ "-"+TempID  ;
                QString todo = QString("SELECT id,multitype,name FROM terminal_name_multi_table  where canaddr = '%1'").arg(TempID);
                LogOut(todo,1);
                if(mpDBOperate->DBSqlQuery(todo.toAscii().data(), &result, DB_PARAM) == 0)
                {
                    if(result.row>0 && atoi(result.result[1]) !=2) //非充电弓    多模块充电枪或单模块充电枪
                    {
                        if(!(QString(result.result[2]).contains("组",Qt::CaseSensitive)))
                        {
                            TempName = "组"+ GunNamedis+ "-"+result.result[2]  ;
                            todo = QString("UPDATE terminal_name_multi_table SET name = '%1' where canaddr = '%2'").arg(TempName).arg(TempID);
                            LogOut(todo,1);
                            mpDBOperate->DBSqlExec(todo.toAscii().data(), DB_PARAM);
                        }
                        else
                            LogOut("terminal_name_multi_table name has contains 组",1);
                    }
                    mpDBOperate->DBQueryFree(&result);
                }
            }
        }
    }else    //单枪模式
    {
        TempID = QString::number(group[0], 10);
        //查询multi_gun_param_tablel如果包含此地址则不需要更新
        QString todo = QString("SELECT id,multitype,name FROM terminal_name_multi_table  where canaddr = '%1'").arg(TempID);
        LogOut(todo,1);
        if(mpDBOperate->DBSqlQuery(todo.toAscii().data(), &result, DB_PARAM) == 0)
        {
            if(result.row>0 && (atoi(result.result[1])) !=2)  //不是充电弓
            {
                if(QString(result.result[2]).contains("组",Qt::CaseSensitive))
                {
                    //TempName = QString(result.result[2]).mid(3) ;
                    int ret =QString(result.result[2]).indexOf("-");
                    TempName = QString(result.result[2]).mid(ret+1);
                }else
                    TempName = QString(result.result[2]);
                todo = QString("UPDATE  terminal_name_multi_table set name ='%1'  where canaddr = '%2'").arg(TempName).arg(TempID);
                LogOut(todo,1);
                 mpDBOperate->DBSqlExec(todo.toAscii().data(), DB_PARAM);
            }
            mpDBOperate->DBQueryFree(&result);
        }
    }
    InfoType = AddrType_Change_ChargeGunGroup_Info;
    LogOut(QString("AddrType_Change_ChargeGunGroup_Info !"), 3);
    emit sigToInBus(qInfoMap, InfoType);
}

unsigned char  MultigunChargeService::GetCoupleGunNamedis(unsigned char canid)
{
    unsigned char result=0;
    if(canid <=ID_MaxACSinCanID)
    {
        result = canid - ID_MinACSinCanID +1;
    }else if(canid < ID_MinDCCanID)
    {
        result = canid - ID_MinACThrCanID + 1;
    }else if(canid <ID_MinCCUCanID )
    {
        result = canid -ID_MinDCCanID +1 ;
    }
    return result;
}
void MultigunChargeService::UpdateTerminalChargeManner(unsigned char canID,unsigned char chargeManner,unsigned char gunType)
{
    TerminalStatus & stTerminalStatus = mgpDevCache->GetUpdateTerminalStatus(canID);
    stTerminalStatus.chargeManner = chargeManner;//单双枪充电
    stTerminalStatus.chargeResponseFlag = false;//不响应充电指令
    stTerminalStatus.gunType = gunType;//主副枪
    mgpDevCache->FreeUpdateTerminalStatus();
    mgpDevCache->SaveTerminalStatus(canID);
}
///
/// \brief ChargeService::ProcChargeMannerResponseResult_offline
/// \param qInfoMap
/// \param InfoType
///离网情况下开始充电，解析平台响应结果，与上传一致则设置终端可响应充电命令
///检查：是否成功／充电方式／主副枪号
void MultigunChargeService::ProcChargeMannerResponseResult_offline(InfoMap &qInfoMap)
{
    //存放解析出的数据
    unsigned char chargeManner;
    unsigned char canID_master;
    unsigned char temp[7];
    memset(temp,0,7);
    if(qInfoMap.contains(Addr_ChargeGunType))
    {
         chargeManner = qInfoMap.value(Addr_ChargeGunType).at(0);
    }else
        return;
    if(qInfoMap.contains(Addr_ChargeGun_Master))
    {
        canID_master = qInfoMap.value(Addr_ChargeGun_Master).at(0);
        temp[0] = canID_master;
    }else
        return ;
    if(qInfoMap.contains(Addr_ChargeGun_Slave1))
        temp[1] = qInfoMap.value(Addr_ChargeGun_Slave1).at(0);
    if(qInfoMap.contains(Addr_ChargeGun_Slave2))
        temp[2] = qInfoMap.value(Addr_ChargeGun_Slave2).at(0);
    if(qInfoMap.contains(Addr_ChargeGun_Slave3))
        temp[3] = qInfoMap.value(Addr_ChargeGun_Slave3).at(0);
    if(qInfoMap.contains(Addr_ChargeGun_Slave4))
        temp[4] = qInfoMap.value(Addr_ChargeGun_Slave4).at(0);
    if(qInfoMap.contains(Addr_ChargeGun_Slave5))
        temp[5] = qInfoMap.value(Addr_ChargeGun_Slave5).at(0);
    if(qInfoMap.contains(Addr_ChargeGun_Slave6))
        temp[6] = qInfoMap.value(Addr_ChargeGun_Slave6).at(0);

    stChargeConfig charge;

    memset(&charge, 0, sizeof(stChargeConfig));
    mgpParamSet->querySetting(&charge, PARAM_CHARGE);
    TerminalStatus & master = mgpDevCache->GetUpdateTerminalStatus(canID_master);
    master.chargeResponseFlag = true;
    mgpDevCache->FreeUpdateTerminalStatus();
    if(mgpDevCache->SaveTerminalStatus(canID_master)  == false)
    {
        LogOut(QString("更新终端状态失败!!!!!!!!!!"), 3);
    }
    if((canID_master >ID_MaxACThrCanID) && (canID_master < ID_MinCCUCanID) && (charge.vinAuto ==1)//&& (chargeManner == 2)
           // &&(chargeManner == charge.coupleGunNum)       //直流机设备才立刻使用VIN去申请充电，交流设备不需要 hd
        && (GetAutochargeSet(temp,chargeManner)))
    {
         //收到确认后，取vin申请充电
         CHARGE_STEP stChargeStep;//存放从缓存中读出的状态机
         if(mgpDevCache->QueryChargeStep(canID_master, stChargeStep) == false)      //hd 2017-6-19 判断有真实有效胡VIN才申请开始充电
         {
              LogOut(QString("QueryChargeStep false "),2);
         }
         else
         {
             InfoMap qInfoMap;
             qInfoMap.insert(Addr_CanID_Comm, QByteArray(1,canID_master));
             qInfoMap.insert(Addr_BatteryVIN_BMS,QByteArray(stChargeStep.sVIN, LENGTH_VIN_NO));
             LogOut(QString("本地应急充电集控自动VIN申请充电     :%1").arg(QString(qInfoMap[Addr_BatteryVIN_BMS])),2);
             //ProcRecvInVinApplyStartCharge(qInfoMap);   //++++++++
             emit sigInVinApplyStartCharge(qInfoMap);
         }
    }
}

/*
 *获取分组内胡所有枪的自动充电设置属性，如果均允许自动充电则返回1 否则返回0
 *temp 数组内包含本组内的CAN地址
 *chargeManner 本组内的枪个数
*/
unsigned char  MultigunChargeService::GetAutochargeSet(unsigned char *temp,unsigned char chargeManner)
{
    QString todo;
    struct db_result_st result;

    for(int i=0;i<chargeManner;i++)
    {
        todo = QString("SELECT id FROM terminal_autocharge_set_table  where canaddr = '%1' and autochargenum ='%2'").arg(temp[i]).arg(chargeManner);
        if(mpDBOperate->DBSqlQuery(todo.toAscii().data(), &result, DB_PARAM) == 0)
        {
            if(result.row <1)
            {
                 mpDBOperate->DBQueryFree(&result);
                 return 0;
            }
            mpDBOperate->DBQueryFree(&result);
        }
        else
            return 0;
    }
    return 1;
}

unsigned char  MultigunChargeService::GetAutochargeSetAlone(unsigned char temp,unsigned char chargeManner)
{
    QString todo;
    struct db_result_st result;

    todo = QString("SELECT * FROM terminal_autocharge_set_table  where canaddr = '%1' ").arg(temp);
    if(mpDBOperate->DBSqlQuery(todo.toAscii().data(), &result, DB_PARAM) == 0)
    {
        if(result.row <1)
        {
            mpDBOperate->DBQueryFree(&result);
            return 0;
        }else if((result.column ==4 ) && ((atoi(result.result[2]) == chargeManner) || (atoi(result.result[3]) )))       //如果
        {
            mpDBOperate->DBQueryFree(&result);
            return 1;
        }
        LogOut(QString("未获得自动充电权限  column = %1").arg(result.column), 3);
        mpDBOperate->DBQueryFree(&result);
    }
    else
        return 0;
    return 0;
}

bool  MultigunChargeService::MultigunGetMultType(stChargeConfig &charge,TerminalStatus &stTerminalStatus,unsigned char flag)
{
    if(flag)
    {
        //判断条件加入CAN地址判断，只有直流设备才进行判断
        if((charge.coupleGun != 0) && (stTerminalStatus.cCanAddr > ID_MaxACThrCanID) && (stTerminalStatus.cCanAddr < ID_MinCCUCanID) &&\
                ((stTerminalStatus.chargeResponseFlag == false) || (!GetAutochargeSetAlone(stTerminalStatus.cCanAddr,stTerminalStatus.chargeManner))))    //双抢使能+平台确认+自动充电属性校验
        {
             LogOut(QString("未获得分组确认或者未获得自动权限!!!!!!!!!!"), 3);
            return false;
        }
        else
            return true;
    }
    return false;
}

//拔枪时，清除单双枪信息 add by FJC　2017/3/27
//平台要求,拔枪后直流机不上传单双枪信息，集控自清除
void  MultigunChargeService::MultigunClearChargeManner(TerminalStatus &st_TempStatusNow,TerminalStatus &st_TempStatusOld)
{
    stChargeConfig charge;
    TerminalStatus  stTerminalStatus_temp;

    mgpParamSet->querySetting(&charge, PARAM_CHARGE);
     mgpDevCache->QueryTerminalStatus(st_TempStatusNow.cCanAddr,stTerminalStatus_temp);


    if((st_TempStatusNow.stFrameRemoteSingle.link_status == 0) &&
          ((st_TempStatusOld.stFrameRemoteSingle.link_status == 1) ||(st_TempStatusOld.stFrameRemoteSingle.link_status == 3))
            && ((charge.coupleGun != 0) || (stTerminalStatus_temp.gunType !=0)))    //拔枪 并且 双枪功能启动
    {
        SetTermNameDB(0,&st_TempStatusNow.cCanAddr);//hd ++++++分组信息有更新需要同步更新终端名称列表（和数据库） 2
        TerminalStatus &stTerminalStatus = mgpDevCache->GetUpdateTerminalStatus(st_TempStatusNow.cCanAddr);
        stTerminalStatus.chargeManner = 0;//单双枪充电
        stTerminalStatus.chargeResponseFlag = false;//不响应充电指令
        stTerminalStatus.gunType = UNKNOWN;//主副枪
        mgpDevCache->FreeUpdateTerminalStatus();
        mgpDevCache->SaveTerminalStatus(st_TempStatusNow.cCanAddr);
    }
}

bool  MultigunChargeService::MultigunVinEmergencyCouple(InfoMap qInfoMap,unsigned char flag)
{
    stChargeConfig charge;
    TerminalStatus stTerminalStatus ;
    unsigned char cCanAddr;

    if(qInfoMap.contains(Addr_CanID_Comm))
        cCanAddr = qInfoMap[Addr_CanID_Comm].at(0);
    else
        return false;

    if(mgpParamSet->querySetting(&charge, PARAM_CHARGE) == false)
    {
        LogOut(QString("获取充电机状态失败1！！！"),3);
    }

    if(mgpDevCache->QueryTerminalStatus(cCanAddr, stTerminalStatus)== false){
        LogOut(QString("获取终端状态失败2!!!!!!!!!!"), 3);
    }
    if(flag)
    {
        if(MultigunGetMultType(charge,stTerminalStatus,1))
        {
            return true;
        }
        return false;
    }
    else
    {
        //判断条件加入CAN地址判断，只有直流设备才进行判断
        if((charge.coupleGun != 0) && (stTerminalStatus.chargeResponseFlag == false) && (cCanAddr >ID_MaxACThrCanID ) && (cCanAddr <ID_MinCCUCanID ))
         {
              LogOut(QString("未获得分组确认!!!!!!!!!!"), 3);
             return false;
         }
         else
             return true;
    }

}

void MultigunChargeService::MultigunSaveChargeStep(unsigned char canaddr)
{
     CHARGE_STEP stChargeStep;
     stChargeConfig charge;
     if(mgpParamSet->querySetting(&charge, PARAM_CHARGE) == false)
     {
         LogOut(QString("获取充电机状态失败1！！！"),3);
     }
     if(charge.coupleGun != 0)    //只有多枪功能启用才更改开始原因
     {
        if(mgpDevCache->QueryChargeStep(canaddr, stChargeStep))
        {
            stChargeStep.ucCmdSrc = CHARGE_CMD_SRC_CSCU;//101 充电系统控制器
            stChargeStep.ucStartReason = START_CHARGE_CMD_RSN_COUPLE_CHARGE_VIN;//120

            if(mgpDevCache->UpateChargeStep(canaddr, stChargeStep))
            {
                //数据持久化
                if(mgpDevCache->SaveChargeStep(canaddr) == false){
                    LogOut(QString("SaveChargeStep  fail!"), 3);
                }
            }
            LogOut(QString("ucCmdSrc = %1  ucStartReason = %2 ").arg(stChargeStep.ucCmdSrc).arg(stChargeStep.ucStartReason), 3);
         }
     }
}

void MultigunChargeService::MultigunSaveChargeVIN(InfoMap qInfoMap, InfoAddrType InfoType,CHARGE_STEP stChargeSteptemp)
{
     CHARGE_STEP stChargeStep;
     if(!qInfoMap.contains(Addr_CanID_Comm) || !qInfoMap.contains(Addr_BatteryVIN_BMS)){
         LogOut(QString().sprintf("[multigun] VIN Data Incomplete!"), 2);
         return ;
     }
     unsigned char cCanAddr = qInfoMap[Addr_CanID_Comm].at(0);
     stChargeStep = stChargeSteptemp;
     if(InfoType == AddrType_VinNum)
     {

        if(mgpDevCache->QueryChargeStep(cCanAddr, stChargeStep))
        {           
             memcpy(stChargeStep.sVIN, qInfoMap[Addr_BatteryVIN_BMS].data(), LENGTH_VIN_NO);
            if(mgpDevCache->UpateChargeStep(cCanAddr, stChargeStep))
            {
                //数据持久化
                if(mgpDevCache->SaveChargeStep(stChargeStep.ucCanAddr) == false){
                    LogOut(QString("SaveChargeStep  fail!"), 3);
                }
            }
         }
        else
        {
            memcpy(stChargeStep.sVIN, qInfoMap[Addr_BatteryVIN_BMS].data(), LENGTH_VIN_NO);
            stChargeStep.ucCanAddr = cCanAddr;
            stChargeStep.cChargeType = 1;//立即充电
            stChargeStep.cChargeWay = 3;//VIN启动
            if(mgpDevCache->AddChargeStep(cCanAddr, stChargeStep) == false){
                LogOut(QString("ProcReadVinSub add chargestep fail!"), 3);
                return ;
            }
            //数据持久化
            if(mgpDevCache->SaveChargeStep(stChargeStep.ucCanAddr) == false){
                LogOut(QString("SaveChargeStep  fail!"), 3);
            }
        }
     }
}
