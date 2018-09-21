CONFIG      += plugin debug_and_release
TARGET      = $$qtLibraryTarget(alertdialogplugin)
TEMPLATE    = lib

HEADERS     = AlertDialogplugin.h \
    AlertDialog.h
SOURCES     = AlertDialogplugin.cpp \
    AlertDialog.cpp
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

include(AlertDialog.pri)

FORMS += \
    AlertDialog.ui
