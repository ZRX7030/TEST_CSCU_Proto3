#!/bin/sh

# Usage: test_storage.sh device

device_name=$1
mount_point=/mnt/test_storage
test_file=$mount_point/test.txt
write_data="Test device '$device_name'. `date`"
read_data=""
interval=1000

# Functions
function success()
{
    echo
    echo -e "\x1b[01;32mSuccess!\x1b[00m"
    exit 0
}
function failed()
{
    echo
    echo -e "\x1b[01;31mFailed!\x1b[00m"
    exit -1
}

echo "--- STORAGE DEVICE TEST ---"
echo "Testing '$device_name'"
echo
echo

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

# Create mount point
if ! [ -d $mount_point ]; then
    echo -e "Create mount point \t\t'$mount_point'"
    mkdir $mount_point
    if [ $? -ne 0 ]; then
        # The command will print the error information
        failed
    fi
    echo -e "\t\t\t\t\t\t\t\t\t[OK]"
fi
usleep $interval

# Mount device
# @TODO: Check if already mounted
umount $mount_point 1>/dev/null 2>/dev/null

echo -e "Mount '$device_name' to \t'$mount_point'"
mount $device_name $mount_point
if [ $? -ne 0 ]; then
    # The command will print the error information
    rmdir $mount_point
    failed
fi
echo -e "\t\t\t\t\t\t\t\t\t[OK]"
usleep $interval

# Create file
echo -e "Create test file \t\t'$test_file'"
touch $test_file
if [ $? -ne 0 ]; then
    # The command will print the error information
    umount $mount_point
    rmdir $mount_point
    failed
fi
echo -e "\t\t\t\t\t\t\t\t\t[OK]"
usleep $interval

# Write to file
echo -e "Write to file \t\t\t'$test_file'"
#echo "  <$write_data>"
echo "$write_data" > $test_file
if [ $? -ne 0 ]; then
    # The command will print the error information
    rm $test_file
    umount $mount_point
    rmdir $mount_point
    failed
fi
echo -e "\t\t\t\t\t\t\t\t\t[OK]"
usleep $interval

# Read from file
echo -e "Read from file \t\t\t'$test_file'"
read_data=`cat $test_file`
if [ $? -ne 0 ]; then
    # The command will print the error information
    rm $test_file
    umount $mount_point
    rmdir $mount_point
    failed
fi
#echo "  <$read_data>"
echo -e "\t\t\t\t\t\t\t\t\t[OK]"
usleep $interval

# Remove file
echo -e "Remove test file \t\t'$test_file'"
rm -f $test_file
if [ $? -ne 0 ]; then
    # The command will print the error information
    umount $mount_point
    rmdir $mount_point
    failed
fi
echo -e "\t\t\t\t\t\t\t\t\t[OK]"
usleep $interval

# Unmount device
echo -e "Unmount \t\t\t'$mount_point'"
umount $mount_point
if [ $? -ne 0 ]; then
    # The command will print the error information
    rmdir $mount_point
    failed
fi
echo -e "\t\t\t\t\t\t\t\t\t[OK]"
usleep $interval

# Remove mount point
echo -e "Remove mount point \t\t'$mount_point'"
rmdir $mount_point
if [ $? -ne 0 ]; then
    # The command will print the error information
    failed
fi
echo -e "\t\t\t\t\t\t\t\t\t[OK]"
usleep $interval

#sync

# Compare "Written data" and "Read data"
echo -e "Compare WRITE with READ"
if [ "$write_data" = "$read_data" ]; then
    echo -e "\t\t\t\t\t\t\t\t\t[OK]"
    success
else 
    echo -e "\t\t\t\t\t\t\t\t\t[ERROR]"
    failed
fi
