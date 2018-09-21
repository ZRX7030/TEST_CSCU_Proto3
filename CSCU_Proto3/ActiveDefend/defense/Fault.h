#ifndef Fault_H
#define Fault_H

#include "Defense.h"

class Fault : public Defense
{
	DECLARE_RUNTIME(Fault)

public:
	Fault();
	~Fault();

	virtual bool isTriggered(TerminalStatus &status);

	static Defense *createInstance();
};

#endif
