#-------------------------------------------------
#
# Project created by QtCreator 2016-10-26T23:30:49
#
#-------------------------------------------------

QT       -= gui

TARGET = NetAddress
TEMPLATE = lib
include(CSCU_A1_Manage.pri)

INCLUDEPATH += \
				Infotag \
				Encrypt \
				CommonFunc \
				DevCache \
				RealDataFilter \
				GeneralData \
                $${TELD_INCLUDEPATH}/include \
				Database \
				ParamSet \
				Log \
				IEC104Server

SOURCES += \
    NetAddress/NetAddress.cpp \

HEADERS +=\
    NetAddress/NetAddress.h \
	GeneralData/ModuleIO.h \
	GeneralData/Bus.h
