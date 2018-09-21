#!/bin/sh

if [ -z "$(pidof screensaver)" ]; then  
    screensaver >/dev/null &
fi

killall ts_calibrate test_ts.sh test_ts_gui test_qt_gif >/dev/null 2>&1
sleep 1
echo
ts_calibrate
