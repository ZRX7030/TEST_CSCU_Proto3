#-------------------------------------------------
#
# Project created by QtCreator 2016-10-26T23:30:49
#
#-------------------------------------------------

QT       -= gui

TARGET = IEC104Server
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
    IEC104Server/IEC104Server.cpp \
	CommonFunc/commfunc.cpp \
	Encrypt/3DES.cpp \
	Encrypt/aes.cpp \
	Encrypt/crc.cpp 

HEADERS +=\
    IEC104Server/IEC104Server.h \
    IEC104Server/IEC104Define.h \
	Encrypt/3DES.h \
	Encrypt/aes.h \
	Encrypt/crc.h \
	CommonFunc/commfunc.h \
	GeneralData/ModuleIO.h \
	GeneralData/Bus.h
