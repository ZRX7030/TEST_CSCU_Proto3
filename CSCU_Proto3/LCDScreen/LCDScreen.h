#ifndef CLCDSCREEN_H
#define CLCDSCREEN_H
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QTextCodec>

#include "LCDScreen/LCDScreenProtocol.h"

const unsigned int PortNum_LCDScreen = 7000;
const unsigned int MinPakLen_LCDScreen = 11;  //最小包长度

typedef enum _ArgWaitCount_LCD
{
    ArgTermCount_LCD = 4,   //接收终端参数查询成功次数
    ArgCCUCount_LCD = 1
}ArgWaitCount_LCD;

typedef struct _ArgDelNode_LCD
{
    unsigned char ucArgCanID;   //终端参数CAN地址记录(用于终端参数查询/设置)
    mutable int iCounter;   //计数器
    int iMaxTime;   //最大次数
    int iDevType; //1: 终端, 2: CCU

}ArgDelNode_LCD;

//const int WaitTermArg_LCD = 5;  //接收终端参数查询成功次数

typedef enum _TimeOut_LCD
{
    TO_CardWaitCardNum_LCD = 30,
    TO_CardWaitAccResult_LCD = 30,
    TO_CardWaitStartResult_LCD = 90,
    TO_CardWaitStopResult_LCD = 90,
    TO_PeakSet_LCD = 5
}TimeOut_LCD;

//1: 读取卡号中,  2: 得到卡号, 3: 获取账户信息中, 4: 得到账户信息结果, 5:申请充电中, 6:得到申请充电结果, 7:申请结束充电中, 8:得到结束充电结果
typedef enum _CardStep_LCD
{
    Step_Free_LCD = 0,
    Step_WaitingCardNum_LCD = 1,
    Step_GetCardNum_LCD = 2,
    Step_WaitingAccResult_LCD = 3,
    Step_GetAccResult_LCD = 4,
    Step_WaitingStartChargeResult_LCD = 5,
    Step_GetStartChargeResult_LCD = 6,
    Step_WaitingStopChargeResult_LCD = 7,
    Step_GetStopChargeResult_LCD  = 8
}CardStep_LCD;

typedef struct _AddrMem_LCD
{
    unsigned char ucAddr;
    unsigned char uiCmdType;
    unsigned int uiDataType;
}AddrMem_LCD;

class cLCDScreen : public CModuleIO
{
    Q_OBJECT
public:
    cLCDScreen();
    ~cLCDScreen();

    //根据配置选项初始化
    int InitModule( QThread* pThread);
    //注册设备到总线
    int RegistModule();
    //启动模块
    int StartModule();
    //停止模块
    int StopModule();
    //模块工作状态
    int ModuleStatus();

private:
    //nihai add
//    void SendTermNameApply();
//    bool ParseCenterTermName(InfoMap CenterMap);
    //nihai end
    //建立TCP监听事件
    void newListen();
    //帧格式校验
    bool CheckFrame(const QByteArray &frameArray);
    //帧多包校验
    bool CheckMutiPackFrame(QList <QByteArray> &FrameList, QByteArray &recvArray);
    //超时时间校验
    void CheckTimeOut();
    //解析帧
    void ParseFrame(const QByteArray &recvArray);
    //直流特殊功能设置类数据
    void ParseDCSpecSet(unsigned char ucAddr, unsigned char ucCmdType, unsigned short usDataType);
    //参数获取
    void ParseArgQuery(const QByteArray &recvArray);
    //解析U盘指令
    void ParseUdiskCmd(const QByteArray &recvArray);
    //解析 总线接收刷卡卡号
    bool ParseCenterCardNumber(InfoMap CenterMap);
    //解析 总线接收账户信息
    bool ParseCenterAccountInfo(InfoMap CenterMap);
    //解析 总线接收刷卡内部申请, 开始充电, 结束充电结果 ---- 内部结果(充电模块返回), stCardResult.ucflag: 1, 开始充电; 2, 结束充电
    bool ParseCenterCardInResult(InfoMap CenterMap, CardResult_LCD &stCardResult);
    //解析 总线接收刷卡开始充电/结束充电结果----外部结果(充电服务模块返回),  stCardResult.ucflag: 1, 开始充电; 2, 结束充电
    bool ParseCenterCardOutResult(InfoMap CenterMap, CardResult_LCD &stCardResult);
    //解析 总线接收VIN开始充电/结束充电结果----外部结果(充电服务模块返回)
    bool ParseCenterVinInResult(InfoMap CenterMap, unsigned char ucType);   //add by songqb 2018-1-17
    //解析 总线接收到U盘处理结果
    bool ParseCenterUdiskResult(InfoMap CenterMap);

    //处理----维护数据指令
    void DealMaintainDataType(const QByteArray &recvArray);
    //处理----查询数据指令
    void DealQueryDataType(const QByteArray &recvArray);
    //处理----设置数据指令
    void DealSetDataType(const QByteArray &recvArray);
    //处理----状态数据指令
    void DealStateDataType(const QByteArray &recvArray);
    //处理----实时数据指令
    void DealRealDataType(const QByteArray &recvArray);
    //处理----历史数据指令
    void DealHisDataType(const QByteArray &recvArray);
    //处理----交互数据指令
    void DealInterActiveType(const QByteArray &recvArray);

    //生成随机码
    unsigned int generateRandCode();
    //生成动态密码
    unsigned int generateDynamicCode(unsigned int param);

    //发送版本信息
    void sendVersion(unsigned char ucAddr);
    //发送 系统参数
    void sendSysParam(unsigned char ucAddr);
    //发送 相别设置
    void sendPhaseType(unsigned char ucAddr);
    //发送 电表数据
    void sendAmmeterAddr(unsigned char ucAddr);
    //发送 登录密码数据
    void sendPasswd(unsigned char ucAddr);
    //发送 IO常开常闭状态
    void sendIOState(unsigned char ucAddr);
    //发送 二维码
    void send2DbarCodes(unsigned char ucAddr);
    //发送 直流柜某参数类型设备个数
    void sendFrameDCCabArgTypeNum(unsigned char ucAddr, unsigned char ucType);
    //发送 系统特殊功能
    void sendSysSpecSet(unsigned char ucAddr);
    //发送 直流特殊功能
    void sendDCSpecSet(unsigned char ucAddr);
    //发送 负荷约束功能
    void sendLoadLimit(unsigned char ucAddr);
    //发送 错峰充电功能
    void sendPeakSet(unsigned char ucAddr);
    //发送 枪参数
    void sendGunArg(unsigned char ucAddr, unsigned char ucArgCanID);
    //发送 CCU参数
    void sendCCUArg(unsigned char ucAddr, unsigned char ucArgCanID);
    //发送 直流柜设备个数
    void sendDCCabDevNum(unsigned char ucAddr, const QByteArray &recvArray);
    //发送 随机码(本地充电)
    void sendRandCode(unsigned char ucAddr);
    //发送 密码比较结果(本地充电)
    void sendDynamicPasswordResult(unsigned char ucAddr, const QByteArray &recvArray);
    //发送 本地密码(本地充电)
    void sendLocalChargePassword(unsigned char ucAddr);

    //发送 终端状态
    void sendTermState(unsigned char ucAddr, unsigned char ucCanID);
    //发送 CSCU状态
    void sendCSCUState(unsigned char ucAddr);
    //发送 终端测量数据----普通版
    void sendTermMeasure_Normal(unsigned char ucAddr, unsigned char ucCanID);
    //发送 终端测量数据----普通版
    void sendTermMeasure_Card(unsigned char ucAddr, unsigned char ucCanID);
    //发送 终端BMS数据
    void sendTermBMS(unsigned char ucAddr, unsigned char ucCanID);
    //发送 终端故障信息数据
    void sendTermFaultInfo(unsigned char ucAddr, unsigned char ucCanID);
    //发送 终端充电报告
    void sendTermChargeReport(unsigned char ucAddr, unsigned char ucCanID);
    //发送 进线侧信息数据
    void sendAmmeterData(unsigned char ucAddr, const QByteArray &recvArray);
    //发送 环境信息
    void sendEnvInfo(unsigned char ucAddr, const QByteArray &recvArray);
    //发送 功率模块实时数据
    void sendModData(unsigned char ucAddr, unsigned char ucCanID, unsigned char ucInnerID);
    //发送 PDU实时数据
    void sendPDUData(unsigned char ucAddr, unsigned char ucCanID, unsigned char ucInnerID);
    //发送 CCU实时数据
    void sendCCUData(unsigned char ucAddr, unsigned char ucCanID, unsigned char ucInnerID);
    /***********************************************************************
     *函数名：sendNoRemoveFaultRecord
     *参数说明：ucAddr   终端地址，recvArray    接收到的原始数据流
     *功能：发送 未消除故障信息
     *备注 : add by muty 20170913
     **********************************************************************/
    void sendNoRemoveFaultRecord(unsigned char ucAddr, const QByteArray &recvArray);
    /***********************************************************************
     *函数名：sendNoRemoveFaultRecordNum
     *参数说明：ucAddr   终端地址，recvArray    接收到的原始数据流
     *功能：发送 未消除故障记录条目
     *备注 : add by muty 20170913
     **********************************************************************/
    void sendNoRemoveFaultRecordNum(unsigned char ucAddr, const QByteArray &recvArray);

    //发送 历史记录条目
    void sendRecordNum(unsigned char ucAddr, const QByteArray &recvArray);
    //发送 故障记录
    void sendFaultRecord(unsigned char ucAddr, const QByteArray &recvArray);
    //发送 充电记录
    void sendChargeRecord(unsigned char ucAddr, const QByteArray &recvArray);
    //发送 操作记录
    void sendOperateRecord(unsigned char ucAddr, const QByteArray &recvArray);

    //发送 人机交互结果
    void sendOperateResult(unsigned char ucAddr, unsigned short usDataType, unsigned char ucResult, unsigned char ucReboot);
    //发送 刷卡结果
    void sendCardResult(unsigned char ucAddr, CardResult_LCD stCardResult, unsigned int uiDataType);
    //发送 升级结果
    void sendUdiskResult(unsigned char ucAddr, UdiskResult_LCD stResult);

    //发送 遥控结果
    void sendCtrlCmdResult(unsigned char ucAddr, LocalCharge_LCD &stLocalCharge);

    //发送 刷卡申请卡号到总线
    void sendCenterApplyCardNum(unsigned char ucAddr, unsigned char ucCanID);
    //发送 刷卡结束请求, 让读卡器停止读卡
    void sendCenterCardStop();
    //发送 申请用户信息到总线
    void sendCenterApplyUserInfo(unsigned char ucCanID);
    //发送 申请开始充电到总线(刷卡)
    void sendCenterApplyStartCharge_Card(CardCmdApply_LCD &stCmdApply);
    //发送 申请开始充电到总线(VIN)
    void sendCenterApplyStartCharge_VIN(CardCmdApply_LCD &stCmdApply);    //add by songqb 2018-1-16
    //发送 申请结束充电到总线(刷卡)
    void sendCenterApplyStopCharge_Card(unsigned char ucCanID);
    //发送 日志导出请求到总线
    void sendCenterLogOutApply();
    //发送 升级请求到总线
    void sendCenterUpdateApply();
    //发送 申请命令到总线
    void sendCenterCmdCtrlApply();

    //更新 系统参数
    int updateSysParam(const char *pData, unsigned short usLen);
    //更新 相别参数
    int updatePhaseType(const char * pData, unsigned short usLen);
    //更新 系统特殊功能设置
    int updateSysSpecSet(const char * pData, unsigned short usLen);
    //更新 直流特殊功能设置
    int updateDCSpecSet(const char * pData, unsigned short usLen);
    //更新 负荷约束设置
    int updateLoadLimitSet(const char * pData, unsigned short usLen);
    //更新 错峰充电设置
    int updatePeakSet(const char * pData, unsigned short usLen);
    //更新 屏幕密码设置
    int updatePasswd(const char * pData, unsigned short usLen);
    //更新 IO常开常闭设置
    int updateIOState(const char * pData, unsigned short usLen);
    //更新 二维码参数设置
    int update2DbarCodes(const char * pData, unsigned short usLen);
    //更新 终端参数设置
    int updateTermArgSet(const char * pData, unsigned short usLen);
    //更新 ccu参数设置
    int updateCCUArgSet(const char * pData, unsigned short usLen);

    //发送 数据到总线
    void SendCenterData(InfoMap ToCenterMap, InfoAddrType enType);
    //发送 消息
    void sendMessage(unsigned char * pData, unsigned int uiLen);

    //申请 直流特殊功能信息 到总线
    void applyCenterDCSpecFunc();
    //更新 本地充电密码设置
    int updateLocalChargePassword(const char * pData);
    //申请 终端参数查询 到总线
    void applyCenterTermArg(unsigned char ucCanID);
    //申请 CCU参数查询 到总线
    void applyCenterCCUArg(unsigned char ucCanID);

    //  GuoCheng Add
    //显示设置失败提示页面
    void DisplaySetPage(const QByteArray &recvArray);

    //结束充电按钮申请结束充电
    void SendApplyStopCharge(unsigned char canaddr);

    //结束充电按钮结果
    void sendCtrlStopCmdResult(unsigned char ucAddr, LocalChargeStop_LCD &stLocalCharge);

    //打印小票
    void sendTickectDevStart(unsigned char canid);
    //打印小票结果
    void ParseTicketPrintResult(InfoMap CenterMap,unsigned char ucAddr);

    void sendTickectDevData(unsigned char ucAddr);

signals:
    void sigToBus(InfoMap, InfoAddrType);

public slots:
    //接收控制中心数据
    void slotFromBus(InfoMap RecvCenterMap, InfoAddrType enType);
    //开始工作
    void ProcStartWork();
private slots:
    //建立连接
    void acceptConnection();
    //接收消息
    void recvMessage();
    //检测连接信号,建立连接
    void ProcOneSecTimeOut();
private:
    bool bWorkFlag;//工作状态标识, 1:工作, 0:空闲
    QList <AddrMem_LCD> DelayCmdList;
    QList <ArgDelNode_LCD> ArgDelList;  //参数设置处理列表
    unsigned int uiTimeCounter;   //超时处理计数器
    unsigned int uiPeakSetCounter;  //错峰功能设置超时处理计数器
    unsigned int uiLocalDynamicPassword;    //本地充电动态密码
    bool bWaitPeakSetFlag;//TRUE: 等待错峰充电设置结果, FALSE: 未等待
    bool bWaitTermArgQueryFlag; //TRUE: 等待终端参数获取结果, FALSE: 未等待

    unsigned char ucAddr_DCSpecSet;//直流特殊功能设置异步数据缓存列表
    unsigned char ucDCSpecAckStep;//直流特殊功能回复设置所在步骤
    unsigned char ucNowAddr;//当前屏幕地址
    unsigned char ucCardStep;//刷卡当前步骤, 1: 读取卡号中,  2: 得到卡号, 3: 获取账户信息中, 4: 得到账户信息结果, 5:申请充电中, 6:得到申请充电结果, 7:申请结束充电中, 8:得到结束充电结果
    unsigned char ucQueryArgStep;   //查询终端参数步骤, >设定值, 查询成功
    UdiskResult_LCD stUresult;//u盘操作结果
    CardResult_LCD stCardResult;//刷卡结果记录
    int iCardLen;   //卡号长度，配合stCardResult使用
    CardCmdApply_LCD stCmdApply;//刷卡申请结果
    LocalCharge_LCD stLocalCharge;  //本地充电结构体
    LocalChargeStop_LCD stLocalChargeStop;  //本地结束充电按钮
    TicketDevResult_LCD stTicketDevResult;// 打印小票结果
    void * *pDepends;
    QTimer * pTimer;
    QTcpServer * pServer;
    QTcpSocket * pSocket;
    cLCDScreenProtocol * pProtocol;
    //外部输入参数
    DevCache * pDevCache;
    ParamSet * pParamSet;
    DBOperate * pDBOperate;
    Log * pLog;
};

#endif // CLCDSCREEN_H
