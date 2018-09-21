#-------------------------------------------------
#
# Project created by QtCreator 2016-11-27T18:30:30
#
#-------------------------------------------------

QT       -=  gui

TARGET = Card
TEMPLATE = lib

include(CSCU_A1_Manage.pri)

INCLUDEPATH += \
                                SerialPort/ \
                                CommonFunc/ \
                                DevCache/ \
                                GeneralData/ \
                                ParamSet/ \
                                Log/ \
                                Database/ \
                                $${TELD_INCLUDEPATH}/include/ \
                                Server/

SOURCES += \
                        Card/Card.cpp \
                        SerialPort/SerialPort.cpp

HEADERS += \
                        Card/Card.h \
                        SerialPort/SerialPort.h


