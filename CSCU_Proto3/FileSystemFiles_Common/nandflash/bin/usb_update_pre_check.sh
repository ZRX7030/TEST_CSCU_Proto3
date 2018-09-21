#!/bin/sh


mount_path=/tmp/usb.tmp
usb_mount=`cat $mount_path`
result_name=/tmp/no_usb_check.tmp

if [ -f $result_name ]; then
        rm $result_name
fi

if [ -z "$usb_mount" ] || [ ! -f "$usb_mount/setup.tar.gz" ]; then
#        touch /tmp/no_usb_check
        echo 2
else
		tar -tf $usb_mount/setup.tar.gz
		if [ "$?" != "0" ]; then
			echo 3   #校验失败
		else
			echo 255  #校验OK 允许升级
		fi
fi
