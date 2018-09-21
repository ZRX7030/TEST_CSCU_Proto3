#!/bin/sh

echo "--- LCD & BACKLIGHT TEST ---"
echo

BRIGHTNESS_CONTROLLER=/sys/class/backlight/pwm-backlight/brightness
brightness_bak=`cat $BRIGHTNESS_CONTROLLER`

# Run GUI demo
`dirname $0`/test_qt_gif -qws > /dev/null &
sleep 1
:<<!
# Current brightness
#echo $brightness_bak > $BRIGHTNESS_CONTROLLER
for i in {1..0}
do
    echo -ne "Current brightness is $brightness_bak%\tPlease wait...[$i]\r"
    sleep 1
done
echo 
!
# Change brightness
for brightness_new in 100 0 50
do
    echo $brightness_new > $BRIGHTNESS_CONTROLLER
    for i in {0..0}
    do
        echo -ne "Set brightness to $brightness_new%\t\tPlease wait...[$i]\r"
        sleep 1
    done
    echo 
done
:<<!
# Restore brightness 
echo $brightness_bak > $BRIGHTNESS_CONTROLLER
for i in {1..0}
do
    echo -ne "Restore brightness to $brightness_bak%\tPlease wait...[$i]\r"
    sleep 1
done
echo 
!
# Exit GUI demo
killall test_qt_gif >/dev/null 2>&1

echo
echo "Done!"
exit 0
