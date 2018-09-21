#-------------------------------------------------
#
# Project created by QtCreator 2017-07-27T18:30:30
#
#-------------------------------------------------

QT       -=  gui

TARGET = TicketDev
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
                        TicketDev/TicketDev.cpp \
                        SerialPort/SerialPort.cpp

HEADERS += \
                        TicketDev/TicketDev.h \
                        SerialPort/SerialPort.h


