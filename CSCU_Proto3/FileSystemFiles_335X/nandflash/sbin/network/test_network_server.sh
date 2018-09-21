#!/bin/sh

# Usage: test_network_server.sh myIP interface

myIP=$1
interface=$2

function check_ip() {
    IP=$1
    VALID_CHECK=$(echo $IP|awk -F. '$1<=255&&$2<=255&&$3<=255&&$4<=255{print "yes"}')
    if echo $IP|grep -E "^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$">/dev/null; then
        if [ ${VALID_CHECK:-no} == "yes" ]; then
            echo "IP $IP available" > /dev/null
        else
            echo "IP $IP not available" > /dev/null
        fi
    else
        echo "IP format error"
        exit -1
    fi
}

if [ $# -ne 2 ]; then
    echo "Syntax error!"
    echo "Usage: test_network_server myIP interface"
    exit -1
fi

check_ip $myIP

echo "--- NETWORK TEST (CLIENT) ---"
echo "myIP=$myIP, interface=$interface"
echo

ifconfig $interface $myIP up
