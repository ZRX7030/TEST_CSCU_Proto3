#!/bin/sh

#添加默认的网关

config=/mnt/nandflash/config.ini

#dns配置
echo "nameserver 114.114.114.114" > /etc/resolv.conf
if [ -f $config ]; then
	dns=`cat $config | sed -n -e"/^DNS=/p" | awk -F"=" '{print $2}'`
	echo "nameserver $dns" >> /etc/resolv.conf
fi

#ip 网关配置
if [ "$1" == "eth0" ]; then
	cat $config | sed -n -e"/^\[NET_0\]/,/^\[/p" | sed -e "/^\[/d" -e "/^$/d"  -e "s/=/ /" > /tmp/net.tmp
elif [ "$1" == "eth1" ]; then
	cat $config | sed -n -e"/^\[NET_1\]/,/^\[/p" | sed -e "/^\[/d" -e "/^$/d"  -e "s/=/ /" > /tmp/net.tmp
else
	exit 1
fi

ip=`cat /tmp/net.tmp | grep "IP" | awk '{print $2}'`
netmask=`cat /tmp/net.tmp | grep "NetMask" | awk '{print $2}'`
gw=`cat /tmp/net.tmp | grep "Gateway" | awk '{print $2}'`

if [ -z "$ip" ] || [ -z "$netmask" ] || [ -z "$gw" ]; then
	ip=10.0.10.123
	netmask=255.255.255.0
	gw=10.0.10.254
fi

ifconfig $1 $ip netmask $netmask up
route add default gw $gw metric 300

rm -f /tmp/net.tmp
