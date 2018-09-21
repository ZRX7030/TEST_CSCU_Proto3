#-------------------------------------------------
#
# Project created by QtCreator 2016-10-26T23:30:49
#
#-------------------------------------------------

QT       -= gui

TARGET = TempHumi
TEMPLATE = lib

DEFINES += TempHumi_LIBRARY
include(CSCU_A1_Manage.pri)

INCLUDEPATH += \
                                GeneralData/ \
                                RealDataFilter/ \
                                Database/ \
                                $${TELD_INCLUDEPATH}/include/ \
                                Log/ \
                                InfoTag/\
                                TempHumi
SOURCES += \
    TempHumi/TempHumi.cpp

HEADERS +=\
    DevCache/DevCache.h \
    RealDataFilter/RealDataFilter.h \
    TempHumi/TempHumi.h
