#include "datacache.h"
#include "protobufserver.h"
#include <math.h>
#include <QFile>
#include <QMap>
#include <QStringList>
#include "qjson/parser.h"
#include "qjson/serializer.h"
#include "monitorprotobuf.pb.h"

#define MODEL_FILE	"/mnt/nandflash/etc/dev_model.json"
#define SPEC_FILE	"/mnt/nandflash/etc/dev_spec.json"
#define CACHE_FILE	"/mnt/nandflash/dev_cache.json"

using namespace monitor::protobuf;

QVariantMap DataCache::mapModel;
QVariantMap DataCache::mapPoint;
QVariantMap DataCache::mapRealSpec;
QVariantMap DataCache::mapDevSpec;
pthread_mutex_t DataCache::mutex = PTHREAD_MUTEX_INITIALIZER;

DataCache::DataCache()
{

}

DataCache::~DataCache()
{

}

bool DataCache::createTable()
{
	QJson::Parser parser;
	bool ok;

	//加载点表模型
	QFile file(MODEL_FILE);
	if(!file.exists()){
		ProtobufServer::server()->writeLog("Load data table failed!");
		file.close();
		return false;
	}
	mapModel = parser.parse(&file, &ok).toMap();
	file.close();

	//清理模型中描述信息
	QVariantMap devices, dev, infoAddr, info;
	for(QVariantMap::Iterator i = mapModel.begin(); i != mapModel.end(); ++i){
		dev = i.value().toMap();
		infoAddr = dev["infoAddr"].toMap();
		for(QVariantMap::Iterator j = infoAddr.begin(); j != infoAddr.end(); ++j){
			info = j.value().toMap();
			info.remove("desc");
			infoAddr[j.key()] = info;
		}
		dev["infoAddr"] = infoAddr;
		mapModel[i.key()] = dev;
	}

	//加载设备规格信息
	file.setFileName(SPEC_FILE);
	if(file.exists()){
		file.open(QIODevice::ReadOnly);
		mapDevSpec = parser.parse(&file, &ok).toMap();
		file.close();
	}

	return true;
}

void DataCache::destroyTable()
{
	pthread_mutex_destroy(&mutex);
}

bool DataCache::query(QString strKey, QDataPointList &list, QString strQuery)
{
	QVariantMap devices, dev, infoAddr, info;
	QJson::Serializer serializer;
	serializer.setIndentMode(QJson::IndentMedium);

	pthread_mutex_lock(&mutex);
	if(!mapPoint.contains(strKey)){
		pthread_mutex_unlock(&mutex);
		return false;
	}
	devices = mapPoint[strKey].toMap();
	pthread_mutex_unlock(&mutex);

	for(QVariantMap::Iterator i = devices.begin(); i != devices.end(); ++i){
		dev = i.value().toMap();
		if(!dev.contains("infoAddr"))
			continue;
		infoAddr = dev["infoAddr"].toMap();
		for(QVariantMap::Iterator j = infoAddr.begin(); j != infoAddr.end(); ++j){
			if(j.key().isEmpty())
				continue;
			info = j.value().toMap();
			if(info["dataType"].toString() != strQuery)
				continue;
			if(info.contains("dataField"))
				continue;

			//数据属性填充
			DataPoint data;
			data.dataType = info["dataType"].toString();
			data.dataName = info["dataName"].toString();
			data.value = info["dataValue"].toString();
			data.canAddr = dev["canAddr"].toString();
			data.devType = dev["devType"].toString();
			data.devName = dev["devName"].toString();
			list.append(data);

			//查询后需更新前一个值，以备突发比较
			if(info["dataType"].toString() == "measure"){
				info["preValue"] = data.value;
				infoAddr[j.key()] = info;
			}
		}
		dev["infoAddr"] = infoAddr;
		devices[i.key()] = dev;
	}

	pthread_mutex_lock(&mutex);
	mapPoint[strKey] = devices;
	pthread_mutex_unlock(&mutex);

	return true;
}

bool DataCache::alarm(InfoMap &map, QDataPointList &burst)
{
	QVariantMap devices, dev, infoAddr, info;
	QString canAddr, devId, devName, devType, devIndex;
	int can, id;

	if(!map.contains(Addr_CanID_Comm))
		return false;

	can = map[Addr_CanID_Comm].at(0);
	canAddr.sprintf("%d", can);
	devIndex.sprintf("%d", can);

	//包含内部id的设备
	if(map.contains(Addr_DevID_DC_Comm)){
		id = map[Addr_DevID_DC_Comm].at(0);
		if(can >= ID_MinCCUCanID && can <= ID_MaxCCUCanID){//CCU上传的信息
			if(id >= InnerID_MinMod && id <= InnerID_MaxMod){//直流模块
				devName = "dcmodular";
				devType.sprintf("%d", Dcmodular);
			}else if(id >= ID_MinDCCanID && id <= ID_MaxDCCanID){//PDU
				devName = "pdu";
				devType.sprintf("%d", PDU);
				devIndex.sprintf("%d", id);
			}else if(id >= ID_MinCCUCanID && id <= ID_MaxCCUCanID){//CCU
				devName = "ccu";
				devType.sprintf("%d", CCU);
				devIndex.sprintf("%d", id);
			}else{
				return false;
			}
		}else{
			return false;	
		}

		devId.sprintf("%d", id);
		pthread_mutex_lock(&mutex);
		if(mapPoint.contains(canAddr)){
			devices = mapPoint[canAddr].toMap();
			dev = devices[devId].toMap();
		}else{
			if(mapDevSpec.contains(canAddr)){
				devices = mapDevSpec[canAddr].toMap();
				dev = devices[devId].toMap();
			}
		}
		pthread_mutex_unlock(&mutex);
	}else{
		return false;	
	}

	if(!map.contains(Addr_DCcabFaultState))
		return false;
	if(!map.contains(Addr_DCcabFaultCode))
		return false;

	uchar c = 0;
	QString str;

	str.sprintf("%d", map[Addr_DCcabFaultCode].at(0));
	DataPoint data;
	data.canAddr = devIndex;
	data.devType = devType;
	data.devName = dev["devName"].toString();
	data.dataType = "alarm";
	data.value = str;
	c = map[Addr_DCcabFaultState].at(0);
	if(c == 0x55){
		c = 1;
	}else if(c == 0x0){
		c = 2;
	}

	str.sprintf("%d", c);
	data.valueDesc = str;
	burst.append(data);

	return true;
}

#define _IS_BURST(value, send, threshold)\
		fabs((double)(value - send)) / (double)send >= (double)threshold ? true : false

bool DataCache::update(InfoMap &map, QDataPointList &burst)
{
	QVariantMap devices, dev, infoAddr, info;
	QString canAddr, devId, devName;
	int can, id;

	if(!map.contains(Addr_CanID_Comm))
		return false;

	can = map[Addr_CanID_Comm].at(0);
	canAddr.sprintf("%d", can);

	//包含内部id的设备
	if(map.contains(Addr_DevID_DC_Comm)){
		id = map[Addr_DevID_DC_Comm].at(0);
		if(can >= ID_MinCCUCanID && can <= ID_MaxCCUCanID){//CCU上传的信息
			if(id >= InnerID_MinMod && id <= InnerID_MaxMod){//直流模块
				devName = "dcmodular";
			}else if(id >= ID_MinDCCanID && id <= ID_MaxDCCanID){//PDU
				devName = "pdu";
			}else if(id >= ID_MinCCUCanID && id <= ID_MaxCCUCanID){//CCU
				devName = "ccu";
			}else{
				return false;
			}
		}else if((can >= ID_MinACSinCanID && can <= ID_MaxACSinCanID) ||
				(can >= ID_MinACSinCanID && can <= ID_MaxACSinCanID)){//交流模块上传的信息
			if((id >= ID_MinACSinCanID && id <= ID_MaxACSinCanID) ||
				(id >= ID_MinACSinCanID && id <= ID_MaxACSinCanID)){//交流模块
				devName = "acmodular";
			}else{
				return false;	
			}
		}else{
			return false;	
		}

		devId.sprintf("%d", id);
		pthread_mutex_lock(&mutex);
		if(mapPoint.contains(canAddr)){
			devices = mapPoint[canAddr].toMap();
			dev = devices[devId].toMap();
		}else{
			if(mapDevSpec.contains(canAddr)){
				devices = mapDevSpec[canAddr].toMap();
				dev = devices[devId].toMap();
			}
		}
		pthread_mutex_unlock(&mutex);
	}else{
		//不包含内部地址，检查是否有名称
		if(!map.contains(Addr_DevData_Type))
			return false;

		//以名称作为KEY
		devId.sprintf("%s", map[Addr_DevData_Type].data());
		devName = devId;

		pthread_mutex_lock(&mutex);
		if(mapPoint.contains(canAddr)){
			devices = mapPoint[canAddr].toMap();
		}else{
			dev = mapModel[devName].toMap();
			devices.insert(devId, dev);
		}
		pthread_mutex_unlock(&mutex);

		if(!devices.contains(devName))
			return false;
		dev = devices[devName].toMap();
	}

	if(!dev.contains("infoAddr"))
		return false;

	infoAddr = dev["infoAddr"].toMap();
	dev["canAddr"] = canAddr;

	for(InfoMap::Iterator it = map.begin(); it != map.end(); ++it){
		QString strInfoAddr, strInfoType, strNewValue, strValue, strPreValue;
		QString strDataName, strDataType;
		bool bBurst = false;

		strInfoAddr.sprintf("%X", it.key());
		if(!infoAddr.contains(strInfoAddr))
			continue;

		info = infoAddr[strInfoAddr].toMap();

		strInfoType = info["infoType"].toString();

		if(strInfoType == "float")
			strNewValue.sprintf("%f", *((float *)it.value().data()));
		else if(strInfoType == "int")
			strNewValue.sprintf("%d", *((int *)it.value().data()));
		else if(strInfoType == "short")
			strNewValue.sprintf("%d", *((short *)it.value().data()));
		else if(strInfoType == "char")
			strNewValue.sprintf("%d", *((char *)it.value().data()));
		else if(strInfoType == "string")
			strNewValue.sprintf("%s", it.value().data());

		if(info.contains("dataField")){
			dev[info["dataField"].toString()] = strNewValue;
			continue;
		}

		if(info.contains("preValue"))
			strPreValue = info["preValue"].toString();

		strDataType = info["dataType"].toString();
		strValue = info["dataValue"].toString();

		//遥测突发检测
		if(strDataType == "measure"){
			if(!strPreValue.isEmpty() && _IS_BURST(strNewValue.toDouble(), strPreValue.toDouble(), 0.1)){
				bBurst = true;
				info["preValue"] = strNewValue;
			}
		}else{
			//遥信，状态，报警突发检测
			if(strNewValue != strValue){
				bBurst = true;
			}
		}

		if(bBurst){
			DataPoint data;
			data.canAddr = dev["canAddr"].toString();
			data.devType = dev["devType"].toString();
			data.devName = dev["devName"].toString();
			data.dataName = info["dataName"].toString();
			data.dataType = info["dataType"].toString();
			data.value = strNewValue;
			burst.append(data);
		}

		//更新数据
		info["dataValue"] = strNewValue;
		infoAddr[strInfoAddr] = info;
	}

	dev["infoAddr"] = infoAddr;
	devices[devId] = dev;

	pthread_mutex_lock(&mutex);
	mapPoint[canAddr] = devices;
	pthread_mutex_unlock(&mutex);

	return true;
}

bool DataCache::saveDevSpec(InfoMap &map)
{	
	QVariantMap devices, dev, infoAddr, info;
	QString canAddr, devId, devName;
	int can, id;
	QJson::Serializer serializer;
	serializer.setIndentMode(QJson::IndentMedium);

	if(!map.contains(Addr_CanID_Comm))
		return false;

	can = map[Addr_CanID_Comm].at(0);
	canAddr.sprintf("%d", can);

	if(!map.contains(Addr_DevID_DC_Comm))
		return false;

	id = map[Addr_DevID_DC_Comm].at(0);

	if(can >= ID_MinCCUCanID && can <= ID_MaxCCUCanID){//CCU上传的信息
		if(id >= InnerID_MinMod && id <= InnerID_MaxMod){//直流模块
			devName = "dcmodular";
		}else if(id >= ID_MinDCCanID && id <= ID_MaxDCCanID){//PDU
			devName = "pdu";
		}else if(id >= ID_MinCCUCanID && id <= ID_MaxCCUCanID){//CCU
			devName = "ccu";
		}else{
			return false;
		}
	}else if((can >= ID_MinACSinCanID && can <= ID_MaxACSinCanID) ||
			(can >= ID_MinACSinCanID && can <= ID_MaxACSinCanID)){//交流模块上传的信息
		if((id >= ID_MinACSinCanID && id <= ID_MaxACSinCanID) ||
				(id >= ID_MinACSinCanID && id <= ID_MaxACSinCanID)){//交流模块
			devName = "acmodular";
		}else{
			return false;	
		}
	}else{
		return false;	
	}

	devId.sprintf("%d", id);
	if(!mapModel.contains(devName))
		return false;

	dev = mapModel[devName].toMap();
	if(!dev.contains("infoAddr"))
		return false;

	if(mapRealSpec.contains(canAddr)){
		devices = mapRealSpec[canAddr].toMap();
	}else{
		devices[devId] = dev;
		mapRealSpec[canAddr] = devices;
	}

	infoAddr = dev["infoAddr"].toMap();
	dev["canAddr"] = devId;

	for(InfoMap::Iterator it = map.begin(); it != map.end(); ++it){
		QString strInfoAddr, strInfoType, strDataType, strValue;

		strInfoAddr.sprintf("%X", it.key());
		if(!infoAddr.contains(strInfoAddr))
			continue;

		info = infoAddr[strInfoAddr].toMap();
		strInfoType = info["infoType"].toString();

		if(strInfoType == "float")
			strValue.sprintf("%f", *((float *)it.value().data()));
		else if(strInfoType == "int")
			strValue.sprintf("%d", *((int *)it.value().data()));
		else if(strInfoType == "short")
			strValue.sprintf("%d", *((short *)it.value().data()));
		else if(strInfoType == "char")
			strValue.sprintf("%d", *((char *)it.value().data()));
		else if(strInfoType == "string")
			strValue.sprintf("%s", it.value().data());

		if(info.contains("dataField")){
			dev[info["dataField"].toString()] = strValue;
			continue;
		}

		info["dataValue"] = strValue;
		infoAddr[strInfoAddr] = info;
		dev["infoAddr"] = infoAddr;
	}

	devices[devId] = dev;
	mapRealSpec[canAddr] = devices;

	//规格信息发送完成，进行保存
	if(map.contains(Addr_SepcEndFlag_Term) && map[Addr_SepcEndFlag_Term].at(0) == 1){
		QFile file(SPEC_FILE);
		file.open(QIODevice::ReadWrite | QIODevice::Truncate);
		file.write(serializer.serialize(mapRealSpec));
		file.close();
		//覆盖保存的规格信息
		mapDevSpec = mapRealSpec;
	}

	return true;
}

bool DataCache::saveCache()
{
	QJson::Serializer serializer;
	serializer.setIndentMode(QJson::IndentMedium);

	QFile file(CACHE_FILE);
	file.open(QIODevice::ReadWrite | QIODevice::Truncate);
	pthread_mutex_lock(&mutex);
	file.write(serializer.serialize(mapPoint));
	pthread_mutex_unlock(&mutex);
	file.close();

	return true;
}

