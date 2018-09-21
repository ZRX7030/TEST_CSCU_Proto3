#!/bin/sh

# Extra APP tarball to nandflash partition
# Yannis, 20170717

if [ $# -ne 1 ]; then
    echo "Syntax error!"
    echo "Usage: $0 file"
    exit 
fi

if ! [ -e $1 ]; then
    echo "Error: file '$1' not exist!"
    exit
fi

filename=$1
filesize=`wc -c $filename | awk '{print $1}'`
partname=/dev/mtd10         # Partition name
blckname=/dev/mtdblock10    # Block name
reqrname=setup.tar.gz       # Required file name
destdir=/                   # Destination directory
mountdir=/mnt/nandflash

if [ $(hostname) != "tgood" ]; then
    echo "Warnning: This script is not running on TGOOD board!"
    read -p "Continue anyway? (y/n) " input
    if [ "$input" != "y" ]; then
        echo "Do nothing. Quit."
        exit
    fi
    echo 
fi

if [ $(basename $1) != "$reqrname" ]; then
    echo "Warning: File basename is not qualified, should be '$reqrname'. "
    read -p "Continue anyway? (y/n) " input
    if [ "$input" != "y" ]; then
        echo "Do nothing. Quit."
        exit
    fi
    echo 
fi

echo "WRITE APP TO NANDFLASH"
echo "file=$filename, size=$filesize, flash=$partname"

echo 
echo "Step 1: Verify tarball"
tar -tf $filename
if [ $? -ne 0 ]; then
    echo 
    echo "Failed to verify tarball"
    exit
fi

echo 
echo "Step 2: un-mount partition"
umount $mountdir
if [ $? -ne 0 ]; then
    echo 
    echo "Failed to un-mount partition"
    exit
fi

echo
echo "Step 3: Erase partition"
flash_erase $partname 0 0
if [ $? -ne 0 ]; then
    echo 
    echo "Failed to erase partition"
    exit
fi

echo 
echo "Step 4: mount partition"
sleep 1  # Wait until the kernel information is shown
mount $blckname $mountdir
if [ $? -ne 0 ]; then
    echo 
    echo "Failed to mount partition"
    exit
fi

echo
echo "Step 5: Extra files"
tar -xf $filename -C $destdir
if [ $? -ne 0 ]; then
    echo 
    echo "Failed to extra files"
    exit
fi

sync

echo
echo "Success!"

