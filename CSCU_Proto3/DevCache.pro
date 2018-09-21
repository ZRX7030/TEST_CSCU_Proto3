#-------------------------------------------------
#
# Project created by QtCreator 2016-10-26T23:30:49
#
#-------------------------------------------------

QT       -= gui

TARGET = DevCache
TEMPLATE = lib
DEFINES += DEVCACHE

include(CSCU_A1_Manage.pri)

INCLUDEPATH += \
				GeneralData/ \
				ParamSet/ \
				Database/ \
				DevCache \
                                $${TELD_INCLUDEPATH}/include \
				Log

SOURCES += \
	DevCache/DevCache.cpp

HEADERS +=\
	DevCache/DevCache.h \
	GeneralData/104_info_struct.h \
        GeneralData/GeneralData.h \
        Infotag/CSCUBus.h \
        Paramset.h \
        Log.h
