#ifndef MODULEIO_H
#define MODULEIO_H

#include <QThread>
#include "Infotag/CSCUBus.h"

class Q_DECL_IMPORT CModuleIO : public QObject
{
public:
    CModuleIO()
	{
		m_pWorkThread = NULL;
		_strLogName = "";
	}

	virtual ~CModuleIO()
	{
	
	}

public:
	//模块初始化
    virtual int InitModule(QThread* pThread) = 0;
	
	//注册模块
    virtual int RegistModule() = 0;

	//启动模块
    virtual int StartModule() = 0;

	//停止模块
    virtual int StopModule() = 0;

	//模块工作状态
    virtual int ModuleStatus() = 0;

protected:
	QThread* m_pWorkThread;
	QString  _strLogName;

signals:
    void sigToBus(InfoMap mapInfo, InfoAddrType type);

public slots:
    void slotFromBus(InfoMap mapInfo, InfoAddrType type);

};

//创建模块实例并导出。函数为全局函数，需要在子类模块文件中实现该函数
//参数说明：
//pDepends：按照固定顺序存储公用模块指针，顺序定义为Common_Module_Index
//argc: 表示pDepends存储指针的数量
extern "C" Q_DECL_IMPORT CModuleIO* CreateDevInstance();

//释放模块实例。函数为全局函数,需要在子类模块文件中实现该函数
extern "C" Q_DECL_IMPORT void DestroyDevInstance(CModuleIO* pModule);

#endif // MODULEIO_H
