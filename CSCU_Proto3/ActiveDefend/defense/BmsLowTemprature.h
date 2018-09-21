#ifndef BMSLOWTEMPRATURE_H
#define BMSLOWTEMPRATURE_H

#include "Defense.h"

class BmsLowTemprature : public Defense
{
	DECLARE_RUNTIME(BmsLowTemprature)

public:
	BmsLowTemprature();
	~BmsLowTemprature();

	virtual bool isTriggered(TerminalStatus &status);

	static Defense *createInstance();
};

#endif
