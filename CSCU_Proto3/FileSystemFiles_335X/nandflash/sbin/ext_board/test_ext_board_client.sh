# Test the extended board, and write the result to file
# Yannis <wuyansky@foxmail.com> 2017-05-26

#!/bin/sh

result_file="/tmp/test_ext_board_result.ini"

# write_result item result
function write_result()
{
    local item=$1
    local result=$2
    
    if [ $result -eq 0 ]; then
        ini_operator write $result_file EXT_BOARD $item OK
    else
        ini_operator write $result_file EXT_BOARD $item ERR
    fi
    echo
    echo "================================================================================"
    echo
}

echo "================================================================================"
echo "TEST EXTENDED BOARD (CLIENT)"
echo
echo "--------------------------------------------------------------------------------"
echo "Initializing..."
# 删除旧的测试结果
ini_operator write $result_file EXT_BOARD "LCD_232"  NULL > /dev/null
ini_operator write $result_file EXT_BOARD "485_3"    NULL > /dev/null
ini_operator write $result_file EXT_BOARD "BT"       NULL > /dev/null
ini_operator write $result_file EXT_BOARD "WIFI"     NULL > /dev/null
ini_operator write $result_file EXT_BOARD "ETH_WIFI" NULL > /dev/null
echo "--------------------------------------------------------------------------------"
#get_ext_infor.sh   # This function has been implemented by udev rules: /etc/udev/rules.d/localextra.rules
#echo "--------------------------------------------------------------------------------"

if [ -d /dev/extboard ]; then
    
    # 调试串口，此处无法测试
    
    comkit -d /dev/extboard/tty_LCD     -m es &
    comkit -d /dev/extboard/tty_Printer -m ec -i 500 -c 4
    result=$?
    write_result "LCD_232" $result
    write_result "485_3" $result
    
    killall comkit
    
    test_bt_client
    write_result "BT" $?
    
    test_wifi_client
    write_result "WIFI" $?
    
    test_network_client.sh "10.10.100.254" eth1
    write_result "ETH_WIFI" $?
    
    # Zigbee，不使用，不做测试。
fi

cat $result_file
echo
test_buzzer.sh 50000 8 > /dev/null
