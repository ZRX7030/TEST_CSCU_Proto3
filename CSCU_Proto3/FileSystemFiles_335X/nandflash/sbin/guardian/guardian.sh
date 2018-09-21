#!/bin/sh

# Yannis, 20170515

cnt1=0
cnt2=0
val=0
USE_INTERNAL_WATCHDOG=1
USE_LCD_HEATER=0

while true
do
    # Every 10 seconds, Check if the watchdog process is running.
    # If not, run it.
    cnt1=`expr $cnt1 + 1`
    if [ $cnt1 -ge 10 ]; then
        cnt1=0
        if [ $USE_INTERNAL_WATCHDOG -eq 1 ]; then
            #echo "Checking process 'watchdog_internal' ..."
            if [ -z "`pidof watchdog_internal`" ]; then
                #echo "Enabling internal watchdog..."
                # The default timeout of the internal watchdog is 60 seconds.
                # We feed it every 20 seconds.
                #watchdog_internal set 60
                watchdog_internal feed 20 &
            fi
        else
            #echo "Checking process 'sp706p_wdt' ..."
            if [ -z "`pidof sp706p_wdt`" ]; then
                echo "Enabling external watchdog..."
                sp706p_wdt on &
            fi
        fi
    fi
    
    # Every 10 seconds, Check if the lcd_heater.sh process is running.
    # If not, run it.
    if [ $USE_LCD_HEATER -eq 1 ]; then
        cnt2=`expr $cnt2 + 1`
        if [ $cnt2 -ge 10 ]; then
            cnt2=0
            if [ -z "`ps | grep lcd_heater.sh | grep -v grep`" ]; then
                echo "Pulling up lcd_heater.sh..."
                lcd_heater.sh &
            fi
        fi
    fi

    # Toggle LED (GPIO3_4)
    if [ $val -ne 0 ]; then 
        val=0
    else
        val=1
    fi
    #gpio write 3 4 $val
    echo $val > /sys/class/gpio/gpio100/value
    
    sleep 1
done
