CONFIG      += designer plugin debug_and_release
TARGET      = $$qtLibraryTarget(tablewidgetkeyboardplugin)
TEMPLATE    = lib

HEADERS     = TableWidgetKeyboardPlugin.h \
    ../Keyboard/Keyboard.h
SOURCES     = TableWidgetKeyboardPlugin.cpp \
    ../Keyboard/Keyboard.cpp
RESOURCES   = icons.qrc
LIBS        += -L. 
INCLUDEPATH += ../Keyboard/
target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS    += target

include(TableWidgetKeyboard.pri)

FORMS += \
    ../Keyboard/Keyboard.ui
