#!/bin/sh

killall "cscu_a1.init"
killall "teui.init"
killall "CSCU_A1"
killall "TEUI"
killall "screensaver"
killall "traffic_statistics"

mobile.sh stop

sleep 5

