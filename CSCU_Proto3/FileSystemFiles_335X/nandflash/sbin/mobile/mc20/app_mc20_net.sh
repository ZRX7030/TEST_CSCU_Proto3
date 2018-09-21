#!/bin/sh

# Application for Quectel 2G module (MC20) : Network service via PPP interface.
# Yannis, 20170801

function mc20_on()
{
    echo "[MC20] Turn on"
    echo 1 > /sys/class/gpio/gpio63/value
    sleep 2
    echo 0 > /sys/class/gpio/gpio63/value
    sleep 5
}

function mc20_off()
{
    echo "[MC20] Turn off"
    echo 1 > /sys/class/gpio/gpio63/value
    sleep 1
    echo 0 > /sys/class/gpio/gpio63/value
    sleep 22
}

# <未调用>
# 复位模块 (复位脚：GPIO1_31) 
function mc20_reset()
{
    echo "[MC20] Reseting..."
    
    test_mc20_net >/dev/null
    ret=$?
    if [ $ret -eq 255 ] || [ $ret -eq 254 ]; then
        # 返回-1 - 串口配置错误
        # 返回-2 - 发送命令无响应
        # 说明模块电源可能被关掉了
        # 重新开启即可
        mc20_on
    else
        # 模块电源是开启的
        # 复位的时候先关再开
        mc20_off
        mc20_on
    fi
    
    # 检查模块电源是否已开启
    test_mc20_net >/dev/null
    ret=$?
    if [ $ret -eq 255 ] || [ $ret -eq 254 ]; then
        echo "[MC20] Reset failed"
    else
        echo "[MC20] Reset OK"
    fi
}

# <未调用>
# 检查模块联网状态
# 返回  0: 已连接
# 返回 -1: 未连接
function mc20_check_connection()
{
    ping 8.8.8.8 -I ppp0 -c 4 -w 15 >/dev/null
    if [ $? -eq 0 ]; then
        return 0
    fi
    
    ping 114.114.114.114 -I ppp0 -c 4 -w 15 >/dev/null
    if [ $? -eq 0 ]; then
        return 0
    fi

    return -1
}


### START ###

errcnt=0

while true; do
    # Check if the pppd is running. If not, run it.
    if [ -z "$(pidof pppd)" ]; then
        echo "[MC20] pppd is not running. Run it now."
        pppd call quectel-ppp >/dev/null 
        # 不要让pppd在后台运行，
        # 也不要在/etc/ppp/peers/quectel-ppp里启用persist，
        # 这样一来：
        # 若拨号失败，pppd就会退出，此脚本继续向下运行;
        # 若拨号成功，pppd就会一直在此阻塞。
    fi
    
    # 运行到这里，pppd已经退出了，可以使用串口了
    echo "[MC20] pppd quit"
    killall pppd >/dev/null 2>&1  #其实不需要这一句
    sleep 3
    
    test_mc20_net >/dev/null
    ret=$?
    if [ $ret -eq 0 ]; then
        # 一切正常，什么也不用做
        # 正常情况下不会进这个分支
        echo "[MC20] Test OK"
        errcnt=0
    elif [ $ret -eq 255 ] || [ $ret -eq 254 ]; then
        # 返回-1 - 串口配置错误
        # 返回-2 - 发送命令无响应
        # 说明模块电源可能被关掉了
        # 重新开启即可
        echo "[MC20] Test failed: Serial port error"
        errcnt=0
        mc20_on
    else
        # 模块电源是开启的（串口是通的），但工作状态不正常
        echo "[MC20] Test failed: ErrCode=$ret"
        errcnt=$(expr $errcnt + 1)
        if [ $errcnt -gt 3 ]; then
            echo "[MC20] Test failed too many times. Reset it now."
            errcnt=0
            # 复位模块
            mc20_off
            mc20_on
        fi
    fi
    
    # For route and DNS configuration, see /etc/ppp/ip-up and /etc/ppp/ip-down
    
    sleep 60
done
