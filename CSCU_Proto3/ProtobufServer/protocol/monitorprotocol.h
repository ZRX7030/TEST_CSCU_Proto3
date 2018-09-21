#ifndef MONITORROTOCOL_H
#define MONITORPROTOCOL_H

#include "protocol.h"
#include "monitorprotobuf.pb.h"
#include <google/protobuf/message.h>

namespace Monitor{
	using namespace monitor::protobuf;
	class MonitorProtocol : public Protocol
	{
		public:
			MonitorProtocol(Net *n);
			~MonitorProtocol();

			//数据帧解析
			bool parse(short type, char *buff, int len);
			//内部模块指令
			bool command(InfoMap &map, InfoAddrType &type);
			//突发
			bool burst(QDataPointList burst);
			//发送网络帧
			bool sendFrame(char frameType, PBMessage *message);
			//上线
			virtual bool login();
			//离线
			virtual void logout(int type = 0);
			//心跳
			virtual void heart();

		private:
			//定时器
			void timerEvent(QTimerEvent *event);
			//解析上相回应
			void parseLogin(char *data, int len);
			//解析心跳回应
			void parseHeart(char *data, int len);
			//解析告警回应
			void parseAlarmAns(char *data, int len);
			//遥测数据召唤
			void measureCall(char *data, int len);
			//状态数据召唤
			void stateCall(char *data, int len);
			//遥信数据召唤
			void signalCall(char *data, int len);
			//遥测数据通知
			void measureNotify(QDataPointList list);
			//状态数据通知
			void stateNotify(QDataPointList list);
			//遥信数据通知
			void signalNotify(QDataPointList list);
			//报警数据通知
			void alarmNotify(QDataPointList list);
			//订单报警通知
			void orderAlarm(bool burst = true);
			//遥控指令
			void control();

			int timerAlarm;
	};
}

#endif
