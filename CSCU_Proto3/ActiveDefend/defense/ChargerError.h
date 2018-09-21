#ifndef CHARGERERROR_H
#define CHARGERERROR_H

#include "Defense.h"

class ChargerError : public Defense
{
	DECLARE_RUNTIME(ChargerError)

public:
	ChargerError();
	~ChargerError();

	virtual bool isTriggered(TerminalStatus &status);

	static Defense *createInstance();
};

#endif
