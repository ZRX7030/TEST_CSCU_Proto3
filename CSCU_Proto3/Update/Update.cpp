#include <stdio.h>
#include <QRegExp>
#include <QDebug>

#include "Update/Update.h"

Update::Update()
{
    cmdSource = 0;
    cmdTypeMaster = 0;
    cmdTypeSlave = 0;
    canAddr = 0;
    autoTimerCount = 0;

    this->updateTime = 0;

    this->log = Log::GetInstance();
    this->paramSet = ParamSet::GetInstance();
    this->devcache = DevCache::GetInstance();
}

Update::~Update()
{

}

/**
 *开始工作数据的关联
 */
void Update::procStartWork(void)
{
    paramSet->querySetting(&cscuSysConfig, PARAM_CSCU_SYS);
    paramSet->querySetting(&server0Config, PARAM_SERVER0);
    mountCheckTimer.setInterval(5000);
    QObject::connect(&mountCheckTimer, SIGNAL(timeout()), this, SLOT(timeOut()));
    mountCheckTimer.start();

    QObject::connect(this, SIGNAL(sigRunUpdate(QString)), this, SLOT(procRunUpdate(QString)));
}

/**
 *U盘插入检测
 */
void Update::udiskInsertCheck(void)
{
    int status = 0;

    QString usbMount = "usb_mount_check.sh " + QString::number(cscuSysConfig.boardType) + " " + QString::number(1, 10);
    status = system(usbMount.toAscii().data());
    if(-1 != status && WIFEXITED(status) && 0 == WEXITSTATUS(status))
    {
        if(cscuSysConfig.boardType == 3)//模块化集控
        {
            QLibrary ElecLib(QString(LIB_PATH) + QString(ELEC_LOCK_LIB_NAME));
            if(!ElecLib.load())
            {
                qDebug()<<ElecLib.errorString();
                return;
            }
            pFunElecLock pElecLock = (pFunElecLock)ElecLib.resolve("Start");
            if(pElecLock == 0x00)
            {
                qDebug()<<ElecLib.errorString();
            }

            ET_Lock etLock;
            memset(&etLock, 0x00, sizeof(ET_Lock));
            if(pElecLock(&etLock) != 0)
            {
                qDebug("U盘鉴权失败");
                return;
            }
            if(etLock.flag == 0)//鉴权
            {
                qDebug("U盘鉴权, 用户识别不匹配");
                return;
            }
        }
        //检测到u盘插入
        InfoMap infoMap;
        char value = 1;
        infoMap.insert(Addr_Checked_USB, QByteArray(&value,1));
        emit sigToBus(infoMap, AddrType_Udisk_Insert);

        system("[ ! -f /mnt/nandflash/etc/autoupdate.conf ] && echo \"update 1\" > /mnt/nandflash/etc/autoupdate.conf &");
    }
    else
    {
        system("[ -f /mnt/nandflash/etc/autoupdate.conf ] && rm -f /mnt/nandflash/etc/autoupdate.conf &");
    }
}
/**
 *终端是否在充电检测
 */
int Update::termChargingCheck(void)
{
    TerminalStatus status;
    int count = 0;
    unsigned char startCanAddr[3] = {1, 151, 181};
    int canNum[3];

    //立即更新
    if(updateTime == 1)
        return 0;

    //空闲更新
    canNum[0] = cscuSysConfig.singlePhase;
    canNum[1] = cscuSysConfig.threePhase;
    canNum[2] = cscuSysConfig.directCurrent;

    for(int i=0; i< 3; i++)
    {
        for(unsigned char k=startCanAddr[i]; k < (canNum[i] + startCanAddr[i]); k++)
        {
            devcache->QueryTerminalStatus(k, status);
            if(status.cStatus != CHARGE_STATUS_FREE)
                count ++;
        }
    }

    return count;
}

/**
 *请求数据查询
 */
void Update::queryRequestInfo(stUpdateRequestData *requestData)
{
    TerminalStatus status;
    int count = 0;
    unsigned char startCanAddr[4] = {1, 151, 181,240};
    int canNum[4];

    canNum[0] = cscuSysConfig.singlePhase;
    canNum[1] = cscuSysConfig.threePhase;
    canNum[2] = cscuSysConfig.directCurrent;
    canNum[3] = cscuSysConfig.CCUnum;

    memset(requestData->cscuVer, 0, sizeof(requestData->cscuVer));
    memset(requestData->teuiVer, 0, sizeof(requestData->teuiVer));
    memset(requestData->stationAddr, 0, sizeof(requestData->stationAddr));
    memset(requestData->macAddr, 0, sizeof(requestData->macAddr));
    memset(requestData->stationName, 0, sizeof(requestData->stationName));
    /*终端版本信息赋值*/
    for(int i=0; i< 3; i++)
    {
        for(unsigned char k=startCanAddr[i]; k < (canNum[i] + startCanAddr[i]); k++)
        {
            devcache->QueryTerminalStatus(k, status);
            requestData->chargerVer.insert(k, QString((char *)status.psTermianlVer));
        }
    }
    if(canNum[3] > 0)
    {
        for(unsigned char k=startCanAddr[3]; k > (startCanAddr[3] - canNum[3]); k--)
        {
            devcache->QueryTerminalStatus(k, status);
            requestData->chargerVer.insert(k, QString((char *)status.psTermianlVer));
        }
    }

    //读取文件中的teui、mac地址
    QFile file("/tmp/version.info");
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QList<QByteArray> list;
        QByteArray arFile;

        arFile = file.readAll();
        list = arFile.split('\n');
        file.close();

        if(list.count() >= 3)
        {
            QString mac,teui;
            if(cscuSysConfig.boardType == 2)
            {
                mac = list.at(2);
                teui = list.at(1);
            }
            else if(cscuSysConfig.boardType == 3)
            {
                mac = list.at(5);
                mac.remove(":");
                teui = list.at(4);
            }

            snprintf(requestData->teuiVer, sizeof(requestData->teuiVer), "%s", teui.toAscii().data());
            snprintf(requestData->macAddr, sizeof(requestData->macAddr), "%s", mac.toAscii().data());
        }
    }
    snprintf(requestData->cscuVer, sizeof(requestData->cscuVer), "%s", MAIN_VERSION);
    snprintf(requestData->stationAddr, sizeof(requestData->stationAddr), "%s", server0Config.stationNo);
}
/**
 *下载结果处理
 */
void Update::dealDownloadResult(void)
{
    if(termChargingCheck())
        return;
    int checkResult = checkDownloadResult();

    switch(checkResult)
    {
    /*cscu下载成功*/
    case 30:
    {
        system("[ ! -f /mnt/nandflash/etc/autoupdate.conf ] && echo \"update 2\" > /mnt/nandflash/etc/autoupdate.conf &");
        paramSet->setRebootFlag(2);
    }break;
        /*模块程序下载成功*/
    case 40:
    {
        unsigned char ucCanTmp;
        QStringList CanList;
        CanList.clear();
        CanList = CanaddrStr.split("-");

        for(int i = 0;i < CanList.size();i++)
        {
            ucCanTmp = 0;
            canAddr = CanList.at(0).toInt();
            ucCanTmp = CanList.at(i).toInt();
            canUpdateMap.insert(ucCanTmp,QString(""));
        }

        if(canUpdateMap.size() > 0)
        {
            InfoMap infoMap;
            infoMap.insert(Addr_Update_Upload_Param, QString("/mnt/nandflash/download/terminal.bin").toLatin1());
            infoMap.insert(Addr_CanID_Comm, QByteArray((char *)&canAddr, 1));
            emit sigToBus(infoMap, AddrType_UpdatePackDir_Dev);

            system("echo \"41\" > /tmp/download.result &");
        }
    }break;
defalut: break;
    }
}
/**
 *查询升级文件
 */
bool Update::queryCanUpdateFile(unsigned char &canAddr, QString & fileName)
{
    bool ret = false;
    QMap<unsigned char,QString>::iterator it;
    for( it = canUpdateMap.begin(); it != canUpdateMap.end(); ++it)
    {
        canAddr = it.key();
        fileName = it.value();
        ret = true;
        break;
    }
    return ret;
}
/**
 *解析版本信息里面的字段
 */
QString Update::resolveVersion(void)
{
    QString retStr = "";

    QString pattern("(.*)_(.*)_(.*)_(.*)");
    QRegExp rx(pattern);

    QString str(MAIN_VERSION);
    int pos = str.indexOf(rx);
    if(pos >= 0)
        retStr = rx.cap(2) + rx.cap(4);
    return retStr;
}
/**
 *自动升级请求判断
 */
void Update::autoUpdateRequest(void)
{
    int ret, valueLen;
    char cmdBuff[256];
    char * value;
    struct receive_buff_st revBuff;
    stUpdateRequestData requestData;

    revBuff.rec_buff = new char[1500];
    revBuff.buff_size = 1500;
    revBuff.rec_size = 0;
    if(revBuff.rec_buff == NULL)
        return;

    /*判断是否发送请求*/
    if(0 != checkDownloadResult())
        return;

    queryRequestInfo(&requestData);

    /*请求包数据组织*/
    struct json_object *object = jsonPackUpdateRequest(requestData);
    value = getJsonPoint(object);
    valueLen = strlen(value);

    /*发送http请求*/
    snprintf(cmdBuff, sizeof(cmdBuff), "http://D-BJ-3rdCOM.chinacloudapp.cn:1195/roam/update_detect");
    ret = sendHttpRequest(cmdBuff, value, valueLen, (void *)"upload", &revBuff);
    if(ret == 1 && revBuff.rec_size)
    {
        /*返回结果解析*/
        revBuff.rec_buff[revBuff.rec_size] = 0;
        stUpdateRequestResultData parseResult;
//        memset(&parseResult, 0, sizeof(stUpdateRequestResultData));
        //int type = jsonCmdTypeParse(revBuff.rec_buff);
        //if(type == 1)
        if(jsonRequestResultParse(revBuff.rec_buff, &parseResult))			//解析请求结果成功
        {
            cmdSource = 3;
            cmdTypeMaster = 1;
            if(parseResult.result == 1 && parseResult.curUpdateType == 1)		//cscu升级
            {
                cmdTypeSlave = 3;
                char cmdBuff[512];

                updateTime = parseResult.updateTime;
                //ftp://ip:port/roam....|md5
                QString paramStr = QString(parseResult.url) + QString("\\\|") + QString(parseResult.md5);
                snprintf(cmdBuff, sizeof(cmdBuff),"update.sh %d %d %d %s &",
                         cmdSource, cmdTypeMaster, cmdTypeSlave, paramStr.toLatin1().data());
                system(cmdBuff);

            }
            else if(parseResult.result == 1 && parseResult.curUpdateType == 2)	//CCU模块升级
            {
                CanaddrStr.clear();
                cmdTypeSlave = 4;
                CanaddrStr = QString(parseResult.CanStr);
                QString fileName;

                QString paramStr = QString(parseResult.url);
                snprintf(cmdBuff, sizeof(cmdBuff),"update.sh %d %d %d %s &",
                         cmdSource, cmdTypeMaster, cmdTypeSlave, paramStr.toLatin1().data());
                system(cmdBuff);

            }
        }
    }
    freeJsonPoint(object);
    delete []revBuff.rec_buff;
}
/**
 *定时检测U盘是否插入
 */
void Update::timeOut(void)
{
    /*u盘插入检测*/
    udiskInsertCheck();

    /*下载结果判断, 判读是否有升级包*/
    dealDownloadResult();

    /*自动升级请求 15min*/
    autoTimerCount++;
    if(autoTimerCount > 180)
    {
        autoTimerCount = 0;
        autoUpdateRequest();
    }
}
/**
 *文件内容读取
 */
int Update::checkDownloadResult(void)
{
    QFile file("/tmp/download.result");
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
		QByteArray line = file.readLine();
		QString str(line);
		file.close();
		return str.toInt();
	}

	return -1;
}
/**
 *执行升级脚本，检测升级结果
 */
void Update::procRunUpdate(QString cmd)
{
    InfoMap infoMap;
    unsigned char value;

    int status = system(cmd.toAscii().data());

    value = 1;
    infoMap.insert(Addr_Cmd_Source, QByteArray((char *)&cmdSource, 1));
    infoMap.insert(Addr_Cmd_Master, QByteArray((char *)&cmdTypeMaster, 1));
    infoMap.insert(Addr_Cmd_Slave, QByteArray((char *)&cmdTypeSlave, 1));

    if(-1 != status && WIFEXITED(status) && 0 == WEXITSTATUS(status))
    {
        //升级成功
        value = 1;
        infoMap.insert(Addr_Back_Result, QByteArray((char *)&value,1));

        if(cmdTypeMaster == 1 && cmdTypeSlave == 4 && cmdSource == 1)  //模块teld平台升级
        {
            infoMap.insert(Addr_Update_Upload_Param, QString("/mnt/nandflash/download/terminal.bin").toLatin1());
            infoMap.insert(Addr_CanID_Comm, QByteArray((char *)&canAddr, 1));
        }
        else if((cmdTypeMaster == 1 && cmdTypeSlave == 3 && cmdSource == 2))	//u盘升级cscu
            paramSet->setRebootFlag(2);
    }
    else
    {
        //失败
        value = 2;
        infoMap.insert(Addr_Back_Result, QByteArray((char *)&value,1));
    }

    //向总线发送执行结果
    if(cmdTypeMaster == 1 && cmdTypeSlave == 4)
        emit sigToBus(infoMap, AddrType_UpdatePackDir_Dev);
    else if(cmdSource != 3)  //网络自动升级
        emit sigToBus(infoMap, AddrType_UpdateResult);
}

/**
 *收到其它模块发送的升级指令
 */
void Update::slotFromBus(InfoMap Map, InfoAddrType enAddrType)
{
    if(enAddrType == AddrType_UpdateResult_Dev)
    {
        if(Map.contains(Addr_CANUpdateResult))		//can的升级结果
        {
            InfoMap infoMap;
            infoMap.insert(Addr_Cmd_Source, QByteArray((char *)&cmdSource, 1));
            infoMap.insert(Addr_Cmd_Master, QByteArray((char *)&cmdTypeMaster, 1));
            infoMap.insert(Addr_Cmd_Slave, QByteArray((char *)&cmdTypeSlave, 1));
            infoMap.insert(Addr_CanID_Comm, QByteArray((char *)&canAddr, 1));

            char value = Map[Addr_CANUpdateResult].data()[0];
            if(value == 0xaa)
                value = 1;
            else if(value == 0x0e)
                return;
            else
                value = 0;
            infoMap.insert(Addr_Back_Result, QByteArray((char *)&value, 1));

            emit sigToBus(infoMap, AddrType_UpdateResult);

            /*收到升级结果后升级下一台*/
            QString fileName;
            canUpdateMap.remove(this->canAddr);
            if(queryCanUpdateFile(this->canAddr, fileName))
            {
                InfoMap infoMap;
                infoMap.insert(Addr_Update_Upload_Param, QString("/mnt/nandflash/download/terminal.bin").toLatin1());
                infoMap.insert(Addr_CanID_Comm, QByteArray((char *)&canAddr, 1));
                emit sigToBus(infoMap, AddrType_UpdatePackDir_Dev);
            }
            else
            {
                /*所有can设备升级完成*/
                system("rm -f /mnt/nandflash/download/terminal.bin &");
                system("echo \"0\" > /tmp/download.result &");
            }
        }
    }
    else if(enAddrType == AddrType_ExecUpdate)			//执行升级、日志导出指令
    {
        char cmdParam[256];
        char cmdBuff[512];

        memset(cmdParam, 0, sizeof(cmdParam));

        cmdSource = 0;
        cmdTypeMaster = 0;
        cmdTypeSlave = 0;

        QByteArray array;
        array = Map[Addr_Cmd_Source];
        if(array.size())
            cmdSource = array.data()[0];
        else
            return;

        array = Map[Addr_Cmd_Master];
        if(array.size())
            cmdTypeMaster = array.data()[0];
        else
            return;

        array = Map[Addr_Cmd_Slave];
        if(array.size())
            cmdTypeSlave = array.data()[0];

        array = Map[Addr_Update_Upload_Param];
        if(array.size())
            memcpy(cmdParam, array.data(), (unsigned int)array.size() > sizeof(cmdParam)-1?sizeof(cmdParam)-1:array.size());

        if(Map.contains(Addr_CanID_Comm))
            this->canAddr = Map[Addr_CanID_Comm].data()[0];

        QString paramStr = QString(cmdParam);
        paramStr.replace(QRegExp("[|]"), "\\\|");

        snprintf(cmdBuff, sizeof(cmdBuff),"update.sh %d %d %d %s",
                 cmdSource, cmdTypeMaster, cmdTypeSlave, paramStr.toLatin1().data());

        /*u盘升级自动重启*/
        if(cmdSource == 2 && cmdTypeMaster == 1)
        {
            paramSet->setRebootFlag(2);
        }
        else
            emit sigRunUpdate(QString(cmdBuff));
    }
}

int Update::InitModule(QThread* pThread)
{
    m_pWorkThread = pThread;

    this->moveToThread(m_pWorkThread);
    QObject::connect(m_pWorkThread, SIGNAL(started()), this, SLOT(procStartWork()));

    return 0;
}
int Update::RegistModule()
{
	QList<int> List;

	List.append(AddrType_ExecUpdate);
	List.append( AddrType_UpdateResult_Dev);
	CBus::GetInstance()->RegistDev(this, List);

    return 0;
}

int Update::StartModule()
{
    m_pWorkThread->start();
    return 0;
}

int Update::StopModule()
{
    m_pWorkThread->exit();
    return 0;
}

int Update::ModuleStatus()
{
    if(m_pWorkThread->isRunning())
        return 0;
    else
        return 1;
}

//SO库调用实现函数, 创建新实例返回
CModuleIO* CreateDevInstance()
{
    return new Update();
}

//实例销毁
void DestroyDevInstance(CModuleIO* pModule)
{
    if(pModule){
        delete pModule;
    }
}


