#include "J1939GeneralProtocol.h"

cLongPackageModule::cLongPackageModule(unsigned char ucCanIDIn)
{
    bFreeFlag = TRUE;//TRUE:空闲,FALSE:有长包在处理
    bUsedFlag = FALSE;//TRUE:使用过的, FALSE:未使用过的
    bValidFlag = FALSE;//TRUE:数据有效, FALSE:数据无效
    bUpdateFlag = FALSE;//TRUE: 进行升级处理, FALSE: 进行普通长包处理
    ucCanID = ucCanIDIn;
    pLongPackage = (FrameLongPackage *)new unsigned char[sizeof(FrameLongPackage)];
    pLongPackage->ucTermID = ucCanIDIn;
    pLongPackage->pData = NULL;
    ucCounter = 0;//计数器清零

    //共用变量
    pDataOffset = NULL;//长包数据偏移
    ucPackNext = 1;//下一个要传输的数据包编号(发送排序)
    ucPackLast = 0;//上一个已经传输的数据包的编号(接收校验)
    //正常长包
    usDataLength = 0;//长包数据长度
    ucPackTotalNum = 0;//数据总包数
    ucPackTransNum = 0;//数据包已经传输数(已经发送/接收, 总数)
    ucPackAllowed = 0;//允许发送或接收的数据包数
    //升级长包
    uiDataLength_U = 0;//程序总字节数----升级用
    uiLongPackLength_U = 0;//长包数据长度
    uiDataTransLength_U = 0;//程序已经传输字节数
    pDataOffset_old = NULL;//长包原始位置
    uiLongPackNum_U = 0; //分割的长包数量
    uiLongPackTransNum_U = 0;//已经传输的长包数量
    usPackTransNum_U = 0;//数据包已经传输数(已经发送/接收, 总数)----升级用
    usPackAllowed_U = 0;//允许发送或接收的数据包数----升级用
    ucReSendCount = 0;//单包接收失败,重发计数

}

cLongPackageModule::~cLongPackageModule()
{
//    int id = this->ucCanID;
    pDataOffset = NULL;
    if( pLongPackage != NULL)
    {
        if(pLongPackage->pData != NULL)
        {
            delete pLongPackage->pData;
            pLongPackage->pData = NULL;
        }
        delete pLongPackage;
    }
    pLongPackage = NULL;

}

void cLongPackageModule::Clear()
{

    bFreeFlag = TRUE; //TRUE:空闲,FALSE:有长包在处理
    bUsedFlag = FALSE;
    bValidFlag = FALSE;

    ucCounter = 0;//计数器清零

    //正常长包
    pDataOffset = NULL;//长包数据偏移
    usDataLength = 0;//长包数据长度
    ucPackTotalNum = 0;//数据总包数
    ucPackTransNum = 0;//数据包已经传输数(已经发送/接收)
    ucPackAllowed = 0;//允许发送或接收的数据包数
    ucPackNext = 0;//下一个要传输的数据包编号
    //升级长包
    uiDataLength_U = 0;//程序总字节数----升级用
    usPackTotalNum_U = 0;//程序总包数----升级用
    usPackTransNum_U = 0;//数据包已经传输数(已经发送/接收, 总数)----升级用
    usPackAllowed_U = 0;//允许发送或接收的数据包数----升级用

    memset(pLongPackage->ucPGN, 0x00, sizeof(pLongPackage->ucPGN));
    pLongPackage->ucTermID = 0;
    pLongPackage->uiDataLength = 0;

    if(this->pLongPackage->pData != NULL)
    {
        delete this->pLongPackage->pData;
    }
    this->pLongPackage->pData = NULL;
}

cJ1939GeneralProtocol::cJ1939GeneralProtocol()
{
    pTerminalSendList = new QList <can_frame *> ;//终端CAN帧发送列表
    pSendListMutex = new QMutex;//发送列表操作锁
    pModuleMapMutex = new QMutex(); //长包模块操作锁

}

cJ1939GeneralProtocol::~cJ1939GeneralProtocol()
{
    delete pTerminalSendList; //终端CAN帧发送列表
    delete pSendListMutex; //发送列表操作锁
    delete pModuleMapMutex; //长包模块操作锁
}

void cJ1939GeneralProtocol::ParseTelecontrol(InfoMap CenterMap, unsigned char ucTermID)
{
    unsigned int uiChargeCmdType = 0;
    InfoMap::iterator itTarget;
    //解析Map内容
    for(itTarget = CenterMap.begin(); itTarget != CenterMap.end(); itTarget++)
    {
        switch (itTarget.key())
        {
        //解析遥控指令
        case Addr_ChargeCmd_Ctrl:
        {
            uiChargeCmdType = itTarget.value().at(0);
            break;
        }
        default:
            break;
        }
    }

    if(uiChargeCmdType == CHARGE_CMD_TYPE_ADJUST_CHARGE)
    {
        ParseLimitChargeCurrent(CenterMap,ucTermID);
    }
//    ParseConrolEMSBreaker(CenterMap,ucTermID);

    //发送遥控指令
    if(uiChargeCmdType != 0)
    {
        SendCmd(uiChargeCmdType, ucTermID);
    }
    else
    {
    }
}

//解析遥调指令<----控制中心
//void cJ1939GeneralProtocol::ParseTeleadjust(InfoMap CenterMap, unsigned char ucTermID)
//{
//    ucTermID = 0;
//    CenterMap.clear();
//}

//解析刷卡账户列表<----控制中心
void cJ1939GeneralProtocol::ParseAccountInfoList(InfoMap CenterMap, unsigned char ucTermID)
{
    AccountInfo stAccount;
    PolicyInfo stPolicy;
    unsigned char ucCount = 0;
    if(CenterMap[Addr_CardAccountType].at(0) == 03)//账户列表
    {
        memcpy((char *)&stAccount, CenterMap[Addr_CardAccountList].data(), CenterMap[Addr_CardAccountList].length());
        SendFrameAccountInfo(ucTermID, stAccount);
    }
    else if(CenterMap[Addr_CardAccountType].at(0) == 04)//计费策略
    {
        ucCount = CenterMap[Addr_CardPolicy].at(0);
        for(unsigned char i = 0 ; i < ucCount; i++)
        {
            memcpy((char *)&stPolicy, CenterMap[Addr_CardPolicy].data() + 1 + i * sizeof(stPolicy), sizeof(stPolicy));
            SendFramePricePolicy(ucTermID, stPolicy, 1, i);
            SendFramePricePolicy(ucTermID, stPolicy, 2, i);
            SendFramePricePolicy(ucTermID, stPolicy, 3, i);
        }
    }
    else if(CenterMap[Addr_ScanCode_Type].at(0) == 03)//账户列表(扫码)
    {
        memcpy((char *)&stAccount, CenterMap[Addr_ScanCode_Type].data(), CenterMap[Addr_ScanCode_Type].length());
        SendFrameAccountInfo(ucTermID, stAccount);
    }
    else if(CenterMap[Addr_ScanCode_Type].at(0) == 04)//计费策略(扫码)
    {
        ucCount = CenterMap[Addr_CardPolicy].at(0);
        for(unsigned char i = 0 ; i < ucCount; i++)
        {
            memcpy((char *)&stPolicy, CenterMap[Addr_CardPolicy].data() + 1 + i * sizeof(stPolicy), sizeof(stPolicy));
            SendFramePricePolicy(ucTermID, stPolicy, 1, i);
            SendFramePricePolicy(ucTermID, stPolicy, 2, i);
            SendFramePricePolicy(ucTermID, stPolicy, 3, i);
        }
    }
}

//解析刷卡内部返回结果<----控制中心
//0x01: 失败, 0xFF, 成功
void cJ1939GeneralProtocol::ParseCardInResult(InfoMap CenterMap, unsigned char ucTermID)
{
    unsigned char ucResult = 0;
    if(CenterMap.contains(Addr_InApplyStartCharge_Result))//为开始充电结果
    {
        ucResult = (unsigned char)CenterMap[Addr_InApplyStartCharge_Result].at(0);
    }
    else if(CenterMap.contains(Addr_InApplyStopCharge_Result))//为结束充电结果
    {
        ucResult = (unsigned char)CenterMap[Addr_InApplyStopCharge_Result].at(0);
    }
    if(ucResult != 0xFF)
    {
        ucResult = 0x0F;//未找到充电业务信息(若内部校验不成功,返回该信息)
        SendFrameCardResult(ucTermID, ucResult);
    }
}

//解析刷卡内部返回结果<----控制中心
//0x01: 失败, 0xFF, 成功
void cJ1939GeneralProtocol::ParseCardOutResult(InfoMap CenterMap, unsigned char ucTermID)
{
    unsigned char ucResult = 0;
    if(CenterMap.contains(Addr_CardApplyCharge_Result))//为开始充电结果
    {
        ucResult = (unsigned char)CenterMap[Addr_CardApplyCharge_Result].at(0);
    }
    else if(CenterMap.contains(Addr_CardStopCharge_Result))//为结束充电结果
    {
        ucResult = (unsigned char)CenterMap[Addr_CardStopCharge_Result].at(0);
    }
    else if(CenterMap.contains(Addr_ScanCode_StartCharge_Result))//为开始充电结果(扫码)
    {
        ucResult = (unsigned char)CenterMap[Addr_ScanCode_StartCharge_Result].at(0);
    }
    else if(CenterMap.contains(Addr_ScanCode_StopCharge_Result))//为结束充电结果(扫码)
    {
        ucResult = (unsigned char)CenterMap[Addr_ScanCode_StopCharge_Result].at(0);
    }
    SendFrameCardResult(ucTermID, ucResult);
}

void cJ1939GeneralProtocol::ParseConrolEMSBreaker(InfoMap CenterMap, unsigned char ucTermID)
{
    SendFrameConrolEMSBreaker(ucTermID);
    Q_UNUSED(CenterMap);
}


//解析限制充电电流<----控制中心
void cJ1939GeneralProtocol::ParseLimitChargeCurrent(InfoMap CenterMap, unsigned char ucTermID)
{
    float fCurrent = 0;
    if(!CenterMap.contains(Addr_AdjustCurrent_Adj))
    {
        return;
    }

    fCurrent =*((float*)CenterMap[Addr_AdjustCurrent_Adj].data());
    SendFrameAdjustCurrentVoltage(ucTermID, fCurrent, 0);
}

//解析限制运行功率<----控制中心
void cJ1939GeneralProtocol::ParseLimitPower(InfoMap CenterMap, unsigned char ucTermID)
{
    int Power = 0;
    if(!CenterMap.contains(Addr_MaxLoad))
    {
        return;
    }
    Power =*((int*)CenterMap[Addr_MaxLoad].data());
    SendFrameAdjustPower(ucTermID, Power);
}

//解析模块升级包下载完成<----控制中心
void cJ1939GeneralProtocol::ParseMoudleUpdateDir(InfoMap CenterMap, unsigned char ucTermID)
{
    if(!CenterMap.contains(Addr_Update_Upload_Param))
    {
        return;
    }
//    cLongPackageModule * pLongPackageModule = CheckModule(ucTermID);
    cLongPackageModule * pLongPackageModule = new cLongPackageModule(ucTermID);
    QByteArray updatePath = CenterMap[Addr_Update_Upload_Param];
    PrepareUpdate(updatePath.constData(), pLongPackageModule, ucTermID);
    pModuleSendMapMutex->lock();
    pModuleMapToSend->insert(ucTermID, pLongPackageModule);
    pModuleSendMapMutex->unlock();
}

//pCanFrame CAN组帧后存储位置,ucSa 源地址,ucPf  PGN类型,ucPs 目的地址,ucPriority 优先级,ucLength 数据长度
bool cJ1939GeneralProtocol::SetFrameHead(can_frame *pCanFrame, unsigned char ucSa, unsigned char ucPf, unsigned char ucPs,unsigned char ucPriority, unsigned char ucLength)
{
    FrameHead stFrameHead;
    stFrameHead.ucSa = ucSa;//终端can地址
    stFrameHead.ucPs = ucPs;
    stFrameHead.ucPf = ucPf;
    stFrameHead.stBitFlag.ucDp = 0;
    stFrameHead.stBitFlag.ucReserved = 0;
    if(ucPriority > 7)//优先级判断
    {
        return FALSE;
    }
    stFrameHead.stBitFlag.ucPriority = ucPriority;
    stFrameHead.stBitFlag.ucUnuse = 5;//扩展帧
    pCanFrame->can_dlc = ucLength;
    memcpy(&pCanFrame->can_id, &stFrameHead,sizeof(pCanFrame->can_id));
    return TRUE;
}

void cJ1939GeneralProtocol::SendFrame()
{
    emit sigSendCanData(pTerminalSendList, pSendListMutex);
}

//判断是否有帧要发送, 有:返回TRUE, 无:返回FALSE
bool cJ1939GeneralProtocol::HasFrameToSend()
{
    bool bRet = FALSE;
    HasLongFrameToSend();
    pSendListMutex->lock();
    bRet = !pTerminalSendList->isEmpty();
    pSendListMutex->unlock();
    return bRet;
}

//是否有帧需要处理(目前为长帧)
bool cJ1939GeneralProtocol::HasFrameToDeal()
{
    if(HasLongFrameToDeal() == TRUE)
    {
        return TRUE;
    }
    return false;
}

//是否有长帧需要处理
bool cJ1939GeneralProtocol::HasLongFrameToDeal()
{
    bool bRET = FALSE;
    pModuleMapMutex->lock();
    if(pModuleMap->isEmpty())
    {
        pModuleMapMutex->unlock();
        return FALSE;
    }
    ModuleMap::iterator it;
    for(it = pModuleMap->begin(); it != pModuleMap->end(); it++)
    {
        //不处于工作状态
        if(it.value()->bFreeFlag == TRUE)
        {
            bRET =  TRUE;
        }
        it.value()->ucCounter++;
        //超时,置相关标志位
//        if(it.value()->ucCounter > TI_LongPackageOverTime && (it.value()->bUpdateFlag != true
//                                                              || it.value()->ucCounter > TI_LongPackageUpdataOverTime))
        if(it.value()->ucCounter > TI_LongPackageOverTime)
        {
            it.value()->bFreeFlag = TRUE;
            it.value()->bUsedFlag = TRUE;
            it.value()->bValidFlag = FALSE;
            bRET =  TRUE;
        }
    }
    pModuleMapMutex->unlock();
    return bRET;
}

//是否有长帧要发送
bool cJ1939GeneralProtocol::HasLongFrameToSend()
{
    bool bRet = FALSE;
    unsigned char ucKey = 0;
    ModuleMap::iterator it;
    pModuleSendMapMutex->lock();
    pModuleMapMutex->lock();
    bRet = !pModuleMapToSend->isEmpty();
    for(it = pModuleMapToSend->begin(); it != pModuleMapToSend->end(); it++)
    {
        ucKey = it.key();
        if(pModuleMap->contains(ucKey))
        {
            continue;
        }
        else
        {
            pModuleMap->insert(ucKey, it.value());
            if(it.value()->bUpdateFlag == TRUE)
            {
//                SendFrameUpdateApply(it.value());//此处屏蔽   不要打开 zjq
            }
            else
            {
                SendFrameApplySend(it.value());
            }
            pModuleMapToSend->remove(ucKey);
//            it--;
            if(pModuleMapToSend->isEmpty())
            {
                break;
            }
            else
            {
                it = pModuleMap->begin();
            }
        }
    }
    pModuleMapMutex->unlock();
    pModuleSendMapMutex->unlock();
    return bRet;
}

//帧处理
void cJ1939GeneralProtocol::DealFrame()
{
    CheckModuleFree();
}

//确认对应指令类型----解析命令返回帧
bool cJ1939GeneralProtocol::CheckCmdType(unsigned char &ucCmdType, unsigned char &ucPF)
{
    bool bParseSuccess = TRUE; //返回帧能够解析 : TRUE; 不能解析: FALSE
    switch(ucPF)
    {
    case PF_StartCharge : //开始充电
    {
        ucCmdType = CHARGE_CMD_TYPE_START_CHARGE_NOW ;
        break;
    }
    case PF_StopCharge : //终止充电
    {
        ucCmdType = CHARGE_CMD_TYPE_STOP_CHARGE ;
        break;
    }
    case PF_PauseCharge : //暂停充电
    {
        ucCmdType = CHARGE_CMD_TYPE_PAUSH_CHARGE ;
        break;
    }
    case PF_RecoverCharge : //恢复暂停充电
    {
        ucCmdType = CHARGE_CMD_TYPE_RESUME ;
        break;
    }
    case PF_LimitCharge : //限制充电
    {
        ucCmdType = CHARGE_CMD_TYPE_LIMIT_CHARGE ;
        break;
    }
    case PF_ResetCharge : //复位充电
    {
        ucCmdType = CHARGE_CMD_TYPE_RESET ;
        break;
    }
    case PF_StartDischarge : //开始放电
    {
        ucCmdType = CHARGE_CMD_TYPE_START_DISCHARGE ;
        break;
    }
    case PF_StopDischarge : //结束放电
    {
        ucCmdType = CHARGE_CMD_TYPE_STOP_DISCHARGE ;
        break;
    }
    default :
        bParseSuccess = FALSE; //其他指令, 解析不成功
        ucCmdType = 0;
        break;
        //记录日志
    }
    return bParseSuccess;
}

//确认指令返回结果----解析命令返回帧
void cJ1939GeneralProtocol::CheckCmdAck(unsigned char &ucCtrlAck, unsigned char &ucFrameACK)
{
    switch(ucFrameACK)
    {
    case Ack_CmdAck : //终端响应成功
    {
        ucCtrlAck = CMD_ACK_TYPE_SUCCESS ;
        break;
    }
    case Ack_CmdNAck : //终端响应失败
    {
        ucCtrlAck = CMD_ACK_TYPE_FAIL ;
        break;
    }
    case Ack_CmdCallRefuse : //终端拒绝访问
    {
        ucCtrlAck = CMD_ACK_TYPE_REJECT ;
        break;
    }
    case Ack_CmdCanNotAck : //终端无法响应
    {
        ucCtrlAck = CMD_ACK_TYPE_CANNOT_ACK ;
        break;
    }
    case Ack_CmdPDUFault:
    case Ack_CmdNoPowerAccept:
    case Ack_CmdScram:
    case Ack_CmdLinkBreak:
    case Ack_CmdLastProtectNoEnd:
    case Ack_CmdLaskBMSOutTime:
    case Ack_CmdChargerInUsed:
        ucCtrlAck = ucFrameACK+37;
        break;
    default: //其他均解析为终端响应失败
    {
        ucCtrlAck = CMD_ACK_TYPE_FAIL ;
        break;
    }
    }
}

//确认连接状态
unsigned char cJ1939GeneralProtocol::CheckLinkState(unsigned char ucLinkStateIn)
{
    return ucLinkStateIn;
}

//检查长包模块图, 若对应CAN地址没有在对应的长包处理模块中,则创建, 返回对应模块指针
cLongPackageModule * cJ1939GeneralProtocol::CheckModule(unsigned char ucCanID)
{
    ModuleMap ::iterator it, itbak;
    pModuleMapMutex->lock();
    //删除Map中无效空间
    if(!pModuleMap->isEmpty())
    {
        it = pModuleMap->begin();

        while(it != pModuleMap->end())
        {
            if(it.value() == NULL)
            {
                itbak = it;
                ++it;
                pModuleMap->erase(itbak);
                continue;
            }
            if(it.value()->pLongPackage == NULL)
            {
                itbak = it;
                ++it;
                delete it.value();
                pModuleMap->erase(itbak);
            }
            ++it;
        }

    }

    //找到图中有对应CAN地址
    if(pModuleMap->contains(ucCanID))
    {
        it = pModuleMap->find(ucCanID);
        //该长包处理模块使用过,则删除该记录并根据有效性决定是否解析
        if((it.value()->bUsedFlag == TRUE))
        {

            //数据有效,可以解析
            if(it.value()->bValidFlag == TRUE)
            {
                ParseLongFrame(it.value()->pLongPackage);
            }
            if(it.value() != NULL)
            {
                delete it.value();
                it.value() = NULL;
                pModuleMap->remove(ucCanID);
                pModuleMap->erase(it);
            }
        }
    }
    //图中没有对应CAN地址
    if(!pModuleMap->contains(ucCanID))
    {
        //若该CAN地址有要发送的长帧, 则返回NULL, 不分配新的CAN地址
        if(CheckLongPackSendMap(ucCanID))
        {
            pModuleMapMutex->unlock();
            return NULL;
        }
        else
        {
            cLongPackageModule * pNewModule = new cLongPackageModule(ucCanID);
            pModuleMap->insert(ucCanID, pNewModule);
        }
    }

    it = pModuleMap->find(ucCanID);
    pModuleMapMutex->unlock();
    return it.value();
}

//检查长包是否空闲, 若空闲, 则处理长包并删除
void cJ1939GeneralProtocol::CheckModuleFree()
{
    int count = 0;

    pModuleMapMutex->lock();
    ModuleMap :: iterator it, itbak;
    if(pModuleMap->isEmpty())
    {
        pModuleMapMutex->unlock();
        return;
    }

    for(it = pModuleMap->begin(); it != pModuleMap->end(); ++it)
    {
        if(it.value() == NULL)
        {
            continue;
        }
        //对于可以解析帧, 则立即处理,回收资源
        if((it.value()->bFreeFlag == TRUE) && (it.value()->bUsedFlag == TRUE) && (it.value()->bUpdateFlag == FALSE))// && ((it.value()->bValidFlag == TRUE)))
        {
            //可删 begin
            if(it.value()->bValidFlag == TRUE)
            {
                ParseLongFrame(it.value()->pLongPackage);
            }
            //可删 end
            itbak = it;
            ++it;
            delete itbak.value();
            itbak.value() = NULL;
//            pModuleMap->remove(it.key());
            pModuleMap->erase(itbak);
        }

    }
    count++;
    pModuleMapMutex->unlock();
}

//检查终端是否有长帧要发送
bool cJ1939GeneralProtocol::CheckLongPackSendMap(unsigned char ucCanID)
{
    bool bRet = FALSE;
    pModuleSendMapMutex->lock();

    if(pModuleMapToSend->contains(ucCanID))
    {
        bRet = TRUE;
    }
    pModuleSendMapMutex->unlock();
    return bRet;
}

//升级准备处理
void cJ1939GeneralProtocol::PrepareUpdate(const char *pProPath, cLongPackageModule *pModule, unsigned char ucCanID)
{
    CanMapNode newNode;
    QByteArray tempArray;
    pModule->bUpdateFlag = TRUE;
    if(!GetUpdateProgramData(pProPath, pModule))
    {
        pModule->bFreeFlag = TRUE;
        pModule->bUsedFlag = TRUE;
        pModule->bValidFlag = FALSE;
        char tempch = 0x02;
        tempArray.append(char(tempch));
        //插入升级失败结果
        InsertCanInfoMapLine((char *)&tempch, sizeof(tempch), Addr_CANUpdateResult, newNode.stCanMap);
        //插入CAN地址
        InsertCanInfoMapLine((char *)&ucCanID, sizeof(ucCanID), Addr_CanID_Comm, newNode.stCanMap);

        newNode.enType = AddrType_UpdateResult_Dev;
        //发送升级失败
        SendCenterData(newNode.enType, newNode.stCanMap);
    }
    else
    {
        pModule->bFreeFlag = FALSE;
        pModule->bUsedFlag = FALSE;
        pModule->bValidFlag = FALSE;
        //发送升级请求
        SendFrameUpdateRequest(ucCanID);
    }
}
///
/// \brief cJ1939GeneralProtocol::MakeFrameAck
/// \param pCanFrame
/// \param ucCanID
///
void cJ1939GeneralProtocol::MakeFrameAck(can_frame *pCanFrame, unsigned char ucCanID,unsigned char result)
{
    SetFrameHead(pCanFrame, ID_DefaultControlCenterCanID, PF_CtrlCmdAck, ucCanID, PriorityDefault, DL_CtrlCmdAck);
    memset(pCanFrame->data,0x0,8);
    pCanFrame->data[0] = (__u8)result;
    pCanFrame->data[2] = 0xFF;
    pCanFrame->data[3] = 0xFF;
    pCanFrame->data[4] = 0xFF;
    pCanFrame->data[6] = 0x49;//帧ID,充电模式和充电枪号
}

//生成长帧
/// \brief cJ1939GeneralProtocol::MakeFrameLongPackage
/// \param pLongFrame : 所用长帧
/// \param ucTermID : 目的终端地址
/// \param ucPF : PGN编号
/// \param uiLength : 长帧长度
void cJ1939GeneralProtocol::MakeFrameLongPackage(FrameLongPackage * &pLongFrame, unsigned char ucTermID, unsigned char ucPF, unsigned int uiLength)
{
    pLongFrame->ucPGN[0] = 0x00;
    pLongFrame->ucPGN[1] = ucPF;
    pLongFrame->ucPGN[2] = 0x00;
    pLongFrame->pData = new unsigned char [uiLength];
    pLongFrame->uiDataLength = uiLength;
    pLongFrame->ucTermID = ucTermID;
}

//生成--版本确认帧
void cJ1939GeneralProtocol::MakeFrameProVerAck(can_frame *pCanFrame, unsigned char * pProVer, unsigned char ucCanID)
{
    pProVer[3] = 0xAA;
    SetFrameHead(pCanFrame, ID_DefaultControlCenterCanID, PF_TimeSync, ucCanID, PriorityDefault, DL_ProVerInfo);
    memcpy(pCanFrame->data,pProVer, 4);
}

//生成--对时帧
void cJ1939GeneralProtocol::MakeFrameTimeSync(can_frame *pCanFrame, unsigned char ucCanID)
{
    QDateTime NowDateTime = QDateTime::currentDateTime();
    QDate NowDate = NowDateTime.date();
    QTime NowTime = NowDateTime.time();
    unsigned char ucTime[7];
    unsigned short usYear;
    FrameTimeSync stFrame;

    ucTime[0] = NowTime.second()/10 *16 + NowTime.second()%10;
    ucTime[1] = NowTime.minute()/10 *16 + NowTime.minute()%10;
    ucTime[2] = NowTime.hour()/10 *16 + NowTime.hour()%10;
    ucTime[3] = NowDate.day()/10 *16 + NowDate.day()%10;
    ucTime[4] = NowDate.month()/10 *16 + NowDate.month()%10;
    usYear = (NowDate.year() - 2000)/10 *16 + (NowDate.year() - 2000)%10 + 0x2000;
    ucTime[5] = usYear&0xFF;
    ucTime[6] = usYear >> 8;
    memcpy((unsigned char *)&stFrame, (unsigned char *)&ucTime, 7);

    SetFrameHead(pCanFrame, ID_DefaultControlCenterCanID, PF_TimeSync, ucCanID, PriorityDefault, DL_TimeSync);
    memcpy(pCanFrame->data,(unsigned char *) &stFrame,sizeof(stFrame));
}

//生成--请求PGN帧
void cJ1939GeneralProtocol::MakeFrameAppyPGN(can_frame *pCanFrame, unsigned char ucCanID, unsigned char ucPf)
{
    FrameAppyPGN stFrame;

    stFrame.ucDestPGN[0] = 0x00;
    stFrame.ucDestPGN[1] = ucPf;
    stFrame.ucDestPGN[2] = 0x00;

    SetFrameHead(pCanFrame, ID_DefaultControlCenterCanID, PF_ApplyPGN, ucCanID, PriorityDefault, DL_ApplyPGN);
    memcpy(pCanFrame->data,(unsigned char *) &stFrame,sizeof(stFrame));
}

//生成--参数查询帧
void cJ1939GeneralProtocol::MakeFrameAppyArg(can_frame *pCanFrame, unsigned char ucCanID, unsigned char ucInnerID, unsigned char ucType, unsigned char ucPf)
{
    FrameApplyArg stFrame;
    stFrame.ucInnerID = ucInnerID;
    stFrame.ucType = ucType;
    stFrame.ucPGN[0] = 0x00;
    stFrame.ucPGN[1] = ucPf;
    stFrame.ucPGN[2] = 0x00;
    memset(stFrame.ucReserved, 0x00, 3);

    SetFrameHead(pCanFrame, ID_DefaultControlCenterCanID, PF_ApplyArg, ucCanID, PriorityDefault, DL_ApplyArg);
    memcpy(pCanFrame->data,(unsigned char *) &stFrame,sizeof(stFrame));
}

//生成--开光伏帧
void cJ1939GeneralProtocol::MakeFrameStartPV(can_frame *pCanFrame, unsigned char ucCanID, unsigned char moduleID)
{
    FramePVPowerON stFrame;
    stFrame.moduleID = moduleID;
    stFrame.usVaildFlag = 0x55;

    SetFrameHead(pCanFrame, ID_DefaultControlCenterCanID, PF_PVSwitch, ucCanID, PriorityDefault, DL_PVSwitch);
    memcpy(pCanFrame->data,(unsigned char *) &stFrame,sizeof(stFrame));
}

//生成--关光伏帧
void cJ1939GeneralProtocol::MakeFrameStopPV(can_frame *pCanFrame, unsigned char ucCanID, unsigned char moduleID)
{
    FramePVPowerON stFrame;
    stFrame.moduleID = moduleID;
    stFrame.usVaildFlag = 0xAA;

    SetFrameHead(pCanFrame, ID_DefaultControlCenterCanID, PF_PVSwitch, ucCanID, PriorityDefault, DL_PVSwitch);
    memcpy(pCanFrame->data,(unsigned char *) &stFrame,sizeof(stFrame));
}

//生成--开始充电帧
void cJ1939GeneralProtocol::MakeFrameStartCharge(can_frame *pCanFrame, unsigned char ucCanID)
{
    FrameStartCharge stFrame;
    stFrame.usVaildFlag = 0x55;

    SetFrameHead(pCanFrame, ID_DefaultControlCenterCanID, PF_StartCharge, ucCanID, PriorityDefault, DL_StartCharge);
    memcpy(pCanFrame->data,(unsigned char *) &stFrame,sizeof(stFrame));
}

//生成--结束充电帧
void cJ1939GeneralProtocol::MakeFrameStopCharge(can_frame *pCanFrame, unsigned char ucCanID)
{
    FrameStopCharge stFrame;
    stFrame.usVaildFlag = 0x55;

    SetFrameHead(pCanFrame, ID_DefaultControlCenterCanID, PF_StopCharge, ucCanID, PriorityDefault, DL_StopCharge);
    memcpy(pCanFrame->data,(unsigned char *) &stFrame,sizeof(stFrame));
}

//生成--开始放电帧
void cJ1939GeneralProtocol::MakeFrameStartDisCharge(can_frame *pCanFrame, unsigned char ucCanID)
{
    FrameStartCharge stFrame;
    stFrame.usVaildFlag = 0x55;

    SetFrameHead(pCanFrame, ID_DefaultControlCenterCanID, PF_StartDischarge, ucCanID, PriorityDefault, DL_StartDischarge);
    memcpy(pCanFrame->data,(unsigned char *) &stFrame,sizeof(stFrame));
}

//生成--结束放电帧
void cJ1939GeneralProtocol::MakeFrameStopDisCharge(can_frame *pCanFrame, unsigned char ucCanID)
{
    FrameStopCharge stFrame;
    stFrame.usVaildFlag = 0x55;

    SetFrameHead(pCanFrame, ID_DefaultControlCenterCanID, PF_StopDischarge, ucCanID, PriorityDefault, DL_StopDischarge);
    memcpy(pCanFrame->data,(unsigned char *) &stFrame,sizeof(stFrame));
}

//生成--暂停充电帧
void cJ1939GeneralProtocol::MakeFramePauseCharge(can_frame *pCanFrame, unsigned char ucCanID)
{
    FrameStopCharge stFrame;
    stFrame.usVaildFlag = 0x55;

    SetFrameHead(pCanFrame, ID_DefaultControlCenterCanID, PF_PauseCharge, ucCanID, PriorityDefault, DL_PauseCharge);
    memcpy(pCanFrame->data,(unsigned char *) &stFrame,sizeof(stFrame));
}

//生成--恢复充电帧
void cJ1939GeneralProtocol::MakeFrameRecoverCharge(can_frame *pCanFrame, unsigned char ucCanID)
{
    FrameStopCharge stFrame;
    stFrame.usVaildFlag = 0x55;

    SetFrameHead(pCanFrame, ID_DefaultControlCenterCanID, PF_RecoverCharge, ucCanID, PriorityDefault, DL_RecoverCharge);
    memcpy(pCanFrame->data,(unsigned char *) &stFrame,sizeof(stFrame));
}

void cJ1939GeneralProtocol::MakeFrameConrolEMSBreaker(can_frame *pCanFrame,char index)
{
    FrameEMSControl stFrame;
    memset(&stFrame,0x0,sizeof(FrameEMSControl));

    stFrame.controlCMD=0xFF&(1<<index);
    SetFrameHead(pCanFrame, ID_DefaultControlCenterCanID, 0x10, 99, PriorityDefault, DL_ChargeParamSet);
    memcpy(pCanFrame->data,(unsigned char *) &stFrame,sizeof(stFrame));
}
//生成--调整充电电压电流帧(参数设置帧)
void cJ1939GeneralProtocol::MakeFrameAdjustCurrentVoltage(can_frame *pCanFrame, unsigned char ucCanID, float fCur, float fVol)
{
    FrameChargeParamSet stFrame;

    stFrame.sMaxChargeCurrent = short((400 - fCur)* 10);
    stFrame.sMaxChargeVoltage = short((fVol)* 10);
    SetFrameHead(pCanFrame, ID_DefaultControlCenterCanID, PF_ChargeParamSet, ucCanID, PriorityDefault, DL_ChargeParamSet);
    memcpy(pCanFrame->data,(unsigned char *) &stFrame,sizeof(stFrame));
}

//生成--调整充电电压电流帧(参数设置帧)
void cJ1939GeneralProtocol::MakeFrameAdjustPower(can_frame *pCanFrame, unsigned char ucCanID, float fPower)
{
    FrameChargePowerSet stFrame;
    memset((char*)&stFrame,0x0,sizeof(stFrame));

    stFrame.sMaxChargePower = short((fPower)* 10);
    SetFrameHead(pCanFrame, ID_DefaultControlCenterCanID, PF_ModulePowerControl, ucCanID, PriorityDefault, DL_DataTran);
    memcpy(pCanFrame->data,(unsigned char *) &stFrame,sizeof(stFrame));
}

//生成--账户信息帧
void cJ1939GeneralProtocol::MakeFrameAccountInfo(can_frame *pCanFrame, unsigned char ucCanID, AccountInfo &stInfo)
{
    FrameAccountInfo stFrame;
    stFrame.ucAccType = stInfo.stAccount[0].ucType;
    stFrame.uiAccBalance = stInfo.stAccount[0].uiValue;
    SetFrameHead(pCanFrame, ID_DefaultControlCenterCanID, PF_AccBAckInfo, ucCanID, PrioritySeven, DL_AccBAckInfoMoney);
    memcpy(pCanFrame->data, (unsigned char *)&stFrame, sizeof(stFrame));
}

//生成--计费策略帧, ucType: 1, 开始时间; 2, 结束时间
void cJ1939GeneralProtocol::MakeFramePricePolicy(can_frame *pCanFrame, unsigned char ucCanID, PolicyInfo &stInfo, unsigned char ucType, unsigned char ucNum)
{
    FrameBillingPolicyTime stTime;
    FrameBillingPolicyPrice stPrice;
    QDate DateTime = QDate::currentDate();
    unsigned char ucDate[6];
    unsigned short usYear;
    //计费策略时间
    if((ucType == 0x01)||(ucType == 0x02))//开始时间or结束时间
    {
        stTime.stByte1.ucCmdType = ucType;
        stTime.stByte1.ucPriceTactic = ucNum;
        if(ucType == 0x01)
        {
            ucDate[0] = stInfo.ucStartM/10 *16 + stInfo.ucStartM%10;
            ucDate[1] = stInfo.ucStartH/10 *16 + stInfo.ucStartH%10;;
        }
        else
        {
            ucDate[0] = stInfo.ucStopM/10 *16 + stInfo.ucStopM%10;
            ucDate[1] = stInfo.ucStopH/10 *16 + stInfo.ucStopH%10;
        }
        ucDate[2] = DateTime.day()/10 *16 + DateTime.day()%10;
        ucDate[3] = DateTime.month()/10 *16 + DateTime.month()%10;
        usYear = (DateTime.year() - 2000)/10 *16 + (DateTime.year() - 2000)%10 + 0x2000;
        ucDate[4] = usYear&0xFF;
        ucDate[5] = usYear >> 8;
        //        CharToBCD((unsigned char *)&ucDate, 6, (unsigned char *)&(stTime.ucMin));
        memcpy(((unsigned char *)&stTime) + 1, (unsigned char *)&ucDate, 6);
        SetFrameHead(pCanFrame, ID_DefaultControlCenterCanID, PF_BillingPolicy, ucCanID, PrioritySeven, DL_BillingPolicyTime);
        memcpy(pCanFrame->data, (unsigned char *)&stTime, sizeof(stTime));
    }
    else if(ucType == 0x03)//金额
    {
        stPrice.stByte1.ucCmdType = ucType;
        stPrice.stByte1.ucPriceTactic = ucNum;
        stPrice.usEnergyPrice = stInfo.usEnergyPrice;
        stPrice.usServicePrice = stInfo.usServicePrice;
        SetFrameHead(pCanFrame, ID_DefaultControlCenterCanID, PF_BillingPolicy, ucCanID, PrioritySeven, DL_BillingPolicyPrice);
        memcpy(pCanFrame->data, (unsigned char *)&stPrice, sizeof(stPrice));
    }
}

//生成--刷卡结果帧
void cJ1939GeneralProtocol::MakeFrameCardResult(can_frame *pCanFrame, unsigned char ucCanID, unsigned char ucResult)
{
    FrameAbnormalInfo stFrame;
    stFrame.ucAccType = 0x00;
//    stFrame.usAbInfo = htons((unsigned short)ucResult);
    stFrame.usAbInfo = ucResult;
    SetFrameHead(pCanFrame, ID_DefaultControlCenterCanID, PF_AccBAckInfo, ucCanID, PrioritySeven, DL_AccBAckInfoAbnormal);
    memcpy(pCanFrame->data, (unsigned char *)&stFrame, sizeof(stFrame));
}

//生成--数据传输帧
void cJ1939GeneralProtocol::MakeFrameDataTran(can_frame *pCanFrame, cLongPackageModule * pModule)
{
    FrameDataTran stFrame;
    //实际包长
    unsigned char ucPackageLength = DL_DataTran ;
    stFrame.ucPackageId = pModule->ucPackNext;

    if(pModule->bUpdateFlag == TRUE)//发送升级长包
    {
        if(pModule->usPackTransNum_U >= pModule->usPackAllowed_U - 1)
        {
            ucPackageLength = pModule->uiLongPackLength_U - pModule->usPackTransNum_U*7 + 1;
        }
        pModule->usPackTransNum_U++;
        pModule->uiDataTransLength_U+=ucPackageLength - 1;
    }
    else    //发送普通长包
    {
        //剩余最后一包数据
        if(pModule->ucPackTransNum == pModule->ucPackTotalNum - 1)
        {
            ucPackageLength = pModule->usDataLength - pModule->ucPackTotalNum * 7 + 1;
        }
        pModule->ucPackTransNum++;
    }
    //组帧
    memcpy(stFrame.ucData, pModule->pDataOffset, ucPackageLength - 1);
    memcpy(pCanFrame->data,(unsigned char *) &stFrame, ucPackageLength);
    //数据指针偏移
    pModule->pDataOffset += (ucPackageLength - 1);
    //    for(int i = 0; i < ucPackageLength - 1; i++)
    //    {
    //    }
    //    for(int i = 0; i < ucPackageLength - 1; i++)
    //    {
    //    }
    //下一个包编号增加
    pModule->ucPackNext++;
    SetFrameHead(pCanFrame, ID_DefaultControlCenterCanID, PF_DataTran, pModule->ucCanID, PrioritySeven, ucPackageLength);
}

//生成连接管理报文
bool cJ1939GeneralProtocol::MakeFrameLinkManage(can_frame *pCanFrame, unsigned char ucCtrlManage, cLongPackageModule *pModule)
{
    FrameLinkManage stFrame;
    stFrame.ucCtrlManage = ucCtrlManage;
    pModule->ucPackLast = 0;
    switch(stFrame.ucCtrlManage)
    {
    case Link_ApplySend://请求发送
        //数据长度
        stFrame.data.stApplySend.usDataLength = pModule->usDataLength;
        //总包数
        stFrame.data.stApplySend.ucPackageTotalNum = pModule->ucPackTotalNum;
        //预留字段
        stFrame.data.stApplySend.ucReserved = 0xFF;
        //要发送的PGN
        memcpy(stFrame.data.stApplySend.ucPackagePGN, pModule->pLongPackage->ucPGN,sizeof(pModule->pLongPackage->ucPGN));
        pModule->bFreeFlag = FALSE;
        break;
    case Link_AllowSend://允许发送
        //允许发送或接收的数据包数
        stFrame.data.stAllowSend.ucPackageAllowedSendNum = pModule->ucPackAllowed;
        //下一个要传输的数据包编号
        stFrame.data.stAllowSend.ucPackageNextSendNum = pModule->ucPackNext;
        //要发送的PGN
        memcpy(stFrame.data.stAllowSend.ucPackagePGN, pModule->pLongPackage->ucPGN, sizeof(pModule->pLongPackage->ucPGN));
        //预留字段
        memset(stFrame.data.stAllowSend.ucReserved,0xFF,sizeof(stFrame.data.stAllowSend.ucReserved));
        pModule->bFreeFlag = FALSE;
        break;
    case Link_MsgEndAck://结束应答
        //总包数
        stFrame.data.stMsgEndAck.ucPackageTotalNum = pModule->ucPackTotalNum;
        //要发送的PGN
        memcpy(stFrame.data.stMsgEndAck.ucPackagePGN, pModule->pLongPackage->ucPGN, sizeof(pModule->pLongPackage->ucPGN));
        //预留字段
        stFrame.data.stMsgEndAck.ucReserved = 0xFF;
        //数据长度
        stFrame.data.stMsgEndAck.usDataLength = pModule->usDataLength;
        pModule->bFreeFlag = TRUE;
        pModule->bUsedFlag = TRUE;
        break;
    case Link_Abandon://放弃连接
        //要发送的PGN
        memcpy(stFrame.data.stLinkAbandon.ucPackagePGN, pModule->pLongPackage->ucPGN, sizeof(pModule->pLongPackage->ucPGN));
        pModule->bFreeFlag = TRUE;
        pModule->bUsedFlag = TRUE;
        break;
    default:
        return FALSE;
        break;
    }
    SetFrameHead(pCanFrame, ID_DefaultControlCenterCanID, PF_LinkManage, pModule->ucCanID, PrioritySeven, DL_LinkManage);
    memcpy(pCanFrame->data,(unsigned char *)&stFrame,DL_LinkManage);
    return TRUE;
}

//生成--升级请求帧
void cJ1939GeneralProtocol::MakeFrameUpdateRequest(can_frame *pCanFrame, unsigned char ucCanID)
{
    FrameUpdateRequest stFrame;

    stFrame.ucVaildFlag = 0x55;
    SetFrameHead(pCanFrame, ID_DefaultControlCenterCanID, PF_UpdateRequest, ucCanID, PriorityDefault, DL_UpdateRequest);
    memcpy(pCanFrame->data,(unsigned char *) &stFrame,sizeof(stFrame));
}

//生成--升级管理帧
bool cJ1939GeneralProtocol::MakeFrameUpdateManage(can_frame *pCanFrame, unsigned char ucCtrlManage, cLongPackageModule *pModule)
{
    FrameUpdateManage stFrame;
    stFrame.ucCtrlManage = ucCtrlManage;
    pModule->ucPackLast = 0;
    switch(stFrame.ucCtrlManage)
    {
    case Link_ApplySend://请求发送
        //是最后一个长包,
        if((pModule->uiDataLength_U - pModule->uiDataTransLength_U) <= UpdatePerLongPackLongth)
        {
            pModule->uiLongPackLength_U = pModule->uiDataLength_U - pModule->uiDataTransLength_U;
        }
        else
        {
            pModule->uiLongPackLength_U = UpdatePerLongPackLongth;
        }
        //数据长度
        stFrame.data.stApplySend.uiProgramLength = pModule->uiLongPackLength_U;
        //总包数
        if(pModule->uiLongPackLength_U%7 == 0)
        {
            stFrame.data.stApplySend.usPackageNum = pModule->uiLongPackLength_U/7;
        }
        else
        {
            stFrame.data.stApplySend.usPackageNum = pModule->uiLongPackLength_U/7 + 1;
        }
        //预留字段
        stFrame.data.stApplySend.ucReserved = 0xFF;
        //长包模块处理
        pModule->bFreeFlag = FALSE;
        break;
    case Link_AllowSend://允许发送
        break;
    case Link_MsgEndAck://结束应答
        //程序字节数(每阶段传输字节数)
        stFrame.data.stMsgEndAck.uiProgramLength = pModule->uiLongPackLength_U;
        //CRC校验结果
        stFrame.data.stMsgEndAck.usCRCValue = Verify_CRC16(pModule->pDataOffset_old,(pModule->pDataOffset - pModule->pDataOffset_old));
        //预留字段
        stFrame.data.stMsgEndAck.ucReserved = 0xFF;
        break;
    case Link_Abandon://放弃连接
        //要发送的PGN
        memset(stFrame.data.stLinkAbandon.ucLinkAbandon, 0xFF, sizeof(stFrame.data.stLinkAbandon.ucLinkAbandon));
        pModule->bFreeFlag = TRUE;
        pModule->bUsedFlag = TRUE;
        break;
    default:
        return FALSE;
        break;
    }
    SetFrameHead(pCanFrame, ID_DefaultControlCenterCanID, PF_UpadateManage, pModule->ucCanID, PrioritySeven, DL_LinkManage);
    memcpy(pCanFrame->data,(unsigned char *)&stFrame,DL_LinkManage);
    return TRUE;
}

//生成--升级完成帧
void cJ1939GeneralProtocol::MakeFrameProgramFinishSend(can_frame *pCanFrame, cLongPackageModule *pModule)
{
    FrameProgramFinishSend stFrame;

    memset(stFrame.ucReserved, 0x00, sizeof(stFrame.ucReserved));
    stFrame.uiProgramLength = pModule->uiDataLength_U;
    stFrame.usCRCValue = Verify_CRC16(pModule->pLongPackage->pData, pModule->uiDataLength_U);
    SetFrameHead(pCanFrame, ID_DefaultControlCenterCanID, PF_ProgramFinishSend, pModule->ucCanID, PriorityDefault, DL_ProgramFinishSend);
    memcpy(pCanFrame->data,(unsigned char *) &stFrame,sizeof(stFrame));
}

//发送--控制指令
void cJ1939GeneralProtocol::SendCmd(unsigned int uiChargeCmdType, unsigned char ucCanID)
{
    switch(uiChargeCmdType)
    {
    case CHARGE_CMD_TYPE_START_CHARGE_NOW : //01H 立即充电
    case CHARGE_CMD_TYPE_START_CHARGE_ECONOMIC : // 02H 经济充电
    {
        SendFrameStartCharge(ucCanID);

        switch(CheckTermType(ucCanID))
        {
        case TermType_ACSin:
            SendFrameAdjustCurrentVoltage(ucCanID, 32.0, 0);//临时方案：开始充电时放开限制
            break;
        case TermType_ACThr:
            break;
        case TermType_DC:
            SendFrameAdjustCurrentVoltage(ucCanID, 400.0, 0);//临时方案：开始充电时放开限制
            break;
        default:
            break;
        }
        break;
    }
    case CHARGE_CMD_TYPE_STOP_CHARGE : //03H 终止充电
    {
        SendFrameStopCharge(ucCanID);
        break;
    }
    case CHARGE_CMD_TYPE_START_DISCHARGE:
        SendFrameStartDisCharge(ucCanID);
        break;
    case CHARGE_CMD_TYPE_STOP_DISCHARGE:
        SendFrameStopDisCharge(ucCanID);
        break;
    case CHARGE_CMD_TYPE_PAUSH_CHARGE : // 06H 暂停充电   add by zjq  2018.1.16
        SendFramePauseCharge(ucCanID);
        break;
    case CHARGE_CMD_TYPE_RESUME : // 08H 恢复充电   add by zjq  2018.1.16
        SendFrameRecoverCharge(ucCanID);
        break;
    default:
    {
        break;
    }
    }
}

//发送--版本确认帧
void cJ1939GeneralProtocol::SendFrameProVerAck(unsigned char ucCanID, unsigned char * pVer)
{
    can_frame *pCanFrame = new(can_frame);
    MakeFrameProVerAck(pCanFrame, pVer, ucCanID);
    pSendListMutex->lock();
    pTerminalSendList->append(pCanFrame);
    pSendListMutex->unlock();
}

//发送--对时帧
void cJ1939GeneralProtocol::SendFrameTimeSync(unsigned char ucCanID)
{
    can_frame *pCanFrame = new(can_frame);
    MakeFrameTimeSync(pCanFrame, ucCanID);
    pSendListMutex->lock();
    pTerminalSendList->append(pCanFrame);
    pSendListMutex->unlock();
}

//发送--应答帧
void cJ1939GeneralProtocol::SendFrameAck(unsigned char ucCanID,unsigned char result)
{
    can_frame *pCanFrame = new(can_frame);
    MakeFrameAck(pCanFrame, ucCanID,result);
    pSendListMutex->lock();
    pTerminalSendList->append(pCanFrame);
    pSendListMutex->unlock();
}

//发送--请求PGN帧
void cJ1939GeneralProtocol::SendFrameAppyPGN(unsigned char ucCanID, unsigned char ucPf)
{
    can_frame *pCanFrame = new(can_frame);
    MakeFrameAppyPGN(pCanFrame, ucCanID, ucPf);
    pSendListMutex->lock();
    pTerminalSendList->append(pCanFrame);
    pSendListMutex->unlock();
}

//发送--请求参数帧
void cJ1939GeneralProtocol::SendFrameAppyArg(unsigned char ucCanID, unsigned char ucInnerID, unsigned char ucType, unsigned char ucPf)
{
    can_frame *pCanFrame = new(can_frame);
    MakeFrameAppyArg(pCanFrame, ucCanID, ucInnerID, ucType, ucPf);
    pSendListMutex->lock();
    pTerminalSendList->append(pCanFrame);
    pSendListMutex->unlock();
}

//发送--开光伏帧
//moduleID:模块为1~7     光伏柜断路器QF1~QF7　用地址21~27 55脱扣
void cJ1939GeneralProtocol::SendFrameStartPV(unsigned char ucCanID, unsigned char moduleID)
{
    can_frame *pCanFrame = new(can_frame);
    MakeFrameStartPV(pCanFrame, ucCanID, moduleID);
    pSendListMutex->lock();
    pTerminalSendList->append(pCanFrame);
    pSendListMutex->unlock();
}

//发送--关光伏帧
void cJ1939GeneralProtocol::SendFrameStopPV(unsigned char ucCanID, unsigned char moduleID)
{
    can_frame *pCanFrame = new(can_frame);
    MakeFrameStopPV(pCanFrame, ucCanID, moduleID);
    pSendListMutex->lock();
    pTerminalSendList->append(pCanFrame);
    pSendListMutex->unlock();
}

//发送--开始充电帧
void cJ1939GeneralProtocol::SendFrameStartCharge(unsigned char ucCanID)
{
    can_frame *pCanFrame = new(can_frame);
    MakeFrameStartCharge(pCanFrame, ucCanID);
    pSendListMutex->lock();
    pTerminalSendList->append(pCanFrame);
    pSendListMutex->unlock();
}

//发送--结束充电帧
void cJ1939GeneralProtocol::SendFrameStopCharge(unsigned char ucCanID)
{
    can_frame *pCanFrame = new(can_frame);
    MakeFrameStopCharge(pCanFrame, ucCanID);
    pSendListMutex->lock();
    pTerminalSendList->append(pCanFrame);
    pSendListMutex->unlock();
}

//发送--开始放电电帧
void cJ1939GeneralProtocol::SendFrameStartDisCharge(unsigned char ucCanID)
{
    can_frame *pCanFrame = new(can_frame);
    MakeFrameStartDisCharge(pCanFrame, ucCanID);
    pSendListMutex->lock();
    pTerminalSendList->append(pCanFrame);
    pSendListMutex->unlock();
}

//发送--结束放电帧
void cJ1939GeneralProtocol::SendFrameStopDisCharge(unsigned char ucCanID)
{
    can_frame *pCanFrame = new(can_frame);
    MakeFrameStopDisCharge(pCanFrame, ucCanID);
    pSendListMutex->lock();
    pTerminalSendList->append(pCanFrame);
    pSendListMutex->unlock();
}

//发送--恢复充电帧
void cJ1939GeneralProtocol::SendFramePauseCharge(unsigned char ucCanID)
{
    can_frame *pCanFrame = new(can_frame);
    MakeFramePauseCharge(pCanFrame, ucCanID);
    pSendListMutex->lock();
    pTerminalSendList->append(pCanFrame);
    pSendListMutex->unlock();
}

//发送--恢复充电帧
void cJ1939GeneralProtocol::SendFrameRecoverCharge(unsigned char ucCanID)
{
    can_frame *pCanFrame = new(can_frame);
    MakeFrameRecoverCharge(pCanFrame, ucCanID);
    pSendListMutex->lock();
    pTerminalSendList->append(pCanFrame);
    pSendListMutex->unlock();
}

void cJ1939GeneralProtocol::SendFrameConrolEMSBreaker(char index)
{
can_frame *pCanFrame = new(can_frame);
MakeFrameConrolEMSBreaker(pCanFrame,index);
pSendListMutex->lock();
pTerminalSendList->append(pCanFrame);
pSendListMutex->unlock();
}
//发送--调整充电电压电流帧(参数设置帧)
void cJ1939GeneralProtocol::SendFrameAdjustCurrentVoltage(unsigned char ucCanID, float fCur, float fVol)
{
    can_frame *pCanFrame = new(can_frame);
    MakeFrameAdjustCurrentVoltage(pCanFrame, ucCanID, fCur, fVol);
    pSendListMutex->lock();
    pTerminalSendList->append(pCanFrame);
    pSendListMutex->unlock();
}

//发送--调整充电电压电流帧(参数设置帧)
void cJ1939GeneralProtocol::SendFrameAdjustPower(unsigned char ucCanID, int fPower)
{
    can_frame *pCanFrame = new(can_frame);
    MakeFrameAdjustPower(pCanFrame, ucCanID, fPower);
    pSendListMutex->lock();
    pTerminalSendList->append(pCanFrame);
    pSendListMutex->unlock();
}

//生成--账户信息帧
void cJ1939GeneralProtocol::SendFrameAccountInfo(unsigned char ucCanID, AccountInfo &stInfo)
{
    can_frame *pCanFrame = new(can_frame);
    MakeFrameAccountInfo(pCanFrame, ucCanID, stInfo);
    pSendListMutex->lock();
    pTerminalSendList->append(pCanFrame);
    pSendListMutex->unlock();
}

//发送--计费策略帧
//ucType: 1, 开始时间; 2, 结束时间; 3, 计费信息
void cJ1939GeneralProtocol::SendFramePricePolicy(unsigned char ucCanID, PolicyInfo &stInfo, unsigned char ucType, unsigned char ucNum)
{
    can_frame *pCanFrame = new(can_frame);
    MakeFramePricePolicy(pCanFrame, ucCanID, stInfo, ucType, ucNum);
    pSendListMutex->lock();
    pTerminalSendList->append(pCanFrame);
    pSendListMutex->unlock();
}

//生成--异常信息帧
void cJ1939GeneralProtocol::SendFrameCardResult(unsigned char ucCanID, unsigned char ucResult)
{
    can_frame *pCanFrame = new(can_frame);
    MakeFrameCardResult(pCanFrame, ucCanID, ucResult);
    pSendListMutex->lock();
    pTerminalSendList->append(pCanFrame);
    pSendListMutex->unlock();
}

//发送--升级请求帧
void cJ1939GeneralProtocol::SendFrameUpdateRequest(unsigned char ucCanID)
{
    can_frame *pCanFrame = new(can_frame);
    MakeFrameUpdateRequest(pCanFrame, ucCanID);
    pSendListMutex->lock();
    pTerminalSendList->append(pCanFrame);
    pSendListMutex->unlock();
}

//发送数据传输帧
void cJ1939GeneralProtocol::SendFrameDataTran(cLongPackageModule * pModule)
{

    pModule->ucCounter = 0;
    can_frame *pCanFrame = new(can_frame);
    MakeFrameDataTran(pCanFrame, pModule);
    pSendListMutex->lock();
    pTerminalSendList->append(pCanFrame);
    pSendListMutex->unlock();
}

//发送发送请求报文----连接管理
void cJ1939GeneralProtocol::SendFrameApplySend(cLongPackageModule *pModule)
{
    pModule->ucCounter = 0;
    can_frame * pCanFrame = new(can_frame);
    MakeFrameLinkManage(pCanFrame, Link_ApplySend, pModule);
    pSendListMutex->lock();
    pTerminalSendList->append(pCanFrame);
    pSendListMutex->unlock();
}

//发送允许发送报文----连接管理
void cJ1939GeneralProtocol::SendFrameAllowSend(cLongPackageModule *pModule)
{
    pModule->ucCounter = 0;
    can_frame * pCanFrame = new(can_frame);
    MakeFrameLinkManage(pCanFrame, Link_AllowSend, pModule);
    pSendListMutex->lock();
    pTerminalSendList->append(pCanFrame);
    pSendListMutex->unlock();
}

//发送放弃连接报文----连接管理
void cJ1939GeneralProtocol::SendFrameLinkAbandon(cLongPackageModule *pModule)
{
    pModule->ucCounter = 0;
    can_frame * pCanFrame = new(can_frame);
    MakeFrameLinkManage(pCanFrame, Link_Abandon, pModule);
    pSendListMutex->lock();
    pTerminalSendList->append(pCanFrame);
    pSendListMutex->unlock();
}

//发送传输结束报文----连接管理
void cJ1939GeneralProtocol::SendFrameLinkMsgEndAck(cLongPackageModule *pModule)
{
    pModule->ucCounter = 0;
    can_frame * pCanFrame = new(can_frame);
    MakeFrameLinkManage(pCanFrame, Link_MsgEndAck, pModule);
    pSendListMutex->lock();
    pTerminalSendList->append(pCanFrame);
    pSendListMutex->unlock();
}

//发送升级请求报文----升级管理
void cJ1939GeneralProtocol::SendFrameUpdateApply(cLongPackageModule *pModule)
{
    pModule->ucCounter = 0;
    can_frame * pCanFrame = new(can_frame);
    MakeFrameUpdateManage(pCanFrame, Link_ApplySend, pModule);
    pSendListMutex->lock();
    pTerminalSendList->append(pCanFrame);
    pSendListMutex->unlock();
}

//发送放弃连接帧----升级管理
void cJ1939GeneralProtocol::SendFrameUpdateAbandon(cLongPackageModule *pModule)
{
    pModule->ucCounter = 0;
    can_frame * pCanFrame = new(can_frame);
    MakeFrameUpdateManage(pCanFrame, Link_Abandon, pModule);
    pSendListMutex->lock();
    pTerminalSendList->append(pCanFrame);
    pSendListMutex->unlock();
}

//发送升级传输结束报文----升级管理
void cJ1939GeneralProtocol::SendFrameUpdateMsgEndAck(cLongPackageModule *pModule)
{
    pModule->ucCounter = 0;
    can_frame * pCanFrame = new(can_frame);
    MakeFrameUpdateManage(pCanFrame, Link_MsgEndAck, pModule);
    pSendListMutex->lock();
    pTerminalSendList->append(pCanFrame);
    pSendListMutex->unlock();
}

//发送程序发送完成报文
void cJ1939GeneralProtocol::SendFrameProgramFinishSend(cLongPackageModule *pModule)
{
    pModule->ucCounter = 0;
    can_frame * pCanFrame = new(can_frame);
    MakeFrameProgramFinishSend(pCanFrame, pModule);
    pSendListMutex->lock();
    pTerminalSendList->append(pCanFrame);
    pSendListMutex->unlock();
}

//解析协议版本号
void cJ1939GeneralProtocol::ParseFrameProVer(unsigned char * pData, unsigned char ucCanID)
{
//    GetProVerEnum(pData);
    if(pData[3] == 0x00)
    {
        SendFrameProVerAck(ucCanID, pData);
    }
}

//解析卡号
void cJ1939GeneralProtocol::ParseFrameCardNum(unsigned char * pData, unsigned char ucLen, QList<CanMapNode>  &ToCenterList)
{
    CanMapNode newNode;
    unsigned char ucType = 0x01;//获取账户列表
    newNode.enType = AddrType_SingleCardApplyAccountInfo;
    //添加CAN刷卡卡号
    InsertCanInfoMapLine((char *)pData, ucLen, Addr_CardAccount, newNode.stCanMap);
    //添加类型
    InsertCanInfoMapLine((char *)&ucType, sizeof(ucType), Addr_CardAccountType, newNode.stCanMap);

    //添加到List
    ToCenterList.append(newNode);
}

//解析申请账户信息
void cJ1939GeneralProtocol::ParseFrameApplyAccount(unsigned char * pData, QList<CanMapNode>  &ToCenterList)
{
    CanMapNode newNode;
    unsigned char ucType = 0x04;//获取计费策略
    if(pData[0] != 0x00)
    {
        return;
    }
    newNode.enType = AddrType_SingleCardApplyAccountInfo;
    //添加类型
    InsertCanInfoMapLine((char *)&ucType, sizeof(ucType), Addr_CardAccountType, newNode.stCanMap);

    //添加到List
    ToCenterList.append(newNode);
}

//解析刷卡申请开始充电
void cJ1939GeneralProtocol::ParseFrameCardStart(unsigned char * pData, QList<CanMapNode>  &ToCenterList)
{
    FrameCardApplyStartCharge strFrame;
    CanMapNode newNode;
    float fValue = 0;
    unsigned int uiValue;
    unsigned char ucChargeType_104 = 4;//1按时间充/2按金额充/3按电量充/4充满为止
    //解帧
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    if(strFrame.ucValid != 0x55)
    {
        return;
    }
    newNode.enType = AddrType_InApplyStartChargeByChargeEquipment;
    //添加充电类型
    //InsertCanInfoMapLine((char *)strFrame.ucChargeType, sizeof(strFrame.ucChargeType), Addr_CardApplyChargeType, newNode.stCanMap);

    if(strFrame.ucChargeType == 1)//充满为止
    {
        ucChargeType_104 = 4;  //4充满为止
        uiValue = 0xffffffff;
        InsertCanInfoMapLine((char *)&uiValue, sizeof(fValue), Addr_CardChargeTypeValue, newNode.stCanMap);
    }
    else if(strFrame.ucChargeType == 2)//添加充电金额
    {
         ucChargeType_104 = strFrame.ucChargeType;
//        fValue = ((float)strFrame.usChargeAmount)/100.0;
        uiValue = strFrame.usChargeAmount;
        InsertCanInfoMapLine((char *)&uiValue, sizeof(fValue), Addr_CardChargeTypeValue, newNode.stCanMap);
    }
    else if(strFrame.ucChargeType == 3)//添加充电电量
    {
         ucChargeType_104 = strFrame.ucChargeType;
        uiValue = strFrame.usChargeAmount/10;
        InsertCanInfoMapLine((char *)&uiValue, sizeof(fValue), Addr_CardChargeTypeValue, newNode.stCanMap);
    }
    else if(strFrame.ucChargeType == 4)//添加充电时间
    {
         ucChargeType_104 = 1;  //1按时间充
        uiValue = strFrame.usChargeAmount;
        InsertCanInfoMapLine((char *)&uiValue, sizeof(fValue), Addr_CardChargeTypeValue, newNode.stCanMap);
    }

    //添加充电类型
    InsertCanInfoMapLine((char *)&ucChargeType_104, sizeof(ucChargeType_104), Addr_CardApplyChargeType, newNode.stCanMap);
    //添加充电类型(扫码)
    InsertCanInfoMapLine((char *)&ucChargeType_104, sizeof(ucChargeType_104), Addr_ScanCode_Charge_Type, newNode.stCanMap);
    //添加到List
    ToCenterList.append(newNode);
}

//解析刷卡申请结束充电
void cJ1939GeneralProtocol::ParseFrameCardStop(unsigned char * pData, QList<CanMapNode>  &ToCenterList)
{
    CanMapNode newNode;
    FrameCardApplyStopCharge strFrame;
    //解帧
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    if(strFrame.ucValid != 0x55)
    {
        return;
    }
    newNode.enType = AddrType_InApplyStopChargeByChargeEquipment;
    //添加到List
    ToCenterList.append(newNode);
}

//解析命令返回帧
void cJ1939GeneralProtocol::ParseFrameCmdAck(unsigned char * pData, QList<CanMapNode>  &ToCenterList)
{
    CanMapNode newNode;
    //相关数据项定义
    FrameCmdAck strFrame;

    unsigned char ucCtrlAck;//响应结果
    unsigned char ucCmdType;//对应命令类型--到总线
    unsigned char ucPGNAcked[3];//对应命令PGN

    //解帧
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));
    memcpy(ucPGNAcked, strFrame.ucPGNAcked, sizeof(ucPGNAcked));

    //转换ACK返回值到总线响应结果
    CheckCmdAck(ucCtrlAck, strFrame.ucCtrlAck);

    //确认对应指令类型,若协议中不存在该指令则不向内部发送.
    if(!CheckCmdType(ucCmdType, ucPGNAcked[1]))
    {
        return;
    }
    else
    {
        //添加响应结果
        InsertCanInfoMapLine((char *)&ucCtrlAck, sizeof(ucCtrlAck), Addr_AckResult_Ctrl, newNode.stCanMap);
        //添加命令类型
        InsertCanInfoMapLine((char *)&ucCmdType, sizeof(ucCmdType), Addr_ChargeCmd_Ctrl, newNode.stCanMap);

        newNode.enType = AddrType_CmdCtrl_Ack;
        //添加到List
        ToCenterList.append(newNode);
    }
}

//解析遥调命令返回帧
void cJ1939GeneralProtocol::ParseFrameParamAck(unsigned char * pData, QList<CanMapNode>  &ToCenterList)
{
    CanMapNode newNode;
    //定义相关数据
//    unsigned char ucAck;//返回结果

    //解帧
    FrameParamAck strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));
//    ucAck = (unsigned char)strFrame.ucAck;
//    newNode.enType = AddrType_TermAdjustmentAck;
//    InsertCanInfoMapLine((char *)&strFrame.ucAck, sizeof(strFrame.ucAck), Addr_CCU_ParamResult_Adj, newNode.stCanMap);

    //添加返回结果
    InsertCanInfoMapLine((char *)&strFrame.ucAck, sizeof(strFrame.ucAck), Addr_ArgSetResult, newNode.stCanMap);
    //添加配置参数编号
    InsertCanInfoMapLine((char *)&strFrame.ucArgNo, sizeof(strFrame.ucArgNo), Addr_ArgNo, newNode.stCanMap);
    //添加内部ID
    InsertCanInfoMapLine((char *)&strFrame.ucInnerID, sizeof(strFrame.ucInnerID), Addr_DevID_DC_Comm, newNode.stCanMap);
    //获取参数设置对应主题
    newNode.enType = GetParamAckType(strFrame.ucPGN[1]);

    //添加到List
    ToCenterList.append(newNode);
}

//解析连接管理帧
void cJ1939GeneralProtocol::ParseFrameLinkMange(unsigned char * pData, cLongPackageModule *pModule)
{
    FrameLinkManage stFrame;

    if(pModule == NULL)
    {
        return;
    }
    //得到控制字
    stFrame.ucCtrlManage = pData[0];
    pModule->ucPackLast = 0;

    switch(stFrame.ucCtrlManage)
    {
    case Link_ApplySend://请求发送
    {
        //已经有长包在处理, 不处理该终端的请求发送
        if(pModule->bFreeFlag == FALSE)
        {
            return;
        }
        pModule->ucCounter = 0;
        if(pModule->pLongPackage->pData != NULL)
        {
            pModule->Clear();
        }
        //解析内容
        memcpy((unsigned char *)&stFrame.data.stApplySend,(unsigned char *)&(pData[1]),DL_LinkManage - 1);
        //获取PGN
        memcpy(pModule->pLongPackage->ucPGN,stFrame.data.stApplySend.ucPackagePGN,sizeof(pModule->pLongPackage->ucPGN));
        //获取总字节长度
        pModule->usDataLength = stFrame.data.stApplySend.usDataLength;
        //获取总数据包数
        pModule->ucPackTotalNum = stFrame.data.stApplySend.ucPackageTotalNum;
        //允许发送数据包数 = 总数据包数
        pModule->ucPackAllowed = pModule->ucPackTotalNum;
        //给长包指针结构体内数据指针分配内存

        pModule->pLongPackage->pData = new unsigned char[pModule->usDataLength + 8];
        pModule->pLongPackage->uiDataLength = pModule->usDataLength;
        pModule->pDataOffset = pModule->pLongPackage->pData;
        SendFrameAllowSend(pModule);
        pModule->bFreeFlag = FALSE;
        break;
    }
    case Link_AllowSend://允许发送
    {
        pModule->ucCounter = 0;
        //解析内容
        memcpy((unsigned char *)&stFrame.data.stAllowSend,(unsigned char *)&(pData[1]),DL_LinkManage - 1);
        //允许发送的数据包数
        pModule->ucPackAllowed = stFrame.data.stAllowSend.ucPackageAllowedSendNum;
        //下一个要传输的数据包编号
        pModule->ucPackNext =  stFrame.data.stAllowSend.ucPackageNextSendNum;
        //计算要发送数据指针位置----长包数据偏移
        pModule->pDataOffset = pModule->pLongPackage->pData + pModule->ucPackTransNum * (DL_LinkManage - 1);
        //保存数据原始指针(升级计算CRC用)
        pModule->pDataOffset_old = pModule->pDataOffset;
        //校验PGN
        if(pModule->pLongPackage->ucPGN[1] != stFrame.data.stAllowSend.ucPackagePGN[1])
        {
            return;
        }
        for(int i = 0; i < pModule->ucPackAllowed;i++)
        {
            SendFrameDataTran(pModule);
            pModule->ucPackTransNum++;
        }
        pModule->bFreeFlag = FALSE;
        break;
    }
    case Link_MsgEndAck://结束应答
        pModule->bFreeFlag = TRUE;
        pModule->bUsedFlag = TRUE;
        pModule->Clear();
        break;
    case Link_Abandon://放弃连接
        pModule->bFreeFlag = TRUE;
        pModule->bUsedFlag = TRUE;
        pModule->bValidFlag = FALSE;
        pModule->Clear();
        break;
    default:
        pModule->bFreeFlag = TRUE;
        pModule->bUsedFlag = TRUE;
        pModule->bValidFlag = FALSE;
        pModule->Clear();
        break;
    }
}

//解析数据传输帧
void cJ1939GeneralProtocol::ParseFrameDataTran(unsigned char * pData, cLongPackageModule * pModule)
{
    if(pModule == NULL)
    {
        return;
    }
    pModule->ucCounter = 0;
    //写数据
    memcpy(pModule->pDataOffset, pData+1, DL_DataTran -1);
    //已经传输数据包+1
    pModule->ucPackTransNum++;
    //包顺序校验
    if(pModule->ucPackLast + 1 != pData[0])
    {
        SendFrameLinkAbandon(pModule);
        pModule->bFreeFlag = TRUE;
        pModule->bValidFlag = FALSE;
        pModule->bUsedFlag = TRUE;
        return;
    }
    pModule->ucPackLast++;
    //接收完成,发送命令回复
    if(pModule->ucPackTransNum >= pModule->ucPackTotalNum)
    {
        pModule->pDataOffset = pModule->pLongPackage->pData;
        SendFrameLinkMsgEndAck(pModule);
        pModule->bValidFlag = TRUE;
        return;
    }
    //数据指针偏移
    pModule->pDataOffset += (DL_DataTran - 1);
}

//解析升级请求回复帧
void cJ1939GeneralProtocol::ParseFrameUpdateRequestAck(unsigned char * pData, cLongPackageModule * pModule, QList<CanMapNode>  &ToCenterList)
{
//    pModule->ucCounter = 0;
    CanMapNode newNode;
    FrameUpdateRequestAck stFrame;
    if(pModule == NULL)
    {
        return;
    }
    pModule->ucCounter = 0;
    memcpy((unsigned char *)&stFrame, pData, DL_UpdateRequestAck);
    if(stFrame.ucUpdateAck != AllowUpdate)//设备不允许升级
    {
        pModule->bFreeFlag = TRUE;
        pModule->bUsedFlag = TRUE;
        pModule->bValidFlag = FALSE;
        //添加设备升级结果
        InsertCanInfoMapLine((char *)&stFrame.ucUpdateAck, sizeof(stFrame.ucUpdateAck), Addr_CANUpdateResult, newNode.stCanMap);

        newNode.enType = AddrType_UpdateResult_Dev;
        //添加到List
        ToCenterList.append(newNode);
    }
    else    //设备允许升级, 发送升级请求
    {
        SendFrameUpdateApply(pModule);
    }
}

//解析本包数据接收完成回复帧
void cJ1939GeneralProtocol::ParseFramePackageRecvAck(unsigned char * pData, cLongPackageModule * pModule, QList<CanMapNode>  &ToCenterList)
{
//    pModule->ucCounter = 0;
    CanMapNode newNode;
    FramePackageRecvAck stFrame;
    if(pModule == NULL)
    {
        return;
    }
    pModule->ucCounter = 0;
    memcpy((unsigned char *)&stFrame, pData, DL_PackageRecvAck);
    if(stFrame.ucPackageRecvAck != 0xAA)//本包传输不成功
    {
        if(pModule->ucReSendCount > ucMaxUpPackReSend)//重发超过最大次数
        {
            unsigned char ucRet = 05;//传输错误超过3次
            pModule->bFreeFlag = TRUE;
            pModule->bUsedFlag = TRUE;
            pModule->bValidFlag = FALSE;

            //添加设备升级结果
            InsertCanInfoMapLine((char *)&ucRet, sizeof(ucRet), Addr_CANUpdateResult, newNode.stCanMap);
            newNode.enType = AddrType_UpdateResult_Dev;
            //添加到List
            ToCenterList.append(newNode);
        }
        else
        {
            pModule->ucReSendCount++;
            pModule->pDataOffset = pModule->pDataOffset_old;
            pModule->usPackTransNum_U-=pModule->usPackAllowed_U;
        }
    }
    else//本包传输成功
    {
        pModule->ucReSendCount = 0;//重发计数重新计数

        if(pModule->uiDataLength_U == pModule->uiDataTransLength_U)//程序总字节数==传输字节数, 传输完成
        {
            ModuleMap :: iterator it,itbak;
            QByteArray tempArray;
            CanMapNode newNode;
            pModule->bFreeFlag = TRUE;
            pModule->bUsedFlag = TRUE;
            pModule->bValidFlag = FALSE;
            SendFrameProgramFinishSend(pModule);

            //发送升级成功
            char tempch = 0xaa;
            tempArray.append(char(tempch));
            InsertCanInfoMapLine((char *)&tempch, sizeof(tempch), Addr_CANUpdateResult, newNode.stCanMap);
            InsertCanInfoMapLine((char *)&pModule->ucCanID, sizeof(pModule->ucCanID), Addr_CanID_Comm, newNode.stCanMap);
            newNode.enType = AddrType_UpdateResult_Dev;
            SendCenterData(newNode.enType, newNode.stCanMap);

            for(it = pModuleMap->begin(); it != pModuleMap->end(); ++it)
            {
                if(it.key() != pModule->ucCanID)
                    continue;

                itbak = it;
                delete itbak.value();
                itbak.value() = NULL;
                pModuleMap->erase(itbak);
            }
        }
        else//传输未完成
        {
            SendFrameUpdateApply(pModule);
        }
    }
}

//解析升级管理帧
void cJ1939GeneralProtocol::ParseFrameUpdateManage(unsigned char * pData, cLongPackageModule *pModule)
{
//    pModule->ucCounter = 0;
    FrameUpdateManage stFrame;
    if(pModule == NULL)
    {
        return;
    }
    //得到控制字
    stFrame.ucCtrlManage = pData[0];
    pModule->ucPackLast = 0;
    pModule->ucCounter = 0;
    switch(stFrame.ucCtrlManage)
    {
    case Link_ApplySend://请求发送
    {
        //已经有长包在处理, 不处理该终端的请求发送
        SendFrameUpdateAbandon(pModule);
        pModule->bFreeFlag = TRUE;
        pModule->bUsedFlag = TRUE;
        pModule->bValidFlag = FALSE;
        break;
    }
    case Link_AllowSend://允许发送
    {
        pModule->ucCounter = 0;

        //解析内容
        memcpy((unsigned char *)&stFrame.data.stAllowSend,(unsigned char *)&(pData[1]),DL_UpadteManage - 1);
        //允许发送的数据包数
        pModule->usPackAllowed_U = stFrame.data.stAllowSend.usAllowSendPackNum;
        //下一个要传输的数据包编号
        pModule->ucPackNext =  stFrame.data.stAllowSend.ucPackageNextSendNum;
        //计算要发送数据指针位置----长包数据偏移
        pModule->pDataOffset = pModule->pLongPackage->pData + pModule->uiLongPackTransNum_U * UpdatePerLongPackLongth;
        pModule->pDataOffset_old = pModule->pDataOffset;
        pModule->usPackTransNum_U = 0;
        //发送长包数据
        if(pModule->bUpdateFlag == TRUE)//为升级长包
        {
            for(int i = 0; i < pModule->usPackAllowed_U; i++)
            {
                SendFrameDataTran(pModule);
            }
            SendFrameUpdateMsgEndAck(pModule);
            pModule->uiLongPackTransNum_U++;
            if(pModule->uiLongPackTransNum_U >= pModule->uiLongPackNum_U)
            {
            }
        }
        else//为普通长包
        {
            for(int i = 0; i < pModule->ucPackAllowed; i++)
            {
                SendFrameDataTran(pModule);
                pModule->ucPackTransNum++;
            }
        }
        pModule->bFreeFlag = FALSE;
        break;
    }
    case Link_MsgEndAck://结束应答
        pModule->bFreeFlag = TRUE;
        pModule->bUsedFlag = TRUE;
        pModule->Clear();
        break;
    case Link_Abandon://放弃连接
        pModule->bFreeFlag = TRUE;
        pModule->bUsedFlag = TRUE;
        pModule->Clear();
        break;
    default:
        break;
    }
}

//解析升级完成帧
void cJ1939GeneralProtocol::ParseFrameProgramRecvFinsh(unsigned char * pData, QList<CanMapNode>  &ToCenterList)
{
    CanMapNode newNode;
    FrameProgramRecvFinsh stFrame;
    memcpy((unsigned char *)&stFrame, pData, DL_ProgramRecvFinsh);
//    //添加设备升级结果
//    InsertCanInfoMapLine((char *)&stFrame.ucFinshRecvAck, sizeof(stFrame.ucFinshRecvAck), Addr_CANUpdateResult, newNode.stCanMap);
//    newNode.enType = AddrType_UpdateResult_Dev;
//    ToCenterList.append(newNode);
}

//解析升级版本信息
void cJ1939GeneralProtocol::ParseFrameUpdateAck(unsigned char * pData, QList<CanMapNode>  &ToCenterList)
{
    CanMapNode newNode;
    FrameUpdateAck stFrame;
    QString strVer;
    memcpy((unsigned char *)&stFrame, pData, DL_UpdateAck);
    strVer = QString::number(stFrame.ucVersion[2],16)
            +"." + QString::number(stFrame.ucVersion[1],16)
            +"." + QString::number(stFrame.ucVersion[0],16);
    //添加软件版本号
//    InsertCanInfoMapLine(strVer.toAscii().data(), strVer.length(), Addr_SoftwareVer_Term, newNode.stCanMap);
    newNode.stCanMap.insert(Addr_SoftwareVer_Term, strVer.toAscii());
    newNode.enType = AddrType_TermSignal;
    ToCenterList.append(newNode);
    newNode.stCanMap.clear();
    //升级结果不是默认值, 添加升级结果
    if(stFrame.ucUpdateResult!=0xFF)
    {
        InsertCanInfoMapLine((char *)&stFrame.ucUpdateResult, sizeof(stFrame.ucUpdateResult), Addr_CANUpdateResult, newNode.stCanMap);
        newNode.enType = AddrType_UpdateResult_Dev;
        ToCenterList.append(newNode);
    }
}

//解析设备模块信息
void cJ1939GeneralProtocol::ParseSpecificInfo(FrameLongPackage * pLongPackage, QList<CanMapNode>  &ToCenterList)
{
    CanMapNode newNode;
    FrameSpecificInfo stFrame;
    QString tempString;
    if(pLongPackage->pData == NULL)
    {
        return;
    }
    memcpy((unsigned char *)&stFrame, pLongPackage->pData, DL_SpecificInfo);
    //添加设备规格信息传输完成标志
    InsertCanInfoMapLine((char *)&stFrame.ucEndFlag, sizeof(stFrame.ucEndFlag), Addr_SepcEndFlag_Term, newNode.stCanMap);
    //添加槽位号
    InsertCanInfoMapLine((char *)&stFrame.ucSlotNum, sizeof(stFrame.ucSlotNum), Addr_SlotNum_Term, newNode.stCanMap);
    //添加软件版本号
    tempString = QString::number(stFrame.ucSoftwareVer[2], 10) + "." + QString::number(stFrame.ucSoftwareVer[1], 10) + "." + QString::number(stFrame.ucSoftwareVer[0], 10);
    newNode.stCanMap.insert(Addr_SoftwareVer_Term, tempString.toAscii());
    //添加软件版本号1
    tempString = QString::number(stFrame.ucSoftwareVer1[2], 10) + "." + QString::number(stFrame.ucSoftwareVer1[1], 10) + "." + QString::number(stFrame.ucSoftwareVer1[0], 10);
    newNode.stCanMap.insert(Addr_SoftwareVer1_Term, tempString.toAscii());
    //添加软件版本号2
    tempString = QString::number(stFrame.ucSoftwareVer2[2], 10) + "." + QString::number(stFrame.ucSoftwareVer2[1], 10) + "." + QString::number(stFrame.ucSoftwareVer2[0], 10);
    newNode.stCanMap.insert(Addr_SoftwareVer2_Term, tempString.toAscii());

    //添加硬件版本号
    tempString = QString::number(stFrame.ucHardwareVer[2], 10) + "." + QString::number(stFrame.ucHardwareVer[1], 10) + "." + QString::number(stFrame.ucHardwareVer[0], 10);
    newNode.stCanMap.insert(Addr_HardwareVer_Term, tempString.toAscii());

    //添加条码号
    InsertCanInfoMapLine(stFrame.chSerialNumber, 32, Addr_SerialNumber_Term, newNode.stCanMap);

    //添加模块ID
    InsertCanInfoMapLine((char *)&stFrame.ucDevID, sizeof(stFrame.ucDevID), Addr_DevID_DC_Comm, newNode.stCanMap);

    newNode.enType = AddrType_ModuleSpecInfo;
    ToCenterList.append(newNode);
}

//将升级文件加载到内存
//pProPath: 升级文件路径
//cLongPackageModule * pModule: 长帧模块
bool cJ1939GeneralProtocol::GetUpdateProgramData(const char *pProPath, cLongPackageModule * pModule)
{
    QFile UpdateFile(pProPath);
    QByteArray ArrayFile;
    if(!UpdateFile.exists())
    {
        return FALSE;
    }
    //QIODevice::ReadOnly, QIODevice::WriteOnly, or QIODevice::ReadWrite.
    UpdateFile.open(QIODevice::ReadOnly);
    ArrayFile = UpdateFile.readAll();
    if(ArrayFile.length() == 0)
    {
        return FALSE;
    }

    try
    {
        pModule->pLongPackage->pData = new unsigned char[ArrayFile.size()];
    }
    catch(std::bad_alloc&e)
    {
        pModule->pLongPackage->pData = NULL;
        return FALSE;
    }

    //将文件加载到内存
    memcpy(pModule->pLongPackage->pData, (unsigned char *)(ArrayFile.data()), ArrayFile.size());
    //总分割长包包数
    pModule->uiLongPackNum_U = ArrayFile.size()/UpdatePerLongPackLongth;
    if(ArrayFile.size()%UpdatePerLongPackLongth != 0)
    {
        pModule->uiLongPackNum_U++;
    }

    //升级包总长度
    pModule->uiDataLength_U = ArrayFile.size();
    //下一包编号
    pModule->ucPackNext = 1;
    return TRUE;
}

//双抢模式返回结果
void cJ1939GeneralProtocol::ParseChargeTypeReault(InfoMap CenterMap, unsigned char ucTermID)
{
    unsigned int uiChargeResult = 0;
    InfoMap::iterator itTarget;
    //解析Map内容
    for(itTarget = CenterMap.begin(); itTarget != CenterMap.end(); itTarget++)
    {
        switch (itTarget.key())
        {
        //解析遥控指令
        case Addr_DCChargeMannerSetResult_Term:
        {
            uiChargeResult = itTarget.value().at(0);
            break;
        }
        default:
            break;
        }
    }
       SendFrameAck(ucTermID,uiChargeResult);//应答E8
}
