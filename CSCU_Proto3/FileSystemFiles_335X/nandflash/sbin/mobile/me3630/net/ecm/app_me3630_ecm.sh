#!/bin/sh

# Application for ZTE 4G module (ME3610/ME3630) : Network service via ECM interface.
# NOTE: Please make sure the 4G module is presented when running this script!
# Yannis, 20170726

function reset()
{
    echo "4G module is reseting..."
    
    echo 1 > /sys/class/gpio/gpio63/value
    sleep 1
    echo 0 > /sys/class/gpio/gpio63/value
    sleep 15  #等待4G模块重新枚举
}

function is_connected()
{
    me3630_ecm_isconnected >/dev/null 2>&1
    ret=$?
    if [ $ret -eq 0 ]; then
        echo "4G module network checked OK"
    else
        echo "4G module network checked ERROR "
    fi

    return $ret
}

function connect()
{
    echo "4G module is connecting to network..."
    
    #me3630_ecm_connect debug >/dev/null 2>&1
    me3630_ecm_connect >/dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "4G module failed to connect to network"
        return -1
    fi
    
    #udhcpc -i usb0 >/dev/null 2>&1
    dhcpcd usb0 -d >/dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "4G module failed to run 'dhcpcd usb0'"
        return -1
    fi
    
    # 连接成功
    echo "4G module has connected to network"
    return 0
}

function disconnect()
{
    echo "4G module is disconnecting from network..."
    
    me3630_ecm_disconnect >/dev/null 2>&1
    
    killall me3630_ecm_connect >/dev/null 2>&1
    killall dhcpcd >/dev/null 2>&1
    sleep 3
}



### START ###

try_lmt=2
rst_lmt=3
err_cnt=0 

connect

while true; do
    # 考虑到GPS应用程序可能占用AT端口，此处做了重试机制
    retries=0
    while [ $retries -lt $try_lmt ]; do
        is_connected
        if [ $? -eq 0 ]; then  # 已连接
            break
        fi
        sleep 30
        retries=$(expr $retries + 1)
    done

    if [ $retries -lt $try_lmt ]; then  # 已连接
        err_cnt=0 
    else # 未连接
        err_cnt=$(expr $err_cnt + 1)
        disconnect
        connect
    fi

    if [ $err_cnt -ge $rst_lmt ]; then  # 超时
        err_cnt=0 
        disconnect
        reset
    fi
    
    sleep 30
done

