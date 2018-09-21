#!/bin/sh

CONFIG_FILE=/mnt/nandflash/config.ini
PATH_DISK=/mnt/nandflash/database

. /mnt/nandflash/bin/functions

#时间计算
current_time=`date +%Y-%m-%d\ %H:%M:%S`
time_base=$(date +%s -d "$current_time")

diff_pro=$((30*24*60*60))
diff_real=$((15*24*60*60))

time_pro=`second_to_time $(($time_base-$diff_pro))`
time_real=`second_to_time $(($time_base-$diff_real))`

#删除静态表数据
sql="delete from charge_order where EndTime < \"$time_pro\" and EndTime != \"\""
sqlite3  $PATH_DISK/process_record.db "$sql"

sql="delete from plug_gun_table where pull_record_time < \"$time_pro\""
sqlite3  $PATH_DISK/process_record.db "$sql"

sql="delete from terminal_online_table where charger_offline_time < \"$time_pro\""
sqlite3  $PATH_DISK/process_record.db "$sql" 

sql="delete from dc_cabinet_fault_table where stop_time < \"$time_pro\" and stop_time != \"\""
sqlite3  $PATH_DISK/process_record.db "$sql" 

sql="delete from terminal_fault_table where fault_stop_time < \"$time_pro\" and fault_stop_time != \"\""
sqlite3  $PATH_DISK/process_record.db "$sql" 

sql="delete from cscu_online_table where terminal_offline_time < \"$time_pro\""
sqlite3  $PATH_DISK/process_record.db "$sql" 

sql="delete from cscu_reboot_table where record_time < \"$time_pro\""
sqlite3  $PATH_DISK/process_record.db "$sql" 

sql="delete from bms_static_table where record_time < \"$time_pro\""
sqlite3  $PATH_DISK/process_record.db "$sql" 

sql="delete from operate_record_table where opt_time < \"$time_pro\""
sqlite3  $PATH_DISK/process_record.db "$sql" 


#动态数据表的删除
if [ -f "$CONFIG_FILE" ]; then
	num_1=`cat "$CONFIG_FILE" | sed -n "/^SinglePhase=/p" | sed "s/SinglePhase=//g"`
	num_2=`cat "$CONFIG_FILE" | sed -n "/^ThreePhase=/p" | sed "s/ThreePhase=//g"`
	num_3=`cat "$CONFIG_FILE" | sed -n "/^DirectCurrent=/p" | sed "s/DirectCurrent=//g"`
else
	num_1=0
	num_2=0
	num_3=1
fi

#删除直流独有数据
for i in $(seq 181 $((180+$num_3)))
do
	table_name="bms_dynamic_""$i""_table";
	sql="delete from $table_name where record_time < \"$time_real\""
	sqlite3 $PATH_DISK/real_record.db "$sql" 
done

#删除直流、交流公有数据
arrayb1=$(($num_1))  
arrayb2=$(($num_2+150))
arrayb3=$(($num_3+180))
arraya1=1
arraya2=151 
arraya3=181
for k in $(seq 1 3)
do
	eval count="$""num_""$k"
	eval start="$""arraya""$k"
	for i in $(seq $start $(($start+$count-1)))
	do
		table_name="charge_process_""$i""_table";
		sql="delete from $table_name where record_time < \"$time_real\""
		sqlite3 $PATH_DISK/real_record.db "$sql" 

		table_name="charge_energy_""$i""_table";
		sql="delete from $table_name where NowTime < \"$time_pro\""
		sqlite3 $PATH_DISK/real_record.db "$sql" 
	done
done
