#-------------------------------------------------
#
# Project created by QtCreator 2016-09-28T09:16:35
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = CSCU_A1
CONFIG   += console
CONFIG   -= app_bundle

include(CSCU_A1_Manage.pri)

TEMPLATE =  subdirs

SUBDIRS += \
ParamSet.pro  \
Log.pro  \
DBOperate.pro  \
Bus.pro  \
DevCache.pro  \
RealDataFilter.pro  \
ChargeEquipment.pro  \
DeviceManage.pro \
ChargeService.pro  \
IEC104Server.pro  \
IEC104LocalServer.pro \
SerialScreen.pro \
LCDScreen.pro \
Ammeter.pro \
Update.pro \
LoadSchedule.pro \
TempHumi.pro \
Card.pro \
WebServer.pro \
ParkingLock.pro \
ScanCode.pro \
TicketDev.pro \
ActiveDefend.pro \
PbMonitorServer.pro \
PbChargeServer.pro \
NetAddress.pro \
CSCUApp.pro

equals(HWENV, "IM_9280"){
	SUBDIRS += GPIO.pro 
}else:equals(HWENV, "IM_335X"){
	SUBDIRS += GPIO_335X.pro \
			   TEUI.pro
}
CONFIG += creat_prl

generate.commands = chmod +x $$PWD/setup/create_setup.sh ; sh -x $$PWD/setup/create_setup.sh $$OUT_PWD $$PWD $${HWENV}
first.depends = $(first) generate
QMAKE_EXTRA_TARGETS += first generate
