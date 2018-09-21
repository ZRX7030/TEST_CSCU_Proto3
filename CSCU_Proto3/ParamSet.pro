#-------------------------------------------------
#
# Project created by QtCreator 2016-10-26T23:30:49
#
#-------------------------------------------------

QT       -= gui

TARGET = ParamSet
TEMPLATE = lib

DEFINES += PARAMSET
include(CSCU_A1_Manage.pri)

INCLUDEPATH += \
		GeneralData/ \
		Log/ \
		ParamSet/ \
		Database/ \
                $${TELD_INCLUDEPATH}/include

SOURCES += \
	ParamSet/ParamSet.cpp \
	CommonFunc/commfunc.cpp	

HEADERS +=\
	GeneralData/104_info_struct.h \
	Infotag/CSCUBus.h \
	CommonFunc/commfunc.h \
	Log/Log.h
