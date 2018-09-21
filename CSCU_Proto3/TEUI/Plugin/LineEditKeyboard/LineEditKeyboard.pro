CONFIG      += designer plugin debug_and_release
TARGET      = $$qtLibraryTarget(lineeditkeyboardplugin)
TEMPLATE    = lib

HEADERS     = LineEditKeyboardplugin.h \
    ../Keyboard/Keyboard.h
SOURCES     = LineEditKeyboardplugin.cpp \
    ../Keyboard/Keyboard.cpp
RESOURCES   = icons.qrc
LIBS        += -L. 

target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS    += target

include(LineEditKeyboard.pri)
INCLUDEPATH +=../Keyboard/

FORMS += \
    ../Keyboard/Keyboard.ui
