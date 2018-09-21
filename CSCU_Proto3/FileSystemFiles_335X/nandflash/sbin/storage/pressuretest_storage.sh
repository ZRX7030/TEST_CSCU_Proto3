#!/bin/sh

# Usage: pressuretest_storage.sh device

device_name=$1
mount_point=/mnt/test_storage
video_file=panda.mp4


# Validate arguments
if [ $# -ne 1 ]; then
    echo "Syntax error!"
    echo "Usage: $0 device"
    exit -1
fi

if ! [ -b $device_name ]; then
    echo "Error: No such storage device '$device_name'"
    exit -1
fi

umount $mount_point 1>/dev/null 2>/dev/null
rmdir $mount_point  1>/dev/null 2>/dev/null
mkdir $mount_point  1>/dev/null 2>/dev/null

mount $device_name $mount_point
if [ $? -ne 0 ]; then
    # The command will print the error information
    rmdir $mount_point
    exit -1
fi

cp -f /home/root/media/$video_file $mount_point/
if [ $? -ne 0 ]; then
    # The command will print the error information
    umount $mount_point
    rmdir $mount_point
    exit -1
fi

echo "--------------------------------- START ----------------------------------"
# Loop here
mplayer -fs -loop 0 "$mount_point/$video_file"

echo "---------------------------------- END -----------------------------------"
rm "$mount_point/$video_file" 1>/dev/null 2>/dev/null
umount $mount_point 1>/dev/null 2>/dev/null
rmdir $mount_point  1>/dev/null 2>/dev/null
