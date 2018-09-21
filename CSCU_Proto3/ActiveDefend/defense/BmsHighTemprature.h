#ifndef BMSHIGHTEMPRATURE_H
#define BMSHIGHTEMPRATURE_H

#include "Defense.h"

class BmsHighTemprature : public Defense
{
	DECLARE_RUNTIME(BmsHighTemprature)

public:
	BmsHighTemprature();
	~BmsHighTemprature();

	virtual bool isTriggered(TerminalStatus &status);

	static Defense *createInstance();
};

#endif
