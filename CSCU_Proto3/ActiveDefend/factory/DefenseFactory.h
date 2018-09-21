#ifndef DEFENSEFACTORY_H
#define DEFENSEFACTORY_H

#include <QString>
#include <QMap>
#include "../defense/Defense.h"

class DefenseFactory
{
public:	
	DefenseFactory();
	~DefenseFactory();

	static void registDefense(QString strClassName, CreateClass func);
	static Defense* getDefenseByName(QString strClassName);
	static QList<Defense*> getDefenseList();

private:
	static QMap<QString, CreateClass> &getClassMap();
};

#endif
