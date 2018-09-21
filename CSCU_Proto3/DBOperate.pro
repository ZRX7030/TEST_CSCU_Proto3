#-------------------------------------------------
#
# Project created by QtCreator 2016-10-26T23:30:49
#
#-------------------------------------------------

QT       -= gui

TARGET = DBOperate
TEMPLATE = lib

include(CSCU_A1_Manage.pri)

DEFINES += DBOPERATE

INCLUDEPATH += \
			GeneralData/ \
                        Database

LIBS += -L$${TELD_LIBPATH} -lsqlite3

SOURCES += \
	Database/database.cpp \
	Database/DBOperate.cpp 

HEADERS +=\
