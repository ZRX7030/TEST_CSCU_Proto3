#include "LCDScreenProtocol.h"

cLCDScreenProtocol::cLCDScreenProtocol()
{
	_strLogName = "screen";
    //错峰设置数据接收
    memset(PeakSet, 0x00, sizeof(PeakSet));
    //刷卡卡号
    CardNum.clear();

    pDevCache = DevCache::GetInstance();
    pDBOperate = DBOperate::GetInstance();
    pParamSet = ParamSet::GetInstance();
    pLog = Log::GetInstance();

    QueryParamInfo();
    //初始化终端名称图
    InitTermNameMap();
    InitTermNameMapShow();
    InitTermNameMapMulti();
    //初始化U盘状态
    ucUDiskState = 0; //U盘插入状态: 1: 插入, 2: 拔下

}

//转换  错峰充电格式
void cLCDScreenProtocol::ChangePeakFormat(PeakSet_LCD * &pPeakSet, const stTPFVConfig & stPeak)
{
    pPeakSet->ucType = stPeak.time_seg;
    pPeakSet->ucStartH = stPeak.start_hour;
    pPeakSet->ucStartM = stPeak.start_minute;
    pPeakSet->ucStopH = stPeak.stop_hour;
    pPeakSet->ucStopM = stPeak.stop_minute;
    pPeakSet->ucSOC = stPeak.limit_soc;
    pPeakSet->ucCurrent = stPeak.limit_current;
}

//转换  故障代码为字符串
void cLCDScreenProtocol::ChangeFaultCodeToArray(unsigned char ucFaultCode, QByteArray &FaultArray)
{
    switch(ucFaultCode)
    {
    case 0:
        FaultArray = "无故障";
        break;
    case 1:
        FaultArray = "欠压";
        break;
    case 2:
        FaultArray = "过压";
        break;
    case 3:
        FaultArray = "过流";
        break;
    case 4:
        FaultArray = "电表通讯故障";
        break;
    case 5:
        FaultArray = "充电枪故障";
        break;
    case 6:
        FaultArray = "漏电故障";
        break;
    case 7:
        FaultArray = "计量失效";
        break;
    case 8:
        FaultArray = "继电器过温";
        break;
    case 9:
        FaultArray = "继电器粘连";
        break;
    case 0x0A:
        FaultArray = "枪头过温";
        break;
    case 0x10:
        FaultArray = "读取配置文件失败";
        break;
    case 0x11:
        FaultArray = "GPRS通信故障，上电失败";
        break;
    case 0x12:
        FaultArray = "读卡器没有ESAM卡";
        break;
    case 0x13:
        FaultArray = "读卡器通信故障";
        break;
    case 0x14:
        FaultArray = "检测到未加密的卡";
        break;
    case 0x30:
        FaultArray = "充电模块故障";
        break;
    case 0x31:
        FaultArray = "主接触器故障";
        break;
    case 0x32:
        FaultArray = "辅助接触器故障";
        break;
    case 0x33:
        FaultArray = "交流接触器故障";
        break;
    case 0x34:
        FaultArray = "熔断器故障";
        break;
    case 0x35:
        FaultArray = "防雷故障";
        break;
    case 0x36:
        FaultArray = "电磁锁故障";
        break;
    case 0x37:
        FaultArray = "辅助电源故障";
        break;
    case 0x38:
        FaultArray = "充电模块风扇故障";
        break;
    case 0x39:
        FaultArray = "直流输出过压";
        break;
    case 0x3A:
        FaultArray = "直流输出欠压";
        break;
    case 0x3B:
        FaultArray = "直流输出过流";
        break;
    case 0x3C:
        FaultArray = "直流输出反接";
        break;
    case 0x3D:
        FaultArray = "充电模块输入过压";
        break;
    case 0x3E:
        FaultArray = "充电模块输入欠压";
        break;
    case 0x3F:
        FaultArray = "交流输入频率过频";
        break;
    case 0x40:
        FaultArray = "交流输入频率欠频";
        break;
    case 0x41:
        FaultArray = "交流输入电压不平衡";
        break;
    case 0x42:
        FaultArray = "交流输入A相缺相";
        break;
    case 0x43:
        FaultArray = "交流输入B相缺相";
        break;
    case 0x44:
        FaultArray = "交流输入C相缺相";
        break;
    case 0x45:
        FaultArray = "交流输入过载";
        break;
    case 0x46:
        FaultArray = "交流输入异常";
        break;
    case 0x47:
        FaultArray = "充电模块输出过压";
        break;
    case 0x48:
        FaultArray = "充电模块过流";
        break;
    case 0x49:
        FaultArray = "充电模块过温";
        break;
    case 0x4A:
        FaultArray = "环境温度过温";
        break;
    case 0x4B:
        FaultArray = "环境温度过低";
        break;
    case 0x4C:
        FaultArray = "充电模块通信故障";
        break;
    case 0x4D:
        FaultArray = "充电模块命令执行失败";
        break;
    case 0x4E:
        FaultArray = "充电控制器通信故障";
        break;
    case 0x4F:
        FaultArray = "采集板通讯故障";
        break;
    case 0x50:
        FaultArray = "电表通讯异常";
        break;
    case 0x51:
        FaultArray = "与集控器通信中断";
        break;
    case 0x52:
        FaultArray = "读卡器通信故障";
        break;
    case 0x53:
        FaultArray = "绝缘故障";
        break;
    case 0x54:
        FaultArray = "模块类型不一致";
        break;
    case 0x55:
        FaultArray = "紧急停机";
        break;
    case 0x56:   //单桩使用
        FaultArray = "主从通信故障";
        break;
    case 0x57:
        FaultArray = "电表校验错误";
        break;
    case 0x58:
        FaultArray = "门磁故障";
        break;
    case 0x59:
        FaultArray = "系统风机故障";
        break;
    case 0x5A:
        FaultArray = "并联接触器故障";
        break;
    case 0x5b:
        FaultArray = "并联接触器驱动失效告警";
        break;
    case 0x5c:
        FaultArray = "绝缘检测告警";
        break;
    case 0x5F:
        FaultArray = "CCU ID重复";
        break;
    //  GuoCheng Add Begin
    case 0x60:
        FaultArray = "BMS数据异常";
        break;
    case 0x61:
        FaultArray = "单体电池过压";
        break;
    case 0x62:
        FaultArray = "整包电池过压";
        break;
    case 0x63:
        FaultArray = "整包电池过流";
        break;
    case 0x64:
        FaultArray = "电池过充";
        break;
    case 0x65:
        FaultArray = "电池电压异常";
        break;
    case 0x66:
        FaultArray = "BMS温度过低";
        break;
    case 0x67:
        FaultArray = "BMS热失控";
        break;
    case 0x68:
        FaultArray = "BMS辅源异常";
        break;
    case 0x69:
        FaultArray = "CCU辅源异常";
        break;
    case 0x6A:
        FaultArray = "继电器开路";
        break;
    //  GuoCheng Add End
    case 0x70:
        FaultArray = "电池电压过高";
        break;
    case 0x71:
        FaultArray = "电池SOC过高";
        break;
    case 0x72:
        FaultArray = "电池SOC过低";
        break;
    case 0x73:
        FaultArray = "电池充电过流";
        break;
    case 0x74:
        FaultArray = "电池温度过高";
        break;
    case 0x75:
        FaultArray = "电池绝缘故障";
        break;
    case 0x76:
        FaultArray = "电池输出连接器异常";
        break;
    case 0x90:
        FaultArray = "ZIGBEE模块离线";
        break;
    case 0x91:
        FaultArray = "车档器电压告警";
        break;
        //新增故障数据 smm 2017-08-12
      case 0xa0:
          FaultArray = "RJH1内风机故障";
          break;
      case 0xa1:
          FaultArray = "RJH1外风机故障";
          break;
      case 0xa2:
          FaultArray = "RJH1温度传感器故障";
          break;
      case 0xa3:
          FaultArray = "RHJ1过压故障";
          break;
      case 0xa4:
          FaultArray = "RJH1欠压故障";
          break;
      case 0xa5:
          FaultArray = "RJH1加热器故障";
          break;
      case 0xa6:
          FaultArray = "RJH1通讯故障";
          break;
      case 0xa7:
          FaultArray = "RJH2内风机故障";
          break;
      case 0xa8:
          FaultArray = "RJH2外风机故障";
          break;
      case 0xa9:
          FaultArray = "RJH2温度传感器故障";
          break;
      case 0xaa:
          FaultArray = "RJH2过压故障";
          break;
      case 0xab:
          FaultArray = "RJH2欠压故障";
          break;
      case 0xac:
          FaultArray = "RJH2加热器故障";
          break;
      case 0xad:
          FaultArray = "RJH2通讯故障";
          break;
        //预留部分空
      case 0xb0:
          FaultArray = "湿度告警故障";
          break;
      case 0xb1:
          FaultArray = "枪头过温告警";
          break;
    default:
        FaultArray = "未定义的故障";
        break;
    }
}

//转换  故障代码为字符串
void cLCDScreenProtocol::ChangeFaultCodeToEnglishArray(unsigned char ucFaultCode, QByteArray &FaultArray)
{
    switch(ucFaultCode)
    {
    case 0:
        FaultArray = "Fault-free";
        break;
    case 1:
        FaultArray = "Undervoltage";
        break;
    case 2:
        FaultArray = "Overvoltage";
        break;
    case 3:
        FaultArray = "Overcurrent";
        break;
    case 4:
        FaultArray = "Communication failure of electric meter";
        break;
    case 5:
        FaultArray = "Charging gun failure";
        break;
    case 6:
        FaultArray = "Failure by electric leakage";
        break;
    case 7:
        FaultArray = "Metering failure";
        break;
    case 8:
        FaultArray = "Relay over-temperature";
        break;
    case 9:
        FaultArray = "Relay erosion";
        break;
    case 0x0A:
        FaultArray = "Gun head over-temperature";
        break;
    case 0x10:
        FaultArray = "Failed to read the configuration file";
        break;
    case 0x11:
        FaultArray = "GPRS communication failure.Power-on failure";
        break;
    case 0x12:
        FaultArray = "No ESAM card in the reader";
        break;
    case 0x13:
        FaultArray = "Reader communication failure";
        break;
    case 0x14:
        FaultArray = "Unencrypted card detected";
        break;
    case 0x30:
        FaultArray = "Charging module fault";
        break;
    case 0x31:
        FaultArray = "Main contactor fault";
        break;
    case 0x32:
        FaultArray = "Auxiliary contactor fault";
        break;
    case 0x33:
        FaultArray = "AC contactor fault";
        break;
    case 0x34:
        FaultArray = "Fuse fault";
        break;
    case 0x35:
        FaultArray = "Lightning protection fault";
        break;
    case 0x36:
        FaultArray = "Electromagnetic lock fault";
        break;
    case 0x37:
        FaultArray = "Auxiliary power fault";
        break;
    case 0x38:
        FaultArray = "Fault of fan for charging module";
        break;
    case 0x39:
        FaultArray = "DC output overvoltage";
        break;
    case 0x3A:
        FaultArray = "DC output undervoltage";
        break;
    case 0x3B:
        FaultArray = "DC output overcurrent";
        break;
    case 0x3C:
        FaultArray = "DC output reverse connection";
        break;
    case 0x3D:
        FaultArray = "Charging module input overvoltage";
        break;
    case 0x3E:
        FaultArray = "Charging module input undervoltage";
        break;
    case 0x3F:
        FaultArray = "AC input overfrequency";
        break;
    case 0x40:
        FaultArray = "AC input underfrequency";
        break;
    case 0x41:
        FaultArray = "AC input voltage unbalance";
        break;
    case 0x42:
        FaultArray = "AC input phase A (default)";
        break;
    case 0x43:
        FaultArray = "AC input phase B (default)";
        break;
    case 0x44:
        FaultArray = "AC input phase C (default)";
        break;
    case 0x45:
        FaultArray = "AC input overload";
        break;
    case 0x46:
        FaultArray = "AC input abnormality";
        break;
    case 0x47:
        FaultArray = "Charging module output overvoltage";
        break;
    case 0x48:
        FaultArray = "Charging module overcurrent";
        break;
    case 0x49:
        FaultArray = "Charging module over-temperature";
        break;
    case 0x4A:
        FaultArray = "Higher ambient temperature";
        break;
    case 0x4B:
        FaultArray = "Lower ambient temperature";
        break;
    case 0x4C:
        FaultArray = "Communication failure of charging module";
        break;
    case 0x4D:
        FaultArray = "Failed to execute charging module command";
        break;
    case 0x4E:
        FaultArray = "Communication failure of charging controller";
        break;
    case 0x4F:
        FaultArray = "Communication failure of acquisition board";
        break;
    case 0x50:
        FaultArray = "Electric meter communication abnormality";
        break;
    case 0x51:
        FaultArray = "Failure of communication with central controller";
        break;
    case 0x52:
        FaultArray = "Reader communication failure";
        break;
    case 0x53:
        FaultArray = "Insulation fault";
        break;
    case 0x54:
        FaultArray = "Inconsistent module types";
        break;
    case 0x55:
        FaultArray = "Emergency stop";
        break;
    case 0x56:   //单桩使用
        FaultArray = "Master-slave communication failure";
        break;
    case 0x57:
        FaultArray = "Electric meter calibration error";
        break;
    case 0x58:
        FaultArray = "Door contact fault";
        break;
    case 0x59:
        FaultArray = "System fan fault";
        break;
    case 0x5A:
        FaultArray = "Paralleled contactor fault";
        break;
    case 0x5b:
        FaultArray = "Paralleled contactor drive failure alarm";
        break;
    case 0x5c:
        FaultArray = "Insulation detection alarm";
        break;
    case 0x5F:
        FaultArray = "CCU ID repetition";
        break;
    //  GuoCheng Add Begin
    case 0x60:
        FaultArray = "BMS data exception";
        break;
    case 0x61:
        FaultArray = "Cell overvoltage";
        break;
    case 0x62:
        FaultArray = "Battery pack overvoltage";
        break;
    case 0x63:
        FaultArray = "Battery pack overcurrent";
        break;
    case 0x64:
        FaultArray = "Battery overcharge";
        break;
    case 0x65:
        FaultArray = "Battery voltage abnormality";
        break;
    case 0x66:
        FaultArray = "BMS under-temperature";
        break;
    case 0x67:
        FaultArray = "BMS thermal runaway";
        break;
    case 0x68:
        FaultArray = "BMS auxiliary source abnormality";
        break;
    case 0x69:
        FaultArray = "BMS auxiliary source abnormality";
        break;
    case 0x6A:
        FaultArray = "Relay open-circuit";
        break;
    //  GuoCheng Add End
    case 0x70:
        FaultArray = "Battery overvoltage";
        break;
    case 0x71:
        FaultArray = "Higher battery SOC";
        break;
    case 0x72:
        FaultArray = "Lower battery SOC";
        break;
    case 0x73:
        FaultArray = "Battery charging overcurrent";
        break;
    case 0x74:
        FaultArray = "Battery over-temperature";
        break;
    case 0x75:
        FaultArray = "Battery insulation fault";
        break;
    case 0x76:
        FaultArray = "Battery output connector abnormality";
        break;
    case 0x90:
        FaultArray = "ZIGBEE module off-line";
        break;
    case 0x91:
        FaultArray = "Stop buffer voltage alarm";
        break;
        //新增故障数据 smm 2017-08-12
      case 0xa0:
          FaultArray = "RJH1 internal fan fault";
          break;
      case 0xa1:
          FaultArray = "RJH1 external fan fault";
          break;
      case 0xa2:
          FaultArray = "RJH1 temperature sensor fault";
          break;
      case 0xa3:
          FaultArray = "RHJ1 overvoltage fault";
          break;
      case 0xa4:
          FaultArray = "RJH1 undervoltage fault";
          break;
      case 0xa5:
          FaultArray = "RJH1 heater fault";
          break;
      case 0xa6:
          FaultArray = "RJH1 communication failure";
          break;
      case 0xa7:
          FaultArray = "RJH2 internal fan fault";
          break;
      case 0xa8:
          FaultArray = "RJH2 external fan fault";
          break;
      case 0xa9:
          FaultArray = "RJH2 temperature sensor fault";
          break;
      case 0xaa:
          FaultArray = "RJH2 overvoltage fault";
          break;
      case 0xab:
          FaultArray = "RJH2 undervoltage fault";
          break;
      case 0xac:
          FaultArray = "RJH2 heater fault";
          break;
      case 0xad:
          FaultArray = "RJH2 communication failure";
          break;
        //预留部分空
      case 0xb0:
          FaultArray = "Humidity alarm fault";
          break;
      case 0xb1:
          FaultArray = "Gun head overtemperature alarm";
          break;
    default:
        FaultArray = "Undefined fault";
        break;
    }
}

//转换  设备中止原因为字符串
void cLCDScreenProtocol::ChangeDevStopReasonToArray(unsigned char ucStopReason, QByteArray &Array)
{
    switch(ucStopReason)
    {
    case 1:
        Array = "BMS中止";
        break;
    case 2:
        Array = "平台中止";
        break;
    case 3:
        Array = "人工设定条件中止";
        break;
    case 4:
        Array = "手动点击直流机界面中止";
        break;
    case 5:
        Array = "充电机故障中止";
        break;
    case 6:
        Array = "连接器异常中止";
        break;
    case 7:
        Array = "连接器拔出中止";
        break;
    case 8:
        Array = "强制开关中止";
        break;
    case 9:
        Array = "主接触器异常中止";
        break;
    case 10:
        Array = "辅助接触器异常中止";
        break;
    case 11:
        Array = "电子锁异常中止";
        break;
    case 12:
        Array = "充电电压异常中止";
        break;
    case 13:
        Array = "充电电流异常中止";
        break;
    case 14:
        Array = "充电电流不匹配中止";
        break;
    case 15:
        Array = "充电电压不匹配中止";
        break;
    case 16:
        Array = "控制器通信故障中止";
        break;
    case 17:
        Array = "电能表通信故障中止";
        break;
    case 18:
        Array = "后台通讯中止";
        break;
    case 19:
        Array = "SOC满中止";
        break;
    case 20:
        Array = "系统模式类型转换中止";
        break;
    case 21:
        Array = "断电中止";
        break;
    case 22:
        Array = "主动防护电池过温中止";
        break;
    case 23:
        Array = "主动防护电池低温中止";
        break;
    case 24:
        Array = "主动防护电池热失控中止";
        break;
    case 25:
        Array = "主动防护电池过充中止";
        break;
    case 26:
        Array = "BMS辅助电源异常中止";
        break;
    case 27:
        Array = "BMS接触器开路故障中止";
        break;
    case 28:
        Array = "主动防护BMS数据超范围中止";
        break;
    case 29: //nihai add 2017-05-11 ,中止原因遗漏
        Array = "PDU过温中止";
        break;
    case 30:
        Array = "主动防护电池端口电压异常中止";
        break;
    case 31:
        Array = "车辆控制器故障中止";
        break;
    case 32:
        Array = "导引电路电压跳变中止";
        break;
    case 33:
        Array = "拔枪中止";
        break;
    case 34:
        Array = "设备过温中止";
        break;
    case 35:
        Array = "过流中止";
        break;
    case 36:
        Array = "过压中止";
        break;
    case 37:
        Array = "欠压中止";
        break;
    case 38:
        Array = "CP线低压故障中止";
        break;
    case 39:
        Array = "漏电故障中止";
        break;
    case 40:
        Array = "充电枪座过温中止";
        break;
    case 41:
        Array = "继电器异常中止";
        break;
    case 42:
        Array = "CAN地址冲突中止";
        break;
    case 51:
        Array = "主动防护电池单体过压中止";
        break;
    case 52:
        Array = "主动防护电池整包过压中止";
        break;
    case 53:
        Array = "电表校验错误中止";
        break;
    case 54:
        Array = "BMS通讯超时中止";
        break;
    case 55:
        Array = "电量不能提供中止";
        break;
    case 56:
        Array = "风扇故障中止";
        break;
    case 57:
        Array = "急停中止";
        break;
    case 58:
        Array = "充电机绝缘异常中止";
        break;
    case 59:
        Array = "充电机无可用模块";
        break;
    case 60:
        Array = "车辆BMS粘连中止";
        break;
    case 61:
        Array = "未获取车辆VIN码或车牌号中止";
        break;
    case 62:
        Array = "熔断器故障中止";
        break;
    case 63:
        Array = "模块匹配异常中止";
        break;
    case 66:
        Array = "车辆电池达到目标SOC中止";
        break;
    case 67:
        Array = "车辆达到总电压目标值中止";
        break;
    case 68:
        Array = "车辆达到单体电压目标值中止";
        break;
    case 69:
        Array = "车辆bms绝缘故障中止";
        break;
    case 70:
        Array = "车辆输出连接器过温故障中止";
        break;
    case 71:
        Array = "车辆BMS元件、输出连接器过温中止";
        break;
    case 72:
        Array = "车辆充电连接器故障中止";
        break;
    case 73:
        Array = "车辆电池组温度过高故障中止";
        break;
    case 74:
        Array = "车辆高压继电器故障中止";
        break;
    case 75:
        Array = "车辆监测点CC2电压检测故障中止";
        break;
    case 76:
        Array = "车辆其他故障";
        break;
    case 77:
        Array = "车辆电流过大中止";
        break;
    case 78:
        Array = "车辆电压异常中止";
        break;
    case 79:
        Array = "BMS中止";
        break;
    case 91:
        Array = "集控显示屏中止";
        break;
    case 92:
        Array = "集控离线刷卡中止";
        break;
    case 93:
        Array = "集控电度数异常中止";
        break;
    case 94:
        Array = "集控错峰功能中止";
        break;
    case 95:
        Array = "集控门磁报警触发中止";
        break;
    default:
        Array = "未定义的中止原因";
        break;
    }
}

//转换  设备中止原因为字符串
void cLCDScreenProtocol::ChangeDevStopReasonToEnglishArray(unsigned char ucStopReason, QByteArray &Array)
{
    switch(ucStopReason)
    {
    case 1:
        Array = "BMS interruption";
        break;
    case 2:
        Array = "Platform interruption";
        break;
    case 3:
        Array = "Interruption by manual conditions setting";
        break;
    case 4:
        Array = "Interruption by DC machine interface";
        break;
    case 5:
        Array = "Charger fault interruption";
        break;
    case 6:
        Array = "Interruption due to connector abnormality";
        break;
    case 7:
        Array = "Connector pull-out interruption";
        break;
    case 8:
        Array = "Forced switch interruption";
        break;
    case 9:
        Array = "Main contactor abnormality";
        break;
    case 10:
        Array = "Auxiliary contactor abnormality";
        break;
    case 11:
        Array = "Electronic lock abnormality";
        break;
    case 12:
        Array = "Interruption due to charging voltage abnormality";
        break;
    case 13:
        Array = "Interruption due to charging current abnormality";
        break;
    case 14:
        Array = "Interruption due to charging current mismatch";
        break;
    case 15:
        Array = "Interruption due to charging voltage mismatch";
        break;
    case 16:
        Array = "Controller communication failure interruption";
        break;
    case 17:
        Array = "Meter communication failure interruption";
        break;
    case 18:
        Array = "Background communication interruption";
        break;
    case 19:
        Array = "Full SOC interruption";
        break;
    case 20:
        Array = "Interruption due to system mode conversion";
        break;
    case 21:
        Array = "Outage interruption";
        break;
    case 22:
        Array = "Interruption due to battery over-temperature";
        break;
    case 23:
        Array = "Interruption due to battery under-temperature";
        break;
    case 24:
        Array = "Interruption due to battery thermal runaway";
        break;
    case 25:
        Array = "Interruption due to battery overcharge";
        break;
    case 26:
        Array = "Interruption due to BMS power supply abnormality";
        break;
    case 27:
        Array = "Interruption due to BMS fault";
        break;
    case 28:
        Array = "Interruption due to BMS data out-of-range";
        break;
    case 29: //nihai add 2017-05-11 ,中止原因遗漏
        Array = "PDU over-temperature interruption";
        break;
    case 30:
        Array = "Interruption due to port voltage abnormality";
        break;
    case 31:
        Array = "Interruption due to vehicle controller fault";
        break;
    case 32:
        Array = "Interruption due to lead circuit voltage jump";
        break;
    case 33:
        Array = "Gun pull interruption";
        break;
    case 34:
        Array = "Equipment over-temperature interruption";
        break;
    case 35:
        Array = "Overcurrent interruption";
        break;
    case 36:
        Array = "Overvoltage interruption";
        break;
    case 37:
        Array = "Undervoltage interruption";
        break;
    case 38:
        Array = "Interruption due to CP line low-voltage fault";
        break;
    case 39:
        Array = "Interruption due to power leakage fault";
        break;
    case 40:
        Array = "Interruption due to gun over-temperature";
        break;
    case 41:
        Array = "Relay abnormality interruption";
        break;
    case 42:
        Array = "Interruption due to CAN address conflict ";
        break;
    case 51:
        Array = "Interruption due to cell overvoltage";
        break;
    case 52:
        Array = "Interruption due to battery pack overvoltage";
        break;
    case 53:
        Array = "Interruption due to meter calibration error";
        break;
    case 54:
        Array = "BMS communication timeout interruption";
        break;
    case 55:
        Array = "Interruption due to no power supply";
        break;
    case 56:
        Array = "Interruption due to fan fault";
        break;
    case 57:
        Array = "Emergency stop interruption";
        break;
    case 58:
        Array = "Interruption due to insulation abnormality";
        break;
    case 59:
        Array = "Modules unavailable for charger";
        break;
    case 60:
        Array = "Interruption due to vehicle BMS adhesion";
        break;
    case 61:
        Array = "Interruption due to failure to obtain VIN";
        break;
    case 62:
        Array = "Interruption due to fuse fault";
        break;
    case 63:
        Array = "Interruption due to module matching abnormality";
        break;
    case 66:
        Array = "Vehicle battery reaching the expected SOC";
        break;
    case 67:
        Array = "Vehicle reaching the total expected voltage";
        break;
    case 68:
        Array = "Vehicle reaching the single expected voltage";
        break;
    case 69:
        Array = "Vehicle BMS insulation fault";
        break;
    case 70:
        Array = "Vehicle output connector over-temperature fault";
        break;
    case 71:
        Array = "Vehicle output connectors over-temperature";
        break;
    case 72:
        Array = "Vehicle charging connector fault";
        break;
    case 73:
        Array = "Excessive high vehicle battery pack temperature";
        break;
    case 74:
        Array = "Interruption due to vehicle HV relay fault";
        break;
    case 75:
        Array = "Vehicle monitoring CC2 voltage detection fault";
        break;
    case 76:
        Array = "Other vehicle faults";
        break;
    case 77:
        Array = "Interruption due to high vehicle current";
        break;
    case 78:
        Array = "Interruption due to vehicle voltage abnormality";
        break;
    case 79:
        Array = "BMS interruption";
        break;
    case 91:
        Array = "Centralized controller display interruption";
        break;
    case 92:
        Array = "Centralized controller offline swipe interruption";
        break;
    case 93:
        Array = "Interruption due to abnormal kWh";
        break;
    case 94:
        Array = "Centralized control off-peak function interruption";
        break;
    case 95:
        Array = "Abort triggerd by door magnetic alarm";
        break;
    default:
        Array = "Undefined fault";
        break;
    }
}

//转换  设备管理故障代码为字符串
void cLCDScreenProtocol::ChangeFaultCodeToArray_DeviceManage(unsigned char ucFaultCode, QByteArray &FaultArray)
{
    switch(ucFaultCode)
    {
    //模块
    case 0:
        FaultArray = "无故障";
        break;
    case 1:
        FaultArray = "输出过压";
        break;
    case 2:
        FaultArray = "过温";
        break;
    case 3:
        FaultArray = "风扇故障";
        break;
    case 4:
        FaultArray = "模块EEPROM故障";
        break;
    case 5:
        FaultArray = "模块交流欠压告警";
        break;
    case 6:
        FaultArray = "模块交流不平衡告警";
        break;
    case 7:
        FaultArray = "模块交流缺相告警";
        break;
    case 8:
        FaultArray = "模块严重不均流";
        break;
    case 9:
        FaultArray = "模块ID重复";
        break;
    case 10:
        FaultArray = "模块交流过压";
        break;
    case 11:
        FaultArray = "模块PFC保护";
        break;
    case 12:
        FaultArray = "模块短路保护";
        break;
    case 13:
        FaultArray = "模块内部通信异常告警";
        break;
    case 14:
        FaultArray = "模块放电电路告警";
        break;
    case 15:
        FaultArray = "DCDC管子开路告警";
        break;
    case 16:
        FaultArray = "模块离线告警";
        break;
    //PDU
    case 31:
        FaultArray = "PDU过温";
        break;
    case 32:
        FaultArray = "风扇故障";
        break;
    case 33:
        FaultArray = "温度传感器故障";
        break;
    case 34:
        FaultArray = "PDU输出短路故障";
        break;
    case 35:
        FaultArray = "继电器异常告警";
        break;
    case 36:
        FaultArray = "电表离线";
        break;
    case 37:
        FaultArray = "BMS供电电源异常（12/24V）";
        break;
    case 38:
        FaultArray = "辅助电源异常";
        break;
    case 39:
        FaultArray = "绝缘检测异常";
        break;
    case 40:
        FaultArray = "PDU的ID重复";
        break;
    case 41:
        FaultArray = "电子锁反馈异常";
        break;
    case 42:
        FaultArray = "电表校验异常";
        break;
    case 43:
        FaultArray = "CC1电压异常告警";
        break;
    case 44:
        FaultArray = "BMS继电器粘连告警";
        break;
    case 45:
        FaultArray = "急停告警";
        break;
    case 46:
        FaultArray = "BMS数据超出范围保护";
        break;
    case 47:
        FaultArray = "电池单体过压保护";
        break;
    case 48:
        FaultArray = "PDU输出过压保护";
        break;
    case 49:
        FaultArray = "过流保护";
        break;
    case 50:
        FaultArray = "过温保护";
        break;
    case 51:
        FaultArray = "低温保护";
        break;
    case 52:
        FaultArray = "热失控保护";
        break;
    case 53:
        FaultArray = "电池电压异常保护";
        break;
    case 54:
        FaultArray = "绝缘检测故障";
        break;
    case 55:
        FaultArray = "数据上传重复保护";
        break;
    case 56:
        FaultArray = "过充保护";
        break;
    case 57:
        FaultArray = "继电器开路保护";
        break;
    case 58:
        FaultArray = "通道匹配异常";
        break;
    case 59:
        FaultArray = "熔断丝开路故障";
        break;
    case 60:
        FaultArray = "PDU离线告警";
        break;
    case 61:
        FaultArray = "PDU输出反接";
        break;
    case 62:
        FaultArray = "所有通道匹配异常";
        break;
    case 63:
        FaultArray = "充电电压不匹配";
        break;
    case 64:
        FaultArray= "电池整包过压保护";
        break;
    case 65:
        FaultArray = "充电系统不匹配";
        break;
    case 66:
        FaultArray = "电表参数不匹配";
        break;
    case 67:
        FaultArray = "PDU条码异常";
        break;

        //CCU
    case 71:
        FaultArray = "直流柜条码告警";
        break;
    case 72:
        FaultArray = "设置量错误告警";
        break;
    case 73:
        FaultArray = "系统急停告警";
        break;
    case 74:
        FaultArray = "环境温度告警";
        break;
    case 75:
        FaultArray = "通道匹配异常";
        break;
    case 76:
        FaultArray = "主从通讯异常";
        break;
    case 77:
        FaultArray = "门磁异常告警";
        break;
    case 78:
        FaultArray = "防雷器故障";
        break;
    case 79:
        FaultArray = "系统风机故障";
        break;
    case 80:
        FaultArray = "并联接触器故障";
        break;
    case 81:
        FaultArray = "系统输入欠压";
        break;
    case 82:
        FaultArray = "系统输入过压";
        break;
    case 83:
        FaultArray = "系统输入缺相";
        break;
    case 84:
        FaultArray = "系统无可用模块";
        break;
    case 85:
        FaultArray = "系统无可用终端";
        break;
    case 86:
        FaultArray = "输入接触器反馈异常";
        break;
    case 87:
        FaultArray = "充电模块混插";
        break;
    case 88:
        FaultArray = "CCU的ID重复";
        break;
    case 89:
        FaultArray = "背板不匹配";
        break;

        //BOX
    case 91:
        FaultArray = "急停告警";
        break;
    case 92:
        FaultArray = "12VPE电源故障";
        break;
    case 93:
        FaultArray = "分支1接触器故障";
        break;
    case 94:
        FaultArray = "分支2接触器故障";
        break;
    case 95:
        FaultArray = "分支3接触器故障";
        break;
    case 96:
        FaultArray = "分支4接触器故障";
        break;
    case 97:
        FaultArray = "BMS电源1失效故障";
        break;
    case 98:
        FaultArray = "BMS电源2失效故障";
        break;
    case 99:
        FaultArray = "BMS电源3失效故障";
        break;
    case 100:
        FaultArray = "BMS电源4失效故障";
        break;
    case 101:
        FaultArray = "分支箱离线告警";
        break;
    default:
        FaultArray = "未定义故障";
        break;
    }
}

//转换  设备管理故障代码为字符串
void cLCDScreenProtocol::ChangeFaultCodeToEnglishArray_DeviceManage(unsigned char ucFaultCode, QByteArray &FaultArray)
{
    switch(ucFaultCode)
    {
    //模块
    case 0:
        FaultArray = "Fault-free";
        break;
    case 1:
        FaultArray = "Output overvoltage";
        break;
    case 2:
        FaultArray = "Over-temperature";
        break;
    case 3:
        FaultArray = "Fan fault";
        break;
    case 4:
        FaultArray = "Module EEPROM fault";
        break;
    case 5:
        FaultArray = "Module AC undervoltage alarm";
        break;
    case 6:
        FaultArray = "Module AC imbalance alarm";
        break;
    case 7:
        FaultArray = "Module AC phase default alarm";
        break;
    case 8:
        FaultArray = "Serious current imbalance of module";
        break;
    case 9:
        FaultArray = "Module ID repetition";
        break;
    case 10:
        FaultArray = "Module AC overvoltage";
        break;
    case 11:
        FaultArray = "Module PFC protection";
        break;
    case 12:
        FaultArray = "Module short-circuit protection";
        break;
    case 13:
        FaultArray = "Communication Error";
        break;
    case 14:
        FaultArray = "Discharge Circuit Fault";
        break;
    case 15:
        FaultArray = "DC Tube Open Alarm";
        break;
    case 16:
        FaultArray = "Module Disconnect Alarm";
        break;
    //PDU
    case 31:
        FaultArray = "PDU Over Temperature";
        break;
    case 32:
        FaultArray = "Fan Fault";
        break;
    case 33:
        FaultArray = "Temperature Sensor Fault";
        break;
    case 34:
        FaultArray = "PDU Output Short Circuit";
        break;
    case 35:
        FaultArray = "Relay Abnormal Alarm";
        break;
    case 36:
        FaultArray = "Meter Offline";
        break;
    case 37:
        FaultArray = "BMS Power Supply Abnormal";
        break;
    case 38:
        FaultArray = "Auxiliary Power Fault";
        break;
    case 39:
        FaultArray = "Insulation Fault";
        break;
    case 40:
        FaultArray = "PDU ID Repeat";
        break;
    case 41:
        FaultArray = "Electricity Leakage Feedback Fault";
        break;
    case 42:
        FaultArray = "Metering Fault";
        break;
    case 43:
        FaultArray = "CC1 Voltage Fault";
        break;
    case 44:
        FaultArray = "BMS Relay Adhesion";
        break;
    case 45:
        FaultArray = "Emergency Stop";
        break;
    case 46:
        FaultArray = "BMS Data Exception";
        break;
    case 47:
        FaultArray = "Battery Overvoltage Protection";
        break;
    case 48:
        FaultArray = "PDU Output Overvoltage Protection";
        break;
    case 49:
        FaultArray = "Overcurrent Protection";
        break;
    case 50:
        FaultArray = "Over Temperature Protection";
        break;
    case 51:
        FaultArray = "Low Temperature Protection";
        break;
    case 52:
        FaultArray = "Thermal Runaway Protection";
        break;
    case 53:
        FaultArray = "Battery Voltage Exception Protection";
        break;
    case 54:
        FaultArray = "Insulation Detection Fault";
        break;
    case 55:
        FaultArray = "Data Uploads Protected";
        break;
    case 56:
        FaultArray = "Overcharge Protection";
        break;
    case 57:
        FaultArray = "Relay Open Circuit Protection";
        break;
    case 58:
        FaultArray = "Channel Match Exception";
        break;
    case 59:
        FaultArray = "Fuse Open Circuit Fault";
        break;
    case 60:
        FaultArray = "PDU Offline Alarm";
        break;
    case 61:
        FaultArray = "PDU Output Reverse";
        break;
    case 62:
        FaultArray = "All Channels Match Exception";
        break;
    case 63:
        FaultArray = "Charging Voltage Not Match";
        break;
    case 65:
        FaultArray = "Charging System Not Match";
        break;
    case 66:
        FaultArray = "Meter Parameters Not Match";
        break;
    case 67:
        FaultArray = "PDU Barcode Exception";
        break;

        //CCU
    case 71:
        FaultArray = "DC Cabinet Bar Code Alarm";
        break;
    case 72:
        FaultArray = "Set Amount of Error Alarm";
        break;
    case 74:
        FaultArray = "Environment Temperature Alarm";
        break;
    case 75:
        FaultArray = "Channel Match Exception";
        break;
    case 76:
        FaultArray = "Master-Slave Communication Exception";
        break;
    case 77:
        FaultArray = "Menci Abnormal Alarm";
        break;
    case 78:
        FaultArray = "Lightning Protection Fault";
        break;
    case 79:
        FaultArray = "System Fan Fault";
        break;
    case 80:
        FaultArray = "Parallel Contactor Fault";
        break;
    case 81:
        FaultArray = "System Input Undervoltage";
        break;
    case 82:
        FaultArray = "System Input Overvoltage";
        break;
    case 83:
        FaultArray = "System Input Miss";
        break;
    case 84:
        FaultArray = "System No Modules ";
        break;
    case 85:
        FaultArray = "System No Terminanl";
        break;
    case 86:
        FaultArray = "Input Contactor Feedback Error";
        break;
    case 87:
        FaultArray = "Charge Module Intermixing";
        break;
    case 88:
        FaultArray = "CCU ID Repeat";
        break;

        //BOX
    case 91:
        FaultArray = "Emergency Stop";
        break;
    case 92:
        FaultArray = "12VPE Power Fault";
        break;
    case 93:
        FaultArray = "Branch 1 Contactor Fault";
        break;
    case 94:
        FaultArray = "Branch 2 Contactor Fault";
        break;
    case 95:
        FaultArray = "Branch 3 Contactor Fault";
        break;
    case 96:
        FaultArray = "Branch 4 Contactor Fault";
        break;
    case 97:
        FaultArray = "BMS Power Supply 1 Fault";
        break;
    case 98:
        FaultArray = "BMS Power Supply 2 Fault";
        break;
    case 99:
        FaultArray = "BMS Power Supply 3 Fault";
        break;
    case 100:
        FaultArray = "BMS Power Supply 4 Fault";
        break;
    case 101:
        FaultArray = "Branch Box Off-line Alarm";
        break;
    default:
        FaultArray = "Undefined suspension";
        break;
    }
}

//获取配置信息
void cLCDScreenProtocol::QueryParamInfo()
{
    ThreePhaseTypeConfig.phaseTypeConfig.clear();
    AllTPFVConfig.tpfvConfig.clear();
    AllAmmeterConfig.ammeterConfig.clear();
    if( (!pParamSet->querySetting(&cscuSysConfig, PARAM_CSCU_SYS))
            ||(!pParamSet->querySetting(&NetConfig,PARAM_NET0))
            ||(!pParamSet->querySetting(&ServerConfig,PARAM_SERVER0))
            ||(!pParamSet->querySetting(&ChargeConfig,PARAM_CHARGE))
            ||(!pParamSet->querySetting(&AllAmmeterConfig,PARAM_AMMETER))
            ||(!pParamSet->querySetting(&IOConfig, PARAM_IO))
            ||(!pParamSet->querySetting(&ThreePhaseTypeConfig, PARAM_PHASE_TYPE))
            ||(!pParamSet->querySetting(&SmartChargeConfig, PARAM_SMARTCHARGE))
            ||(!pParamSet->querySetting(&PowerLimitConfig, PARAM_POWERLIMIT))
            ||(!pParamSet->querySetting(&AllTPFVConfig, PARAM_TPFV))
            )
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! Screen Load Param ERROR!");
    }
}

//获取CSCU版本信息
void cLCDScreenProtocol::GetFrameCSCUVersion(unsigned char * &pData, unsigned short &usLen)
{
    //分配内存
    pData = new unsigned char[sizeof(CSCUVersion_LCD)];
    memset(pData, 0, sizeof(CSCUVersion_LCD));
    CSCUVersion_LCD * pScreen = (CSCUVersion_LCD *)pData;
    QFile verFile(VERSION_FILE_NAME);
    QList<QByteArray> list;
    QByteArray arFile;
    //长度
    usLen = sizeof(CSCUVersion_LCD);
    memset(pData, 0x00, usLen);

    if(!verFile.exists())
    {
        return;
    }
    verFile.open(QIODevice::ReadOnly);
    arFile = verFile.readAll();
    list = arFile.split('\n');
    verFile.close();
    if(list.count() < 6)
    {
        return;
    }
    //赋值
    strncpy(pScreen->chCoreVer, list.at(0).data(), sizeof(pScreen->chCoreVer));
    strncpy(pScreen->chFSVer, list.at(1).data(), sizeof(pScreen->chFSVer));
    strncpy(pScreen->chCSCUVer, list.at(2).data(), sizeof(pScreen->chCSCUVer));
    strncpy(pScreen->chTEUIVer, list.at(3).data(), sizeof(pScreen->chTEUIVer));
    strncpy(pScreen->chHardVer, list.at(4).data(), sizeof(pScreen->chHardVer));
    strncpy(pScreen->chMACid, list.at(5).data(), sizeof(pScreen->chMACid));
    strncpy(pScreen->chSN,list.at(6).data(),sizeof(pScreen->chSN));   //nihai add 2017-07-29 增加产品序列号
}

//获取CSCU设置数据项
//pData: 输入帧内容, usLen: 帧长度
void cLCDScreenProtocol::GetFrameCSCUSet(unsigned char * &pData, unsigned short &usLen)
{
    //获取配置
    QueryParamInfo();
    //分配内存
    pData = new unsigned char[sizeof(CSCUSet_LCD)];
    memset(pData, 0, sizeof(CSCUSet_LCD));
    CSCUSet_LCD * pScreen = (CSCUSet_LCD *)pData;
    //长度
    usLen = sizeof(CSCUSet_LCD);
    //赋值
    memcpy(pScreen->chLocolIp, NetConfig.localIp, sizeof(pScreen->chLocolIp));
    memcpy(pScreen->chMask, NetConfig.netMask, sizeof(pScreen->chMask));
    memcpy(pScreen->chGateWay, NetConfig.gateway, sizeof(pScreen->chGateWay));
    memcpy(pScreen->chDNS, cscuSysConfig.dns, sizeof(pScreen->chDNS));
    memcpy(pScreen->chServerIP1, ServerConfig.serverIp1, sizeof(pScreen->chServerIP1));
    pScreen->usPortNum1 = (unsigned short)ServerConfig.serverPort1;
    memcpy(pScreen->chServerIP2, ServerConfig.serverIp2, sizeof(pScreen->chServerIP2));
    pScreen->usPortNum2 = (unsigned short)ServerConfig.serverPort2;
    memcpy(pScreen->chServerIP3, ServerConfig.serverIp3, sizeof(pScreen->chServerIP3));
    pScreen->usPortNum3 = (unsigned short)ServerConfig.serverPort3;
    memcpy(pScreen->chStationAddr, ServerConfig.stationNo, sizeof(pScreen->chStationAddr));
    pScreen->ucZigbeeID = 0;
    pScreen->ucACThrNum = cscuSysConfig.threePhase;
    pScreen->ucACSinNum = cscuSysConfig.singlePhase;
    pScreen->ucDCNum = cscuSysConfig.directCurrent;
}

//获取三相相别设置数据项
//pData: 输入帧内容, usLen: 帧长度
bool cLCDScreenProtocol::GetFramePhaseTypeSet(unsigned char * &pData, unsigned short &usLen)
{
    //获取配置
    QueryParamInfo();
    //    if(ThreePhaseTypeConfig.phaseTypeConfig.count() == 0)
    //    {
    //        return FALSE;
    //    }
    //长度计算
    usLen = cscuSysConfig.singlePhase * 2 +1;
    //分配内存
    pData = new unsigned char[usLen];
    memset(pData, 0, usLen);

    //三相相别总记录条目数
    pData[0] = cscuSysConfig.singlePhase;

    //数据赋值
    for(unsigned char i = 0 ; i < cscuSysConfig.singlePhase; i++)
    {
        pData[i*2 + 1] = i + 1;
        if(i < ThreePhaseTypeConfig.phaseTypeConfig.count())
        {
            pData[i*2 + 2] = ThreePhaseTypeConfig.phaseTypeConfig.at(i).type;
        }
        else
        {
            pData[i*2 + 2] = 0;
        }
    }
    return TRUE;
}

//获取  屏幕密码数据项
bool cLCDScreenProtocol::GetFramePasswd(unsigned char * &pData, unsigned short &usLen)
{
    //获取配置
    QueryParamInfo();
    //分配内存
    pData = new unsigned char[4];
    //长度计算
    usLen = 4;
    memset(pData, 0, 4);
    //数据赋值
    memcpy(pData, (unsigned char *)&cscuSysConfig.password, 4);

    return TRUE;
}

//获取  IO常开常闭设置数据项
bool cLCDScreenProtocol::GetFrameIOState(unsigned char * &pData, unsigned short &usLen)
{
    //获取配置
    QueryParamInfo();
    //分配内存
    pData = new unsigned char[10];
    //长度计算
    usLen = 10;

    //数据赋值
    memcpy(pData, IOConfig.inOpenColse, 10);

    return TRUE;
}

//获取  电表地址设置数据项
bool cLCDScreenProtocol::GetFrameAmmeterAddr(unsigned char * &pData, unsigned short &usLen)
{
    //获取配置
    QueryParamInfo();
    if(AllAmmeterConfig.ammeterConfig.count() == 0)
    {
        return FALSE;
    }
    //分配内存
    pData = new unsigned char[AllAmmeterConfig.ammeterConfig.count() * 6 +1];
    //长度计算
    usLen = AllAmmeterConfig.ammeterConfig.count() * 6 +1;

    memset(pData, 0, AllAmmeterConfig.ammeterConfig.count() * 6 +1);

    //总记录条目数
    pData[0] = AllAmmeterConfig.ammeterConfig.count();

    //数据赋值
    for(unsigned char i = 0 ; i < AllAmmeterConfig.ammeterConfig.count(); i++)
    {
        memcpy(pData + i*6 + 1, AllAmmeterConfig.ammeterConfig.at(i).addr, 6);
    }
    return TRUE;
}

//获取错峰功能设置数据项
bool cLCDScreenProtocol::GetFramePeakSet(unsigned char * &pData, unsigned short &usLen)
{
    //获取配置
    QueryParamInfo();
    PeakSet_LCD * pLCD;
    //    if(AllTPFVConfig.tpfvConfig.count() == 0)
    //    {
    //        return FALSE;
    //    }

    //分配内存
    pData = new unsigned char[AllTPFVConfig.tpfvConfig.count() * sizeof(PeakSet_LCD) +2];
    //长度
    usLen = AllTPFVConfig.tpfvConfig.count() * sizeof(PeakSet_LCD) +2;

    memset(pData, 0, usLen);

    //错峰充电功能开启关闭
    pData[0] = SmartChargeConfig.sSmartCharge_Enable;
    //错峰充电总记录条目数
    pData[1] = AllTPFVConfig.tpfvConfig.count();
    //错峰充电数据赋值
    for(unsigned char i = 0 ; i < AllTPFVConfig.tpfvConfig.count(); i++)
    {
        pLCD = (PeakSet_LCD *)(pData + i * sizeof(PeakSet_LCD) + 2);
        ChangePeakFormat(pLCD, AllTPFVConfig.tpfvConfig.at(i));
    }
    return TRUE;
}

//获取 终端参数设置数据项
bool cLCDScreenProtocol::GetFrameGunArg(unsigned char * &pData, unsigned short &usLen, unsigned char ucCanID)
{
    //长度
    usLen = sizeof(DCTermParamSet_LCD);
    //分配内存
    pData = new unsigned char[usLen];
    DCTermParamSet_LCD * pScreen = (DCTermParamSet_LCD *)pData;
    db_result_st dbst;
    QString todo;
    int iTemp = 0;

    memset(pData, 0, usLen);
    pScreen->ucCanID = ucCanID;
    pScreen->ucTermID = ucCanID;
    //数据库操作----查询静态参数
    todo = "SELECT eleclock_type, vin_enable, auxpower_type, eleclock_enable, max_gun_current, bms_pro_type FROM dcstatic_param_table WHERE canaddr = "+ QString::number(ucCanID, 10);
    if(0 != pDBOperate->DBSqlQuery(todo.toAscii().data(), &dbst, DB_PARAM))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! ParseTermStaticArg Query Error");
        return FALSE;
    }
    if(dbst.row != 0)
    {
        pScreen->ucElecLockType = (unsigned char) (atoi(dbst.result[0]) + 1);
        pScreen->ucVINEnableFlag = (unsigned char) (atoi(dbst.result[1]) + 1);
        pScreen->ucAuxPowerSet = (unsigned char) (atoi(dbst.result[2]) + 1);
        pScreen->ucElecLockEnableFlag = (unsigned char) (atoi(dbst.result[3]) + 1);
        pScreen->usGunMaxCurrent = 0 - atof(dbst.result[4]);
        //新老国标赋值
        iTemp = atoi(dbst.result[5]);
        if(iTemp == 0)
        {
            pScreen->ucBMSProType = 2;
        }
        else if(iTemp == 1)
        {
            pScreen->ucBMSProType = 1;
        }
        else if(iTemp ==2 )
        {
            pScreen->ucBMSProType = 3; //nihai add
        }
    }
    pDBOperate->DBQueryFree(&dbst);

    //数据库操作----查询动态参数
    todo = "SELECT work_type, work_status, strategy, priority_level FROM dcdynamic_param_table WHERE canaddr = "+ QString::number(ucCanID, 10);
    if(0 != pDBOperate->DBSqlQuery(todo.toAscii().data(), &dbst, DB_PARAM))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! ParseTermDynamicArg Query Error");
        return FALSE;
    }
    if(dbst.row != 0)
    {
        //        pScreen->ucGroupStrategy = (unsigned char) atoi(dbst.result[0]);
        pScreen->ucGroupType = (unsigned char) atoi(dbst.result[1]);
        pScreen->ucGroupStrategy = (unsigned char) atoi(dbst.result[2]);
        pScreen->ucPriority = (unsigned char) atoi(dbst.result[3]);
    }
    pDBOperate->DBQueryFree(&dbst);
    usleep(100*1000);  //等待数据库数据插入，数据库更新时，先将数据删除，再增加，因此查询时需要等待数据插入
    //数据库操作----查询主动防护参数
    todo = "SELECT lowtemp_disable FROM activeprotection_param_table WHERE canaddr = "+ QString::number(ucCanID, 10);
    if(0 != pDBOperate->DBSqlQuery(todo.toAscii().data(), &dbst, DB_PARAM))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! ParseTermStaticArg Query Error");
        return FALSE;
    }
    if(dbst.row != 0)
    {
        pScreen->ucNoBelowTempProtect = (unsigned char) atoi(dbst.result[0]) + 1;
    }
    pDBOperate->DBQueryFree(&dbst);

    return TRUE;
}

//获取 CCU参数设置数据项
bool cLCDScreenProtocol::GetFrameCCUArg(unsigned char * &pData, unsigned short &usLen, unsigned char ucCanID)
{
    //长度
    usLen = sizeof(DCCCUParamSet_LCD);
    //分配内存
    pData = new unsigned char[usLen];
    DCCCUParamSet_LCD * pScreen = (DCCCUParamSet_LCD *)pData;
    db_result_st dbst;
    QString todo;

    memset(pData, 0, usLen);
    //数据库操作----查询静态参数
    todo = "SELECT canaddr, start_addr, max_power FROM ccu_param_table WHERE canaddr = "+ QString::number(ucCanID, 10);
    if(0 != pDBOperate->DBSqlQuery(todo.toAscii().data(), &dbst, DB_PARAM))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! GetFrameCCUArg Query Error");
        return FALSE;
    }
    if(dbst.row != 0)
    {
        pScreen->ucCanID = (unsigned char) atoi(dbst.result[0]);
        pScreen->ucTermStartID = (unsigned char) atoi(dbst.result[1]);
        pScreen->usCabMaxPower = (unsigned short) atof(dbst.result[2]);
        pScreen->ucCCUID = pScreen->ucCanID;
    }
    pDBOperate->DBQueryFree(&dbst);
    return TRUE;
}

//获取直流柜某参数类型设备个数
bool cLCDScreenProtocol::GetDCCabArgTypeNum(unsigned char * &pData, unsigned short &usLen, unsigned char ucType)
{
    QList<SpecificInfo_DCcab> list;
    queryDBSpecInfoRecord(list);
    QList<unsigned char >canIDlist, sendList;
    TerminalDataMap::iterator it;
    TerminalDataMap tempMap;
    switch(ucType)
    {
    case 1: //终端个数
    {
        QVariant var;
        pDevCache->QueryDCCabinetMeter(0, " ", var, CACHE_TERMDATA);
        tempMap = var.value<TerminalDataMap>();
        for(it = tempMap.begin(); it != tempMap.end(); ++it)
        {
            canIDlist.append(it.value());
        }
        break;
    }
    case 2: //CCU个数
    {
        for(int i = 0 ; i < list.count(); i++)
        {
            if( (list.at(i).ucDevID >= ID_MinCCUCanID) && (list.at(i).ucDevID <= ID_MaxCCUCanID))
            {
                canIDlist.append(list.at(i).ucDevID);
            }
        }
        break;
    }
    default:
        break;
    }
    //排序
    for(int i = 0 ; i < canIDlist.count(); i++)
    {
        unsigned char ucMin = canIDlist.at(i);
        for(int j = i ; j < canIDlist.count(); j++)
        {
            if(ucMin > canIDlist.at(j))
            {
                ucMin = canIDlist.at(j);
            }
        }
        sendList.append(ucMin);
    }

    usLen = sendList.count() + 2;
    pData = new unsigned char[usLen];
    pData[0] = ucType;
    pData[1] = sendList.count();
    for(int i = 0 ; i < sendList.count(); i++)
    {
        pData[i+2] = sendList.at(i);
    }
   return TRUE;
}

//获取直流机设备个数
bool cLCDScreenProtocol::GetDCCabDevNum(unsigned char * &pData, unsigned short &usLen, unsigned char ucType)
{
    QList<SpecificInfo_DCcab> list, selist, sortlist;
    queryDBSpecInfoRecord(list);

    switch(ucType)
    {
    case 1: //CCU个数
    {
        for(int i = 0 ; i < list.count(); i++)
        {
            if( (list.at(i).ucDevID >= ID_MinCCUCanID) && (list.at(i).ucDevID <= ID_MaxCCUCanID))
            {
                selist.append(list.at(i));
            }
        }
        break;
    }
    case 2: //PDU个数
    {
        for(int i = 0 ; i < list.count(); i++)
        {
            if( (list.at(i).ucDevID >= InnerID_MinPDU) && (list.at(i).ucDevID <= InnerID_MaxPDU))
            {
                selist.append(list.at(i));
            }
        }
        break;
    }
    case 3: //模块
    {
        for(int i = 0 ; i < list.count(); i++)
        {
            if( (list.at(i).ucDevID >= InnerID_MinMod) && (list.at(i).ucDevID <= InnerID_MaxMod))
            {
                selist.append(list.at(i));
            }
        }
        break;
    }
    default:
        break;
    }

//    //给设备内部ID进行排序
//    for(int i = 0; i < selist.count(); i++)
//    {
//        SpecificInfo_DCcab stTemp = selist[i];
//        for(int j = i; j < selist.count(); j++)
//        {
//            if(stTemp.ucDevID > selist.at(j).ucDevID)
//            {
//                stTemp = selist.at(j);
//            }
//        }
//        sortlist.append(stTemp);
//    }

//    //赋值
//    usLen = 2*sortlist.count() + 2;
//    pData = new unsigned char[usLen];
//    pData[0] = ucType;
//    pData[1] = sortlist.count();

//    for(int i = 0, j = 0 ; i < sortlist.count(); i++)
//    {
//        j = 2 * i;
//        pData[ j + 2] = sortlist.at(i).ucCCUAddr;
//        pData[ j + 3] = sortlist.at(i).ucDevID;
//    }
    //赋值
    usLen = 2*selist.count() + 2;
    pData = new unsigned char[usLen];
    pData[0] = ucType;
    pData[1] = selist.count();

    for(int i = 0, j = 0 ; i < selist.count(); i++)
    {
        j = 2 * i;
        pData[ j + 2] = selist.at(i).ucCCUAddr;
        pData[ j + 3] = selist.at(i).ucDevID;
    }
    return TRUE;
}

//获取二维码设置数据项
bool cLCDScreenProtocol::GetFrame2DbarCodes(unsigned char * &pData, unsigned short &usLen)
{
    db_result_st dbst;
    Node_2DbarCodeRecord_LCD tempNode;
    QList <Node_2DbarCodeRecord_LCD> barCodeList;
    Node_2DbarCodeRecord_LCD * pNode;
    TermNameMap::iterator it;
    //获取配置
    //UpdateOperateDB(01, "屏幕设置", "获取二维码设置");  //nihai add 删除获取二维码的操作日志记录
    if(0 != pDBOperate->DBSqlQuery("SELECT canaddr, code FROM terminal_code_param_table", &dbst, DB_PARAM))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! terminal_code_param_table Query Error");
    }
    else
    {
        for(int i = 0; i < dbst.row; i++)
        {
            memset(&tempNode, 0x00, sizeof(tempNode));
            tempNode.ucCanID = atoi(dbst.result[i * dbst.column]);
            strncpy(tempNode.chCode, dbst.result[i * dbst.column + 1], sizeof(tempNode.chCode));
            barCodeList.append(tempNode);
        }
    }
    pDBOperate->DBQueryFree(&dbst);
    if(NameMap.isEmpty())
    {
    }
    //如果数据库为空,则按照配置文件中各终端类型数量填充数据
    if(barCodeList.isEmpty())
    {
        memset(tempNode.chCode, 0x00, sizeof(tempNode.chCode));
        for(it = NameMap.begin() ; it!= NameMap.end() ; ++it)
        {
            tempNode.ucCanID = it.key();
            barCodeList.append(tempNode);
        }
    }
    //组帧赋值
    usLen = sizeof(Node_2DbarCodeRecord_LCD) * barCodeList.count() + 1;
    pData = new unsigned char[usLen];
    memset(pData, 0x00, usLen);

    pData[0] = barCodeList.count();
    for(int i = 0 ; i < barCodeList.count(); i++)
    {
        pNode = (Node_2DbarCodeRecord_LCD *)(pData + i*sizeof(Node_2DbarCodeRecord_LCD) + 1);
        strncpy(pNode->chCode, barCodeList.at(i).chCode, sizeof(pNode->chCode));
        pNode->ucCanID = barCodeList.at(i).ucCanID;
    }
    return TRUE;
}

//获取系统特殊功能设置数据项
void cLCDScreenProtocol::GetFrameSysSpecSet(unsigned char * &pData, unsigned short &usLen)
{
    //获取配置
    QueryParamInfo();
    //分配内存
    pData = new unsigned char[sizeof(SysSpecSet_LCD)];
    memset(pData, 0, sizeof(SysSpecSet_LCD *));
    SysSpecSet_LCD * pScreen = (SysSpecSet_LCD *)pData;
    //长度
    usLen = sizeof(SysSpecSet_LCD);
    pScreen->ucVINOffline = ChargeConfig.vinOffline;   //本地IP
    pScreen->ucLocalStop = ChargeConfig.localStop;    //本地结束
    pScreen->ucCardType = ChargeConfig.cardType;    //卡类型
    pScreen->ucVINAuto = ChargeConfig.vinAuto;    //vin自动充电
    pScreen->ucCardAuto = ChargeConfig.cardAuto;    //刷卡自动充电
    pScreen->ucVINType = ChargeConfig.vinType;    //vin或车牌号
    pScreen->ucEnergyFilter = ChargeConfig.energyFilter;    //异常电度数过滤
    pScreen->ucLocalChargeEnable = ChargeConfig.localChargeEnable;    //本地充电使能开关
    pScreen->ucLocalChargeType = ChargeConfig.localChargeType;    //本地充电方式选择
    pScreen->ucDevType = ChargeConfig.ucDevType;  //nihai add
    pScreen->coupleGun = ChargeConfig.coupleGun;   //add by songqb
    pScreen->ucticketEnable = ChargeConfig.ticketEnable; //小票机选择
    pScreen->ucLanguage = ChargeConfig.languageType;
}

//获取直流特殊功能设置数据项
void cLCDScreenProtocol::GetFrameDCSpecSet(unsigned char * &pData, unsigned short &usLen)
{
    Node_DCSpecSet_LCD tempNode;
    QList <Node_DCSpecSet_LCD> DBList;
    db_result_st dbst;
    QString todo;

    for(unsigned char i = 0; i < cscuSysConfig.directCurrent; i++)
    {
        tempNode.ucCanAddr = ID_MinDCCanID + i;

        //数据库操作----查询静态参数
        todo = "SELECT auxpower_type  FROM dcstatic_param_table WHERE canaddr = "+ QString::number(tempNode.ucCanAddr, 10);
        if(0 != pDBOperate->DBSqlQuery(todo.toAscii().data(), &dbst, DB_PARAM))
        {
            pLog->getLogPoint(_strLogName)->info("ERROR! ParseTermStaticArg Query Error");
        }
        if(dbst.row != 0)
        {
            tempNode.ucAuxType = (unsigned char) atoi(dbst.result[0]);
        }
        pDBOperate->DBQueryFree(&dbst);
        //数据库操作----查询动态参数
        todo = "SELECT work_status, strategy FROM dcdynamic_param_table WHERE canaddr = "+ QString::number(tempNode.ucCanAddr, 10);
        if(0 != pDBOperate->DBSqlQuery(todo.toAscii().data(), &dbst, DB_PARAM))
        {
            pLog->getLogPoint(_strLogName)->info("ERROR! ParseTermDynamicArg Query Error");
        }
        if(dbst.row != 0)
        {
            tempNode.ucTermWorkState = (unsigned char) atoi(dbst.result[0]);
            tempNode.ucGroupStrategy = (unsigned char) atoi(dbst.result[1]);
        }
        pDBOperate->DBQueryFree(&dbst);
        DBList.append(tempNode);
    }
    //长度
    usLen = sizeof(Node_DCSpecSet_LCD) * DBList.count() + 1;
    //分配内存
    pData = new unsigned char[usLen];
    memset(pData, 0, usLen);
    for(unsigned char i = 0; i < DBList.count(); i++)
    {
        Node_DCSpecSet_LCD * pScreen = (Node_DCSpecSet_LCD *)(pData + i * sizeof(Node_DCSpecSet_LCD) + 1);
        pScreen->ucCanAddr = DBList.at(i).ucCanAddr;
        pScreen->ucAuxType = DBList.at(i).ucAuxType;
        pScreen->ucGroupStrategy = DBList.at(i).ucGroupStrategy;
        pScreen->ucTermWorkState = DBList.at(i).ucTermWorkState;
    }
}

//获取负荷约束功能设置数据项
void cLCDScreenProtocol::GetFrameLoadLimit(unsigned char * &pData, unsigned short &usLen)
{
    //获取配置
    QueryParamInfo();
    //分配内存
    pData = new unsigned char[sizeof(LoadLimit_LCD)];
    memset(pData, 0, sizeof(LoadLimit_LCD));
    LoadLimit_LCD * pScreen = (LoadLimit_LCD *)pData;
    //长度
    usLen = sizeof(LoadLimit_LCD);

    pScreen->ucLoadLimit = PowerLimitConfig.sPowerLimit_Enable; //负荷约束使能
    pScreen->ucCCUNum = PowerLimitConfig.sCCUcount;   //集控下CCU数量
    pScreen->usTotalPower = ((unsigned short)PowerLimitConfig.STATION_LIMT_POWER);    //场站总限制功率
    pScreen->usSecurePower = ((unsigned short)PowerLimitConfig.SAFE_CHARGE_POWER);    //场站安全充电功率
    pScreen->usLimitPower = ((unsigned short)PowerLimitConfig.sSUMPower_Manual);    //手动设置充电限制功率
    pScreen->ucDynamicEnable = PowerLimitConfig.sSUMPower_Ammeter_Enable;    //电表动态计算设置限制功率
    pScreen->ucLocalEnable = PowerLimitConfig.sSUMPower_Manual_Enable;    //点屏设置限制功率
    pScreen->ucRemoteEnable = PowerLimitConfig.sSUMPower_Server_Enable;    //服务器下发设置限制功率
}

//获取本地充电密码数据项
void cLCDScreenProtocol::GetFrameLocalChargePassword(unsigned char * &pData, unsigned short &usLen)
{
    //获取配置
    QueryParamInfo();
    //分配内存
    usLen = 4;
    pData = new unsigned char[usLen];
    memset(pData, 0, 4);
    *(int *)pData = cscuSysConfig.localChargePassword;
}

//获取  进线侧数据项
bool cLCDScreenProtocol::GetFrameAmmeterData(unsigned char * &pData, unsigned short &usLen, QByteArray &AddrArray)
{
    stAmmeterData ammeterData;
    QVariant Var;
    QVariant Param;

    Param.setValue(AddrArray);
    //获取配置
    QueryParamInfo();
    if(AllAmmeterConfig.ammeterConfig.isEmpty())
    {
        return FALSE;
    }
    if(!pDevCache->QueryRealStatusMeter(Var, CACHE_INLINE_AMMETER, Param))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! SendMainPageData  QueryRealStatusMeter Error!");
        return FALSE;
    }
    ammeterData = Var.value<stAmmeterData>();

    //分配内存
    pData = new unsigned char[sizeof(FrameAmmeterData_LCD)];
    memset(pData, 0, sizeof(FrameAmmeterData_LCD));
    FrameAmmeterData_LCD * pScreen = (FrameAmmeterData_LCD *)pData;
    //长度
    usLen = sizeof(FrameAmmeterData_LCD);

    //数据赋值
    memcpy(pScreen->chAddr, AddrArray.data(), 6);

    pScreen->A_voltage = ammeterData.Vol_A *10;
    pScreen->A_current = ammeterData.Cur_A*100;
    pScreen->B_voltage = ammeterData.Vol_B *10;
    pScreen->B_current = ammeterData.Cur_B*100;
    pScreen->C_voltage = ammeterData.Vol_C *10;
    pScreen->C_current = ammeterData.Cur_C*100;
    pScreen->reactive_power = ammeterData.TotalRePower*100;  //nihai add 20170601  增加总无功功率
    pScreen->active_power = ammeterData.TotalPower*100;
    pScreen->power_factor = ammeterData.PowerFactor*1000;
    pScreen->ReactiveSensibilityEnergy = ammeterData.ReactiveSensibilityEnergy *100;
    pScreen->ReactiveCapacityEnergy = ammeterData.ReactiveCapacityEnergy *100;
    pScreen->neutralLine_current = ammeterData.Cur_0 *100;
    pScreen->ActiveAbsorbEnergy = ammeterData.ActiveAbsorbEnergy *100;
    pScreen->ActiveLiberateEnergy = ammeterData.ActiveLiberateEnergy *100;
    /*  //ocean debug
           pScreen->A_voltage,pScreen->B_voltage,pScreen->C_voltage,
            pScreen->A_current, pScreen->B_current, pScreen->C_current,
           pScreen->active_power,
           pScreen->power_factor,
           pScreen->ReactiveSensibilityEnergy,
           pScreen->neutralLine_current,
           pScreen->ActiveAbsorbEnergy,
           pScreen->ActiveLiberateEnergy
           );
           */
    return TRUE;
}

//获取  环境信息
bool cLCDScreenProtocol::GetFrameEnvInfo(unsigned char * &pData, unsigned short &usLen, QByteArray &AddrArray)
{
    Q_UNUSED(AddrArray);
    unsigned short alarm1;          //告警数据1
    unsigned short alarm2;          //告警数据2
    unsigned short usTemp;        //单独一位告警器状态
    //获取配置
    QueryParamInfo();
    //分配内存
    pData = new unsigned char[sizeof(EnvInfo_LCD)];
    memset(pData, 0, sizeof(EnvInfo_LCD));
    EnvInfo_LCD * pScreen = (EnvInfo_LCD *)pData;
    //长度
    usLen = sizeof(EnvInfo_LCD);

    //数据赋值
    QVariant Var, Param;
    if(!pDevCache->QueryRealStatusMeter(Var, CACHE_STATUS, Param))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! SendMainPageData  QueryRealStatusMeter Error!");
        return FALSE;
    }
    alarm1 = Var.value<RealStatusData>().alarm1;
    alarm2 = Var.value<RealStatusData>().alarm2;
    Q_UNUSED(alarm2);
    //温湿度
    pScreen->usHumi = Var.value<RealStatusData>().humidity;
    pScreen->usTemp = Var.value<RealStatusData>().temperature;
    //告警
    for(unsigned char uc = 0; uc < 8; uc++)
    {
        usTemp = (alarm1 >> uc)&(0x0001);
        (usTemp == 0)?(pScreen->ucIOState[uc] = 0):(pScreen->ucIOState[uc] = 1);
    }
    return TRUE;
}

//获取  模块实时数据
bool cLCDScreenProtocol::GetFrameModData(unsigned char * &pData, unsigned short &usLen, unsigned char ucCanID, unsigned char ucInnerID)
{
    //长度
    usLen = sizeof(MODData_LCD);
    //分配内存
    pData = new unsigned char[usLen];
    memset(pData, 0, usLen);
    MODData_LCD * pScreen = (MODData_LCD *)pData;

    db_result_st dbst;
    QString todo;
    SpecificInfo_DCcab tempStr;
    QByteArray inID;
    QVariant var;
    inID.append((char *)&ucInnerID, 1);

    //CAN地址和内部ID赋值
    pScreen->ucCanID = ucCanID;
    pScreen->ucInnerID = ucInnerID;
    //数据库操作----查询规格参数
    todo.sprintf("SELECT slotnum, serialnum, softversion1,  softversion2, softversion3, hdwversion FROM format_data_table WHERE canaddr = %d and interid = %d ",ucCanID, ucInnerID);
    if(0 != pDBOperate->DBSqlQuery(todo.toAscii().data(), &dbst, DB_PROCESS_RECORD))
    {
        //pLog->getLogPoint(LOG_ENERGY_CONSUME_SERVER)->info("ERROR! ParseTermStaticArg Query Error");
    }
    if(dbst.row != 0)
    {
        tempStr.ucCCUAddr = ucCanID;
        tempStr.ucDevID = ucInnerID;

        tempStr.ucSlotNum = atoi(dbst.result[0]);
        tempStr.SerialNumber = dbst.result[1];
        tempStr.SoftwareVer = dbst.result[2];
        tempStr.SoftwareVer1 = dbst.result[3];
        tempStr.SoftwareVer2 = dbst.result[4];
        tempStr.HardwareVer = dbst.result[5];

        memcpy(pScreen->chHardwareVer, tempStr.HardwareVer.data(), tempStr.HardwareVer.length());
        memcpy(pScreen->chSerialNumber, tempStr.SerialNumber.data(), tempStr.SerialNumber.length());
        memcpy(pScreen->chSoftwareVer, tempStr.SoftwareVer.data(), tempStr.SoftwareVer.length());
        memcpy(pScreen->chSoftwareVer1, tempStr.SoftwareVer1.data(), tempStr.SoftwareVer1.length());
        memcpy(pScreen->chSoftwareVer2, tempStr.SoftwareVer2.data(), tempStr.SoftwareVer2.length());
    }
    pDBOperate->DBQueryFree(&dbst);
    //获取实时数据
    if(pDevCache->QueryDCCabinetMeter(ucCanID, "",var, CACHE_DCMODULEDATA)== FALSE)
    {
//        return FALSE;
        pScreen->ucWorkState = 3; //离线 nihai add
    }
    else
    {
        pScreen->sM1Temperature = var.value<stDCModuleDatasSingle>().mapSingle[inID].m1Temp;
        pScreen->ucAlarmState = var.value<stDCModuleDatasSingle>().mapSingle[inID].warning_status;
        pScreen->ucGroupNum = var.value<stDCModuleDatasSingle>().mapSingle[inID].group;

        pScreen->ucWorkState = var.value<stDCModuleDatasSingle>().mapSingle[inID].runStatus;
        pScreen->usInAVoltage = var.value<stDCModuleDatasSingle>().mapSingle[inID].inVoloatgeA*10;
        pScreen->usInBVoltage = var.value<stDCModuleDatasSingle>().mapSingle[inID].inVoloatgeB*10;
        pScreen->usInCVoltage = var.value<stDCModuleDatasSingle>().mapSingle[inID].inVoloatgeC*10;
        pScreen->sOutCurrent = var.value<stDCModuleDatasSingle>().mapSingle[inID].outCurrent*100;
        pScreen->usOutVoltage = var.value<stDCModuleDatasSingle>().mapSingle[inID].outVolatge*10;
    }


    return TRUE;
}

//获取  PDU实时数据
bool cLCDScreenProtocol::GetFramePDUData(unsigned char * &pData, unsigned short &usLen, unsigned char ucCanID, unsigned char ucInnerID)
{
    //长度
    usLen = sizeof(PDUData_LCD);
    //分配内存
    pData = new unsigned char[usLen];
    memset(pData, 0, usLen);
    PDUData_LCD * pScreen = (PDUData_LCD *)pData;

    db_result_st dbst;
    QString todo;
    SpecificInfo_DCcab tempStr;
    QByteArray inID;
    QVariant var;
    inID.append((char)ucInnerID);

    //CAN地址和内部ID赋值
    pScreen->ucCanID = ucCanID;
    pScreen->ucInnerID = ucInnerID;

    //数据库操作----查询静态参数
    todo.sprintf("SELECT slotnum, serialnum, softversion1,  softversion2, softversion3, hdwversion FROM format_data_table WHERE canaddr = %d and interid = %d ",ucCanID, ucInnerID);
    if(0 != pDBOperate->DBSqlQuery(todo.toAscii().data(), &dbst, DB_PROCESS_RECORD))
    {
        //pLog->getLogPoint(LOG_ENERGY_CONSUME_SERVER)->info("ERROR! ParseTermStaticArg Query Error");
    }
    if(dbst.row != 0)
    {
        tempStr.ucCCUAddr = ucCanID;
        tempStr.ucDevID = ucInnerID;

        tempStr.ucSlotNum = atoi(dbst.result[0]);
        tempStr.SerialNumber = dbst.result[1];
        tempStr.SoftwareVer = dbst.result[2];
        tempStr.SoftwareVer1 = dbst.result[3];
        tempStr.SoftwareVer2 = dbst.result[4];
        tempStr.HardwareVer = dbst.result[5];

        memcpy(pScreen->chHardwareVer, tempStr.HardwareVer.data(), tempStr.HardwareVer.length());
        memcpy(pScreen->chSerialNumber, tempStr.SerialNumber.data(), tempStr.SerialNumber.length());
        memcpy(pScreen->chSoftwareVer, tempStr.SoftwareVer.data(), tempStr.SoftwareVer.length());
        memcpy(pScreen->chSoftwareVer1, tempStr.SoftwareVer1.data(), tempStr.SoftwareVer1.length());
        memcpy(pScreen->chSoftwareVer2, tempStr.SoftwareVer2.data(), tempStr.SoftwareVer2.length());
    }
    pDBOperate->DBQueryFree(&dbst);

    if(pDevCache->QueryDCCabinetMeter(ucCanID, "",var, CACHE_DCPDUDATA) == FALSE)
    {
//        return FALSE;
        pScreen->ucWorkState = 2;  //离线，CCU 离线后，状态要重置为离线状态
    }
    else
    {
        pScreen->sRadiatorTemperature = var.value<stPDUDatasSingle>().mapSingle[inID].coolingTemp;
        pScreen->ucAlarmState = var.value<stPDUDatasSingle>().mapSingle[inID].warning_status;
        pScreen->ucWorkState = var.value<stPDUDatasSingle>().mapSingle[inID].runStatus;
        pScreen->sOutCurrent = var.value<stPDUDatasSingle>().mapSingle[inID].outCurrent*10; //mod by muty 20170912
        pScreen->sOutVoltage = var.value<stPDUDatasSingle>().mapSingle[inID].outVolatge*10;
    }
    return TRUE;
}

//获取  CCU实时数据
bool cLCDScreenProtocol::GetFrameCCUData(unsigned char * &pData, unsigned short &usLen, unsigned char ucCanID, unsigned char ucInnerID)
{
    //长度
    usLen = sizeof(CCUData_LCD);
    //分配内存
    pData = new unsigned char[usLen];
    memset(pData, 0, usLen);
    CCUData_LCD * pScreen = (CCUData_LCD *)pData;

    db_result_st dbst;
    QString todo;
    SpecificInfo_DCcab tempStr;
    QByteArray inID;
    QVariant var;
    inID.append((char)ucInnerID);

    //CAN地址和内部ID赋值
    pScreen->ucCanID = ucCanID;
    pScreen->ucInnerID = ucInnerID;

    //数据库操作----查询静态参数
    todo.sprintf("SELECT slotnum, serialnum, softversion1,  softversion2, softversion3, hdwversion FROM format_data_table WHERE canaddr = %d and interid = %d ",ucCanID, ucInnerID);
    if(0 != pDBOperate->DBSqlQuery(todo.toAscii().data(), &dbst, DB_PROCESS_RECORD))
    {
        //pLog->getLogPoint(LOG_ENERGY_CONSUME_SERVER)->info("ERROR! ParseTermStaticArg Query Error");
    }
    if(dbst.row != 0)
    {
        tempStr.ucCCUAddr = ucCanID;
        tempStr.ucDevID = ucInnerID;

        tempStr.ucSlotNum = atoi(dbst.result[0]);
        tempStr.SerialNumber = dbst.result[1];
        tempStr.SoftwareVer = dbst.result[2];
        tempStr.SoftwareVer1 = dbst.result[3];
        tempStr.SoftwareVer2 = dbst.result[4];
        tempStr.HardwareVer = dbst.result[5];
    }

    memcpy(pScreen->chHardwareVer, tempStr.HardwareVer.data(), tempStr.HardwareVer.length());
    memcpy(pScreen->chSerialNumber, tempStr.SerialNumber.data(), tempStr.SerialNumber.length());
    memcpy(pScreen->chSoftwareVer, tempStr.SoftwareVer.data(), tempStr.SoftwareVer.length());
    memcpy(pScreen->chSoftwareVer1, tempStr.SoftwareVer1.data(), tempStr.SoftwareVer1.length());
    memcpy(pScreen->chSoftwareVer2, tempStr.SoftwareVer2.data(), tempStr.SoftwareVer2.length());


    pDBOperate->DBQueryFree(&dbst);
    if(pDevCache->QueryDCCabinetMeter(ucCanID, "",var, CACHE_DCCCUDATA) == FALSE)
    {
//        return FALSE;
        pScreen->ucWorkState = 3;  //离线 nihai add
    }
    else
    {
        pScreen->sEnvTemperature = var.value<stCCUDatasItem>().ccuenvtemp;
        pScreen->ucAlarmState = var.value<stCCUDatasItem>().warning_status;
        pScreen->ucWorkState = var.value<stCCUDatasItem>().runStatus;
        pScreen->usNowPower = (var.value<stCCUDatasItem>().cabnowpower)*10;    //add by songqb 一位小数
    }

    return TRUE;
}

//获取  历史记录条目
bool cLCDScreenProtocol::GetFrameRecordNum(unsigned char * &pData, unsigned short &usLen, const QByteArray &AddrArray)
{
    bool bRet = FALSE;
    unsigned short usDataType = AddrArray.at(9) + AddrArray.at(10) *256;    //数据类型
    unsigned short usTotalPageNum = 0;   //总页面数
    unsigned short usTotalRecordNum = 0; //总记录条数
    switch(usDataType)
    {
    case Data_FaultRecord_LCD: //故障记录
        QueryFaultRecord();
        if(!FaultRecordList.isEmpty())
        {
            //总记录条目数
            usTotalRecordNum = FaultRecordList.count();
            //每页iFaultRecordPerPage条记录数
            (usTotalRecordNum%iFaultRecordPerPage == 0)?(usTotalPageNum = FaultRecordList.count()/iFaultRecordPerPage):(usTotalPageNum = FaultRecordList.count()/iFaultRecordPerPage + 1);
            bRet = TRUE;
        }
        break;
    case Data_ChargeRecord_LCD: //充电记录
        QueryChargeRecord();
        if(!ChargeRecordList.isEmpty())
        {
            //总记录条目数
            usTotalRecordNum = ChargeRecordList.count();
            //每页iChargeRecordPerPage条记录数
            (usTotalRecordNum%iChargeRecordPerPage == 0)?(usTotalPageNum = usTotalRecordNum/iChargeRecordPerPage):(usTotalPageNum = usTotalRecordNum/iChargeRecordPerPage + 1);
            bRet = TRUE;
        }
        break;
    case Data_OperateRecord_LCD:    //操作记录
        QueryOperateRecord();
        if(!OperateRecordList.isEmpty())
        {
            //总记录条目数
            usTotalRecordNum = OperateRecordList.count();
            //每页iOperateRecordPerPage条记录数
            (usTotalRecordNum%iOperateRecordPerPage == 0)?(usTotalPageNum = usTotalRecordNum/iOperateRecordPerPage):(usTotalPageNum = usTotalRecordNum/iOperateRecordPerPage + 1);
            bRet = TRUE;
        }
        break;
    default:
        bRet = FALSE;
        break;
    }
    if(bRet == TRUE)
    {
        //分配内存
        pData = new unsigned char[sizeof(FrameRecordNum_LCD)];
        memset(pData, 0, sizeof(FrameRecordNum_LCD *));
        FrameRecordNum_LCD * pScreen = (FrameRecordNum_LCD *)pData;
        //长度
        usLen = sizeof(FrameRecordNum_LCD);
        //赋值
        pScreen->usType = usDataType;
        pScreen->usPageNum = usTotalPageNum;
        pScreen->usRecordNum = usTotalRecordNum;
    }
    return bRet;
}

//获取  终端状态
bool cLCDScreenProtocol::GetFrameTermState(unsigned char * &pData, unsigned short &usLen, unsigned char ucCanID)
{
    TerminalStatus stTerm;
    TermNameMap NameMapTemp;
    stChargeConfig charge;
    pParamSet->querySetting(&charge, PARAM_CHARGE);

    if(! pDevCache->QueryTerminalStatus(ucCanID,stTerm))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! Screen pDevCache Query  Error!");
        return FALSE;
    }
    if(!NameMap.contains(ucCanID))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! Screen NameMap Have No Such CanAddr!");
        return FALSE;
    }
    if(NameMapShow.isEmpty() || (NameMap.size() <NameMapShow.size()) || (charge.coupleGun == 0))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! NameMapShow Empty");
        NameMapTemp = NameMap;
    }
    else
    {
         NameMapTemp = NameMapShow;
    }
    //分配内存
    pData = new unsigned char[sizeof(TermState_LCD)];
    memset(pData, 0, sizeof(TermState_LCD *));
    TermState_LCD * pScreen = (TermState_LCD *)pData;
    //长度
    usLen = sizeof(TermState_LCD);

    pScreen->ucCanAddr = ucCanID;
    pScreen->ucLinkState = stTerm.stFrameRemoteSingle.link_status;
    pScreen->ucChargeState = stTerm.stFrameRemoteSingle.charge_status;
     pScreen->ucLogicState = stTerm.cStatus;
    if(charge.coupleGun != 0)
    {
        if(stTerm.gunType == SLAVE_GUN)//副枪
        {
            pScreen->ucLogicState = CHARGE_STATUS_SLAVEGUN;
        }
        else if(stTerm.gunType == COUPLE_ERR)//配对错误
        {
            pScreen->ucLogicState = CHARGE_STATUS_COUPLE_ERR;
        }
    }
    pScreen->ucSOC = stTerm.stFrameBmsInfo.batery_SOC;
    memset(pScreen->chName, 0x00, sizeof(pScreen->chName));
    strncpy(pScreen->chName, NameMapTemp[ucCanID].data(), NameMapTemp[ucCanID].length());
    return TRUE;
}

//获取  终端遥测数据
bool cLCDScreenProtocol::GetFrameTermMeasure_Normal(unsigned char * &pData, unsigned short &usLen, unsigned char ucCanID)
{
    TerminalStatus stTerm;
    if(! pDevCache->QueryTerminalStatus(ucCanID,stTerm))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! Screen pDevCache Query  Error!");
        return FALSE;
    }
    //分配内存
    pData = new unsigned char[sizeof(TermMeasure_Normal_LCD)];
    memset(pData, 0, sizeof(TermMeasure_Normal_LCD));
    TermMeasure_Normal_LCD * pScreen = (TermMeasure_Normal_LCD *)pData;
    //长度
    usLen = sizeof(TermMeasure_Normal_LCD);

    pScreen->ucCanAddr = ucCanID;
    pScreen->ucState = stTerm.cStatus;
    //A,B,C三相电压,电流
    pScreen->A_voltage = (short)(stTerm.stFrameRemoteMeSurement1.A_voltage/0.1);
    pScreen->B_voltage = (short)(stTerm.stFrameRemoteMeSurement1.B_voltage/0.1);
    pScreen->C_voltage = (short)(stTerm.stFrameRemoteMeSurement1.C_voltage/0.1);
    pScreen->A_current = (short)(stTerm.stFrameRemoteMeSurement1.A_current/0.1);
    pScreen->B_current = (short)(stTerm.stFrameRemoteMeSurement1.B_current/0.1);
    pScreen->C_current = (short)(stTerm.stFrameRemoteMeSurement1.C_current/0.1);

    //有功功率,无功功率 功率因数
    pScreen->active_power = (short)(stTerm.stFrameRemoteMeSurement1.active_power/0.01);
    pScreen->reactive_power = (short)(stTerm.stFrameRemoteMeSurement1.reactive_power/0.01);
    pScreen->power_factor = (short)(stTerm.stFrameRemoteMeSurement1.power_factor/0.001 );
    //零线电流
    pScreen->neutralLine_current = (short)(stTerm.stFrameRemoteMeSurement1.neutralLine_current/0.1);
    //直流侧电压,电流
    pScreen->voltage_of_dc = (short)(stTerm.stFrameRemoteMeSurement1.voltage_of_dc/0.1);
    pScreen->current_of_dc = (short)(stTerm.stFrameRemoteMeSurement1.current_of_dc/0.1);
    //总有功电能,总无功电能
    pScreen->active_electric_energy = (int)(stTerm.stFrameRemoteMeSurement2.active_electric_energy);
    pScreen->reactive_electric_energy = (int)(stTerm.stFrameRemoteMeSurement2.reactive_electric_energy);
    //电压不平衡率, 电流不平衡率
    pScreen->voltage_unbalance_rate = (short)(stTerm.stFrameRemoteMeSurement1.voltage_unbalance_rate/0.01);
    pScreen->current_unbalance_rate = (short)(stTerm.stFrameRemoteMeSurement1.current_unbalance_rate/0.01);

    pScreen->gun_temp = (short)(stTerm.Qiangtoutemp/0.1);
    return TRUE;
}

//获取  终端遥测数据----刷卡版
bool cLCDScreenProtocol::GetFrameTermMeasure_Card(unsigned char * &pData, unsigned short &usLen, unsigned char ucCanID)
{
    TerminalStatus stTerm;
    CHARGE_STEP temStep;
    QDateTime StartTime, NowTime;
    if(! pDevCache->QueryTerminalStatus(ucCanID,stTerm))
    {
        return FALSE;
    }

    //相关充电状态查询
    if(!pDevCache->QueryChargeStep(stTerm.cCanAddr, temStep))
    {
        return FALSE;
    }

    //分配内存
    pData = new unsigned char[sizeof(FrameTermDetail_Card_LCD)];
    memset(pData, 0, sizeof(FrameTermDetail_Card_LCD));
    FrameTermDetail_Card_LCD * pScreen = (FrameTermDetail_Card_LCD *)pData;
    //长度
    usLen = sizeof(FrameTermDetail_Card_LCD);

    pScreen->ucCanAddr = ucCanID;
    pScreen->A_voltage = stTerm.stFrameRemoteMeSurement1.A_voltage/0.1;
    pScreen->B_voltage = stTerm.stFrameRemoteMeSurement1.B_voltage/0.1;
    pScreen->C_voltage = stTerm.stFrameRemoteMeSurement1.C_voltage/0.1;
    pScreen->A_current = stTerm.stFrameRemoteMeSurement1.A_current/0.01;
    pScreen->B_current = stTerm.stFrameRemoteMeSurement1.B_current/0.01;
    pScreen->C_current = stTerm.stFrameRemoteMeSurement1.C_current/0.01;
    pScreen->voltage_of_dc = stTerm.stFrameRemoteMeSurement1.voltage_of_dc/0.1;
    pScreen->current_of_dc = stTerm.stFrameRemoteMeSurement1.current_of_dc/0.1;

    /*  //ocean debug
                                                                pScreen->A_voltage,pScreen->B_voltage,pScreen->C_voltage,
                                                                pScreen->A_current,pScreen->B_current,pScreen->C_current);
    */

    //开始时间
    StartTime = QDateTime::fromString(QString(temStep.sStartTime),"yyyy-MM-dd HH:mm:ss");
    strncpy(pScreen->chStartTime, temStep.sStartTime, sizeof(temStep.sStartTime));
    //充电时间
    NowTime = QDateTime::currentDateTime();
    pScreen->usChargeTime = StartTime.secsTo(NowTime)/60; //充电时间, 分钟
    //功率, 电能
    pScreen->sActivePower = (short)(stTerm.stFrameRemoteMeSurement1.active_power/0.1);
    pScreen->uiTotalEnergy = (unsigned int)(stTerm.stFrameRemoteMeSurement2.active_electric_energy);
    //SOC
    pScreen->ucSOC = stTerm.stFrameBmsInfo.batery_SOC;

    int iChargeEnergy = 0;
    int iTempChargeEnergy = 0;
    iTempChargeEnergy = stTerm.stFrameRemoteMeSurement2.active_electric_energy - temStep.u32EnergyStartCharge;
    if(iTempChargeEnergy >= 0){
        iChargeEnergy = iTempChargeEnergy;
    }
    else{
        double dAmmeterRange = pParamSet->getAmmeterRange(stTerm.cCanAddr);
        if(( temStep.u32EnergyStartCharge > ((int)(dAmmeterRange * 100) - 60000) ) \
                && ( stTerm.stFrameRemoteMeSurement2.active_electric_energy <  60000) ){
            iChargeEnergy = (int)(dAmmeterRange * 100.0) - temStep.u32EnergyStartCharge + stTerm.stFrameRemoteMeSurement2.active_electric_energy;
        }
    }
    pScreen->usChargeEnergy = iChargeEnergy;

    return TRUE;
}

//获取  终端遥测数据
bool cLCDScreenProtocol::GetFrameTermBMS(unsigned char * &pData, unsigned short &usLen, unsigned char ucCanID)
{
    TerminalStatus stTerm;
    if(! pDevCache->QueryTerminalStatus(ucCanID,stTerm))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! Screen pDevCache Query  Error!");
        return FALSE;
    }
    //分配内存
    pData = new unsigned char[sizeof(TermBMS_LCD)];
    memset(pData, 0, sizeof(TermBMS_LCD *));
    TermBMS_LCD * pScreen = (TermBMS_LCD *)pData;
    //长度
    usLen = sizeof(TermBMS_LCD);

    pScreen->ucCanAddr = ucCanID;
    pScreen->ucState = stTerm.cStatus;
    //数据赋值
    pScreen->usBMSNeedVoltage = (unsigned short)(stTerm.stFrameBmsInfo.BMS_need_voltage/0.1);
    pScreen->sBMSNeedCurrent = (short)(stTerm.stFrameBmsInfo.BMS_need_current/0.1);
    pScreen->ucBaterySOC = stTerm.stFrameBmsInfo.batery_SOC;
    pScreen->usMaxBateryTemperature = (short)stTerm.stFrameBmsInfo.max_batery_temperature;
    pScreen->sMaxBateryVoltage = (stTerm.stFrameBmsInfo.max_batery_voltage/0.01);
    pScreen->usLowestBatteryTemperature = (unsigned short)stTerm.stFrameBmsInfo.lowest_battery_temperature;
    pScreen->usLowestChargeVoltage = (unsigned short)(stTerm.stFrameBmsInfo.lowest_charge_voltage/0.01);

    return TRUE;
}

//获取  终端故障信息
bool cLCDScreenProtocol::GetFrameTermFaultInfo(unsigned char * &pData, unsigned short &usLen, unsigned char ucCanID)
{
    unsigned char ucFaultCode, ucNum;//故障条目数
    QByteArray tempArray;
    unsigned short usLen1, usLen2;    //故障列表长度, 充电报告长度

    //数据库操作----查询故障记录
    db_result_st dbst;
    QList <FrameTermNowFaultNode_LCD> list;
    FrameTermNowFaultNode_LCD tempNode_32, tempNode_77;

    //终端故障查询
    QString exec = "SELECT fault_start_time, fault_code FROM terminal_fault_table where canaddr = " + QString::number(ucCanID) + " order by id desc limit 1";
    if(0 != pDBOperate->DBSqlQuery(exec.toAscii().data(), &dbst, DB_PROCESS_RECORD))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! QueryChargeRecord Query Error");
        return FALSE;
    }
    //终端故障赋值
    for(int i = 0; i < dbst.row; i++)
    {
        memset(&tempNode_32, 0x00, sizeof(tempNode_32));
        tempNode_32.ucCanAddr = ucCanID;
        tempNode_32.ucInnerID = 0;
        strncpy(tempNode_32.ucFaultStartTime, dbst.result[i * dbst.column], sizeof(tempNode_32.ucFaultStartTime));
        ucFaultCode = (unsigned char) atoi(dbst.result[i * dbst.column + 1]);
        if(ChargeConfig.languageType == 1)
            ChangeFaultCodeToArray(ucFaultCode, tempArray);
        else if(ChargeConfig.languageType == 2)
            ChangeFaultCodeToEnglishArray(ucFaultCode, tempArray);
        strncpy(tempNode_32.chFaultReason, tempArray.data(), tempArray.length());
        list.append(tempNode_32);
    }
    pDBOperate->DBQueryFree(&dbst);
    memset(&dbst,0x00, sizeof(dbst));
    //设备管理故障查询
    //1. 获取故障列表
    FaultRecord_DCcab tempRecord;
    QList <FaultRecord_DCcab> tempFaultList;
    if(0 != pDBOperate->DBSqlQuery("SELECT canaddr, module_id, fault_code, min_pdu_id, max_pdu_id, start_time, serialnum FROM dc_cabinet_fault_table where record_state = 85", &dbst, DB_PROCESS_RECORD))
    {
        return FALSE;
    }

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

    //2. 筛选对应的PDU的故障信息
    for(int i = 0; i < tempFaultList.count(); i++)
    {
        if( (tempFaultList.at(i).ucMaxPDUID >= ucCanID) && (tempFaultList.at(i).ucMinPDUID <= ucCanID))
        {
            memset(&tempNode_77, 0x00, sizeof(tempNode_77));
            tempNode_77.ucCanAddr = tempFaultList.at(i).ucCCUAddr;
            tempNode_77.ucInnerID = tempFaultList.at(i).ucDevID;
            strncpy(tempNode_77.ucFaultStartTime, tempFaultList.at(i).StartTime.data(), sizeof(tempNode_77.ucFaultStartTime));
            ucFaultCode = tempFaultList.at(i).ucFaultCode;
            if(ChargeConfig.languageType == 1)
                ChangeFaultCodeToArray_DeviceManage(ucFaultCode, tempArray);
            else if(ChargeConfig.languageType == 2)
                ChangeFaultCodeToEnglishArray_DeviceManage(ucFaultCode, tempArray);
            strncpy(tempNode_77.chFaultReason, tempArray.data(), tempArray.length());
            list.append(tempNode_77);
        }
    }

    //条目数赋值
    (list.count() > 10)?(ucNum = 10):(ucNum = list.count());
    //长度
    usLen1 = sizeof(FrameTermNowFaultNode_LCD) ;
    usLen2 = sizeof(FrameTermFaultReport_LCD);
    usLen = usLen1 * ucNum + usLen2 + 1;
    //分配内存
    pData = new unsigned char[usLen];
    memset(pData, 0x00, usLen);
    pData[0] = ucNum;
    //故障信息
    for(int i = 0 ; i < ucNum; i++)
    {
        memcpy(pData + usLen1 * i + 1, (char *)&list.at(i), usLen1);
    }

    //nihai add 20170522增加充电报告信息
    CHARGE_STEP stChargeStep;
    QByteArray stopArray;
    //查询
    if(pDevCache->QueryChargeStep(ucCanID, stChargeStep))
    {
        FrameTermFaultReport_LCD  * pScreen = (FrameTermFaultReport_LCD *)(pData + usLen1 * ucNum + 1);
        if(ChargeConfig.languageType == 1)
            ChangeDevStopReasonToArray(stChargeStep.ucStopReasonDev, stopArray);
        else if(ChargeConfig.languageType == 2)
            ChangeDevStopReasonToEnglishArray(stChargeStep.ucStopReasonDev, stopArray);
        //数据赋值
        memcpy(pScreen->chChargeStartTime,stChargeStep.sStartTime,sizeof(pScreen->chChargeStartTime));
        memcpy(pScreen->chChargeStopTime,stChargeStep.sEndTime,sizeof(pScreen->chChargeStopTime));
        memcpy(pScreen->chStopReason,stopArray.data(),stopArray.length());
    }

    return TRUE;
}

//获取  第X页充电记录
bool cLCDScreenProtocol::GetFrameChargeRecord(unsigned char * &pData, unsigned short &usLen, unsigned char ucPageNum)
{
    Node_ChargeRecord_LCD * pNode;
    unsigned char ucTotalNum = 0;   //所有记录页数, 10条记录为1页
    unsigned char ucPackRecordNum = 0;//本包记录条目
    if(ChargeRecordList.isEmpty())
    {
        return FALSE;
    }
    (ChargeRecordList.count()%iChargeRecordPerPage == 0)?(ucTotalNum = ChargeRecordList.count()/iChargeRecordPerPage):(ucTotalNum = ChargeRecordList.count()/iChargeRecordPerPage + 1);
    if(ucPageNum > ucTotalNum)  //查询页出错
    {
        return FALSE;
    }
    else if(ucPageNum == ucTotalNum) //最后一页
    {
        //本包记录条目
        ucPackRecordNum = ChargeRecordList.count() - (ucTotalNum - 1)*iChargeRecordPerPage;
    }
    else //中间页
    {
        //本包记录条目
        ucPackRecordNum = iChargeRecordPerPage;
    }
    //长度
    usLen = sizeof(Node_ChargeRecord_LCD)*ucPackRecordNum + 2;
    //分配内存
    pData = new unsigned char[usLen];
    pData[0] = ucPageNum;
    pData[1] = ucPackRecordNum;
    for(unsigned short i =  0; i < ucPackRecordNum; i++)
    {
        pNode = (Node_ChargeRecord_LCD * )(pData + 2 + sizeof(Node_ChargeRecord_LCD)* i);
        pNode->ucCanAddr = ChargeRecordList.at(i + iChargeRecordPerPage* (ucPageNum - 1)).ucCanAddr;
        pNode->sEnergy = ChargeRecordList.at(i + iChargeRecordPerPage* (ucPageNum - 1)).sEnergy;
        strncpy(pNode->chStartTime, ChargeRecordList.at(i + iChargeRecordPerPage* (ucPageNum - 1)).chStartTime, sizeof(pNode->chStartTime));
        strncpy(pNode->chStopTime, ChargeRecordList.at(i + iChargeRecordPerPage* (ucPageNum - 1)).chStopTime, sizeof(pNode->chStopTime));
        strncpy(pNode->chReason, ChargeRecordList.at(i + iChargeRecordPerPage* (ucPageNum - 1)).chReason, sizeof(pNode->chReason));
    }
    return TRUE;
}

//获取  第X页故障记录
bool cLCDScreenProtocol::GetFrameFaultRecord(unsigned char * &pData, unsigned short &usLen, unsigned char ucPageNum)
{
    Node_FaultRecord_LCD * pNode;
    unsigned char ucTotalNum = 0;   //所有记录页数, iFaultRecordPerPage条记录为1页
    unsigned char ucPackRecordNum = 0;//本包记录条目
    if(FaultRecordList.isEmpty())
    {
        return FALSE;
    }
    (FaultRecordList.count()%iFaultRecordPerPage == 0)?(ucTotalNum = FaultRecordList.count()/iFaultRecordPerPage):(ucTotalNum = FaultRecordList.count()/iFaultRecordPerPage + 1);
    if(ucPageNum > ucTotalNum)  //查询页出错
    {
        return FALSE;
    }
    else if(ucPageNum == ucTotalNum) //最后一页
    {
        //本包记录条目
        ucPackRecordNum = FaultRecordList.count() - (ucTotalNum - 1)*iFaultRecordPerPage;
    }
    else //中间页
    {
        //本包记录条目
        ucPackRecordNum = iFaultRecordPerPage;
    }
    //长度
    usLen = sizeof(Node_FaultRecord_LCD)*ucPackRecordNum + 2;
    //分配内存
    pData = new unsigned char[usLen];
    pData[0] = ucPageNum;
    pData[1] = ucPackRecordNum;
    for(unsigned short i =  0; i < ucPackRecordNum; i++)
    {
        pNode = (Node_FaultRecord_LCD * )(pData + 2 + sizeof(Node_FaultRecord_LCD)* i);
        pNode->ucCanAddr = FaultRecordList.at(i + iFaultRecordPerPage* (ucPageNum - 1)).ucCanAddr;
        pNode->ucInnerAddr = FaultRecordList.at(i + iFaultRecordPerPage* (ucPageNum - 1)).ucInnerAddr;
        pNode->ucMinPDUAddr = FaultRecordList.at(i + iFaultRecordPerPage* (ucPageNum - 1)).ucMinPDUAddr;
        pNode->ucMaxPDUAddr = FaultRecordList.at(i + iFaultRecordPerPage* (ucPageNum - 1)).ucMaxPDUAddr;
        pNode->chType = FaultRecordList.at(i + iFaultRecordPerPage* (ucPageNum - 1)).chType;

        strncpy(pNode->chStartTime, FaultRecordList.at(i + iFaultRecordPerPage* (ucPageNum - 1)).chStartTime, sizeof(pNode->chStartTime));
        strncpy(pNode->chStopTime, FaultRecordList.at(i + iFaultRecordPerPage* (ucPageNum - 1)).chStopTime, sizeof(pNode->chStopTime));
        strncpy(pNode->chReason, FaultRecordList.at(i + iFaultRecordPerPage* (ucPageNum - 1)).chReason, sizeof(pNode->chReason));
    }
    return TRUE;
}

//获取  第X页操作记录
bool cLCDScreenProtocol::GetFrameOperateRecord(unsigned char * &pData, unsigned short &usLen, unsigned char ucPageNum)
{
    Node_OperateRecord_LCD * pNode;
    unsigned char ucTotalNum = 0;   //所有记录页数, iOperateRecordPerPage条记录为1页
    unsigned char ucPackRecordNum = 0;//本包记录条目
    if(OperateRecordList.isEmpty())
    {
        return FALSE;
    }
    (OperateRecordList.count()%iOperateRecordPerPage == 0)?(ucTotalNum = OperateRecordList.count()/iOperateRecordPerPage):(ucTotalNum = OperateRecordList.count()/iOperateRecordPerPage + 1);
    if(ucPageNum > ucTotalNum)  //查询页出错
    {
        return FALSE;
    }
    else if(ucPageNum == ucTotalNum) //最后一页
    {
        //本包记录条目
        ucPackRecordNum = OperateRecordList.count() - (ucTotalNum - 1)*iOperateRecordPerPage;
    }
    else //中间页
    {
        //本包记录条目
        ucPackRecordNum = iOperateRecordPerPage;
    }
    //长度
    usLen = sizeof(Node_OperateRecord_LCD)*ucPackRecordNum + 2;
    //分配内存
    pData = new unsigned char[usLen];
    pData[0] = ucPageNum;
    pData[1] = ucPackRecordNum;
    for(unsigned short i =  0; i < ucPackRecordNum; i++)
    {
        pNode = (Node_OperateRecord_LCD * )(pData + 2 + sizeof(Node_OperateRecord_LCD)* i);
        pNode->ucOptType = OperateRecordList.at(i + iOperateRecordPerPage* (ucPageNum - 1)).ucOptType;
        strncpy(pNode->chOptName, OperateRecordList.at(i + iOperateRecordPerPage* (ucPageNum - 1)).chOptName, sizeof(pNode->chOptName));
        strncpy(pNode->chOptTime, OperateRecordList.at(i + iOperateRecordPerPage* (ucPageNum - 1)).chOptTime, sizeof(pNode->chOptTime));
        strncpy(pNode->chOptData, OperateRecordList.at(i + iOperateRecordPerPage* (ucPageNum - 1)).chOptData, sizeof(pNode->chOptData));
    }
    return TRUE;
}

//获取  终端充电报告
bool cLCDScreenProtocol::GetFrameTermChargeReport(unsigned char * &pData, unsigned short &usLen, unsigned char ucCanID)
{
    //    QDateTime tempTime;
    CHARGE_STEP stChargeStep;
    QByteArray stopArray;
    //查询
    if(!pDevCache->QueryChargeStep(ucCanID, stChargeStep))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! Screen pDevCache Query  Error!");
        return FALSE;
    }
    if(ChargeConfig.languageType == 1)
        ChangeDevStopReasonToArray(stChargeStep.ucStopReasonDev, stopArray);
    else if(ChargeConfig.languageType == 2)
        ChangeDevStopReasonToEnglishArray(stChargeStep.ucStopReasonDev, stopArray);
    //分配内存
    pData = new unsigned char[sizeof(FrameTermChargeReport_LCD)];
    memset(pData, 0, sizeof(FrameTermChargeReport_LCD));
    FrameTermChargeReport_LCD * pScreen = (FrameTermChargeReport_LCD *)pData;
    memset(pScreen,0,sizeof(FrameTermChargeReport_LCD));  //nihai add 20170528  乱码问题
    //长度
    usLen = sizeof(FrameTermChargeReport_LCD);
    //数据赋值
    pScreen->ucCanAddr = stChargeStep.ucCanAddr;
    pScreen->ucStartSOC = stChargeStep.ucStartSOC;
    pScreen->ucStopSOC = stChargeStep.ucEndSOC;
    pScreen->sEnergy = (unsigned short)(stChargeStep.u32TotalChargeEnergy);
    strncpy(pScreen->chStartTime, stChargeStep.sStartTime, sizeof(pScreen->chStartTime));
    strncpy(pScreen->chStopTime, stChargeStep.sEndTime, sizeof(pScreen->chStopTime));
    pScreen->usChargeTime = stChargeStep.u12TotalChargeTime;
    strncpy(pScreen->chStopReason, stopArray.data(), stopArray.length());

    return TRUE;
}



/***********************************************************************
 *函数名：GetFrameNoRemoveFaultRecordNum
 *参数说明：pData    出参，需要发送的数据流，usLen     出参，需要发送的数据流长度
 *                  AddrArray    接收到的数据流
 *返回值:      true:获取成功,false:获取失败
 *功能：获取 未消除故障信息条目
 *备注 ：by muty 20170913
 **********************************************************************/
bool cLCDScreenProtocol::GetFrameNoRemoveFaultRecordNum(unsigned char * &pData, unsigned short &usLen)
{
    bool bRet = FALSE;
    //unsigned short usDataType = AddrArray.at(9) + AddrArray.at(10) *256;    //数据类型
    unsigned short usTotalPageNum = 0;   //总页面数
    unsigned short usTotalRecordNum = 0; //总记录条数

    QueryNoRemoveFaultRecord();
    if(!NoRemoveFaultRecordList.isEmpty())
    {
        //总记录条目数
        usTotalRecordNum = NoRemoveFaultRecordList.count();
        //每页iFaultRecordPerPage条记录数
        (usTotalRecordNum%iFaultRecordPerPage == 0)?(usTotalPageNum = NoRemoveFaultRecordList.count()/iFaultRecordPerPage):(usTotalPageNum = NoRemoveFaultRecordList.count()/iFaultRecordPerPage + 1);
        bRet = TRUE;
    }
    if(bRet == TRUE)
    {
        //分配内存
        pData = new unsigned char[sizeof(NoRemoveFrameRecordNum_LCD)];
        memset(pData, 0, sizeof(NoRemoveFrameRecordNum_LCD *));
        NoRemoveFrameRecordNum_LCD * pScreen = (NoRemoveFrameRecordNum_LCD *)pData;
        //长度
        usLen = sizeof(NoRemoveFrameRecordNum_LCD);
        //赋值
        pScreen->usPageNum = usTotalPageNum;
        pScreen->usRecordNum = usTotalRecordNum;
    }
    return bRet;
}

/***********************************************************************
 *函数名：GetFrameNoRemoveFaultRecord
 *参数说明：pData    出参，需要发送的数据流，usLen     出参，需要发送的数据流长度
 *                  ucPageNum  获取第几页
 *返回值:      true:获取成功,false:获取失败
 *功能：获取 未消除故障信息
 *备注 : add by muty 20170913
 **********************************************************************/
bool cLCDScreenProtocol::GetFrameNoRemoveFaultRecord(unsigned char * &pData, unsigned short &usLen,unsigned char ucPageNum)
{
    Node_NoRemoveFaultRecord_LCD * pNode;
    unsigned char ucTotalNum = 0;   //所有记录页数, iFaultRecordPerPage条记录为1页
    unsigned char ucPackRecordNum = 0;//本包记录条目
    if(NoRemoveFaultRecordList.isEmpty())
    {
        return FALSE;
    }
    (NoRemoveFaultRecordList.count()%iFaultRecordPerPage == 0)?(ucTotalNum = NoRemoveFaultRecordList.count()/iFaultRecordPerPage):(ucTotalNum = NoRemoveFaultRecordList.count()/iFaultRecordPerPage + 1);
    if(ucPageNum > ucTotalNum)  //查询页出错
    {
        return FALSE;
    }
    else if(ucPageNum == ucTotalNum) //最后一页
    {
        //本包记录条目
        ucPackRecordNum = NoRemoveFaultRecordList.count() - (ucTotalNum - 1)*iFaultRecordPerPage;
    }
    else //中间页
    {
        //本包记录条目
        ucPackRecordNum = iFaultRecordPerPage;
    }
    //长度
    usLen = sizeof(Node_NoRemoveFaultRecord_LCD)*ucPackRecordNum + 2;
    //分配内存
    pData = new unsigned char[usLen];
    pData[0] = ucPageNum;
    pData[1] = ucPackRecordNum;
    for(unsigned short i =  0; i < ucPackRecordNum; i++)
    {
        pNode = (Node_NoRemoveFaultRecord_LCD * )(pData + 2 + sizeof(Node_NoRemoveFaultRecord_LCD)* i);
        pNode->ucCanAddr = NoRemoveFaultRecordList.at(i + iFaultRecordPerPage* (ucPageNum - 1)).ucCanAddr;
        pNode->ucInnerAddr = NoRemoveFaultRecordList.at(i + iFaultRecordPerPage* (ucPageNum - 1)).ucInnerAddr;
        pNode->ucMinPDUAddr = NoRemoveFaultRecordList.at(i + iFaultRecordPerPage* (ucPageNum - 1)).ucMinPDUAddr;
        pNode->ucMaxPDUAddr = NoRemoveFaultRecordList.at(i + iFaultRecordPerPage* (ucPageNum - 1)).ucMaxPDUAddr;
        pNode->chType = NoRemoveFaultRecordList.at(i + iFaultRecordPerPage* (ucPageNum - 1)).chType;

        strncpy(pNode->chStartTime, NoRemoveFaultRecordList.at(i + iFaultRecordPerPage* (ucPageNum - 1)).chStartTime, sizeof(pNode->chStartTime));
        strncpy(pNode->chReason, NoRemoveFaultRecordList.at(i + iFaultRecordPerPage* (ucPageNum - 1)).chReason, sizeof(pNode->chReason));
    }
    return TRUE;
}




//生成  申请卡号---->控制中心
void cLCDScreenProtocol::MakeCenterApplyCardNum(InfoMap &ToCenterMap, unsigned char ucCanID)
{
    QByteArray ArrayTemp;
    //插入CAN地址
    ArrayTemp.append(ucCanID);
    ToCenterMap.insert(Addr_CanID_Comm, ArrayTemp);
    //插入卡类型
    ArrayTemp.clear();
    ArrayTemp.append((char)ChargeConfig.cardType);
    ToCenterMap.insert(Addr_CardType, ArrayTemp);
}

//生成  申请用户信息---->控制中心
void cLCDScreenProtocol::MakeCenterApplyUserInfo(InfoMap &ToCenterMap, unsigned char ucCanID, QByteArray CardNum)
{
    QByteArray ArrayTemp;
    //插入CAN地址
    ArrayTemp.append(ucCanID);
    ToCenterMap.insert(Addr_CanID_Comm, ArrayTemp);
    //插入卡号
    ToCenterMap.insert(Addr_CardAccount, CardNum);
}

//生成  刷卡开始充电请求---->控制中心
void cLCDScreenProtocol::MakeCenterApplyStartCharge_Card(InfoMap &ToCenterMap, CardCmdApply_LCD &stCmdApply, QByteArray CardNum)
{
    QByteArray ArrayTemp;
    int fChargeEnergy = 0;
    //插入CAN地址
    ArrayTemp.append(stCmdApply.ucCanID);
    ToCenterMap.insert(Addr_CanID_Comm, ArrayTemp);

//    //插入充电模式
//    ArrayTemp.clear();
//    ArrayTemp.append(stCmdApply.ucType);
//    ToCenterMap.insert(Addr_CardApplyChargeType, ArrayTemp);//充电类型
//    //充电类型值
//    if(stCmdApply.ucType == 2)  //按照电量充
//    {
//        ArrayTemp.clear();
//        fChargeEnergy = ((float)(stCmdApply.usEnergy))/100.0;
//        ArrayTemp.append((char *)&fChargeEnergy, 4);
//        ToCenterMap.insert(Addr_CardChargeTypeValue, ArrayTemp);//充电电量
//    }
    if(stCmdApply.ucType == 1)   //自动充满
    {
        ArrayTemp.clear();
        ArrayTemp.append(4);
        ToCenterMap.insert(Addr_CardApplyChargeType, ArrayTemp);//充电类型
        ArrayTemp.clear();
        fChargeEnergy = 0xffffffff;
        ArrayTemp.append((char *)&fChargeEnergy, 4);
        ToCenterMap.insert(Addr_CardChargeTypeValue, ArrayTemp);//充电电量
    }
    else if(stCmdApply.ucType == 2)
    {
        ArrayTemp.clear();
        ArrayTemp.append(3);
        ToCenterMap.insert(Addr_CardApplyChargeType, ArrayTemp);//充电类型
        ArrayTemp.clear();
        fChargeEnergy = stCmdApply.usEnergy;
        ArrayTemp.append((char *)&fChargeEnergy, 4);
        ToCenterMap.insert(Addr_CardChargeTypeValue, ArrayTemp);//充电电量
    }
    //插入卡号
    ToCenterMap.insert(Addr_CardAccount, CardNum);
}


//生成  刷卡停止充电请求---->控制中心
void cLCDScreenProtocol::MakeCenterApplyStopCharge_Card(InfoMap &ToCenterMap, unsigned char ucCanID, QByteArray CardNum)
{
    QByteArray ArrayTemp;
    //插入CAN地址
    ArrayTemp.append(ucCanID);
    ToCenterMap.insert(Addr_CanID_Comm, ArrayTemp);
    //插入卡号
    ToCenterMap.insert(Addr_CardAccount, CardNum);
}
//MakFrameCSCUVersion

//生成CSCU版本信息
void cLCDScreenProtocol::MakFrameCSCUVersion(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr)
{
    unsigned char * pData = NULL;//原始数据指针
    GetFrameCSCUVersion(pData, usLen);//原始数据赋值
    MakeFrameFormat(pDestData, ucAddr, NormalType_LCD, Cmd_Mainten_LCD, Data_Version_LCD, usLen, pData);//发送组帧
    //删除原始数据帧
    delete pData;
}

//生成CSCU设置项
void cLCDScreenProtocol::MakFrameCSCUSet(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr)
{
    unsigned char * pData = NULL;//原始数据指针
    GetFrameCSCUSet(pData, usLen);//原始数据赋值
    MakeFrameFormat(pDestData, ucAddr, NormalType_LCD, Cmd_Query_LCD, Data_SysParam_LCD, usLen, pData);//发送组帧
    //删除原始数据帧
    delete pData;
}

//生成相别设置相
bool cLCDScreenProtocol::MakFramePhaseTypeSet(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr)
{
    unsigned char * pData = NULL;//原始数据指针
    if(GetFramePhaseTypeSet(pData, usLen) == FALSE)//原始数据赋值
    {
        return FALSE;
    }
    MakeFrameFormat(pDestData, ucAddr, NormalType_LCD, Cmd_Query_LCD, Data_PhaseType_LCD, usLen, pData);//发送组帧
    //删除原始数据帧
    delete pData;
    return TRUE;
}

//生成 屏幕密码项
bool cLCDScreenProtocol::MakFramePasswd(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr)
{
    unsigned char * pData = NULL;//原始数据指针
    if(GetFramePasswd(pData, usLen) == FALSE)//原始数据赋值
    {
        return FALSE;
    }
    MakeFrameFormat(pDestData, ucAddr, NormalType_LCD, Cmd_Query_LCD, Data_Passwd_LCD, usLen, pData);//发送组帧
    //删除原始数据帧
    delete pData;
    return TRUE;
}

//生成 IO常开常闭状态
bool cLCDScreenProtocol::MakFrameIOState(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr)
{
    unsigned char * pData = NULL;//原始数据指针
    if(GetFrameIOState(pData, usLen) == FALSE)//原始数据赋值
    {
        return FALSE;
    }
    MakeFrameFormat(pDestData, ucAddr, NormalType_LCD, Cmd_Query_LCD, Data_IOState_LCD, usLen, pData);//发送组帧
    //删除原始数据帧
    delete pData;
    return TRUE;
}

//生成  电表地址项
bool cLCDScreenProtocol::MakFrameAmmeterAddr(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr)
{
    unsigned char * pData = NULL;//原始数据指针
    if(GetFrameAmmeterAddr(pData, usLen) == FALSE)//原始数据赋值
    {
        return FALSE;
    }
    MakeFrameFormat(pDestData, ucAddr, NormalType_LCD, Cmd_Query_LCD, Data_AmmeterAddr_LCD, usLen, pData);//发送组帧
    //删除原始数据帧
    delete pData;
    return TRUE;
}

//生成  进线侧数据项
bool cLCDScreenProtocol::MakFrameAmmeterData(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, const char * pRawData)
{
    unsigned char * pData = NULL;//原始数据指针
    QByteArray addrArray;
    addrArray.append(pRawData+9, 6);

    if(GetFrameAmmeterData(pData, usLen, addrArray) == FALSE)//原始数据赋值
    {
        return FALSE;
    }
    MakeFrameFormat(pDestData, ucAddr, NormalType_LCD, Cmd_RealData_LCD, Data_AmmeterData_LCD, usLen, pData);//发送组帧

    //删除原始数据帧
    delete pData;
    return TRUE;
}

//生成  环境信息
bool cLCDScreenProtocol::MakFrameEnvInfo(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, const char * pRawData)
{
    unsigned char * pData = NULL;//原始数据指针
    QByteArray addrArray;
    addrArray.append(pRawData+7, 6);
    if(GetFrameEnvInfo(pData, usLen, addrArray) == FALSE)//原始数据赋值
    {
        return FALSE;
    }
    MakeFrameFormat(pDestData, ucAddr, NormalType_LCD, Cmd_RealData_LCD, Data_EnvInfo_LCD, usLen, pData);//发送组帧
    //删除原始数据帧
    delete pData;
    return TRUE;
}

//生成 模块实时数据
bool cLCDScreenProtocol::MakFrameModData(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, unsigned char &ucCanID, unsigned char &ucInnerID)
{
    unsigned char * pData = NULL;//原始数据指针
    if(GetFrameModData(pData, usLen, ucCanID, ucInnerID) == FALSE)//原始数据赋值
    {
        return FALSE;
    }
    MakeFrameFormat(pDestData, ucAddr, NormalType_LCD, Cmd_RealData_LCD, Data_DCMOD_LCD, usLen, pData);//发送组帧
    //删除原始数据帧
    delete pData;
    return TRUE;
}

//生成 PDU实时数据
bool cLCDScreenProtocol::MakFramePDUData(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, unsigned char &ucCanID, unsigned char &ucInnerID)
{
    unsigned char * pData = NULL;//原始数据指针
    if(GetFramePDUData(pData, usLen, ucCanID, ucInnerID) == FALSE)//原始数据赋值
    {
        return FALSE;
    }
    MakeFrameFormat(pDestData, ucAddr, NormalType_LCD, Cmd_RealData_LCD, Data_DCPDU_LCD, usLen, pData);//发送组帧
    //删除原始数据帧
    delete pData;
    return TRUE;
}

//生成 CCU实时数据
bool cLCDScreenProtocol::MakFrameCCUData(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, unsigned char &ucCanID, unsigned char &ucInnerID)
{
    unsigned char * pData = NULL;//原始数据指针
    if(GetFrameCCUData(pData, usLen, ucCanID, ucInnerID) == FALSE)//原始数据赋值
    {
        return FALSE;
    }
    MakeFrameFormat(pDestData, ucAddr, NormalType_LCD, Cmd_RealData_LCD, Data_DCCCU_LCD, usLen, pData);//发送组帧
    //删除原始数据帧
    delete pData;
    return TRUE;
}

//生成错峰充电设置
bool cLCDScreenProtocol::MakFramePeakSet(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr)
{
    unsigned char * pData = NULL;//原始数据指针
    if(GetFramePeakSet(pData, usLen) == FALSE)//原始数据赋值
    {
        return FALSE;
    }
    MakeFrameFormat(pDestData, ucAddr, NormalType_LCD, Cmd_Query_LCD, Data_PeakSet_LCD, usLen, pData);//发送组帧
    //删除原始数据帧
    delete pData;
    return TRUE;
}

//生成终端设置
bool cLCDScreenProtocol::MakFrameGunSet(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, unsigned char ucCanID)
{
    unsigned char * pData = NULL;//原始数据指针
    if(GetFrameGunArg(pData, usLen, ucCanID) == FALSE)//原始数据赋值
    {
        return FALSE;
    }
    MakeFrameFormat(pDestData, ucAddr, NormalType_LCD, Cmd_Query_LCD, Data_TermParam_DC_LCD, usLen, pData);//发送组帧

    //删除原始数据帧
    delete pData;
    return TRUE;
}

//生成CCU设置
bool cLCDScreenProtocol::MakFrameCCUSet(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, unsigned char ucCanID)
{
    unsigned char * pData = NULL;//原始数据指针
    if(GetFrameCCUArg(pData, usLen, ucCanID) == FALSE)//原始数据赋值
    {
        return FALSE;
    }
    MakeFrameFormat(pDestData, ucAddr, NormalType_LCD, Cmd_Query_LCD, Data_CCUParam_LCD, usLen, pData);//发送组帧
    //删除原始数据帧
    delete pData;
    return TRUE;
}

//生成直流柜某参数类型设备个数
bool cLCDScreenProtocol::MakFrameDCCabArgTypeNum(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, unsigned char ucType)
{
    unsigned char * pData = NULL;//原始数据指针
    if(GetDCCabArgTypeNum(pData, usLen, ucType) == FALSE)//原始数据赋值
    {
        return FALSE;
    }
    MakeFrameFormat(pDestData, ucAddr, NormalType_LCD, Cmd_Query_LCD, Data_DCCabArgTypeNum_LCD, usLen, pData);//发送组帧
    //删除原始数据帧
    delete pData;
    return TRUE;
}

//生成直流柜设备个数
bool cLCDScreenProtocol::MakFrameDCCabDevNum(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, unsigned char ucType)
{
    unsigned char * pData = NULL;//原始数据指针
    if(GetDCCabDevNum(pData, usLen, ucType) == FALSE)//原始数据赋值
    {
        return FALSE;
    }
    MakeFrameFormat(pDestData, ucAddr, NormalType_LCD, Cmd_Query_LCD, Data_DCCabDeviceNum_LCD, usLen, pData);//发送组帧

    for(int i = 0 ; i < usLen; i++)
    {
    }
    //删除原始数据帧
    delete pData;
    return TRUE;
}

//生成随机码
bool cLCDScreenProtocol::MakFrameRandCode(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, unsigned int uiCode)
{

    unsigned char * pData = (unsigned char *)&uiCode;//原始数据指针
    usLen = 4;
    MakeFrameFormat(pDestData, ucAddr, NormalType_LCD, Cmd_Query_LCD, Data_LocalDynamicPassword_LCD, usLen, pData);//发送组帧
    return TRUE;
}

//生成二维码查询结果
void cLCDScreenProtocol::MakFrame2DbarCodes(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr)
{
    unsigned char * pData = NULL;//原始数据指针
    GetFrame2DbarCodes(pData, usLen);//原始数据赋值
    MakeFrameFormat(pDestData, ucAddr, NormalType_LCD, Cmd_Query_LCD, Data_2DbarCodes_LCD, usLen, pData);//发送组帧
    //删除原始数据帧
    delete pData;
}

//生成系统特殊功能设置项
void cLCDScreenProtocol::MakFrameSysSpecSet(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr)
{
    unsigned char * pData = NULL;//原始数据指针
    GetFrameSysSpecSet(pData, usLen);//原始数据赋值
    MakeFrameFormat(pDestData, ucAddr, NormalType_LCD, Cmd_Query_LCD, Data_SysSpecSet_LCD, usLen, pData);//发送组帧
    //删除原始数据帧
    delete pData;
}

//生成直流特殊功能设置项
void cLCDScreenProtocol::MakFrameDCSpecSet(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr)
{
    unsigned char * pData = NULL;//原始数据指针
    GetFrameDCSpecSet(pData, usLen);//原始数据赋值
    MakeFrameFormat(pDestData, ucAddr, NormalType_LCD, Cmd_Query_LCD, Data_DCSpecSet_LCD, usLen, pData);//发送组帧
    //删除原始数据帧
    delete pData;
}

//生成  负荷约束功能设置数据项
void cLCDScreenProtocol::MakFrameLoadLimit(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr)
{
    unsigned char * pData = NULL;//原始数据指针
    GetFrameLoadLimit(pData, usLen);//原始数据赋值
    MakeFrameFormat(pDestData, ucAddr, NormalType_LCD, Cmd_Query_LCD, Data_LoadLimit_LCD, usLen, pData);//发送组帧
    //删除原始数据帧
    delete pData;
}

//生成 本地充电密码数据项
void cLCDScreenProtocol::MakFrameLocalChargePassword(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr)
{
    unsigned char * pData = NULL;//原始数据指针
    GetFrameLocalChargePassword(pData, usLen);//原始数据赋值
    MakeFrameFormat(pDestData, ucAddr, NormalType_LCD, Cmd_Query_LCD, Data_LocalChargePassword_LCD, usLen, pData);//发送组帧
    //删除原始数据帧
    delete pData;
}


//生成  终端状态数据项
bool cLCDScreenProtocol::MakFrameTermState(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, unsigned char &ucCanID)
{
    unsigned char * pData = NULL;//原始数据指针
    if(GetFrameTermState(pData, usLen, ucCanID) == FALSE)//原始数据赋值
    {
        return FALSE;
    }
    MakeFrameFormat(pDestData, ucAddr, NormalType_LCD, Cmd_State_LCD, Data_TermState_LCD, usLen, pData);//发送组帧
    //删除原始数据帧
    delete pData;
    return TRUE;
}

//生成  CSCU状态数据项
bool cLCDScreenProtocol::MakFrameCSCUState(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr)
{
    unsigned char * pData = new (unsigned char[3]);
    unsigned char ucNetState = 0;
    unsigned char ucEmergencyState = 0;
    QVariant Var, Param;
    usLen = 3;  //长度3
    //获取网络状态
    if(!pDevCache->QueryRealStatusMeter(Var, CACHE_STATUS, Param))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! SendMainPageData  QueryRealStatusMeter Error!");
        delete pData;
        return FALSE;
    }
    ucNetState = Var.value<RealStatusData>().connectStatus;
    ucEmergencyState = Var.value<RealStatusData>().emergencyStatus;

    pData[0] = ucNetState;
    pData[1] = ucUDiskState;
    pData[2] = ucEmergencyState;
    MakeFrameFormat(pDestData, ucAddr, NormalType_LCD, Cmd_State_LCD, Data_CSCUState_LCD, usLen, pData);//发送组帧
    delete pData;
    return TRUE;
}

//生成  终端测量数据项----普通版
bool cLCDScreenProtocol::MakFrameTermMeasure_Normal(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, unsigned char &ucCanID)
{
    unsigned char * pData = NULL;//原始数据指针
    if(GetFrameTermMeasure_Normal(pData, usLen, ucCanID) == FALSE)//原始数据赋值
    {
        return FALSE;
    }
    MakeFrameFormat(pDestData, ucAddr, NormalType_LCD, Cmd_RealData_LCD, Data_TermMeasure_Normal_LCD, usLen, pData);//发送组帧
    //删除原始数据帧
    delete pData;
    return TRUE;
}

//生成  终端测量数据项----普通版
bool cLCDScreenProtocol::MakFrameTermMeasure_Card(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, unsigned char &ucCanID)
{
    unsigned char * pData = NULL;//原始数据指针
    if(GetFrameTermMeasure_Card(pData, usLen, ucCanID) == FALSE)//原始数据赋值
    {
        return FALSE;
    }
    MakeFrameFormat(pDestData, ucAddr, NormalType_LCD, Cmd_RealData_LCD, Data_TermMeasure_Card_LCD, usLen, pData);//发送组帧
    //删除原始数据帧
    delete pData;
    return TRUE;
}

//生成  终端BMS数据项
bool cLCDScreenProtocol::MakFrameTermBMS(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, unsigned char &ucCanID)
{
    unsigned char * pData = NULL;//原始数据指针
    if(GetFrameTermBMS(pData, usLen, ucCanID) == FALSE)//原始数据赋值
    {
        return FALSE;
    }
    MakeFrameFormat(pDestData, ucAddr, NormalType_LCD, Cmd_RealData_LCD, Data_TermBMS_LCD, usLen, pData);//发送组帧
    //删除原始数据帧
    delete pData;
    return TRUE;
}

//生成  终端故障信息数据项
bool cLCDScreenProtocol::MakFrameTermFaultInfo(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, unsigned char &ucCanID)
{
    unsigned char * pData = NULL;//原始数据指针
    if(GetFrameTermFaultInfo(pData, usLen, ucCanID) == FALSE)//原始数据赋值
    {
        return FALSE;
    }
    MakeFrameFormat(pDestData, ucAddr, NormalType_LCD, Cmd_RealData_LCD, Data_TermFaultInfo_LCD, usLen, pData);//发送组帧

    //删除原始数据帧
    delete pData;
    return TRUE;
}

//生成  终端充电报告数据项
bool cLCDScreenProtocol::MakFrameTermChargeReport(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, unsigned char &ucCanID)
{
    unsigned char * pData = NULL;//原始数据指针
    if(GetFrameTermChargeReport(pData, usLen, ucCanID) == FALSE)//原始数据赋值
    {
        return FALSE;
    }
    MakeFrameFormat(pDestData, ucAddr, NormalType_LCD, Cmd_HisData_LCD, Data_TermChargeReport_LCD, usLen, pData);//发送组帧
    //删除原始数据帧
    delete pData;
    return TRUE;
}

//生成  历史记录条目
bool cLCDScreenProtocol::MakeFrameRecordNum(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, const QByteArray &recvArray)
{
    unsigned char * pData = NULL;//原始数据指针
    if(GetFrameRecordNum(pData, usLen, recvArray) == FALSE)//原始数据赋值
    {
        return FALSE;
    }
    MakeFrameFormat(pDestData, ucAddr, NormalType_LCD, Cmd_HisData_LCD, Data_RecordNum_LCD, usLen, pData);//发送组帧
    //删除原始数据帧
    delete pData;
    return TRUE;
}

//生成  故障记录数据项
bool cLCDScreenProtocol::MakFrameFaultRecord(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, unsigned char &ucPageNum)
{
    unsigned char * pData = NULL;//原始数据指针
    if(GetFrameFaultRecord(pData, usLen, ucPageNum) == FALSE)//原始数据赋值
    {
        return FALSE;
    }
    MakeFrameFormat(pDestData, ucAddr, NormalType_LCD, Cmd_HisData_LCD, Data_FaultRecord_LCD, usLen, pData);//发送组帧
    //删除原始数据帧
    delete pData;
    return TRUE;
}
//生成  充电记录数据项
bool cLCDScreenProtocol::MakFrameChargeRecord(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, unsigned char &ucPageNum)
{
    unsigned char * pData = NULL;//原始数据指针
    if(GetFrameChargeRecord(pData, usLen, ucPageNum) == FALSE)//原始数据赋值
    {
        return FALSE;
    }
    MakeFrameFormat(pDestData, ucAddr, NormalType_LCD, Cmd_HisData_LCD, Data_ChargeRecord_LCD, usLen, pData);//发送组帧
    //删除原始数据帧
    delete pData;
    return TRUE;
}

//生成  操作记录数据项
bool cLCDScreenProtocol::MakFrameOperateRecord(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, unsigned char &ucPageNum)
{
    unsigned char * pData = NULL;//原始数据指针
    if(GetFrameOperateRecord(pData, usLen, ucPageNum) == FALSE)//原始数据赋值
    {
        return FALSE;
    }
    MakeFrameFormat(pDestData, ucAddr, NormalType_LCD, Cmd_HisData_LCD, Data_OperateRecord_LCD, usLen, pData);//发送组帧
    //删除原始数据帧
    delete pData;
    return TRUE;
}

/***********************************************************************
 *函数名       ：MakFrameNoRemoveFaultRecordNum
 *参数说明   ：pDestData    出参 需要发送的数据流，usLen     出参，需要发送的数据流长度
 *                      ucAddr   客户端地址，recvArray 接收到的原始数据
 *返回值        :  true:生成成功,false:生成失败
 *功能           ：生成  未消除故障信息条目
 *备注            :  add by muty 20170913
 **********************************************************************/
bool cLCDScreenProtocol::MakFrameNoRemoveFaultRecordNum(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, const QByteArray &recvArray)
{
    Q_UNUSED(recvArray);
    unsigned char * pData = NULL;//原始数据指针
    if(GetFrameNoRemoveFaultRecordNum(pData, usLen) == FALSE)//原始数据赋值
    {
        return FALSE;
    }
    MakeFrameFormat(pDestData, ucAddr, NormalType_LCD, Cmd_RealData_LCD, Data_NoRemoveFaultRecordNum_LCD, usLen, pData);//发送组帧
    //删除原始数据帧
    delete pData;
    return TRUE;
}

/***********************************************************************
 *函数名：MakFrameNoRemoveFaultRecord
 *参数说明：pDestData    出参 需要发送的数据流，usLen     出参，需要发送的数据流长度
 *                  ucAddr   客户端地址，ucPageNum 请求的第几页
 *返回值:      true:生成成功,false:生成失败
 *功能：生成  未消除故障记录
 *备注 : add by muty 20170913
 **********************************************************************/
bool cLCDScreenProtocol::MakFrameNoRemoveFaultRecord(unsigned char * &pDestData, unsigned short &usLen, unsigned char &ucAddr, unsigned char &ucPageNum)
{
    unsigned char * pData = NULL;//原始数据指针
    if(GetFrameNoRemoveFaultRecord(pData, usLen, ucPageNum) == FALSE)//原始数据赋值
    {
        return FALSE;
    }
    MakeFrameFormat(pDestData, ucAddr, NormalType_LCD, Cmd_RealData_LCD, Data_NoRemoveFaultRecord_LCD, usLen, pData);//发送组帧
    //删除原始数据帧
    delete pData;
    return TRUE;
}

//解析 系统参数设置指令
int cLCDScreenProtocol::ParseSysParam(const char * pData, unsigned short usLen)
{
    int ret = 0;
    //获取配置
    QueryParamInfo();
    CSCUSet_LCD stFrame;
    memcpy((char *)&stFrame, pData, usLen);
    //更新CSCU参数
    if( (stFrame.ucACThrNum != cscuSysConfig.threePhase)
            || (stFrame.ucACSinNum != cscuSysConfig.singlePhase)
            || (stFrame.ucDCNum != cscuSysConfig.directCurrent)
            || (strncmp(stFrame.chDNS, cscuSysConfig.dns, sizeof(stFrame.chDNS)) != 0))

    {
//        pDBOperate->DBSqlExec("DELETE FROM terminal_name_table", DB_PARAM);
        cscuSysConfig.singlePhase = stFrame.ucACSinNum;
        cscuSysConfig.threePhase = stFrame.ucACThrNum;
        cscuSysConfig.directCurrent = stFrame.ucDCNum;
        strncpy(cscuSysConfig.dns, stFrame.chDNS, sizeof(stFrame.chDNS));
        if(ChargeConfig.languageType == 1)
            UpdateOperateDB(01, "屏幕设置", "更新CSCU参数");
        else if(ChargeConfig.languageType == 2)
            UpdateOperateDB(01, "屏幕设置", "update the CSCU parameters");
        pParamSet->updateSetting(&cscuSysConfig, PARAM_CSCU_SYS);
        ret = 1;
    }

    //更新NET0参数
    if( (strncmp(stFrame.chLocolIp, NetConfig.localIp, sizeof(stFrame.chLocolIp)) != 0)
            || (strncmp(stFrame.chMask, NetConfig.netMask, sizeof(stFrame.chMask)) != 0)
            || (strncmp(stFrame.chGateWay, NetConfig.gateway, sizeof(stFrame.chGateWay)) != 0) )
    {
        strncpy(NetConfig.localIp, stFrame.chLocolIp, sizeof(stFrame.chLocolIp));
        strncpy(NetConfig.netMask, stFrame.chMask, sizeof(stFrame.chMask));
        strncpy(NetConfig.gateway, stFrame.chGateWay, sizeof(stFrame.chGateWay));
        if(ChargeConfig.languageType == 1)
            UpdateOperateDB(01, "屏幕设置", "更新NET0参数");
        else if(ChargeConfig.languageType == 2)
            UpdateOperateDB(01, "屏幕设置", "update the NET0 parameters");
        pParamSet->updateSetting(&NetConfig, PARAM_NET0);
        ret = 1;
    }

    //更新服务器参数
    if( (strncmp(stFrame.chServerIP1, ServerConfig.serverIp1, sizeof(stFrame.chServerIP1)) != 0)
            || (stFrame.usPortNum1 != ServerConfig.serverPort1)
            || (strncmp(stFrame.chServerIP2, ServerConfig.serverIp2, sizeof(stFrame.chServerIP2)) != 0)
            || (stFrame.usPortNum2 != ServerConfig.serverPort2)
            || (strncmp(stFrame.chServerIP3, ServerConfig.serverIp3, sizeof(stFrame.chServerIP3)) != 0)
            || (stFrame.usPortNum3 != ServerConfig.serverPort3)
            || (strncmp(stFrame.chStationAddr, ServerConfig.stationNo, sizeof(stFrame.chStationAddr)) != 0) )
    {
        strncpy(ServerConfig.serverIp1, stFrame.chServerIP1, sizeof(stFrame.chServerIP1));
        ServerConfig.serverPort1 = stFrame.usPortNum1;
        strncpy(ServerConfig.serverIp2, stFrame.chServerIP2, sizeof(stFrame.chServerIP2));
        ServerConfig.serverPort2 = stFrame.usPortNum2;
        strncpy(ServerConfig.serverIp3, stFrame.chServerIP3, sizeof(stFrame.chServerIP3));
        ServerConfig.serverPort3 = stFrame.usPortNum3;
        strncpy(ServerConfig.stationNo, stFrame.chStationAddr, sizeof(stFrame.chStationAddr));
        if(ChargeConfig.languageType == 1)
            UpdateOperateDB(01, "屏幕设置", "更新server0参数");
        else if(ChargeConfig.languageType == 1)
            UpdateOperateDB(01, "屏幕设置", "update the server0 parameters");
        pParamSet->updateSetting(&ServerConfig, PARAM_SERVER0);
        ret = 1;
    }
    return ret;
}

//解析 相别参数设置指令
int cLCDScreenProtocol::ParsePhaseParam(const char * pData, unsigned short usLen)
{
    Q_UNUSED(usLen);
    stThreePhaseTypeConfig tempConfig;//三相相别全部参数, 接收屏幕设置
    stPhaseTypeConfig tempNode; //三相相别记录节点
    unsigned char ucNum = pData[0];
    int ret = 0;
    //获取配置
    QueryParamInfo();

    //写入数据库
    for(int i = 0; i < ucNum; i++)
    {
        tempNode.canaddr = (pData[1 + 2* i]);
        tempNode.type = (pData[2 + 2* i]);
        tempConfig.phaseTypeConfig.append(tempNode);
    }
    if(ChargeConfig.languageType == 1)
        UpdateOperateDB(01, "屏幕设置", "更新相别参数");
    else if(ChargeConfig.languageType == 2)
        UpdateOperateDB(01, "屏幕设置", "update three phase parameters");
    pParamSet->updateSetting(&tempConfig, PARAM_PHASE_TYPE);
    return ret;
}

//解析 系统特殊功能设置
int cLCDScreenProtocol::ParseSysSpecSet(const char * pData, unsigned short usLen)
{
    Q_UNUSED(usLen);
    SysSpecSet_LCD stFrame;
    int ret = 0;
    //获取配置
    QueryParamInfo();
    memcpy((char *)&stFrame, pData, sizeof(stFrame));

    //新旧设置更新比较
    if( (stFrame.ucLocalChargeEnable != ChargeConfig.localChargeEnable)
                   ||(stFrame.ucLocalChargeType != ChargeConfig.localChargeType)
                   ||(stFrame.coupleGun != ChargeConfig.coupleGun))
    {
        ChargeConfig.localChargeEnable = stFrame.ucLocalChargeEnable;
        ChargeConfig.localChargeType = stFrame.ucLocalChargeType;
        ChargeConfig.coupleGun = stFrame.coupleGun;  //add by songqb
        if(ChargeConfig.languageType == 1)
            UpdateOperateDB(01, "屏幕设置", "更新系统特殊功能参数");
        else if(ChargeConfig.languageType == 2)
            UpdateOperateDB(01, "屏幕设置", "update system special function parameters");
        pParamSet->updateSetting(&ChargeConfig, PARAM_CHARGE);
        ret = 0;
    }

    if( (stFrame.ucVINOffline != ChargeConfig.vinOffline)
            ||(stFrame.ucLocalStop != ChargeConfig.localStop)
            ||(stFrame.ucCardType != ChargeConfig.cardType)
            ||(stFrame.ucVINAuto != ChargeConfig.vinAuto)
            ||(stFrame.ucCardAuto != ChargeConfig.cardAuto)
            ||(stFrame.ucVINType != ChargeConfig.vinType)
            ||(stFrame.ucEnergyFilter != ChargeConfig.energyFilter)
            ||(stFrame.ucDevType != ChargeConfig.ucDevType) //mod by muty 20171011 单桩/群充设置
            ||(stFrame.ucticketEnable != ChargeConfig.ticketEnable)
            ||(stFrame.ucLanguage != ChargeConfig.languageType)
            )
    {
        pDBOperate->DBSqlExec("DELETE FROM terminal_name_table", DB_PARAM);
        ChargeConfig.vinOffline = stFrame.ucVINOffline;
        ChargeConfig.localStop = stFrame.ucLocalStop;
        ChargeConfig.cardType = stFrame.ucCardType;
        ChargeConfig.vinAuto = stFrame.ucVINAuto;
        ChargeConfig.cardAuto = stFrame.ucCardAuto;
        ChargeConfig.vinType = stFrame.ucVINType;
        ChargeConfig.energyFilter = stFrame.ucEnergyFilter;
        ChargeConfig.ucDevType = stFrame.ucDevType; //mod by muty 20171011 单桩/群充设置
        ChargeConfig.ticketEnable = stFrame.ucticketEnable;
        ChargeConfig.languageType = stFrame.ucLanguage;
        if(ChargeConfig.languageType == 1)
            UpdateOperateDB(01, "屏幕设置", "更新系统特殊功能参数");
        else if(ChargeConfig.languageType == 2)
            UpdateOperateDB(01, "屏幕设置", "update system special function parameters");
        pParamSet->updateSetting(&ChargeConfig, PARAM_CHARGE);
        ret = 1;
    }

    return ret;
}

//解析 负荷约束功能设置
int cLCDScreenProtocol::ParseLoadLimitSet(const char * pData, unsigned short usLen)
{
    Q_UNUSED(usLen);
    LoadLimit_LCD stFrame;
    int ret = 0;
    //获取配置
    QueryParamInfo();
    memcpy((char *)&stFrame, pData, sizeof(stFrame));
    if( (PowerLimitConfig.sCCUcount != stFrame.ucCCUNum)
            || (PowerLimitConfig.STATION_LIMT_POWER != stFrame.usTotalPower)
            || (PowerLimitConfig.SAFE_CHARGE_POWER != stFrame.usSecurePower)
            || (PowerLimitConfig.sSUMPower_Manual != stFrame.usLimitPower)
            || (PowerLimitConfig.sSUMPower_Ammeter_Enable != stFrame.ucDynamicEnable)
            || (PowerLimitConfig.sSUMPower_Manual_Enable != stFrame.ucLocalEnable)
            || (PowerLimitConfig.sSUMPower_Server_Enable != stFrame.ucRemoteEnable)
            || (PowerLimitConfig.sPowerLimit_Enable != stFrame.ucLoadLimit)
            )
    {
        PowerLimitConfig.sCCUcount = stFrame.ucCCUNum;
        PowerLimitConfig.STATION_LIMT_POWER = stFrame.usTotalPower;
        PowerLimitConfig.SAFE_CHARGE_POWER = stFrame.usSecurePower;
        PowerLimitConfig.sSUMPower_Manual = stFrame.usLimitPower;
        PowerLimitConfig.sSUMPower_Ammeter_Enable = stFrame.ucDynamicEnable;
        PowerLimitConfig.sSUMPower_Manual_Enable = stFrame.ucLocalEnable;
        PowerLimitConfig.sSUMPower_Server_Enable = stFrame.ucRemoteEnable;
        PowerLimitConfig.sPowerLimit_Enable = stFrame.ucLoadLimit;
        if(ChargeConfig.languageType == 1)
            UpdateOperateDB(01, "屏幕设置", "更新负荷约束功能参数");
        else if(ChargeConfig.languageType == 2)
            UpdateOperateDB(01, "屏幕设置", "update load constraint function parameters");
        pParamSet->updateSetting(&PowerLimitConfig, PARAM_POWERLIMIT);
    }
    return ret;
}

//解析 错峰充电功能设置
int cLCDScreenProtocol::ParsePeakSet(const char * pData, unsigned short usLen, InfoMap &ToCenterMap)
{
    unsigned char ucEnable = pData[0];
    unsigned char ucNum = pData[1];
    QByteArray tempArray;
    stAllTPFVConfig tempList;
    stTPFVConfig tempConfig;
    PeakSet_LCD stPeak;
    int ret = 0;
    tempList.tpfvConfig.clear();
    for(unsigned char i = 0, j = 0; i < ucNum; i++)
    {
        memcpy((unsigned char *)&stPeak, pData + 2 + i* sizeof(stPeak), sizeof(stPeak));
        if((stPeak.ucStartH == 0) && (stPeak.ucStartM == 0) && (stPeak.ucStopH == 0) && (stPeak.ucStopM == 0))
        {
            continue;
        }
        tempConfig.limit_current = stPeak.ucCurrent;
        tempConfig.limit_soc = stPeak.ucSOC;
        tempConfig.start_hour = stPeak.ucStartH;
        tempConfig.start_minute = stPeak.ucStartM;
        tempConfig.stop_hour = stPeak.ucStopH;
        tempConfig.stop_minute = stPeak.ucStopM;
        tempConfig.time_seg = stPeak.ucType;
        tempList.tpfvConfig.append(tempConfig);

        tempArray.clear();
        tempArray.append((char *)&tempConfig, sizeof(tempConfig));
        ToCenterMap.insert(i, tempArray);
        j++;
    }
    SmartChargeConfig.sSmartCharge_Enable = ucEnable;

    pParamSet->updateSetting(&SmartChargeConfig, PARAM_SMARTCHARGE);
    if(ChargeConfig.languageType == 1)
        UpdateOperateDB(01, "屏幕设置", "更新错峰充电功能参数");
    else if(ChargeConfig.languageType == 2)
        UpdateOperateDB(01, "屏幕设置", "update peak load function parameters");
    for(int i = 0 ; i < usLen; i++)
    {
    }
    return ret;
}

//解析 屏幕密码设置
int cLCDScreenProtocol::ParsePasswd(const char * pData, unsigned short usLen)
{
    Q_UNUSED(usLen);
    int ret = 0;
    //获取配置
    QueryParamInfo();
    cscuSysConfig.password = *(int *)(pData);
    if(ChargeConfig.languageType == 1)
        UpdateOperateDB(01, "屏幕设置", "更新屏幕密码");
    else if(ChargeConfig.languageType == 2)
        UpdateOperateDB(01, "屏幕设置", "update the screen password");
    pParamSet->updateSetting(&cscuSysConfig, PARAM_CSCU_SYS);
    return ret;
}

//解析 IO常开常闭设置
int cLCDScreenProtocol::ParseIOState(const char * pData, unsigned short usLen)
{
    Q_UNUSED(usLen);
    int ret = 0;
    //获取配置
    QueryParamInfo();
    memcpy(&IOConfig.inOpenColse, pData, 10);
    if(ChargeConfig.languageType == 1)
        UpdateOperateDB(01, "屏幕设置", "更新IO常开常闭参数");
    else if(ChargeConfig.languageType == 2)
        UpdateOperateDB(01, "屏幕设置", "update IO open or close parameters");
    pParamSet->updateSetting(&IOConfig, PARAM_IO);
    return ret;
}

//解析 二维码设置
int cLCDScreenProtocol::Parse2DbarCodes(const char *pData, unsigned short usLen)
{
    Q_UNUSED(usLen);
    int ret = 0;
    int iNum = pData[0];
    Node_2DbarCodeRecord_LCD * pNode;
    if(ChargeConfig.languageType == 1)
        UpdateOperateDB(01, "屏幕设置", "更新二维码设置");
    else if(ChargeConfig.languageType == 1)
        UpdateOperateDB(01, "屏幕设置", "update QR code settings");
    QString todo = "delete FROM terminal_code_param_table";
    pDBOperate->DBSqlExec(todo.toAscii().data(),DB_PARAM);
    for(int i = 0 ; i < iNum; i++)
    {
        pNode = (Node_2DbarCodeRecord_LCD *)(pData + 1 + sizeof(Node_2DbarCodeRecord_LCD) * i);
        todo.sprintf("INSERT INTO terminal_code_param_table ( canaddr, code) VALUES( %d , '%s' ) ", pNode->ucCanID, pNode->chCode);
        pDBOperate->DBSqlExec(todo.toAscii().data(),DB_PARAM);
    }
    return ret;
}

//解析 终端参数设置
int cLCDScreenProtocol::ParseTermArgSet(const char *pData, unsigned short usLen, QList<LCDMapNode> &mapList)
{
    Q_UNUSED(usLen);
    int ret = 0;
    LCDMapNode tempNode;
    QByteArray tempArray, canIDarray;
    DCTermParamSet_LCD stFrame;
    FrameActiveProtectionSet activeFrame;
    FrameGeneralStaticParamSet staticFrame;
    FrameGeneralDynamicParamSet dynamicFrame;

    memcpy(&stFrame, pData, sizeof(stFrame));
    canIDarray.append((char)stFrame.ucCanID);

    //主动防护帧设置
    memset(&activeFrame, 0xFF, sizeof(activeFrame));
    activeFrame.ucNoBelowTempProtect = stFrame.ucNoBelowTempProtect -1; //ocean add 2017-05-10
    tempArray.clear();
    tempArray.append((char *)&activeFrame, sizeof(activeFrame));
    tempNode.stMap.insert(Addr_ArgData, tempArray);
    tempNode.stMap.insert(Addr_CanID_Comm, canIDarray);
    tempNode.enType = AddrType_ActiveProtectApply;
    mapList.append(tempNode);


    //通用静态帧设置
    staticFrame.stByte1.ucAuxPowerSet = stFrame.ucAuxPowerSet -1 ;  //ocean add 2017-05-10
    staticFrame.stByte1.ucElecLockEnableFlag = stFrame.ucElecLockEnableFlag -1 ; //ocean modify
    staticFrame.stByte1.ucElecLockType = stFrame.ucElecLockType -1;  //ocean modify
    staticFrame.stByte1.ucVINEnableFlag = stFrame.ucVINEnableFlag-1; //ocean modify
    //新老国标赋值
    if(stFrame.ucBMSProType == 2)
    {
        staticFrame.stByte2.ucBMSProType = 0;
    }
    else if(stFrame.ucBMSProType == 1)
    {
        staticFrame.stByte2.ucBMSProType = 1;
    }
    else if(stFrame.ucBMSProType == 3) //nihai add
    {
        staticFrame.stByte2.ucBMSProType = 2;  //GB27930_2015
    }

//    staticFrame.stByte2.ucBMSProType = stFrame.ucBMSProType -1;
    staticFrame.stByte2.ucResrved = 0xF;
    staticFrame.ucTermID = stFrame.ucTermID;
    staticFrame.usGunMaxCurrent = (2000 - stFrame.usGunMaxCurrent)*10;
    memset(staticFrame.ucReserved, 0x00, sizeof(staticFrame.ucReserved));


    tempArray.clear();
    tempArray.append((char *)&staticFrame, sizeof(staticFrame));
    tempNode.stMap.insert(Addr_ArgData,tempArray);
    tempNode.stMap.insert(Addr_CanID_Comm, canIDarray);
    tempNode.enType = AddrType_GeneralStaticArgApply;
    mapList.append(tempNode);

    //通用动态帧设置-1
    //    memset(dynamicFrame, 0xFF, sizeof(dynamicFrame));
    dynamicFrame.ucParamType = 1;
    dynamicFrame.Data.stCmd01.ucPriority = stFrame.ucPriority;
    memset(dynamicFrame.Data.stCmd01.ucReserved, 0x00, sizeof(dynamicFrame.Data.stCmd01.ucReserved));

    tempArray.clear();
    tempArray.append((char *)&dynamicFrame, sizeof(dynamicFrame));
    tempNode.stMap.insert(Addr_ArgData,tempArray);
    tempArray.clear();
    tempArray.append((char)dynamicFrame.ucParamType);
    tempNode.stMap.insert(Addr_ArgNo, tempArray);
    tempNode.stMap.insert(Addr_CanID_Comm, canIDarray);
    tempNode.enType = AddrType_GeneralDynamicArgApply;
    mapList.append(tempNode);

    //通用动态帧设置-3
    //    memset(dynamicFrame, 0xFF, sizeof(dynamicFrame));
    dynamicFrame.ucParamType = 3;
    dynamicFrame.Data.stCmd03.ucGroupStrategy = stFrame.ucGroupStrategy; //ocean modify
    dynamicFrame.Data.stCmd03.ucGroupType = stFrame.ucGroupType; //ocean modify
    dynamicFrame.Data.stCmd03.ucWorkModle = 0xFF;


    memset(dynamicFrame.Data.stCmd03.ucReserved, 0x00, sizeof(dynamicFrame.Data.stCmd03.ucReserved));

    tempArray.clear();
    tempArray.append((char *)&dynamicFrame, sizeof(dynamicFrame));
    tempNode.stMap.insert(Addr_ArgData,tempArray);
    tempArray.clear();
    tempArray.append((char)dynamicFrame.ucParamType);
    tempNode.stMap.insert(Addr_ArgNo, tempArray);
    tempNode.stMap.insert(Addr_CanID_Comm, canIDarray);
    tempNode.enType = AddrType_GeneralDynamicArgApply;
    mapList.append(tempNode);

    return ret;
}

//解析 CCU参数设置
int cLCDScreenProtocol::ParseCCUArgSet(const char *pData, unsigned short usLen, QList<LCDMapNode> &mapList)
{
    Q_UNUSED(usLen);
    int ret = 0;
    LCDMapNode tempNode;
    QByteArray tempArray, canIDarray;
    DCCCUParamSet_LCD stFrame;
    FrameCCUParamSet_CCU ccuArgFrame;
    memcpy(&stFrame, pData, sizeof(stFrame));
    canIDarray.append((char)stFrame.ucCanID);

    //CCU参数设置
    ccuArgFrame.ucCCUID =  stFrame.ucCCUID;
    ccuArgFrame.ucTermStartID = stFrame.ucTermStartID;
    ccuArgFrame.usCabMaxPower = stFrame.usCabMaxPower * 10; //分辨率0.1
    memset(ccuArgFrame.ucReserved3, 0x00, sizeof(ccuArgFrame.ucReserved3));
    tempArray.clear();
    tempArray.append((char *)&ccuArgFrame, sizeof(ccuArgFrame));
    tempNode.stMap.insert(Addr_ArgData,tempArray);
    tempNode.stMap.insert(Addr_CanID_Comm, canIDarray);
    tempNode.enType = AddrType_CCUArgApply;
    mapList.append(tempNode);
    return ret;
}

//解析 本地充电密码设置
int cLCDScreenProtocol::ParseLocalChargePassword(const char *pData)
{
    int ret = 0;
    //获取配置
    QueryParamInfo();
    cscuSysConfig.localChargePassword = *(int *)(pData);
    UpdateOperateDB(01, "屏幕设置", "更新本地充电密码");
    pParamSet->updateSetting(&cscuSysConfig, PARAM_CSCU_SYS);
    return ret;
}

//解析 刷卡申请卡号信息
void cLCDScreenProtocol::ParseCardInfoApply(const char * pData, unsigned short usLen, CardResult_LCD & stResult)
{
    CardInfoApply_LCD stFrame;
    memcpy(&stFrame, pData, usLen);
    stResult.ucCanID = stFrame.ucCanID;
    //stResult.ucCardFlag = stFrame.ucCardFlag; //nihai add 20170523 增加刷卡开始停止
    stResult.ucFlag = stFrame.ucFlag;
    //    stResult.ucState = stFrame.ucState;
}

//解析 刷卡申请充电,结束充电
void cLCDScreenProtocol::ParseCardCmdApply(const char * pData, unsigned short usLen, CardCmdApply_LCD & stResult)
{
    CardCmdApply_LCD stFrame;
    memcpy(&stFrame, pData, usLen);
    stResult.ucCanID = stFrame.ucCanID;
    stResult.ucFlag = stFrame.ucFlag;
    stResult.ucState = stFrame.ucState;
    stResult.ucType = stFrame.ucType;
    stResult.usEnergy = stFrame.usEnergy;
}

//获取 人机交互结果
void cLCDScreenProtocol::MakeFrameOperateResult(unsigned char * &pSend, unsigned short &usLen, unsigned char &ucAddr, unsigned short usDataType, unsigned char ucResult, unsigned char ucReboot)
{
    //长度
    usLen = sizeof(FrameOperateResult_LCD);
    //分配内存
    unsigned char * pData = new unsigned char[usLen];
    memset(pData, 0, usLen);
    FrameOperateResult_LCD * pScreen = (FrameOperateResult_LCD *)pData;
    pScreen->usDataType = usDataType;
    pScreen->ucResult = ucResult;
    pScreen->ucReboot = ucReboot;
    MakeFrameFormat(pSend, ucAddr, NormalType_LCD, Cmd_InterActive_LCD, Data_SetResult_LCD, usLen, pData);
    delete pData;
}

//生成 刷卡结果
void cLCDScreenProtocol::MakeFrameCardResult(unsigned char * &pSend, unsigned short &usLen, unsigned char &ucAddr, CardResult_LCD &stCardResult, unsigned int &uiDataType)
{
    TerminalStatus stTerm;
    if(! pDevCache->QueryTerminalStatus(stCardResult.ucCanID,stTerm))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! Screen pDevCache Query  Error!");
        return ;
    }
    //长度
    usLen = sizeof(CardResult_LCD);
    //分配内存
    unsigned char * pData = new unsigned char[usLen];
    memset(pData, 0, usLen);
    //赋值
    stCardResult.ucState = stTerm.cStatus;
    memcpy(pData, &stCardResult, usLen);
    MakeFrameFormat(pSend, ucAddr, NormalType_LCD, Cmd_InterActive_LCD, uiDataType, usLen, pData);
    delete pData;
}

//生成 U盘结果
void cLCDScreenProtocol::MakeFrameUdiskResult(unsigned char * &pSend, unsigned short &usLen, unsigned char &ucAddr, UdiskResult_LCD & stResult)
{
    //长度
    usLen = sizeof(UdiskResult_LCD);
    //分配内存
    unsigned char * pData = new unsigned char[usLen];
    memset(pData, 0, usLen);
    //赋值
    memcpy(pData, &stResult, usLen);
    MakeFrameFormat(pSend, ucAddr, NormalType_LCD, Cmd_InterActive_LCD, Data_UdiskUpdateAck_LCD, usLen, pData);
    delete pData;
}

//生成 遥控指令结果
void cLCDScreenProtocol::MakeFrameCtrlCmdResult(unsigned char * &pSend, unsigned short &usLen, unsigned char &ucAddr, LocalCharge_LCD &stLocalCharge)
{
    //长度
    usLen = sizeof(LocalCharge_LCD);
    //分配内存
    unsigned char * pData = new unsigned char[usLen];
    memset(pData, 0, usLen);
    //赋值
    memcpy(pData, &stLocalCharge, usLen);
    MakeFrameFormat(pSend, ucAddr, NormalType_LCD, Cmd_InterActive_LCD, Data_LocalChargeCmd_LCD, usLen, pData);
    delete pData;
}

//帧格式打包
///
/// \brief cLCDScreenProtocol::MakeFrameFormat
/// \param pSendFrame : 组帧后指针
/// \param ucAddr : 屏幕地址
/// \param ucProType : 协议类型
/// \param ucCmdType : 通信指令类型
/// \param usDataType : 数据类型
/// \param usLen : 数据长度
/// \param pData : 未打包前原始数据
///
inline void cLCDScreenProtocol::MakeFrameFormat(unsigned char * &pSendFrame, unsigned char &ucAddr, unsigned char ucProType, unsigned char ucCmdType,
                                                unsigned short usDataType, unsigned short &usLen, unsigned char * pData)
{
    unsigned char ucSum = 0;
    pSendFrame = new unsigned char[usLen + 11];
    //    pSendFrame = new unsigned char[150];
    pSendFrame[0] = 0x68;   //帧头0x68
    pSendFrame[1] = ucAddr; //屏幕地址
    pSendFrame[2] = 0x68;   //帧头0x68
    pSendFrame[3] = ucProType;  //协议类型
    pSendFrame[4] = ucCmdType;  //通信指令类型
    pSendFrame[5] = (usDataType & (0x00FF)) ;  //数据类型高位
    pSendFrame[6] = (usDataType & (0xFF00)) >> 8;  //数据类型低位
    pSendFrame[7] = (usLen & (0x00FF)) ;  //数据长度高位
    pSendFrame[8] = (usLen & (0xFF00)) >> 8;  //数据长度低位

    memcpy(pSendFrame + 9, pData, usLen);   //数据赋值
    //计算校验和
    for(unsigned short i = 3; i <= usLen + 6 + 2; i++)
    {
        ucSum += pSendFrame[i];
    }
    pSendFrame[usLen + 9] = ucSum;   //校验和
    pSendFrame[usLen + 10] = 0x16;   //帧尾
    usLen += 11;
}

//查询 充电记录
void cLCDScreenProtocol::QueryChargeRecord()
{
    //数据库操作----查询充电记录
    db_result_st dbst;
    Node_ChargeRecord_LCD tempNode;
    unsigned char ucDevStopReason = 0;
    QByteArray tempArray;
    ChargeRecordList.clear();
    if(0 != pDBOperate->DBSqlQuery("SELECT canaddr, StartTime, EndTime, TotalChargeEnergy, DevStopReason FROM charge_order", &dbst, DB_PROCESS_RECORD))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! QueryChargeRecord Query Error");
        return;
    }
    //充电记录赋值
    for(int i = 0; i < dbst.row; i++)
    {
        memset(&tempNode, 0x00, sizeof(tempNode));
        tempNode.ucCanAddr = atoi(dbst.result[i * dbst.column]);
        strncpy(tempNode.chStartTime, dbst.result[i * dbst.column + 1], sizeof(tempNode.chStartTime));
        strncpy(tempNode.chStopTime, dbst.result[i * dbst.column + 2], sizeof(tempNode.chStopTime));
        tempNode.sEnergy = (short) (atoi(dbst.result[i * dbst.column + 3])); //nihai 20170606 modify 根据协议电能需要扩大100倍,将atoi修改为atof
        ucDevStopReason = (short) atoi(dbst.result[i * dbst.column + 4]);
        if(ChargeConfig.languageType == 1)
            ChangeDevStopReasonToArray(ucDevStopReason, tempArray);
        else if(ChargeConfig.languageType == 2)
            ChangeDevStopReasonToEnglishArray(ucDevStopReason, tempArray);
        strncpy(tempNode.chReason, tempArray.data(), tempArray.length());
        ChargeRecordList.append(tempNode);
    }
    pDBOperate->DBQueryFree(&dbst);
}

//查询 故障记录
void cLCDScreenProtocol::QueryFaultRecord()
{
    //数据库操作----查询故障记录
    db_result_st dbst;
    Node_FaultRecord_LCD tempNode;
    unsigned char ucFaultCode = 0;
    QByteArray tempArray;
    FaultRecordList.clear();
    if(0 != pDBOperate->DBSqlQuery("SELECT canaddr, fault_start_time, fault_stop_time, fault_code FROM terminal_fault_table  where fault_stop_time!=\"\"", &dbst, DB_PROCESS_RECORD)) // mod by muty 20170919
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! QueryChargeRecord Query Error");
        return;
    }
    //故障记录赋值
    for(int i = 0; i < dbst.row; i++)
    {
        memset(&tempNode, 0x00, sizeof(tempNode));
        tempNode.ucCanAddr = atoi(dbst.result[i * dbst.column]);
        tempNode.ucInnerAddr = 0;
        tempNode.chType = 1;
        tempNode.ucMaxPDUAddr = 0;
        tempNode.ucMinPDUAddr = 0;
        strncpy(tempNode.chStartTime, dbst.result[i * dbst.column + 1], sizeof(tempNode.chStartTime));
        strncpy(tempNode.chStopTime, dbst.result[i * dbst.column + 2], sizeof(tempNode.chStopTime));
        ucFaultCode = (unsigned char) atoi(dbst.result[i * dbst.column + 3]);
        if(ChargeConfig.languageType == 1)
            ChangeFaultCodeToArray(ucFaultCode, tempArray);
        else if(ChargeConfig.languageType == 2)
            ChangeFaultCodeToEnglishArray(ucFaultCode, tempArray);
        strncpy(tempNode.chReason, tempArray.data(), tempArray.length());
        FaultRecordList.append(tempNode);
    }
    pDBOperate->DBQueryFree(&dbst);
    memset(&dbst, 0x00, sizeof(dbst));
    //设备管理故障赋值
    if(0 != pDBOperate->DBSqlQuery("SELECT canaddr, module_id, fault_code, min_pdu_id, max_pdu_id, start_time, stop_time FROM dc_cabinet_fault_table where record_state = 0 AND stop_time !=\"\"", &dbst, DB_PROCESS_RECORD))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! QueryChargeRecord Query Error");
        return;
    }
    //故障记录赋值
    for(int i = 0; i < dbst.row; i++)
    {
        memset(&tempNode, 0x00, sizeof(tempNode));
        tempNode.ucCanAddr = atoi(dbst.result[i * dbst.column]);
        tempNode.ucInnerAddr = atoi(dbst.result[i * dbst.column + 1]);
        ucFaultCode = (unsigned char) atoi(dbst.result[i * dbst.column + 2]);
        if(ChargeConfig.languageType == 1)
            ChangeFaultCodeToArray_DeviceManage(ucFaultCode, tempArray);
        else if(ChargeConfig.languageType == 2)
            ChangeFaultCodeToEnglishArray_DeviceManage(ucFaultCode, tempArray);
        strncpy(tempNode.chReason, tempArray.data(), tempArray.length());

        tempNode.chType = 2;
        tempNode.ucMinPDUAddr = atoi(dbst.result[i * dbst.column + 3]);
        tempNode.ucMaxPDUAddr = atoi(dbst.result[i * dbst.column + 4]);
        strncpy(tempNode.chStartTime, dbst.result[i * dbst.column + 5], sizeof(tempNode.chStartTime));
        strncpy(tempNode.chStopTime, dbst.result[i * dbst.column + 6], sizeof(tempNode.chStopTime));
        FaultRecordList.append(tempNode);
    }
    pDBOperate->DBQueryFree(&dbst);
    memset(&dbst, 0x00, sizeof(dbst));

}

/***********************************************************************
 *函数名：GetFrameNoRemoveFaultRecordNum
 *参数说明：null
 *返回值:      void
 *功能：查询 未消除故障记录
 *备注 ：by muty 20170913
 **********************************************************************/
void cLCDScreenProtocol::QueryNoRemoveFaultRecord()
{
    //数据库操作----查询故障记录
    db_result_st dbst;
    Node_NoRemoveFaultRecord_LCD tempNode;
    unsigned char ucFaultCode = 0;
    QByteArray tempArray;
    NoRemoveFaultRecordList.clear();
    if(0 != pDBOperate->DBSqlQuery("SELECT canaddr, fault_start_time,  fault_code FROM terminal_fault_table where fault_stop_time=\"\"", &dbst, DB_PROCESS_RECORD)) // test 查询条件需要调试
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! QueryNoRemoveFaultRecord Query Error");
        return;
    }
    //未消除故障记录赋值
    for(int i = 0; i < dbst.row; i++)
    {
        memset(&tempNode, 0x00, sizeof(tempNode));
        tempNode.ucCanAddr = atoi(dbst.result[i * dbst.column]);
        tempNode.ucInnerAddr = 0;
        tempNode.chType = 1;
        tempNode.ucMaxPDUAddr = 0;
        tempNode.ucMinPDUAddr = 0;
        strncpy(tempNode.chStartTime, dbst.result[i * dbst.column + 1], sizeof(tempNode.chStartTime));
        ucFaultCode = (unsigned char) atoi(dbst.result[i * dbst.column + 2]);
        if(ChargeConfig.languageType == 1)
            ChangeFaultCodeToArray(ucFaultCode, tempArray);
        else if(ChargeConfig.languageType == 2)
            ChangeFaultCodeToEnglishArray(ucFaultCode, tempArray);
        strncpy(tempNode.chReason, tempArray.data(), tempArray.length());
        NoRemoveFaultRecordList.append(tempNode);
    }
    pDBOperate->DBQueryFree(&dbst);
    memset(&dbst, 0x00, sizeof(dbst));
    //设备管理故障赋值
    if(0 != pDBOperate->DBSqlQuery("SELECT canaddr, module_id, fault_code, min_pdu_id, max_pdu_id, start_time FROM dc_cabinet_fault_table where record_state = 85 AND stop_time is null", &dbst, DB_PROCESS_RECORD))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! QueryNoRemoveFaultRecord Query Error");
        return;
    }
    //故障记录赋值
    for(int i = 0; i < dbst.row; i++)
    {
        memset(&tempNode, 0x00, sizeof(tempNode));
        tempNode.ucCanAddr = atoi(dbst.result[i * dbst.column]);
        tempNode.ucInnerAddr = atoi(dbst.result[i * dbst.column + 1]);
        ucFaultCode = (unsigned char) atoi(dbst.result[i * dbst.column + 2]);

        if(ChargeConfig.languageType == 1)
            ChangeFaultCodeToArray_DeviceManage(ucFaultCode, tempArray);
        else if(ChargeConfig.languageType == 2)
            ChangeFaultCodeToEnglishArray_DeviceManage(ucFaultCode, tempArray);

        strncpy(tempNode.chReason, tempArray.data(), tempArray.length());

        tempNode.chType = 2;
        tempNode.ucMinPDUAddr = atoi(dbst.result[i * dbst.column + 3]);
        tempNode.ucMaxPDUAddr = atoi(dbst.result[i * dbst.column + 4]);
        strncpy(tempNode.chStartTime, dbst.result[i * dbst.column + 5], sizeof(tempNode.chStartTime));
        NoRemoveFaultRecordList.append(tempNode);
    }
    pDBOperate->DBQueryFree(&dbst);
    memset(&dbst, 0x00, sizeof(dbst));
}


//查询 操作记录
void cLCDScreenProtocol::QueryOperateRecord()
{
    //数据库操作----查询操作记录
    db_result_st dbst;
    Node_OperateRecord_LCD tempNode;
    OperateRecordList.clear();
    if(0 != pDBOperate->DBSqlQuery("SELECT opt_type, opt_name, opt_time, opt_data FROM operate_record_table", &dbst, DB_PROCESS_RECORD))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! QueryOperateRecord Query Error");
        return;
    }
    //操作记录赋值
    for(int i = 0; i < dbst.row; i++)
    {
        memset(&tempNode, 0x00, sizeof(tempNode));
        tempNode.ucOptType = atoi(dbst.result[i * dbst.column]);
        strncpy(tempNode.chOptName, dbst.result[i * dbst.column + 1], sizeof(tempNode.chOptName));
        strncpy(tempNode.chOptTime, dbst.result[i * dbst.column + 2], sizeof(tempNode.chOptTime));
        strncpy(tempNode.chOptData, dbst.result[i * dbst.column + 3], sizeof(tempNode.chOptData));
        OperateRecordList.append(tempNode);
    }
    pDBOperate->DBQueryFree(&dbst);
}

//查询 数据库, 设备规格信息记录
void cLCDScreenProtocol::queryDBSpecInfoRecord(QList<SpecificInfo_DCcab> &list)
{
    db_result_st dbst;
    QString todo;
    SpecificInfo_DCcab tempStr;
    //数据库操作----查询静态参数
    todo = "SELECT canaddr, interid, slotnum, serialnum, softversion1,  softversion2, softversion3, hdwversion FROM format_data_table order by canaddr,interid";  //nihai 增加排序
    if(0 != pDBOperate->DBSqlQuery(todo.toAscii().data(), &dbst, DB_PROCESS_RECORD))
    {
        //pLog->getLogPoint(LOG_ENERGY_CONSUME_SERVER)->info("ERROR! ParseTermStaticArg Query Error");
    }
    if(dbst.row != 0)
    {
        for(int i = 0; i < dbst.row; i++)
        {
            tempStr.ucCCUAddr = atoi(dbst.result[i * dbst.column]);
            tempStr.ucDevID = atoi(dbst.result[i * dbst.column + 1]);
            tempStr.ucSlotNum = atoi(dbst.result[i * dbst.column + 2]);
            tempStr.SerialNumber = dbst.result[i * dbst.column + 3];
            tempStr.SoftwareVer = dbst.result[i * dbst.column + 4];
            tempStr.SoftwareVer1 = dbst.result[i * dbst.column + 5];
            tempStr.SoftwareVer2 = dbst.result[i * dbst.column + 6];
            tempStr.HardwareVer = dbst.result[i * dbst.column + 7];
            list.append(tempStr);
        }
    }
    pDBOperate->DBQueryFree(&dbst);
}

//更新 操作记录
//ucOptType: 01, 屏幕操作 02, U盘操作
//name: 操作者身份
//data: 操作内容
void cLCDScreenProtocol::UpdateOperateDB(unsigned char ucOptType, const char * pname, const char *  pdata)
{
    //数据库操作----更新操作记录

    QString exec;
    QString time;
    time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    OperateRecordList.clear();
    exec.sprintf("INSERT INTO operate_record_table ( opt_type, opt_name, opt_time, opt_data ) VALUES( %d , '%s', '%s', '%s' ) ", ucOptType, pname, time.toAscii().data(), pdata);
    pDBOperate->DBSqlExec(exec.toAscii().data(), DB_PROCESS_RECORD);
}

//初始化终端名称图(写TermNameMap)
void cLCDScreenProtocol::InitTermNameMap()
{
    //数据库操作----查询终端名称表,并赋值到TermNameMap
    db_result_st dbst;
    int iTermID = 0;
    char * pName = NULL;
    QByteArray tmpArray;
    if(0 != pDBOperate->DBSqlQuery("SELECT canaddr, name FROM terminal_name_table", &dbst, DB_PARAM))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! InitTermNameMap Query Error");
        return;
    }
    //终端名称赋值
    for(int i = 0; i < dbst.row; i++)
    {
        iTermID = atoi(dbst.result[i * dbst.column]);
        pName = dbst.result[i * dbst.column + 1];
        tmpArray.clear();
        tmpArray.append(pName);
        NameMap.insert((unsigned char )iTermID, tmpArray);
    }
    pDBOperate->DBQueryFree(&dbst);
}
//初始化终端名称图(写TermNameMap)
void cLCDScreenProtocol::InitTermNameMapShow()
{
    //数据库操作----查询终端名称表,并赋值到TermNameMap
    db_result_st dbst;
    int iTermID = 0;
    char * pName = NULL;
    QByteArray tmpArray;
    QString todo = "SELECT canaddr, name FROM terminal_name_show_table";
    if(0 != pDBOperate->DBSqlQuery(todo.toAscii().data(), &dbst, DB_PARAM))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! InitTermNameMap Query Error");
        NameMapShow = NameMap;
        return;
    }
    if(dbst.row<1)
    {
        NameMapShow = NameMap;
    }else
    {
        NameMapShow.clear();
        //终端名称赋值
        for(int i = 0; i < dbst.row; i++)
        {
            iTermID = atoi(dbst.result[i * dbst.column]);
            pName = dbst.result[i * dbst.column + 1];
            tmpArray.clear();
            tmpArray.append(pName);
            NameMapShow.insert((unsigned char )iTermID, tmpArray);
        }
    }
    pDBOperate->DBQueryFree(&dbst);
}

//初始化终端名称图(写TermNameMap)
void cLCDScreenProtocol::InitTermNameMapMulti()
{
    //数据库操作----查询终端名称表,并赋值到TermNameMap
    //printf("+++++++++++InitTermNameMapShow \n");
    db_result_st dbst;
    int iTermID = 0;
    char * pName = NULL;
    QByteArray tmpArray;
    QString todo = "SELECT canaddr, name FROM terminal_name_multi_table";
    if(0 != pDBOperate->DBSqlQuery(todo.toAscii().data(), &dbst, DB_PARAM))
    {
        pLog->getLogPoint(_strLogName)->info("ERROR! InitTermNameMap Query Error");
        NameMapShow = NameMap;
        return;
    }
    if(dbst.row<1)
    {
        NameMapShow = NameMap;
    }else
    {
        NameMapShow.clear();
        terminalNum = dbst.row;
        //终端名称赋值
        for(int i = 0; i < dbst.row; i++)
        {
            unsigned char cannum = i + 181;
            iTermID = atoi(dbst.result[i * dbst.column]);
            pName = dbst.result[i * dbst.column + 1];
            tmpArray.clear();
            tmpArray.append(pName);
            NameMapShow.insert((unsigned char )iTermID, tmpArray);
            canAddr.insert(cannum,(unsigned char )iTermID);
        }
    }
    pDBOperate->DBQueryFree(&dbst);
}

//生成 遥控指令结果
void cLCDScreenProtocol::MakeFrameCtrlStopCmdResult(unsigned char * &pSend, unsigned short &usLen, unsigned char &ucAddr, LocalChargeStop_LCD &stLocalCharge)
{
    //长度
    usLen = sizeof(LocalChargeStop_LCD);
    //分配内存
    unsigned char * pData = new unsigned char[usLen];
    memset(pData, 0, usLen);
    //赋值
    memcpy(pData, &stLocalCharge, usLen);
    MakeFrameFormat(pSend, ucAddr, NormalType_LCD, Cmd_InterActive_LCD, Data_LocalChargeStop_LCD, usLen, pData);
    delete pData;
}

//生成 遥控指令结果
void cLCDScreenProtocol::MakeFrameTickectDevResult(unsigned char * &pSend, unsigned short &usLen, unsigned char &ucAddr, TicketDevResult_LCD &stTicketDev)
{
    //长度
    usLen = sizeof(TicketDevResult_LCD);
    //分配内存
    unsigned char * pData = new unsigned char[usLen];
    memset(pData, 0, usLen);
    //赋值
    memcpy(pData, &stTicketDev, usLen);
    MakeFrameFormat(pSend, ucAddr, NormalType_LCD, Cmd_InterActive_LCD, Data_TicketDev_LCD, usLen, pData);
    delete pData;
}
