#!/bin/sh

# Application for ZTE 4G module (ME3610/ME3630) : Network service via PPP interface.
# Yannis, 20170729


function reset()
{
    echo "4G module is reseting..."

    echo 1 > /sys/class/gpio/gpio63/value
    sleep 1
    echo 0 > /sys/class/gpio/gpio63/value
    sleep 15  #等待4G模块重新枚举
}


### START ###

errcnt=0

while true; do
    # Check if the pppd is running. If not, run it.
    if [ -z "$(pidof pppd)" ]; then
        echo "4G module is connecting to network..."
        me3630_ppp_connect >/dev/null &
    fi

    test_me3630_net >/dev/null
    if [ $? -eq 0 ]; then 
        errcnt=0
    else # The module is dead or crazy...
        echo "4G network test failed"
        errcnt=$(expr $errcnt + 1)
    fi

    if [ $errcnt -gt 3 ]; then
        errcnt=0
        killall me3630_ppp_connect
        killall pppd
        killall chat
        reset
    fi

    # For route and DNS configuration, see /etc/ppp/ip-up and /etc/ppp/ip-down
    
    sleep 60
done

