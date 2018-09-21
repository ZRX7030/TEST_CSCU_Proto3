#!/bin/sh

# 把指定的整机条码字符串写入文件，并记录执行结果
# Yannis, 20170724

result_file="/tmp/test_base_board_result.ini"
partition=/dev/mtdblock11
partition_char=/dev/mtd11
mount_point=/mnt/product_infor
file_name=Product_SN.txt

# write_result item result
function write_result()
{
    local item=$1
    local result=$2
    
    if [ $result -eq 0 ]; then
        ini_operator write $result_file BASE_BOARD $item OK
    else
        ini_operator write $result_file BASE_BOARD $item ERR
    fi
    echo
    echo "================================================================================"
    echo
}

function func_failed()
{
    echo
    echo "Failed!"
    write_result "SN_STATUS" 1
    exit 1
}

if [ $# -ne 1 ]; then
    echo "Syntax error!"
    echo "Usage: $0 string"
    func_failed
fi

if [ $(echo "$1" | wc -L) -ne 23  ]; then
    echo "Error: SN length != 23"
    func_failed
fi

# Unmount(if mounted) before formatting
umount $partition

# Erase the partition (And format it for yaffs2)
flash_erase $partition_char 0 0 
if [ $? -ne 0 ]; then
    func_failed
fi

# Mount with read & write property
mount $partition $mount_point -o rw 
if [ $? -ne 0 ]; then
    func_failed
fi

# Write SN to file 
echo $1 > $mount_point/$file_name
if [ $? -ne 0 ]; then
    func_failed
fi

sync

# Do not use 'remount' here, or the data on this partition 
# will be lost after a sudden power-loss.
#mount $partition -o remount,ro
#if [ $? -ne 0 ]; then
#    func_failed
#fi

# Unmount so that we can mount again
umount $partition
if [ $? -ne 0 ]; then
    func_failed
fi

# Mount again with read only property
mount $partition $mount_point -o ro 

write_result "SN_STATUS" 0

