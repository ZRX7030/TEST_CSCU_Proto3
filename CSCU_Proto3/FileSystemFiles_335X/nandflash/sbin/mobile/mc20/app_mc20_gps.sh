#!/bin/bash

# Application for Quectel 2G module (MC20) : GPS service.
# Yannis, 20170714

### START ###

# Timeout: 100s
for i in {10..1}
do
    echo "2G module is getting GPS data...[$i]"
    gps_data=$(test_mc20_gps | grep "Got valid GPS data: " | awk -F "Got valid GPS data: " '{print $2}')
    if [ -n "$gps_data" ]; then
        echo "2G module has got GPS data"
        echo "$gps_data" > /mnt/nandflash/gpsinfor
        break
    else
        echo "2G module failed to get GPS data"
    fi
    sleep 10
done
