#!/bin/sh

if [ $# -eq 0 ]; then
    interval=500000
    count=3
elif [ $# -eq 2 ]; then
    interval=$1
    count=$2
else
    echo "Syntax error!"
    echo "Usage: $0 interval(us) count"
    exit 1
fi

echo "Testing Buzzer"
echo

for i in $(seq 1 $count)
do
    echo "Buzzer ON"
    #gpio write 1 19 1 > /dev/null
    echo 1 > /sys/class/gpio/gpio51/value
    usleep $interval
    
    echo "Buzzer OFF"
    #gpio write 1 19 0 > /dev/null
    echo 0 > /sys/class/gpio/gpio51/value
    usleep $interval
done

echo
echo "Done!"
exit 0
