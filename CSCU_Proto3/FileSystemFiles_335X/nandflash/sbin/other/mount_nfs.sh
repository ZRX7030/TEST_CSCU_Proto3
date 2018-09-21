# Auto mount VM NFS rootfs to Board.
# Yannis, 20170223

#!/bin/bash

VM_IP_ADDR=172.0.0.11
MY_IP_ADDR=172.0.0.12
shared=/mnt/shared

if ! [ -d $shared ]; then
	mkdir $shared
fi

ifconfig eth0 $MY_IP_ADDR/16 up

mount -t nfs -o nolock $VM_IP_ADDR:/home/forlinx/shared $shared

cd $shared

