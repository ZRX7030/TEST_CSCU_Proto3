#!/bin/sh

REMOTE_CONF=/mnt/nandflash/etc/remote.conf
CONFIG_FILE=/tmp/config.ini.tmp

exit 1

if [ ! -f "/mnt/nandflash/bin/upload" ]; then
	echo "upload program  not exit."
	exit 1
fi

net_flag=false
for i in $(seq 1 2)
do
	ret_code=`curl -I -s --connect-timeout 10 www.baidu.com -w %{http_code} | tail -n1`
	if [ "$ret_code" = "200" ]; then
		net_flag=true
		break
	else
		sleep 600
	fi
done	

if [ "$net_flag" = "false" ]; then
	exit 1
fi

cp -f /mnt/nandflash/config.ini $CONFIG_FILE

station=""
if [ -f "$CONFIG_FILE" ]; then
	ac1num=`cat "$CONFIG_FILE" | grep "SinglePhase" | sed "s/SinglePhase=//g"`
	ac3num=`cat "$CONFIG_FILE" | grep "ThreePhase" | sed "s/ThreePhase=//g"`
	dcnum=`cat "$CONFIG_FILE" | grep "DirectCurrent" | sed "s/DirectCurrent=//g"`
	station=`cat $CONFIG_FILE | sed -n -e"/^\[SERVER0\]/,/^\[/p" | sed -n  "/^StationNo=/p" | sed "s/StationNo=//"`
	#station=`cat "$CONFIG_FILE" | grep "StationNo" | sed "s/StationNo=//g"`
else 
	exit 1
fi

if [ -f "$REMOTE_CONF" ]; then
	host=`cat $REMOTE_CONF | grep "webservice" | awk '{print $2":"$3}'`
else
	host="D-BJ-3rdCOM.chinacloudapp.cn:8080"
fi

random=$(($RANDOM%3000))
sleep $random
/mnt/nandflash/bin/upload -H "$host" -a "$station"  -s $ac1num -t $ac3num -d $dcnum -m 1000
