#!/bin/sh

remote_conf=/mnt/nandflash/etc/remote.conf
pwd_file=/mnt/nandflash/etc/openvpn/user-pass.txt
config_file=/mnt/nandflash/config.ini

echo "start openvpn....."

if [ -f $config_file ]; then
	station1=`cat $config_file | sed -n -e"/^\[SERVER0\]/,/^\[/p" | sed -n  "/^StationNo=/p" | sed "s/StationNo=//"`
	station2=`cat $config_file | sed -n -e"/^\[SERVER1\]/,/^\[/p" | sed -n  "/^StationNo=/p" | sed "s/StationNo=//"`
else
	station1=""
	station2=""
	echo  "$config_file not exist."
fi

macaddr=`ifconfig eth0 | grep HWaddr | awk '{print $5}' | sed "s/://g"`

echo "$macaddr-$station1-$station2" > $pwd_file
echo "teldvpn" >> $pwd_file

if [ -f "$remote_conf" ]; then
	host=`cat $remote_conf | grep "vpn" | awk '{print $2" "$3}'`
else
	host="D-BJ-3rdCOM.chinacloudapp.cn 1194"
fi

sed -i "s/^remote .*/remote $host/" /mnt/nandflash/etc/openvpn/client.conf

openvpn --cd /mnt/nandflash/etc/openvpn/ --config client.conf --auth-user-pass user-pass.txt  script-security 3 > /dev/null &
