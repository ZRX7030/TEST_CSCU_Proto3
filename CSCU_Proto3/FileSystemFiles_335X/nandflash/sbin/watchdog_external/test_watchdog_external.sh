#!/bin/sh

# Yannis, 20170515

function show_usage()
{
    echo "Syntax error!"
    echo "Usage: "
    echo "  $0 enable  - Enable the watchdog, and feed it every 1 second"
    echo "  $0 disable - Disable the watchdog"
    echo "  $0 trigger - Stop feeding the watchdog. This will trigger a reset."
}

if [ $# -ne 1 ]; then
    show_usage
    exit -1
fi

if [ "$1" == "enable" ]; then
    if [ -n "`ps | grep sp706p_wdt | grep -v grep`" ]; then
        echo "The external watchdog is already enabled"
        exit -1
    fi
    echo "Enabling watchdog..."
    sp706p_wdt on &
elif [ "$1" == "disable" ]; then
    echo "Disabling watchdog..."
    sp706p_wdt off
elif [ "$1" == "trigger" ]; then
    if [ -z "`ps | grep sp706p_wdt | grep -v grep`" ]; then
        echo "The external watchdog is not enabled, we cannot trigger it."
        exit -1
    fi
    echo "Triggering watchdog..."
    echo 1 > /sys/class/leds/heartbeat/enable_ctl
else 
    show_usage
    exit -1
fi
