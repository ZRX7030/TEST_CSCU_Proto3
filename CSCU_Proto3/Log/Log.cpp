#include "Log/Log.h"
#include "Log/log4qt/propertyconfigurator.h"

Log::Log()
{
	Log4Qt::PropertyConfigurator::configure("/mnt/nandflash/log4j.properties");
}

Log::~Log()
{

}

Log *Log::GetInstance()
{
	static Log *ins = NULL;
	if(!ins){
		ins = new Log();
	}

	return ins;
}

Log4Qt::Logger *Log::getLogPoint(QString logName)
{
	Log4Qt::Logger *log = NULL;

	if(logName.isEmpty())
		return NULL;

	log = Log4Qt::Logger::logger(logName);

	return log;
}
