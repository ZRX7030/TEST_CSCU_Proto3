#!/bin/sh

file=/tmp/version.info

mac_addr=`ifconfig | grep eth0 | awk '{print $5}' | sed "s/://g"`
cscu_ver=`cat /mnt/nandflash/config.ini | sed -n -e"/^\[CSCUSys\]/,/^\[/p" | sed -n  "/^Version=/p" | sed "s/Version=//"`
teui_ver=""

echo "$cscu_ver" > $file
echo "$teui_ver" >> $file
echo "$mac_addr" >> $file

