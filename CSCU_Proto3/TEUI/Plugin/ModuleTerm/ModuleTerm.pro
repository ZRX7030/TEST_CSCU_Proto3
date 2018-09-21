CONFIG      += plugin debug_and_release
TARGET      = $$qtLibraryTarget(moduletermplugin)
TEMPLATE    = lib

HEADERS     = ModuleTermPlugin.h \
    ModuleTerm.h
SOURCES     = ModuleTermPlugin.cpp \
    ModuleTerm.cpp
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

include(ModuleTerm.pri)

FORMS += \
    ModuleTerm.ui
