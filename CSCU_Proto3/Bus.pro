#-------------------------------------------------
#
# Project created by QtCreator 2016-10-26T23:30:49
#
#-------------------------------------------------

QT       -= gui

TARGET = Bus
TEMPLATE = lib

DEFINES += BUS
include(CSCU_A1_Manage.pri)

INCLUDEPATH += \
		GeneralData/ \
		ChargeEquipment \
		ChargeService \
		Database \
		DevCache \
		ParamSet \
		RealDataFilter \
		SerialPort \
		SerialScreen \
		LCDScreen \
		IEC104Server \
		Update \
		DeviceManage \
		Log \
                Card \
                Card \
                ParkingLock \
                $${TELD_INCLUDEPATH}/include

SOURCES += \
	Bus/Bus.cpp \
    Bus/CModuleHelper.cpp
HEADERS +=\
	Bus/Bus.h \
	Infotag/CSCUBus.h \
    Bus/CModuleHelper.h
