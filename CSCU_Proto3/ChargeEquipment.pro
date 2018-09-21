#-------------------------------------------------
#
# Project created by QtCreator 2016-10-26T23:30:49
#
#-------------------------------------------------

QT       -= gui

TARGET = ChargeEquipment
TEMPLATE = lib

include(CSCU_A1_Manage.pri)

INCLUDEPATH += \
				ChargeEquipment/TerminalProtocol/J1939Protocol/ \
				ChargeEquipment/TerminalProtocol/ \
				ChargeEquipment/ \
				GeneralData/ \
				RealDataFilter/ \
				Database/ \
                                $${TELD_INCLUDEPATH}/include/ \
				Log/ \
				InfoTag/ 

SOURCES += \
   	ChargeEquipment/ChargeEquipment.cpp \
    ChargeEquipment/Can/CanSocket.cpp \
    ChargeEquipment/Can/CanBus.cpp \
    ChargeEquipment/TerminalProtocol/TerminalProtocol.cpp \
    ChargeEquipment/TerminalProtocol/J1939Protocol/J1939GeneralProtocol.cpp \
    ChargeEquipment/TerminalProtocol/J1939Protocol/DCCanProtocol.cpp \
    ChargeEquipment/TerminalProtocol/J1939Protocol/ACThrCanProtocol.cpp \
    ChargeEquipment/TerminalProtocol/J1939Protocol/ACSinCanProtocol.cpp \
    ChargeEquipment/TerminalProtocol/J1939Protocol/EnergyPlanProtocol.cpp \
    ChargeEquipment/TerminalProtocol/J1939Protocol/CSCUCanProtocol.cpp
	
HEADERS +=\
	ChargeEquipment/ChargeEquipment.h \
    ChargeEquipment/Can/CanSocket.h \
    ChargeEquipment/Can/CanBus.h \
    ChargeEquipment/TerminalProtocol/TerminalProtocol.h \
    ChargeEquipment/TerminalProtocol/J1939Protocol/J1939GeneralProtocol.h \
    ChargeEquipment/TerminalProtocol/J1939Protocol/DCCanProtocol.h \
    ChargeEquipment/TerminalProtocol/J1939Protocol/ACThrCanProtocol.h \
    ChargeEquipment/TerminalProtocol/J1939Protocol/ACSinCanProtocol.h \
	DevCache/DevCache.h \
    Infotag/CSCUBus.h \
    GeneralData/GeneralData.h \
    RealDataFilter/RealDataFilter.h \
    ChargeEquipment/TerminalProtocol/J1939Protocol/EnergyPlanProtocol.h \
    ChargeEquipment/TerminalProtocol/J1939Protocol/CSCUCanProtocol.h
