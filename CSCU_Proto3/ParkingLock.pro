#-------------------------------------------------
#
# Project created by QtCreator 2017-06-24T10:13:30
#
#-------------------------------------------------

QT       -=  gui

TARGET = ParkingLock
TEMPLATE = lib

include(CSCU_A1_Manage.pri)

#DESTDIR = $$PWD/bin
#MOC_DIR = $$PWD/build_$$TARGET
#RCC_DIR = $$PWD/build_$$TARGET
#UI_DIR = $$PWD/build_$$TARGET
#OBJECTS_DIR = $$PWD/build_$$TARGET

INCLUDEPATH += \
                                ParkingLock/Can1/ \
                                ParkingLock/ \
                                CommonFunc/ \
                                DevCache/ \
                                GeneralData/ \
                                ParamSet/ \
                                Log/ \
                                Database/ \
                                $${TELD_INCLUDEPATH}/include/ \
                                Server/

SOURCES += \
                       ParkingLock/ParkingLock.cpp \
                       ParkingLock/ParkingLockProtocol.cpp \
                       ParkingLock/Can1/Can1Socket.cpp \
                       ParkingLock/Can1/Can1Bus.cpp


HEADERS += \
                     ParkingLock/ParkingLock.h \
                     ParkingLock/Can1/Can1Socket.h \
                     ParkingLock/ParkingLockProtocol.h \
                     ParkingLock/Can1/Can1Bus.h


