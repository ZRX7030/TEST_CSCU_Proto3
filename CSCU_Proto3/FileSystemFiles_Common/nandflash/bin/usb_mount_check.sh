#!/bin/sh

#$1: board type;   
#1: standard type; 2: 2.0 type; 3: module type 

dev=0
mount_path=/tmp/usb.tmp

echo  "" > $mount_path

if [ $1 -eq 3 ] ;then
	dev=`fdisk -l | sed -n "/^\/dev\/sda/p" | awk '{print $1}'`
	if [ -z "$dev" ]; then
		dev=`fdisk -l | sed -n "/^\/dev\/sdb/p" | awk '{print $1}'`
	fi
else
	dev=`fdisk -l | sed -n "/^\//p" | awk '{print $1}'`
fi

if [ -z "$dev" ]; then
	exit 1	
else
	mount_dir=`df -h | grep $dev | awk '{print $6}'`
	if [ -z "$mount_dir" ]; then
		exit 1
	else
		echo  "$mount_dir/" > $mount_path
		[ $2 -eq 1 ] && udisk_buzzer_out.sh &
		exit 0
	fi
fi
