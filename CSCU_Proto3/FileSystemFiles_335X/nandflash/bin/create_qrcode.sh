#!/bin/sh

QRCMD=/mnt/nandflash/bin/qrencode

sqlite3 /mnt/nandflash/database/param_config.db "select canaddr,code from terminal_code_param_table" > /tmp/code_param

while read line
do
	qrcode=`echo "$line" | awk -F\| '{print $2}'`
	canaddr=`echo "$line" | awk -F\| '{print $1}'`
	qrpng="/tmp/""$canaddr"".png"

	if [ ! -z "$qrpng" ] && [ ! -z "$qrcode" ]; then
		$QRCMD -o $qrpng -t PNG --background=ffffff00 $qrcode
	fi
done  < /tmp/code_param

rm -f /tmp/code_param
