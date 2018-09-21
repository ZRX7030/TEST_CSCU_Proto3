#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <QVariant>
#include <QDebug>

#include "Common.h"
#include "TeuiProtocol.h"
#include "InfoData.h"

TeuiProtocol::TeuiProtocol(unsigned char station_addr, CBus *bus)
{
    stationAddr = station_addr;
    this->bus = bus;
}

TeuiProtocol::~TeuiProtocol()
{
	
}

void TeuiProtocol::dealParaseDatas(stParseData parseData)
{
	QVariant var;
	switch(parseData.cmdSlave)
    {
        case InfoConfigCSCU:
			{
				if(parseData.len == sizeof(stCSCUParam))
					var.setValue(*((stCSCUParam *)parseData.dst));
			}break;
        case InfoMaintainVersion:    //版本信息
            {
                if( parseData.len == sizeof(stVersionInformation) )
                    var.setValue(*((stVersionInformation *)parseData.dst));
                //qDebug() << "1 is" << parseData.len  << "resl is =" << sizeof(stVersionInformation) ;
            }break;
        case InfoConfigSys:
			{
			}break;
        case InfoConfigQRCode:
              break;
        case InfoConfigChargeModule:
            break;
        case InfoConfigTime:			//时间设置
            break;
        case InfoConfigPassword:		//密码数据
            {
                if(parseData.len == sizeof(unsigned int))
                    var.setValue(*((unsigned int *)parseData.dst));
            }break;
		case InfoConfigIO:
			{
				if( parseData.len == sizeof(stIOConfigParam) )
					var.setValue(*((stIOConfigParam *)parseData.dst));
			}break;
        case InfoConfigPhaseType:		//三相相别数据
			{
				if( parseData.len == (1+parseData.dst[0] * sizeof(stPhaseType)) )
				{
					int count = parseData.dst[0];
					unsigned char *point = &parseData.dst[1];
					stAllPhaseType allPhaseType;
					
					for(int i=0; i<count; i++)
					{
						stPhaseType phaseType;
						memcpy((unsigned char *)&phaseType, point, sizeof(stPhaseType));
						allPhaseType.phaseList.append(phaseType);

						point += sizeof(stPhaseType);
					}
					var.setValue(allPhaseType);
				}
			}break;
        case InfoConfigTerminalQRCode:   //二维码生成
            {
                if( parseData.len == (1+parseData.dst[0] *sizeof(stQRcodeCreate)) )
                {
                    int count = parseData.dst[0];
                    unsigned char *point = &parseData.dst[1];
                    stAllQRcodeCreate allQRcodeCreate;

                    for(int i=0; i<count; i++)
                    {
                        stQRcodeCreate codeCreate;
						//qDebug() << "InfoConfigTerminalQRCode=canaAddr=" << *point;
                        memcpy((unsigned char *)&codeCreate, point, sizeof(stQRcodeCreate));
                        allQRcodeCreate.codeList.append(codeCreate);

                        point += sizeof(stQRcodeCreate);
                    }
                    var.setValue(allQRcodeCreate);
                }
            }break;
        case InfoConfigSpecialFunc:		//特殊功能的数据
			{
				if(parseData.len == sizeof(stSpecialFunc))
					var.setValue(*((stSpecialFunc *)parseData.dst));
			}break;
        case InfoConfigDcSpecialFunc:
            break;
        case InfoConfigLoad:			//负荷约束的数据
			{
                //qDebug() << "stPowerLimitParam len=" << sizeof(stPowerLimitParam) << "parseData.len=" << parseData.len;
				if(parseData.len == sizeof(stPowerLimitParam))
					var.setValue(*((stPowerLimitParam *)parseData.dst));
			}break;
        case InfoConfigTPFV:			//错峰充电参数
			{
				if( parseData.len == (2+parseData.dst[1] * sizeof(stTPFVParam)) )
				{
					int count = parseData.dst[1];
					unsigned char *point = &parseData.dst[2];
					stAllTPFVParam allTPFVParam;
		
					allTPFVParam.peakCharegeEnable = parseData.dst[0];
					for(int i=0; i<count; i++)
					{
						stTPFVParam type;
						memcpy((unsigned char *)&type, point, sizeof(stTPFVParam));
						allTPFVParam.tpfvList.append(type);

						point += sizeof(stTPFVParam);
					}
					var.setValue(allTPFVParam);
				}
			}break;
		case InfoConfigAmmeterAddr:				//电表地址参数
			{
				if( parseData.len == (1+parseData.dst[0] * sizeof(stAmmeterAddr)) )
				{
					int count = parseData.dst[0];
					unsigned char *point = &parseData.dst[1];
					stAllAmmeterAddr allParam;

					for(int i=0; i<count; i++)
					{
						stAmmeterAddr addr;
						memcpy((unsigned char *)&addr, point, sizeof(stAmmeterAddr));
						allParam.ammeterList.append(addr);
						point += sizeof(unsigned char);
					}
					var.setValue(allParam);
				}
			}break;
		case InfoConfigDCChargerTypeNum:  //直流机终端参数、监控设备参数数量
            {
				if( parseData.len == (2+parseData.dst[1] * sizeof(unsigned char)) )
				{
					int count = parseData.dst[1];
					unsigned char *point = &parseData.dst[2];
					stDCChargerDeviceNum allParam;

					allParam.type = parseData.dst[0];
					for(int i=0; i<count; i++)
					{
                        //qDebug() << "dccharger can addr is-------------" << *point;
						unsigned char type;
						memcpy(&type, point, sizeof(unsigned char));
						allParam.addrList.append(type);
						point += sizeof(unsigned char);
					}
					var.setValue(allParam);
				}
			}break;
		case InfoConfigDCChargerTerm:  //直流机终端参数
            {
                //qDebug() << "dccharger terminal--------parseData.len=" << parseData.len << "sizeof(stDCChargerTermParam)=" <<sizeof(stDCChargerTermParam);
				if( parseData.len == sizeof(stDCChargerTermParam) )
					var.setValue(*((stDCChargerTermParam *)parseData.dst));
			}break;
		case InfoConfigDCChargerMonitor:  //直流机监控设备参数
            {
                //qDebug() << "dccharger monitor------parseData.len" << parseData.len << "sizeof(stDCChargerMonitorParam)=" <<sizeof(stDCChargerMonitorParam);
				if( parseData.len == sizeof(stDCChargerMonitorParam) )
					var.setValue(*((stDCChargerMonitorParam *)parseData.dst));
			}break;
		case InfoRealFault:				//实时故障数据查询
			{
                //qDebug() << "parseData.len=////////////////////////" << parseData.len << "sizeof(stTerminalFault)=\\\\\\\\\\\\\\\\" <<(102+parseData.dst[0] * sizeof(stTerminalFault)) ;

                if( parseData.len == (102+parseData.dst[0] * sizeof(stTerminalFault)) )
                {
                    int count = parseData.dst[0];
                    unsigned char *point = &parseData.dst[1];
                    stAllTerminalFault allParam;
                    allParam.totalnum=parseData.dst[0];
                    //qDebug() <<"11111111111111111111";
                    for(int i=0; i<count; i++)
                    {
                        stTerminalFault fault;
                        memcpy((unsigned char *)&fault, point, sizeof(stTerminalFault));
                        allParam.faultList.append(fault);
                        point += sizeof(stTerminalFault);
                    }
                    //qDebug() <<"2222222222222222222222222222222";
                    memcpy(allParam.chargestartTime, point, sizeof(allParam.chargestartTime));
                    point+= sizeof(allParam.chargestartTime);
                    memcpy(allParam.chargestopTime, point, sizeof(allParam.chargestopTime));
                    point+= sizeof(allParam.chargestopTime);
                    allParam.currentSOC = *(point++);
                    memcpy(allParam.stopReson, point, sizeof(allParam.stopReson));
                   // point+= sizeof(allParam.stopReson);
                    var.setValue(allParam);
                    //qDebug() <<"33333333333333333333333333333333333333";
                }

			}break;
		case InfoRealCharge:			//充电中实时数据
			{
				//qDebug() << "parseData.len=" << parseData.len << "sizeof(stChargeReal)=" <<sizeof(stChargeReal);
				if( parseData.len == sizeof(stChargeReal) )
					var.setValue(*((stChargeReal *)parseData.dst));
			}break;
		case InfoStatusBase:			//终端的实时状态数据
			{
				if( parseData.len == sizeof(stTerminalStatus) )
					var.setValue(*((stTerminalStatus *)parseData.dst));
			}break;
		case InfoStatusCSCU:			//cscu状态数据
			{
				if( parseData.len == sizeof(stCSCUStatus) )
					var.setValue(*((stCSCUStatus *)parseData.dst));
			}break;
		case InfoRealModule:
			break;
		case InfoRealStatus:			//实时终端的数据
			{
				if( parseData.len == sizeof(stTerminalReal) )
					var.setValue(*((stTerminalReal *)parseData.dst));
			}break;
		case InfoRealBMS:				//实时bms数据
			{
				if( parseData.len == sizeof(stTerminalBMS) )
					var.setValue(*((stTerminalBMS *)parseData.dst));
			}break;
		case InfoRealAmmeter:			//实时电表数据
			{
				//qDebug() << "parseData.lenarseData.len=" <<  parseData.len <<  "  struct=" << sizeof(stAmmeterData);
				if( parseData.len == sizeof(stAmmeterData) )
				{
					var.setValue(*((stAmmeterData *)parseData.dst));
				}
			}break;
		case InfoRealStation:		//环境数据
			{
				if( parseData.len == sizeof(stStationRealData) )
					var.setValue(*((stStationRealData *)parseData.dst));
			}break;
        case InfoConfigDCMPCNum:    //直流机下pdu、ccu、模块数量
			{
				if( parseData.len == (2+parseData.dst[1] * sizeof(stDCChargerTypeNum)) )
				{
					int count = parseData.dst[1];
					unsigned char *point = &parseData.dst[2];
					stAllDCChargerTypeNum allParam;
					allParam.type = parseData.dst[0];
					for(int i=0; i<count; i++)
					{
						stDCChargerTypeNum type;

						memcpy((unsigned char *)&type, point, sizeof(stDCChargerTypeNum));
						allParam.listNum.append(type);
						point += sizeof(stDCChargerTypeNum);
					}
					var.setValue(allParam);
				}
			}break;
		case InfoRealDCModule:		//直流模块数据
			{
				//qDebug() << "module data is parase len=" << paraseData.len << "sizeof=" << sizeof(stDCModuleRealData)	;
				if( parseData.len == sizeof(stDCModuleRealData) )
					var.setValue(*((stDCModuleRealData *)parseData.dst));
			}break;
		case InfoRealDCPdu:		//直流pdu数据
			{
				//qDebug() << "pdu data is parase len=" << paraseData.len << "sizeof=" << sizeof(stDCPduRealData)	;
				if( parseData.len == sizeof(stDCPduRealData) )
					var.setValue(*((stDCPduRealData *)parseData.dst));
			}break;
		case InfoRealDCCcu:			//直流ccu数据
			{
				//qDebug() << "ccu data is parase len=" << paraseData.len << "sizeof=" << sizeof(stDCCcuRealData)	;						if( parseData.len == sizeof(stDCCcuRealData) )
					var.setValue(*((stDCCcuRealData *)parseData.dst));
			}break;
		case InfoHistoryTotal:			//历史数据总共数目
			{
				if( parseData.len == sizeof(stHistoryInfo) )
					var.setValue(*((stHistoryInfo *)parseData.dst));
			}break;
		case InfoHistoryOperate:		//历史操作记录
			{
				//qDebug() << "history operate 1";
				if( parseData.len == (2+parseData.dst[1] * sizeof(stHistoryOperate)) )
				{
					int count = parseData.dst[1];
					unsigned char *point = &parseData.dst[2];

					stAllHistoryOperate operateHist;
					operateHist.currentPage = parseData.dst[0];
					for(int i=0; i<count; i++)
					{
						stHistoryOperate history;
						memcpy((unsigned char *)&history, point, sizeof(stHistoryOperate));
						operateHist.operateList.append(history);

						point += sizeof(stHistoryOperate);
					}
					var.setValue(operateHist);
				}
			}break;
		case InfoHistoryFault:			//历史故障数据
			{
				if( parseData.len == (2+parseData.dst[1] * sizeof(stHistoryFault)) )
				{
					int count = parseData.dst[1];
					unsigned char *point = &parseData.dst[2];

					stAllHistoryFault faultHist;
					faultHist.currentPage = parseData.dst[0];
					for(int i=0; i<count; i++)
					{
						stHistoryFault history;
						memcpy((unsigned char *)&history, point, sizeof(stHistoryFault));
						faultHist.faultList.append(history);

						point += sizeof(stHistoryFault);
					}
					var.setValue(faultHist);
				}
			}break;
		case InfoHistoryCharge:			//历史充电数据
			{
				if( parseData.len == (2+parseData.dst[1] * sizeof(stHistoryCharge)) )
				{
					int count = parseData.dst[1];
					unsigned char *point = &parseData.dst[2];
					stAllHistoryCharge chargeHist;

					chargeHist.currentPage = parseData.dst[0];
					for(int i=0; i<count; i++)
					{
						stHistoryCharge history;
						memcpy((unsigned char *)&history, point, sizeof(stHistoryCharge));
						chargeHist.chargeList.append(history);

						point += sizeof(stHistoryCharge);
					}
					var.setValue(chargeHist);
				}
			}break;
		case InfoRealtimeFaultTotal:    //实时故障数据总条目 //add by yanwei 20170914
            {
                if( parseData.len == sizeof(stRealtimeInfo) )
                    var.setValue(*((stRealtimeInfo *)parseData.dst));
            }break;
        case InfoRealtimeFault:     //add by yanwei 20170914
            {
                if( parseData.len == (2+parseData.dst[1] * sizeof(stRealtimeFault)) )
                {
                    int count = parseData.dst[1];
                    unsigned char *point = &parseData.dst[2];

                    stAllRealtimeFault faultRealtime;
                    faultRealtime.currentPage = parseData.dst[0];
                    for(int i=0; i<count; i++)
                    {
                        stRealtimeFault realtime;
                        memcpy((unsigned char *)&realtime, point, sizeof(stRealtimeFault));
                        faultRealtime.faultList.append(realtime);

                        point += sizeof(stRealtimeFault);
                    }
                    var.setValue(faultRealtime);
                }
            }break;
		case InfoChargeReport:				//充电完成报告
			{
                //qDebug() << "Charge report rcv datalen=" <<  parseData.len << "struct size=" << sizeof(stChargeReport);
				if( parseData.len == sizeof(stChargeReport) )
					var.setValue(*((stChargeReport *)parseData.dst));
			}break;
		case InfoExchangeParamResult:   //交互结果返回
			{
				if( parseData.len == sizeof(stExchangeResult) )
					var.setValue(*((stExchangeResult *)parseData.dst));
			}break;
		case InfoExchangeApplayChargeCmd:	//下发申请卡片信息
			{
			}break;
		case InfoExchangeApplayChargeResult:	//读申请开始充电/结束充电结果
			{
				if( parseData.len == sizeof(stApplayChargeResult) )
					var.setValue(*((stApplayChargeResult *)parseData.dst));
			}break;
		case InfoExchangeChargeCmd:	//下发启动充电/结束充电命令
			{
			}break;
        case InfoExchangeButtonStopCharge:	//按钮结束充电结果  add by songqb
            {
                if( parseData.len == sizeof(stButtonStopResult) )
                    var.setValue(*((stButtonStopResult *)parseData.dst));
            }break;
         case InfoExchangePrintPaper:       //打印小票结果   add by songqb
            {
                if( parseData.len == sizeof(stPrintPaperResult))
                    var.setValue(*((stPrintPaperResult *)parseData.dst));
            }break;
		case InfoExchangeChargeResult:	//读取启动充电/结束充电结果
			{
				if( parseData.len == sizeof(stChargeResult) )
					var.setValue(*((stChargeResult *)parseData.dst));
			}break;
		case InfoExchangeUpdateExportResult:	//升级 数据导出结果
			{
        //qDebug() << "-----InfoExchangeUpdateExportResult.......len1=" <<parseData.len << "len2=" << sizeof(_UpdateExportResult);
                if( parseData.len == sizeof(stUpdateExportResult) )
                {
                    //qDebug() << "-----InfoExchangeUpdateExportResult.......2........................";
                    var.setValue(*((stUpdateExportResult *)parseData.dst));
                }
			}break;
        case InfoConfigChargeSelectPassword:    //充电模式选择查询随机码
            {
                if(parseData.len == sizeof(unsigned int))
                    var.setValue(*((unsigned int *)parseData.dst));
            }break;
        case InfoConfigChangeChargePassword:    //充电密码
            {
                if(parseData.len == sizeof(unsigned int))
                    var.setValue(*((unsigned int *)parseData.dst));
            }break;

		default:break;
	}

	if(!var.isNull())
	{
		InfoMap Map;
		Map.insert((InfoBodyType)parseData.cmdSlave, var);
		bus->sendTodest(Map, (InfoAddrType)(parseData.cmdMaster & 0x7f));
	}
}
/**
 *对接收到socket过来的原始数据
 */
void TeuiProtocol::receivePackageDatas(QVariant var)
{
	stDataBuffer buffer= var.value<stDataBuffer>();

	unsigned char *point = buffer.buff;
	unsigned char *end = buffer.buff + buffer.len;

    //printf("rcv socket size =%d\n", buffer.len);
	while(point < end)
	{
		memset(&paraseData, 0, sizeof(paraseData));
		if(0 == rcvBaseCheck(point, end - point, &paraseData, &point))
			break;
		/*解析协议类型、数据长度、数据内容*/
		dealParaseDatas(paraseData);
	}
}

unsigned char TeuiProtocol::getSum(unsigned char *src, int len)
{
	unsigned char sum = 0;
	for(int i=0; i<len; i++)
		sum += src[i];
	return sum;
}

/**
 *整个数据包的组织
 *包含信息体  信息体内容
 */
int TeuiProtocol::organizeWholePack(unsigned char *data, int max_len, unsigned char master, unsigned short slave, unsigned char *info_data, int info_len)
{
	int	count = 0;

	data[count++] = 0x68;
	data[count++] = stationAddr;
	data[count++] = 0x68;

	data[count++] = 0x01;       //协议类型

	data[count++] = master;
	data[count++] = slave & 0xff;
	data[count++] = (slave & 0xff00)>>8;

	data[count++] = (unsigned char)(info_len & 0xff);
	data[count++] = (unsigned char)((info_len & 0xff00) >> 8);

	if(info_data != NULL && info_len > 0)
		memcpy(&data[count], info_data, info_len);
	count += info_len;

	data[count++] = getSum(data+3, count-3);

	data[count++] = 0x16;
	return count;
}
/**
 *数据包基础分析,判断数据包的格式解析出数据域
 */
int TeuiProtocol::rcvBaseCheck(unsigned char *src_data, int src_len, stParseData *data_area, unsigned char **next_point)
{
	unsigned char *src = src_data;

	if(src_len <= 0)
		return 0;

	if( src[0] == 0x68 && src[2] == 0x68 && src[1] == stationAddr )
	{

		int tmp_len = src[8];
		tmp_len <<= 8;
		tmp_len |= src[7];

		if(tmp_len > (src_len-11) )
			return 0;

		if( src[tmp_len+10] == 0x16 )
			//if( getSum(src+3, tmp_len+6) == src[tmp_len+9] && src[tmp_len+10] == 0x16 )
		{
			data_area->cmdMaster = src[4];

			data_area->cmdSlave = src[6];
			data_area->cmdSlave <<= 8;
			data_area->cmdSlave |= src[5];

            data_area->len = tmp_len;
            data_area->dst = (unsigned char *)&src[9];

            *next_point = src_data+tmp_len+11;

			return 1;
		}
	}

	return 0;
}
/**
 * @brief 发送协议数据
 * @param var
 * @param Type
 */
int TeuiProtocol::sendProtocolData(InfoProtocol Info, InfoAddrType Type)
{
    if(Info.size() < 2)
        return 0;

    QByteArray typeRcv = Info.value(InfoDataType);
    Info.remove(InfoDataType);

    InfoProtocol::iterator it;
    unsigned char Master = Type | (typeRcv.data()[0] << 7);
    for( it = Info.begin(); it !=  Info.end(); ++it)
    {
        QByteArray  value= it.value();
        InfoBodyType key = it.key();

        unsigned char *dstData = new unsigned char[value.size()+20];
        int len = organizeWholePack(dstData, value.size()+20, Master , key, (unsigned char *)value.data(), value.size());
		if(len)
            emit sendPackageDatas(dstData, len);
        delete []dstData;
    }

    return 1;
}
