#include "protocol.h"
#include <arpa/inet.h>
#include "protobufserver.h"

Protocol::Protocol(Net *n)
{
	net = n;
	serverType = 0;
	connected = false;
	logged = false;
	terminated = false;

    ProtobufServer::server()->getCtrlSetting(ctrlAddr, cscuKey,ctrlSwVersion);
	ProtobufServer::server()->getTermSetting(acNum, tacNum, dcNum);
	ProtobufServer::server()->getNetSetting(serverAddr, serverPort, encryptNet);

	for(uchar i = ID_MinACSinCanID; i < ID_MinACSinCanID + acNum; i++){
		_canRange.append(i);
	}
	for(uchar i = ID_MinACThrCanID; i < ID_MinACThrCanID + tacNum; i++){
		_canRange.append(i);
	}
	for(uchar i = ID_MinDCCanID; i < ID_MinDCCanID + dcNum; i++){
		_canRange.append(i);
	}
}

Protocol::~Protocol()
{

}

bool Protocol::command(InfoMap &map, InfoAddrType &type)
{
	return true;
}

bool Protocol::burst(QDataPointList burst)
{
	return true;
}

bool Protocol::parse(short type, char *buff, int len)
{
	return true;
}

bool Protocol::login()
{
	return true;	
}

void Protocol::logout(int type)
{
    net->close();
	connected = false;

	logged = false;
}

void Protocol::heart()
{

}

void Protocol::onNetConnected()
{
    writeLog("Net connected",2);

	connected = true;

	login();
}

void Protocol::onNetDisconnected()
{
    writeLog("Net disconnected",2);

	connected = false;

	logged = false;

	emit active(logged);
}

void Protocol::onNetError(int err)
{
    writeLog(QString().sprintf("Net error: %s", net->errorString(err)),2);

	connected = false;
	logged = false;

	emit active(logged);
}

void Protocol::onNetReceive()
{
	NetFrame *f;

	char buff[RECV_BUFF_LEN + 1] = {0};
	int recv, buff_len, payload_len;
	if(!net){
		return;
	}

	while((recv = net->read(buff, RECV_BUFF_LEN)) > 0){
		arBuff.append(buff, recv);
	}

	buff_len = arBuff.length();

	while(buff_len >= sizeof(NetFrame)){
		f = (NetFrame *)arBuff.data();
		payload_len = ntohl(f->payload_len);

		if(f->symbol != FRAME_FLAG){
			arBuff.clear();
			break;
		}

		if(buff_len < sizeof(NetFrame) + payload_len){
			break;
		}

		parse(f->msgType, f->payload, payload_len);

		buff_len = buff_len - sizeof(NetFrame) - payload_len;
		arBuff = arBuff.right(buff_len);
	}
}

bool Protocol::sendFrame(char frameType, PBMessage *message)
{
	NetFrame frame;
	QByteArray ar;
	string str;

    if(!net || !connected)
		return false;

	message->SerializeToString(&str);
	memset(&frame, 0, sizeof(NetFrame));

	frame.symbol = FRAME_FLAG;
	frame.header_len = sizeof(NetFrame);
	frame.proto_ver = 0x03;
	frame.encoding_type = 0x01;
	frame.msgType = frameType;
	frame.payload_len = htonl(str.length());
	memcpy(frame.ctrlAddr, ctrlAddr.c_str(), ctrlAddr.length() > sizeof(frame.ctrlAddr) ? sizeof(frame.ctrlAddr) : ctrlAddr.length());

	ar.append((char *)&frame, sizeof(NetFrame));
	ar.append(str.c_str(), str.length());
	net->write(ar.data(), ar.length());

	return true;
}

void Protocol::destroy()
{
	terminated = true;
}

void Protocol::writeLog(QString strLog, int level)
{
	ProtobufServer::server()->writeLog(strLog, level);
}
