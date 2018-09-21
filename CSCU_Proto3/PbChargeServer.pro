#-------------------------------------------------
#
# Project created by QtCreator 2016-10-26T23:30:49
#
#-------------------------------------------------

QT       -= gui

TARGET = PbChargeServer
TEMPLATE = lib

include(CSCU_A1_Manage.pri)

INCLUDEPATH += \
	ProtobufServer/codec \
	ProtobufServer/protocol \
	ProtobufServer/cache \
	ProtobufServer/net \
	ProtobufServer/server \
	Infotag \
	CommonFunc \
	DevCache \
	RealDataFilter \
	GeneralData \
	Database \
	ParamSet \
	Log

CXXFLAGS += -DPROTOBUF_CHARGE_SERVER

LIBS += \
	-L$$PWD/FileSystemFiles_9280/nandflash/lib \
        -lssl -lcrypto -lprotobuf

SOURCES += \
	ProtobufServer/codec/chargeprotobuf.pb.cc\
	ProtobufServer/net/net.cpp \
	ProtobufServer/net/tcpnet.cpp \
	ProtobufServer/protocol/protocol.cpp \
	ProtobufServer/protocol/chargeprotocol.cpp \
	ProtobufServer/server/chargeserver.cpp \
	ProtobufServer/server/protobufserver.cpp \
	CommonFunc/commfunc.cpp

HEADERS +=\
	ProtobufServer/codec/chargeprotobuf.pb.h\
	ProtobufServer/net/net.h\
	ProtobufServer/net/tcpnet.h \
	ProtobufServer/protocol/protocol.h \
	ProtobufServer/protocol/chargeprotocol.h \
	ProtobufServer/server/chargeserver.h \
	ProtobufServer/server/protobufserver.h \
	CommonFunc/commfunc.h \
	GeneralData/ModuleIO.h \
	GeneralData/Bus.h
