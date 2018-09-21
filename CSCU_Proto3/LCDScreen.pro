#-------------------------------------------------
#
# Project created by QtCreator 2016-10-26T23:30:49
#
#-------------------------------------------------

QT       -= gui
#QT      += net

TARGET = LCDScreen
TEMPLATE = lib

DEFINES += LCDSCREEN_LIBRARY
include(CSCU_A1_Manage.pri)

INCLUDEPATH += \
                                GeneralData/ \
                                RealDataFilter/ \
                                Database/ \
                                $${TELD_INCLUDEPATH}/include/ \
                                Log/ \
                                InfoTag

SOURCES += \
    LCDScreen/LCDScreen.cpp \
    LCDScreen/LCDScreenProtocol.cpp


HEADERS +=\
    DevCache/DevCache.h \
    Infotag/CSCUBus.h \
    GeneralData/GeneralData.h \
    RealDataFilter/RealDataFilter.h \
    LCDScreen/LCDScreen.h \
    LCDScreen/LCDScreenProtocol.h
