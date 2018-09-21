#-------------------------------------------------
#
# Project created by QtCreator 2016-11-28T09:23:40
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TEUI
TEMPLATE = app

include(CSCU_A1_Manage.pri)

INCLUDEPATH += \
	TEUI/ \
	TEUI/src/ \
	TEUI/Plugin/ChargeTerm/ \
	TEUI/Plugin/Keyboard/ \
	TEUI/Plugin/LineEditKeyboard/ \
	TEUI/Plugin/TableWidgetKeyboard/ \
	TEUI/Plugin/ModuleTerm/ \
	TEUI/Plugin/TimeLimit/ \
	TEUI/src/SocketClient/ \
	TEUI/src/Protocol/ \
	TEUI/src/Charge/ \
	TEUI/src/Common/ \
	TEUI/src/Maintain/ \
	TEUI/src/Maintain/ParamSet/ \
	TEUI/src/Maintain/RealData/ \
	TEUI/src/Maintain/HistoryData/ \
	TEUI/src/Maintain/VersionInfo/ \
	TEUI/src/Update/

SOURCES += \
    TEUI/src/main.cpp \
    TEUI/src/Maintain/Maintain.cpp \
    TEUI/src/Maintain/ParamSet/MaintainCSCUSet.cpp \
    TEUI/src/TeuiMainWindow.cpp \
    TEUI/src/SocketClient/Sockets.cpp \
    TEUI/src/Protocol/TeuiProtocol.cpp \
    TEUI/src/Protocol/Bus.cpp \
    TEUI/src/Maintain/HistoryData/ChargingRecord.cpp \
    TEUI/src/Maintain/RealData/TerminalInformation.cpp \
    TEUI/src/Maintain/RealData/LineSideInformation.cpp \
    TEUI/src/Maintain/RealData/BMSInformation.cpp \
    TEUI/src/Maintain/HistoryData/FailureInformation.cpp \
    TEUI/src/Charge/TerminalChargingInformation.cpp \
    TEUI/src/Maintain/ParamSet/PeakChargingInformationView.cpp \
    TEUI/src/Maintain/RealData/TerminalInformationSurvey.cpp \
    TEUI/src/Maintain/ParamSet/PasswordSet.cpp \
    TEUI/src/Maintain/ParamSet/ActriphaseSet.cpp \
    TEUI/src/Maintain/ParamSet/SpecialFeatureSet.cpp \
    TEUI/src/Maintain/ParamSet/DcTerminalSpeFeaSet.cpp \
    TEUI/src/Maintain/ParamSet/LoadConstraintSet.cpp \
    TEUI/src/Charge/MainCharge.cpp \
    TEUI/Plugin/ChargeTerm/ChargeTerm.cpp \
    TEUI/Plugin/Keyboard/Keyboard.cpp \
    TEUI/Plugin/LineEditKeyboard/LineEditKeyboard.cpp \
    TEUI/Plugin/TableWidgetKeyboard/TableWidgetKeyboard.cpp \
    TEUI/src/Maintain/VersionInfo/VersionInformation.cpp \
    TEUI/src/Maintain/RealData/SubStationEnMonitoringInformation.cpp \
    TEUI/src/Maintain/PasswordLogin.cpp \
    TEUI/src/Update/ExportData.cpp \
    TEUI/src/Common/StatusRemindWindow.cpp \
    TEUI/src/Charge/ChargeReportFinish.cpp \
    TEUI/src/Charge/ChargingReport.cpp \
    TEUI/Plugin/ModuleTerm/ModuleTerm.cpp \
    TEUI/src/Maintain/RealData/ModuleInformation.cpp \
    TEUI/src/Maintain/RealData/ModuleInformationSurvey.cpp \
    TEUI/src/Maintain/HistoryData/OperateRecordInformation.cpp \
    TEUI/src/Charge/ApplayCharge.cpp \
    TEUI/src/Charge/ChagingFailureInformation.cpp \
    TEUI/src/Charge/ChargeTypeSelect.cpp \
    TEUI/src/Charge/FlashWaitWidget.cpp \
    TEUI/Plugin/TimeLimit/TimeLimit.cpp \
    TEUI/src/Charge/ChargeMangeBase.cpp \
    TEUI/src/Update/FlashExportLog.cpp \
    TEUI/src/Charge/VINStartCharging.cpp \
    TEUI/src/Maintain/ParamSet/QRcodeCreate.cpp \
    TEUI/src/Maintain/ParamSet/DCChargerSet.cpp \
    TEUI/src/Maintain/RealData/DCChargerData.cpp \
    TEUI/src/Maintain/ParamSet/SystemTimeSet.cpp \
    TEUI/src/Maintain/ParamSet/StartTypeSelect.cpp \
    TEUI/src/Maintain/ParamSet/ChargeModeSelect.cpp \
    TEUI/src/Maintain/ParamSet/ChargePassword.cpp \
    TEUI/src/Charge/PasswordCharge.cpp \
    TEUI/src/Maintain/ParamSet/LogoSet.cpp \
    TEUI/src/Maintain/RealData/RealtimeFaultInformation.cpp \
    TEUI/src/Maintain/ParamSet/CoupleGunSet.cpp \
	TEUI/src/Maintain/VersionInfo/LanguageSelect.cpp \
    TEUI/src/Charge/PrintPaper.cpp

HEADERS  += \
    TEUI/src/Maintain/Maintain.h \
    TEUI/src/Maintain/ParamSet/MaintainCSCUSet.h \
    TEUI/src/TeuiMainWindow.h \
    TEUI/src/SocketClient/Sockets.h \
    TEUI/src/Protocol/TeuiProtocol.h \
    TEUI/src/Protocol/Bus.h \
    TEUI/src/Common/Common.h \
    TEUI/src/Maintain/HistoryData/ChargingRecord.h \
    TEUI/src/Protocol/SwapBase.h \
    TEUI/src/Common/InfoData.h \
    TEUI/src/Protocol/ProtocolBase.h \
    TEUI/src/Maintain/RealData/TerminalInformation.h \
    TEUI/src/Maintain/RealData/LineSideInformation.h \
    TEUI/src/Maintain/RealData/BMSInformation.h \
    TEUI/src/Maintain/HistoryData/FailureInformation.h \
    TEUI/src/Charge/TerminalChargingInformation.h \
    TEUI/src/Maintain/ParamSet/PeakChargingInformationView.h \
    TEUI/src/Maintain/RealData/TerminalInformationSurvey.h \
    TEUI/src/Maintain/ParamSet/PasswordSet.h \
    TEUI/src/Maintain/ParamSet/ActriphaseSet.h \
    TEUI/src/Maintain/ParamSet/SpecialFeatureSet.h \
    TEUI/src/Maintain/ParamSet/DcTerminalSpeFeaSet.h \
    TEUI/src/Maintain/ParamSet/LoadConstraintSet.h \
    TEUI/src/Charge/MainCharge.h \
    TEUI/Plugin/ChargeTerm/ChargeTerm.h \
    TEUI/Plugin/Keyboard/Keyboard.h \
    TEUI/Plugin/LineEditKeyboard/LineEditKeyboard.h \
    TEUI/Plugin/TableWidgetKeyboard/TableWidgetKeyboard.h \
    TEUI/src/Maintain/VersionInfo/VersionInformation.h \
    TEUI/src/Maintain/RealData/SubStationEnMonitoringInformation.h \
    TEUI/src/Maintain/PasswordLogin.h \
    TEUI/src/Update/ExportData.h \
    TEUI/src/Common/StatusRemindWindow.h \
    TEUI/src/Charge/ChargeReportFinish.h \
    TEUI/src/Charge/ChargingReport.h \
    TEUI/Plugin/ModuleTerm/ModuleTerm.h \
    TEUI/src/Maintain/RealData/ModuleInformation.h \
    TEUI/src/Maintain/RealData/ModuleInformationSurvey.h \
    TEUI/src/Maintain/HistoryData/OperateRecordInformation.h \
    TEUI/src/Charge/ApplayCharge.h \
    TEUI/src/Charge/ChagingFailureInformation.h \
    TEUI/src/Charge/ChargeTypeSelect.h \
    TEUI/src/Charge/FlashWaitWidget.h \
    TEUI/Plugin/TimeLimit/TimeLimit.h \
    TEUI/src/Charge/ChargeMangeBase.h \
    TEUI/src/Update/FlashExportLog.h \
    TEUI/src/Charge/VINStartCharging.h \
    TEUI/src/Maintain/ParamSet/QRcodeCreate.h \
    TEUI/src/Maintain/ParamSet/DCChargerSet.h \
    TEUI/src/Maintain/RealData/DCChargerData.h \
    TEUI/src/Maintain/ParamSet/SystemTimeSet.h \
    TEUI/src/Maintain/ParamSet/StartTypeSelect.h \
    TEUI/src/Maintain/ParamSet/ChargeModeSelect.h \
    TEUI/src/Maintain/ParamSet/ChargePassword.h \
    TEUI/src/Charge/PasswordCharge.h \
    TEUI/src/Maintain/ParamSet/LogoSet.h \
    TEUI/src/Maintain/RealData/RealtimeFaultInformation.h \
    TEUI/src/Maintain/ParamSet/CoupleGunSet.h \
	TEUI/src/Maintain/VersionInfo/LanguageSelect.h \
    TEUI/src/Charge/PrintPaper.h

FORMS    += \
    TEUI/ui/Maintain/Maintain.ui \
    TEUI/ui/Maintain/ParamSet/MaintainCSCUSet.ui \
    TEUI/ui/TeuiMainWindow.ui \
    TEUI/ui/Maintain/HistoryData/ChargingRecord.ui \
    TEUI/ui/Maintain/RealData/TerminalInformation.ui \
    TEUI/ui/Maintain/RealData/LineSideInformation.ui \
    TEUI/ui/Maintain/RealData/BMSInformation.ui \
    TEUI/ui/Maintain/HistoryData/FailureInformation.ui \
    TEUI/ui/Charge/TerminalChargingInformation.ui \
    TEUI/ui/Maintain/ParamSet/PeakChargingInformationView.ui \
    TEUI/ui/Maintain/RealData/TerminalInformationSurvey.ui \
    TEUI/ui/Maintain/ParamSet/PasswordSet.ui \
    TEUI/ui/Maintain/ParamSet/ActriphaseSet.ui \
    TEUI/ui/Maintain/ParamSet/SpecialFeatureSet.ui \
    TEUI/ui/Maintain/ParamSet/DcTerminalSpeFeaSet.ui \
    TEUI/ui/Maintain/ParamSet/LoadConstraintSet.ui \
    TEUI/Plugin/ChargeTerm/ChargeTerm.ui \
    TEUI/Plugin/Keyboard/Keyboard.ui \
    TEUI/ui/Maintain/VersionInfo/VersionInformation.ui \
    TEUI/ui/Maintain/RealData/SubStationEnMonitoringInformation.ui \
    TEUI/ui/Maintain/PasswordLogin.ui \
    TEUI/ui/Update/ExportData.ui \
    TEUI/ui/Common/StatusRemindWindow.ui \
    TEUI/ui/Charge/ChargeReportFinish.ui \
    TEUI/ui/Charge/ChargingReport.ui \
    TEUI/Plugin/ModuleTerm/ModuleTerm.ui \
    TEUI/ui/Maintain/RealData/ModuleInformation.ui \
    TEUI/ui/Maintain/RealData/ModuleInformationSurvey.ui \
    TEUI/ui/Maintain/HistoryData/OperateRecordInformation.ui \
    TEUI/ui/Charge/ApplayCharge.ui \
    TEUI/ui/Charge/ChagingFailureInformation.ui \
    TEUI/ui/Charge/ChargeTypeSelect.ui \
    TEUI/ui/Charge/FlashWaitWidget.ui \
    TEUI/Plugin/TimeLimit/TimeLimit.ui \
    TEUI/ui/Update/FlashExportLog.ui \
    TEUI/ui/Charge/VINStartCharging.ui \
    TEUI/ui/Maintain/ParamSet/QRcodeCreate.ui \
    TEUI/ui/Maintain/ParamSet/DCChargerSet.ui \
    TEUI/ui/Maintain/RealData/DCChargerData.ui \
    TEUI/ui/Maintain/ParamSet/SystemTimeSet.ui \
    TEUI/ui/Maintain/ParamSet/StartTypeSelect.ui \
    TEUI/ui/Charge/MainCharge.ui \
    TEUI/ui/Charge/MainCharge.ui \
    TEUI/ui/Maintain/ParamSet/ChargeModeSelect.ui \
    TEUI/ui/Maintain/ParamSet/ChargePassword.ui \
    TEUI/ui/Charge/PasswordCharge.ui \
    TEUI/ui/Maintain/ParamSet/LogoSet.ui \
    TEUI/ui/Maintain/RealData/RealtimeFaultInformation.ui \
    TEUI/ui/Maintain/ParamSet/CoupleGunSet.ui \
	TEUI/ui/Maintain/VersionInfo/LanguageSelect.ui \
    TEUI/ui/Charge/PrintPaper.ui

RESOURCES += \
    TEUI/qrc/teui.qrc

DISTFILES +=

OTHER_FILES +=
