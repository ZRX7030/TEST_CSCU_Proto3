#-------------------------------------------------
#
# Project created by QtCreator 2016-10-26T23:30:49
#
#-------------------------------------------------

QT       -= gui

TARGET = ScanCode
TEMPLATE = lib
include(CSCU_A1_Manage.pri)

DEFINES += ScanCode_LIBRARY

INCLUDEPATH += \
                                GeneralData/ \
                                RealDataFilter/ \
                                Database/ \
                                ScanCode/ \
                                $${TELD_INCLUDEPATH}/include/ \
                                Log/ \
                                InfoTag

LIBS += \
                -L$${TELD_LIBPATH} \
                -L$$OUT_PWD \
                -ljson-c

SOURCES += \
    ScanCode/ScanCode.cpp \
    ScanCode/DealScanCode.cpp \
    ScanCode/DES.cpp

HEADERS +=\
    ScanCode/ScanCode.h \
    ScanCode/DealScanCode.h \
    ScanCode/DES.h
