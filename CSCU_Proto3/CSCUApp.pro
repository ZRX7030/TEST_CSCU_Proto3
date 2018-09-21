#-------------------------------------------------
#
# Project created by QtCreator 2016-09-28T09:16:35
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = CSCU_A1
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

include(CSCU_A1_Manage.pri)

INCLUDEPATH += \
		CSCUApp \
		Infotag \
        GeneralData \
		DevCache \
        Log \
		Bus \
        RealDataFilter  \
        $${TELD_INCLUDEPATH}/include \
        Database \
        ParamSet

LIBS += \
    -L$${TELD_LIBPATH} \
    -L$$DESTDIR \
    -lLog \
    -lsqlite3 \
    -lDBOperate \
    -lParamSet \
    -lRealDataFilter \
    -lDevCache \
    -lBus \

SOURCES += \
  	CSCUApp/main.cpp \
  	CSCUApp/CSCUApp.cpp

HEADERS += \
	CSCUApp/CSCUApp.h \
    DevCache/DevCache.h \
    RealDataFilter/RealDataFilter.h \
    GeneralData/ModuleIO.h \
    Bus/Bus.h  \
    ParamSet/ParamSet.h \
    Database/DBOperate.h \
    Log/Log.h
