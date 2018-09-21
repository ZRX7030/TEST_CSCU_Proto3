#!/bin/sh

# Write kernel image to nandflash partition
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
partnameA=/dev/mtd6  # Partition 'Kernel A'
partnameB=/dev/mtd7  # Partition 'Kernel B'
reqrname=uImage     # Required file name

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

echo "WRITE KERNEL IMAGE TO NANDFLASH"
echo "file=$filename, size=$filesize, flashA=$partnameA, flashB=$partnameB"
echo 

flash_erase $partnameA 0 0
if [ $? -ne 0 ]; then
    echo 
    echo "Failed to erase Flash A"
    exit
fi

nandwrite -p $partnameA $filename
if [ $? -ne 0 ]; then
    echo 
    echo "Failed to write Flash A"
    exit
fi

flash_erase $partnameB 0 0
if [ $? -ne 0 ]; then
    echo 
    echo "Failed to erase Flash B"
    exit
fi

nandwrite -p $partnameB $filename
if [ $? -ne 0 ]; then
    echo 
    echo "Failed to write Flash B"
    exit
fi

sync

echo
echo "Success!"

