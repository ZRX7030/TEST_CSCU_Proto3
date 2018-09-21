QT       -=  gui


#TARGET = GPIO_335X
TARGET = GPIO
TEMPLATE = lib
include(CSCU_A1_Manage.pri)

INCLUDEPATH += \
                                GPIO_335X/ \
                                GeneralData/ \
                                RealDataFilter/ \
                                Database/ \
                                $${TELD_INCLUDEPATH}/include/ \
                                Log/ \
                                InfoTag/

HEADERS += \
        DevCache/DevCache.h \
    Infotag/CSCUBus.h \
    GeneralData/GeneralData.h \
    GeneralData/Bus.h \
    RealDataFilter/RealDataFilter.h \
    GPIO_335X/fl335x_drivers.h \
    GPIO_335X/io335x.h

SOURCES += \
    GPIO_335X/io335x.cpp
