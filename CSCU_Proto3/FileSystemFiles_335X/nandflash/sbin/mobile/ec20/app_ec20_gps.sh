#!/bin/bash

# Application for Quectel 4G module (EC20) : GPS service.
# Yannis, 20170714


### START ###


while true; do
    echo "4G module is getting GPS data..."
    gps_data=$(test_ec20_gps /dev/ttyO1 | grep "Got valid GPS data: " | awk -F "Got valid GPS data: " '{print $2}')
    if [ -n "$gps_data" ]; then
        echo "4G module has got GPS data"
        echo "$gps_data" > /mnt/nandflash/gpsinfor
    else
        echo "4G module failed to get GPS data"
    fi
    
    sleep 180
done

