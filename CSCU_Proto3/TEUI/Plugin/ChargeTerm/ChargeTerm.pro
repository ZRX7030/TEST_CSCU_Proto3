CONFIG      += plugin debug_and_release
TARGET      = $$qtLibraryTarget(chargetermplugin)
TEMPLATE    = lib

HEADERS     = ChargeTermPlugin.h \
    ChargeTerm.h
SOURCES     = ChargeTermPlugin.cpp \
    ChargeTerm.cpp
RESOURCES   = \
    ../../TEUI/qrc/teui.qrc
LIBS        += -L. 

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += designer
} else {
    CONFIG += designer
}

target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS    += target

include(ChargeTerm.pri)

FORMS += \
    ChargeTerm.ui
