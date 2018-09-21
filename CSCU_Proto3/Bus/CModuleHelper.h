#ifndef CMODULEHELPER_H
#define CMODULEHELPER_H

#include <QObject>
#include "Infotag/CSCUBus.h"
class CModuleHelper : public QObject
{
    Q_OBJECT
public:
    CModuleHelper();
    ~CModuleHelper();

     void sigHelper(InfoMap qInfoMap, InfoAddrType InfoType);
signals:
    void sigToBus(InfoMap qInfoMap, InfoAddrType InfoType);// 向BUS发送数据

};

#endif // CMODULEHELPER_H
