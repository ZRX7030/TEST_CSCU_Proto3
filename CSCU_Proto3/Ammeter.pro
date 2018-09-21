#-------------------------------------------------
#
# Project created by QtCreator 2016-11-05T17:36:21
#
#-------------------------------------------------

QT       -= gui


TARGET = Ammeter
TEMPLATE = lib

include(CSCU_A1_Manage.pri)

INCLUDEPATH += \
                                Ammeter/ammeterData/ \
                                Ammeter/ \
                                Ammeter/Protocol/ \
                                GeneralData/ \
                                RealDataFilter/ \
                                Database/ \
                                $${TELD_INCLUDEPATH}/include/ \
                                Log/ \
                                InfoTag/

SOURCES += \
    Ammeter/ammeterData.cpp \
    Ammeter/Protocol/dlt645_07.cpp \
    Ammeter/Protocol/dlt645_97.cpp \
    Ammeter/Protocol/modbus.cpp

HEADERS  += Ammeter/Protocol/dlt645_97.h \
    Ammeter/ammeterData.h \
    Ammeter/Protocol/modbus.h \
    Ammeter/Protocol/dlt645_07.h \
        DevCache/DevCache.h \
    Infotag/CSCUBus.h \
    GeneralData/GeneralData.h \
    GeneralData/Bus.h \
    RealDataFilter/RealDataFilter.h
