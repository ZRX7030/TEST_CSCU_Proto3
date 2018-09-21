#include "DefenseHelper.h"
#include "factory/DefenseFactory.h"

DefenseHelper::DefenseHelper(QString strClassName, CreateClass func)
{
	DefenseFactory::registDefense(strClassName, func);
}
