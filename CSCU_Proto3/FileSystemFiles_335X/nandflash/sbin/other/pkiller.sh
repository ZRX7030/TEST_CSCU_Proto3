#!/bin/bash

pname=$1
puser=$2

# Syntax check
if [[ -z $pname ]]; then
  echo "Syntax error!"
  echo "Usage: pkiller pname [puser]"
  exit 1
fi

# Default user is root
if [[ -z $puser ]]; then
  puser=root
fi

# Find the pids by pname
#pid1=`ps aux | grep $pname | grep $puser | grep -v 'grep' | grep -v $0 | awk '{print $2}'`
pid1=`ps | grep $pname | grep $puser | grep -v 'grep' | grep -v $0 | awk '{print $1}' | tr '\n' ' '`
cnt1=`echo "$pid1" | wc -w`

if [[ $cnt1 -eq 0 ]]; then
    echo "Cannot find any process '$pname' running by '$puser'"
    exit 1
fi

# Kill the process(es)
echo "Killing process pid='$pid1', please wait ..."
kill $pid1
sleep 5

# Check the result
ret=$?
#pid2=`ps aux | grep $pname | grep $puser | grep -v 'grep' | grep -v $0 | awk '{print $2}'`
pid2=`ps | grep $pname | grep $puser | grep -v 'grep' | grep -v $0 | awk '{print $1}' | tr '\n' ' '`
cnt2=`echo "$pid2" | wc -w`

if [[ $cnt2 -eq 0 && $ret -eq 0 ]]; then
  echo "$cnt1 process(es) killed"
  exit 0
else
  echo "Failed! Process pid2='$pid2' still running!"
  exit 1
fi
