QT       -= gui


TARGET = LoadSchedule
TEMPLATE = lib
include(CSCU_A1_Manage.pri)

INCLUDEPATH += \
                                LoadSchedule/ \
                                GeneralData/ \
                                RealDataFilter/ \
                                Database/ \
                                $${TELD_INCLUDEPATH}/include/ \
                                Log/ \
                                InfoTag/

HEADERS += \
    LoadSchedule/loadSchedule.h \
    LoadSchedule/powerLimit.h \
    LoadSchedule/smartcharge.h \
    LoadSchedule/peakshaving.h \
    LoadSchedule/vechiclepriority.h

SOURCES += \
    LoadSchedule/loadSchedule.cpp \
    LoadSchedule/powerLimit.cpp \
    LoadSchedule/smartcharge.cpp \
    LoadSchedule/peakshaving.cpp \
    LoadSchedule/vechiclepriority.cpp
