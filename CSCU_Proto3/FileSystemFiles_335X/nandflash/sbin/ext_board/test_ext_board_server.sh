# Test the extended board (server)
# Yannis <wuyansky@foxmail.com> 2017-05-26

#!/bin/sh

function error_loop()
{
    while true
    do 
        test_buzzer.sh > /dev/null
        sleep 1
    done
}


echo "================================================================================"
echo "TEST EXTENDED BOARD (SERVER)"
echo

#get_ext_infor.sh  # This function has been implemented by udev rules: /etc/udev/rules.d/localextra.rules
#echo "--------------------------------------------------------------------------------"

if [ -d /dev/extboard ]; then
    test_bt_server
    if [ $? -ne 0 ]; then
        error_loop
    fi
    
    test_wifi_server
    if [ $? -ne 0 ]; then
        error_loop
    fi
else
    echo -e "\x1b[01;31mError: Extended board not found!\x1b[00m"
    error_loop
fi

echo
echo "================================================================================"
echo
