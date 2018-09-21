#!/bin/sh

# Usage: test_network_client.sh destIP interface

destIP=$1
interface=$2
count=4
deadline=10

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
    echo "Usage: test_network destIP interface"
    exit -1
fi

check_ip $destIP

# 备份本机IP
myIP_bak=`ifconfig $interface | grep "inet addr:" | awk -F "inet addr:" '{print $2}' | awk -F " " '{print $1}'`
if [ -z "$myIP_bak" ]; then
    echo "Cannot get the IP of '$interface'"
    exit -1
fi

# 临时更改本机IP，使其和$destIP在同一网段内。
tmp=`echo $destIP | awk -F "." '{print $4}'`
if [ $tmp -lt 254 ]; then
    tmp=`expr $tmp + 1`
else
    tmp=`expr $tmp - 1`
fi
myIP=`echo $destIP | awk -F "." '{print $1}'`.`echo $destIP | awk -F "." '{print $2}'`.`echo $destIP | awk -F "." '{print $3}'`.$tmp

echo "--- NETWORK TEST (CLIENT) ---"
echo "myIP=$myIP, destIP=$destIP, interface=$interface"
echo

ifconfig $interface $myIP up
sleep 3

ping $destIP -I $interface -c $count -w $deadline
if [ $? -eq 0 ]; then
    echo
    echo -e "\x1b[01;32mSuccess!\x1b[00m"
    # 在退出之前，恢复原先的IP。
    ifconfig $interface $myIP_bak
    exit 0
else 
    echo
    echo -e "\x1b[01;31mFailed!\x1b[00m"
    # 在退出之前，恢复原先的IP。
    ifconfig $interface $myIP_bak
    exit -1
fi
