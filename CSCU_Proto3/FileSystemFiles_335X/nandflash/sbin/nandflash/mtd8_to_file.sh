#!/bin/bash

if [ $# -ne 2 ]; then
    echo "Syntax error!"
    echo "Usage: $0 length file"
    exit 1
fi

if ! [[ $1 != *[!0-9]* ]]; then
    echo "Syntax error!"
    echo "Usage: $0 length file"
    echo "Argument 'length' must be a number!"
    exit 1
fi

device=/dev/mtd8
baseaddr=0x1c00000
offset=0x100000 # The 2nd block
address=$(printf "0x%x" $(expr $(($baseaddr)) + $(($offset))))  # address = baseaddr + offset
pagesize=0x1000   # 4K
length=$1
dumprawfile="`dirname $0`/dumpraw.txt"
dumpfile=$2

echo "--------------------------------------------------------------------------------"
echo "DUMP NANDFLASH TO FILE"
echo "device=$device, address=$address (baseaddr=$baseaddr, offset=$offset)"
echo "dump length=$length, file=$dumpfile"
echo "--------------------------------------------------------------------------------"
echo "Read: size=$pagesize"
echo
nanddump -a -s $offset -l $pagesize --bb=skipbad $device -f $dumprawfile 
echo
dd if=$dumprawfile of=$dumpfile count=1 bs=$length
dumpfilesize=`wc -c $dumpfile | awk '{print $1}'`
echo "--------------------------------------------------------------------------------"
echo "File: name=$dumpfile, size=$dumpfilesize"
echo
hexdump -C $dumpfile
if [ $? -eq 0 ]; then
    echo
    echo -e "\x1b[01;32mSuccess!\x1b[00m"
    rm -f $dumprawfile
    exit 0
else
    echo
    echo -e "\x1b[01;31mFailed!\x1b[00m"
    rm -f $dumprawfile
    exit 2
fi
