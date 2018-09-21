#-------------------------------------------------
#
# Project created by QtCreator 2016-10-26T23:30:49
#
#-------------------------------------------------

QT       -= gui

TARGET = WebServer
TEMPLATE = lib

include(CSCU_A1_Manage.pri)

INCLUDEPATH += \
			GeneralData/ \
			CommonFunc/ \
			Encrypt/ \
			Database/ \
			ParamSet/ \
                        $${TELD_INCLUDEPATH}/include/ \
                        Log/ \
			InfoTag

LIBS += \
                -L$${TELD_LIBPATH} \
		-lcurl \
		-ljson-c \
		-lsqlite3

SOURCES += \
			WebServer/WebServer.cpp \
			WebServer/HttpRequest.cpp \
			CommonFunc/commfunc.cpp \
			Encrypt/aes_cbc.cpp

HEADERS += \
			WebServer/WebServer.h \
			WebServer/HttpRequest.h \
			CommonFunc/commfunc.h \
			Encrypt/aes_cbc.h

