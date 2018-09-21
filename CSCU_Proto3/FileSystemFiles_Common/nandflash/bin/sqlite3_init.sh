#!/bin/sh

CONFIG_FILE=/mnt/nandflash/config.ini
PATH_DISK=/mnt/nandflash/database
PATH_TOOL=/mnt/nandflash/sql
TMP_FILE=/tmp/table_create.tmp

echo "database start init...."
#sqlite3 $PATH_DISK/data_retry.db .tables 

if [ ! -d "$PATH_DISK" ]; then
	mkdir -p $PATH_DISK
fi

if [ -f /mnt/nandflash/etc/database.conf ]; then
	while read line
	do 
		cmd result	
		cmd=`echo $line | awk '{print $1}'`
		result=`echo $line | awk '{print $2}'`
		[ "$cmd" = "del" ] && [ ! -z $result ] && rm -rf $PATH_DISK/$result
	done < /mnt/nandflash/etc/database.conf
	
	rm -f /mnt/nandflash/etc/database.conf
fi

#创建表
sqlite3 $PATH_DISK/down_save.db < $PATH_TOOL/tables_down_save.sql
sqlite3 $PATH_DISK/process_record.db < $PATH_TOOL/tables_process_record.sql
sqlite3 $PATH_DISK/param_config.db < $PATH_TOOL/tables_param.sql
sqlite3 $PATH_DISK/authentication.db < $PATH_TOOL/tables_authentication.sql

if [ -f "$CONFIG_FILE" ]; then
	num_1=`cat "$CONFIG_FILE" | sed -n "/^SinglePhase=/p" | sed "s/SinglePhase=//g"`
	num_2=`cat "$CONFIG_FILE" | sed -n "/^ThreePhase=/p" | sed "s/ThreePhase=//g"`
	num_3=`cat "$CONFIG_FILE" | sed -n "/^DirectCurrent=/p" | sed "s/DirectCurrent=//g"`
else
	num_1=0
	num_2=0
	num_3=1
fi
echo "sqlite3_init.sh find SinglePhase,ThreePhase,DirectCurrent"  $num_1,$num_2,$num_3
#创建直流
cp -f $PATH_TOOL/tables_real_record_single.template /tmp/tables_record.template
for i in $(seq 181 $((180+$num_3)))
do
	sed "s/CHANGE/$i/g" /tmp/tables_record.template > ${TMP_FILE}
	sqlite3  $PATH_DISK/real_record.db < ${TMP_FILE}
done
#创建交流、直流
cp -f $PATH_TOOL/tables_real_record_all.template /tmp/tables_record.template
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
		sed "s/CHANGE/$i/g" /tmp/tables_record.template > ${TMP_FILE}
		sqlite3  $PATH_DISK/real_record.db < ${TMP_FILE}
	done
done
#删除缓存文件
rm -f /tmp/tables_record.template
rm -f ${TMP_FILE}

#删除多于的数据
array_table_name1="phasetype_param_table" 
array_table_name2="terminal_param_table" 
array_table_name3="status_save_table" 
array_table_name4="charge_save_table"

arrayb1=$(($num_1))  
arrayb2=$(($num_2+150))
arrayb3=$(($num_3+180))

arraya1=150
arraya2=180 
arraya3=250

for i in  $(seq 1 4) 
do      
	for k in $(seq 1 3)
	do
		eval table_name="$""array_table_name""$i"
		eval canaddr_start="$""arrayb""$k"
		eval canaddr_stop="$""arraya""$k"

		exec_sql="delete from $table_name where canaddr > $canaddr_start and canaddr < $canaddr_stop"
		if [ $i -gt 2 ];then
			sqlite3 $PATH_DISK/down_save.db "$exec_sql"
		else
			sqlite3 $PATH_DISK/param_config.db "$exec_sql"
		fi
	done
done
#初始化默认名字
/mnt/nandflash/bin/default_param_init.sh
echo "database init end......"
