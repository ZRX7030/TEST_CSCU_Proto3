#ifndef CHARGEPROTOCOL_H
#define CHARGEPROTOCOL_H

#include "protocol.h"
#include "chargeprotobuf.pb.h"
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>


namespace Charge{
	using namespace std;
	using namespace charge::protobuf;
	typedef google::protobuf::RepeatedPtrField<ChargingStrategy> PowerCurveList;
	typedef google::protobuf::RepeatedPtrField<AccountingStrategy> PolicyList;

	class ChargeProtocol : public Protocol
	{
		Q_OBJECT
		public:
			ChargeProtocol(Net *n);
			~ChargeProtocol();

			//数据帧解析
			bool parse(short type, char *buff, int len);
			//内部模块指令
			bool command(InfoMap &map, InfoAddrType &type);
			//上线
			bool login();
			//离线
			void logout(int type = 0);
			//心跳
			void heart();

		signals:
			void emergency(int flag);
			void syncAccount();

		private:
			//解析登录结果
			void parseLoginAns(char *data, int len);
			//解析心跳应答
			void parseHeartAns(char *data, int len);
			//发送心跳应答
			void sendHeartAns(char *data, int len);

			//解析开始充电指令
			void parseStartCharge(char *data, int len);
			//解析结束充电指令
			void parseStopCharge(char *data, int len);
			//解析暂停充电指令
			void parsePauseCharge(char *data, int len);
			//解析恢复充电指令
			void parseResumeCharge(char *data, int len);
			//充电指令应答
			void chargeAns(InfoMap &map);
			//发送开始充电指令应答
			void sendStartChargeAns(uchar can, QByteArray code, BoolEnum result);
			//发送结束充电指令应答
			void sendStopChargeAns(uchar can, QByteArray code, BoolEnum result);
			//发送暂停充电应答
			void sendPauseChargeAns(uchar can, QByteArray code, BoolEnum result);
			//发送恢复充电应答
			void sendResumeChargeAns(uchar can, QByteArray code, BoolEnum result);

			//申请账户
			void applyAccount(InfoMap &map);
			//解析账户详情应答
			void parseAccountAns(char *data, int len);
			//申请开始充电
			void applyStartCharge(InfoMap &map);
			//申请结束充电
			void applyStopCharge(InfoMap &map);
			//解析申请开始充电应答
			void parseApplyStartChargeAns(char *data, int len);
			//解析申请结束充电应答
			void parseApplyStopChargeAns(char *data, int len);

			//账户信息同步请求
            bool syncAccount(int accountType);
			//解析账户信息
			void parseSyncAccount(char *data, int len);

			//解析功率曲线指令
			void parsePowerCurve(char *data, int len);
			//保存功率曲线
			uint savePowerCurve(PowerCurveList list, int canAddr, int curveType, string billCode);
			//保存充电策略
			uint saveChargePolicy(PolicyList list, int canAddr, string billCode);

			//提交订单
			void uploadBill(InfoMap &map);
			//创建订单
			bool createBill(BillInfo *bill, QString strBillId, int idType = 1);
			//解析未确认订单召唤
			void unconfirmedBillCall(char *data, int len);
			//解析流水号召唤订单
			void codeBillCall(char *data, int len);
			//解析订单提交应答
			void parseUploadBillAns(char *data, int len);
			//充电订单通知
            void billNotify(uchar canAddr, QString strGuid);
			//发送未确认订单
			void sendUnConfirmedBill(int canAddr);
            //发送未被确认订单 add by zrx
            void sendUnConfirmedBillOne(int canAddr);
            void queryUnConfirmedBillOne(int canAddr); //查询未被确认的订单
            void sendUnConfirmedBillSendReult(InfoMap map);  //未确认订单召唤返回失败

			//创建充电机状态
			void createChargerState(ChargerState *state, char logicStatus);
			//突发充电机逻辑工作状态
			void burstLogicState(InfoMap &map);
			//发送充电机状态
			void chargerStateCall(char *data, int len);
			//突发遥信
			void burstSignal(InfoMap &map);
			//遥信召唤
			void signalCall(char *data, int len);
			//遥测召唤
			void measureCall(char *data, int len);
			//发送遥测数据
            void sendMeasure(uchar canAddr, TerminalStatus &status, bool bCall = true);
			//BMS充电数据召唤
			void bmsChargeCall(char *data, int len);
			//发送bms充电数据
			void sendBmsCharge(uchar canAddr, TerminalStatus &status, bool bCall = true);
			//发送bms参数
			void sendBmsParam(uchar canAddr, TerminalStatus &status);

			//下发多枪信息
			void setGunGroup(char *data, int len);
			//保存多枪信息
			bool saveGunGroup(InfoMap map);
			//上报多枪信息
			void gunGroupNotify(InfoMap map);
			//上报多枪信息应答
			void parseGunGroupAns(char *data, int len);
			//解析应急充电指令
			void parseEmergency(char *data, int len);
			//解析删除白名单
			void parseRmWhite(char *data, int len);
			//设置系统时间
			void setSystemTime(qint64 time);

			//未确认订单发送线程
			static void *billProc(void *p);
			//未确认订单召唤地址
			int billCanAddr;
			//未确认订单线程
			pthread_t billThread;

            bool bSendBillStart;  //未被确认订单流程开始标志位

			//发送网络帧
			bool sendFrame(char frameType, PBMessage *message);
    };
}

#endif
