#ifndef BMSERROR_H
#define BMSERROR_H

#include "Defense.h"

class BmsError : public Defense
{
	DECLARE_RUNTIME(BmsError)

public:
	BmsError();
	~BmsError();

	virtual bool isTriggered(TerminalStatus &status);
	static Defense *createInstance();
};

#endif
