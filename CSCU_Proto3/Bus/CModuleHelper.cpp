#include "CModuleHelper.h"

CModuleHelper::CModuleHelper()
{
}

CModuleHelper::~CModuleHelper()
{

}
void CModuleHelper::sigHelper(InfoMap qInfoMap, InfoAddrType InfoType)
{
    emit sigToBus(qInfoMap,InfoType);
}
