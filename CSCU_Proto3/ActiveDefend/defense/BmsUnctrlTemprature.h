#ifndef BMSUNCTRLTEMPRATURE_H
#define BMSUNCTRLTEMPRATURE_H

#include "Defense.h"

class BmsUnctrlTemprature : public Defense
{
	DECLARE_RUNTIME(BmsUnctrlTemprature)

public:
	BmsUnctrlTemprature();
	~BmsUnctrlTemprature();

	virtual bool isTriggered(TerminalStatus &status);

	static Defense *createInstance();

private:
	int m_iPreTime;
	int m_iPreTemprature;
};

#endif
