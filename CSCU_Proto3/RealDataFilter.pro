#-------------------------------------------------
#
# Project created by QtCreator 2016-10-26T23:30:49
#
#-------------------------------------------------

QT       -= gui

TARGET = RealDataFilter
TEMPLATE = lib

DEFINES += REALDATAFILTER
QMAKE_CXXFLAGS += -fno-strict-aliasing
include(CSCU_A1_Manage.pri)

INCLUDEPATH += \
		GeneralData/ \
		RealDataFilter/ \
		RealDataFilter/RealData \
		DevCache/ \
		ParamSet/ \
		Log/ \
		Database/ \
        $${TELD_INCLUDEPATH}/include

SOURCES += \
	RealDataFilter/RealDataFilter.cpp 

HEADERS +=\
	RealDataFilter/RealDataFilter.h \
	GeneralData/104_info_struct.h \
	Infotag/CSCUBus.h
