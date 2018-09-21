#ifndef CHARGERRELAY_H
#define CHARGERRELAY_H

#include "Defense.h"

class ChargerRelay : public Defense
{
	DECLARE_RUNTIME(ChargerRelay)

public:
	ChargerRelay();
	~ChargerRelay();

	virtual bool isTriggered(TerminalStatus &status);

	static Defense *createInstance();
private:
	int m_iPreTime;
	int m_iAlarmCnt;
};

#endif
