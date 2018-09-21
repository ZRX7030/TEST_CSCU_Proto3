#!/bin/sh

# 功能：将用户给定的字符串写入Flash分区/dev/mtd8的指定位置（详见file_to_mtd8.sh）
# 注意：此脚本会在写完string之后，额外写入一个换行符'0x0A'
# 作者：Yannis, 20170525

result_file="/tmp/test_base_board_result.ini"

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


if [ $# -ne 1 ]; then
    echo "Syntax error!"
    echo "Usage: $0 string"
    exit 1
fi

string=$1
file="/tmp/write_to_mtd8.txt"   #要写入Flash的条码文件

echo $string > $file
file_to_mtd8.sh $file
write_result "SN_STATUS" $?
