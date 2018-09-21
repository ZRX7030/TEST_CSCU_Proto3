#!/bin/sh

file=/tmp/version.info

mac_addr=`ifconfig | grep eth0 | awk '{print $5}'`

kernel_ver=`cat /proc/version_tgood | sed -n -e"/^\[TGOOD Kernel Version\]/,/^\[/p" | sed -n  "/^Version=/p" | sed "s/Version=//"`
hd_ver=`cat /proc/version_tgood | sed -n -e"/^\[TGOOD Hardware Version\]/,/^\[/p" | sed -n  "/^Version=/p" | sed "s/Version=//"`
filesys_ver=`cat /etc/version_tgood  | sed -n  "/^Version=/p" | sed "s/Version=//"`
cscu_ver=`cat /mnt/nandflash/config.ini | sed -n -e"/^\[CSCUSys\]/,/^\[/p" | sed -n  "/^Version=/p" | sed "s/Version=//"`
teui_ver=""
Product_SN=`cat /tmp/Product_SN.txt`
echo "$kernel_ver" > $file
echo "$filesys_ver" >> $file
echo "$cscu_ver" >> $file
echo "$teui_ver" >> $file
echo "$hd_ver" >> $file
echo "$mac_addr" >> $file
echo "$Product_SN" >>$file

# Yannis
/mnt/nandflash/patch/patch.sh

