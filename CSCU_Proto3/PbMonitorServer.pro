#-------------------------------------------------
#
# Project created by QtCreator 2016-10-26T23:30:49
#
#-------------------------------------------------

QT       -= gui

TARGET = PbMonitorServer
TEMPLATE = lib

include(CSCU_A1_Manage.pri)

INCLUDEPATH += \
	ProtobufServer/codec \
	ProtobufServer/protocol \
	ProtobufServer/cache \
	ProtobufServer/net \
	ProtobufServer/server \
	Infotag \
	DevCache \
	RealDataFilter \
	GeneralData \
	Database \
	ParamSet \
	Log

CXXFLAGS += -DPROTOBUF_MONITOR_SERVER

LIBS += \
	-L$$PWD/FileSystemFiles_9280/nandflash/lib \
	-lqjson -lssl -lcrypto -lprotobuf

SOURCES += \
	ProtobufServer/codec/monitorprotobuf.pb.cc\
	ProtobufServer/net/net.cpp \
	ProtobufServer/net/tcpnet.cpp \
	ProtobufServer/protocol/protocol.cpp \
	ProtobufServer/protocol/monitorprotocol.cpp \
	ProtobufServer/cache/datacache.cpp \
	ProtobufServer/server/monitorserver.cpp \
	ProtobufServer/server/protobufserver.cpp

HEADERS +=\
	ProtobufServer/codec/monitorprotobuf.pb.h\
	ProtobufServer/net/net.h\
	ProtobufServer/net/tcpnet.h \
	ProtobufServer/protocol/protocol.h \
	ProtobufServer/protocol/monitorprotocol.h \
	ProtobufServer/cache/datacache.h \
	ProtobufServer/server/monitorserver.h \
	ProtobufServer/server/protobufserver.h \
	GeneralData/ModuleIO.h \
	GeneralData/Bus.h
