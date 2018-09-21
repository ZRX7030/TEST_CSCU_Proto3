#ifndef GENERALDATA_H
#define GENERALDATA_H

#include <QDateTime>
#include <QMap>
#include <QByteArray>
#include <QVariant>
#include <linux/types.h>

#include "104_info_struct.h"

//const int作为key值会导致map内部错误???
typedef  const unsigned int InfoAddr;

//终端名称对应图, key-终端地址, value-终端名称 ---- 串口屏模块, LCD屏模块共用结构
typedef QMap <unsigned char, QByteArray> TermNameMap;

//主版本号, 配置路径, Lib库路径

#define MAIN_VERSION	"CSCU_A1_30001_alpha"

#define MAIN_CONFIG		"/mnt/nandflash/config.ini"
#define LIB_CONFIG		"/mnt/nandflash/libconfig.ini"
#define LIB_PATH		"/mnt/nandflash/lib/"

#define TIMEOUT_CHARGE_STEP_WAITCMD_ACK 5
#define TIMEOUT_CHARGE_STEP_WAITDEV_START_CHARGE 60
#define TIMEOUT_CHARGE_STEP_WAITDEV_STOP_CHARGE 60
#define TIMEOUT_CHARGE_STEP_WAITDEV_START_DISCHARGE 60
#define TIMEOUT_CHARGE_STEP_WAITDEV_LIMIT 60
#define TIMEOUT_CHARGE_STEP_WAITDEV_PAUSH 60
#define TIMEOUT_CHARGE_STEP_WAITDEV_RESUME 60
#define TIMEOUT_CHARGE_STEP_WAITDEV_RESET 60
#define TIMEOUT_CHARGE_STEP_WAITSEVER_ACK_APPLY_START_CHARGE_VIN 15
#define TIMEOUT_CHARGE_STEP_WAITSEVER_ACK_APPLY_STOP_CHARGE_VIN 15
#define TIMEOUT_CHARGE_STEP_WAITSEVER_ACK_APPLY_START_CHARGE_CARD 15
#define TIMEOUT_CHARGE_STEP_WAITSEVER_ACK_APPLY_STOP_CHARGE_CARD 15
#define TIMEOUT_CHARGE_STEP_WAITDEV_STOP_REASON 2

//串口屏串口说明:
//#define SCREEN_SERIAL_NUM_3     "/dev/ttyUSB2"  //模块化集控
#define SCREEN_SERIAL_NUM_3     "/dev/ttyX_2"  //模块化集控
#define SCREEN_SERIAL_NUM_2     "/dev/ttyS2"  //2.0集控
#define SCREEN_SERIAL_NUM_1      "/dev/ttyS2"//"/dev/ttyS1"  //1.0集控

//电表串口说明:
#define AMMETER_SERIAL_NUM_3   "/dev/ttyO2"  //模块化集控
#define AMMETER_SERIAL_NUM_2   "/dev/ttyS4" //2.0集控
#define AMMETER_SERIAL_NUM_1   "/dev/ttyS3" //简版集控

//刷卡器串口说明:
#define CARD_SERIAL_NUM_2   "/dev/ttyO3" //模块化集控
#define CARD_SERIAL_NUM_1   "/dev/ttyS3" //2.0集控
#define CARD_SERIAL_NUM_3   "/dev/ttyS5" //2.0集控(适用于1.0地板)

//小票机串口说明:
#define TicketDev_SERIAL_NUM  "/dev/ttyS6"  //2.0集控
#define TicketDev_SERIAL_NUM_1  "/dev/ttyX_1"  //模块化集控

//温湿度计串口说明:

#define TEMPERATURE_SERIAL_NUM_3   "/dev/ttyO4"//模块化集控
#define TEMPERATURE_SERIAL_NUM_2   "/dev/ttyS5"//2.0集控
#define TEMPERATURE_SERIAL_NUM_1   "/dev/ttyS3"//原板集控

//车位锁串口说明
#define PARKINGLOCK_SERIAL_NUM_3   "/dev/ttyO4"//模块化集控
#define PARKINGLOCK_SERIAL_NUM_2   "/dev/ttyS5"//2.0集控
#define PARKINGLOCK_SERIAL_NUM_1   "/dev/ttyS3"//原板集控

#define FRAME_START 0x68
#define FRAME_END 0x16
#define OFFSET 0x33
#define DLT_read_max_num 200

//单双枪枪类型
#define UNKNOWN          0
#define MASTER_GUN      1
#define SLAVE_GUN          2
#define COUPLE_ERR        3
//单双枪充电方式
#define SINGLE_CHARGE  1
#define COUPLE_CHARGE 2

//CSCU总线信息体地址分类
typedef enum __CSCU_INFOTAG_TYPE{
    CSCU_INFOTAG_TYPE_CMD =0, //
    CSCU_INFOTAG_TYPE_ACK =1 //
}CSCU_INFOTAG_TYPE;

//645协议命令帧
typedef struct _FRAME_DLT_645
{
    unsigned char frame_start;
    unsigned char addr[6];
    unsigned char frame_start2;
    unsigned char controlCode;
    unsigned char data_length;
    unsigned char data[DLT_read_max_num];
    unsigned char checkCode;
    unsigned char frame_end;
}__attribute__((packed)) FRAME_DLT_645;

////电表信息结构体
//typedef struct _ammeterPara{
//    char addr[6];                        //电表地址
//    unsigned short proType;     //协议类型
//    unsigned short PT_value;    //电压变比
//    unsigned short CT_value;    //电流变比
//}ammeterPara;

//编程备用
//case CHARGE_CMD_SRC_NON: //无
//case CHARGE_CMD_SRC_CLOUD: //云平台
//case CHARGE_CMD_SRC_STATION_MONITOR: //场站监控
//case CHARGE_CMD_SRC_CSCU: //充电系统控制器
//case CHARGE_CMD_SRC_CAN_DEV:  //充电设备

//充电指令触发源
typedef enum __CHARGE_CMD_SRC_TYPE{
    CHARGE_CMD_SRC_NON =0, //无
    CHARGE_CMD_SRC_APP = 1,//手机APP
    CHARGE_CMD_SRC_STATION_MONITOR = 2, //场站监控
    CHARGE_CMD_SRC_WEB_PAGE = 3, //调度页面
    CHARGE_CMD_SRC_CLOUD = 4, //平台操作
    CHARGE_CMD_SRC_DEV_APPLY = 5, //设备侧请求
    CHARGE_CMD_SRC_OPEN_CLOUD = 6, //第三方平台
    //以上为IEC104定义V2.2.4,以下为CSCU内部扩展
    CHARGE_CMD_SRC_CSCU = 101, //101 充电系统控制器
    CHARGE_CMD_SRC_CAN_DEV = 102, //102 充电设备
    CHARGE_CMD_SRC_CSCU_EMERGENCY = 103, //103 应急充电
}CHARGE_CMD_SRC_TYPE;


//开始充电指令原因
typedef enum __START_CHARGE_CMD_RSN{
    START_CHARGE_CMD_RSN_NON =0, //无

    START_CHARGE_CMD_RSN_USER_MAKE = 1,//用户主动操作
    START_CHARGE_CMD_RSN_TIMING = 2, //定时开始
    START_CHARGE_CMD_RSN_RESEND = 3,//重发
    START_CHARGE_CMD_RSN_QUEUE = 4,//排队
    START_CHARGE_CMD_RSN_VIN_APPLY = 5,//vin申请
    START_CHARGE_CMD_RSN_CAR_LICENCE_APPLY = 6,//车牌号申请
    START_CHARGE_CMD_RSN_CARD_APPLY = 7,//刷卡申请
    //-------------以上为IEC104协议定义V2.2.4-----以下为CSCU内部定义
    START_CHARGE_CMD_RSN_STATION_ADMIN = 101, //场站管理员
    START_CHARGE_CMD_RSN_CARD_REMOTE = 102 , //102 集中刷卡（远程）
    START_CHARGE_CMD_RSN_CARD_LOCAL = 103 , //103 集中刷卡（本地）
    START_CHARGE_CMD_RSN_TEUI  = 104, //本地交互界面按钮
    START_CHARGE_CMD_RSN_BUTTON_LOCAL = 105 , //本地物理按钮
    START_CHARGE_CMD_RSN_DEV_CARD_REMOTE  = 106, //106 单桩刷卡（远程）
    START_CHARGE_CMD_RSN_DEV_CARD_LOCAL = 107 , //107 单桩刷卡（本地）
    START_CHARGE_CMD_RSN_DEV_VIN_REMOTE = 108 , //108 VIN号（远程）
    START_CHARGE_CMD_RSN_DEV_VIN_LOCAL = 109,  //109 VIN号（本地）
    START_CHARGE_CMD_RSN_DEV_CAR_LICENCE_REMOTE = 110, //110 车牌号（远程）
    START_CHARGE_CMD_RSN_DEV_CAR_LICENCE_LOCAL = 111,  //111 车牌号（本地）
    START_CHARGE_CMD_RSN_DEV_SELF = 112, //设备自己启动
    START_CHARGE_CMD_RSN_VIN6 = 113, //113VIN号后6位（本地）
    START_CHARGE_CMD_RSN_SMART_CHARGE_VIN = 114, //错峰充电VIN申请
    START_CHARGE_CMD_RSN_SMART_CHARGE_CAR_LICENCE = 115, //错峰充电车牌号申请
    START_CHARGE_CMD_RSN_SMART_CHARGE_CARD = 116,//错峰充电卡号申请
    START_CHARGE_CMD_RSN_EMERGENCY_VIN = 117,//VIN应急充电
    START_CHARGE_CMD_RSN_EMERGENCY_CAR_LICENCE = 118,//车牌号应急充电
    START_CHARGE_CMD_RSN_EMERGENCY_CARD = 119,//刷卡应急充电
    START_CHARGE_CMD_RSN_PEAK_CHARGE = 120,//削峰填谷申请   hd
    START_CHARGE_CMD_RSN_ECNOMIC_CHARGE = 121,//经济充电
    START_CHARGE_CMD_RSN_COUPLE_CHARGE_VIN = 122//多枪自动VIN申请
}START_CHARGE_CMD_RSN;

//编程备用
//case CHARGE_STATUS_CHANGE_NON: //无
//case CHARGE_STATUS_CHANGE_START_CHARGE: //开始充电
//case CHARGE_STATUS_CHANGE_STOP_CHARGE: //停止充电
//case CHARGE_STATUS_CHANGE_START_DISCHARGE: //开始放电
//case CHARGE_STATUS_CHANGE_STOP_DISCHARGE: //结束放电
//case CHARGE_STATUS_CHANGE_START_CHARGE_FAIL: //启动充电失败

//add by YCZ 2016-08-16
//充电机状态变化类型
typedef enum __CHARGE_STATUS_CHANGE_TYPE{
    CHARGE_STATUS_CHANGE_NON =0, //无
    CHARGE_STATUS_CHANGE_START_CHARGE =1, //开始充电
    CHARGE_STATUS_CHANGE_STOP_CHARGE, //停止充电
    CHARGE_STATUS_CHANGE_PAUSE_CHARGE, //暂停充电--适用于充电中->暂停
    CHARGE_STATUS_CHANGE_START_DISCHARGE, //开始放电
    CHARGE_STATUS_CHANGE_STOP_DISCHARGE, //结束放电
    CHARGE_STATUS_CHANGE_PAUSE_DISCHARGE, //暂停放电
    CHARGE_STATUS_CHANGE_START_CHARGE_FAIL,//启动充电失败
    CHARGE_STATUS_CHANGE_STARTING, //启动中
    CHARGE_STATUS_CHANGE_RECOVER_PAUSE_CHARGE ,//恢复暂停充电
    CHARGE_STATUS_CHANGE_START_PAUSE_CHARGE //开始暂停充电--适用于枪连接->暂停
}CHARGE_STATUS_CHANGE_TYPE;

//状态机超时定时器,或者指令校验判断的充电指令执行结果
typedef enum __CMD_END_REASON_TYPE{
    CMD_END_REASON_NULL,//空
    //-------------------触发类---------------------------------//
    CMD_END_REASON_ACK_OK,//响应成功
    CMD_END_REASON_WAITACK_WRONG,//非允许ACK
    CMD_END_REASON_EXE_OK,//执行成功
    CMD_END_REASON_START_CHARGE_FAIL,//启动失败
    //-------------------超时类---------------------------------//
    CMD_END_REASON_WAITACK_TIMEOUT,//接收ACK超时
    CMD_END_REASON_START_CHARGE_TIMEOUT,//启动充电超时
    CMD_END_REASON_STOP_CHARGE_TIMEOUT, //结束充电超时
    CMD_END_REASON_START_DISCHARGE_TIMEOUT, //启动放电超时
    CMD_END_REASON_STOP_DISCHARGE_TIMEOUT ,//结束放电超时
    //-------------------逻辑加工类---------------------------------//
    CMD_END_REASON_IERR,//内部错误
    CMD_END_REASON_ALREADY_EXE,//已经有指令执行
    CMD_END_REASON_ALREADY_CHARGING,//已经在充电
    CMD_END_REASON_GUN_NOLINK,//未插抢
    CMD_END_REASON_NOT_CHARGING,//没有在充电
    CMD_END_REASON_DEV_OFFLINE,//设备离线
    CMD_END_REASON_DEV_FAULT,//设备故障
    CMD_END_REASON_NOW_LIMIT,//设备限制充电中
    CMD_END_REASON_NOW_PAUSH,//设备暂停充电中
    CMD_END_REASON_NOW_DISCHARGING//设备放电中
}CMD_END_REASON_TYPE;


//编程备用
//case CHARGE_STATUS_REALTIME_STANDBY: //0 待机
//case CHARGE_STATUS_REALTIME_CHARGING://1.充电中
//case CHARGE_STATUS_REALTIME_FAULT://2.故障
//case CHARGE_STATUS_REALTIME_STARTING://3.启动中
//case CHARGE_STATUS_REALTIME_PAUSE://4.暂停
//case CHARGE_STATUS_REALTIME_LIMIT://5.限制
//case CHARGE_STATUS_REALTIME_OFFLINE://6.离线
//case CHARGE_STATUS_REALTIME_SWITCH://7.切换中
//case CHARGE_STATUS_REALTIME_DISCHARGING: //8放电

//实时工作状态 7种
typedef enum __CHARGE_STATUS_REALTIME{
    CHARGE_STATUS_REALTIME_STANDBY = 0,//0 待机
    CHARGE_STATUS_REALTIME_CHARGING,//1.充电中
    CHARGE_STATUS_REALTIME_FAULT,//2.故障
    CHARGE_STATUS_REALTIME_STARTING,//3.启动中
    CHARGE_STATUS_REALTIME_PAUSE,//4.暂停
    CHARGE_STATUS_REALTIME_LIMIT,//5.限制
    CHARGE_STATUS_REALTIME_OFFLINE,//6.离线
    CHARGE_STATUS_REALTIME_SWITCH,//7.切换中
    CHARGE_STATUS_REALTIME_DISCHARGING, //8放电
    CHARGE_STATUS_REALTIME_SLAVE,//9副枪
    CHARGE_STATUS_REALTIME_WAITING =10, //10 等待中
    CHARGE_STATUS_REALTIME_CARPAUSE,//11 车辆暂停
    CHARGE_STATUS_REALTIME_DEVPAUSE,//12 充电设备暂停
    CHARGE_STATUS_REALTIME_UNKNOWN = 0xFF//未定义充电机状态
}CHARGE_STATUS_REALTIME;


//编程备用
//case CHARGE_STATUS_STARTING://0启动中
//case CHARGE_STATUS_GUN_STANDBY://1待机-枪已连接，等待充电
//case CHARGE_STATUS_LIMIT://2充电-限制
//case CHARGE_STATUS_PAUSH://3充电-暂停
//case CHARGE_STATUS_CHARGING://4充电-充电中
//case CHARGE_STATUS_SWITCH://5待机-切换中
//case CHARGE_STATUS_FREE://6待机-空闲
//case CHARGE_STATUS_DISCONNECT://7离线-未通信
//case CHARGE_STATUS_FINISH://8待机-已完成
//case CHARGE_STATUS_FAULT://9故障
//case CHARGE_STATUS_DISCHARGING://10放电
//case CHARGE_STATUS_FULL=13://13待机-车已充满
//case CHARGE_STATUS_FINISH1://14待机-手动断开
//case CHARGE_STATUS_QUEUE1://15充电-排队1
//case CHARGE_STATUS_QUEUE2://16充电-排队2
//case CHARGE_STATUS_QUEUE3://17充电-排队3
//case CHARGE_STATUS_QUEUE4://18充电-排队3
//case CHARGE_STATUS_QUEUE5://19充电-排队5
//case CHARGE_STATUS_QUEUE6://20充电-排队6
//case CHARGE_STATUS_QUEUE7://21充电-排队7

//逻辑工作状态(三大类:A属于充电业务 B非充电业务 C为止 )
typedef enum __CHARGE_STATUS{
    CHARGE_STATUS_STARTING = 0,//0启动中 A
    CHARGE_STATUS_GUN_STANDBY,//1待机-枪已连接，等待充电 B
    CHARGE_STATUS_LIMIT,//2充电-限制 A
    CHARGE_STATUS_PAUSH,//3充电-暂停 A
    CHARGE_STATUS_CHARGING,//4充电-充电中 A
    CHARGE_STATUS_SWITCH,//5待机-切换中 B
    CHARGE_STATUS_FREE,//6待机-空闲 B
    CHARGE_STATUS_DISCONNECT,//7离线-未通信 C
    CHARGE_STATUS_FINISH,//8待机-已完成 A
    CHARGE_STATUS_FAULT,//9故障 B
    CHARGE_STATUS_DISCHARGING,//10放电 A
    CHARGE_STATUS_SLAVEGUN,//11副枪 A
    CHARGE_STATUS_COUPLE_ERR,//12配对错误
    CHARGE_STATUS_FULL=13,//13待机-车已充满 A
    CHARGE_STATUS_FINISH1,//14待机-手动断开 B
    CHARGE_STATUS_QUEUE1,//15充电-排队1 B
    CHARGE_STATUS_QUEUE2,//16充电-排队2 B
    CHARGE_STATUS_QUEUE3,//17充电-排队3 B
    CHARGE_STATUS_QUEUE4,//18充电-排队3 B
    CHARGE_STATUS_QUEUE5,//19充电-排队5 B
    CHARGE_STATUS_QUEUE6,//20充电-排队6 B
    CHARGE_STATUS_QUEUE7,//21充电-排队7  B
    CHARGE_STATUS_WAITING=25,   //25等待中
    CHARGE_STATUS_CARPAUSH=30,//30暂停状态细分--车辆暂停
    CHARGE_STATUS_DEVPAUSH,//31暂停状态细分--充电设备暂停
    CHARGE_STATUS_CSCUPAUSH,//32暂停状态细分--集控暂停
    CHARGE_STATUS_SCHEDUING//33调度中

}CHARGE_STATUS;

//编码备用
//case CHARGE_CMD_TYPE_START_CHARGE_NOW://01H 立即充电
//case CHARGE_CMD_TYPE_START_CHARGE_ECONOMIC:// 02H 经济充电
//case CHARGE_CMD_TYPE_STOP_CHARGE://03H 终止充电
//case CHARGE_CMD_TYPE_STOP_CHARGE_URGENCY://04H 紧急停止充电
//case CHARGE_CMD_TYPE_LIMIT_CHARGE://05H 限制充电,对充电中的设备进行限功率
//case CHARGE_CMD_TYPE_PAUSH_CHARGE://06H 暂停充电
//case CHARGE_CMD_TYPE_ADJUST_CHARGE://07H 调整充电电流
//case CHARGE_CMD_TYPE_RESUME://08H 恢复充电，让暂停充电的模块继续充电
//case CHARGE_CMD_TYPE_RESET://09H 复位，让限制充电的模块取消限制
//case CHARGE_CMD_TYPE_START_DISCHARGE://0AH 开始放电
//case CHARGE_CMD_TYPE_STOP_DISCHARGE://0BH 结束放电

//充电指令类型枚举
typedef enum __CHARGE_CMD_TYPE
{
    CHARGE_CMD_TYPE_NONE = 0,    //无 hd
    CHARGE_CMD_TYPE_START_CHARGE_NOW = 1,//01H 立即充电
    CHARGE_CMD_TYPE_START_CHARGE_ECONOMIC,// 02H 经济充电
    CHARGE_CMD_TYPE_STOP_CHARGE,//03H 终止充电
    CHARGE_CMD_TYPE_STOP_CHARGE_URGENCY,//04H 紧急停止充电
    CHARGE_CMD_TYPE_LIMIT_CHARGE,//05H 限制充电,对充电中的设备进行限功率
    CHARGE_CMD_TYPE_PAUSH_CHARGE,//06H 暂停充电
    CHARGE_CMD_TYPE_ADJUST_CHARGE,//07H 调整充电电流
    CHARGE_CMD_TYPE_RESUME,//08H 恢复充电，让暂停充电的模块继续充电
    CHARGE_CMD_TYPE_RESET,//09H 复位，让限制充电的模块取消限制
    CHARGE_CMD_TYPE_START_DISCHARGE,//0AH 开始放电
    CHARGE_CMD_TYPE_STOP_DISCHARGE,//0BH 结束放电
//    CHARGE_CMD_TYPE_CAR_PAUSH_CHARGE,//0CH 暂停充电--车辆
//    CHARGE_CMD_TYPE_DEVPAUSH_CHARGE,//0DH 结束放电--充电设备
    CHARGE_CMD_TYPE_ADJUST_POWER   //能效临时用调整功率
}CHARGE_CMD_TYPE;

//响应结果
typedef enum __CMD_ACK_TYPE
{
    CMD_ACK_TYPE_FAIL = 1,//终端响应失败//CAN
    CMD_ACK_TYPE_ALREADY_CHARGING,//终端已经在充电
    CMD_ACK_TYPE_NOT_CHARGING,//终端没有在充电
    CMD_ACK_TYPE_NO_MODULE,//当前没有模块可用
    CMD_ACK_TYPE_REJECT  = 18,   //18,终端拒绝访问//CAN
    CMD_ACK_TYPE_CANNOT_ACK, //19,终端无法响应//CAN
    CMD_ACK_TYPE_ALREADY_EXE ,//20,终端已经有指令执行中
    CMD_ACK_TYPE_ALREADY_STARTING ,//21,终端处于启动中，无需重复启动
    CMD_ACK_TYPE_GUN_NOLINK,//22,终端没有插抢
    CMD_ACK_TYPE_DEV_OFFLINE,//23,终端离线
    CMD_ACK_TYPE_DEV_FAULT,//24,终端故障
    CMD_ACK_TYPE_NOT_PAUSHING,//25,终端没有处于暂停中
    CMD_ACK_TYPE_NOT_LIMITING,//26,终端没有处于限制中
    CMD_ACK_TYPE_NOT_DISCHARGING,//27,终端没有处于放电中
    CMD_ACK_TYPE_TIMEOUT,//28,终端响应超时
    CMD_ACK_TYPE_NOW_LIMIT,//29,终端处于限制中
    CMD_ACK_TYPE_NOW_PAUSH,//30,终端处于暂停
    CMD_ACK_TYPE_NOW_DISCHARGING,//31,终端处于放电中
    CMD_ACK_TYPE_NO_TERMINAL,//32,没有该终端
    CMD_ACK_TYPE_IERR,//33,内部错误
    CMD_ACK_TYPE_CANID_ERRO,//34,CAN地址错误

    //以下为响应结果执行结果合并做依据---2017-01-20
    CMD_ACK_TYPE_WAIT_START_CHARGE_TIMEOUT,//80,等待启动充电超时
    CMD_ACK_TYPE_WAIT_START_DISCHARGE_TIMEOUT,//81,等待启动放电超时

    CMD_ACK_TYPE_WAIT_START_CHARGE_FAIL,//82,启动充电失败
    CMD_ACK_TYPE_WAIT_START_DISCHARGE_FAIL,//83,启动放电失败

    CMD_ACK_TYPE_WAIT_STOP_CHARGE_TIMEOUT,//84,等待结束充电超时
    CMD_ACK_TYPE_WAIT_STOP_DISCHARGE_TIMEOUT,//85,等待结束放电超时

    CMD_ACK_TYPE_WAIT_STOP_CHARGE_FAIL,//86,结束充电失败
    CMD_ACK_TYPE_WAIT_STOP_DISCHARGE_FAIL,//87,结束充电失败

    //NACK启动失败结果码
    CMD_ACK_TYPE_PDUFault = 50,  //充电机故障无法完成充电05
    CMD_ACK_TYPE_NoPowerAccept,//当前无模块可用，无法完成充电3B
    CMD_ACK_TYPE_Scram,//急停告警，无法完成充电 39
    CMD_ACK_TYPE_LinkBreak,//连接器断开，无法完成充电 07
    CMD_ACK_TYPE_LastProtectNoEnd,//前一次充电因主动防护终止未拔枪，无法完成充电 --
    CMD_ACK_TYPE_LaskBMSOutTime,//前一次充电BMS通信超时终止，无法完成充电 36
    CMD_ACK_TYPE_ChargerInUsed,//充电机正在配置，充电或升级，无法完成充电 40

    //最终结果
    CMD_ACK_TYPE_SUCCESS = 0XFF//终端响应成功//CAN
}CMD_ACK_TYPE;



//执行结果
typedef enum __CMD_ACK_EXE_TYPE
{
    CMD_ACK_EXE_TYPE_FAIL = 1,//执行失败
    CMD_ACK_EXE_TYPE_START_CHARGE_FAIL,//启动失败
    CMD_ACK_EXE_TYPE_START_CHARGE_TIMEOUT,//启动超时
    CMD_ACK_EXE_TYPE_STOP_CHARGE_FAIL,//停止失败
    CMD_ACK_EXE_TYPE_STOP_CHARGE_TIMEOUT,//停止超时
    CMD_ACK_EXE_TYPE_START_DISCHARGE_FAIL,//启动放电失败
    CMD_ACK_EXE_TYPE_START_DISCHARGE_TIMEOUT,//启动放电超时
    CMD_ACK_EXE_TYPE_STOP_DISCHARGE_FAIL,//停止放电失败
    CMD_ACK_EXE_TYPE_STOP_DISCHARGE_TIMEOUT,//停止放电超时
    CMD_ACK_EXE_TYPE_NOT_CHARGING,//切换失败
//    CMD_ACK_EXE_TYPE_PAUSE_CHARGING_FAILE,//暂停充电失败
    CMD_ACK_EXE_TYPE_SUCCESS = 0XFF//执行成功
}CMD_ACK_EXE_TYPE;


//编程备用
//case CHARGE_STEP_NORMAL://常规状态
//case CHARGE_STEP_WAITCMD_ACK://收到服务器指令后通过CAN总线发送CMD，等待ACK回复,超时时间5S
//case CHARGE_STEP_WAITDEV_START_CHARGE://等待启动充电，超时时间60S
//case CHARGE_STEP_WAITDEV_STOP_CHARGE: //等待结束充电，超时时间60S
//case CHARGE_STEP_WAITDEV_LIMIT_CHARGE://等待限制充电成功，超时时间60S
//case CHARGE_STEP_WAITDEV_PAUSH_CHARGE://等待暂停充电成功，超时时间60s
//case CHARGE_STEP_WAITDEV_RESUME_CHARGE://等待恢复充电成功，超时时间60S，让暂停充电的模块继续充电
//case CHARGE_STEP_WAITDEV_RESET_CHARGE://等待复位充电，超时时间60S，让限制充电的模块取消限制
//case CHARGE_STEP_WAITDEV_START_DISCHARGE://等待启动放电，超时时间60S
//case CHARGE_STEP_WAITDEV_STOP_DISCHARGE://等待结束放电，超时时间60S
//case CHARGE_STEP_WAITSEVER_ACK_APPLY_START_CHARGE_VIN://等待服务器返回VIN申请充电结果，超时时间15S
//case CHARGE_STEP_WAITSEVER_ACK_APPLY_STOP_CHARGE_VIN://等待服务器返回VIN申请充电结果，超时时间15S
//case CHARGE_STEP_WAITSEVER_ACK_APPLY_START_CHARGE_CARD://等待服务器返回刷卡申请充电结果，超时时间15S
//case CHARGE_STEP_WAITSEVER_ACK_APPLY_STOP_CHARGE_CARD://等待服务器返回刷卡申请结束充电结果，超时时间15S
//case CHARGE_STEP_WAITDEV_STOP_REASON://等待中止原因，超时时间2S,直流


//设备充电业务控制状态机
typedef enum __CHARGE_STEP_TYPE{
    CHARGE_STEP_NORMAL,//常规状态
    CHARGE_STEP_WAITCMD_ACK = 1,//1.收到服务器指令后通过CAN总线发送CMD，等待ACK回复,超时时间5S
    CHARGE_STEP_WAITDEV_START_CHARGE,//等待启动充电，超时时间60S---80S  hd 2018-8-21
    CHARGE_STEP_WAITDEV_STOP_CHARGE,//等待结束充电，超时时间60S
    CHARGE_STEP_WAITDEV_LIMIT_CHARGE,//等待限制充电成功，超时时间60S
    CHARGE_STEP_WAITDEV_PAUSH_CHARGE,//等待暂停充电成功，超时时间60s
    CHARGE_STEP_WAITDEV_RESUME_CHARGE,//等待恢复充电成功，超时时间60S，让暂停充电的模块继续充电
    CHARGE_STEP_WAITDEV_RESET_CHARGE,//等待复位充电，超时时间60S，让限制充电的模块取消限制
    CHARGE_STEP_WAITDEV_START_DISCHARGE,//等待启动放电，超时时间60S
    CHARGE_STEP_WAITDEV_STOP_DISCHARGE,//等待结束放电，超时时间60S
    CHARGE_STEP_WAITSEVER_ACK_APPLY_START_CHARGE_VIN,//等待服务器返回VIN申请充电结果，超时时间15S
    CHARGE_STEP_WAITSEVER_ACK_APPLY_STOP_CHARGE_VIN,//等待服务器返回VIN申请充电结果，超时时间15S
    CHARGE_STEP_WAITSEVER_ACK_APPLY_START_CHARGE_CARD,//等待服务器返回刷卡申请充电结果，超时时间15S
    CHARGE_STEP_WAITSEVER_ACK_APPLY_STOP_CHARGE_CARD,//等待服务器返回刷卡申请结束充电结果，超时时间15S
    CHARGE_STEP_WAITDEV_STOP_REASON,//等待中止原因，超时时间2S,直流
    CHARGE_STEP_WAITPEAK_START//等待削峰填谷开始充电返回5S hd
}CHARGE_STEP_TYPE;

typedef enum __CHARGE_TYPE{
    CHARGE_TYPE_IMMEDIATELY = 1,//立即充电
    CHARGE_TYPE_ECONOMICAL,//2经济充电
    CHARGE_TYPE_STOP,//3终止充电
    CHARGE_TYPE_EMERGENCY_STOP,//4紧急停止充电
    CHARGE_TYPE_LIMIT,//5限制充电
    CHARGE_TYPE_PAUSE,//6暂停充电
    CHARGE_TYPE_ADJUST_CURRENT,//7调整充电电流
    CHARGE_TYPE_RECOVER,//8恢复充电
    CHARGE_TYPE_RESET,//9复位
    CHARGE_TYPE_DISCHARGE,//10开始放电
    CHARGE_TYPE_STOP_DISCHARGE,//11结束放电
    CHARGE_TYPE_CAN_CPU,//12 CAN设备CPU卡充电
    CHARGE_TYPE_CAN_ID,//13
    CHARGE_TYPE_CAN_VIN,//14
    CHARGE_TYPE_ZIGBEE_CPU,//15 ZIGBEE设备CPU卡充电
    CHARGE_TYPE_ZIGBEE_ID,//16
    CHARGE_TYPE_ZIGBEE_VIN,//17
    CHARGE_TYPE_MANUAL,//18
    CHARGE_TYPE_CAR_LISENCE,//19 车牌号申请充电 add by njs 2016-07-21
    CHARGE_TYPE_QUEUE//轮充
}CHARGE_TYPE;

//add by YCZ 2016-08-16
//设备类型
typedef enum __DEV_TYPE{
    DEV_TYPE_AC = 1, //1~110 共110个
    DEV_TYPE_BRANCH_BOX,//111~123 共13个
    DEV_TYPE_3AC,//151~180 共30个
    DEV_TYPE_DC,//181~230 共50个
    DEV_TYPE_CCU,//231~240 共10个
    DEV_TYPE_CENTER//241~253 共13个
}DEV_TYPE;

//描述订单状态，同时也表示订单的优先级搞定
//值越大表示优先级越高
typedef enum __ORDER_STATUS{
    ORDER_STATUS_NON,	//无
    ORDER_STATUS_FAIL,	//结算完成-失败
    ORDER_STATUS_OK,	//结算完成-成功
    ORDER_STATUS_QUEUE,	//订单排队中
    ORDER_STATUS_ING	//进行中-待结算
}ORDER_STATUS;

//订单类型
typedef enum __ORDER_TYPE{
    ORDER_TYPE_NONE = 0,
    ORDER_NORMAL,		//正常订单
    ORDER_EMERGENCY,	//应急订单
    ORDER_ECNOMIC		//经济充电订单
}ORDER_TYPE;

//充电方式
typedef enum __CHARGE_WAY{
    CHARGE_WAY_REMOTE,//平台启动
    CHARGE_WAY_CARD,//刷卡启动
    CHARGE_WAY_VIN,//VIN启动
    CHARGE_WAY_CARNO//车牌号启动
}CHARGE_WAY;

typedef struct _InfoTAG
{
    int i;//信息体地址
    unsigned char sData[8];  //信息体数据
} InfoTAG;

/*进线侧电表数据定义*/
typedef struct __AmmeterData
{
    unsigned char addr[6];
    float Vol_A;
    float Vol_B;
    float Vol_C;
    float Cur_A;
    float Cur_B;
    float Cur_C;
    float TotalPower;
    float TotalRePower;
    float PowerFactor;
    float Cur_0;
    float VolUnbalance;
    float CurUnbalance;
    float HarmDistortion;
    float Power_A;
    float Power_B;
    float Power_C;
    float RePower_A;
    float RePower_B;
    float RePower_C;
    float PowerFactor_A;
    float PowerFactor_B;
    float PowerFactor_C;
    uint ActiveAbsorbEnergy; //正向有功电能
    uint ActiveLiberateEnergy;//反向有功电能
    uint ReactiveSensibilityEnergy;//感性无功电能
    uint ReactiveCapacityEnergy;//容性无功电能
}stAmmeterData;
Q_DECLARE_METATYPE(stAmmeterData)



/*远程抄表电表数据定义-电表电能数据类型1*/
typedef struct __RemoteAmmeterType1Data
{
    unsigned char addr[6];
    float current_active_absorb_energy;                            //(当前)正向有功总电能
    float current_active_absorb_rate1_energy;                 //(当前)正向有功费率1电能
    float current_active_absorb_rate2_energy;                 //(当前)正向有功费率2电能
    float current_active_absorb_rate3_energy;                 //(当前)正向有功费率3电能
    float current_active_absorb_rate4_energy;                 //(当前)正向有功费率4电能
    float current_active_liberate_energy;                          //(当前)反向有功总电能
    float current_reactive_absorb_energy;                       //(当前)正向无功总电能
    float current_reactive_liberate_energy;                       //(当前)反向无功总电能
    float current_reactive_absorb_energy1;						//(当前)正向无功总电能-（当前）第一象限无功总电能
    float current_reactive_absorb_energy2;						//(当前)正向无功总电能-（当前）第二象限无功总电能
    float current_reactive_liberate_energy1;						//(当前)反向无功总电能-（当前）第三象限无功总电能
    float current_reactive_liberate_energy2;						//(当前)反向无功总电能-（当前）第四象限无功总电能
    float current_active_absorb_max_demand;                 //(当前)正向有功总最大需量
    char current_active_absorb_max_demand_time[5];	 //(当前)正向有功总最大需量发生时间
    float current_active_liberate_max_demand;                //(当前)反向有功总最大需量
    char current_active_liberate_max_demand_time[5];	 //(当前)反向有功总最大需量发生时间
}stRemoteAmmeterType1Data;
Q_DECLARE_METATYPE(stRemoteAmmeterType1Data)

/*远程抄表电表数据定义-电表电能数据类型2*/
typedef struct __RemoteAmmeterType2Data
{
    unsigned char addr[6];
    char hour_freeze_time[5];                                              //（上 1 次）整点冻结时间
    float hour_freeze_active_absorb_energy;                    //（上 1 次）整点冻结正向有功总电能
    float hour_freeze_active_liberate_energy;                //（上 1 次）整点冻结反向有功总电能
}stRemoteAmmeterType2Data;
Q_DECLARE_METATYPE(stRemoteAmmeterType2Data)

/*远程抄表电表数据定义-电表电能数据类型3*/
typedef struct __RemoteAmmeterType3Data
{
    unsigned char addr[6];
    char day_freeze_time[5];                                              //(上1次)日冻结时间
    float day_freeze_active_absorb_energy;                    //（上1次）日冻结正向有功电能数据
    float day_freeze_active_absorb_rate1_energy;
    float day_freeze_active_absorb_rate2_energy;
    float day_freeze_active_absorb_rate3_energy;
    float day_freeze_active_absorb_rate4_energy;
    float day_freeze_active_liberate_energy;                //（上1次）日冻结反向有功电能数据
    float day_freeze_reactive_absorb_energy;               //（上1次）日冻结正向无功电能数据
    float day_freeze_reactive_liberate_energy;              //（上1次）日冻结反向无功电能数据
    float day_freeze_reactive_absorb_energy1;              //（上1次）日冻结正向无功电能数据-（当前）第一象限无功总电能
    float day_freeze_reactive_absorb_energy2;              //（上1次）日冻结正向无功电能数据-（当前）第二象限无功总电能
    float day_freeze_reactive_liberate_energy1;             //（上1次）日冻结反向无功电能数据-（当前）第三象限无功总电能
    float day_freeze_reactive_liberate_energy2;             //（上1次）日冻结反向无功电能数据-（当前）第四象限无功总电能
    float day_freeze_active_absorb_max_demand;                  //（上1次）日冻结正向有功最大需量
    char day_freeze_active_absorb_max_demand_time[5];    //（上1次）日冻结正向有功最大需量发生时间数据
    float day_freeze_active_liberate_max_demand;                //（上1次）日冻结反向有功最大需量
    char day_freeze_active_liberate_max_demand_time[5];  //（上1次）日冻结反向有功最大需量及发生时间数据
}stRemoteAmmeterType3Data;
Q_DECLARE_METATYPE(stRemoteAmmeterType3Data)

/*远程抄表电表数据定义-电表电能数据类型4*/
typedef struct __RemoteAmmeterType4Data
{
    unsigned char addr[6];
    float settlement_active_absorb_energy;                            //(上1结算日)正向有功总电能
    float settlement_active_absorb_rate1_energy;                 //(上1结算日)正向有功费率1电能
    float settlement_active_absorb_rate2_energy;                 //(上1结算日)正向有功费率2电能
    float settlement_active_absorb_rate3_energy;                 //(上1结算日)正向有功费率3电能
    float settlement_active_absorb_rate4_energy;                 //(上1结算日)正向有功费率4电能
    float settlement_active_liberate_energy;                          //(上1结算日)反向有功总电能
    float settlement_reactive_absorb_energy;                       //(当前)正向无功总电能
    float settlement_reactive_liberate_energy;                       //(当前)正向无功总电能
    float settlement_reactive_absorb_energy1;						//(上1结算日)正向无功总电能-（上1结算日）第一象限无功总电能
    float settlement_reactive_absorb_energy2;						//(上1结算日)正向无功总电能-（上1结算日）第二象限无功总电能
    float settlement_reactive_liberate_energy1;						//(上1结算日)反向无功总电能-（上1结算日）第三象限无功总电能
    float settlement_reactive_liberate_energy2;						//(上1结算日)反向无功总电能-（上1结算日）第四象限无功总电能
    float settlement_active_absorb_max_demand;                 //(上1结算日)正向有功总最大需量
    char settlement_active_absorb_max_demand_time[5];	 //(上1结算日)正向有功总最大需量发生时间
    float settlement_active_liberate_max_demand;                //(上1结算日)反向有功总最大需量
    char settlement_active_liberate_max_demand_time[5];	 //(上1结算日)反向有功总最大需量发生时间
}stRemoteAmmeterType4Data;
Q_DECLARE_METATYPE(stRemoteAmmeterType4Data)

typedef struct __InLineAmmeterData
{
    QMap<QByteArray, stAmmeterData> ammeterData;
}InLineAmmeterData;

typedef struct __RemoteAmmeterType1
{
    QMap<QByteArray, stRemoteAmmeterType1Data> remoteAmmeterType1Data;
}RemoteAmmeterType1;

typedef struct __RemoteAmmeterType2
{
    QMap<QByteArray, stRemoteAmmeterType2Data> remoteAmmeterType2Data;
}RemoteAmmeterType2;

typedef struct __RemoteAmmeterType3
{
    QMap<QByteArray, stRemoteAmmeterType3Data> remoteAmmeterType3Data;
}RemoteAmmeterType3;

typedef struct __RemoteAmmeterType4
{
    QMap<QByteArray, stRemoteAmmeterType4Data> remoteAmmeterType4Data;
}RemoteAmmeterType4;

/*实时状态数据*/
typedef struct __RealStatusData
{
    bool connectStatus;				//网络连接状态，true  已连接
    bool emergencyStatus;			//应急充电状态 true：应急状态 false：正常状态
    bool linkStatus;				//通信链路连接状态。3.0协议增加
    unsigned short alarm1;			//告警数据
    unsigned short alarm2;
    short temperature;				//温度
    short humidity;					//湿度
}RealStatusData;
Q_DECLARE_METATYPE(RealStatusData)


/*ccu实时数据*/
typedef struct __CCURealData
{
    int ccutotalruntime;
    short ccuenvtemp;			//ccu环境温度
    float cabratedpower;		//直流柜额定输出功率
    float cabnowpower;			//直流柜当前输出功率
}CCURealData;
Q_DECLARE_METATYPE(CCURealData)

typedef struct __CCUAllRealData
{
    QMap<unsigned char, CCURealData> ccuData;
}CCUAllRealData;
Q_DECLARE_METATYPE(CCUAllRealData)


/*cscu实时状态数据、测量数据*/
typedef struct __RealStatusMeterData
{
    RealStatusData realData;
    InLineAmmeterData inLineAmmeter;
    RemoteAmmeterType1 remoteAmmeterType1;
    RemoteAmmeterType2 remoteAmmeterType2;
    RemoteAmmeterType3 remoteAmmeterType3;
    RemoteAmmeterType4 remoteAmmeterType4;
    CCUAllRealData ccuAllData;

}RealStatusMeterData;

#define LENGTH_TERMINAL_VER 16 //string
#define LENGTH_EVENT_NO    9   //bcd hex
#define LENGTH_CARDNO     17   //bcd类型的字符串
#define LENGTH_USER_NO    18   //保留
#define LENGTH_GUID_NO    16  //hex
#define LENGTH_VIN_NO   17       //string
#define LENGTH_CAR_LISENCE 10  	//宇通定制
#define LENGTH_BILL_CODE	40  //string

#define		ALL_VALID_NUMBER	0x0000000f


/*ccu数据项*/
typedef struct __CCUDatasItem
{
    char runStatus;					//直流柜运行状态
    char inRelayStatus;				//输入接触器状态
    char linkageRelayStatus;		//联动接触器状态
    char sysType;					//系统机型
    char warning_status;			//告警状态

    unsigned int ccutotalruntime;
	short ccuenvtemp;				//ccu环境温度
	float cabratedpower;			//直流柜额定输出功率
	float cabnowpower;				//直流柜当前输出功率
    short JiGuihumidty;            //机柜湿度 smm add
}stCCUDatasItem;
Q_DECLARE_METATYPE(stCCUDatasItem)

/*ccu设备规格信息*/
typedef struct __CCUADviceSpecificationsMap
{
    char ModuleID;					//模块ID号
    char DeviceSpecificationsFlag;				//设备规格信息传输完成标志(CAN)
    char SlotNum;		//槽位号
    QByteArray SerialNumber;//模块序列号
    QByteArray SoftwareVer; //软件版1
    QByteArray SoftwareVer1;//软件版2
    QByteArray SoftwareVer2;//软件版3
    QByteArray HardwareVer;//硬件版本

}stCCUADviceSpecificationsMap;
Q_DECLARE_METATYPE(stCCUADviceSpecificationsMap)

typedef struct __CCUADviceSpecificationsMapSingle
{
    QMap<QByteArray, stCCUADviceSpecificationsMap> mapSingle;
}stCCUADviceSpecificationsSingle;
Q_DECLARE_METATYPE(stCCUADviceSpecificationsSingle)

/*pdu数据项*/
typedef struct __PDUDatasItem
{
    char runStatus;					//PDU运行状态
    char warning_status;			//告警状态
    char greenLight;
    char yellowLight;
    char redLight;
    float outVolatge;				//输出电压
    float outCurrent;				//输出电流
    short stationTemp;				//环境温度
    short coolingTemp;				//散热温度
    unsigned int totalRuntime;				//总运行时间
    unsigned int switchNum;					//切换次数
    float energy;					//电能表读数
    unsigned short resistanceMinus; //负对地阻值
    unsigned short resistancePlus; //正对地阻值
    float setVoltage;  //PDU设模块输出电压
    float setCurrent;  //PDU设模块限流点
}stPDUDatasItem;
Q_DECLARE_METATYPE(stPDUDatasItem)

typedef struct __PDUDatasSingle
{
    QMap<QByteArray, stPDUDatasItem> mapSingle;
}stPDUDatasSingle;
Q_DECLARE_METATYPE(stPDUDatasSingle)

//能效系统柜子信息
typedef struct __CabinetData
{
    QMap<QByteArray, QByteArray> devData;
}stCabinetData;
Q_DECLARE_METATYPE(stCabinetData)


/*分支箱数据项*/
typedef struct __BranchDatasItem
{
    char runStatus;					//PDU运行状态

    float outVolatge;				//输出电压
    float outCurrent;				//输出电流
    short stationTemp;				//环境温度
    short m1Temp;					//m1板温度
    unsigned int totalRuntime;				//总运行时间
    unsigned int switchNum;					//切换次数
    //	float energy;					//电能表读数
}stBranchDatasItem;
Q_DECLARE_METATYPE(stBranchDatasItem)

typedef struct __BranchDatasSingle
{
    QMap<QByteArray, stBranchDatasItem> mapSingle;
}stBranchDatasSingle;
Q_DECLARE_METATYPE(stBranchDatasSingle)


/*直流模块数据项*/
typedef struct __DCModuleDatasItem
{
    char runStatus;				//模块运行状态
    char group;					//所属分组
    char warning_status;			//告警状态, 1:故障,0正常

    float inVoloatgeA;			//输入A相电压
    float inVoloatgeB;			//输入B相电压
    float inVoloatgeC;			//输入C相电压
    float inCurrent;			//输入电流
    float outVolatge;			//输出电压
    float outCurrent;			//输出电流
    short stationTemp;			//环境温度
    short m1Temp;				//M1板温度
    unsigned int totalRuntime;			//总运行时间
    unsigned int switchNum;				//切换次数
}stDCModuleDatasItem;
Q_DECLARE_METATYPE(stDCModuleDatasItem)

typedef struct __DCModuleDatasSingle
{
    QMap<QByteArray, stDCModuleDatasItem> mapSingle;
}stDCModuleDatasSingle;
Q_DECLARE_METATYPE(stDCModuleDatasSingle)

//能效计划数据结构定义 add by XX 2017-08-15
//储能柜
typedef struct __EnergyStorageCabinetInfo
{
    mutable int onlineCounter       ;   //在线计数器
    unsigned char mainBreaker		;//1闭合 0断开
    unsigned char slaveBreaker		;//1闭合 0断开
    unsigned char DCBreaker1		;//1闭合 0断开
    unsigned char DCBreaker2		;//1闭合 0断开
    unsigned char fireExtinguisher_1		;//1正常 0异常
    unsigned char DCBreaker3		;//1闭合 0断开
    unsigned char DCBreaker4		;//1闭合 0断开
    unsigned char reserve	;//1暂停/2限制
    unsigned char tripFeedback;
    unsigned char fireExtinguisher_2		;//1正常 0异常
    unsigned char waterIn		;//1正常 0异常

}stEnergyStorageCabinetInfo;
Q_DECLARE_METATYPE(stEnergyStorageCabinetInfo)

//储能电池
typedef struct __EnergyStorageBatteryInfo
{
    mutable int onlineCounter       ;   //在线计数器
    struct
    {
        unsigned char CRC;//
        unsigned char bmsHeartBeat	;//0待机状态/1充电状态/2故障状态/3启动中
        unsigned char test		;//0断开/1闭合
        unsigned char tankSwitch;
        unsigned char singleOverVolAlarm    ;
        unsigned char singleLowVolAlarm  ;
        unsigned char OverTempAlarm  ;
        unsigned char BelowTempAlarm  ;
        unsigned char insulationAlarm;
        unsigned char BMScommuFault  ;
        unsigned char BMScontrolPower	;
        unsigned char BMSfullPowerON	;
        unsigned char BMSsysStatus	;
        unsigned char ESSfullEnergy	;
        unsigned char ESSfullDisCharge	;
        unsigned char ApplyACInfo;
        unsigned char ApplySysInfo;
        float SOC;
    }B2C_STATUS;
    struct
    {
        unsigned char CRC;//
        unsigned char tankNum;
        float BMShighVol;
        float BMScur;
    }B2C_SUMDATA1;
    struct
    {
        unsigned char CRC;//
        unsigned char reserve;
        float BMSchargeEnergy;
        float BMSdisChargeEnergy;
        unsigned char  SOH;
        unsigned char  reverse;
    }B2C_SUMDATA2;
    struct
    {
        unsigned char sysHumidity;//
        float singleMaxVol;
        float singleMinVol;
        float singleMaxTem;
        float singleMinTem;
        float  sysTemp;
    }B2C_SUMDATA3;
    struct
    {
        float BMSlimitDischargeCur;//
        float BMSlimitChargeCur;
        float BMSlimitChargeVol;
        float BMSlimitDisChargeVol;
    }B2C_LIMIT;
    float outVol;
    float fuseVol;
    float breakerVol;
    float cur;
    float dcVol;
    float dcCur;
    float dcPower;
    float dcPositiveEnergy;
    float dcDisPositiveEnergy;
    unsigned short dcPT;
    unsigned short dcCT;
}stEnergyStorageBatteryInfo;
Q_DECLARE_METATYPE(stEnergyStorageBatteryInfo)

//光伏柜
typedef struct __PhotoVoltaicCabinetInfo
{
    mutable int onlineCounter       ;   //在线计数器
    unsigned char breaker1	;//0断开1闭合
    unsigned char breaker2	;//0断开1闭合
    unsigned char breaker3	;//0断开1闭合
    unsigned char breaker4	;//0断开1闭合
    unsigned char breaker5	;//0断开1闭合
    unsigned char breaker6	;//0断开1闭合
    unsigned char breaker7	;//0断开1闭合
    unsigned char smokeSensor	;//0异常/1正常
}stPhotoVoltaicCabinetInfo;
Q_DECLARE_METATYPE(stPhotoVoltaicCabinetInfo)

//系统控制柜
typedef struct __SystemControlCabinetInfo
{
    mutable int onlineCounter       ;   //在线计数器
    unsigned char lowVolTravelSwitch	;//0关门/  １开门
    unsigned char highVolTravelSwitch	;//0关门/  １开门
    unsigned char dormTravelSwitch	;//0关门/  １开门
    unsigned char lowVolSmokeSensor	;//0关门/  １开门
    unsigned char transformerSmokeSensor ;//0关门/  １开门
    unsigned char highVolSmokeSensor	;//0关门/  １开门
    unsigned char outEmergencyStop     ;//0关门/  １开门
    unsigned char centerContorlEmergencyStop   ;//0关门/  １开门总控柜面板急停
    unsigned char transformerOverTemp	;//0关门/  １开门
    unsigned char transformerTempControlerFault	;//0关门/  １开门
    unsigned char waterIn  ;

}stSystemControlCabinetInfo;
Q_DECLARE_METATYPE(stSystemControlCabinetInfo)

//四象限柜
typedef struct __FourQuadrantCabinetInfo
{
    mutable int onlineCounter       ;   //在线计数器
//    unsigned char ACbreaker1	;//0断开/1吸合
    unsigned char ACbreaker2	;//0断开/1吸合
    unsigned char ACbreaker3	;//0断开/1吸合
//    unsigned char DCbreaker1	;//0断开/1吸合
    unsigned char DCbreaker2	;//0断开/1吸合
    unsigned char DCbreaker3	;//0断开/1吸合
    unsigned char surgeFeedback	;//浪涌反馈0异常/1正常
    unsigned char fireExtinguisher2	;//灭火器 0异常/1正常
    unsigned char fireExtinguisher3	;//灭火器 0异常/1正常

}stFourQuadrantCabinetInfo;
Q_DECLARE_METATYPE(stFourQuadrantCabinetInfo)

//充放电柜
typedef struct __ChargeDischargeCabinetInfo
{
    mutable int onlineCounter       ;   //在线计数器
    unsigned char DCDC1breaker	;//0断开１闭合
    unsigned char DCDC1fireExtinguisher	;//
    unsigned char DCDC2breaker	;//0断开１闭合
    unsigned char DCDC2fireExtinguisher	;//
    float DCDCsumVol;
    float DCDCsumCur;

}stChargeDischargeCabinetInfo;
Q_DECLARE_METATYPE(stChargeDischargeCabinetInfo)

//总配电柜
typedef struct __TotalDistributionCabinetInfo
{
    mutable int onlineCounter       ;   //在线计数器
    unsigned char sumBreaker	;//0断开/1闭合
    unsigned char loadBreaker1	;//0正常1异常
    unsigned char loadBreaker2	;//0正常1异常
    unsigned char loadBreaker3	;//0正常1异常
    unsigned char loadBreaker4	;//0正常1异常
    unsigned char acBreaker	;//0正常1异常
    unsigned char fireExtinguisher	;//0正常1异常

}stTotalDistributionCabinetInfo;
Q_DECLARE_METATYPE(stTotalDistributionCabinetInfo)

//功率优化器
typedef struct __PowerOptimizerSingle
{
    mutable int onlineCounter       ;   //在线计数器
    short PowerOptimizerID;
    float inVol1;
    float inVol2;
    float inVol3;
    float inVol4;
    float curBranch1;
    float curBranch2;
    float curBranch3;
    float curBranch4;
    float curBranch5;
    float curBranch6;
    float curBranch7;
    float curBranch8;
    float curBranch9;
    float curBranch10;
    float curBranch11;
    float curBranch12;
    float curBranch13;
    float curBranch14;
    float curBranch15;
    float curBranch16;
    float realPower;
    float radiatorTemp;

    struct
    {
        unsigned char bit0		: 1;//
        unsigned char bit1		: 1;//
        unsigned char bit2		: 1;//
        unsigned char bit3		: 1;//
        unsigned char bit4		: 1;//
        unsigned char bit5		: 1;//
        unsigned char bit6		: 1;//
        unsigned char bit7		: 1;//
        unsigned char reserve	;//启停按钮处于STOP
    }fault1;
    struct
    {
        unsigned char bit0		: 1;//汇流箱通信错误告警
        unsigned char bit1		: 1;//汇流箱电流不平衡告警
        unsigned char bit2		: 1;//1号汇流箱电流异常告警
        unsigned char bit3		: 1;//2号汇流箱电流异常告警
        unsigned char bit4		: 1;//3号汇流箱电流异常告警
        unsigned char bit5		: 1;//4号汇流箱电流异常告警
        unsigned char bit6		: 1;//5号汇流箱电流异常告警
        unsigned char bit7		: 1;//6号汇流箱电流异常告警
        unsigned char bit8		: 1;//7号汇流箱电流异常告警
        unsigned char bit9		: 1;//8号汇流箱电流异常告警
        unsigned char bit10		: 1;//
        unsigned char bit11		: 1;//正母线对地绝缘故障
        unsigned char bit12		: 1;//负母线对地绝缘故障
        unsigned char bit13		: 1;//烟雾传感器故障
        unsigned char bit14		: 1;//交流侧防雷器故障
        unsigned char bit15		: 1;//直流侧防雷器故障
    }fault2;

    struct
    {
        unsigned char bit0		: 1;//
        unsigned char bit1		: 1;//
        unsigned char bit2		: 1;//
        unsigned char bit3		: 1;//
        unsigned char bit4		: 1;//
        unsigned char bit5		: 1;//
        unsigned char bit6		: 1;//
        unsigned char bit7		: 1;//
        unsigned char bit8		: 1;//
        unsigned char bit9		: 1;//
        unsigned char bit10		: 1;//
        unsigned char bit11		: 1;//
        unsigned char bit12		: 1;//
        unsigned char bit13		: 1;//
        unsigned char bit14		: 1;//
        unsigned char bit15		: 1;//
    }warning;

    short combinerStatus;
    unsigned int softVer;
//    short softVer_L;
//    short softVer_H;
    struct
    {
        unsigned char bit0		: 1;//
        unsigned char bit1		: 1;//
        unsigned char reserve		: 6;//启停按钮处于STOP
    }sysRequestStatus;

    float inVol5;
    float inVol6;
    float inVol7;
    float inVol8;
    float outVol;
}stPowerOptimizerSingle;
Q_DECLARE_METATYPE(stPowerOptimizerSingle)

typedef struct __PowerOptimizerInfo
{
    QMap<QByteArray, stPowerOptimizerSingle> mapSingle;
}stPowerOptimizerInfo;
Q_DECLARE_METATYPE(stPowerOptimizerInfo)

//温湿度计
typedef struct __HygrothermographSingle
{
    float tempurature;
    float humility;
}stHygrothermographSingle;
Q_DECLARE_METATYPE(stHygrothermographSingle)

typedef struct __HygrothermographInfo
{
    QMap<QByteArray, stHygrothermographSingle> mapSingle;
}stHygrothermographInfo;
Q_DECLARE_METATYPE(stHygrothermographInfo)

//独立逆变柜
typedef struct __SingleInverterCabInfo
{
    mutable int onlineCounter       ;   //在线计数器
    unsigned char ACbreaker	;//0断开/1吸合
    unsigned char DCbreaker	;//0断开/1吸合
    unsigned char surgeFeedback_Cabinet2	;//浪涌反馈0异常/1正常
    unsigned char fireExtinguisher_Cabinet3	;//灭火器0异常/1正常
}stSingleInverterCabInfo;
Q_DECLARE_METATYPE(stSingleInverterCabInfo)

//ACDC模块
typedef struct __ACDCModuleSingle
{
    mutable int onlineCounter       ;   //在线计数器
    float vol_A;
    float cur_A;
    float vol_B;
    float cur_B;
    float vol_C;
    float cur_C;
    float frequency;

    float sysActivePower;//系统有功功率
    float sysReActivePower;//系统无功功率
    float sysApparentPower;//系统视在功率
    float PF;
    float DCpositiveCur;//直流正电流
    float DCnegativeCur;//直流负电流
    float DCpositiveBusBarVol;//直流正母线电压
    float DCnegativeBusBarVol;//直流负母线电压
    float DCbilateralBusBarVol;//直流双边母线电压
    float DCpower;//直流功率
    unsigned short devStatus;//设备状态
    unsigned short warningStatus;//告警状态
    unsigned short faultStatus;//故障状态

    unsigned int HWVersion;
    unsigned int SWVersion;
    //保留28字节？？？？？？？？？？
    float tmp_IGBT1;
    float tmp_IGBT2;
    float tmp_IGBT3;
    float tmp_IGBT4;
    float tmp_IGBT5;
    float tmp_IGBT6;
    float tmp_IN;
    float tmp_OUT;
    //保留8字节
    float inductance1_cur;
    float inductance2_cur;
    float inductance3_cur;
    float inductance4_cur;
    float inductance5_cur;
    float inductance6_cur;
}stACDCModuleSingle;
Q_DECLARE_METATYPE(stACDCModuleSingle)

typedef struct __ACDCModuleInfo
{
    QMap<QByteArray, stACDCModuleSingle> mapSingle;
}stACDCModuleInfo;
Q_DECLARE_METATYPE(stACDCModuleInfo)

//充放电柜DCDC模块信息
typedef struct __DCDC_CDCModuleSingle
{
    mutable int onlineCounter       ;   //在线计数器
    unsigned char boardID;
    unsigned char status	;//模块工作状态

    unsigned char alarm0	;//模块保护(2)
    unsigned char alarm1	;//模块故障(1)
    unsigned char alarm2	;//过温
    unsigned char alarm3	;//输出过压
    unsigned char alarm4	;//温度限功率状态
    unsigned char alarm5	;//交流限功率状态
    unsigned char alarm6	;//模块EEPROM故障
    unsigned char alarm7	;//风扇故障
    unsigned char alarm8	;//模块WALK-In功能使能
    unsigned char alarm9	;//风扇全速
    unsigned char alarm10	;//模块开关机
    unsigned char alarm11	;//模块限功率
    unsigned char alarm12	;//模块CAN错误状态
    unsigned char alarm13	;//模块电流均流告警
    unsigned char alarm14	;//模块识别
    unsigned char alarm15	;//过压脱离继电器动作
    unsigned char alarm16	;//模块交流缺相告警
    unsigned char alarm17	;//模块交流不平衡告警
    unsigned char alarm18	;//模块交流欠压告警
    unsigned char alarm19	;//模块顺序起机功能使能
    unsigned char alarm20	;//模块PFC保护
    unsigned char alarm21	;//模块交流过压
    unsigned char alarm22	;//模块ID重复
    unsigned char alarm23	;//模块严重不均流
    unsigned char alarm24	;//模块输出欠压告警(仅TEC模块)
    unsigned char alarm25	;//模块重载告警
    unsigned char alarm26	;//模块插拔故障(模块放电电路故障)
    unsigned char alarm27	;//模块轻微不均流
    unsigned char alarm28	;//模块PDU分组完成标志
    unsigned char alarm29	;//DC模块地址识别标志
    unsigned char alarm30	;//模块启动完成(模块输出熔丝断告警)
    unsigned char alarm31	;//模块内部通信异常告警

    float outVol;
    float outCur;
    float inVol;
    float inCur;
    float boardTemp_M1;
    float envTemp;
    float runTime;
    float chargeDisChargeTimes;
}stDCDC_CDCModuleSingle;
Q_DECLARE_METATYPE(stDCDC_CDCModuleSingle)

typedef struct __DCDC_CDCModuleInfo
{
    QMap<QByteArray, stDCDC_CDCModuleSingle> mapSingle;
}stDCDC_CDCModuleInfo;
Q_DECLARE_METATYPE(stDCDC_CDCModuleInfo)

//储能柜DCDC模块
typedef struct __EnergyStorageDCDCSingle
{
    mutable int onlineCounter;  //在线计数器
    unsigned short moduleID;

    float cur_in;
    float cur_out;
    float vol_in;
    float vol_out;
    float vol_battery;
    float power_dc;
    struct
    {
        unsigned char warning		: 1;//0断开/1闭合
        unsigned char run		: 1;//0断开/1闭合
        unsigned char fault		: 1;//0断开/1闭合
        unsigned char offline : 1;
        unsigned char reserve  :4;
        unsigned char reserve1;
    }devStatus;
    struct
    {
        unsigned char fun1		: 1;//0断开/1闭合
        unsigned char fun2		: 1;//0断开/1闭合
        unsigned char fun3		: 1;//0断开/1闭合
        unsigned char reserve  : 5;
        unsigned char reserve1;
    }warningStatus;
    struct
    {
        unsigned char fault0		: 1;//0断开/1闭合
        unsigned char fault1		: 1;//0断开/1闭合
        unsigned char fault2		: 1;//0断开/1闭合
        unsigned char fault3     : 1;
        unsigned char fault4		: 1;//0断开/1闭合
        unsigned char fault5		: 1;//0断开/1闭合
        unsigned char fault6		: 1;//0断开/1闭合
        unsigned char fault7     : 1;
        unsigned char fault8		: 1;//0断开/1闭合
        unsigned char fault9		: 1;//0断开/1闭合
        unsigned char fault10		: 1;//0断开/1闭合
        unsigned char fault11     : 1;
        unsigned char fault12		: 1;//0断开/1闭合
        unsigned char fault13     : 1;
        unsigned char fault14		: 1;//0断开/1闭合
        unsigned char fault15		: 1;//0断开/1闭合
    }faultStatus;

    unsigned int HWVersion;
    unsigned int SWVersion;
    float tmp_IGBT1;
    float tmp_IGBT2;
    float tmp_IGBT3;
    float tmp_IGBT4;
    float tmp_IGBT5;
    float tmp_IGBT6;
    float tmp_IN;
    float tmp_OUT;
}stEnergyStorageDCDCSingle;
Q_DECLARE_METATYPE(stEnergyStorageDCDCSingle)

typedef struct __DCDC_ESInfo
{
    QMap<QByteArray, stEnergyStorageDCDCSingle> mapSingle;
}stDCDC_ESInfo;
Q_DECLARE_METATYPE(stDCDC_ESInfo)

//EMS数据

typedef struct __EMS_Info
{
    mutable int onlineCounter       ;   //在线计数器
    float tempHighVol;
    float humidityHighVol;
    float tempLowVol_in;
    float humidityLowVol_in;
    float tempLowVol_out;
    float humidityLowVol_out;

    unsigned char smokeSensor_lowVolIn		;//0断开/1吸合
    unsigned char frameFeedback		;//0断开/1吸合
    unsigned char minorLoadbreaker_630A1		;//0断开/1吸合
    unsigned char minorLoadbreaker_630A2		;//0断开/1吸合
    unsigned char smokeSensor_lowVolOut		;//0断开/1吸合
    unsigned char minorLoadbreaker_400A1		;//浪涌反馈0异常/1正常
    unsigned char minorLoadbreaker_400A2		;//灭火器0异常/1正常
    unsigned char minorLoadbreaker_400A3		;//灭火器0异常/1正常
    unsigned char acdcBreaker		;//灭火器0异常/1正常
    unsigned char importBreaker1		;//灭火器0异常/1正常
    unsigned char importBreaker2		;//灭火器0异常/1正常
    unsigned char emergncyStop		;//灭火器0异常/1正常

    unsigned char SmoothSwitch_acdc_FQ;
    unsigned char SmoothSwitch_acdc_SI;
    unsigned char SmoothSwitch_unit1_ES;
    unsigned char SmoothSwitch_unit2_ES;
    unsigned char GridState;
}stEMSInfo;
Q_DECLARE_METATYPE(stEMSInfo)

/*能效管理数据结构定义 add by XX 2017-08-15*/
typedef QMap<unsigned char, stEnergyStorageCabinetInfo> ESCabAllDataMap;
typedef QMap<unsigned char, stEnergyStorageBatteryInfo> ESBatAllDataMap;
typedef QMap<unsigned char, stPhotoVoltaicCabinetInfo> PhCabAllDataMap;
typedef QMap<unsigned char, stSystemControlCabinetInfo> SCCabAllDataMap;
typedef QMap<unsigned char, stFourQuadrantCabinetInfo> FQCabAllDataMap;
typedef QMap<unsigned char, stChargeDischargeCabinetInfo> CDCabAllDataMap;
typedef QMap<unsigned char, stTotalDistributionCabinetInfo> TDCabAllDataMap;
typedef QMap<unsigned char, stPowerOptimizerInfo> POModAllDataMap;
typedef QMap<unsigned char, stHygrothermographInfo> HygorothermographMap;
typedef QMap<unsigned char, stSingleInverterCabInfo> SICabAllDataMap;
typedef QMap<unsigned char, stACDCModuleInfo> ACDCModAllDataMap;
typedef QMap<unsigned char, stDCDC_CDCModuleInfo> DCDC_CDCModAllDataMap;    //充放电柜DCDC模块
typedef QMap<unsigned char, stDCDC_ESInfo> DCDC_ESAllDataMap;    //储能柜DCDC模块
typedef QMap<unsigned char, stEMSInfo> EMSAllDataMap;   //EMS数据

typedef struct __EnergyPlanDatas
{
    ESCabAllDataMap esCabMap;
    ESBatAllDataMap esBatMap;
    PhCabAllDataMap phCabMap;
    SCCabAllDataMap scCabMap;
    FQCabAllDataMap fqCabMap;
    CDCabAllDataMap cdCabMap;
    TDCabAllDataMap tdCabMap;
    POModAllDataMap poModMap;
    HygorothermographMap hyModMap;
    SICabAllDataMap siCabMap;
    ACDCModAllDataMap   acdcModMap;
    DCDC_CDCModAllDataMap dcdcModMap_cd;
    DCDC_ESAllDataMap dcdcModMap_es;
    EMSAllDataMap emsMap;
}stEnergyPlanDatas;

Q_DECLARE_METATYPE(ESCabAllDataMap)
Q_DECLARE_METATYPE(ESBatAllDataMap)
Q_DECLARE_METATYPE(PhCabAllDataMap)
Q_DECLARE_METATYPE(SCCabAllDataMap)
Q_DECLARE_METATYPE(FQCabAllDataMap)
Q_DECLARE_METATYPE(CDCabAllDataMap)
Q_DECLARE_METATYPE(TDCabAllDataMap)
Q_DECLARE_METATYPE(POModAllDataMap)
Q_DECLARE_METATYPE(HygorothermographMap)
Q_DECLARE_METATYPE(SICabAllDataMap)
Q_DECLARE_METATYPE(ACDCModAllDataMap)
Q_DECLARE_METATYPE(DCDC_CDCModAllDataMap)
Q_DECLARE_METATYPE(DCDC_ESAllDataMap)
Q_DECLARE_METATYPE(EMSAllDataMap)

/*直流柜数据结构定义*/
typedef QMap<unsigned char, stCCUDatasItem>			CCUAllDatasMap;
typedef QMap<unsigned char, stPDUDatasSingle>		PDUAllDatasMap;
typedef QMap<unsigned char, stBranchDatasSingle>	BranchAllDatasMap;
typedef QMap<unsigned char, stDCModuleDatasSingle>  DCModuleDatasMap;
typedef QMap<unsigned char, unsigned char>			TerminalDataMap;
typedef QMap<unsigned char, stCCUADviceSpecificationsSingle>			CCUADviceSpecificationsMap;

typedef struct __DCCabinetDatas
{
    CCUAllDatasMap ccuMap;
    PDUAllDatasMap pduMap;
    BranchAllDatasMap branchMap;
    DCModuleDatasMap dcmoduleMap;
    TerminalDataMap terminalMap;
    CCUADviceSpecificationsMap ccuDeviceMap;
}stDCCabinetDatas;
Q_DECLARE_METATYPE(CCUAllDatasMap)
Q_DECLARE_METATYPE(PDUAllDatasMap)
Q_DECLARE_METATYPE(BranchAllDatasMap)
Q_DECLARE_METATYPE(DCModuleDatasMap)
Q_DECLARE_METATYPE(TerminalDataMap)
Q_DECLARE_METATYPE(CCUADviceSpecificationsMap)

//能效系统柜子信息
typedef QMap<unsigned char, stCabinetData>		CabinetAllDatasMap;

typedef struct __TerminalStatus
{
	unsigned char cCanAddr;//CAN地址
	unsigned int  validFlag;		//数据有效标志位，值为ALL_VALID_NUMBER即有效
	unsigned char psTermianlVer[LENGTH_TERMINAL_VER];//模块版本
	bool bEnabled;//是否激活，不可用不显示状态信息  ++++++++
	unsigned char  cType; //类型 0：交流单项    1:交流三相     2：直流     3 : 分支箱    4 : CCU
	unsigned char  cStatus;//modiby by YCZ 2016-01-22 设备充电状态的细化，结合枪状态。见enum CHARGE_STATUS
	CHARGE_STATUS_CHANGE_TYPE en_ChargeStatusChangeType;//用来描述充电机的状态变化
	char lastActiveTime[25];						//

	//终端状态
	FRAME_REMOTE_SINGLE stFrameRemoteSingle;//遥信
	FRAME_REMOTE_MEASUREMENT1 stFrameRemoteMeSurement1;//遥测1
	FRAME_REMOTE_MEASUREMENT2 stFrameRemoteMeSurement2;//遥测2
	FRAME_BMS_INFO  stFrameBmsInfo;//BMS信息 充电中
	FRAME_BMS_HAND  stFrameBmsHand;//BMS信息 握手
	FRAME_BMS_PARAM  stFrameBmsParam;//BMS信息 参数配置
	FRAME_BMS_CHARGE_TERM  stFrameBmsChargeTerm;//BMS信息 充电终止

	unsigned char FreeModule;//是否有空闲模块

    bool chargeResponseFlag;//响应充电指令标志位:true响应，false不响应
    unsigned char chargeManner;//充电方式：０未知，１双枪充电，２单枪充电
    unsigned char gunType;//枪类型：０未知，１主枪，２副枪，3配对错误
    unsigned char gunnum;//枪分组数据量

    //枪头温度
    float Qiangtoutemp;//smm add 枪头温度
    unsigned char bTicketPrint;   //打印小票标志位 1 允许打印  2 不允许打印
} TerminalStatus;
Q_DECLARE_METATYPE(TerminalStatus)

//充电业务状态属性数据
typedef struct __CHARGE_STEP_VALUE
{
    unsigned char uc_hold_time;//状态超时时间
    CHARGE_STEP_TYPE uc_charge_step_timeout;//超时返回应切换的状态
    char sStartTimeStep[20];//状态机切换的时间
}CHARGE_STEP_VALUE;

#define RESEND_TIMES 5//充电指令重发次数

//充电业务指令重发数据
typedef struct __REPEAT_CMD
{
    __u8 NowCount;//当前条数
    __u8 MaxCount;//总条数
}REPEAT_CMD;

//用于记录ZIGBEE充电发起者的信息
typedef struct __ZIGBEE_ADDR
{
    unsigned  short  pdu_primary_address;      //Zigbee网络主地址
    unsigned  short   pdu_secondary_address; //网络次地址, 取值范围：1~65535，默认值：1
} ZIGBEE_ADDR;

//启动充电方式
typedef enum 
{
	UNKNOWN_START = 0,//自启动
	CLOUD_START,	//平台启动
	CARD_START,		//刷卡启动
	VIN_START,		//VIN启动
	PLATE_START,	//车牌启动
	SCANCODE_START	//反扫码启动
}START_CHARGE_WAY;

typedef enum
{
	UNKNOWN_TYPE = 0,	
	IMMEDIATELY_CHARGE,	//立即充电
	SAVE_CHARGE,		//经济充电
	URGENT_CHARGE,		//限制充电
	ALLOWED_SECOND,		//按时间充电
	ALLOWED_KWH,		//按电量充电
	ALLOWED_RMB,		//按金额充电
	ALTERNATE_CHARGE,	//轮充
	DISCHARGE			//放电
}START_CHARGE_TYPE;

//充电业务状态机及属性(包含指令跟订单信息)
typedef struct __CHARGE_STEP
{
    //-------------------指令订单共用---------------------------------//
    unsigned char ucCanAddr;          //CAN地址
    unsigned char ucCmdSrc;           //指令源 (枚举变量 CHARGE_CMD_SRC_TYPE) 二期
    unsigned char ucStartReason;//开始原因 (枚举变量  START_CHARGE_CMD_RSN)  二期
    char sGUIDNo[LENGTH_GUID_NO];//GUID号 二期
    char sEventNo[LENGTH_EVENT_NO];//流水号,以下为业务识别信息
    char sCardNo[LENGTH_CARDNO];//卡号
    char sScanCodeNo[LENGTH_GUID_NO];//反向扫码客户ID
    char sVIN[LENGTH_VIN_NO];//VIN号
    char sCarLisence[LENGTH_CAR_LISENCE];//车牌号
    char ucQueueMsgFromServer;//轮充模式下，服务器下发的排队信息
    //-------------------指令相关---------------------------------//
    unsigned char ucCmdValue;       //指令控制字 (枚举变量 CHARGE_CMD_TYPE)
    ZIGBEE_ADDR stZigeeAddr;       // ZIGBEE充电发起者的信息 预留
    //-------------------指令状态机相关---------------------------------//
    char  sRecvTime[20];//接收指令时间 ["2016-11-18 10:40:41"]
    CHARGE_STEP_TYPE enChargeStep;//指令执行的状态 （枚举变量 CHARGE_STEP_TYPE)
    CHARGE_STEP_VALUE stChargeStepValue;//状态的属性数据
    CMD_ACK_TYPE enCmdAck;        //指令回复ACK
    REPEAT_CMD stRepeatCmd;        // 充电业务指令重发数据
    CMD_END_REASON_TYPE enCmdEndReason; //状态机超时,指令校验,定时器判断的充电指令执行结果
    //-------------------订单相关---------------------------------//
    char sOrderUUID[LENGTH_GUID_NO];//内部产生的uuid
    ORDER_STATUS enOrderStatus;//订单状态
    uint u32EnergyPausetCharge;//开始暂停充电时的电能 单位0.01kw  add by zjq
    uint u32EnergyStartCharge;//开始充电时的电能 单位0.01kw
    uint u32EnergyEndCharge;//结束充电时的电能 单位0.01kw
    uint u32TotalChargeEnergy;//总充放电电能 单位0.01kw
    char sStartTime[20];//开始充电时间
    char sEndTime[20];//结束充电时间
    unsigned char sLastRecordEnergy[2];//上次记录充电电量时间(用于存储冻结电量)
    unsigned short u12TotalChargeTime;	//累计充电时长
    __u8 ucStartSOC;//开始SOC
    __u8 ucEndSOC;//结束SOC

    unsigned char ucQueueMsg;//群充模式，策略A下车辆充电排队信息
    unsigned char ucStopReasonDev;//设备中止充电原因  ++++++++ stop_reason
    unsigned char ucStopReasonCloud;//服务器中止充电原因
    unsigned char ucStopReasonCSCU;//CSCU中止充电原因
    //-------------------刷卡申请充电相关---------------------------------//
    char CardSrcType;//卡号来源类型1.单桩刷卡, 2.集中刷卡

    //应急充电
    char cOrderType;	//订单类型 0：正常订单 1：应急订单 ２：经济订单
    char cQueueGroup;	//所属轮充组
    char cChargeType;	//充电类型 
    char cChargeWay;	//启动方式 1：平台启动，2：设备启动—刷卡，3：设备启动—VIN，4：设备启动—车牌号
    uint fLimitEnergy;//限制电量 保留两位小数，限制电量充电时必填
    char cGunNum;		//枪数量 默认1
	char cOrderSync;	//订单同步状态 0：未同步 1：不同步 2：已同步

    //3.0协议新增
    uchar cPolicyType;		//策略类型 0：充电 1：放电
    char sMarkId[40];
    char sUserId[40];
    uchar cUserType;				//用户类型
    uchar cUserLevel;				//用户级别 0：不可欠费 1：可欠费
    float fCommonBalance;			//余额
    int iPeriodIndex;			//当前时段编号
    int iPeriodStart;			//时段开始时间
    int iPeriodStop;			//时段结束时间
    int iPeriodCost;			//时段费用
    float fPeriodEnergyStart;	//时段开始电量
    float fPeriodEnergyStop;	//时段结束电量
    int iChargedFees;			//订单已充电费用
    int iChargingFees;			//订单当前充电费用

    //削峰填谷
    //unsigned char PeakShavingRetryTimes ;   //设备回复NACK允许错峰重复的次数3次

	//南京3.0协议
    char sBillCode[LENGTH_BILL_CODE];//订单号
	uint iCurveState;//0：未启用功率曲线 1：功率曲线下发时间
	int iCurveCnt;//功率曲线检测计数
	char cCurveType;//功率曲线类型
	int iCurveStart;//功率曲线时段开始
	int iCurveStop;//功率曲线时段结束
    double iCurveValue;//功率曲线限值
    char sSchedulStatus;  //调度状态
//    //冻结电量
    //bool bChargeFreezeEnery;//集控刚刚启动:false/ 冻结电量无误或已经修正 :true
}CHARGE_STEP;

//解析刷卡相关数据
typedef struct _AccountData
{
    unsigned char ucType;//账户类型
    int uiValue;//账户金额
}__attribute__ ((packed))AccountData;

typedef struct _AccountInfo
{
    unsigned char ucNum;//账户数量
    AccountData stAccount[5];//账户详情5个
}__attribute__ ((packed))AccountInfo;

typedef struct _PolicyInfo
{
    unsigned char ucStartH;
    unsigned char ucStartM;
    unsigned char ucStopH;
    unsigned char ucStopM;
    unsigned short usEnergyPrice;
    unsigned short usServicePrice;
}PolicyInfo;

//主动防护参数设置
typedef struct _FrameActiveProtectionSet
{
    unsigned short usBalanceCurrentCoefficient; //均衡阶段电流系数
    unsigned char ucBalanceTime;//均衡时间

    unsigned char ucTempThreshold;//温升阈值
    unsigned char ucTempRiseEnsureTime;//温升确认时间
    unsigned char ucNoTempProtect;//禁止热失控防护

    unsigned short usSingleOverVoltageThreshold;//单体过压阈值
    unsigned char ucSingleOverVoltageEnsureTime;//单体过压确认时间
    unsigned char ucNoSingleOverVoltageProtect;//禁止单体过压防护

    unsigned short usTotalOverVoltageThreshold;//整体过压阈值
    unsigned char ucOverVoltageEnsureTime;//整体过压确认时间
    unsigned char ucNoOverVoltageProtect;//禁止整体过压防护

    unsigned short usOverCurrentThreshold;//过流阈值
    unsigned char ucOverCurrentEnsureTime;//过流确认时间
    unsigned char ucNoOverCurrentProtect;//禁止过流防护

    unsigned char ucOverTempThreshold;//过温阈值
    unsigned char ucOverTempEnsureTime;//过温确认时间
    unsigned char ucNoOverTempProtect;//禁止过温防护

    unsigned char ucBelowTempThreshold;//低温保护阈值
    unsigned char ucBelowTempEnsureTime;//低温确认时间
    unsigned char ucNoBelowTempProtect;//禁止低温防护
    unsigned short usBMSRelayAdhesionVoltageThreshold;//继电器粘连电压阈值
    unsigned char ucBMSRelayAdhesionEnsureTime;//继电器粘连确认时间
    unsigned char ucNoBMSRelayAdhesion;//禁止继电器粘连防护

    unsigned short usBMSRelayBreakOffVoltageThreshold;//继电器开路电压阈值
    unsigned short usBMSRelayBreakOffCurrentThreshold;//继电器开路电流阈值
    unsigned char ucBMSRelayBreakOffEnsureTime;//继电器开路确认时间
    unsigned char ucNoBMSRelayBreakOff;//禁止继电器开路防护

    unsigned char ucOverChargeJudgemetCoefficient; //过充判断系数设置
    unsigned char ucOverChargeEnergyReferanceValue; //过充判断电量参考值
    unsigned char ucOverChargeEnergyEnsureTime; //过充判断确认时间

    unsigned char ucNoOverCharge; //禁止过充防护
    unsigned char ucBMSDataRepetTime;//BMS数据重复时间
    unsigned char ucNoBMSDataRepet;//禁止BMS数据重复防护
    unsigned char ucNoBMSCheck; //禁止BMS数据校验告警检测

    unsigned short usOVTh_LNCM;//三元锂电池过压阈值
    unsigned short usOVTh_LTO;//钛酸锂电池过压阈值
    unsigned short usOVTh_LMO;//锰酸锂电池过压阈值

    unsigned char ucOTTh_LNCM;//三元锂电池过温阈值
    unsigned char ucOTTh_LTO;//钛酸锂电池过温阈值
    unsigned char ucOTTh_LMO;//锰酸锂电池过温阈值

}__attribute__ ((packed)) FrameActiveProtectionSet;

//柔性充电设置
typedef union _FlexChargeArg
{
    struct
    {
        unsigned char ucStartSOC;
        unsigned char ucStopSOC;
        unsigned short usCurCoefficent;
    }stType1[20];
    struct
    {
        unsigned char ucTempStart;
        unsigned char ucTempStop;
        unsigned short usCurCoefficent;
    }stType2[20];
    struct
    {
        unsigned short usTimeLength;
        unsigned short usCurCoefficent;
    }stType3[20];
}__attribute__ ((packed)) FlexChargeArg;

//柔性充电参数设置 : 不定长长包,不进行模拟
typedef struct _FrameFlexibleParamSet
{
    unsigned char ucType;   //指令类型
    FlexChargeArg Data; //柔性充电参数数据
}__attribute__ ((packed)) FrameFlexibleParamSet;

//通用静态参数设置
typedef struct _FrameGeneralStaticParamSet
{
    struct
    {
        unsigned char ucVINEnableFlag : 2;  //VIN使能标识
        unsigned char ucElecLockType : 2;    //电子锁类型
        unsigned char ucElecLockEnableFlag : 2; //电子锁使能
        unsigned char ucAuxPowerSet : 2;//辅源类型
    }stByte1;
    struct
    {
        unsigned char ucBMSProType : 4; //新老国标类型设置
        unsigned char ucResrved : 4; //预留(0xFF)
    }stByte2;
    unsigned short usGunMaxCurrent; //枪头最大电流
    unsigned char ucTermID; //终端ID设置
    unsigned char ucReserved[3];

}__attribute__ ((packed)) FrameGeneralStaticParamSet;

//通用动态参数设置
typedef struct _FrameGeneralDynamicParamSet
{
    unsigned char ucParamType;
    union
    {
        struct  //指令类型 : 01
        {
            unsigned char ucPriority;      //充电优先级
            unsigned char ucReserved[6];    //预留6字节
        }stCmd01;
        struct  //指令类型 : 02
        {
            unsigned char ucSec;
            unsigned char ucMin;
            unsigned char ucHour;
            unsigned char ucDay;
            unsigned char ucMonth;
            unsigned char ucYearLow;
            unsigned char ucYearHigh;
        }stCmd02;
        struct  //指令类型 : 03
        {
            unsigned char ucWorkModle;  //充电模式
            unsigned char ucGroupType;       //群充轮充控制模式
            unsigned char ucGroupStrategy;          //群充策略
            unsigned char ucReserved[4];
        }stCmd03;
    }Data;
}__attribute__ ((packed)) FrameGeneralDynamicParamSet;

//CCU参数设置
typedef struct _FrameCCUParamSet_CCU
{
    unsigned char ucCCUID;    //设置CCU的ID
    unsigned char ucTermStartID;    //设置直流柜终端的起始地址
    unsigned short usCabMaxPower;//直流柜最大输出功率
    unsigned char ucReserved3[4];

}__attribute__ ((packed)) FrameCCUParamSet_CCU;

//分支箱参数设置
typedef struct _FrameBranchParamSet_CCU
{
    unsigned char ucBranchID;    //分支箱内部编号
    unsigned char ucNewID;  //分支箱的新ID
    unsigned char ucBranchNum;//分支数
    struct
    {
        unsigned char ucLightEnable : 2;
        unsigned char ucReserved : 6;
    }stByte4;
    unsigned char ucReserved[4];

}__attribute__ ((packed)) FrameBranchParamSet_CCU;

//PDU参数设置
typedef struct _FramePDUParamSet_CCU
{
    unsigned char ucPDUID;    //分支箱内部编号
    unsigned char ucNewID;  //分支箱的新ID
    unsigned char ucGunNum;//枪头数
    struct
    {
        unsigned char ucLightEnable : 2;
        unsigned char ucReserved : 6;
    }stByte4;
    unsigned char ucReserved[4];

}__attribute__ ((packed)) FramePDUParamSet_CCU;

//模块参数设置
typedef struct _FrameModuleParamSet_CCU
{
    unsigned char ucModuleID;    //模块ID
    struct
    {
        unsigned char ucLightEnable : 2;
        unsigned char ucReserved : 6;
    }stByte2;
    unsigned char ucReserved[6];
}__attribute__ ((packed)) FrameModuleParamSet_CCU;

//GPS获取结构体
typedef struct _GPS_DevMng
{
    QDateTime utc;//时间年月日时分秒
    char dw;//定位状态，A=有效定位，V=无效定位
    double Latitude;//纬度ddmm.mmmm（度分）格式（前导位数不足则补0）
    double Longitude;//经度dddmm.mmmm（度分）格式（前导位数不足则补0）
    double e_rate;//地面速率（000.0~999.9节，Knot，前导位数不足则补0）
    double e_course;//地面航向（000.0~359.9度，以真北为参考基准，前导位数不足则补0）
    double M_vt;//磁偏角（000.0~180.0度，前导位数不足则补0）
    char M_id;//模式指示（仅NMEA0183 3.00版本输出，A=自主定位，D=差分，E=估算，N=数据无效）
}GPS_DevMng;

//流量统计结构体
typedef struct _TrafficState_DevMng
{
    unsigned long long tx;
    unsigned long long rx;
    unsigned long sp_t;
    unsigned long sp_r;
}__attribute__ ((packed)) TrafficState_DevMng;


//直流柜故障记录信息----存入数据库
typedef struct _FaultRecord_DCcab
{
    unsigned char ucCCUAddr;
    unsigned char ucDevID;
    unsigned char ucFaultCode;
    unsigned char ucMinPDUID;
    unsigned char ucMaxPDUID;
    mutable unsigned char ucFaultState;
    mutable unsigned char ucCounter;    //超时定时器, 超过设定时间没有收到故障维持报文,则认为故障解除
    int iSerialNum; //唯一编号
    QByteArray StartTime;    //故障开始时间
}FaultRecord_DCcab;

//设备模块信息----存入数据库
typedef struct _SpecificInfo_DCcab
{
    unsigned char ucCCUAddr;
    unsigned char ucDevID;
    unsigned char ucSlotNum;
    QByteArray SoftwareVer;
    QByteArray SoftwareVer1;
    QByteArray SoftwareVer2;
    QByteArray HardwareVer;
    QByteArray SerialNumber;
}SpecificInfo_DCcab;

//设备的类型
typedef enum _EnumDeviceType
{
    DEV_CSCU = 1,
    DEV_AMMETER = 2,
    DEV_CCU = 3,
    DEV_PDU = 4,
    DEV_GUN = 5,
    DEV_MODULE = 6,
    DEV_INVERTER = 7,   //逆变器   (未用到)
    DEV_STORAGE_CABINET = 8,    //储能柜
    DEV_STORAGE_BATTERY = 9, //储能电池
//    DEV_DCDC_MODULE = 10,   //DCDC模块
    DEV_PV_CONTROL_CABINET = 11,    //光伏控制柜
    DEV_AC_ENTER_DISTRIBUTION_CAB = 12, //交流进线配电柜(未用到)
    DEV_SYSTEM_CONTROL_CAB = 13,    //系统控制柜
    DEV_FOUR_QUADRANT_CHARGE_CAB = 14,  //四象限变换柜
    DEV_ACDC_MODULE = 15,  //ACDC模块
    DEV_DC_CHARGE_DISCHARGE_CAB = 16,   //直流充放电柜
    DEV_TOTAL_DISTRIBUTION_CAB = 17, //总配电柜
    DEV_POWER_OPTIMIZER = 18, //功率优化器
    DEV_TEMP_HUMI_SENSOR = 19,   //温湿度传感器
    DEV_ACDC_INDEPENDENT_INVERTER_CABINET = 20,  //独立逆变柜
    DEV_CHARGE_DISCHARGE_CAB_DCDC_MODULE = 23,
    DEV_ENERGY_STORAGE_CAB_DCDC_MODULE = 24,
    DEV_EMS_CAB = 25   //EMS设备柜

}EnumDeviceType;

//设备子类型
typedef enum _EnumSubDeviceType
{
    SUB_DEV_UNKNOWN = 0,
    SUB_DEV_DC_GUN = 501,
    SUB_DEV_AC_GUN = 502,
    SUB_DEV_CHARGE_BOW = 503,
    SUB_DEV_WIRELESS_GUN = 504

}EnumSubDeviceType;
//设备节点的结构
typedef struct _TDevice
{
    int  iParentID;      //上层节点设备编码 内部ID或CAN ID
    int  iCanID;        //设备编码 内部ID或CAN ID
    char  chName[40];      //设备节点名字, 场站名称, 终端名称
    EnumDeviceType  enDeviceType;      //设备类型
    EnumSubDeviceType enSubDeviceType;  //设备子类型
}__attribute__ ((packed)) TDevice;

//CCU 在线状态信息
typedef struct _CCUStatusOnline
{
    unsigned int ui_time; //上一次接收数据的时间
    unsigned char uc_status;  //0 表示离线，1表示在线
}CCUStatusOnline;

#endif // GENERALDATA_H
