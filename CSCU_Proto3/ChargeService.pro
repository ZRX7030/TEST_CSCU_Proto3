#-------------------------------------------------
#
# Project created by QtCreator 2016-10-26T23:30:49
#
#-------------------------------------------------

QT       -= gui
#QT       += sql

TARGET = ChargeService
TEMPLATE = lib

include(CSCU_A1_Manage.pri)

INCLUDEPATH += \
				ChargeEquipment/TerminalProtocol/J1939Protocol/ \
				Encrypt/ \
				CommonFunc/ \
				DevCache/ \
				GeneralData/ \
				ParamSet/ \
                Log/ \
                Database/ \
                $${TELD_INCLUDEPATH}/include/ \
				Server/

SOURCES += \
		ChargeService/ChargeService.cpp \
    	ChargeService/CardChargeFun.cpp \
		CommonFunc/commfunc.cpp \ 
    ChargeService/multigunchargeservice.cpp \
    ChargeService/ChargeServiceSon.cpp

HEADERS +=\
		ChargeService/ChargeService.h \
    	ChargeService/CardChargeFun.h \
		CommonFunc/commfunc.h \
    ChargeService/multigunchargeservice.h \
    ChargeService/ChargeServiceSon.h
