#ifndef BMSOVERVOLTAGE_H
#define BMSOVERVOLTAGE_H

#include "Defense.h"

class BmsOverVoltage : public Defense
{
	DECLARE_RUNTIME(BmsOverVoltage)

public:
	BmsOverVoltage();
	~BmsOverVoltage();

	virtual bool isTriggered(TerminalStatus &status);

	static Defense *createInstance();
};

#endif
