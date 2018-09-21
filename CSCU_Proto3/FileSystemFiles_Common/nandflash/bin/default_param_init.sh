#!/bin/sh

CHARGE_STOP_REASON_FILE=/mnt/nandflash/etc/charge_stop_name.conf
ORDER_STOP_REASON_FILE=/mnt/nandflash/etc/order_stop_name.conf
ORDER_START_REASON_FILE=/mnt/nandflash/etc/order_start_name.conf
ORDER_CMDSRC_REASON_FILE=/mnt/nandflash/etc/order_cmdsrc_name.conf
ORDER_FAULT_CODE_FILE=/mnt/nandflash/etc/fault_name.conf
DATABASE=/mnt/nandflash/database/param_config.db

file_name1=$CHARGE_STOP_REASON_FILE
file_name2=$ORDER_STOP_REASON_FILE
file_name3=$ORDER_START_REASON_FILE
file_name4=$ORDER_CMDSRC_REASON_FILE
file_name5=$ORDER_FAULT_CODE_FILE

table_name1=charge_stop_name_table
table_name2=order_stop_name_table
table_name3=order_start_name_table
table_name4=order_cmdsrc_name_table
table_name5=fault_name_table

for i in $(seq 1 5)
do
	eval table_name="$""table_name""$i"
	eval file_name="$""file_name""$i"
	
	sql="select count(*) from $table_name"
	row_num=`sqlite3 "$DATABASE" "$sql"`
	if [ "$row_num" == "0" ]; then
		while read line
		do
			code=`echo $line | awk '{print $1}'`
			name=`echo $line | awk '{print $2}'`

			sql="insert into $table_name (code, name) values("$code", \"$name\")"
			sqlite3 "$DATABASE" "$sql"
		done < "$file_name"
	fi
done

