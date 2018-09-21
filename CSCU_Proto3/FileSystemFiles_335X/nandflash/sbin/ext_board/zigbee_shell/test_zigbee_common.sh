# Zigbee test (common functions)
# Yannis <wuyansky@foxmail.com> 2017-04-07

#!/bin/sh

# The serial port we use to communicate
default_device=/dev/ttyUSB0
device=""

# The file to save received data
# Use ramfs for speed up
ack_file=/dev/shm/zigbee_ack.txt

# The acknowledge or data received
ack=""

# validate_args script_argc script_argv0 script_argv1 ...
function validate_args()
{
    script_argc=$1
    script_argv0=$2
    script_argv1=$3
    
    if [ $script_argc == "0" ]; then
        device=$default_device
    elif [ $script_argc == "1" ]; then
        device=$script_argv1
    else 
        echo "Syntax error!"
        echo "Usage: $script_argv0 [serial_port]"
        exit -1
    fi
    
    if ! [ -c $device ]; then
        echo "Error! No such serial port '$device'"
        echo "Usage: $script_argv0 [serial_port]"
        exit -1
    fi
}

function init()
{
    # Reset the Zigbee module
    #gpio write 1 23 0 > /dev/null
    echo 0 > /sys/class/gpio/gpio55/value
    if [ $? -ne 0 ]; then
        echo "Error while invoking 'gpio'"
        exit -1
    fi
    usleep 100000
    #gpio write 1 23 1 > /dev/null
    echo 1 > /sys/class/gpio/gpio55/value
    usleep 2000000
    
    # Set UART parameters (baudrate)
    comkit -d $device -b 115200 -m pc > /dev/null
    if [ $? -ne 0 ]; then
        echo "Error while invoking 'comkit'"
        exit -1
    fi
    
    # Start ack-monitor (blocked read)
    rm $ack_file >/dev/null 2>&1
    cat $device > $ack_file &
}

function deinit()
{
    # Stop ack-monitor
    killall -q "cat" >/dev/null 2>&1
    rm $ack_file
}

function success()
{
    echo "--------------------------------------------------------------------------------"
    echo
    deinit
    echo
    echo -e "\x1b[01;32mSuccess!\x1b[00m"
    exit 0
}

function failed()
{
    echo "--------------------------------------------------------------------------------"
    echo
    deinit
    echo
    echo -e "\x1b[01;31mFailed!\x1b[00m"
    exit -1
}

# write_cmd cmd
function write_cmd()
{
    echo -n "" > $ack_file
    cmd=`echo -en $1 | hexdump -C | tr -s ' '`
    cmd=`echo $cmd | awk -F "00000000 | \\|" '{print $2}'`
    echo -e "CMD = \x1b[01;35m$cmd\x1b[00m"
    echo -en "$1" > $device
}

# write_data data
function write_data()
{
    echo -n "" > $ack_file
    data=`echo -en $1 | hexdump -C | tr -s ' '`
    #data=`echo $data | awk -F "00000000 | \\|" '{print $2}'`
    echo -e "DAT = \x1b[01;36m$data\x1b[00m"
    echo -en "$1" > $device
}

# read_ack timeout_in_ms
function read_ack()
{
    usleep `expr $1 \* 1000`
    ack=`hexdump -C $ack_file | tr -s ' '`
    #ack=`echo $ack | awk -F "00000000 | \\|" '{print $2}'`
    echo -e "ACK = \x1b[01;33m$ack\x1b[00m"
    echo -n "" > $ack_file
}

# check_ack expected_ack timeout_in_ms
function check_ack()
{
    read_ack $2
    
    expected_ack=`echo -en $1 | hexdump -C | tr -s ' '`
    expected_ack=`echo $expected_ack | awk -F "00000000 | \\|" '{print $2}'`
    #echo "exp = $expected_ack"
    
    if [[ "$ack" =~ "$expected_ack" ]]; then
        echo -e "\t\t\t\t\t\t\t\t[OK]"
        echo
    else
        echo -e "\t\t\t\t\t\t\t\t[ERROR]"
        failed
    fi
}
