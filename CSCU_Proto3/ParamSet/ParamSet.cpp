#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <QFile>
#include <QList>
#include <QDebug>


#include "CommonFunc/commfunc.h"
#include "ParamSet/ParamSet.h"

bool ParamSet::CheckDBTermName()
{
    unsigned char ucACSinNum = 0;
    unsigned char ucACThrNum = 0;
    unsigned char ucDCNum = 0;
    int iCanIDTemp = 0;

    //数据库操作----查询终端名称表
    db_result_st dbst;
    db->DBSqlQuery("SELECT canaddr FROM terminal_name_table", &dbst, DB_PARAM);
    //校验指针是否为空
    if(dbst.result == NULL)
    {
        db->DBQueryFree(&dbst);
        return FALSE;
    }
    //校验终端类型,数量一致
    for(int i = 0; i < dbst.row; i++)
    {
        iCanIDTemp = atoi(dbst.result[i]);
        switch(CheckTermType((unsigned char)iCanIDTemp))
        {
        case TermType_ACSin:
            ucACSinNum++;
            break;
        case TermType_ACThr:
            ucACThrNum++;
            break;
        case TermType_DC:
            ucDCNum++;
            break;
        default:
            break;
        }
    }
    db->DBQueryFree(&dbst);
    if( (ucACSinNum == cscuSysConfig.singlePhase)
            && (ucACThrNum == cscuSysConfig.threePhase)
            && (ucDCNum == cscuSysConfig.directCurrent) )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void ParamSet::InitTermNameDB()
{
    QString TempName, TempID, DBExec, todo;
    //删除当前表中所有内容
    todo = "DELETE FROM terminal_name_table";
    db->DBSqlExec(todo.toAscii().data(), DB_PARAM);
    //终端二维码设置表
    todo = "DELETE FROM terminal_code_param_table";
    db->DBSqlExec(todo.toAscii().data(), DB_PARAM);
    //多枪充电专用表
    todo = "DELETE FROM terminal_name_show_table";
    db->DBSqlExec(todo.toAscii().data(), DB_PARAM);
    //多枪充电专用表
    todo = "DELETE FROM terminal_name_multi_table";
    db->DBSqlExec(todo.toAscii().data(), DB_PARAM);


    //插入各终端名称对应表
    for(int i = 0; i < cscuSysConfig.singlePhase; i++)
    {
        TempID = QString::number(ID_MinACSinCanID + i , 10);
        if(chargeConfig.languageType == 1)
            TempName = TempID + "号交流" ;
        else if(chargeConfig.languageType == 2)
            TempName = TempID + "AC" ;
        DBExec = "INSERT INTO terminal_name_table ( canaddr, name)  VALUES( " + TempID + " , " + " '" + TempName + "' " + " ) ";
        db->DBSqlExec(DBExec.toAscii().data(), DB_PARAM);
        DBExec = "INSERT INTO terminal_name_show_table ( canaddr, name) VALUES( " + TempID + " , " + " '" + TempName + "' " + " ) ";
        db->DBSqlExec(DBExec.toAscii().data(), DB_PARAM);
        DBExec = "INSERT INTO terminal_name_multi_table ( canaddr, name,multitype) VALUES( " + TempID + " , " + " '" + TempName + "' " +" , "+" '"+"0"+"' "+ " ) ";
        db->DBSqlExec(DBExec.toAscii().data(), DB_PARAM);
    }
    for(int i = 0; i < cscuSysConfig.threePhase; i++)
    {
        TempID = QString::number(ID_MinACThrCanID + i , 10);
        if(chargeConfig.languageType == 1)
            TempName = TempID + "号三相" ;
        else if(chargeConfig.languageType == 2)
            TempName = TempID + "Three-AC" ;
        DBExec = "INSERT INTO terminal_name_table ( canaddr, name)  VALUES( " + TempID + " , " + " '" + TempName + "' " + " ) ";
        db->DBSqlExec(DBExec.toAscii().data(), DB_PARAM);
        DBExec = "INSERT INTO terminal_name_show_table ( canaddr, name) VALUES( " + TempID + " , " + " '" + TempName + "' " + " ) ";
        db->DBSqlExec(DBExec.toAscii().data(), DB_PARAM);
        DBExec = "INSERT INTO terminal_name_multi_table ( canaddr, name,multitype) VALUES( " + TempID + " , " + " '" + TempName + "' " + "  , "+" '"+"0"+"' "+ " ) ";
        db->DBSqlExec(DBExec.toAscii().data(), DB_PARAM);
    }
    for(int i = 0; i < cscuSysConfig.directCurrent; i++)
    {
        TempID = QString::number(ID_MinDCCanID + i , 10);
        if(chargeConfig.languageType == 1)
            TempName = TempID + "号直流" ;
        if(chargeConfig.languageType == 2)
            TempName = TempID + "DC" ;
        DBExec = "INSERT INTO terminal_name_table ( canaddr, name)  VALUES( " + TempID + " , " + " '" + TempName + "' " + " ) ";
        db->DBSqlExec(DBExec.toAscii().data(), DB_PARAM);
        DBExec = "INSERT INTO terminal_name_show_table ( canaddr, name) VALUES( " + TempID + " , " + " '" + TempName + "' " + " ) ";
        db->DBSqlExec(DBExec.toAscii().data(), DB_PARAM);
        DBExec = "INSERT INTO terminal_name_multi_table ( canaddr, name,multitype) VALUES( " + TempID + " , " + " '" + TempName + "' " + " , "+" '"+"0"+"' "+ "  ) ";
        db->DBSqlExec(DBExec.toAscii().data(), DB_PARAM);
    }
    InitMultiGunName();
}

//初始化终端名称列表(写数据库)
//若终端个数不一致, 则清空二维码数据库
void ParamSet::InitSignleTermNameDB()
{
    QString TempName, TempID, DBExec, todo;
    //删除当前表中所有内容
    todo = "DELETE FROM terminal_name_table";
    db->DBSqlExec(todo.toAscii().data(), DB_PARAM);
    //终端二维码设置表
    todo = "DELETE FROM terminal_code_param_table";
    db->DBSqlExec(todo.toAscii().data(), DB_PARAM);
    //多枪充电专用表
    todo = "DELETE FROM terminal_name_show_table";
    db->DBSqlExec(todo.toAscii().data(), DB_PARAM);

    //插入各终端名称对应表
    int j = 0;

    for(int i = 0; i < cscuSysConfig.directCurrent; i++)
    {
        TempID = QString::number(ID_MinDCCanID + i , 10);
        if(chargeConfig.languageType == 1)
            TempName = QString('A'+j++)+"直流";
        else if(chargeConfig.languageType == 2)
            TempName = QString('A'+j++)+"-"+"DC";
        DBExec = "INSERT INTO terminal_name_table ( canaddr, name)  VALUES( " + TempID + " , " + " '" + TempName + "' " + " ) ";
        db->DBSqlExec(DBExec.toAscii().data(), DB_PARAM);
        DBExec = "INSERT INTO terminal_name_show_table ( canaddr, name) VALUES( " + TempID + " , " + " '" + TempName + "' " + " ) ";
        db->DBSqlExec(DBExec.toAscii().data(), DB_PARAM);
    }

    for(int i = 0; i < cscuSysConfig.threePhase; i++)
    {
        TempID = QString::number(ID_MinACThrCanID + i , 10);
        if(chargeConfig.languageType == 1)
            TempName = QString('A'+j++)+"三相";
        else if(chargeConfig.languageType == 2)
            TempName = QString('A'+j++)+"-"+"Three-AC";
        DBExec = "INSERT INTO terminal_name_table ( canaddr, name)  VALUES( " + TempID + " , " + " '" + TempName + "' " + " ) ";
        db->DBSqlExec(DBExec.toAscii().data(), DB_PARAM);
        DBExec = "INSERT INTO terminal_name_show_table ( canaddr, name) VALUES( " + TempID + " , " + " '" + TempName + "' " + " ) ";
        db->DBSqlExec(DBExec.toAscii().data(), DB_PARAM);
    }
    for(int i = 0; i < cscuSysConfig.singlePhase; i++)
    {
        TempID = QString::number(ID_MinACSinCanID + i , 10);
        if(chargeConfig.languageType == 1)
            TempName = QString('A'+j++)+"交流";
        else if(chargeConfig.languageType == 2)
            TempName = QString('A'+j++)+"-"+"AC";
        DBExec = "INSERT INTO terminal_name_table ( canaddr, name)  VALUES( " + TempID + " , " + " '" + TempName + "' " + " ) ";
        db->DBSqlExec(DBExec.toAscii().data(), DB_PARAM);
        DBExec = "INSERT INTO terminal_name_show_table ( canaddr, name) VALUES( " + TempID + " , " + " '" + TempName + "' " + " ) ";
        db->DBSqlExec(DBExec.toAscii().data(), DB_PARAM);
    }
}
ParamSet::ParamSet()
{
	char cmdBuff[300];
	filename = MAIN_CONFIG;
	bus = CBus::GetInstance();
	db = DBOperate::GetInstance();
	log = Log::GetInstance();

	rebootFlag = 0;
	terminalNumChange = false;
	dnsChange = false;
	configInited = false;

	QFile *pFile = new QFile(filename);
    if(!pFile->exists()){
		initDefaultSetting();
	}
	delete pFile;
	loadSetting();  
    //校验不通过
    if(!CheckDBTermName())
    {
        if(chargeConfig.ucDevType==1)
        {
            InitSignleTermNameDB();  //初始化单桩名称
        }else
        {
            InitTermNameDB(); //群充名称
        }
    }
    QObject::connect(this, SIGNAL(sigUpdateSetting(QVariant, int)), this, SLOT(slotUpdateSetting(QVariant, int)));

	QList<int> list;
	//list.append(AddrType_TermSignal);
    bus->RegistDev(this, list);

    updateSetting(&can0Config, PARAM_CAN0);
	updateSetting(&can1Config, PARAM_CAN1);
	updateSetting(&net0Config, PARAM_NET0);
	updateSetting(&net1Config, PARAM_NET1);
	updateSetting(&cscuSysConfig, PARAM_CSCU_SYS);
	updateSetting(&ccuSysConfig, PARAM_CCU_SYS);
	updateSetting(&chargeConfig, PARAM_CHARGE);
	updateSetting(&screenConfig, PARAM_SCREEN);
	updateSetting(&ioConfig, PARAM_IO);
    updateSetting(&magneticConfig, PARAM_MAGNETIC);
    updateSetting(&extraConfig, PARAM_EXTRA);
	updateSetting(&powerLimitConfig, PARAM_POWERLIMIT);
	updateSetting(&smartChargeConfig, PARAM_SMARTCHARGE);
	updateSetting(&server0Config, PARAM_SERVER0);
	updateSetting(&server1Config, PARAM_SERVER1);
	updateSetting(&serverListConfig, PARAM_SERVERLIST);
    updateSetting(&smartCarConfig, PARAM_SMARTCAR);
	updateSetting(&emergencyConfig, PARAM_EMERGENCY);
	updateSetting(&webserverConfig, PARAM_WEBSERVER);
    updateSetting(&serverDMConfig, PARAM_SERVER_DM);

	loadAmmeterRange();
	loadAcMeterScale();

	configInited = true;
	
    snprintf(cmdBuff, sizeof(cmdBuff), "ifconfig eth0 %s netmask %s up\n", net0Config.localIp, net0Config.netMask);
    system(cmdBuff);
    snprintf(cmdBuff, sizeof(cmdBuff),"route add default gw %s  metric 300\n", net0Config.gateway);
    system(cmdBuff);
    snprintf(cmdBuff, sizeof(cmdBuff), "ifconfig eth1 %s netmask %s up\n", net1Config.localIp, net1Config.netMask);
    system(cmdBuff);
}

ParamSet::~ParamSet()
{

}

/**
 *
 */
int ParamSet::needReboot()
{
	return rebootFlag;
}

void ParamSet::setRebootFlag(int flag)
{
	rebootFlag = flag;
}
/*
void ParamSet::slotFromBus(InfoMap TelecontrolMap, InfoAddrType enAddrType) //test
{
#if 0
	if(enAddrType == AddrType_TermSignal)
	{
	}
#endif
}*/
/**
 *加载配置
 */
void ParamSet::loadSetting()
{
    QSettings *settings = new QSettings(filename, QSettings::IniFormat);
	QString strTmp;

    settings->beginGroup("CAN_0");
    can0Config.canAddr = settings->value("CanAddr", 250).toInt();
    can0Config.canRate = settings->value("Rate", 10000).toInt();
    settings->endGroup();

	settings->beginGroup("CAN_1");
    can1Config.canAddr = settings->value("CanAddr", 251).toInt();
    can1Config.canRate = settings->value("Rate", 10000).toInt();
    settings->endGroup();

    settings->beginGroup("SERVER0");
	strTmp = settings->value("ServerIP1","").toString();
    snprintf(server0Config.serverIp1, sizeof(server0Config.serverIp1), "%s", strTmp.toAscii().data());
    server0Config.serverPort1 = settings->value("ServerPort1", "").toInt();
	strTmp = settings->value("ServerIP2","").toString();
    snprintf(server0Config.serverIp2, sizeof(server0Config.serverIp2), "%s", strTmp.toAscii().data());
    server0Config.serverPort2 = settings->value("ServerPort2", "").toInt();
	strTmp = settings->value("ServerIP3","").toString();
    snprintf(server0Config.serverIp3, sizeof(server0Config.serverIp3), "%s", strTmp.toAscii().data());
    server0Config.serverPort3 = settings->value("ServerPort3", "").toInt();
	//strTmp = settings->value("StationName", "").toString(); 
	//snprintf(server0Config.stationName, sizeof(server0Config.stationName), "%s", strTmp.toAscii().data());
	strTmp = settings->value("StationNo", "").toString();
	snprintf(server0Config.stationNo, sizeof(server0Config.stationNo), "%s", strTmp.toAscii().data());
	strTmp = settings->value("AesKey", "41414141414141414141414141414141").toString();
	snprintf(server0Config.aesKey, sizeof(server0Config.aesKey), "%s", strTmp.toAscii().data());
    server0Config.encrypt = settings->value("Encrypt", true).toBool();
	settings->endGroup();

    settings->beginGroup("SERVER1");
	strTmp = settings->value("ServerIP","").toString();
    snprintf(server1Config.serverIp, sizeof(server1Config.serverIp), "%s", strTmp.toAscii().data());
    server1Config.serverPort = settings->value("ServerPort", "").toInt();
	//strTmp = settings->value("StationName", "").toString(); 
	//snprintf(server1Config.stationName, sizeof(server1Config.stationName), "%s", strTmp.toAscii().data());
	strTmp = settings->value("StationNo", "").toString();
	snprintf(server1Config.stationNo, sizeof(server1Config.stationNo), "%s", strTmp.toAscii().data());
	strTmp = settings->value("AesKey", "41414141414141414141414141414141").toString();
	snprintf(server1Config.aesKey, sizeof(server1Config.aesKey), "%s", strTmp.toAscii().data());
	server1Config.encrypt = settings->value("Encrypt", 1).toInt();
    settings->endGroup();

	settings->beginGroup("SERVER_LIST");
	strTmp = settings->value("ServerIP1","").toString();
    snprintf(serverListConfig.serverIp1, sizeof(serverListConfig.serverIp1), "%s", strTmp.toAscii().data());
    serverListConfig.serverPort1 = settings->value("ServerPort1", "").toInt();
	strTmp = settings->value("ServerIP2","").toString();
    snprintf(serverListConfig.serverIp2, sizeof(serverListConfig.serverIp2), "%s", strTmp.toAscii().data());
    serverListConfig.serverPort2 = settings->value("ServerPort2", "").toInt();
	strTmp = settings->value("ServerIP3","").toString();
    snprintf(serverListConfig.serverIp3, sizeof(serverListConfig.serverIp3), "%s", strTmp.toAscii().data());
    serverListConfig.serverPort3 = settings->value("ServerPort3", "").toInt();
    settings->endGroup();

    //设备管理服务器参数设置项 add by XX 2017-03-30
    settings->beginGroup("SERVER_DEVICE_MANAGE");
    strTmp = settings->value("ServerIP","").toString();
    snprintf(serverDMConfig.serverIp, sizeof(serverDMConfig.serverIp), "%s", strTmp.toAscii().data());
    serverDMConfig.serverPort = settings->value("ServerPort", "0").toInt();
    strTmp = settings->value("UserName","admin").toString();
    snprintf(serverDMConfig.userName, sizeof(serverDMConfig.userName), "%s", strTmp.toAscii().data());
    strTmp = settings->value("Passwd", "tgood300001").toString();
    snprintf(serverDMConfig.passwd, sizeof(serverDMConfig.passwd), "%s", strTmp.toAscii().data());
    settings->endGroup();

    settings->beginGroup("NET_0");
	strTmp = settings->value("IP","10.0.10.123").toString();
    snprintf(net0Config.localIp, sizeof(net0Config.localIp),"%s", strTmp.toAscii().data());
	strTmp = settings->value("NetMask", "255.255.255.0").toString();
    snprintf(net0Config.netMask, sizeof(net0Config.netMask), "%s", strTmp.toAscii().data());
    strTmp = settings->value("Gateway", "10.0.10.254").toString();
	snprintf(net0Config.gateway, sizeof(net0Config.gateway), "%s", strTmp.toAscii().data());
    settings->endGroup();
	
	settings->beginGroup("NET_1");
   	strTmp = settings->value("IP","192.168.1.123").toString();
    snprintf(net1Config.localIp, sizeof(net1Config.localIp),"%s", strTmp.toAscii().data());
	strTmp = settings->value("NetMask", "255.255.255.0").toString();
    snprintf(net1Config.netMask, sizeof(net1Config.netMask), "%s", strTmp.toAscii().data());
    strTmp = settings->value("Gateway", "192.168.1.254").toString();
	snprintf(net1Config.gateway, sizeof(net1Config.gateway), "%s", strTmp.toAscii().data());
	settings->endGroup();

	settings->beginGroup("CSCUSys");
	strTmp = settings->value("Version","CSCU_A1_BG").toString();
	snprintf(cscuSysConfig.version, sizeof(cscuSysConfig.version), "%s", strTmp.toAscii().data());
    strTmp = settings->value("DNS", "114.114.114.114").toString();
	snprintf(cscuSysConfig.dns, sizeof(cscuSysConfig.dns), "%s", strTmp.toAscii().data());

	strTmp = settings->value("StationName", "").toString(); 
	snprintf(cscuSysConfig.stationName, sizeof(cscuSysConfig.stationName), "%s", strTmp.toAscii().data());
	
	cscuSysConfig.normalCardType = settings->value("NormalCard", 1).toInt();
    cscuSysConfig.boardType = settings->value("BoardType", 3).toInt();
    cscuSysConfig.password = settings->value("Password", 0x493e1).toInt();
    cscuSysConfig.localChargePassword = settings->value("LocalChargePassword", 0x493e1).toInt();

    cscuSysConfig.CCUnum= settings->value("CCUNum",0).toInt();
    cscuSysConfig.directCurrent= settings->value("DirectCurrent",1).toInt();
    cscuSysConfig.singlePhase = settings->value("SinglePhase", 0).toInt();
    cscuSysConfig.threePhase = settings->value("ThreePhase", 0).toInt();
    settings->endGroup();

    settings->beginGroup("CCUSys");
    settings->endGroup();

    //change by XX 2017-06-14
	settings->beginGroup("CHARGE");
	chargeConfig.vinOffline = settings->value("VinOffLine", 0).toInt();	
	chargeConfig.localStop = settings->value("LocalStop", 0).toInt();	
    chargeConfig.cardType = settings->value("CardType", 2).toInt();
	chargeConfig.vinAuto =  settings->value("VinAuto", 0).toInt();
	chargeConfig.cardAuto = settings->value("CardAuto", 0).toInt();
	chargeConfig.vinType = settings->value("VinType", 0).toInt();
    chargeConfig.coupleGun = settings->value("CoupleGun",0).toInt();
    //chargeConfig.coupleGunNum = settings->value("CoupleGunNum",0).toInt();
    chargeConfig.energyFilter = settings->value("EnergyFilter", 0).toInt();;
    chargeConfig.localPolicy = settings->value("LocalPolicy", 0).toInt();
    chargeConfig.fgpjEnable = settings->value("FGPJEnable", 0).toInt();

    chargeConfig.localChargeEnable = settings->value("LocalChargeEnable", 0).toInt();
    chargeConfig.localChargeType = settings->value("LocalChargeType", 0).toInt();

	chargeConfig.meterType = settings->value("MeterType", 0).toInt();	
    chargeConfig.ticketEnable = settings->value("TicketEnable",0).toInt();
    chargeConfig.ucDevType = settings->value("DevType",0).toInt();   //nihai add 增加设备类型设置
    chargeConfig.languageType = settings->value("Language",1).toInt();
	settings->endGroup();


	settings->beginGroup("SCREEN");
	strTmp = settings->value("Version", "v2.0").toString();
	snprintf(screenConfig.version, sizeof(screenConfig.version), "%s", strTmp.toAscii().data() );
	strTmp = settings->value("Password", "0001e240").toString();
	snprintf(screenConfig.screenPwd, sizeof(screenConfig.screenPwd), "%s", strTmp.toAscii().data());
	settings->endGroup();

    settings->beginGroup("POWER_LIMIT");
    powerLimitConfig.sPowerLimit_Enable = settings->value("PowerLimit_Enable",FALSE).toBool();
    powerLimitConfig.sCCUcount = settings->value("CCUCount",0).toInt();
    powerLimitConfig.STATION_LIMT_POWER = settings->value("StationLimitPower",0).toInt();
    powerLimitConfig.SAFE_CHARGE_POWER = settings->value("StationSafePower",0).toInt();
    powerLimitConfig.sSUMPower_Manual = settings->value("SumPower_Manual", 0).toInt();
    powerLimitConfig.sSUMPower_Server = settings->value("SumPower_Server", 0).toInt();;
    powerLimitConfig.sSUMPower_Ammeter_Enable = settings->value("SumPower_Ammeter_Enable", FALSE).toBool();
    powerLimitConfig.sSUMPower_Manual_Enable = settings->value("SumPower_Manual_Enable", FALSE).toBool();
    powerLimitConfig.sSUMPower_Server_Enable = settings->value("SumPower_Server_Enable", FALSE).toBool();
    settings->endGroup();

    settings->beginGroup("SMART_CHARGE");
    smartChargeConfig.sSmartCharge_Enable = settings->value("SmartCharge_Enable",FALSE).toBool();
    settings->endGroup();

    settings->beginGroup("MAGNETICSWITCH");
    magneticConfig.bOpenDoorPowerOutages = settings->value("bOpenDoorPowerOutages", FALSE).toBool();
    settings->endGroup();

	settings->beginGroup("CSCU_EXTRA");
    extraConfig.coupleGun300KW = settings->value("CoupleGun300KW", 0).toInt();
    extraConfig.localAddress = settings->value("LocalAddress", 0).toInt();
    settings->endGroup();

    settings->beginGroup("SMART_CAR");
    smartCarConfig.sSmartCar_Enable = settings->value("SmartCar_Enable",FALSE).toBool();
    smartCarConfig.CCUtotalnum = settings->value("CCUtotalnum",0).toInt();
    smartCarConfig.gunnum[0] = settings->value("GunNum1",0).toInt();
    smartCarConfig.ModuleNum[0] = settings->value("ModuleNum1",0).toInt();

    smartCarConfig.gunnum[1] = settings->value("GunNum2",0).toInt();
     smartCarConfig.ModuleNum[1] = settings->value("ModuleNum2",0).toInt();

    smartCarConfig.gunnum[2] = settings->value("GunNum3",0).toInt();
     smartCarConfig.ModuleNum[2] = settings->value("ModuleNum3",0).toInt();

    smartCarConfig.gunnum[3] = settings->value("GunNum4",0).toInt();
    smartCarConfig.ModuleNum[3] = settings->value("ModuleNum4",0).toInt();
    settings->endGroup();

	char nameStr[20];
	settings->beginGroup("IOIN");
	for(int i=0; i< 10; i++)
	{
		snprintf(nameStr, sizeof(nameStr), "DIN%d", i+1);
		ioConfig.inOpenColse[i] = settings->value(QString(nameStr), 0).toInt();
	}
	settings->endGroup();


    settings->beginGroup("EMERGENCY_CHARGE");
	emergencyConfig.emergency_enable = settings->value("emergency_enable", false).toBool();
	emergencyConfig.vin_authenticate = settings->value("vin_authenticate", true).toBool();
	emergencyConfig.car_authenticate = settings->value("car_authenticate", true).toBool();
	emergencyConfig.card_authenticate = settings->value("card_authenticate", true).toBool();
	emergencyConfig.queue_gun = settings->value("queue_gun", false).toBool();
	emergencyConfig.queue_card = settings->value("queue_card", false).toBool();
	emergencyConfig.queue_car = settings->value("queue_car", false).toBool();
	emergencyConfig.check_time = settings->value("check_time", 40).toInt();
	emergencyConfig.duration = settings->value("duration", 10).toInt();
	emergencyConfig.order_count = settings->value("order_count", 2000).toInt();
	settings->endGroup();

	settings->beginGroup("WEBSERVER");
	snprintf(webserverConfig.url, sizeof(webserverConfig.url), "%s", settings->value("Url", "").toString().toAscii().data());
	webserverConfig.port = settings->value("Port", 0).toInt();
	snprintf(webserverConfig.operator_id, sizeof(webserverConfig.operator_id), "%s", 
			settings->value("OperatorID", "").toString().toAscii().data());
	snprintf(webserverConfig.operator_secret, sizeof(webserverConfig.operator_secret), "%s", 
			settings->value("OperatorSecret", "").toString().toAscii().data());
	snprintf(webserverConfig.data_secret, sizeof(webserverConfig.data_secret), "%s", 
			settings->value("DataSecret", "").toString().toAscii().data());
	snprintf(webserverConfig.aes_key, sizeof(webserverConfig.aes_key), "%s", 
			settings->value("AesKey", "").toString().toAscii().data());
	snprintf(webserverConfig.aes_iv, sizeof(webserverConfig.aes_iv), "%s", 
			settings->value("AesIV", "").toString().toAscii().data());
	settings->endGroup();
} 
/**
 *初始化默认配置， 在检测不到配置文件的事调用
 */
void ParamSet::initDefaultSetting()
{
	QSettings *settings =  new QSettings(filename,QSettings::IniFormat);

	settings->beginGroup("CAN_0");
	settings->setValue("CanAddr", 250);
	settings->setValue("Rate", 10000);
	settings->endGroup();

	settings->beginGroup("CAN_1");
	settings->setValue("CanAddr", 250);
	settings->setValue("Rate", 10000);
	settings->endGroup();

	settings->beginGroup("SERVER0");
	settings->setValue("ServerIP1", "");
	settings->setValue("ServerPort1", "");
	settings->setValue("ServerIP2", "");
	settings->setValue("ServerPort2", "");
	settings->setValue("ServerIP3", "");
	settings->setValue("ServerPort3", "");
	settings->setValue("StationNo", "");
	//settings->setValue("StationName", "特来电充电站");
	settings->setValue("Encrypt", 1);
	settings->setValue("AesKey","41414141414141414141414141414141");
	settings->endGroup();

	settings->beginGroup("SERVER1");
	settings->setValue("ServerIP", "");
	settings->setValue("ServerPort", "");
	settings->setValue("StationNo", "");
	//settings->setValue("StationName", "特来电充电站");
	settings->setValue("Encrypt", 1);
	settings->setValue("AesKey","41414141414141414141414141414141");
	settings->endGroup();

	settings->beginGroup("SERVER_LIST");
	settings->setValue("ServerIP1","");
    settings->setValue("ServerPort1", "");
	settings->setValue("ServerIP2","");
    settings->setValue("ServerPort2", "");
	settings->setValue("ServerIP3","");
    settings->setValue("ServerPort3", "");
    settings->endGroup();

    //设备管理服务器参数设置项 add by XX 2017-03-30
    settings->beginGroup("SERVER_DEVICE_MANAGE");
    settings->setValue("ServerIP", "");
    settings->setValue("ServerPort", "");
    settings->setValue("UserName", "");
    settings->setValue("Passwd", "");
    settings->endGroup();

	settings->beginGroup("NET_0");
	settings->setValue("IP", "10.0.10.123");
	settings->setValue("NetMask", "255.255.255.0");
	settings->setValue("Gateway", "10.0.10.254");
	settings->endGroup();

	settings->beginGroup("NET_1");
	settings->setValue("IP", "192.168.1.123");
	settings->setValue("NetMask", "255.255.255.0");
	settings->setValue("Gateway", "192.168.1.1");
	settings->endGroup();

	settings->beginGroup("CSCUSys");
	settings->setValue("StationName", "特来电充电站");
	settings->setValue("NormalCard", 1);
    settings->setValue("BoardType", 3);
    settings->setValue("Password", 0x493e1);
    settings->setValue("LocalChargePassword", 0x493e1);

	settings->setValue("DirectCurrent", 1);
	settings->setValue("SinglePhase", 0);
	settings->setValue("ThreePhase", 0);
    settings->setValue("CCUNum", 0);
    settings->setValue("DevType",2); //nihai add 默认使用群充系统
	settings->setValue("Version", "CSCU_A_G1");
	settings->setValue("DNS", "114.114.114.114");
	settings->endGroup();

	settings->beginGroup("CCUSys");
	settings->endGroup();

	settings->beginGroup("CHARGE");
	settings->setValue("VinOffLine", 0);
	settings->setValue("LocalStop", 0);
    settings->setValue("CardType", 2);
	settings->setValue("VinAuto", 0);
	settings->setValue("CardAuto",1);
	settings->setValue("VinType", 1);
	settings->setValue("EnergyFilter", 0);
    settings->setValue("CoupleGun",0);
    settings->setValue("LocalPolicy", 0);
    settings->setValue("FGPJEnable", 0);
    settings->setValue("Language",1);
    settings->setValue("LocalChargeEnable", 0);
    settings->setValue("LocalChargeType", 0);
	settings->setValue("MeterType", 0);
	settings->endGroup();

	settings->beginGroup("SCREEN");
	settings->setValue("Version", "v2.0");
	settings->setValue("Password", "0001e240");
	settings->endGroup();

    settings->beginGroup("POWER_LIMIT");
    settings->setValue("PowerLimit_Enable", FALSE);
    settings->setValue("CCUCount", 0);
    settings->setValue("StationLimitPower", 0);
    settings->setValue("StationSafePower", 0);
    settings->setValue("SumPower_Manual", 0);
    settings->setValue("SumPower_Server", 0);
    settings->setValue("SumPower_Ammeter_Enable", FALSE);
    settings->setValue("SumPower_Manual_Enable", FALSE);
    settings->setValue("SumPower_Server_Enable", FALSE);
    settings->endGroup();

    settings->beginGroup("SMART_CHARGE");
    settings->setValue("SmartCharge_Enable",FALSE);
    settings->endGroup();

    settings->beginGroup("MAGNETICSWITCH");
    settings->setValue("bOpenDoorPowerOutages", FALSE);
    settings->endGroup();

	settings->beginGroup("CSCU_EXTRA");
    settings->setValue("CoupleGun300KW", 0);
    settings->setValue("LocalAddress", 0);
    settings->endGroup();

	settings->beginGroup("EMERGENCY_CHARGE");
	settings->setValue("emergency_enable", false);
	settings->setValue("vin_authenticate", true);
	settings->setValue("car_authenticate", true);
	settings->setValue("card_authenticate", true);
	settings->setValue("queue_gun", false);
	settings->setValue("queue_card", false);
	settings->setValue("queue_car", false);
	settings->setValue("check_time", 40);
	settings->setValue("duration", 10);
	settings->setValue("order_count", 2000);
	settings->endGroup();

	settings->beginGroup("WEBSERVER");
	settings->setValue("Url", "");
	settings->setValue("Port", 0);
	settings->setValue("OperatorID", "");
	settings->setValue("OperatorSecret", "");
	settings->setValue("DataSecret", "");
	settings->setValue("AesKey", "");
	settings->setValue("AesIV", "");
	settings->endGroup();

	char nameStr[20];
	settings->beginGroup("IOIN");
	for(int i=0; i< 10; i++)
	{
		snprintf(nameStr, sizeof(nameStr), "DIN%d", i+1);
		settings->setValue(nameStr, 0);
	}
	settings->endGroup();

	delete settings;
}
/**
 *更新配置
 */
bool ParamSet::updateSetting(void *data, int type)
{
	QVariant var;
	if(type > PARAM_IO)
	{
		if(type == PARAM_AMMETER)
			var.setValue(*((stAllAmmeterConfig *)data));
		else if(type == PARAM_PHASE_TYPE)
			var.setValue(*((stThreePhaseTypeConfig *)data));
		else if(type == PARAM_TPFV)
			var.setValue(*((stAllTPFVConfig *)data));
		else if(type == PARAM_LOCALPOLICY)
			var.setValue(*((AllLocalPolicyConfig *)data));
		else if(type == PARAM_FGPJ)
			var.setValue(*((AllFGPJConfig *)data));
	}
	else
		var.setValue(*((unParamConfig *)data));

	emit sigUpdateSetting(var, type);

	return true;
}

/**
 *查询直流机终端数据
 */
void ParamSet::queryTerminalConfig(stTerminalConfig * terminalConfig, unsigned char canaddr)
{
	struct db_result_st result;
	char cmd_sql[256];

	snprintf(cmd_sql, sizeof(cmd_sql), "select canaddr, strategy, contrlo_mode, aux_type from terminal_param_table where canaddr=%d", canaddr);

	if(0 == db->DBSqlQuery((char *)cmd_sql, &result, DB_PARAM))
	{
		stTerminalConfig config;

		if(result.column == 4)
		{
			config.canaddr = atoi(result.result[result.column]);
			config.strategy = atoi(result.result[result.column+1]);
			config.controlMode = atoi(result.result[result.column+2]);
			config.auxPowerType = atoi(result.result[result.column+3]);
			
			*terminalConfig = config;
		}

		db->DBQueryFree(&result);
	}
}
/**
 *更新直流机终端数据
 */
void ParamSet::updateTerminalConfig(stTerminalConfig config, unsigned char canaddr)
{
	char cmd_buff[256];
	struct db_result_st result;
	snprintf(cmd_buff, sizeof(cmd_buff), "select id from terminal_param_table where canaddr=\"%d\" limit 1", canaddr);
	
	if(0 == db->DBSqlQuery((char *)cmd_buff, &result, DB_PARAM))
	{
		if(result.row == 0 && result.column == 0)
			snprintf(cmd_buff, sizeof(cmd_buff), "insert into terminal_param_table (canaddr, data) values(\"%d\", "")", canaddr);
		else
			snprintf(cmd_buff, sizeof(cmd_buff), "update terminal_param_table set strategy=\"%d\" contrlo_mode=\"%d\" aux_type=\"%d\" where canaddr=\"%d\"", config.strategy, config.controlMode, config.auxPowerType, canaddr);
	
		db->DBQueryFree(&result);
		
		db->DBSqlExec(cmd_buff, DB_PARAM);
	}
}
/**
 *更新到配置文件
 */
void ParamSet::updateConfig(int type)
{
	QSettings *settings = new QSettings(filename,QSettings::IniFormat);

	switch(type)
	{
		case PARAM_CAN0:
			settings->beginGroup("CAN_0");
			settings->setValue("CanAddr", can0Config.canAddr);
			settings->setValue("Rate", can0Config.canRate);
			settings->endGroup();
			break;
		case PARAM_CAN1:
			settings->beginGroup("CAN_1");
			settings->setValue("CanAddr", can1Config.canAddr);
			settings->setValue("Rate", can1Config.canRate);
			settings->endGroup();
			break;
		case PARAM_NET0:
			settings->beginGroup("NET_0");
			settings->setValue("IP", QString(net0Config.localIp));
			settings->setValue("NetMask", QString(net0Config.netMask));
			settings->setValue("Gateway", QString(net0Config.gateway));
			settings->endGroup();
			break;
		case PARAM_NET1:
			settings->beginGroup("NET_1");
			settings->setValue("IP", QString(net1Config.localIp));
			settings->setValue("NetMask", QString(net1Config.netMask));
			settings->setValue("Gateway", QString(net1Config.gateway));
			settings->endGroup();
			break;
		case PARAM_SERVER0:
			settings->beginGroup("SERVER0");
			settings->setValue("ServerIP1", QString(server0Config.serverIp1));
			settings->setValue("ServerPort1", server0Config.serverPort1);
			settings->setValue("ServerIP2", QString(server0Config.serverIp2));
			settings->setValue("ServerPort2", server0Config.serverPort2);
			settings->setValue("ServerIP3", QString(server0Config.serverIp3));
			settings->setValue("ServerPort3", server0Config.serverPort3);
			settings->setValue("StationNo", QString(server0Config.stationNo));
			//settings->setValue("StationName", QString(server0Config.stationName));
			settings->setValue("Encrypt", server0Config.encrypt);
			settings->setValue("AesKey", QString(server0Config.aesKey));
			settings->endGroup();
			break;
		case PARAM_SERVER1:
			settings->beginGroup("SERVER1");
			settings->setValue("ServerIP", QString(server1Config.serverIp));
			settings->setValue("ServerPort", server1Config.serverPort);
			settings->setValue("StationNo", QString(server1Config.stationNo));
			//settings->setValue("StationName", QString(server1Config.stationName));
			settings->setValue("Encrypt", server1Config.encrypt);
			settings->setValue("AesKey", QString(server1Config.aesKey));
			settings->endGroup();
			break;
		case PARAM_SERVERLIST:
			settings->beginGroup("SERVER_LIST");
			settings->setValue("ServerIP1", QString(serverListConfig.serverIp1));
			settings->setValue("ServerPort1", serverListConfig.serverPort1);
			settings->setValue("ServerIP2", QString(serverListConfig.serverIp2));
			settings->setValue("ServerPort2", serverListConfig.serverPort2);
			settings->setValue("ServerIP3", QString(serverListConfig.serverIp3));
			settings->setValue("ServerPort3", serverListConfig.serverPort3);
			settings->endGroup();
			break;
		case PARAM_SERVER_DM:   //add  by XX 2017-03-30
			settings->beginGroup("SERVER_DEVICE_MANAGE");
			settings->setValue("ServerIP", QString(serverDMConfig.serverIp));
			settings->setValue("ServerPort", serverDMConfig.serverPort);
			settings->setValue("UserName", QString(serverDMConfig.userName));
			settings->setValue("Passwd", serverDMConfig.passwd);
			settings->endGroup();
			break;
		case PARAM_CHARGE:
			settings->beginGroup("CHARGE");
			settings->setValue("VinOffLine", chargeConfig.vinOffline);
			settings->setValue("LocalStop", chargeConfig.localStop);
			settings->setValue("CardType", chargeConfig.cardType);
			settings->setValue("VinAuto", chargeConfig.vinAuto);
			settings->setValue("CardAuto", chargeConfig.cardAuto);
			settings->setValue("VinType", chargeConfig.vinType);
			settings->setValue("EnergyFilter", chargeConfig.energyFilter);
            settings->setValue("CoupleGun", chargeConfig.coupleGun);
			settings->setValue("LocalPolicy", chargeConfig.localPolicy);
			settings->setValue("FGPJEnable", chargeConfig.fgpjEnable);
            settings->setValue("LocalChargeEnable", chargeConfig.localChargeEnable);
            settings->setValue("LocalChargeType", chargeConfig.localChargeType);
			settings->setValue("MeterType", chargeConfig.meterType);
            settings->setValue("TicketEnable",chargeConfig.ticketEnable);
            settings->setValue("DevType",chargeConfig.ucDevType);  //nihai add 增加单桩和群充设置
            settings->setValue("Language",chargeConfig.languageType);
			settings->endGroup();
			break;
		case PARAM_CSCU_SYS:
			settings->beginGroup("CSCUSys");
			settings->setValue("StationName", QString(cscuSysConfig.stationName));
			settings->setValue("NormalCard", cscuSysConfig.normalCardType);
			settings->setValue("BoardType", cscuSysConfig.boardType);
			settings->setValue("Password", cscuSysConfig.password);
            settings->setValue("LocalChargePassword", cscuSysConfig.localChargePassword);

            settings->setValue("CCUNum", cscuSysConfig.CCUnum);
			settings->setValue("DirectCurrent", cscuSysConfig.directCurrent);
			settings->setValue("SinglePhase", cscuSysConfig.singlePhase);
			settings->setValue("ThreePhase", cscuSysConfig.threePhase);
			settings->setValue("Version", QString(cscuSysConfig.version));
			settings->setValue("DNS", QString(cscuSysConfig.dns));
			settings->endGroup();
			break;
		case PARAM_CCU_SYS:
			settings->beginGroup("CCUSys");
			settings->endGroup();
			break;
		case PARAM_POWERLIMIT:
			settings->beginGroup("POWER_LIMIT");
			settings->setValue("PowerLimit_Enable", powerLimitConfig.sPowerLimit_Enable);
			settings->setValue("CCUCount", powerLimitConfig.sCCUcount);
			settings->setValue("StationLimitPower", powerLimitConfig.STATION_LIMT_POWER);
			settings->setValue("StationSafePower", powerLimitConfig.SAFE_CHARGE_POWER);
			settings->setValue("SumPower_Manual", powerLimitConfig.sSUMPower_Manual);
			settings->setValue("SumPower_Server", powerLimitConfig.sSUMPower_Server);
			settings->setValue("SumPower_Ammeter_Enable", powerLimitConfig.sSUMPower_Ammeter_Enable);
			settings->setValue("SumPower_Manual_Enable", powerLimitConfig.sSUMPower_Manual_Enable);
			settings->setValue("SumPower_Server_Enable", powerLimitConfig.sSUMPower_Server_Enable);
			settings->endGroup();
			break;
		case PARAM_SMARTCHARGE: 
			settings->beginGroup("SMART_CHARGE");
			settings->setValue("SmartCharge_Enable", smartChargeConfig.sSmartCharge_Enable);
			settings->endGroup();
			break;
		case PARAM_SCREEN:
			settings->beginGroup("SCREEN");
			settings->setValue("Version", screenConfig.version);
			settings->setValue("Password", screenConfig.screenPwd);
			settings->endGroup();
			break;
		case PARAM_IO:
			{
				char nameStr[20];
				settings->beginGroup("IOIN");
				for(int i=0; i< 10; i++)
				{
					snprintf(nameStr, sizeof(nameStr), "DIN%d", i+1);
					settings->setValue(QString(nameStr), ioConfig.inOpenColse[i]);
				}
				settings->endGroup();
			}
			break;
    	case PARAM_MAGNETIC:
            settings->beginGroup("MAGNETICSWITCH");
            settings->setValue("bOpenDoorPowerOutages", magneticConfig.bOpenDoorPowerOutages);
            settings->endGroup();
            break;
		case PARAM_EXTRA:
            settings->beginGroup("CSCU_EXTRA");
            settings->setValue("CoupleGun300KW", extraConfig.coupleGun300KW);
            settings->setValue("LocalAddress", extraConfig.localAddress);
            settings->endGroup();
            break;
		case PARAM_SMARTCAR:
			settings->beginGroup("SMART_CAR");
			settings->setValue("SmartCar_Enable", smartCarConfig.sSmartCar_Enable);
			settings->setValue("CCUtotalnum", smartCarConfig.CCUtotalnum);
			settings->setValue("GunNum1", smartCarConfig.gunnum[0]);
			settings->setValue("ModuleNum1", smartCarConfig.ModuleNum[0]);
			settings->setValue("GunNum2", smartCarConfig.gunnum[1]);
			settings->setValue("ModuleNum2", smartCarConfig.ModuleNum[1]);
			settings->setValue("GunNum3", smartCarConfig.gunnum[2]);
			settings->setValue("ModuleNum3", smartCarConfig.ModuleNum[2]);
			settings->setValue("GunNum4", smartCarConfig.gunnum[3]);
			settings->setValue("ModuleNum4", smartCarConfig.ModuleNum[3]);
			settings->endGroup();
			break;

		case PARAM_EMERGENCY:
			settings->beginGroup("EMERGENCY_CHARGE");
			settings->setValue("emergency_enable", emergencyConfig.emergency_enable);
			settings->setValue("vin_authenticate", emergencyConfig.vin_authenticate);
			settings->setValue("car_authenticate", emergencyConfig.car_authenticate);
			settings->setValue("card_authenticate", emergencyConfig.card_authenticate);
			settings->setValue("queue_gun", emergencyConfig.queue_gun);
			settings->setValue("queue_card", emergencyConfig.queue_card);
			settings->setValue("queue_car", emergencyConfig.queue_car);
			settings->setValue("check_time", emergencyConfig.check_time);
			settings->setValue("duration", emergencyConfig.duration);
			settings->setValue("order_count", emergencyConfig.order_count);
			settings->endGroup();
			break;
		case PARAM_WEBSERVER:
			settings->beginGroup("WEBSERVER");
			settings->setValue("Url", webserverConfig.url);
			settings->setValue("Port", webserverConfig.port);
			settings->setValue("OperatorID", webserverConfig.operator_id);
			settings->setValue("OperatorSecret", webserverConfig.operator_secret);
			settings->setValue("DataSecret", webserverConfig.data_secret);
			settings->setValue("AesKey", webserverConfig.aes_key);
			settings->setValue("AesIV", webserverConfig.aes_iv);
			settings->endGroup();
			break;
		default: 
			break;
	}

	delete settings;

	if(PARAM_NET0 == type || dnsChange == true)
	{
		dnsChange = false;
		if(configInited)
			system("/mnt/nandflash/bin/net_config.sh eth0 &");
	}

	if( terminalNumChange == true)	//终端数量发生了变化
	{
		terminalNumChange = false;
		if(configInited)
			system("echo \"del real_record.db\" > /mnt/nandflash/etc/database.conf &");
	}

	if(type < PARM_COUNT && configInited)
	{
		InfoMap map;
		map.insert(Addr_Param_Change, QByteArray((char *)&type, sizeof(type)));
		emit sigToBus(map, AddrType_ParamChange);		//配置变化 
	}	
}
/**
 *查询电表配置
 */
void ParamSet::queryAmmeterConfig(stAllAmmeterConfig *config)
{
	struct db_result_st result;
	QList<stAmmeterConfig> *ammeterCfg = &config->ammeterConfig;

    const char *cmd_sql = "select addr,dev_type,vol_ratio,cur_ratio,fun_type,enable from ammeter_param_table where enable=\"1\"";
	if(0 == db->DBSqlQuery((char *)cmd_sql, &result, DB_PARAM))
	{
        stAmmeterConfig config;
        if(result.column == 6)
		{
			for(int i=0; i<result.row ;i++)
			{

				CharToBCD((unsigned char *)result.result[i*result.column], strlen(result.result[i*result.column]), config.addr);
				config.devType = atoi(result.result[i*result.column+1]);
				config.voltageRatio = atoi(result.result[i*result.column+2]);
				config.currentRatio = atoi(result.result[i*result.column+3]);
                config.funType = atoi(result.result[i*result.column+4]);
                config.enable = atoi(result.result[i*result.column+5]);

				ammeterCfg->append(config);
			}
		}
		db->DBQueryFree(&result);
	}
}

/**
 *更新、添加电表配置
 */
void ParamSet::updateAmmeterConfig(stAllAmmeterConfig &ammeterConfig)
{
	char cmd_sql[256];
	char tmp_buff[15];
	//const char *del_sql = "delete from ammeter_param_table where ";
	
	QList<stAmmeterConfig> *cfgList = &ammeterConfig.ammeterConfig;

	//db->DBSqlExec((char *)del_sql, DB_PARAM);
	for(int i=0; i<cfgList->size(); i++)
	{
		stAmmeterConfig ammeter = cfgList->at(i);

		BCDToChar((unsigned char *)ammeter.addr, 6, (unsigned char *)tmp_buff);
		tmp_buff[12] = 0;
	
		snprintf(cmd_sql, sizeof(cmd_sql), "delete from ammeter_param_table where addr=\"%s\"", tmp_buff);
		db->DBSqlExec((char *)cmd_sql, DB_PARAM);

        snprintf(cmd_sql, sizeof(cmd_sql), "insert into ammeter_param_table (addr, dev_type, vol_ratio, cur_ratio, enable,fun_type) values(\"%s\",%d,%d,%d,%d,%d)", tmp_buff, ammeter.devType, ammeter.voltageRatio,ammeter.currentRatio, ammeter.enable, ammeter.funType);

		db->DBSqlExec((char *)cmd_sql, DB_PARAM);
	}
}

/**
 * 三相相别类型查询
 */
void ParamSet::queryPhaseTypeConfig(stThreePhaseTypeConfig *phaseConfig)
{
	struct db_result_st result;
	QList<stPhaseTypeConfig> *phaseCfg = &phaseConfig->phaseTypeConfig;

	const char *cmd_sql = "select canaddr, phase_type from phasetype_param_table";

	if(0 == db->DBSqlQuery((char *)cmd_sql, &result, DB_PARAM))
	{
		stPhaseTypeConfig config;

		if(result.column == 2)
		{
			for(int i=0; i<result.row ;i++)
			{
				config.canaddr = atoi(result.result[i*result.column]);
				config.type = atoi(result.result[i*result.column+1]);
				phaseCfg->append(config);
			}
		}

		db->DBQueryFree(&result);
	}
}
/**
 *三相相别类型设置
 */
void ParamSet::updatePhaseTypeConfig(stThreePhaseTypeConfig &config)
{
	char cmd_sql[256];
	const char *del_sql = "delete from phasetype_param_table";

	QList<stPhaseTypeConfig> *cfgList = &config.phaseTypeConfig;

	db->DBSqlExec((char *)del_sql, DB_PARAM);
	for(int i=0; i<cfgList->size(); i++)
	{
		stPhaseTypeConfig cfg = cfgList->at(i);

		snprintf(cmd_sql, sizeof(cmd_sql), "insert into phasetype_param_table (canaddr, phase_type) values(%d,%d)", cfg.canaddr,cfg.type);

		db->DBSqlExec((char *)cmd_sql, DB_PARAM);
	}
}


/**
 *尖峰平谷参数查询
 */
void ParamSet::queryTPFVConfig(stAllTPFVConfig *config)
{
	struct db_result_st result;
	QList<stTPFVConfig> *tpfvCfg = &config->tpfvConfig;

	const char *cmd_sql = "select time_seg, start_hour, start_minute, stop_hour, stop_minute, limit_soc, limit_current from tpfv_param_table";

	if(0 == db->DBSqlQuery((char *)cmd_sql, &result, DB_PARAM))
	{
		stTPFVConfig cfg;

		if(result.column == 7)
		{
			tpfvCfg->clear();
			for(int i=0; i<result.row ;i++)
			{
				cfg.time_seg = atoi(result.result[i*result.column]);
				cfg.start_hour = atoi(result.result[i*result.column+1]);
				cfg.start_minute = atoi(result.result[i*result.column+2]);
				cfg.stop_hour = atoi(result.result[i*result.column+3]);
				cfg.stop_minute = atoi(result.result[i*result.column+4]);
				cfg.limit_soc = atoi(result.result[i*result.column+5]);
				cfg.limit_current = atoi(result.result[i*result.column+6]);

				tpfvCfg->append(cfg);
			}
		}

		db->DBQueryFree(&result);
	}
}
/**
 *尖峰平谷参数设置
 */
void ParamSet::updateTPFVConfig(stAllTPFVConfig &config)
{
	char cmd_sql[256];
	const char *del_sql = "delete from tpfv_param_table";

	QList<stTPFVConfig> *cfgList = &config.tpfvConfig;

	db->DBSqlExec((char *)del_sql, DB_PARAM);
	for(int i=0; i<cfgList->size(); i++)
	{
		stTPFVConfig cfg = cfgList->at(i);

		snprintf(cmd_sql, sizeof(cmd_sql), "insert into tpfv_param_table (time_seg, start_hour, start_minute, stop_hour, stop_minute, limit_soc, limit_current) values(%d,%d,%d,%d,%d,%d,%d)", cfg.time_seg, cfg.start_hour, cfg.start_minute, cfg.stop_hour, cfg.stop_minute, cfg.limit_soc, cfg.limit_current);

		db->DBSqlExec((char *)cmd_sql, DB_PARAM);
	}
}

bool ParamSet::queryLocalPolicyConfig(AllLocalPolicyConfig *conf)
{
	struct db_result_st result;
	LocalPolicyConfig cfg;
	QString strSql;

	QList<LocalPolicyConfig> *policyCfg = &conf->policyConfig;

	strSql = "SELECT policy_index, start_hour, start_minute, stop_hour, stop_minute, \
			  electric_fee, service_fee FROM local_policy_table ORDER BY policy_index ASC";

	if(db->DBSqlQuery(strSql.toAscii().data(), &result, DB_PARAM) != 0)
		return false;

	int iIndex = 0;
	policyCfg->clear();
	for(int i=0; i<result.row ;i++)
	{
		iIndex = i * result.column;
		cfg.policy_index = atoi(result.result[iIndex++]); 
		cfg.start_hour = atoi(result.result[iIndex++]);
		cfg.start_minute = atoi(result.result[iIndex++]);
		cfg.stop_hour = atoi(result.result[iIndex++]);
		cfg.stop_minute = atoi(result.result[iIndex++]);
		cfg.electric_fee = atoi(result.result[iIndex++]);
		cfg.service_fee = atoi(result.result[iIndex++]);

		policyCfg->append(cfg);
	}

	db->DBQueryFree(&result);
	return true;
}

bool ParamSet::updateLocalPolicyConfig(AllLocalPolicyConfig *conf)
{
	QString strSql;
	QList<LocalPolicyConfig> *cfgList = &conf->policyConfig;

	//if(db->DBSqlExec((char *)"BEGIN", DB_PARAM) != 0)
	//	return false;

	strSql = "DELETE FROM local_policy_table";
	if(db->DBSqlExec(strSql.toAscii().data(), DB_PARAM) != 0)
		return false;

	for(int i = 0; i < cfgList->size(); i++)
	{
		LocalPolicyConfig cfg = cfgList->at(i);
		strSql.sprintf("INSERT INTO local_policy_table (policy_index, start_hour, start_minute, \
				stop_hour, stop_minute, electric_fee, service_fee) VALUES (%d, %d, %d, %d, %d, %d, %d)", 
				cfg.policy_index, cfg.start_hour, cfg.start_minute, cfg.stop_hour, 
				cfg.stop_minute, cfg.electric_fee, cfg.service_fee);
		if(db->DBSqlExec(strSql.toAscii().data(), DB_PARAM) != 0){
			//db->DBSqlExec((char *)"ROLLBACK", DB_PARAM);
			return false;
		}
	}

	//if(db->DBSqlExec((char *)"COMMIT", DB_PARAM) != 0)
	//	return false;

	return true;
}

bool ParamSet::queryFGPJConfig(AllFGPJConfig *conf)
{
	struct db_result_st result;
	FGPJConfig cfg;
	QString strSql;

	QList<FGPJConfig> *fgpjCfg = &conf->fgpjConfig;

	strSql = "SELECT time_seg, start_hour, start_minute, stop_hour, \
			  stop_minute FROM fgpj_param_table ORDER BY time_seg ASC;";

	if(db->DBSqlQuery(strSql.toAscii().data(), &result, DB_PARAM) != 0)
		return false;

	int iIndex = 0;
	fgpjCfg->clear();
	for(int i=0; i<result.row ;i++)
	{
		iIndex = i * result.column;
		cfg.time_seg = atoi(result.result[iIndex++]);
		cfg.start_hour = atoi(result.result[iIndex++]);
		cfg.start_minute = atoi(result.result[iIndex++]);
		cfg.stop_hour = atoi(result.result[iIndex++]);
		cfg.stop_minute = atoi(result.result[iIndex++]);

		fgpjCfg->append(cfg);
	}

	db->DBQueryFree(&result);
	return true;
}

bool ParamSet::updateFGPJConfig(AllFGPJConfig *conf)
{
	QString strSql;
	QList<FGPJConfig> *cfgList = &conf->fgpjConfig;

	//if(db->DBSqlExec((char *)"BEGIN", DB_PARAM) != 0)
	//	return false;

	strSql = "DELETE FROM fgpj_param_table";
	if(db->DBSqlExec(strSql.toAscii().data(), DB_PARAM) != 0)
		return false;

	for(int i = 0; i < cfgList->size(); i++)
	{
		FGPJConfig cfg = cfgList->at(i);
		strSql.sprintf("INSERT INTO fgpj_param_table (time_seg, start_hour, start_minute, \
				stop_hour, stop_minute) VALUES (%d, %d, %d, %d, %d)", 
				cfg.time_seg, cfg.start_hour, cfg.start_minute, cfg.stop_hour, cfg.stop_minute);
		if(db->DBSqlExec(strSql.toAscii().data(), DB_PARAM) != 0){
			//db->DBSqlExec((char *)"ROLLBACK", DB_PARAM);
			return false;
		}
	}

	//if(db->DBSqlExec((char *)"COMMIT", DB_PARAM) != 0)
	//	return false;

	return true;
}
/**
 *查询配置
 */
bool ParamSet::querySetting(void *data, int type)
{
	unParamConfig *paramConfig = (unParamConfig *)data;
	bool ret = true;
	switch(type)
	{
		case PARAM_CAN0:
			paramConfig->can0Config = can0Config;
			break;
		case PARAM_CAN1:
			paramConfig->can1Config = can1Config; 
			break;
		case PARAM_NET0:
			paramConfig->net0Config = net0Config;
			break;
		case PARAM_NET1:
			paramConfig->net1Config = net1Config; 
			break;
		case PARAM_SERVER0:
			paramConfig->server0Config = server0Config; 
			break;
		case PARAM_SERVER1:
			paramConfig->server1Config = server1Config; 
			break;
		case PARAM_SERVERLIST:
			paramConfig->serverListConfig = serverListConfig; 
			break;
        case PARAM_SERVER_DM:
            paramConfig->serverDMConfig = serverDMConfig;
            break;
		case PARAM_CHARGE:
			paramConfig->chargeConfig = chargeConfig; 
			break;
		case PARAM_CSCU_SYS:
			paramConfig->cscuSysConfig = cscuSysConfig; 
			break;
		case PARAM_CCU_SYS:
			paramConfig->ccuSysConfig = ccuSysConfig; 
			break;
		case PARAM_SCREEN:
			paramConfig->screenConfig = screenConfig; 
			break;
		case PARAM_EMERGENCY:
			paramConfig->emergencyConfig = emergencyConfig; 
			break;
		case PARAM_WEBSERVER:
			paramConfig->webserverConfig = webserverConfig; 
			break;
		case PARAM_IO:
			paramConfig->ioConfig = ioConfig; 
			break;
        case PARAM_MAGNETIC:
            paramConfig->magneticConfig = magneticConfig;
            break;
        case PARAM_EXTRA:
            paramConfig->extraConfig = extraConfig;
            break;
		case PARAM_POWERLIMIT:
			paramConfig->powerLimitConfig = powerLimitConfig;
			break;
		case PARAM_SMARTCHARGE:
			paramConfig->smartChargeConfig = smartChargeConfig;
			break;
		case PARAM_AMMETER:
			queryAmmeterConfig((stAllAmmeterConfig *)data);
			break;
		case PARAM_PHASE_TYPE:
			queryPhaseTypeConfig((stThreePhaseTypeConfig *)data);
			break;
		case PARAM_TPFV:
			queryTPFVConfig((stAllTPFVConfig *)data);
			break;

    case PARAM_SMARTCAR:
        paramConfig->smartCarConfig = smartCarConfig;
        break;

		case PARAM_LOCALPOLICY:
			ret = queryLocalPolicyConfig((AllLocalPolicyConfig *)data);
			break;
		case PARAM_FGPJ:
			ret = queryFGPJConfig((AllFGPJConfig *)data);
			break;

		default: 
			ret = false;
			break;
	}
	return ret;
}

/**
 * 更新配置的槽函数
 */
void ParamSet::slotUpdateSetting(QVariant var, int type)
{
	unParamConfig paramConfig = var.value<unParamConfig>();
	switch(type)
	{
		case PARAM_CAN0:
			can0Config = paramConfig.can0Config; 
			break;
		case PARAM_CAN1:
			can1Config = paramConfig.can1Config; 
			break;
		case PARAM_NET0:
			{
                if( (strcmp(net0Config.localIp , paramConfig.net0Config.localIp) != 0) ||
                        (strcmp(net0Config.netMask , paramConfig.net0Config.netMask) != 0) ||
                        (strcmp(net0Config.gateway , paramConfig.net0Config.gateway) != 0)
                    )
                    rebootFlag = 2;

				net0Config = paramConfig.net0Config; 
			}
			break;
		case PARAM_NET1:
			net1Config = paramConfig.net1Config; 
			break;
        case PARAM_SERVER0:
        {
            if(strcmp(server0Config.stationNo, paramConfig.server0Config.stationNo) != 0 ||
               strcmp(server0Config.serverIp1, paramConfig.server0Config.serverIp1) != 0 ||
               server0Config.serverPort1 != paramConfig.server0Config.serverPort1 ||
               strcmp(server0Config.serverIp2, paramConfig.server0Config.serverIp2) != 0 ||
               server0Config.serverPort2 != paramConfig.server0Config.serverPort2 ||
               strcmp(server0Config.serverIp3, paramConfig.server0Config.serverIp3) != 0 ||
               server0Config.serverPort3 != paramConfig.server0Config.serverPort3)
            {
                rebootFlag = 2;
            }
             server0Config = paramConfig.server0Config;
             break;
        }
		case PARAM_SERVER1:
			server1Config = paramConfig.server1Config; 
			break;
		case PARAM_SERVERLIST: 
			serverListConfig = paramConfig.serverListConfig;
			break;
        case PARAM_SERVER_DM:
            serverDMConfig = paramConfig.serverDMConfig;
            break;
        case PARAM_CHARGE: //changed by XX 2017-05-16, 2017-06-14
            {
                if( (chargeConfig.cardAuto != paramConfig.chargeConfig.cardAuto)
                        ||(chargeConfig.cardType != paramConfig.chargeConfig.cardType)
                        ||(chargeConfig.energyFilter != paramConfig.chargeConfig.energyFilter)
                        ||(chargeConfig.fgpjEnable != paramConfig.chargeConfig.fgpjEnable)
                        ||(chargeConfig.localPolicy != paramConfig.chargeConfig.localPolicy)
                        ||(chargeConfig.localStop != paramConfig.chargeConfig.localStop)
//                        ||(chargeConfig.localChargeEnable != paramConfig.chargeConfig.localChargeEnable)
//                        ||(chargeConfig.localChargeType != paramConfig.chargeConfig.localChargeType)
                        ||(chargeConfig.vinAuto != paramConfig.chargeConfig.vinAuto)
                        ||(chargeConfig.vinOffline != paramConfig.chargeConfig.vinOffline)
                        ||(chargeConfig.vinType != paramConfig.chargeConfig.vinType)
                        ||(chargeConfig.ucDevType != paramConfig.chargeConfig.ucDevType)
                        ||(chargeConfig.languageType != paramConfig.chargeConfig.languageType))
                {
                    rebootFlag = 2;
                }

                chargeConfig = paramConfig.chargeConfig;

                break;
            }
        case PARAM_CSCU_SYS:
			{
				if( cscuSysConfig.directCurrent != paramConfig.cscuSysConfig.directCurrent ||
						cscuSysConfig.singlePhase != paramConfig.cscuSysConfig.singlePhase ||
                        cscuSysConfig.threePhase != paramConfig.cscuSysConfig.threePhase   ||
                        strcmp(cscuSysConfig.dns, paramConfig.cscuSysConfig.dns) !=0)
				{
                    rebootFlag = 2;
					terminalNumChange = true;
				}

				if( strcmp(cscuSysConfig.dns, paramConfig.cscuSysConfig.dns) != 0 )
				{
					dnsChange = true;
                    rebootFlag = 2;
				}
				
				cscuSysConfig = paramConfig.cscuSysConfig; 
			}
			break;
		case PARAM_CCU_SYS:
			ccuSysConfig = paramConfig.ccuSysConfig; 
			break;
		case PARAM_POWERLIMIT:
			powerLimitConfig = paramConfig.powerLimitConfig; 
			break;
		case PARAM_SMARTCHARGE:
			smartChargeConfig = paramConfig.smartChargeConfig; 
			break;
		case PARAM_SCREEN:
			screenConfig = paramConfig.screenConfig; 
			break;
		case PARAM_EMERGENCY:
			emergencyConfig = paramConfig.emergencyConfig;
			break;
		case PARAM_WEBSERVER:
			webserverConfig = paramConfig.webserverConfig;
			break;
		case PARAM_IO:
			ioConfig = paramConfig.ioConfig; 
			break;
        case PARAM_MAGNETIC:
           	magneticConfig = paramConfig.magneticConfig;
            break;
        case PARAM_EXTRA:
           	extraConfig = paramConfig.extraConfig;
            break;
		case PARAM_AMMETER:
			{
				stAllAmmeterConfig config = var.value<stAllAmmeterConfig>();
				updateAmmeterConfig(config);
			}break;
		case PARAM_PHASE_TYPE:
			{
				stThreePhaseTypeConfig config = var.value<stThreePhaseTypeConfig>();
				updatePhaseTypeConfig(config);
			}break;
		case PARAM_TPFV:
			{
				stAllTPFVConfig config = var.value<stAllTPFVConfig>();
				updateTPFVConfig(config);
			}break;

    case PARAM_SMARTCAR:
        smartCarConfig = paramConfig.smartCarConfig;
        break;

		case PARAM_LOCALPOLICY:
			{
				AllLocalPolicyConfig config = var.value<AllLocalPolicyConfig>();
				updateLocalPolicyConfig(&config);
			}
			break;
		case PARAM_FGPJ:
			{
				AllFGPJConfig config = var.value<AllFGPJConfig>();
				updateFGPJConfig(&config);
			}
			break;


		default: break;
	}

	updateConfig(type);
}


bool ParamSet::loadAmmeterRange()
{
	struct db_result_st result;
	QString strSql;
	uchar cCanAddr;
	double dRange;

	strSql.sprintf("SELECT can_addr, max_range FROM ammeter_range_table;");
	if(db->DBSqlQuery(strSql.toAscii().data(), &result, DB_PARAM) == 0){
		for(int i = 0; i < result.row; i++){
			cCanAddr = atoi(result.result[result.column * i]);
			dRange = atof(result.result[result.column * i + 1]);
			if(dRange <= 0.0)
				dRange = 999999.99;
			m_mapAmmeterRange[cCanAddr] = dRange;
		}
		db->DBQueryFree(&result);
	}

	return false;
}

double ParamSet::getAmmeterRange(uchar cCanAddr)
{
	if(m_mapAmmeterRange.contains(cCanAddr)){
		return m_mapAmmeterRange[cCanAddr];	
	}

	return 999999.99;
}

bool ParamSet::loadAcMeterScale()
{
	struct db_result_st result;
	QString strSql;
	uchar cCanAddr;
	double dRange;

	strSql.sprintf("SELECT can_addr, ac_scale FROM acmeter_scale_table;");
	if(db->DBSqlQuery(strSql.toAscii().data(), &result, DB_PARAM) == 0){
		for(int i = 0; i < result.row; i++){
			cCanAddr = atoi(result.result[result.column * i]);
			dRange = atof(result.result[result.column * i + 1]);
			if(dRange > 0.0)
				m_mapAcMeterScale[cCanAddr] = dRange;
		}
		db->DBQueryFree(&result);
	}

	return false;
}

double ParamSet::getAcMeterScale(uchar cCanAddr)
{
	if(m_mapAcMeterScale.contains(cCanAddr)){
		return m_mapAcMeterScale[cCanAddr];	
	}

	return 0.0;
}

/*
 * 动态库接口函数，初始化模块
 * pThread	输入 模块将要运行的线程
 * 返回值 	0表示无错误，-1表示有错误
 */
int ParamSet::InitModule(QThread* pThread)
{
    return 0;
}

/*
 * 动态库接口函数，向BUS注册模块
 * pBus		输入 BUS模块指针
 * 返回值 	0表示无错误，-1表示有错误
 */
int ParamSet::RegistModule()
{
    return 0;
}

/*
 * 动态库接口函数，启动模块
 * 返回值 	0表示无错误，-1表示有错误
 */
int ParamSet::StartModule()
{
    return 0;
}

/*
 * 动态库接口函数，停止模块
 * 返回值 	0表示无错误，-1表示有错误
 */
int ParamSet::StopModule()
{
    return 0;
}

/*
 * 动态库接口函数，模块工作状态
 * 返回值 	0表示无错误，-1表示有错误
 */
int ParamSet::ModuleStatus()
{
    return 0;
}

ParamSet *ParamSet::GetInstance()
{
	static ParamSet *ins = NULL;
	if(!ins){
		ins = new ParamSet();
	}

	return ins;
}

//通过配置的多枪组更改显示的名称
void ParamSet::InitMultiGunName()
{
    //多枪功能启动 umlti_gun_param_table有内容，根据此内容更新terminal_name_multi_table表
    QString TempName, TempID, DBExec, todo;

    db_result_st dbst;
    int imultitype =0;
    char * pName =NULL;
    int imultinumber =0;
    int icanaddr=0;

    todo = "SELECT * FROM multi_gun_param_table";
    if(db->DBSqlQuery(todo.toAscii().data(),&dbst,DB_PARAM) !=0)
    {
        return;
    }
    for(int i=0;i<dbst.row;i++)
    {
        imultitype = atoi(dbst.result[i*dbst.column +1]);
        pName = dbst.result[i*dbst.column +2];
        imultinumber = atoi(dbst.result[i*dbst.column +3]);
        if(imultitype == GUNTYPE)
        {
            if(imultinumber !=0)
           {
                icanaddr = atoi(dbst.result[i*dbst.column +4]);
                TempID = QString::number(icanaddr ,10);
                DBExec = QString("UPDATE  terminal_name_multi_table SET multitype='%1'  WHERE canaddr = '%2'").arg(1).arg(TempID);
                db->DBSqlExec(DBExec.toAscii().data(),DB_PARAM);
            }
            for(int y=1;y<imultinumber;y++)
            {
                icanaddr = atoi(dbst.result[i*dbst.column +4+y]);
                TempID = QString::number(icanaddr ,10);
                DBExec = QString("DELETE  FROM terminal_name_multi_table WHERE canaddr='%1' ").arg(TempID);
                db->DBSqlExec(DBExec.toAscii().data(),DB_PARAM);
             }

        }else if(imultitype == BOWTYPE)
        {
            if(imultinumber !=0)
            {
                icanaddr = atoi(dbst.result[i*dbst.column +4]);
                TempID = QString::number(icanaddr ,10);
                TempName = pName;
                DBExec = QString("UPDATE  terminal_name_multi_table SET name='%1' ,multitype='%2'  WHERE canaddr = '%3'").arg(TempName).arg(2).arg(TempID);
                db->DBSqlExec(DBExec.toAscii().data(),DB_PARAM);
                for(int y=1;y<imultinumber;y++)
                {
                    icanaddr = atoi(dbst.result[i*dbst.column +4+y]);
                    TempID = QString::number(icanaddr ,10);
                    DBExec = QString("DELETE  FROM terminal_name_multi_table WHERE canaddr='%1' ").arg(TempID);
                    db->DBSqlExec(DBExec.toAscii().data(),DB_PARAM);
                }
             }
        }
    }
}
