# Bluetooth test (common functions)
# Yannis <wuyansky@foxmail.com> 2017-04-01

#!/bin/sh

# The serial port we use to communicate
default_device=/dev/ttyUSB3
device=""

# The file to save received data
# Use ramfs for speed up
ack_file=/dev/shm/bt_ack.txt

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
    # Reset the BT module
    echo "Reseting the module..."
    gpio write 2 0 0 > /dev/null
    sleep 1
    gpio write 2 0 1 > /dev/null
    sleep 5
    
    # Set UART parameters (baudrate)
    comkit -d $device -b 57600 -m pc > /dev/null
    if [ $? -ne 0 ]; then
        echo "Error while invoking 'comkit'"
        exit -1
    fi
    
    # Start ack-monitor (blocked read)
    touch $ack_file
    cat $device > $ack_file &
    
    # We are initially in Transparent Mode (probably),
    # Let's enter Command Mode now
    write_data "+++a"
    check_ack "a+ok" 1000
    
:<<!
    write_data "+++"
    check_ack "a" 500
    
    write_data "a"
    check_ack "+ok" 500
!
}

function deinit()
{
    # We are in Command Mode now (probably),
    # Let go back to the initial Transparent Mode.
    # Note that we don't check the ack here, since we could be already in Transparent Mode.
    echo -en "AT+ENTM\r\n" > $device
    
    # Stop ack-monitor
    killall -q "cat" >/dev/null 2>&1
    rm $ack_file >/dev/null 2>&1
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
    echo "" > $ack_file
    echo -e "CMD = \x1b[01;35m$1\x1b[00m"
    echo -en "$1\r\n" > $device
}

# write_data data
function write_data()
{
    echo "" > $ack_file
    echo -e "DAT = \x1b[01;36m$1\x1b[00m"
    echo -en "$1" > $device
}

# read_ack timeout_in_ms
function read_ack()
{
    usleep `expr $1 \* 1000`
    ack=`cat $ack_file`
    echo -e "ACK = \x1b[01;33m$ack\x1b[00m"
    echo "" > $ack_file
}

# check_ack expected_ack timeout_in_ms
function check_ack()
{
    read_ack $2
    
    if [[ "$ack" =~ "$1" ]]; then
        echo -e "\t\t\t\t\t\t\t\t[OK]"
        echo
    else
        echo -e "\t\t\t\t\t\t\t\t[ERROR]"
        failed
    fi
}
