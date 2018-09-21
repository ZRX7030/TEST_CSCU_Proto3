#ifndef BMSOVERCURRENT_H
#define BMSOVERCURRENT_H

#include "Defense.h"

class BmsOverCurrent : public Defense
{
	DECLARE_RUNTIME(BmsOverCurrent)

public:
	BmsOverCurrent();
	~BmsOverCurrent();

	virtual bool isTriggered(TerminalStatus &status);

	static Defense *createInstance();
};

#endif
