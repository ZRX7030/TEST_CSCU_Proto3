# Test the base board, and write the result to file
# Yannis <wuyansky@foxmail.com> 2017-05-26

#!/bin/sh

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

# get_di_do_result "DI_1"
function get_di_do_result()
{
    if [ -n "$(echo "$di_do_output" | grep "@  $1" | grep "OK")" ]; then
        return 0
    else 
        return -1
    fi
}

echo "================================================================================"
echo "TEST BASE BOARD"
echo

# DEBUG_TTL不在此处测试

di_do_output="`test_di_do`"

get_di_do_result "DI_1"
write_result "DI_1" $?

get_di_do_result "DI_2"
write_result "DI_2" $?

get_di_do_result "DI_3"
write_result "DI_3" $?

get_di_do_result "DI_4"
write_result "DI_4" $?

get_di_do_result "DI_5"
write_result "DI_5" $?

get_di_do_result "DI_6"
write_result "DI_6" $?

get_di_do_result "DI_7"
write_result "DI_7" $?

get_di_do_result "DI_8"
write_result "DI_8" $?

get_di_do_result "DO_1"
write_result "DO_1" $?

get_di_do_result "DO_2"
write_result "DO_2" $?

get_di_do_result "DO_3"
write_result "DO_3" $?

get_di_do_result "DO_4"
write_result "DO_4" $?

get_di_do_result "DO_5"
write_result "DO_5" $?

get_di_do_result "DO_6"
write_result "DO_6" $?

test_rtc_time.sh
ret=$?
write_result "RTC" $ret

test_rtc_clk on
# 检测1Hz信号不在此处测试
#write_result "CLK_OUT" $?

test_network_client.sh "192.168.31.110" eth0
ret=$?
write_result "LAN" $ret

#for storage_dev in `ls /dev/sd?? /dev/mmcblk0p?`
test_storage.sh "/dev/mmcblk0p1"
ret=$?
write_result "SD" $ret

# i=0
# for storage_dev in `ls /dev/sd??`
# do
    # test_storage.sh $storage_dev
    # i=`expr $i + 1`
    # write_result "USB$i"
# done

# @TODO: 此处的设备号不固定，和USB1、USB2的对应关系也不确定
test_storage.sh "/dev/sda1"
ret=$?
write_result "USB1" $ret

test_storage.sh "/dev/sdb1"
ret=$?
write_result "USB2" $ret

test_can.sh
ret=$?
write_result "CAN0" $ret
write_result "CAN1" $ret

#comkit -d /dev/ttyO3 -m ec -i 500 -c 4
#ret=$?
# MODBUS通讯口，被工装占用了。这里将其置为"OK"。
# 如果MODBUS工装读到了，一切正常；如果读不到，工装会自己处理成"ERR"状态。
ret=0
write_result "RFID_TTL" $ret
write_result "RFID_232" $ret

comkit -d /dev/ttyO2 -m es &
comkit -d /dev/ttyO4 -m ec -i 500 -c 4
ret=$?
write_result "485_1" $ret
write_result "485_2" $ret
killall comkit >/dev/null 2>&1

sync
cat $result_file
echo
test_buzzer.sh 50000 8 > /dev/null 
