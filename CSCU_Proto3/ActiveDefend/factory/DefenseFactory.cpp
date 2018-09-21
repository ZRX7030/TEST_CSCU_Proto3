#include "DefenseFactory.h"

DefenseFactory::DefenseFactory()
{

}

DefenseFactory::~DefenseFactory()
{

}

QMap<QString, CreateClass> &DefenseFactory::getClassMap()
{
	static QMap<QString, CreateClass> mapClass;

	return mapClass;
}

void DefenseFactory::registDefense(QString strClassName, CreateClass func)
{
	QMap<QString, CreateClass> &mapClass = getClassMap();
	mapClass.insert(strClassName, func);
}

Defense* DefenseFactory::getDefenseByName(QString strClassName)
{
	QMap<QString, CreateClass> &mapClass = getClassMap();
	if(mapClass.contains(strClassName)){
		return mapClass[strClassName]();	
	}

	return NULL;
}

QList<Defense*> DefenseFactory::getDefenseList()
{
	QMap<QString, CreateClass> &mapClass = getClassMap();
	QMap<QString, CreateClass>::Iterator it;
	QList<Defense *> list;

	for(it = mapClass.begin(); it != mapClass.end(); ++it){
		list.append(it.value()());	
	}

	return list;
}
