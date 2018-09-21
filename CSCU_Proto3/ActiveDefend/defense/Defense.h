#ifndef _DEFENSE_H
#define _DEFENSE_H

#include "GeneralData.h"
#include "DefenseHelper.h"

typedef enum
{
	LEVEL_1 = 1,
	LEVEL_2,
	LEVEL_COUNT
}DefendLevel;

typedef struct _DefendLevelInfo
{
	int result;
	char triggerTime[20];
	char defendTime[20];
}DefendLevelInfo;

typedef struct _TriggerdInfo
{
	int canAddr;
	int level;
	DefendLevelInfo *dfLevel[LEVEL_COUNT];
	Defense *df;
}TriggeredInfo;

class Defense
{
public:
	virtual ~Defense();

	virtual bool isTriggered(TerminalStatus &status) = 0;

	QString getDesc();

	virtual void alarm(uchar canAddr, uchar action);
	virtual void stopCharge(uchar canAddr);
	virtual void trip();
	virtual bool dfLog(TriggeredInfo *info);

protected:
	Defense();

	void writeLog(QString strLog, int level = 2);
	QString m_strDesc;
	uchar alarmCode;
};

#endif
