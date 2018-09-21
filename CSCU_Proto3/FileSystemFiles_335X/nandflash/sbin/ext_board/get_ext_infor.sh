# Get the extended board's information
# Yannis <wuyansky@foxmail.com> 2017-05-11
# NOTE: This file is obsolete, the function has been replaced by udev rules: 
#       /etc/udev/rules.d/localextra.rules

#!/bin/sh

# Write extended board type (X1/NONE) in this file
EXT_TYPE_FILE="/dev/exttype"

# The prefix of the serial ports name that X1 board presents
X1_PORT_PREFIX="/dev/ttyX_"

function find_X1()
{
    local X1_identifier    # X1 board identifier (USB VID,PID)
    local i
    
    # Wait several seconds while booting, untill the driver is loaded.
    for i in {3..0}
    do
        echo -ne "Looking for X1 board...[$i]\r"
        X1_identifier=`lsusb 2>/dev/null | grep "04e2:1414"`
        if [ -n "$X1_identifier" ]; then
            echo "Looking for X1 board...    "
            echo "X1 board found"
            return 1
        fi
        sleep 1
    done
    
    echo "Looking for X1 board...    "
    echo "X1 board not found"
    return 0
}

function create_links_for_X1()
{
    local portnames=`dmesg | grep "Vizzini USB serial port converter now attached to " | tail -n 4 | awk -F "attached to " '{print $2}' | tr -s '\n' ' '`
    local portname
    local portlink
    local i=0
    
    #echo "portnames = $portnames"
    
    for portname in $portnames
    do
        portlink="${X1_PORT_PREFIX}${i}"
        i=`expr $i + 1`
        
        ln -sf "/dev/${portname}" "$portlink"
        if [ -L "$portlink" ]; then
            echo "Created link $portlink -> /dev/${portname}"
        else
            echo "Failed to create link $portlink -> /dev/${portname}"
        fi
    done
}

# Start

#echo "Getting extended board's information"

rm -f $EXT_TYPE_FILE > /dev/null
rm -f $X1_PORT_PREFIX* > /dev/null

find_X1
if [ $? -eq 1 ]; then
    echo "X1" > $EXT_TYPE_FILE
    create_links_for_X1
else
    echo "NONE" > $EXT_TYPE_FILE
fi

#sync
#echo "$EXT_TYPE_FILE = `cat $EXT_TYPE_FILE`"
