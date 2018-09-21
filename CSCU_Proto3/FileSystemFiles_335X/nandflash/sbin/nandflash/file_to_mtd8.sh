#!/bin/sh

if [ $# -ne 1 ]; then
    echo "Syntax error!"
    echo "Usage: $0 file"
    exit 1
fi

if [ ! -e $1 ]; then
    echo "Error: File '$1' not exist!"
    exit 1
fi

device=/dev/mtd8
baseaddr=0x1c00000
offset=0x100000 # The 2nd block
address=$(printf "0x%x" $(expr $(($baseaddr)) + $(($offset))))  # address = baseaddr + offset
pagesize=0x1000   # 4K
blocksize=0x80000 # 512K
datafile=$1
datafilesize=`wc -c $datafile | awk '{print $1}'`
dumprawfile="`dirname $0`/dumpraw.txt"
dumpfile="`dirname $0`/dump.txt"

echo "--------------------------------------------------------------------------------"
echo "WRITE FILE TO NANDFLASH"
echo "device=$device, address=$address (baseaddr=$baseaddr, offset=$offset)"
echo "--------------------------------------------------------------------------------"
echo "File: name=$datafile, size=$datafilesize"
echo
hexdump -C $datafile
echo "--------------------------------------------------------------------------------"
#echo "Read: size=$pagesize"
#echo
#nanddump -c -s $offset -l $pagesize --bb=skipbad $device | head -n 3
#echo "--------------------------------------------------------------------------------"
echo "Erase: size=$blocksize"
echo
flash_erase $device $offset 1
echo "--------------------------------------------------------------------------------"
# Mark bad blocks
echo "Test: size=$blocksize"
nandtest --markbad -o $offset -l $blocksize -k $device
echo "--------------------------------------------------------------------------------"
#echo "Read: size=$pagesize"
#echo
#nanddump -c -s $offset -l $pagesize --bb=skipbad $device | head -n 3
#echo "--------------------------------------------------------------------------------"
echo "Write: size=$pagesize"
echo
nandwrite -s $offset -p --markbad $device $datafile
echo "--------------------------------------------------------------------------------"
echo "Read: size=$pagesize"
echo
nanddump -a -s $offset -l $pagesize --bb=skipbad $device -f $dumprawfile 
echo
dd if=$dumprawfile of=$dumpfile count=1 bs=$datafilesize
echo "--------------------------------------------------------------------------------"
echo "File: $dumpfile"
echo
hexdump -C $dumpfile
echo "--------------------------------------------------------------------------------"
echo "Compare: $datafile <--> $dumpfile"
echo
cmp -l $datafile $dumpfile
if [ $? -eq 0 ]; then
    echo -e "\x1b[01;32mSuccess!\x1b[00m"
    rm -f $dumprawfile
    rm -f $dumpfile
    exit 0
else
    echo -e "\x1b[01;31mFailed!\x1b[00m"
    rm -f $dumprawfile
    rm -f $dumpfile
    exit 2
fi
