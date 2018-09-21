# CAN test
# Connection CAN0 and CAN1 together, then run this script
# Yannis <wuyansky@foxmail.com> 2017-05-05

#!/bin/sh

recv_file="/dev/shm/test_can.txt"
fail=0

echo "CAN TEST"
echo "--------------------------------------------------------------------------------"

# Init
canconfig can0 stop
canconfig can1 stop
killall candump > /dev/null
rm -f $recv_file > /dev/null

# Configure devices
canconfig can0 bitrate 125000 ctrlmode triple-sampling on
canconfig can1 bitrate 125000 ctrlmode triple-sampling on

# start devices
canconfig can0 start
canconfig can1 start
echo


# CAN0 for receiving
candump can0 > $recv_file &
usleep 500000
# CAN1 for sending
cansend can1 -i 0x10 0x11 0x22 0x33 0x44 0x55 0x66 0x77 0x88
usleep 500000
# Check result
recv_msg=`cat $recv_file | grep "11 22 33 44 55 66 77 88"`
if [ -z "$recv_msg" ]; then
    echo "Test can0 <-- can1    [ERROR]"
    fail=1
else
    echo "Test can0 <-- can1    [OK]"
fi
# Clean up
killall candump > /dev/null
rm -f $recv_file > /dev/null
echo


# CAN1 for receiving
candump can1 > $recv_file &
usleep 500000
# CAN0 for sending
cansend can0 -i 0x10 0x88 0x77 0x66 0x55 0x44 0x33 0x22 0x11
usleep 500000
# Check result
recv_msg=`cat $recv_file | grep "88 77 66 55 44 33 22 11"`
if [ -z "$recv_msg" ]; then
    echo "Test can0 --> can1    [ERROR]"
    fail=1
else
    echo "Test can0 --> can1    [OK]"
fi
# Clean up
killall candump > /dev/null
rm -f $recv_file > /dev/null
echo

# Stop devices
usleep 500000
canconfig can0 stop
canconfig can1 stop

echo "--------------------------------------------------------------------------------"

if [ $fail -eq 1 ]; then
    echo -e "\x1b[01;31mFailed!\x1b[00m"
    exit -1
else
    echo -e "\x1b[01;32mSuccess!\x1b[00m"
    exit 0
fi
