#-------------------------------------------------
#
# Project created by QtCreator 2016-10-26T23:30:49
#
#-------------------------------------------------

QT       -= gui

TARGET = Update
TEMPLATE = lib
DEFINES += UPDATE

include(CSCU_A1_Manage.pri)

INCLUDEPATH += \
				GeneralData/ \
				Log/ 

#LIBS += /home/xuxiao/Code/CSCU_A1/CSCU_A1/Update/et299/et299.so
LIBS += -L$${TELD_LIBPATH} -ljson-c -lcurl

SOURCES += \
	Update/Update.cpp				\
	Update/HttpInterface.cpp		\
	Update/JsonInterface.cpp

HEADERS +=\
	Update/Update.h \
	GeneralData/104_info_struct.h \
	GeneralData/GeneralData.h \
	Infotag/CSCUBus.h 
