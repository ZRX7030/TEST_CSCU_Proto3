# Bluetooth test (server, slave)
# Yannis <wuyansky@foxmail.com> 2017-04-01

#!/bin/sh

source `dirname $0`/test_bt_common.sh

# Start here
validate_args $# $0 $1

echo "BLUETOOTH TEST"
echo "--------------------------------------------------------------------------------"

init

# Get BT device information
write_cmd "AT+SHOW?"
check_ack "OK" 1000

:<<!
# Get version
write_cmd "AT+CIVER?"
check_ack "OK" 1000
!

# Set Slave mode
write_cmd "AT+MODE=S"
check_ack "OK" 1000

:<<!
# Transfer data
@TODO: Run comkit as echo server?
!

:<<!
# PS:
# (1) Once a Slave device is connected to a Master device, this Slave device cannot be found by other Master devices.
# (2) For Transparent data transmission, the paired Master and Slave devices must both enter 'Transparent Mode'.
!

# Success, exit
success
