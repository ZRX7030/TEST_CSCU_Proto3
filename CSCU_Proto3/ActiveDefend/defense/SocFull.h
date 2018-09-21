#ifndef SOCFULL_H
#define SOCFULL_H

#include "Defense.h"

class SocFull : public Defense
{
	DECLARE_RUNTIME(SocFull)

public:
	SocFull();
	~SocFull();

	virtual bool isTriggered(TerminalStatus &status);

	static Defense *createInstance();
private:
	int m_iPreTime;
};

#endif
