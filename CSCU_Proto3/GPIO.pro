QT       -=  gui


TARGET = GPIO
TEMPLATE = lib
include(CSCU_A1_Manage.pri)

INCLUDEPATH += \
                                GPIO/ \
                                GeneralData/ \
                                RealDataFilter/ \
                                Database/ \
                                $${TELD_INCLUDEPATH}/include/ \
                                Log/ \
                                InfoTag/

HEADERS += \
    GPIO/io.h \
        DevCache/DevCache.h \
    Infotag/CSCUBus.h \
    GeneralData/GeneralData.h \
    GeneralData/Bus.h \
    RealDataFilter/RealDataFilter.h \
    GPIO/em9280_drivers.h

SOURCES += \
    GPIO/io.cpp
