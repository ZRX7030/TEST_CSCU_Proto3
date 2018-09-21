#!/bin/sh

echo "----- TEST RTC TIME -----"

# Set Systime 
#date MMDDhhmm[[YY]YY][.ss]
date -s "2017-01-02 03:04:05" > /dev/null

# Get SysTime in String format
time1=`date +"%a %Y-%m-%d %T"`
# Convert SysTime to timestamp
timestamp1=`date +%s`
echo "Set RTC Time=$time1, timestamp=$timestamp1"

# SysTime -> HwTime
hwclock -w
# HwTime -> SysTime
hwclock -s

# Get SysTime in String format
time2=`date +"%a %Y-%m-%d %T"`
# Convert SysTime to timestamp
timestamp2=`date +%s`
echo "Get RTC Time=$time2, timestamp=$timestamp2"

# Compare the two timestamps
if [ `expr $timestamp2 - $timestamp1` -lt 5 ]
then 
    echo -e "\x1b[01;32mRTC test OK (^_^)\x1b[00m"
    exit 0
else
    echo -e "\x1b[01;31mRTC test ERROR! (T_T)\x1b[00m"
    exit -1
fi
