#!/bin/sh

work_path=/mnt/nandflash
setup_file=setup.tar.gz
update=$work_path/remote_update.sh

if [ -f "$setup_file" ]; then
	tar -xf $setup_file -C /
	if [ -f "$update" ]; then 
		$update
		rm -f $update
	fi
	exit 0
else
	echo "Error : File \"${setup_file}\" not found!"
	exit 1
fi
