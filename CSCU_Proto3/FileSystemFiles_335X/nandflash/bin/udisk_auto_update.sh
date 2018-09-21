#!/bin/sh

if [ -f /mnt/nandflash/etc/autoupdate.conf ]; then
	res cmd result flag
	flag=0
	res=0
	while read line
	do 
		cmd result	
		cmd=`echo $line | awk '{print $1}'`
		result=`echo $line | awk '{print $2}'`
		[ "$cmd" = "update"  -a $result -eq 1 ] && res=1 && break
	done < /mnt/nandflash/etc/autoupdate.conf
	
	if [ $res -eq 1 ]; then
		for i in $(seq 1 8)
		do
			if /mnt/nandflash/bin/usb_mount_check.sh 0 0; then
				/mnt/nandflash/bin/update.sh 2 1 3 0
				flag=1
				break
			else
				sleep 2	
			fi
		done
	fi
	rm -f /mnt/nandflash/etc/autoupdate.conf
	
	if [ $flag -eq 1 ]; then
#update success
		echo 1 > /tmp/update-result.tmp
		buzzer-out.sh
#		io-out 26 0
#		usleep 100000
#		io-out 26 1
	else 
		echo 0 > /tmp/update-result.tmp
	fi
fi
