#ifndef BMSREPEATINFO_H
#define BMSREPEATINFO_H

#include "Defense.h"

class BmsRepeatInfo : public Defense
{
	DECLARE_RUNTIME(BmsRepeatInfo)

		typedef struct _repeat_info{
			float voltage;
			float current;
			float single_voltage;
			short temprature;
			uchar soc;
			short left_time;
		}repeat_info;
public:
	BmsRepeatInfo();
	~BmsRepeatInfo();

	virtual bool isTriggered(TerminalStatus &status);

	static Defense *createInstance();

private:    
	int m_iChangeCnt;
	int m_iCheckCnt;
	int m_iPreTime;
	repeat_info m_stPreInfo;
};

#endif
