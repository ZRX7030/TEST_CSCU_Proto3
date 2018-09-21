#include "LCDScreen.h"

cLCDScreen::cLCDScreen()
{
	_strLogName = "screen";

    ucNowAddr = 0;
    ucDCSpecAckStep = 0;
    iCardLen = 0;

    pDevCache = DevCache::GetInstance();
    pDBOperate = DBOperate::GetInstance();
    pParamSet = ParamSet::GetInstance();
    pLog =  Log::GetInstance();
}

cLCDScreen::~cLCDScreen()
{
    if(bWorkFlag == TRUE)
    {
        StopModule();
    }
}

void cLCDScreen::acceptConnection()
{
    if((bWorkFlag == FALSE)||(pServer ==NULL))
    {
        return;
    }
    if(pSocket != NULL)
    {
        pSocket->close();
    }
    pSocket = pServer->nextPendingConnection();
    if(pSocket == NULL)
    {
        pServer->disconnect();
        return;
    }
    connect(pSocket, SIGNAL(readyRead()), this, SLOT(recvMessage()));
}

void cLCDScreen::sendMessage(unsigned char * pData, unsigned int uiLen)
{
    if(pSocket == NULL)
    {
        return;
    }
    pSocket->write((char *)pData, uiLen);
}

void cLCDScreen::recvMessage()
{
    QByteArray recvArray = pSocket->readAll();
    QList<QByteArray>FrameList;
    CheckMutiPackFrame(FrameList, recvArray);

    for(unsigned char i = 0 ; i < FrameList.count(); i++)
    {
        ParseFrame(FrameList.at(i));
    }
}

void cLCDScreen::newListen()
{

    stNetConfig net0Config;
    pParamSet->querySetting(&net0Config, PARAM_NET0);
//    if(pServer->listen(QHostAddress(net0Config.localIp), PortNum_LCDScreen))
//    if(pServer->listen(QHostAddress("20.10.0.114") , PortNum_LCDScreen))
    if(pServer->listen(QHostAddress::LocalHost , PortNum_LCDScreen))
    {
    }
    else
    {
    }
}

//根据配置选项初始化
int cLCDScreen::InitModule( QThread* pThread)
{
    m_pWorkThread = pThread;

    this->moveToThread(m_pWorkThread);
    QObject::connect(m_pWorkThread, SIGNAL(started()), this, SLOT(ProcStartWork()));
    return 0;
}

//注册设备到总线
int cLCDScreen::RegistModule()
{
	QList<int> List;

	List.append(AddrType_CmdCtrl_AckResult);    //遥控_充电控制申请ACK
	List.append(AddrType_Udisk_Insert);//检测到U盘插入
	List.append(AddrType_UpdateResult);//升级结果+日志上传结果
	List.append(AddrType_CenterReadCard);//主题二：集中读卡卡号, 刷卡主界面, 刷卡终端详情界面
	List.append(AddrType_ApplyAccountInfoResult_ToScreen);//主题六：充电服务返回账户信息给显示屏
	List.append(AddrType_InApplyStartChargeResult_ToScreen);//主题十：内部申请开始充电结果至显示屏
	List.append(AddrType_OutApplyStartChargeResult_ToScreen);//主题十四：远程申请开始充电结果至显示屏
	List.append(AddrType_InApplyStopChargeResult_ToScreen);//主题十八：内部申请结束充电结果至显示屏
	List.append(AddrType_OutApplyStopChargeResult_ToScreen);//主题二十二：远程申请结束充电结果至显示屏
	List.append(AddrType_SmartChargeSet_Result);//本地设置错峰充电参数设置结果
	List.append(AddrType_GeneralDynamicArgRenewAck_DB); //通用动态参数更新数据库回复
	List.append(AddrType_GeneralStaticArgRenewAck_DB); //通用静态参数更新数据库回复
	List.append(AddrType_InVinApplyStartCharge_Result);   //VIN申请开始充电结果 add by songqb
	List.append(AddrType_Change_ChargeGunGroup_Info);   //多枪分组信息改变，屏幕分组信息调整   add by songqb
	List.append(AddrType_ActiveProtectQueryResult);
	List.append(AddrType_GeneralStaticArgQueryResult);
	List.append(AddrType_GeneralDynamicArgQueryResult);
	List.append(AddrType_CCUQueryResult);
	//        //nihai add nihai getname
	//        List.append(AddrType_TermIndex_QueryFinish);//结束始获取子站名称，CAN地址和终端编号对应关系。服务器发布，显示屏订阅
	//        //nihai end
	List.append(AddrType_MakePrintTicketResult);   //执行打印小票结果, 信息体：CAN地址 ,打印结果 发出方：小票机, 订阅方：显示屏
	List.append(AddrType_ApplyPrintTicket);

	CBus::GetInstance()->RegistDev(this, List);

	return 0;
}

int cLCDScreen::StartModule()
{
    m_pWorkThread->start();

    return 0;
}

//停止模块
int cLCDScreen::StopModule()
{
    if(bWorkFlag == TRUE)
    {
        bWorkFlag = FALSE;
        pServer->close();
        pSocket = NULL;
        pTimer->stop();
        delete pServer;
        delete pTimer;
        delete pProtocol;
        uiTimeCounter = 0;
        ucCardStep = Step_Free_LCD;
    }
    return 0;
}

//模块工作状态
int cLCDScreen::ModuleStatus()
{
    return 0;
}

//校验和
bool cLCDScreen::CheckFrame( const QByteArray &frameArray)
{
    unsigned char ucSum = 0;
    unsigned short usLen = frameArray.length();
    if(usLen < MinPakLen_LCDScreen)
    {
        return FALSE;
    }
    if( (frameArray.at(0) != 0x68) || (frameArray.at(2) != 0x68) || (frameArray.at(usLen -1) != 0x16) )
    {
        return FALSE;
    }
    for(unsigned short i = 3; i < usLen - 2; i++)
    {
        ucSum += frameArray.at(i);
    }
    if(ucSum != frameArray.at(usLen - 2))
    {
        return FALSE;
    }
    return TRUE;
}

//帧多包校验
bool cLCDScreen::CheckMutiPackFrame(QList <QByteArray> &FrameList, QByteArray &recvArray)
{

    unsigned short usPackLen = 0;
    unsigned int uiDealLen = 0;
    unsigned int uiTotalLen = recvArray.length();
    QByteArray sigArray;

    while(uiDealLen < uiTotalLen)
    {
        if(recvArray.length() <MinPakLen_LCDScreen)
        {
            return FALSE;
        }
        //        usPackLen = recvArray.at(7) + recvArray.at(8)* 0xFF + MinPakLen_LCDScreen;
        memcpy(&usPackLen, recvArray.data() +7, 2);
        usPackLen+= MinPakLen_LCDScreen;
        sigArray = recvArray.left(usPackLen);
        uiDealLen += usPackLen;
        recvArray.remove(0, usPackLen);
        if(!CheckFrame(sigArray))
        {
            return FALSE;
        }
        else
        {
            FrameList.append(sigArray);
        }
    }
    return TRUE;
}

//校验超时时间
void cLCDScreen::CheckTimeOut()
{
    uiTimeCounter++;
//    //nihai add
//    if(uiTimeCounter%10==0)
//    {
//        if(ucgetname==0)
//        {
//            SendTermNameApply();
//        }
//    }
//    //nihai end
    if(bWaitPeakSetFlag == TRUE)
    {
        if(uiPeakSetCounter > TO_PeakSet_LCD)
        {
            sendOperateResult(ucNowAddr, Data_PeakSet_LCD, Ack_Failed_LCD, 0);
            bWaitPeakSetFlag = FALSE;
            uiPeakSetCounter = 0;
        }
        uiPeakSetCounter++;
    }
    switch(ucCardStep)
    {
    case Step_WaitingCardNum_LCD:
        if(uiTimeCounter > TO_CardWaitCardNum_LCD)
        {
            stCardResult.ucResult = 2;
            strncpy(stCardResult.chFaultReason, QByteArray("未读取到卡号").data(), 50);
            ucCardStep = Step_GetAccResult_LCD;
            uiTimeCounter = 0;
        }
        break;
    case Step_WaitingAccResult_LCD:
        if(uiTimeCounter > TO_CardWaitCardNum_LCD)
        {
            stCardResult.ucResult = 2;
            strncpy(stCardResult.chFaultReason, QByteArray("未接收到账户信息").data(), 50);
            ucCardStep = Step_GetAccResult_LCD;
            uiTimeCounter = 0;
        }
        break;
    case Step_WaitingStartChargeResult_LCD:
        if(uiTimeCounter > TO_CardWaitStartResult_LCD)
        {
            stCardResult.ucResult = 2;
            strncpy(stCardResult.chFaultReason, QByteArray("未接收到平台返回开始充电结果").data(), 50);
            ucCardStep = Step_GetStartChargeResult_LCD;
            uiTimeCounter = 0;
        }
        break;
    case Step_WaitingStopChargeResult_LCD:
        if(uiTimeCounter > TO_CardWaitStopResult_LCD)
        {
            stCardResult.ucResult = 2;
            strncpy(stCardResult.chFaultReason, QByteArray("未接收到平台返回结束充电结果").data(), 50);
            ucCardStep = Step_GetStopChargeResult_LCD;
            uiTimeCounter = 0;
        }
        break;
    default:
        break;
    }
}

//解析帧
void cLCDScreen::ParseFrame(const QByteArray &recvArray)
{
//        for(int i = 0 ; i < recvArray.count(); i++)
//        {
//        }

    if((pProtocol == NULL) || (bWorkFlag == FALSE))
    {
        return;
    }

    //帧校验
    if(!CheckFrame(recvArray))
    {
        return;
    }
    //帧解析
    switch(recvArray.at(4)) //指令类型
    {
    case Cmd_Mainten_LCD:
        DealMaintainDataType(recvArray);
        break;
    case Cmd_Query_LCD:
        DealQueryDataType(recvArray);
        break;
    case Cmd_ParaSet_LCD:
        DealSetDataType(recvArray);
        break;
    case Cmd_State_LCD:
        DealStateDataType(recvArray);
        break;
    case Cmd_RealData_LCD:
        DealRealDataType(recvArray);
        break;
    case Cmd_HisData_LCD:
        DealHisDataType(recvArray);
        break;
    case Cmd_InterActive_LCD:
        DealInterActiveType(recvArray);
        break;
    default:
        break;
    }
}

//直流特殊功能设置类数据
void cLCDScreen::ParseDCSpecSet(unsigned char ucAddr, unsigned char ucCmdType, unsigned short usDataType)
{
    ucDCSpecAckStep = 0;
    AddrMem_LCD tempNode;
    tempNode.ucAddr = ucAddr;
    tempNode.uiCmdType = ucCmdType;
    tempNode.uiDataType = usDataType;
    DelayCmdList.append(tempNode);
}

//参数获取
void cLCDScreen::ParseArgQuery(const QByteArray &recvArray)
{
    unsigned char ucType = recvArray.at(9);
    ArgDelNode_LCD node;
    ucQueryArgStep = 0;
    node.ucArgCanID = recvArray.at(10);
    node.iCounter = 0;
    node.iDevType = ucType;
    bWaitTermArgQueryFlag = TRUE;
    if(ucType == 1)//终端参数
    {
        node.iMaxTime = ArgTermCount_LCD;
        applyCenterTermArg(node.ucArgCanID);
    }
    else if(ucType == 2)//CCU参数
    {
        node.iMaxTime = ArgCCUCount_LCD;
        applyCenterCCUArg(node.ucArgCanID);
    }
    ArgDelList.append(node);
}

//解析U盘指令
void cLCDScreen::ParseUdiskCmd(const QByteArray &recvArray)
{
    memset(&stUresult, 0x00, sizeof(stUresult));
    switch(recvArray.at(9))
    {
    case 1://升级
        sendCenterUpdateApply();
        break;
    case 2://导出日志
        sendCenterLogOutApply();
        break;
    case 3://导出数据库
        break;
    default:
        break;
    }
}

//解析 总线接收刷卡卡号
bool cLCDScreen::ParseCenterCardNumber(InfoMap CenterMap)
{
    QByteArray CardNum;
    const char *pResult = "未获取到卡号";
    if(CenterMap.isEmpty())
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! CenterMap Empty");
        stCardResult.ucResult = 2;
        strncpy(stCardResult.chFaultReason, pResult, sizeof(stCardResult.chFaultReason));
        return FALSE;
    }
    CardNum = CenterMap[Addr_CardAccount];
    iCardLen = CardNum.size();
    memcpy(stCardResult.chCardNum, CardNum.data(), CardNum.size());
    return TRUE;
}

//解析 总线接收账户信息
bool cLCDScreen::ParseCenterAccountInfo(InfoMap CenterMap)
{
//    AccountInfo stInfo;
    const char *pResult = "未获取到用户信息";
    if(CenterMap.isEmpty())
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! CenterMap Empty");
        stCardResult.ucResult = 2;
        strncpy(stCardResult.chFaultReason, pResult, sizeof(stCardResult.chFaultReason));
        return FALSE;
    }
    else
    {
        if(CenterMap.contains(Addr_Account_Balance))//南京新协议3.0
        {
            stCardResult.uiBalance = *(float*)CenterMap[Addr_Account_Balance].data() * 100;
            stCardResult.ucResult = 1;
        }

        if(CenterMap.contains(Addr_CardAccountList))    //账户信息
        {
            AccountInfo stInfo;
            memcpy((char *)&stInfo, CenterMap[Addr_CardAccountList].data(), CenterMap[Addr_CardAccountList].length());

            QString strlog = QString("stInfo.stAccount[0].uiValue is : %1").arg(stInfo.stAccount[0].uiValue);
            pLog->getLogPoint(_strLogName)->info(strlog);

            //目前仅处理现金账户余额
            if(stInfo.stAccount[0].uiValue == 0x7FFFFFFF)
            {
            }
            else
            {
                stCardResult.uiBalance = stInfo.stAccount[0].uiValue;
            }
            stCardResult.ucResult = 1;
        }
    }

    return TRUE;
}

//解析 总线接收刷卡内部申请, 开始充电, 结束充电结果 ---- 内部结果(充电模块返回), stCardResult.ucFlag: 1, 开始充电; 2, 结束充电
bool cLCDScreen::ParseCenterCardInResult(InfoMap CenterMap, CardResult_LCD &stCardResult)
{
    if(CenterMap.isEmpty())
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! CenterMap Empty");
        return FALSE;
    }
    unsigned char ucCanID;
    unsigned char ucResult;
    QByteArray tempCardNum;
    QByteArray retArray;

    stChargeConfig ChargeConfig;
    pParamSet->querySetting(&ChargeConfig,PARAM_CHARGE);

    ucCanID = (unsigned char)CenterMap[Addr_CanID_Comm].at(0);//CAN地址

    if(stCardResult.ucFlag == 1)//开始充电
    {
        ucResult = (unsigned char)CenterMap[Addr_InApplyStartCharge_Result].at(0);//内部申请结果
    }
    else//结束充电
    {
        ucResult = (unsigned char)CenterMap[Addr_InApplyStopCharge_Result].at(0);//内部结束结果
    }
    tempCardNum = CenterMap[Addr_CardAccount];//卡号
    //CAN地址校验和卡号校验
    if((ucCanID == stCardResult.ucCanID)&&(!memcmp(tempCardNum.data(), stCardResult.chCardNum, tempCardNum.length())))
    {
        //将结果显示到屏幕, 0x01: 失败, 0xFF, 成功
        if(ucResult == 0xFF)//成功
        {
            stCardResult.ucResult = 1;
        }
        else
        {
            //将结果显示到屏幕
            memset(stCardResult.chFaultReason,0x00,sizeof(stCardResult.chFaultReason));
            if(ChargeConfig.languageType == 1)
                retArray = GetCardResult(ucResult, 1);
            else if(ChargeConfig.languageType == 2)
                retArray = GetCardResultEnglish(ucResult, 1);
            strncpy(stCardResult.chFaultReason, retArray.data(), retArray.length());
            stCardResult.ucResult = 2;
        }
        return TRUE;
    }
    return FALSE;
}

//解析 总线接收刷卡开始充电/结束充电结果----外部结果(充电服务模块返回), ucType: 1, 开始充电; 2, 结束充电
bool cLCDScreen::ParseCenterCardOutResult(InfoMap CenterMap, CardResult_LCD &stCardResult)
{
    if(CenterMap.isEmpty())
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! CenterMap Empty");
        return FALSE;
    }
    else//显示结果
    {
        unsigned char ucCanID;
        unsigned char ucResult;
        QByteArray tempName;
        QByteArray retArray;

        stChargeConfig ChargeConfig;
        pParamSet->querySetting(&ChargeConfig,PARAM_CHARGE);

        ucCanID = (unsigned char)CenterMap[Addr_CanID_Comm].at(0);//CAN地址
        if(stCardResult.ucFlag == 1)
        {
            ucResult = (unsigned char)CenterMap[Addr_CardApplyCharge_Result].at(0);//开始充电结果
        }
        else if(stCardResult.ucFlag == 2)
        {
            ucResult = (unsigned char)CenterMap[Addr_CardStopCharge_Result].at(0);//结束充电结果
        }
        tempName = CenterMap[Addr_CardAccount];//卡号

        if((ucCanID == stCardResult.ucCanID)&&(!strncmp(tempName.data(), stCardResult.chCardNum, tempName.length())))
        {
            //将结果显示到屏幕
            if(ChargeConfig.languageType == 1)
                retArray = GetCardResult(ucResult, stCardResult.ucFlag);
            else if(ChargeConfig.languageType == 2)
                retArray = GetCardResultEnglish(ucResult, stCardResult.ucFlag);
            strncpy(stCardResult.chFaultReason, retArray.data(), retArray.length());
            if(ucResult== 0xFF)
            {
                stCardResult.ucResult = 1;
            }
            else
            {
                stCardResult.ucResult = 2;
            }
        }
    }
    return TRUE;
}
//解析　VIN申请充电结果
bool cLCDScreen::ParseCenterVinInResult(InfoMap CenterMap, unsigned char ucType)
{
    if(CenterMap.isEmpty())
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! CenterMap Empty");
        return FALSE;
    }
    else
    {
        unsigned char ucCanID = 0;
        unsigned char ucResult = 0;
        QByteArray tempVINNum;
        QByteArray retArray;

        stChargeConfig ChargeConfig;
        pParamSet->querySetting(&ChargeConfig,PARAM_CHARGE);

        ucCanID = (unsigned char)CenterMap[Addr_CanID_Comm].at(0);//CAN地址

        if(ucType == 1)//开始充电
        {
            ucResult = (unsigned char)CenterMap[Addr_InVINApplyStartChargeType_Result].at(0);//内部申请结果
        }
        else if(ucType == 2)//结束充电
        {
            ucResult = (unsigned char)CenterMap[Addr_InVINApplyStopChargeType_Result].at(0);//内部结束结果
        }
        tempVINNum= CenterMap[Addr_BatteryVIN_BMS];//卡号
        if(ucResult == 0xFF)//成功
        {
            stCardResult.ucResult = 1;
        }
        else
        {
            //将结果显示到屏幕
            memset(stCardResult.chFaultReason,0x00,sizeof(stCardResult.chFaultReason));
            if(ChargeConfig.languageType == 1)
                retArray = GetCardResult(ucResult, 1);
            else if(ChargeConfig.languageType == 2)
                retArray = GetCardResultEnglish(ucResult, 1);
            strncpy(stCardResult.chFaultReason, retArray.data(), retArray.length());
            stCardResult.ucResult = 2;
        }
    }

    return TRUE;
}
//解析 总线接收到U盘处理结果
bool cLCDScreen::ParseCenterUdiskResult(InfoMap CenterMap)
{
    unsigned char ucSource = 0;
    if(CenterMap.isEmpty())
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! CenterMap Empty");

        return FALSE;
    }
    ucSource = CenterMap[Addr_Cmd_Source].data()[0];
    if(ucSource == 1)
    {
        return FALSE;
    }
    stUresult.ucType = CenterMap[Addr_Cmd_Master].data()[0];
    stUresult.ucResult = CenterMap[Addr_Back_Result].data()[0];
    return TRUE;
}

//处理----维护数据指令
void cLCDScreen::DealMaintainDataType(const QByteArray &recvArray)
{
    unsigned char ucAddr = recvArray.at(1);
    unsigned short usDataType =  0;
    memcpy(&usDataType, recvArray.data() + 5, 2);
    ucNowAddr = ucAddr;
    switch(usDataType)
    {
    case Data_Version_LCD:
        sendVersion(ucAddr);
        break;
    default:
        break;
    }
}

//处理----查询数据指令
void cLCDScreen::DealQueryDataType(const QByteArray &recvArray)
{
    unsigned char ucAddr = recvArray.at(1);
    unsigned short usDataType =  0;
    memcpy(&usDataType, recvArray.data() + 5, 2);
    ucNowAddr = ucAddr;
    switch(usDataType)
    {
    case Data_SysParam_LCD: //系统设置类数据
        sendSysParam(ucAddr);
        break;
    case Data_PhaseType_LCD: //三相相别设置数据
        sendPhaseType(ucAddr);
        break;
    case Data_SysSpecSet_LCD: //系统特殊功能设置数据
        sendSysSpecSet(ucAddr);
        break;
    case Data_DCSpecSet_LCD: //直流特殊功能设置类数据
        ParseDCSpecSet(ucAddr, recvArray.at(4), Data_DCSpecSet_LCD);
        applyCenterDCSpecFunc();
        break;
    case Data_LoadLimit_LCD: //负荷约束设置类数据
        sendLoadLimit(ucAddr);
        break;
    case Data_PeakSet_LCD: //错峰充电设置类数据
        sendPeakSet(ucAddr);
        break;
    case Data_AmmeterAddr_LCD: //电表地址数据
        sendAmmeterAddr(ucAddr);
        break;
    case Data_Passwd_LCD:   //登录密码数据
        sendPasswd(ucAddr);
        break;
    case Data_IOState_LCD:  //IO常开常闭状态
        sendIOState(ucAddr);
        break;
    case Data_2DbarCodes_LCD:   //二维码设置
        send2DbarCodes(ucAddr);
        break;
    case Data_DCCabArgTypeNum_LCD:  //直流柜某参数类型设备数量
    {
        sendFrameDCCabArgTypeNum(ucAddr, recvArray.at(9));
        break;
    }
    case Data_TermParam_DC_LCD: //直流终端参数
    case Data_CCUParam_LCD: //CCU参数
        ParseArgQuery(recvArray);
        break;
    case Data_DCSysParam_LCD:   //直流柜系统参数设置
        break;
    case Data_DCCabDeviceNum_LCD:   //直流机设备个数查询
        sendDCCabDevNum(ucAddr, recvArray);
        break;
    case Data_LocalDynamicPassword_LCD:    //随机码上传,管理员密码下发
        sendRandCode(ucAddr);
        break;
    case Data_LocalChargePassword_LCD:  //本地充电密码设置
        sendLocalChargePassword(ucAddr);
        break;
    default:
        break;
    }
}

//处理----设置数据指令
void cLCDScreen::DealSetDataType(const QByteArray &recvArray)
{
    //  GuoCheng Add Begin
    unsigned char ucCanID = 0;
    stCSCUSysConfig cscuSysConfig;       //CSCU参数设置缓存
    TerminalStatus stTemp;
    pParamSet->querySetting(&cscuSysConfig, PARAM_CSCU_SYS);
    for(unsigned char i = 0; i < cscuSysConfig.directCurrent; i++)
    {
        ucCanID = i + ID_MinDCCanID;
        if(!pDevCache->QueryTerminalStatus(ucCanID, stTemp))
        {
            pLog->getLogPoint(_strLogName)->info("ERROR! SendMainPageData  QueryRealStatusMeter Error!");
            DisplaySetPage(recvArray);
            return;
        }
        if(stTemp.en_ChargeStatusChangeType == CHARGE_STATUS_CHANGE_START_CHARGE)
        {
            DisplaySetPage(recvArray);
            return;
        }
    }
    for(unsigned char i = 0; i < cscuSysConfig.singlePhase; i++)
    {
        ucCanID = i + ID_MinACSinCanID;
        if(!pDevCache->QueryTerminalStatus(ucCanID, stTemp))
        {
            pLog->getLogPoint(_strLogName)->info("ERROR! SendMainPageData  QueryRealStatusMeter Error!");
            DisplaySetPage(recvArray);
            return;
        }
        if(stTemp.en_ChargeStatusChangeType == CHARGE_STATUS_CHANGE_START_CHARGE)
        {
            DisplaySetPage(recvArray);
            return;
        }
    }
    for(unsigned char i = 0; i < cscuSysConfig.threePhase; i++)
    {
        ucCanID = i + ID_MinACThrCanID;
        if(!pDevCache->QueryTerminalStatus(ucCanID, stTemp))
        {
            pLog->getLogPoint(_strLogName)->info("ERROR! SendMainPageData  QueryRealStatusMeter Error!");
            DisplaySetPage(recvArray);
            return;
        }
        if(stTemp.en_ChargeStatusChangeType == CHARGE_STATUS_CHANGE_START_CHARGE)
        {
            DisplaySetPage(recvArray);
            return;
        }
    }
    //  GuoCheng Add End

    unsigned char ucAddr = recvArray.at(1);
    unsigned short usDataType =  0;
    memcpy(&usDataType, recvArray.data() + 5, 2);
    ucNowAddr = ucAddr;
    unsigned short usLen = (recvArray.at(7) + recvArray.at(8)*0x100);
    int iReboot;
    switch(usDataType)
    {
    case Data_SysParam_LCD: //系统设置类数据
        iReboot = updateSysParam(recvArray.data()+9, usLen);
        sendOperateResult(ucAddr, Data_SysParam_LCD, Ack_Success_LCD, iReboot);
        break;
    case Data_PhaseType_LCD: //三相相别设置数据
        iReboot = updatePhaseType(recvArray.data()+9, usLen);
        sendOperateResult(ucAddr, Data_PhaseType_LCD, Ack_Success_LCD, iReboot);
        break;
    case Data_SysSpecSet_LCD: //系统特殊功能设置数据
        iReboot = updateSysSpecSet(recvArray.data()+9, usLen);
        sendOperateResult(ucAddr, Data_SysSpecSet_LCD, Ack_Success_LCD, iReboot);
        break;
    case Data_DCSpecSet_LCD: //直流特殊功能设置类数据
        iReboot = updateDCSpecSet(recvArray.data()+9, usLen);
        sendOperateResult(ucAddr, Data_DCSpecSet_LCD, Ack_Success_LCD, iReboot);
        break;
    case Data_LoadLimit_LCD: //负荷约束设置类数据
        iReboot = updateLoadLimitSet(recvArray.data()+9, usLen);
        sendOperateResult(ucAddr, Data_LoadLimit_LCD, Ack_Success_LCD, iReboot);
        break;
    case Data_PeakSet_LCD: //错峰充电设置类数据
        updatePeakSet(recvArray.data()+9, usLen);
        uiPeakSetCounter = 0;
        bWaitPeakSetFlag = TRUE;
        break;
    case Data_AmmeterAddr_LCD: //电表地址数据
        break;
    case Data_Passwd_LCD:   //登录密码数据
        iReboot = updatePasswd(recvArray.data()+9, usLen);
        sendOperateResult(ucAddr, Data_Passwd_LCD, Ack_Success_LCD, iReboot);
        break;
    case Data_IOState_LCD:   //IO常开常闭状态
        iReboot = updateIOState(recvArray.data()+9, usLen);
        sendOperateResult(ucAddr, Data_IOState_LCD, Ack_Success_LCD, iReboot);
        break;
    case Data_2DbarCodes_LCD:   //二维码设置
        iReboot = update2DbarCodes(recvArray.data()+9, usLen);
        sendOperateResult(ucAddr, Data_2DbarCodes_LCD, Ack_Success_LCD, iReboot);
        break;
    case Data_TermParam_DC_LCD: //直流终端参数设置
        iReboot = updateTermArgSet(recvArray.data()+9, usLen);
        sendOperateResult(ucAddr, Data_TermParam_DC_LCD, Ack_Success_LCD, iReboot);
        break;
    case Data_CCUParam_LCD: //CCU参数设置
        iReboot = updateCCUArgSet(recvArray.data()+9, usLen);
        sendOperateResult(ucAddr, Data_CCUParam_LCD, Ack_Success_LCD, iReboot);
        break;
    case Data_LocalDynamicPassword_LCD:    //随机码上传,管理员密码下发
        sendDynamicPasswordResult(ucAddr, recvArray);
        break;
    case Data_LocalChargePassword_LCD:  //本地充电密码设置
        iReboot = updateLocalChargePassword(recvArray.data()+9);
        sendOperateResult(ucAddr, Data_LocalChargePassword_LCD, Ack_Success_LCD, iReboot);
        break;
    default:
        break;
    }
}

//处理----状态数据指令
void cLCDScreen::DealStateDataType(const QByteArray &recvArray)
{
    unsigned short usDataType =  0;
    memcpy(&usDataType, recvArray.data() + 5, 2);
    unsigned char ucAddr = recvArray.at(1);
    ucNowAddr = ucAddr;
    switch(usDataType)
    {
    case Data_TermState_LCD: //终端状态数据, recvArray.at(9) CAN地址
        sendTermState(ucAddr, recvArray.at(9));
        break;
    case Data_CSCUState_LCD:    //CSCU状态数据
        sendCSCUState(ucAddr);
        break;
    default:
        break;
    }
}

//处理----实时数据指令
void cLCDScreen::DealRealDataType(const QByteArray &recvArray)
{
    unsigned char ucAddr = recvArray.at(1);
    unsigned short usDataType =  0;
    memcpy(&usDataType, recvArray.data() + 5, 2);
    ucNowAddr = ucAddr;

    switch(usDataType)
    {
    case Data_TermMeasure_Normal_LCD: //终端测量数据----普通版, recvArray.at(10) CAN地址
        sendTermMeasure_Normal(ucAddr, recvArray.at(9));
        break;
    case Data_TermMeasure_Card_LCD: //终端测量数据----刷卡版, recvArray.at(10) CAN地址
        sendTermMeasure_Card(ucAddr, recvArray.at(9));
        break;
    case Data_TermBMS_LCD: //终端BMS数据
        sendTermBMS(ucAddr, recvArray.at(9));
        break;
    case Data_TermFaultInfo_LCD: //终端故障信息数据
        sendTermFaultInfo(ucAddr, recvArray.at(9));
        break;
    case Data_AmmeterData_LCD:  //进线侧数据
        sendAmmeterData(ucAddr, recvArray);
        break;
    case  Data_EnvInfo_LCD:    //环境信息
        sendEnvInfo(ucAddr, recvArray);
        break;
    case  Data_DCMOD_LCD:    //功率模块信息
        sendModData(ucAddr, recvArray.at(9), recvArray.at(10));
        break;
    case  Data_DCPDU_LCD:    //pdu信息
        sendPDUData(ucAddr, recvArray.at(9), recvArray.at(10));
        break;
    case  Data_DCCCU_LCD:    //ccu信息
        sendCCUData(ucAddr, recvArray.at(9), recvArray.at(10));
        break;
    case  Data_NoRemoveFaultRecordNum_LCD:          //add by muty 20170913 发送未消除故障记录条目
        sendNoRemoveFaultRecordNum(ucAddr, recvArray);
        break;
    case  Data_NoRemoveFaultRecord_LCD:                 //add by muty 20170913 发送未消除故障记录
        sendNoRemoveFaultRecord(ucAddr, recvArray);
        break;
    default:
        break;
    }
}

//处理----历史数据指令
void cLCDScreen::DealHisDataType(const QByteArray &recvArray)
{
    unsigned char ucAddr = recvArray.at(1);
    unsigned short usDataType =  0;
    memcpy(&usDataType, recvArray.data() + 5, 2);
    ucNowAddr = ucAddr;
    switch(usDataType)
    {
    case Data_RecordNum_LCD:  //历史记录条目
        sendRecordNum(ucAddr, recvArray);
        break;
    case Data_FaultRecord_LCD:  //故障记录条目
        sendFaultRecord(ucAddr, recvArray);
        break;
    case Data_ChargeRecord_LCD:  //充电记录条目
        sendChargeRecord(ucAddr, recvArray);
        break;
    case Data_OperateRecord_LCD:    //操作记录条目
        sendOperateRecord(ucAddr, recvArray);
        break;
    case Data_TermChargeReport_LCD:  //终端充电报告
        sendTermChargeReport(ucAddr, recvArray.at(9));
        break;
    default:
        break;
    }
}

//处理----交互数据指令
void cLCDScreen::DealInterActiveType(const QByteArray &recvArray)
{
    unsigned char ucAddr = recvArray.at(1);
    unsigned short usDataType =  0;
    memcpy(&usDataType, recvArray.data() + 5, 2);
    ucNowAddr = ucAddr;
    unsigned short usLen = (recvArray.at(7) + recvArray.at(8)*0x100);

    switch(usDataType)
    {
    case Data_CardInfoApply_LCD:   //刷卡信息回复
    {
        memset(&stCardResult,0x00, sizeof(stCardResult));
        pProtocol->ParseCardInfoApply(recvArray.data()+9, usLen, stCardResult);
        unsigned char ucStopCardFlag=*(recvArray.data()+11); //读卡，停止读卡
        // nihai add 20170523 刷卡停止
        if(ucStopCardFlag==2)
        {
            sendCenterCardStop();  //停止读取卡
        }else //读卡
        {
            sendCenterApplyCardNum(ucNowAddr, stCardResult.ucCanID);
        }//nihai add
        break;
    }
    case Data_CardInfoAck_LCD:    //刷卡返回卡号, 账户信息
    {
        if((ucCardStep == Step_GetCardNum_LCD)
                ||(ucCardStep == Step_WaitingAccResult_LCD)
                ||(ucCardStep == Step_GetAccResult_LCD))
        {
            sendCardResult(ucNowAddr, stCardResult, Data_CardInfoAck_LCD);
        }
        break;
    }
    case Data_CardCmdApply_LCD:     //刷卡开始充电,结束充电命令下发
    {
        memset(&stCmdApply,0x00, sizeof(stCmdApply));
        pProtocol->ParseCardCmdApply(recvArray.data()+9, usLen, stCmdApply);
        stCardResult.ucCanID = stCmdApply.ucCanID;
        stCardResult.ucFlag = stCmdApply.ucFlag;
        if(stCmdApply.ucFlag == 1)
        {
            if(stCmdApply.ucType == 3)
            {
                sendCenterApplyStartCharge_VIN(stCmdApply);
            }
            else
            {
                sendCenterApplyStartCharge_Card(stCmdApply);
            }
        }
        else if(stCmdApply.ucFlag == 2)
        {
            sendCenterApplyStopCharge_Card(stCmdApply.ucCanID);
            ucCardStep = Step_WaitingStopChargeResult_LCD;
        }
        break;
    }
    case Data_CardCmdAck_LCD:    //刷卡返回开始充电结束充电命令结果
    {
        if((ucCardStep == Step_GetStartChargeResult_LCD)||(ucCardStep == Step_GetStopChargeResult_LCD))
        {
            sendCardResult(ucNowAddr, stCardResult, Data_CardCmdAck_LCD);
        }
        break;
    }
    case Data_UdiskUpdate_LCD:  //U盘升级数据导出
    {
        ParseUdiskCmd(recvArray);
        break;
    }
    case Data_UdiskUpdateAck_LCD:  //U盘升级数据导出结果返回
    {
        sendUdiskResult(ucNowAddr, stUresult);
        break;
    }
    case Data_LocalChargeCmd_LCD:   //本地充电指令下发
    {
        memset(&stLocalCharge,0x00, sizeof(stLocalCharge));
        stLocalCharge.ucCanID = recvArray.at(9);
        stLocalCharge.ucState = recvArray.at(10);
        stLocalCharge.ucLocalType = recvArray.at(11);
        stLocalCharge.ucChargeType = recvArray.at(12);

        sendCenterCmdCtrlApply();
        break;
    }
    case Data_LocalChargeStop_LCD:   //本地结束按钮
    {
        unsigned char ucStopCardFlag=*(recvArray.data()+11); //读卡，停止读卡
        memset(&stLocalChargeStop,0x00, sizeof(stLocalChargeStop));
        if(ucStopCardFlag==2)
        {
            memset(&stLocalCharge,0x00, sizeof(stLocalCharge));
            stLocalChargeStop.ucCanID = recvArray.at(9);
            stLocalChargeStop.ucState = recvArray.at(10);
            stLocalChargeStop.ucChargeType = recvArray.at(11);
            SendApplyStopCharge(recvArray.at(9));
            sendCenterCardStop();  //停止读取卡
        }
        break;
    }
    case Data_TicketDev_LCD:
        memset(&stTicketDevResult,0,sizeof(TicketDevResult_LCD));
        stTicketDevResult.ucCanID = *(recvArray.data()+9);
        stChargeConfig ChargeConfig;
        TerminalStatus stTerm;

        pParamSet->querySetting(&ChargeConfig, PARAM_CHARGE);
        if(! pDevCache->QueryTerminalStatus(stTicketDevResult.ucCanID,stTerm))
            {
                pLog->getLogPoint(_strLogName)->info("ERROR! Screen pDevCache Query  Error!");
                return;
            }
        if(ChargeConfig.ticketEnable == 1)  //小票机使能
        {
            if(stTerm.bTicketPrint == 1)
            {
                sendTickectDevStart(stTicketDevResult.ucCanID);
            }
            else if(stTerm.bTicketPrint == 2)
            {
                stTicketDevResult.ucResult = 0;
                const char *temp="您已打印一次,不允许重复打印!";
                strncpy(stTicketDevResult.chFaultReason,temp,sizeof(stTicketDevResult.chFaultReason));

                sendTickectDevData(ucNowAddr);
            }
        }
        else //小票机功能未使能，请先进行配置！
        {
            stTicketDevResult.ucResult = 0;
            const char *temp="小票机功能未使能,请先进行配置!";
            strncpy(stTicketDevResult.chFaultReason,temp,sizeof(stTicketDevResult.chFaultReason));

            sendTickectDevData(ucNowAddr);
        }
        break;
    default:
        break;
    }
}

//生成随机码
unsigned int cLCDScreen::generateRandCode()
{
    int i,j;
    char str_buff[10];
    memset(str_buff, 0, sizeof(str_buff));
    srand((unsigned int)time(0));

    for(i=0; i<6; i++)
    {
        j = 1+(int)(10.0*rand()/(RAND_MAX+1.0));
        if(j >= 10 )
            j = 1;
        str_buff[i] = j+0x30;
    }
    return atoi(str_buff);
}

//生成动态密码
unsigned int cLCDScreen::generateDynamicCode(unsigned int param)
{
    unsigned int value = (param + 300001)%1000000;
    int str_len=0,i;
    char str_param[20];

    memset(str_param, 0, sizeof(str_param));

    snprintf(str_param, sizeof(str_param), "%d", value );
    str_len = strlen(str_param);

    for(i=0; i<str_len; i++)
      str_param[i] = str_param[i] - 0x30;

    for(i=0; i<str_len; i++)
    {
      if(i != (str_len -1))
        str_param[i] = (str_param[i]+str_param[i+1])%10;
    }

    for(i=str_len; i>0; i--)
    {
      if(i != 1)
        str_param[i-1] = (str_param[i-1]+str_param[i-2])%10;
    }

    for(i=0; i<str_len; i++)
      str_param[i] = str_param[i] + 0x30;

    return atoi(str_param);

}

//发送版本信息
void cLCDScreen::sendVersion(unsigned char ucAddr)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    pProtocol->MakFrameCSCUVersion(pData, usLen, ucAddr);
    sendMessage(pData, usLen);
    delete pData;
}

//发送系统参数设置
void cLCDScreen::sendSysParam(unsigned char ucAddr)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    pProtocol->MakFrameCSCUSet(pData, usLen, ucAddr);
    sendMessage(pData, usLen);
    delete pData;
}

//发送相别设置
void cLCDScreen::sendPhaseType(unsigned char ucAddr)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    if(pProtocol->MakFramePhaseTypeSet(pData, usLen, ucAddr) == FALSE)
    {
        return;
    }
    sendMessage(pData, usLen);
    delete pData;
}

//发送 电表数据
void cLCDScreen::sendAmmeterAddr(unsigned char ucAddr)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    if(pProtocol->MakFrameAmmeterAddr(pData, usLen, ucAddr) == FALSE)
    {
        return;
    }
    sendMessage(pData, usLen);

    delete pData;
}

//发送 登录密码数据
void cLCDScreen::sendPasswd(unsigned char ucAddr)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    if(pProtocol->MakFramePasswd(pData, usLen, ucAddr) == FALSE)
    {
        return;
    }
    sendMessage(pData, usLen);
    delete pData;
}

//发送 IO常开常闭状态
void cLCDScreen::sendIOState(unsigned char ucAddr)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    if(pProtocol->MakFrameIOState(pData, usLen, ucAddr) == FALSE)
    {
        return;
    }
    sendMessage(pData, usLen);
    delete pData;
}

//发送 二维码
void cLCDScreen::send2DbarCodes(unsigned char ucAddr)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    pProtocol->MakFrame2DbarCodes(pData, usLen, ucAddr);
    sendMessage(pData, usLen);
    delete pData;
}

//发送 直流柜某参数类型设备个数
void cLCDScreen::sendFrameDCCabArgTypeNum(unsigned char ucAddr, unsigned char ucType)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    pProtocol->MakFrameDCCabArgTypeNum(pData, usLen, ucAddr, ucType);
    sendMessage(pData, usLen);
    delete pData;
}

//发送系统特殊功能设置
void cLCDScreen::sendSysSpecSet(unsigned char ucAddr)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    pProtocol->MakFrameSysSpecSet(pData, usLen, ucAddr);
    sendMessage(pData, usLen);
    delete pData;
}

//发送直流特殊功能设置
void cLCDScreen::sendDCSpecSet(unsigned char ucAddr)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    pProtocol->MakFrameDCSpecSet(pData, usLen, ucAddr);
    sendMessage(pData, usLen);
    delete pData;
}

//发送 负荷约束功能
void cLCDScreen::sendLoadLimit(unsigned char ucAddr)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    pProtocol->MakFrameLoadLimit(pData, usLen, ucAddr);
    sendMessage(pData, usLen);
    delete pData;
}

//发送 错峰充电功能
void cLCDScreen::sendPeakSet(unsigned char ucAddr)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    if(pProtocol->MakFramePeakSet(pData, usLen, ucAddr) == FALSE)
    {
        return;
    }
    sendMessage(pData, usLen);
    for(int i = 0 ; i < usLen; i++)
    {
    }
    delete pData;
}

//发送 枪参数
void cLCDScreen::sendGunArg(unsigned char ucAddr, unsigned char ucArgCanID)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    if(pProtocol->MakFrameGunSet(pData, usLen, ucAddr, ucArgCanID) == FALSE)
    {
        return;
    }
    sendMessage(pData, usLen);
    delete pData;
}

//发送 CCU参数
void cLCDScreen::sendCCUArg(unsigned char ucAddr, unsigned char ucArgCanID)
{

    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    if(pProtocol->MakFrameCCUSet(pData, usLen, ucAddr, ucArgCanID) == FALSE)
    {
        return;
    }
    sendMessage(pData, usLen);
//    for(int i = 0; i < usLen; i++)
//    {
//    }
    delete pData;
}

//发送 直流柜设备个数
void cLCDScreen::sendDCCabDevNum(unsigned char ucAddr, const QByteArray &recvArray)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    unsigned char ucType = recvArray.at(9);
    if(pProtocol->MakFrameDCCabDevNum(pData, usLen, ucAddr, ucType) == FALSE)
    {
        return;
    }
    sendMessage(pData, usLen);
    delete pData;
}

//发送 随机码(本地充电)
void cLCDScreen::sendRandCode(unsigned char ucAddr)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    unsigned int uiRandCode = generateRandCode();
    uiLocalDynamicPassword = generateDynamicCode(uiRandCode);
    if(pProtocol->MakFrameRandCode(pData, usLen, ucAddr, uiRandCode) == FALSE)
    {
        return;
    }
    for(int i = 0; i< usLen; i++)
    {
    }
    sendMessage(pData, usLen);
    delete pData;
}

//发送 密码比较结果(本地充电)
void cLCDScreen::sendDynamicPasswordResult(unsigned char ucAddr, const QByteArray &recvArray)
{

    unsigned char ucResult = 0;
//    unsigned int uiRandCode = generateRandCode();
    unsigned int uiPassword = *((int *)(recvArray.data() + 9));
    int iReboot = 0;
//    uiLocalDynamicPassword = generateDynamicCode(uiRandCode);

    if(uiLocalDynamicPassword == uiPassword)
    {
        ucResult = Ack_Success_LCD;
    }
    else
    {
        ucResult = Ack_Failed_LCD;
    }

    sendOperateResult(ucAddr, Data_LocalDynamicPassword_LCD, ucResult, iReboot);
}

//发送 本地密码(本地充电)
void cLCDScreen::sendLocalChargePassword(unsigned char ucAddr)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    pProtocol->MakFrameLocalChargePassword(pData, usLen, ucAddr);

    sendMessage(pData, usLen);
    delete pData;
}

//发送 终端状态
void cLCDScreen::sendTermState(unsigned char ucAddr, unsigned char ucCanID)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    if(pProtocol->MakFrameTermState(pData, usLen, ucAddr, ucCanID) == FALSE)
    {
        return;
    }
    sendMessage(pData, usLen);
    delete pData;
}

//发送 CSCU状态
void cLCDScreen::sendCSCUState(unsigned char ucAddr)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    if(pProtocol->MakFrameCSCUState(pData, usLen, ucAddr) == FALSE)
    {
        return;
    }
    sendMessage(pData, usLen);
    pProtocol->ucUDiskState = 0;
    delete pData;
}

//发送 终端测量数据----普通版
void cLCDScreen::sendTermMeasure_Normal(unsigned char ucAddr, unsigned char ucCanID)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    if(pProtocol->MakFrameTermMeasure_Normal(pData, usLen, ucAddr, ucCanID) == FALSE)
    {
        return;
    }
    sendMessage(pData, usLen);
    delete pData;
}

//发送 终端测量数据----刷卡版
void cLCDScreen::sendTermMeasure_Card(unsigned char ucAddr, unsigned char ucCanID)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    if(pProtocol->MakFrameTermMeasure_Card(pData, usLen, ucAddr, ucCanID) == FALSE)
    {
        return;
    }
    sendMessage(pData, usLen);
    delete pData;
}

//发送 终端BMS数据
void cLCDScreen::sendTermBMS(unsigned char ucAddr, unsigned char ucCanID)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    if(pProtocol->MakFrameTermBMS(pData, usLen, ucAddr, ucCanID) == FALSE)
    {
        return;
    }
    sendMessage(pData, usLen);
    delete pData;
}

//发送 终端故障信息数据
void cLCDScreen::sendTermFaultInfo(unsigned char ucAddr, unsigned char ucCanID)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    if(pProtocol->MakFrameTermFaultInfo(pData, usLen, ucAddr, ucCanID) == FALSE)
    {
        return;
    }
    sendMessage(pData, usLen);
    delete pData;
}

//发送 终端充电报告
void cLCDScreen::sendTermChargeReport(unsigned char ucAddr, unsigned char ucCanID)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    if(pProtocol->MakFrameTermChargeReport(pData, usLen, ucAddr, ucCanID) == FALSE)
    {
        return;
    }
    sendMessage(pData, usLen);
    delete pData;
}

//发送 进线侧信息数据
void cLCDScreen::sendAmmeterData(unsigned char ucAddr, const QByteArray &recvArray)
{

    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    if(pProtocol->MakFrameAmmeterData(pData, usLen, ucAddr, recvArray.data()) == FALSE)
    {
        return;
    }
    sendMessage(pData, usLen);

    delete pData;
}

//发送 环境信息
void cLCDScreen::sendEnvInfo(unsigned char ucAddr, const QByteArray &recvArray)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    if(pProtocol->MakFrameEnvInfo(pData, usLen, ucAddr, recvArray.data()) == FALSE)
    {
        return;
    }
    sendMessage(pData, usLen);
    delete pData;
}
//发送 功率模块实时数据
void cLCDScreen::sendModData(unsigned char ucAddr, unsigned char ucCanID, unsigned char ucInnerID)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    if(pProtocol->MakFrameModData(pData, usLen, ucAddr, ucCanID, ucInnerID) == FALSE)
    {
        return;
    }
    sendMessage(pData, usLen);
//    for(int i = 0 ; i < usLen; i++)
//    {
//    }
    delete pData;
}

//发送 PDU实时数据
void cLCDScreen::sendPDUData(unsigned char ucAddr, unsigned char ucCanID, unsigned char ucInnerID)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    if(pProtocol->MakFramePDUData(pData, usLen, ucAddr, ucCanID, ucInnerID) == FALSE)
    {
        return;
    }
    sendMessage(pData, usLen);
    delete pData;
}

//发送 CCU实时数据
void cLCDScreen::sendCCUData(unsigned char ucAddr, unsigned char ucCanID, unsigned char ucInnerID)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    if(pProtocol->MakFrameCCUData(pData, usLen, ucAddr, ucCanID, ucInnerID) == FALSE)
    {
        return;
    }
    sendMessage(pData, usLen);
    delete pData;
}

/***********************************************************************
 *函数名：sendNoRemoveFaultRecordNum
 *参数说明：ucAddr   终端地址，recvArray    接收到的原始数据流
 *功能：发送 未消除故障记录条目
 *备注 : add by muty 20170913
 **********************************************************************/
void cLCDScreen::sendNoRemoveFaultRecordNum(unsigned char ucAddr, const QByteArray &recvArray)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;

    if(pProtocol->MakFrameNoRemoveFaultRecordNum(pData, usLen,ucAddr,recvArray) == FALSE)
    {
        return;
    }

    sendMessage(pData, usLen);
    delete pData;
}


/***********************************************************************
 *函数名：sendNoRemoveFaultRecord
 *参数说明：ucAddr   终端地址，recvArray    接收到的原始数据流
 *功能：发送 未消除故障信息
 *备注 : add by muty 20170913
 **********************************************************************/
void cLCDScreen::sendNoRemoveFaultRecord(unsigned char ucAddr, const QByteArray &recvArray)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
     unsigned char ucPageNum = recvArray.at(9);//第几页
    if(pProtocol->MakFrameNoRemoveFaultRecord(pData, usLen, ucAddr, ucPageNum) == FALSE)
    {
        return;
    }
    sendMessage(pData, usLen);
    delete pData;
}


//发送 历史记录条目
void cLCDScreen::sendRecordNum(unsigned char ucAddr, const QByteArray &recvArray)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    if(pProtocol->MakeFrameRecordNum(pData, usLen, ucAddr, recvArray) == FALSE)
    {
        return;
    }
    sendMessage(pData, usLen);
    delete pData;
}

//发送 故障记录
void cLCDScreen::sendFaultRecord(unsigned char ucAddr, const QByteArray &recvArray)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    unsigned char ucPageNum = recvArray.at(9);//第几页
    if(pProtocol->MakFrameFaultRecord(pData, usLen, ucAddr, ucPageNum) == FALSE)
    {
        return;
    }
    sendMessage(pData, usLen);
    delete pData;
}

//发送 充电记录
void cLCDScreen::sendChargeRecord(unsigned char ucAddr, const QByteArray &recvArray)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    unsigned char ucPageNum = recvArray.at(9);//第几页
    if(pProtocol->MakFrameChargeRecord(pData, usLen, ucAddr, ucPageNum) == FALSE)
    {
        return;
    }
    sendMessage(pData, usLen);
    delete pData;
}

//发送 操作记录
void cLCDScreen::sendOperateRecord(unsigned char ucAddr, const QByteArray &recvArray)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    unsigned char ucPageNum = recvArray.at(9);//第几页
    if(pProtocol->MakFrameOperateRecord(pData, usLen, ucAddr, ucPageNum) == FALSE)
    {
        return;
    }
    sendMessage(pData, usLen);
    delete pData;
}

//发送 人机交互结果
void cLCDScreen::sendOperateResult(unsigned char ucAddr,  unsigned short usDataType, unsigned char ucResult, unsigned char ucReboot)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    pProtocol->MakeFrameOperateResult(pData, usLen, ucAddr, usDataType, ucResult, ucReboot);
    sendMessage(pData, usLen);
    delete pData;
}

//发送 刷卡结果
void cLCDScreen::sendCardResult(unsigned char ucAddr, CardResult_LCD stCardResult, unsigned int uiDataType)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    pProtocol->MakeFrameCardResult(pData, usLen, ucAddr, stCardResult, uiDataType);
    sendMessage(pData, usLen);
    delete pData;
}

//发送 升级结果
void cLCDScreen::sendUdiskResult(unsigned char ucAddr, UdiskResult_LCD stResult)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    pProtocol->MakeFrameUdiskResult(pData, usLen, ucAddr, stResult);
    sendMessage(pData, usLen);
    delete pData;
}

//发送 遥控结果
void cLCDScreen::sendCtrlCmdResult(unsigned char ucAddr, LocalCharge_LCD &stLocalCharge)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    pProtocol->MakeFrameCtrlCmdResult(pData, usLen, ucAddr, stLocalCharge);
    sendMessage(pData, usLen);
    delete pData;
}

//发送 刷卡申请卡号到总线
void cLCDScreen::sendCenterApplyCardNum(unsigned char ucAddr, unsigned char ucCanID)
{
    InfoMap ToCenterMap;
    InfoMap::iterator it;
    //记录屏幕地址
    ucNowAddr = ucAddr;
    stCardResult.ucCanID = ucCanID;

    //进行刷卡格式组包
    pProtocol->MakeCenterApplyCardNum(ToCenterMap, ucCanID);
    //发送
    SendCenterData(ToCenterMap, AddrType_ScreenApplyReadCard);
    ucCardStep = Step_WaitingCardNum_LCD;
    uiTimeCounter = 0;
}

//发送 刷卡结束请求, 让读卡器停止读卡
void cLCDScreen::sendCenterCardStop()
{
    InfoMap ToCenterMap;
    //发送
    SendCenterData(ToCenterMap, AddrType_ScreenApplyStopCard);
}

//发送 申请用户信息到总线
void cLCDScreen::sendCenterApplyUserInfo(unsigned char ucCanID)
{
    Q_UNUSED(ucCanID);
    InfoMap ToCenterMap;
    QByteArray CardNum;
    CardNum.append(stCardResult.chCardNum, iCardLen);
    //进行刷卡格式组包
    pProtocol->MakeCenterApplyUserInfo(ToCenterMap, stCardResult.ucCanID, CardNum);
    //发送
    SendCenterData(ToCenterMap, AddrType_CenterCardApplyAccountInfo);
    ucCardStep = Step_GetCardNum_LCD;
    uiTimeCounter = 0;
}

//发送 申请开始充电到总线(刷卡)
void cLCDScreen::sendCenterApplyStartCharge_Card(CardCmdApply_LCD &stCmdApply)
{
    InfoMap ToCenterMap;
    QByteArray CardNum;
    CardNum.append(stCardResult.chCardNum, iCardLen);
    //进行刷卡格式组包
    pProtocol->MakeCenterApplyStartCharge_Card(ToCenterMap, stCmdApply, CardNum);
    //发送
    SendCenterData(ToCenterMap, AddrType_InApplyStartChargeByScreen);
    ucCardStep = Step_WaitingStartChargeResult_LCD;
    uiTimeCounter = 0;
}
//发送 申请开始充电到总线(VIN)
void cLCDScreen::sendCenterApplyStartCharge_VIN(CardCmdApply_LCD &stCmdApply)
{
    InfoMap ToCenterMap;
    CHARGE_STEP stChargeStep;//存放从缓存中读出的状态机
    if(pDevCache->QueryChargeStep(stCmdApply.ucCanID, stChargeStep) != false)
    {
        ToCenterMap.insert(Addr_CanID_Comm, QByteArray(1,stCmdApply.ucCanID));
        ToCenterMap.insert(Addr_BatteryVIN_BMS,QByteArray::fromRawData(stChargeStep.sVIN, LENGTH_VIN_NO));
        ToCenterMap.insert(Addr_VINApplyStartChargeType,QByteArray(1,1));//默认充满为止
        SendCenterData(ToCenterMap, AddrType_VinApplyStartChargeImmed);
        ucCardStep = Step_WaitingStartChargeResult_LCD;
        uiTimeCounter = 0;
    }
    else
    {
        stChargeConfig ChargeConfig;
        pParamSet->querySetting(&ChargeConfig,PARAM_CHARGE);
        pLog->getLogPoint(_strLogName)->info("ERROR! Screen VIN Apply NO ChargeStep !!");
        if(ChargeConfig.languageType == 1)
        {
            const char *pResult = "未收到VIN号,请稍后再试";
            strncpy(stCardResult.chFaultReason, pResult, sizeof(stCardResult.chFaultReason));
        }

        else if(ChargeConfig.languageType == 2)
        {
            const char *pResult1 = "No VIN number is received.";
            strncpy(stCardResult.chFaultReason, pResult1, sizeof(stCardResult.chFaultReason));
        }
        stCardResult.ucResult = 2;
        ucCardStep = Step_GetStartChargeResult_LCD;
        uiTimeCounter = 0;
    }

}
//发送 申请结束充电到总线(刷卡)
void cLCDScreen::sendCenterApplyStopCharge_Card(unsigned char ucCanID)
{
    InfoMap ToCenterMap;
    QByteArray CardNum;
    CardNum.append(stCardResult.chCardNum,iCardLen);
    //进行刷卡格式组包
    pProtocol->MakeCenterApplyStopCharge_Card(ToCenterMap, ucCanID, CardNum);
    //发送
    SendCenterData(ToCenterMap, AddrType_InApplyStopChargeByScreen);
    ucCardStep = Step_WaitingStopChargeResult_LCD;
    uiTimeCounter = 0;
}

//发送 日志导出请求到总线
void cLCDScreen::sendCenterLogOutApply()
{
    InfoMap ToCenterMap;
    QByteArray tempArray;
    tempArray.append(0x02);//来源: U盘
    ToCenterMap.insert(Addr_Cmd_Source, tempArray);
    tempArray.clear();
    tempArray.append(0x02);//命令: 上传
    ToCenterMap.insert(Addr_Cmd_Master, tempArray);
    tempArray.clear();
    tempArray.append(0x01);//类型: 日志
    ToCenterMap.insert(Addr_Cmd_Slave, tempArray);

    //发送
    SendCenterData(ToCenterMap, AddrType_ExecUpdate);
}

//发送 升级请求到总线
void cLCDScreen::sendCenterUpdateApply()
{
    stUresult.ucType = 1;

    InfoMap ToCenterMap;
    QByteArray tempArray;
    tempArray.append(0x02);//来源: U盘
    ToCenterMap.insert(Addr_Cmd_Source, tempArray);
    tempArray.clear();
    tempArray.append(0x01);//命令: 升级
    ToCenterMap.insert(Addr_Cmd_Master, tempArray);    
/*
    QString cmd = "sh /mnt/nandflash/bin/usb_update_pre_check.sh";

    system(cmd.toAscii().data());
    QFile tmpFile("/tmp/no_usb_check");
    if(tmpFile.exists())
    {
        stUresult.ucResult = 2; //没有升级文件
        sendUdiskResult(ucNowAddr, stUresult);
        tmpFile.remove();
    }
    else
    {
        //发送
        SendCenterData(ToCenterMap, AddrType_ExecUpdate);
    }
    */
    char c_result[128];
    int i_rep=0;
    memset(c_result,0,sizeof(c_result));
    FILE *ptr;
    if((ptr=popen("sh /mnt/nandflash/bin/usb_update_pre_check.sh", "r"))!=NULL)
    {
        while(fgets(c_result, 128, ptr)!=NULL)
        {
            if(strlen(c_result)>128)
                break;
        }
        pclose(ptr);
        ptr = NULL;
    }
    else
    {
    }
    i_rep = atoi(c_result);
    if(i_rep==2)
    {
        stUresult.ucResult = 2; //没有升级文件
        sendUdiskResult(ucNowAddr, stUresult);
    }else if(i_rep==3)
    {
        stUresult.ucResult = 3; //没有升级文件
        sendUdiskResult(ucNowAddr, stUresult);
    }else if(i_rep==255)
    {
        //发送
        SendCenterData(ToCenterMap, AddrType_ExecUpdate);
        //发送系统重启
        stUresult.ucResult = 1; //升级成功
        sendUdiskResult(ucNowAddr, stUresult);

    }
}

//发送 申请命令到总线
void cLCDScreen::sendCenterCmdCtrlApply()
{
    InfoMap ToCenterMap;
    QByteArray tempArray;

    tempArray.append(stLocalCharge.ucCanID);
    ToCenterMap.insert(Addr_CanID_Comm, tempArray);
    tempArray.clear();
    if(stLocalCharge.ucChargeType == 1) //开始充电
    {
        tempArray.append(0x01);
    }
    else if(stLocalCharge.ucChargeType == 2) //结束充电
    {
        tempArray.append(0x03);
    }
    ToCenterMap.insert(Addr_ChargeCmd_Ctrl, tempArray);

    //发送
    SendCenterData(ToCenterMap, AddrType_CmdCtrl_Apply);
}

//更新 系统参数
int cLCDScreen::updateSysParam(const char * pData, unsigned short usLen)
{
    return pProtocol->ParseSysParam(pData, usLen);
}

//更新 相别参数
int cLCDScreen::updatePhaseType(const char * pData, unsigned short usLen)
{
    return pProtocol->ParsePhaseParam(pData, usLen);
}

//更新 系统特殊功能设置
int cLCDScreen::updateSysSpecSet(const char * pData, unsigned short usLen)
{
    return pProtocol->ParseSysSpecSet(pData, usLen);
}

//更新 直流特殊功能设置
int cLCDScreen::updateDCSpecSet(const char * pData, unsigned short usLen)
{
    Q_UNUSED(usLen);
    unsigned char ucNum = pData[0];
    InfoMap ToCenterMap;
    QByteArray tempArray;
    for(unsigned char i = 0; i < ucNum; i++)
    {
        tempArray.append(pData[1 + 4*i]);
        ToCenterMap.insert(Addr_CanID_Comm, tempArray);
        tempArray.clear();
        tempArray.append(pData[2 + 4*i]);
        ToCenterMap.insert(Addr_GroupStrategy_GDA, tempArray);
        tempArray.clear();
        tempArray.append(pData[3 + 4*i]);
        ToCenterMap.insert(Addr_WorkState_GDA, tempArray);
        SendCenterData(ToCenterMap, AddrType_GeneralDynamicArgApply);

        ToCenterMap.clear();
        tempArray.clear();
        tempArray.append(pData[4 + 4*i]);
        ToCenterMap.insert(Addr_AuxPowerType_GSA, tempArray);
        tempArray.clear();
        tempArray.append(pData[1 + 4*i]);
        ToCenterMap.insert(Addr_CanID_Comm, tempArray);
        SendCenterData(ToCenterMap, AddrType_GeneralStaticArgApply);
    }
    return 0;
}

//更新 负荷约束设置
int cLCDScreen::updateLoadLimitSet(const char * pData, unsigned short usLen)
{
    return pProtocol->ParseLoadLimitSet(pData, usLen);
}

//更新 错峰充电设置
int cLCDScreen::updatePeakSet(const char * pData, unsigned short usLen)
{
    InfoMap ToCenterMap;

    pProtocol->ParsePeakSet(pData, usLen, ToCenterMap);
    //注:若列表为空, 由负荷调度模块返回失败结果
    SendCenterData(ToCenterMap, AddrType_SmartChargeSet);
    return 0;
}

//更新 屏幕密码设置
int cLCDScreen::updatePasswd(const char * pData, unsigned short usLen)
{
    return pProtocol->ParsePasswd(pData, usLen);
}

//更新 IO常开常闭设置
int cLCDScreen::updateIOState(const char * pData, unsigned short usLen)
{
    return pProtocol->ParseIOState(pData, usLen);
}

//更新 二维码参数设置
int cLCDScreen::update2DbarCodes(const char * pData, unsigned short usLen)
{
    return pProtocol->Parse2DbarCodes(pData, usLen);
}

//更新 终端参数设置
int cLCDScreen::updateTermArgSet(const char * pData, unsigned short usLen)
{
    QList<LCDMapNode> mapList;
    int iRet = pProtocol->ParseTermArgSet(pData, usLen, mapList);
    for(int i = 0 ; i < mapList.count(); i++)
    {
        SendCenterData(mapList.at(i).stMap, mapList.at(i).enType);
    }
    return iRet;
}

//更新 ccu参数设置
int cLCDScreen::updateCCUArgSet(const char * pData, unsigned short usLen)
{
    QList<LCDMapNode> mapList;
    int iRet = pProtocol->ParseCCUArgSet(pData, usLen, mapList);
    for(int i = 0 ; i < mapList.count(); i++)
    {
        SendCenterData(mapList.at(i).stMap, mapList.at(i).enType);
    }
    return iRet;
}

//更新 本地充电密码设置
int cLCDScreen::updateLocalChargePassword(const char * pData)
{
    int iRet = pProtocol->ParseLocalChargePassword(pData);
    return iRet;
}

//申请 直流特殊功能信息 到总线
void cLCDScreen::applyCenterDCSpecFunc()
{
    InfoMap ToCenterMap;
    //发送
    SendCenterData(ToCenterMap, AddrType_GeneralStaticArgRenew_DB);
    SendCenterData(ToCenterMap, AddrType_GeneralDynamicArgRenew_DB);
}

//申请 终端参数查询 到总线
void cLCDScreen::applyCenterTermArg(unsigned char ucCanID)
{
    InfoMap ToCenterMap;
    QByteArray array, arrayID;
    unsigned char ucArgNo = 0;

    arrayID.append((char)ucCanID);
    ToCenterMap.insert(Addr_CanID_Comm, arrayID);
    //发送
    SendCenterData(ToCenterMap, AddrType_ActiveProtectQuery);
    SendCenterData(ToCenterMap, AddrType_GeneralStaticArgQuery);

    array.clear();
    ucArgNo = 1;
    array.append((char)ucArgNo);
    ToCenterMap.clear();
    ToCenterMap.insert(Addr_ArgNo, array);
    ToCenterMap.insert(Addr_CanID_Comm, arrayID);
    SendCenterData(ToCenterMap, AddrType_GeneralDynamicArgQuery);

//    array.clear();
//    ucArgNo = 2;
//    array.append((char)ucArgNo);
//    ToCenterMap.clear();
//    ToCenterMap.insert(Addr_ArgNo, array);
//    ToCenterMap.insert(Addr_CanID_Comm, arrayID);
//    SendCenterData(ToCenterMap, AddrType_GeneralDynamicArgQuery);

    array.clear();
    ucArgNo = 3;
    array.append((char)ucArgNo);
    ToCenterMap.clear();
    ToCenterMap.insert(Addr_ArgNo, array);
    ToCenterMap.insert(Addr_CanID_Comm, arrayID);
    SendCenterData(ToCenterMap, AddrType_GeneralDynamicArgQuery);
}

//申请 CCU参数查询 到总线
void cLCDScreen::applyCenterCCUArg(unsigned char ucCanID)
{
    InfoMap ToCenterMap;
    QByteArray arrayID;

    arrayID.append((char)ucCanID);
    ToCenterMap.insert(Addr_CanID_Comm, arrayID);
    //发送
    SendCenterData(ToCenterMap, AddrType_CCUArgQuery);
}

//接收控制中心数据
void cLCDScreen::slotFromBus(InfoMap RecvCenterMap, InfoAddrType enType)
{
    if(bWorkFlag == FALSE)
    {
        return;
    }
    switch(enType)
    {
    case AddrType_CmdCtrl_AckResult:    //遥控_充电控制申请ACK
    {
        if(RecvCenterMap[Addr_CanID_Comm].at(0) != stLocalCharge.ucCanID &&\
                RecvCenterMap[Addr_CanID_Comm].at(0) != stLocalChargeStop.ucCanID)
        {
            break;
        }
        //开始充电 不一致
        if((RecvCenterMap[Addr_ChargeCmd_Ctrl].at(0) == 1) && (stLocalCharge.ucChargeType != 1))
        {
            break;
        }
        //结束充电 不一致
        if((RecvCenterMap[Addr_ChargeCmd_Ctrl].at(0) == 3) && (stLocalCharge.ucChargeType != 2)&& (stLocalChargeStop.ucChargeType != 2))
        {
            break;
        }
        if(RecvCenterMap[Addr_AckResult_Ctrl].at(0) == 0x01)    //失败
        {
            stLocalCharge.ucResult = 0;
            stLocalChargeStop.ucResult = 0;
        }
        else if(RecvCenterMap[Addr_AckResult_Ctrl].at(0) == 0xFF)   //成功
        {
            stLocalCharge.ucResult = 1;
            stLocalChargeStop.ucResult = 1;
        }
        if(RecvCenterMap[Addr_CanID_Comm].at(0) == stLocalCharge.ucCanID )
            sendCtrlCmdResult(ucNowAddr,stLocalCharge);
        else if(RecvCenterMap[Addr_CanID_Comm].at(0) == stLocalChargeStop.ucCanID)
            sendCtrlStopCmdResult(ucNowAddr,stLocalChargeStop);
        break;
    }
    case AddrType_Udisk_Insert:
    {
        pProtocol->ucUDiskState = 1;
        break;
    }
    case AddrType_GeneralDynamicArgRenewAck_DB: //通用动态参数更新数据库回复
    case AddrType_GeneralStaticArgRenewAck_DB:  //通用静态参数更新数据库回复
    {
        ucDCSpecAckStep++;
        if(ucDCSpecAckStep >= 2)
        {
            for(int i = 0; i  < DelayCmdList.count(); i++)
            {
                if( (DelayCmdList.at(i).uiDataType == Data_DCSpecSet_LCD)
                        &&(DelayCmdList.at(i).uiCmdType == Cmd_Query_LCD) )
                {
                    sendDCSpecSet(DelayCmdList.at(i).ucAddr);
                    DelayCmdList.removeAt(i);
                    i--;
                }
            }
            ucDCSpecAckStep = 0;
        }
        break;
    }
    case AddrType_SmartChargeSet_Result:    //本地设置错峰充电参数设置结果
    {
        if(!RecvCenterMap.contains(Addr_SmartChargeSet_Result))
        {
            return;
        }
        bWaitPeakSetFlag = FALSE;
        if(RecvCenterMap[Addr_SmartChargeSet_Result].at(0) == 0xFF)
        {
            sendOperateResult(ucNowAddr, Data_PeakSet_LCD, Ack_Success_LCD, 0);
        }
        else
        {
            sendOperateResult(ucNowAddr, Data_PeakSet_LCD, Ack_Failed_LCD, 0);
        }
        break;
    }
    case AddrType_CenterReadCard://主题二：集中读卡卡号
    {
        //ocean add 20170522 响应刷卡蜂鸣器
        system("echo 1 > /sys/class/gpio/gpio51/value");

//        usleep(300*1000);
        system("echo 0 > /sys/class/gpio/gpio51/value");
        //ocean add
        ucCardStep = Step_GetCardNum_LCD;
        uiTimeCounter = 0;
        sendCenterCardStop();
        if(ParseCenterCardNumber(RecvCenterMap))
        {
            if(stCardResult.ucFlag == 1)//开始充电, 申请用户信息到总线
            {
                sendCenterApplyUserInfo(stCardResult.ucCanID);
                ucCardStep = Step_WaitingAccResult_LCD;
                uiTimeCounter = 0;
            }
        }
        break;
    }
    case AddrType_ApplyAccountInfoResult_ToScreen://主题六：充电服务返回账户信息给显示屏
    {
        ParseCenterAccountInfo(RecvCenterMap);
        ucCardStep = Step_GetAccResult_LCD;
        uiTimeCounter = 0;
        break;
    }
    case AddrType_InApplyStartChargeResult_ToScreen://主题十：内部申请开始充电结果至显示屏
    case AddrType_InApplyStopChargeResult_ToScreen://主题十八：内部申请结束充电结果至显示屏
    {
        if(ParseCenterCardInResult(RecvCenterMap, stCardResult))
        {
            if(stCardResult.ucResult == 2)
            {
                if(stCardResult.ucFlag == 1)//开始充电
                {
                    ucCardStep = Step_GetStartChargeResult_LCD;
                    uiTimeCounter = 0;
                }
                if(stCardResult.ucFlag == 2)//结束充电
                {
                    ucCardStep = Step_GetStopChargeResult_LCD;
                    uiTimeCounter = 0;
                }
            }
        }
        break;
    }
    case AddrType_OutApplyStartChargeResult_ToScreen://主题十四：远程申请开始充电结果至显示屏
    case AddrType_OutApplyStopChargeResult_ToScreen://主题二十二：远程申请结束充电结果至显示屏
    {
        if(ParseCenterCardOutResult(RecvCenterMap, stCardResult))
        {
            if(stCardResult.ucFlag == 1)//开始充电
            {
                if(ucCardStep == Step_WaitingAccResult_LCD)
                {
                    ucCardStep = Step_GetAccResult_LCD;
                    uiTimeCounter = 0;
                }
                else if (ucCardStep == Step_WaitingStartChargeResult_LCD)
                {
                    ucCardStep = Step_GetStartChargeResult_LCD;
                    uiTimeCounter = 0;
                }
            }
            else if(stCardResult.ucFlag == 2)//结束充电
            {
                ucCardStep = Step_GetStopChargeResult_LCD;
                uiTimeCounter = 0;
            }
        }
        break;
    }
    case AddrType_UpdateResult:
    {
        ParseCenterUdiskResult(RecvCenterMap);
        break;
    }
    case AddrType_InVinApplyStartCharge_Result:
    {
        ParseCenterVinInResult(RecvCenterMap,1);
        ucCardStep = Step_GetStartChargeResult_LCD;
        uiTimeCounter = 0;
        break;
    }
    case AddrType_Change_ChargeGunGroup_Info:
    {
        pProtocol->InitTermNameMapShow();
        pProtocol->InitTermNameMapMulti();
        break;
    }
    case AddrType_ActiveProtectQueryResult:
    case AddrType_GeneralStaticArgQueryResult:
    case AddrType_GeneralDynamicArgQueryResult:
    case AddrType_CCUQueryResult:
    {
        unsigned char ucCanID = RecvCenterMap[Addr_CanID_Comm].at(0);
        for(int i = 0 ; i < ArgDelList.count(); i++)
        {
            if(ArgDelList.at(i).ucArgCanID == ucCanID)
            {
                ArgDelList.at(i).iCounter++;
                if(ArgDelList.at(i).iMaxTime <= ArgDelList.at(i).iCounter)
                {
                    if(ArgDelList.at(i).iDevType == 1)
                    {
                        usleep(30000);
                        sendGunArg(ucNowAddr, ArgDelList.at(i).ucArgCanID);
                    }
                    else if(ArgDelList.at(i).iDevType == 2)
                    {
                        usleep(30000);
                        sendCCUArg(ucNowAddr, ArgDelList.at(i).ucArgCanID);
                    }
                    bWaitTermArgQueryFlag = FALSE;
                    ucQueryArgStep = 0;
                    ArgDelList.removeAt(i);
                    break;
                }
            }
        }
        break;
    }
    case AddrType_MakePrintTicketResult:   //执行打印小票结果
    {
        ParseTicketPrintResult(RecvCenterMap,ucNowAddr);
    }break;
    case AddrType_ApplyPrintTicket:  //申请打印小票一次
    {
        unsigned char ucCanID;
        ucCanID = RecvCenterMap[Addr_CanID_Comm].data()[0];
        TerminalStatus & stTempTerminalStatus = pDevCache->GetUpdateTerminalStatus(ucCanID);
        stTempTerminalStatus.bTicketPrint = 1;
        pDevCache->FreeUpdateTerminalStatus();
        pDevCache->SaveTerminalStatus(ucCanID);
    }break;
    default:
        break;
    }//end switch
}

//开始工作
void cLCDScreen::ProcStartWork()
{
    bWorkFlag = TRUE;
    pServer = new QTcpServer();
    pTimer = new QTimer();
    pProtocol = new cLCDScreenProtocol();
    uiTimeCounter = 0;
    ucCardStep = Step_Free_LCD;
    ucQueryArgStep = 0;
    connect(pServer, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
    connect(pTimer, SIGNAL(timeout()),this, SLOT(ProcOneSecTimeOut()));
    pTimer->start(1000);
    pSocket = NULL;
    newListen();
}

void cLCDScreen::SendCenterData(InfoMap ToCenterMap, InfoAddrType enType)
{
    emit sigToBus(ToCenterMap, enType);
}

//检测连接信号,建立连接
void cLCDScreen::ProcOneSecTimeOut()
{
    if(bWorkFlag == FALSE)
    {
        return;
    }
    CheckTimeOut();
    if(pServer->isListening())
    {
        ;
    }
    else
    {
        pServer->close();
        newListen();
    }
}

//SO库调用实现函数, 创建新实例返回
CModuleIO* CreateDevInstance()
{
    return new cLCDScreen();
}

//实例销毁
void DestroyDevInstance(CModuleIO* pModule)
{
    if(pModule){
        delete pModule;
    }
}

//  GuoCheng Add
void cLCDScreen::DisplaySetPage(const QByteArray &recvArray)
{
    unsigned char ucAddr = recvArray.at(1);
    unsigned short usDataType =  0;
    memcpy(&usDataType, recvArray.data() + 5, 2);
    ucNowAddr = ucAddr;
    switch(usDataType)
    {
    case Data_SysParam_LCD: //系统设置类数据
        sendOperateResult(ucNowAddr, Data_SysParam_LCD, Ack_Failed_LCD, 0);
        break;
    case Data_PhaseType_LCD: //三相相别设置数据
        sendOperateResult(ucNowAddr, Data_PhaseType_LCD, Ack_Failed_LCD, 0);
        break;
    case Data_SysSpecSet_LCD: //系统特殊功能设置数据
        sendOperateResult(ucNowAddr, Data_SysSpecSet_LCD, Ack_Failed_LCD, 0);
        break;
    case Data_DCSpecSet_LCD: //直流特殊功能设置类数据
        sendOperateResult(ucNowAddr, Data_DCSpecSet_LCD, Ack_Failed_LCD, 0);
        break;
    case Data_LoadLimit_LCD: //负荷约束设置类数据
        sendOperateResult(ucNowAddr, Data_LoadLimit_LCD, Ack_Failed_LCD, 0);
        break;
    case Data_PeakSet_LCD: //错峰充电设置类数据
        sendOperateResult(ucNowAddr, Data_PeakSet_LCD, Ack_Failed_LCD, 0);
        break;
    case Data_AmmeterAddr_LCD: //电表地址数据
        break;
    case Data_Passwd_LCD:   //登录密码数据
        sendOperateResult(ucNowAddr, Data_Passwd_LCD, Ack_Failed_LCD, 0);
        break;
    case Data_IOState_LCD:   //IO常开常闭状态
        sendOperateResult(ucNowAddr, Data_IOState_LCD, Ack_Failed_LCD, 0);
        break;
    case Data_2DbarCodes_LCD:   //二维码设置
        sendOperateResult(ucNowAddr, Data_2DbarCodes_LCD, Ack_Failed_LCD, 0);
        break;
    case Data_TermParam_DC_LCD: //直流终端参数设置
        sendOperateResult(ucNowAddr, Data_TermParam_DC_LCD, Ack_Failed_LCD, 0);
        break;
    case Data_CCUParam_LCD: //CCU参数设置
        sendOperateResult(ucNowAddr, Data_CCUParam_LCD, Ack_Failed_LCD, 0);
        break;
    }
}

//发送 屏幕申请结束充电, 发送到总线
void cLCDScreen::SendApplyStopCharge(unsigned char canaddr)
{
    InfoMap ToCenterMap;
    QByteArray SendData;
    SendData.append(canaddr);
    ToCenterMap.insert(Addr_CanID_Comm,SendData);
    //发送
    SendCenterData(ToCenterMap, AddrType_ScreenApplyStopCharge);
}

//发送 遥控结果
void cLCDScreen::sendCtrlStopCmdResult(unsigned char ucAddr, LocalChargeStop_LCD &stLocalCharge)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;
    pProtocol->MakeFrameCtrlStopCmdResult(pData, usLen, ucAddr, stLocalCharge);
    sendMessage(pData, usLen);
    delete pData;
}


 //解析 小票机打印结果1-打印 2-缺纸
void cLCDScreen::ParseTicketPrintResult(InfoMap CenterMap,unsigned char ucAddr)
{
    unsigned char  ucCanID;
    if(CenterMap.isEmpty())
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! CenterMap Empty");

        return ;
    }
    if(!CenterMap.contains(Addr_TicketPrint_Result))
        return;
    if(!CenterMap.contains(Addr_CanID_Comm))
        return;
    ucCanID = CenterMap[Addr_CanID_Comm].data()[0];
    if(ucCanID == stTicketDevResult.ucCanID)
    {
        stTicketDevResult.ucResult = CenterMap[Addr_TicketPrint_Result].data()[0];
        if(stTicketDevResult.ucResult == 1)
        {
            TerminalStatus & stTempTerminalStatus = pDevCache->GetUpdateTerminalStatus(ucCanID);
            stTempTerminalStatus.bTicketPrint = 2;
            pDevCache->FreeUpdateTerminalStatus();
            pDevCache->SaveTerminalStatus(ucCanID);
        }else if(stTicketDevResult.ucResult == 2)
        {
            stTicketDevResult.ucResult = 0;
            const char *temp = "缺纸";
            strncpy(stTicketDevResult.chFaultReason, temp, sizeof(stTicketDevResult.chFaultReason));
        }
        else if(stTicketDevResult.ucResult == 3)
        {
            stTicketDevResult.ucResult = 0;
            const char *temp = "请检查小票机设备连接是否正常!";
            strncpy(stTicketDevResult.chFaultReason, temp, sizeof(stTicketDevResult.chFaultReason));
        }
        else
        {
            stTicketDevResult.ucResult = 0;
            const char *temp = "打印失败！";
            strncpy(stTicketDevResult.chFaultReason, temp, sizeof(stTicketDevResult.chFaultReason));
        }
        sendTickectDevData(ucAddr);
    }
}
//发送 打印小票
void cLCDScreen::sendTickectDevStart(unsigned char canid)
{
    InfoMap ToCenterMap;
    //发送
    ToCenterMap.clear();
    ToCenterMap.insert(Addr_CanID_Comm, QByteArray( 1,canid));  //    //确定CAN地址
    SendCenterData(ToCenterMap, AddrType_MakePrintTicket);
}

void cLCDScreen::sendTickectDevData(unsigned char ucAddr)
{
    unsigned char * pData = NULL;
    unsigned short usLen = 0;

    pProtocol->MakeFrameTickectDevResult(pData, usLen, ucAddr, stTicketDevResult);
    sendMessage(pData, usLen);
    delete pData;
}



