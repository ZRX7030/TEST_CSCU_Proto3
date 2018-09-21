# Initialize the board
# Yannis <wuyansky@foxmail.com> 2017-07-24

#!/bin/sh

echo "Set speaker volume to 100%"
amixer cset name='PCM Playback Volume' 100%,100% > /dev/null

echo "Set LCD brightness to 100%"
echo "100" > /sys/class/backlight/pwm-backlight/brightness

# Product Serial Number 
# Make sure that we provide a SN for APP to read
if [ -e /mnt/product_infor/Product_SN.txt ]; then
    cp /mnt/product_infor/Product_SN.txt /tmp/Product_SN.txt
else
    echo "00000000000000000000000" > /tmp/Product_SN.txt
fi
echo "Product SN: $(cat /tmp/Product_SN.txt)"
