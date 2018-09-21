#-------------------------------------------------
#
# Project created by QtCreator 2016-10-26T23:30:49
#
#-------------------------------------------------

QT       -= gui

TARGET = Log
TEMPLATE = lib

DEFINES += LOG
include(CSCU_A1_Manage.pri)

include(./Log/log4qt/log4qt.pri)


INCLUDEPATH += Log

SOURCES += \
	Log/Log.cpp

HEADERS +=\
	Log/Log.h
