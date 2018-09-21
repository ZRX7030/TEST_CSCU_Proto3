#include "ParkingLockProtocol.h"

cParkingLockProtocol::cParkingLockProtocol()
{
    ;
}

cParkingLockProtocol::~cParkingLockProtocol()
{
}

//解析总线数据
void cParkingLockProtocol::ParseCenterData(InfoMap CenterMap , InfoAddrType enAddrType , unsigned char ucParkingLockAddr)
{
    switch(enAddrType)
    {
    case AddrType_CarLock_Apply: //解析车位锁控制指令
    {
        ParseCarLockCtrlCmd(CenterMap, ucParkingLockAddr);
        break;
    }
    case AddrType_CarLock_ParamSet: //解析车位锁参数设置指令
    {
        ParseCarLockParaSetCmd(CenterMap, ucParkingLockAddr);
        break;
    }
    default:
        break;
    }
}

//解析车位锁开关控制指令
void cParkingLockProtocol::ParseCarLockCtrlCmd(InfoMap CenterMap , unsigned char ucParkingLockAddr)
{
    unsigned char ucCmdType = 0; //控制类型 02升锁并解除休眠   03降锁　04开蜂鸣器   05关蜂鸣器    06降锁进入休眠
    InfoMap::iterator itTarget;

    for(itTarget = CenterMap.begin(); itTarget != CenterMap.end(); itTarget++)//解析Map内容
    {
        switch (itTarget.key())
        {
        //解析遥控指令
        case Addr_CarLockCmd:
        {
            ucCmdType = itTarget.value().at(0);
            break;
        }
        default:
            break;
        }
    }
    //发送遥控指令
    if(ucCmdType != 0)
    {
        SendCtrlCmd(ucCmdType, ucParkingLockAddr);
    }
    else
    {
    }
}

//解析车位锁参数设置指令
void cParkingLockProtocol::ParseCarLockParaSetCmd(InfoMap CenterMap , unsigned char ucParkingLockAddr)
{
    char value[2] ; //地锁初始值和时长
    InfoMap::iterator itTarget;
    for(itTarget = CenterMap.begin(); itTarget != CenterMap.end(); itTarget++)//解析Map内容
    {
        switch (itTarget.key())
        {
        //解析遥控指令
        case Addr_CarLockParaSetCmd:
        {
            memcpy(value, itTarget.value().data(), itTarget.value().size());
            ConvertDataFormat( (unsigned char*) value, 2 );
            break;
        }
        default:
            break;
        }
    }
    //发送遥控指令
    if(value != NULL)
    {
        SendParaSetCmd((unsigned char *)value, ucParkingLockAddr);
    }
    else
    {
    }
}


//发送车位锁控制指令
 void cParkingLockProtocol::SendCtrlCmd(unsigned char ucCmdType, unsigned char ucParkingLockAddr)
 {
     can_frame *pCanFrame = new(can_frame);

     FrameCmd_CarLock stFrame;  //生成数据
     stFrame.ucCarLockAddr = ucParkingLockAddr;
     stFrame.ucCmdType = ucCmdType ;
     SetFrameHead(pCanFrame, ID_DefaultControlCenterCanID, PF_CtrlCmd, ID_CarLockCanID, CAN1PriorityFour , DL_CtrlCmd);//生成帧头
     memcpy(pCanFrame->data,(unsigned char *) &stFrame,sizeof(stFrame));
      emit sigSendCanData(pCanFrame);
 }

 //发送车位锁参数设置指令
  void cParkingLockProtocol::SendParaSetCmd(unsigned char *value, unsigned char ucParkingLockAddr)
  {
      can_frame *pCanFrame = new(can_frame);

      FrameParaSet_CarLock stFrame;  //生成数据
      stFrame.ucCarLockAddr = ucParkingLockAddr;
      stFrame.usSetAddr= 0x0106 ;
      //stFrame.usSetValue = value;
      ConvertDataFormat( value, 2);//大小端转换
      ConvertDataFormat( (unsigned char *)&stFrame.usSetAddr, 2);//大小端转换
      memcpy(stFrame.ucSetValue,value,sizeof(value));
      SetFrameHead(pCanFrame, ID_DefaultControlCenterCanID, PF_ParaSet, ID_CarLockCanID, CAN1PriorityFour , DL_ParaSet);//生成帧头
      memcpy(pCanFrame->data,(unsigned char *) &stFrame,sizeof(stFrame));
      emit sigSendCanData(pCanFrame);
  }


 //pCanFrame CAN组帧后存储位置,ucSa 源地址,ucPf  PGN类型,ucPs 目的地址,ucPriority 优先级,ucLength 数据长度
 bool cParkingLockProtocol::SetFrameHead(can_frame *pCanFrame, unsigned char ucSa, unsigned char ucPf, unsigned char ucPs,unsigned char ucPriority, unsigned char ucLength)
 {
     ParkingLockFrameHead stFrameHead;
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

//解析CAN数据
void cParkingLockProtocol::ParseCanData(can_frame *pCanFrame)
{
    QList <ParkingLockCanMapNode> ToCenterList;    //传递给车位锁类的map
    QByteArray ParkingLockAddrArray;

    ParkingLockFrameHead strFrameHead; //解析帧头
    memcpy((unsigned char *)&strFrameHead, (unsigned char *)&pCanFrame->can_id, sizeof(strFrameHead));

    ParkingLockAddrArray.append(pCanFrame->data[0]); //添加车位锁地址节点

    //解析其他内容
    switch(strFrameHead.ucPf)
    {
        case PF_HeartBeat:
        {
            ParseFrameLifeInfo(pCanFrame->data);
            break;
        }
        case PF_ParkingLockStates:
        {
            ParseFrameParkingLockStates(pCanFrame->data,ToCenterList);
            break;
        }
        case PF_CtrlCmdResponse:
        {
            ParseFrameCtrlCmdAck(pCanFrame->data,ToCenterList);
            break;
        }

        default:
        {
            break;
        }
    }
    //有数据发送
    for(unsigned char i = 0 ; i < ToCenterList.count(); i++)
    {
        ToCenterList.at(i).stCanMap.insert(Addr_CarLockID,ParkingLockAddrArray);
        SendCenterData(ToCenterList.at(i).enType, ToCenterList.at(i).stCanMap);
    }
}

//接收CAN数据
void cParkingLockProtocol::ProcParseData(QList <can_frame *> *pTerminalRecvList)
{
    ParkingLockFrameHead strFrameHead;
    if(!pTerminalRecvList->isEmpty())
    {
        for(int i = 0; i < pTerminalRecvList->count(); i++)
        {
            memcpy((unsigned char *)&strFrameHead, (unsigned char *) &pTerminalRecvList->at(i)->can_id, sizeof(strFrameHead));
            if(strFrameHead.ucSa == ID_CarLockCanID)
            {
                ParseCanData(pTerminalRecvList->at(i));
            }
            delete pTerminalRecvList->at(i);
        }
        pTerminalRecvList->clear();
    }
}

//发送帧到控制中心
void cParkingLockProtocol::SendCenterData(unsigned int &uiInfoAddr, InfoMap &TermMap)
{
    emit sigSendToCenter(uiInfoAddr ,TermMap);
}

//解析车位锁状态
void cParkingLockProtocol::ParseFrameParkingLockStates(unsigned char * pData, QList<ParkingLockCanMapNode>  &ToCenterList)
{
    ParkingLockCanMapNode newNode;
    ParkingLockStatus parkinglockstatus;

    //解帧
    FrameStatesInfo_CarLock  strFrame;
    memcpy((unsigned char *)&strFrame, pData, sizeof(strFrame));

    //车位锁状态
    if(strFrame.stCarLockStates.ucWorkStates == 0)
    {
        parkinglockstatus.cCarLockStatus[0] =2;
    }
    else
    {
        parkinglockstatus.cCarLockStatus[0] = strFrame.stCarLockStates.ucWorkStates;
    }

    if(strFrame.stCarLockStates.ucRockerArmStates == 0)
    {
        parkinglockstatus.cCarLockStatus[1] =3;
    }
    else
    {
        parkinglockstatus.cCarLockStatus[1] = strFrame.stCarLockStates.ucRockerArmStates;
    }

    if(strFrame.stCarLockStates.ucBuzzerStates == 0)
    {
        parkinglockstatus.cCarLockStatus[2] =2;
    }
    else
    {
        parkinglockstatus.cCarLockStatus[2] = strFrame.stCarLockStates.ucBuzzerStates;
    }

    if(strFrame.stCarLockStates.ucMagneticFluctuations == 0)
    {
        parkinglockstatus.cCarLockStatus[3] =2;
    }
    else
    {
        parkinglockstatus.cCarLockStatus[3] = strFrame.stCarLockStates.ucMagneticFluctuations;
    }

    if(strFrame.stCarLockStates.ucAutoUpEnable == 0)
    {
        parkinglockstatus.cCarLockStatus[4] =2;
    }
    else
    {
        parkinglockstatus.cCarLockStatus[4] = strFrame.stCarLockStates.ucAutoUpEnable;
    }

    //车位状态
    if(strFrame.stParkingStates.ucParkingStates == 0)
    {
        parkinglockstatus.cParkingStatus[0] =1;
    }
    else
    {
        parkinglockstatus.cParkingStatus[0] = strFrame.stParkingStates.ucParkingStates;
    }

    parkinglockstatus.cParkingStatus[1] = strFrame.stParkingStates.ucReserve;
    //传感器故障状态
    if(strFrame.stAlarmStates.ucAlarmGeomagneticSensor == 0)
    {
        parkinglockstatus.cSensorFault[0] =2;
    }
    else
    {
        parkinglockstatus.cSensorFault[0] = strFrame.stAlarmStates.ucAlarmGeomagneticSensor;
    }
    if(strFrame.stAlarmStates.ucAlarmUltrasonicProbe1 == 0)
    {
        parkinglockstatus.cSensorFault[1] =2;
    }
    else
    {
        parkinglockstatus.cSensorFault[1] = strFrame.stAlarmStates.ucAlarmUltrasonicProbe1;
    }

    if(strFrame.stAlarmStates.ucAlarmUltrasonicProbe2 == 0)
    {
        parkinglockstatus.cSensorFault[2] =2;
    }
    else
    {
       parkinglockstatus.cSensorFault[2] = strFrame.stAlarmStates.ucAlarmUltrasonicProbe2;
    }
    parkinglockstatus.cSensorFault[3] = strFrame.stAlarmStates.ucReserve1;
    //车位锁结构后电机故障状态
    if(strFrame.stAlarmStates.ucAlarmDeviationPosition == 0)
    {
        parkinglockstatus.cLockStructureFault[0] =2;
    }
    else
    {
        parkinglockstatus.cLockStructureFault[0] = strFrame.stAlarmStates.ucAlarmDeviationPosition;
    }
    if(strFrame.stAlarmStates.ucAlarmTimeOutNotInPlace == 0)
    {
        parkinglockstatus.cLockStructureFault[1] =2;
    }
    else
    {
         parkinglockstatus.cLockStructureFault[1] = strFrame.stAlarmStates.ucAlarmTimeOutNotInPlace;
    }

    if(strFrame.stAlarmStates.ucAlarmUnknown == 0)
    {
        parkinglockstatus.cLockStructureFault[2] =2;
    }
    else
    {
        parkinglockstatus.cLockStructureFault[2] = strFrame.stAlarmStates.ucAlarmUnknown;
    }

    parkinglockstatus.cLockStructureFault[3] = strFrame.stAlarmStates.ucReserve2;

    //添加车位锁状态
    QByteArray termArray;
    termArray.append((char *)(parkinglockstatus.cCarLockStatus),5);
    newNode.stCanMap.insert(Addr_CarLockStates,termArray);
    termArray.clear();
    //添加车位状态
    termArray.append(( char *)(parkinglockstatus.cParkingStatus),2);
    newNode.stCanMap.insert(Addr_ParkingStates,termArray);
    termArray.clear();

    //添加传感器故障状态
    termArray.append(( char *)(parkinglockstatus.cSensorFault),4);
    newNode.stCanMap.insert(Addr_CarLock_SensorFaultStates,termArray);
    termArray.clear();

    //添加锁结构或电机故障状态
    termArray.append(( char *)(parkinglockstatus.cLockStructureFault),4);
    newNode.stCanMap.insert(Addr_CarLock_StructureFaultStates,termArray);
    termArray.clear();

    newNode.enType = AddrType_TermCarLock;
    ToCenterList.append(newNode);
}

//解析控制命令响应数据
void cParkingLockProtocol::ParseFrameCtrlCmdAck(unsigned char * pData,QList<ParkingLockCanMapNode> &ToCenterList)
{
    ParkingLockCanMapNode newNode;
    FrameCmdAck_CarLock strFrame;

    memcpy((unsigned char * )&strFrame,pData,sizeof(strFrame));
    if(strFrame.ucCmdAck == ExecutionSuccess)
    {
        newNode.enType= AddrType_CarLock_Result ;
        strFrame.ucCmdAck = 0xFF ;
        QByteArray termArray;
        termArray.append((char *)&(strFrame.ucCmdAck),sizeof(strFrame.ucCmdAck));
        newNode.stCanMap.insert(Addr_CarLockCmd_Result,termArray);
        ToCenterList.append(newNode);
    }
    else if(strFrame.ucCmdAck == ExecutionFailure)
    {
        newNode.enType= AddrType_CarLock_Result ;
        strFrame.ucCmdAck = 0x01;
        QByteArray termArray;
        termArray.append((char *)&(strFrame.ucCmdAck),sizeof(strFrame.ucCmdAck));
        newNode.stCanMap.insert(Addr_CarLockCmd_Result,termArray);
        ToCenterList.append(newNode);
    }
    else
    {
        return;
    }
}

//解析生命信息帧
void cParkingLockProtocol::ParseFrameLifeInfo(unsigned char * pData)
{
    return;
}
