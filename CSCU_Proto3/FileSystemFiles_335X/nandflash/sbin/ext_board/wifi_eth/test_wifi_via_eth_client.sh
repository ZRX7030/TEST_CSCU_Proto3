#!/bin/sh

eth_wifi=eth1

if [ -z "`ifconfig | grep $eth_wifi`" ]; then
    echo "Setting up $eth_wifi, please wait..."
    ifconfig $eth_wifi 10.10.100.12/8 up
    sleep 6
    echo
fi

`dirname $0`/test_wifi_client
