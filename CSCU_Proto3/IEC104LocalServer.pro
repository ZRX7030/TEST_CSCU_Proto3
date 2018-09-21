#-------------------------------------------------
#
# Project created by QtCreator 2018-07-26T09:12:22
#
#-------------------------------------------------

QT       -= gui

TARGET = IEC104LocalServer
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
				IEC104LocalServer

SOURCES += \
    	IEC104LocalServer/IEC104LocalServer.cpp \
	CommonFunc/commfunc.cpp \
	Encrypt/3DES.cpp \
	Encrypt/aes.cpp \
	Encrypt/crc.cpp 

HEADERS +=\
    	IEC104LocalServer/IEC104LocalServer.h \
    	IEC104Server/IEC104Define.h \
	Encrypt/3DES.h \
	Encrypt/aes.h \
	Encrypt/crc.h \
	CommonFunc/commfunc.h \
	GeneralData/ModuleIO.h \
	GeneralData/Bus.h
