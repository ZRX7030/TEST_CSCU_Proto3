#!/bin/bash

# Application for Quectel 4G module (EC20) : network service.
# Yannis, 20170714


err_cnt=1  # 初值为1，表示一开始就未连接，以便打印调试信息

function init()
{
    echo "4G module is initializing..."
    
    killall test_ec20_net >/dev/null 2>&1
    killall quectel-CM >/dev/null 2>&1
    sleep 1
}

function reset()
{
    echo "4G module is reseting..."
    
    echo 1 > /sys/class/gpio/gpio63/value
    usleep 300000  # 300ms
    echo 0 > /sys/class/gpio/gpio63/value
    #等待4G模块重新枚举
    sleep 30
}


### START ###


init
#reset

while true; do
    #echo "4G module is woking..."
    
    # 如果4G模块拨号程序没有运行，则运行之
    if [ -z "`pidof quectel-CM`" ]; then
        echo "4G module is connecting to network..."
        quectel-CM -s ctnet >/dev/null &
    fi
    
    # 判断4G模块是否连接到网络
    test_ec20_net >/dev/null
    ret=$?
    # 返回-4 (252): 无SIM卡
    # 注意：对于特来电的定向SIM卡，test_ec20_net正常情况下会返回-7（249）
    if [ $ret -eq 0 ] || [ $ret -eq 249 ]; then  # 测试成功
        #echo "4G network test OK"
        err_cnt=0
    else  # 测试失败
        echo "4G network test failed"
        err_cnt=$(expr $err_cnt + 1)
    fi
    
    if [ $err_cnt -gt 3 ]; then  # 3分钟，超时
        err_cnt=0
        init
        reset
    fi
    
    sleep 60
done
