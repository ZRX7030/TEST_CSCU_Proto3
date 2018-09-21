#-------------------------------------------------
#
# Project created by QtCreator 2016-10-26T23:30:49
#
#-------------------------------------------------

QT       -= gui

TARGET = DeviceManage
TEMPLATE = lib

DESTDIR = $$PWD/bin
MOC_DIR = $$PWD/build_$$TARGET
RCC_DIR = $$PWD/build_$$TARGET
UI_DIR = $$PWD/build_$$TARGET
OBJECTS_DIR = $$PWD/build_$$TARGET
DEFINES += DEVICEMANAGE_LIBRARY

include(CSCU_A1_Manage.pri)

INCLUDEPATH += \
                                GeneralData/ \
                                RealDataFilter/ \
                                Database/ \
                                $${TELD_INCLUDEPATH}/include/ \
                                Log/ \
                                InfoTag

SOURCES += \
    DeviceManage/DeviceManage.cpp

HEADERS += \
    DeviceManage/DeviceManage.h

