#ifndef BMSFAULT_H
#define BMSFAULT_H

#include "Defense.h"

class BmsFault : public Defense
{
	DECLARE_RUNTIME(BmsFault)

public:
	BmsFault();
	~BmsFault();

	virtual bool isTriggered(TerminalStatus &status);

	static Defense *createInstance();
};

#endif
