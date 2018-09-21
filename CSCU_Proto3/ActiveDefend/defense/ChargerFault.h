#ifndef CHARGERFAULT_H
#define CHARGERFAULT_H

#include "Defense.h"

class ChargerFault : public Defense
{
	DECLARE_RUNTIME(ChargerFault)

public:
	ChargerFault();
	~ChargerFault();

	virtual bool isTriggered(TerminalStatus &status);

	static Defense *createInstance();
};

#endif
