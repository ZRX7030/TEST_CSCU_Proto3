#-------------------------------------------------
#
# Project created by QtCreator 2016-10-26T23:30:49
#
#-------------------------------------------------

QT       -= gui

TARGET = ActiveDefend
TEMPLATE = lib

include(CSCU_A1_Manage.pri)

INCLUDEPATH += \
				ActiveDefend \
				Infotag \
				Encrypt \
				CommonFunc \
				DevCache \
				RealDataFilter \
				GeneralData \
				FileSystemFiles/include \
				Database \
				ParamSet \
				Log

SOURCES += \
    ActiveDefend/defense/ChargerRelay.cpp \
    ActiveDefend/defense/BmsRepeatInfo.cpp \
    ActiveDefend/defense/BmsOverCurrent.cpp \
    ActiveDefend/defense/BmsOverVoltage.cpp \
    ActiveDefend/defense/BmsSingleOverVoltage.cpp \
    ActiveDefend/defense/BmsOverCharge.cpp \
    ActiveDefend/defense/BmsUnctrlTemprature.cpp \
    ActiveDefend/defense/BmsLowTemprature.cpp \
    ActiveDefend/defense/BmsHighTemprature.cpp \
    ActiveDefend/defense/SocFull.cpp \
    ActiveDefend/defense/Fault.cpp \
    ActiveDefend/defense/ChargerError.cpp \
    ActiveDefend/defense/ChargerFault.cpp \
    ActiveDefend/defense/BmsError.cpp \
    ActiveDefend/defense/BmsFault.cpp \
    ActiveDefend/defense/DefenseHelper.cpp \
    ActiveDefend/defense/Defense.cpp \
	ActiveDefend/factory/DefenseFactory.cpp \
    ActiveDefend/core/ActiveDefend.cpp 

HEADERS +=\
    ActiveDefend/defense/ChargerRelay.h \
    ActiveDefend/defense/BmsRepeatInfo.h \
    ActiveDefend/defense/BmsOverCurrent.h \
    ActiveDefend/defense/BmsOverVoltage.h \
    ActiveDefend/defense/BmsSingleOverVoltage.h \
    ActiveDefend/defense/BmsOverCharge.h \
    ActiveDefend/defense/BmsUnctrlTemprature.h \
    ActiveDefend/defense/BmsLowTemprature.h \
    ActiveDefend/defense/BmsHighTemprature.h \
    ActiveDefend/defense/SocFull.h \
    ActiveDefend/defense/Fault.h \
    ActiveDefend/defense/ChargerError.h \
    ActiveDefend/defense/ChargerFault.h \
    ActiveDefend/defense/BmsError.h \
    ActiveDefend/defense/BmsFault.h \
    ActiveDefend/defense/DefenseHelper.h \
    ActiveDefend/defense/Defense.h \
	ActiveDefend/factory/DefenseFactory.h \
    ActiveDefend/core/ActiveDefend.h
