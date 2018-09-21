# Test the mobile board(2G/4G), and write the result to file
# Yannis <wuyansky@foxmail.com> 2017-05-26

#!/bin/sh

result_file="/tmp/test_mobile_board_result.ini"

# write_result item result
function write_result()
{
    local item=$1
    local result=$2
    
    if [ $result -eq 0 ]; then
        ini_operator write $result_file MOBILE_BOARD $item OK
    else
        ini_operator write $result_file MOBILE_BOARD $item ERR
    fi
    echo
    echo "================================================================================"
    echo
}

function show_usage()
{
    echo "Syntax error!"
    echo "Usage:"
    echo "$0  [2G_4G]"
    echo "$0   2G_ALL | 2G_NET | 2G_GPS"
    echo "$0   4G_ALL | 4G_NET | 4G_GPS"
}


# Start

do_2G_NET="NO"
do_2G_GPS="NO"
do_4G_NET="NO"
do_4G_GPS="NO"

if [ $# -eq 0 ]; then
    do_2G_NET="YES"
    do_2G_GPS="YES"
    do_4G_NET="YES"
    do_4G_GPS="YES"
elif [ $# -eq 1 ]; then
    if  [ $1 == "2G_4G" ]; then
        do_2G_NET="YES"
        do_2G_GPS="YES"
        do_4G_NET="YES"
        do_4G_GPS="YES"
    elif [ $1 == "2G_ALL" ]; then
        do_2G_NET="YES"
        do_2G_GPS="YES"
    elif [ $1 == "2G_NET" ]; then
        do_2G_NET="YES"
    elif [ $1 == "2G_GPS" ]; then
        do_2G_GPS="YES"
    elif [ $1 == "4G_ALL" ]; then
        do_4G_NET="YES"
        do_4G_GPS="YES"
    elif [ $1 == "4G_NET" ]; then
        do_4G_NET="YES"
    elif [ $1 == "4G_GPS" ]; then
        do_4G_GPS="YES"
    else
        show_usage
        exit
    fi
else 
    show_usage
    exit
fi

echo "================================================================================"
echo "TEST MOBILE BOARD"
echo
echo "--------------------------------------------------------------------------------"
echo "Initializing..."
# 删除旧的测试结果
ini_operator write $result_file MOBILE_BOARD "2G_NET" NULL > /dev/null
ini_operator write $result_file MOBILE_BOARD "2G_GPS" NULL > /dev/null
ini_operator write $result_file MOBILE_BOARD "4G_NET" NULL > /dev/null
ini_operator write $result_file MOBILE_BOARD "4G_GPS" NULL > /dev/null
echo "--------------------------------------------------------------------------------"
if [ -d /dev/mc20/ ]; then
    if [ $do_2G_NET == "YES" ]; then
        test_mc20_net
        ret=$?
        # @TODO: 对返回值要做处理，特来电定向SIM卡未注册到网络时，返回值不是0
        write_result "2G_NET" $ret
    fi
    
    if [ $do_2G_GPS == "YES" ]; then
        test_mc20_gps
        ret=$?
        # @TODO: 对返回值要做处理？
        write_result "2G_GPS" $ret
    fi
elif [ -d /dev/ec20/ ]; then
    if [ $do_4G_NET == "YES" ]; then
        test_ec20_net
        ret=$?
        # 注意：对于特来电的定向SIM卡，test_ec20_net正常情况下会返回-7（249）
        if [ $ret -eq 0 ] || [ $ret -eq 249 ]; then
            ret=0
        fi
        write_result "4G_NET" $ret
    fi
    
    if [ $do_4G_GPS == "YES" ]; then
        test_ec20_gps
        ret=$?
        # @TODO: 对返回值要做处理？
        write_result "4G_GPS" $ret
    fi
elif [ -d /dev/me3630/ ]; then
    if [ $do_4G_NET == "YES" ]; then
        test_me3630_net
        ret=$?
        # 注意：对于特来电的定向SIM卡，test_ec20_net正常情况下会返回-7（249）
        if [ $ret -eq 0 ] || [ $ret -eq 249 ]; then
            ret=0
        fi
        write_result "4G_NET" $ret
    fi
    
    if [ $do_4G_GPS == "YES" ]; then
        test_me3630_gps
        ret=$?
        # @TODO: 对返回值要做处理？
        write_result "4G_GPS" $ret
    fi
fi

sync
cat $result_file
echo
test_buzzer.sh 50000 8 > /dev/null &

