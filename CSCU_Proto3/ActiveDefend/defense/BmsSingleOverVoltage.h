#ifndef BMSSINGLEOVERVOLTAGE_H
#define BMSSINGLEOVERVOLTAGE_H

#include "Defense.h"

class BmsSingleOverVoltage : public Defense
{
	DECLARE_RUNTIME(BmsSingleOverVoltage)

public:
	BmsSingleOverVoltage();
	~BmsSingleOverVoltage();

	virtual bool isTriggered(TerminalStatus &status);

	static Defense *createInstance();
};

#endif
