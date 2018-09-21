#ifndef DEFENSEHELPER_H
#define DEFENSEHELPER_H

#include <QString>
#include <stdio.h>

class Defense;

typedef Defense* (*CreateClass)();

class DefenseHelper
{
public:
	DefenseHelper(QString strClassName, CreateClass func);
};

#define DECLARE_RUNTIME(class_name)\
	QString class_name##Name;\
	static DefenseHelper *class_name##Helper;\
	class class_name##Release{\
		public:\
			~class_name##Release()\
			{\
				if(class_name::class_name##Helper){\
					delete class_name::class_name##Helper;\
				}\
			}\
	};\
	static class_name##Release release;

#define IMPLEMENT_RUNTIME(class_name) \
	DefenseHelper *class_name::class_name##Helper \
	= new DefenseHelper(#class_name, class_name::createInstance); \

#endif
