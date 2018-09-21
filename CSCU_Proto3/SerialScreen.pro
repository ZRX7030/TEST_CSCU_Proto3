#-------------------------------------------------
#
# Project created by QtCreator 2016-10-26T23:30:49
#
#-------------------------------------------------

QT       -= gui

TARGET = SerialScreen
TEMPLATE = lib

DEFINES += SERIALSCREEN_LIBRARY
include(CSCU_A1_Manage.pri)

INCLUDEPATH += \
                                SerialPort/ \
                                SerialScreen/ \
                                GeneralData/ \
                                RealDataFilter/ \
                                Database/ \
                                $${TELD_INCLUDEPATH}/include/ \
                                Log/ \
                                InfoTag

SOURCES += \
    SerialPort/SerialPort.cpp \
    SerialScreen/SerialScreen.cpp \
    SerialScreen/SerialScreenProtocol.cpp \
	CommonFunc/commfunc.cpp

HEADERS +=\
    SerialPort/SerialPort.h \
    SerialScreen/ScreenDef.h \
    SerialScreen/SerialScreen.h \
    SerialScreen/SerialScreenProtocol.h \
    DevCache/DevCache.h \
    Infotag/CSCUBus.h \
    GeneralData/GeneralData.h \
    RealDataFilter/RealDataFilter.h \
	CommonFunc/commfunc.h
