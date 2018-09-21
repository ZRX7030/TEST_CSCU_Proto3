#!/bin/sh

config1=/tmp/config.ini
config2=/mnt/nandflash/config.ini

[ -f "$config1" ] && version=`sed -n "/CSCU_A1_/p" $config1 | awk -F '_' '{print $3}'`
[ -z "$version" ] && [ -f "$config2" ] && version=`sed -n "/CSCU_A1_/p" $config2 | awk -F '_' '{print $3}'`
[ -z "$version" ] && exit 1

version_cmp()
{
	if [ -n "$1" -a -n "$2" -a "$1" -lt "$2" ]; then
		echo "Current ${1} Target ${2} And Execute Update..."
		return 0
	fi

	return 1
}

patch_2508()
{
	local target=2508
	version_cmp $version $target
	[ "$?" -ne "0" ] && return $?

	temp=/tmp/charge_order.sql
	echo  "ALTER TABLE charge_order ADD OrderType integer default 1;
	ALTER TABLE charge_order ADD ChargeType integer default 0;
	ALTER TABLE charge_order ADD ChargeWay integer default 0;
	ALTER TABLE charge_order ADD LimitEnergy integer default 0;
	ALTER TABLE charge_order ADD GunNum integer default 1;
	ALTER TABLE charge_order ADD QueueGroup integer default 0;
	ALTER TABLE charge_order ADD QueueIndex integer default 0;
	ALTER TABLE charge_order ADD OrderSync integer default 0;" > $temp
	/mnt/nandflash/bin/sqlite3 /mnt/nandflash/database/process_record.db < $temp > /dev/null 2>&1
	rm -fr $temp
}

patch_2513()
{
	local target=2513
	version_cmp $version $target
	[ "$?" -ne "0" ] && return $?

	temp=/tmp/ammeter_param_table.sql 
	echo  "ALTER TABLE ammeter_param_table ADD fun_type integer default 3;" > $temp > /dev/null 2>&1
	/mnt/nandflash/bin/sqlite3 /mnt/nandflash/database/param_config.db < $temp 
	rm -fr $temp
}
patch_2604()                                                                    
{                                                                               
        local target=2604                                                       
        version_cmp $version $target                                            
        [ "$?" -ne "0" ] && return $?                                           
                                                                                
        temp=/tmp/bms_static_table.sql                                          
        echo  "DROP TABLE if exists bms_static_table ;" > $temp > /dev/null 2>&1
        /mnt/nandflash/bin/sqlite3 /mnt/nandflash/database/process_record.db < $temp > /dev/null 2>&1
        rm -fr $temp                                                            

	temp=/tmp_dc_cabinet_fault.sql
	echo "DROP TABLE if exists dc_cabinet_fault_table ;" > $temp > /dev/null 2>&1
	/mnt/nandflash/bin/sqlite3 /mnt/nandflash/database/process_record.db < $temp > /dev/null 2>&1
	rm -fr $temp

	temp=/tmp/dcstatic_param_table.sql
        echo  "ALTER TABLE dcstatic_param_table ADD bms_pro_type integer default 0;" > $temp > /dev/null 2>&1
        /mnt/nandflash/bin/sqlite3 /mnt/nandflash/database/param_config.db < $temp
        rm -fr $temp

}                                                                               
   

patch_2514()
{
	local target=2514
	version_cmp $version $target
	[ "$?" -ne "0" ] && return $?

	temp=/tmp/process_record.sql 
	echo  "DROP TABLE  IF EXITS bms_static_table;" > $temp > /dev/null 2>&1
	/mnt/nandflash/bin/sqlite3 /mnt/nandflash/database/process_record.db $temp 
	rm -fr $temp 
}

patch_26052()
{
        local target=26052
        version_cmp $version $target
        [ "$?" -ne "0" ] && return $?

        temp=/tmp/charge_order.sql
		echo  " ALTER TABLE charge_order ADD customerID text default '';
			ALTER TABLE dc_cabinet_fault_table ADD serialnum integer default 0;
			ALTER TABLE dc_cabinet_fault_table ADD record_state integer default 0;
			ALTER TABLE bms_static_table ADD canAddr integer default 0;" > $temp
        /mnt/nandflash/bin/sqlite3 /mnt/nandflash/database/process_record.db < $temp > /dev/null 2>&1
        rm -fr $temp
}

patch_26060()
{
if [ -f "$config2" ]; then
        num_1=`cat "$config2" | sed -n "/^SinglePhase=/p" | sed "s/SinglePhase=//g"`
        num_2=`cat "$config2" | sed -n "/^ThreePhase=/p" | sed "s/ThreePhase=//g"`
        num_3=`cat "$config2" | sed -n "/^DirectCurrent=/p" | sed "s/DirectCurrent=//g"`
else
        num_1=0
        num_2=0
        num_3=1
fi

arrayb1=$(($num_1))  
arrayb2=$(($num_2+150))
arrayb3=$(($num_3+180))
arraya1=1
arraya2=151 
arraya3=181
        local target=26060
        version_cmp $version $target
        [ "$?" -ne "0" ] && return $?

	temp=/tmp/dcstatic_param_table.sql
        echo  "ALTER TABLE dcstatic_param_table ADD bms_pro_type integer default 0;" > $temp
        /mnt/nandflash/bin/sqlite3 /mnt/nandflash/database/param_config.db < $temp
        rm -fr $temp
	
	for k in $(seq 1 3)
	do
		eval count="$""num_""$k"
   	        eval start="$""arraya""$k"
	        
		for i in $(seq $start $(($start+$count-1)))
        	do
			temp=/tmp/tables_record.template
                	echo  " ALTER TABLE charge_energy_"$i"_table ADD RealTime datetime default '';" > $temp
		        /mnt/nandflash/bin/sqlite3 /mnt/nandflash/database/real_record.db < $temp > /dev/null 2>&1
        	done
	done
        rm -fr $temp
}

patch_config()
{
	/mnt/nandflash/bin/sqlite3 /mnt/nandflash/database/param_config.db "delete from charge_stop_name_table"
	/mnt/nandflash/bin/sqlite3 /mnt/nandflash/database/param_config.db "delete from order_stop_name_table"
	/mnt/nandflash/bin/sqlite3 /mnt/nandflash/database/param_config.db "delete from order_start_name_table"
	/mnt/nandflash/bin/sqlite3 /mnt/nandflash/database/param_config.db "delete from order_cmdsrc_name_table"
	/mnt/nandflash/bin/sqlite3 /mnt/nandflash/database/param_config.db "delete from fault_name_table"
}

patch_2508
patch_2513
patch_2514
patch_2604
patch_26052
patch_26060
patch_config
