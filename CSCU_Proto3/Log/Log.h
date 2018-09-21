#ifndef			__LOG_H__
#define			__LOG_H__

#include "Log/log4qt/logger.h" 

/*模块调用选择*/

class Q_DECL_IMPORT Log
{
public:
	~Log();

	static Log *GetInstance();

	Log4Qt::Logger *getLogPoint(QString logName);

protected:
	Log();
};


#endif
