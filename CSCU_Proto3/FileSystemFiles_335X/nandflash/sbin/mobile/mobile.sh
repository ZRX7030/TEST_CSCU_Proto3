#!/bin/bash

# Connect to the mobile network (2G/4G), and get the GPS information
# Yannis, 20170710

# 1. The files under /dev/me3630/ and /dev/ec20/ are generated automatically by udev.
# 2. The files under /dev/mc20/ are created manually in this script.
# 3. These files are used to identify which mobile module we are using.
# 4. The ZTE 4G module, ME3610 and ME3630, are almost the same. 
#    They share the same driver and provide the same interface to uppper layers.


function stop_all()
{
    killall traffic_statistics
    killall test_mobile.sh

    killall pppd
    killall test_mc20_gps
    killall test_mc20_net
    killall app_mc20_gps.sh
    killall app_mc20_net.sh
    
    killall quectel-CM
    killall test_ec20_gps
    killall test_ec20_net
    killall app_ec20_gps.sh
    killall app_ec20_net.sh
    
    killall dhcpcd
    killall chat
    killall test_me3630_gps
    killall test_me3630_net
    killall app_me3630_gps.sh
    killall app_me3630_ecm.sh
    killall app_me3630_ppp.sh
    killall me3630_ecm_connect
    killall me3630_ecm_disconnect
    killall me3630_ecm_isconnected
    killall me3630_ppp_connect
    killall me3630_ppp_disconnect

    killall "`basename $0`"
    
    sleep 1
}

function show_usage()
{
    echo "Syntax error!"
    echo "Usage:"
    echo "  $0 [start] - Start the mobile network and GPS service"
    echo "  $0  stop   - Stop the mobile network and GPS service"
}

function init()
{
    # If MC20 module is presented, we create /dev/mc20/tty.
    
    # test_mc20_net returns -1 (255) means com port error
    # test_mc20_net returns -2 (254) means no response
    # test_mc20_net returns -3 (253) means module name not qualified
    test_mc20_net >/dev/null
    ret=$?
    if [ $ret -ne 255 ] && [ $ret -ne 254 ] && [ $ret -ne 253 ]; then # Found MC20
        mkdir -p /dev/mc20/
        ln -sf /dev/ttyO1 /dev/mc20/tty
    else # NOT Found MC20
        rm -rf /dev/mc20/
    fi
    
    # Re-direct /mnt/nandflash/gpsinfor to /tmp/gpsinfor on tmpfs, in case Flash is written too frequently.
    # We don't have to delete this file while powering on, since the file is stored in tmpfs. It will be lost after powering off.
    echo "" > /tmp/gpsinfor
    ln -sf /tmp/gpsinfor /mnt/nandflash/gpsinfor
}


### START ###


if [ $# -gt 1 ]; then
    show_usage
    exit
fi

if [ $# -eq 0 ] || [ "$1" == "start" ]; then
    echo "--- mobile.sh start ---"
    for i in {1..10}; do  # Wait until the 4G module's driver is loaded
        init

        if [ -d /dev/mc20/ ]; then
            echo "2G module (MC20) found"
            app_mc20_gps.sh 
            app_mc20_net.sh &
            # app_mc20_gps.sh and app_mc20_net.sh share the same serial port.
            # Don't run these two scripts at the same time!   
            exit
        elif [ -d /dev/ec20/ ]; then
            echo "4G module (EC20) found"
            app_ec20_net.sh &
            sleep 30 # Make sure the GPS APP does not disturb the NET APP on AT port
            app_ec20_gps.sh &
            exit
        elif [ -d /dev/me3630/ ]; then
            echo "4G module (ME3610/ME3630) found"
            #app_me3630_ecm.sh &
            app_me3630_ppp.sh &
            sleep 30 # Make sure the GPS APP does not disturb the NET APP on AT port
            app_me3630_gps.sh & 
            exit
        else
            echo "No mobile module found. Will try again."
            sleep 1
        fi
    done
    echo "No mobile module found. Give up."
elif [ "$1" == "stop" ]; then
    stop_all
else
    show_usage
fi
