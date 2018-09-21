#!/bin/sh

script_path=/mnt/nandflash
mount_path=/tmp/usb.tmp
dst_path=/
user_path=/mnt/nandflash
tmp_path=/mnt/nandflash/download/tmp
file_name=/mnt/nandflash/download/setup.tar.gz
config_file=/mnt/nandflash/config.ini

update_sys()
{
	if [ -f "$file_name" ]; then
	
		[ -f "$config_file" ] && cp $config_file /tmp

		tar xf $file_name -C $dst_path                         
		if [ ! $? -eq 0 ]; then
			echo "update.sh-----------decompression error"
			rm -f $file_name                                        
			return $?
		fi
		
		rm -f $file_name                                        
	
		#远程升级包中有升级脚本，执行远程升级脚本
		if [ -f "/mnt/nandflash/remote_update.sh" ]; then
			echo "update.sh-----------exec remote_update.sh"
			chmod +x /mnt/nandflash/remote_update.sh
			/mnt/nandflash/remote_update.sh
			rm -f /mnt/nandflash/remote_update.sh			
			return $?
		fi
		
		#rm -rf $tmp_path/*

		echo "update.sh-----------update success"
		return 0
	fi
	return 1
}

#http://ip:port/dir/filname|md5
http_resolve()
{
	uri=`echo "$1" | awk -F\| '{print $1}'`
	md5=`echo "$1" | awk -F\| '{print $2}'`
	return 0
}
#自动升级下载
http_download()
{
	local ret param_user param_outfile

	http_resolve $2
	echo "http_download resolve ok"
	if [ $? -eq 1 ]; then
		echo "http_download faild"
		return 1
	fi

	#正在进行下载
	echo "$1""$1" > "/tmp/download.result"

	#输出文件判断
	if [ $1 -eq 3 ]; then
		param_file=$file_name
	elif [ $1 -eq 4 ]; then
		param_file=/mnt/nandflash/download/terminal.bin
	fi

	wget -O $param_file $uri
	filemd5=`md5sum $param_file`

	#下载结果处理
	if [ $? -eq 0  -a "$md5"="$filemd5" ]; then
		# success
		echo "$1""0" > "/tmp/download.result"
		ret=0
	else
		echo "0" > "/tmp/download.result"
		ret=1
	fi
	return $ret
}


#ftp://ip:port/dir/fileneam|user|pwd
url_resolve()
{
	uri=`echo "$1" | awk -F\| '{print $1}' | sed "s/ftp:\/\///"`
	ipport=`echo "$uri" | awk -F\/ '{print $1}'`
	
	dir=`echo $uri | sed "s/$ipport\///"`
	
	tmp=`echo "$ipport" | grep ":"`
	if [ -z "$tmp" ]; then
		ip=$ipport
		port=21
	else
		ip=`echo $ipport | awk -F: '{print $1}'`
		port=`echo $ipport | awk -F: '{print $2}'`
	fi
	
	user=`echo "$1" | awk -F\| '{print $2}'`
	password=`echo "$1" | awk -F\| '{print $3}'`

	if [ -z "$ip" -o -z "$port" -o -z "$dir" -o -z "$user" -o -z "$password" ]; then
		return 1
	else
		return 0
	fi
}

ftp_download()
{
	url_resolve $2
	echo "ftp_download resolve ok"
	if [ $? -eq 1 ]; then
		echo "ftp_download faild"
		return 1
	fi
	if [ $1 -eq 3 ]; then
		ftpget -P $port -u $user -p $password $ip $file_name $dir
	elif [ $1 -eq 4 ]; then
		ftpget -P $port -u $user -p $password $ip "/mnt/nandflash/download/terminal.bin" $dir
	fi
}

ftp_upload()
{
	local station1 station2 curr_date filename
	
	url_resolve $2
	echo "ftp_upload url resolve ok"
	if [ $? -eq 1 ]; then
		return 1
	fi
	
	curr_date=`date "+%Y%m%d%H%M%S"`
	
	if [ -f $user_path/config.ini ]; then
		station1=`cat $user_path/config.ini | sed -n -e"/^\[SERVER0\]/,/^\[/p" | sed -n  "/^StationNo=/p" | sed "s/StationNo=//"`
		station2=`cat $user_path/config.ini | sed -n -e"/^\[SERVER1\]/,/^\[/p" | sed -n  "/^StationNo=/p" | sed "s/StationNo=//"`
	else
		station1=""
		station2=""
	fi

	if [ $1 == 1 ]; then
		filename="$station1""_""$station2""_""$curr_date""_log".tar.gz
		
		cp -f $user_path/config.ini $user_path/log/
		cp -f $user_path/libconfig.ini $user_path/log/
		
		tar czvf $user_path/$filename $user_path/log $user_path/database
	elif [ $1 == 2 ]; then
		filename="$station1""_""$station2""_""$curr_date""_config".tar.gz
		tar czvf $user_path/$filename $user_path/config.ini $user_path/libconfig.ini
	else
		return 1;
	fi

	ftpput -P $port -u $user -p $password $ip $dir/$filename $user_path/$filename
	res=$?
	rm -f $user_path/$filename
	return $res
}

ui_download()
{
	local usb_mount
	usb_mount=`cat $mount_path`
	if [ ! -z "$usb_mount" ] && [ -f "$usb_mount/setup.tar.gz" ]; then
		cp -f $usb_mount/setup.tar.gz /mnt/nandflash/download/
	else
		return 1
	fi
}

ui_upload()
{
	if /mnt/nandflash/bin/usb_mount_check.sh 0; then
		
		local curr_date station1 station2 filename
	
		cp -f $user_path/config.ini $user_path/log/ || return 1
		cp -f $user_path/libconfig.ini $user_path/log/ || return 1

		curr_date=`date "+%Y%m%d%H%M%S"`
		
		if [ -f $user_path/config.ini ]; then
			station1=`cat $user_path/config.ini | sed -n -e"/^\[SERVER0\]/,/^\[/p" | sed -n  "/^StationNo=/p" | sed "s/StationNo=//"`
			station2=`cat $user_path/config.ini | sed -n -e"/^\[SERVER1\]/,/^\[/p" | sed -n  "/^StationNo=/p" | sed "s/StationNo=//"`
		else
			station1=""
			station2=""
		fi

		filename="$station1""_""$station2""_""$curr_date""_log.tar.gz"
		usb_mount=`cat $mount_path`	
		
		tar czvf $user_path/$filename  $user_path/log $user_path/database
		cp -f $user_path/$filename $usb_mount
		
		if [ $? -eq 0 ];then
			rm -f $user_path/$filename
			return 0 
		else
			rm -f $user_path/$filename
			return 1
		fi
	fi
	
	return 1
}

if [ ! -d $tmp_path ]; then
	mkdir -p $tmp_path
fi
#参数：$1 命令来源 $2 命令主类型 $3 命令从类型 $4 命令参数
case $1 in
	#网络操作
	1)
	case $2 in
		1)
		local netres
		
		ftp_download $3 $4
		if [ ! $? -eq 0 ]; then
			usb_path=`cat $mount_path`
			umount $usb_path
			exit 1
		fi

		if [ $3 -eq 3 ]; then
			update_sys
			netres=$?
		elif [ $3 -eq 4 ]; then
			netres=$?
		else
			netres=1
		fi
		exit $netres
		;;
		2)
		ftp_upload $3 $4	
		exit $?
		;;
	esac
	;;
	#UI操作
	2)
	case $2 in
		1)
		ui_download $3 $4
		if [ ! $? -eq 0 ]; then
			res=1
			usb_path=`cat $mount_path`
			umount $usb_path
			exit $res
		fi
		update_sys
		res=$?
		usb_path=`cat $mount_path`
		umount $usb_path
		exit $res
		;;
		2)
		ui_upload $3 $4	
		res=$?
		usb_path=`cat $mount_path`
		umount $usb_path
		exit $res
		;;
	esac
	;;
	#网络自动操作
	3)
	case $2 in
		1)
			http_download $3 $4
			exit $?
			;;
		2)
			update_sys
			exit $?
			;;
	esac
	;;
	*)
	exit 1
	;;
esac

