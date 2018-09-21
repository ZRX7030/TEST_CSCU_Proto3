#ifndef BMSOVERCHARGE_H
#define BMSOVERCHARGE_H

#include "Defense.h"

class BmsOverCharge : public Defense
{
	DECLARE_RUNTIME(BmsOverCharge)

public:
	BmsOverCharge();
	~BmsOverCharge();

	virtual bool isTriggered(TerminalStatus &status);

	static Defense *createInstance();

private:
	int m_iPreTime;
	double m_fPreEnergy;
	double m_fLeftEnergy;
};

#endif
