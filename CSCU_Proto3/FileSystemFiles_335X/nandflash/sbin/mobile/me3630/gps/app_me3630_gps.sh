#!/bin/sh

# Application for ZTE 4G module (ME3610/ME3630) : GPS service.
# Yannis, 20170714


### START ###


while true; do
    echo "4G module is getting GPS data..."
    gps_data=$(test_me3630_gps /dev/ttyO1 | grep "Got valid GPS data: " | awk -F "Got valid GPS data: " '{print $2}')
    if [ -n "$gps_data" ]; then
        echo "4G module has got GPS data"
        echo "$gps_data" > /mnt/nandflash/gpsinfor
    else
        echo "4G module failed to get GPS data"
    fi
    
    sleep 180
done

