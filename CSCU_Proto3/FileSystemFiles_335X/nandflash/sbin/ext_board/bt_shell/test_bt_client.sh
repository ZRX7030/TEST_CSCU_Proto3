# Bluetooth test (client, master)
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

# Set to Master mode
write_cmd "AT+MODE=M"
check_ack "OK" 1000

# Scan, find Slave devices
write_cmd "AT+SCAN"
check_ack "RSSI:" 5000
# The ACK is like:
#   +SCAN:ON
#   OK
#   No: 1 Addr:D8B04CB010ED  RSSI:-79 dBm

:<<!
# Connet to Slave device
write_cmd "AT+CONN=1"
check_ack "OK" 5000
# The ACK is like:
#   +CONN:1
#   OK
#   USR-BL100 V1.0.2
!

:<<!
# Transfer data
# The Master device is already in 'Transparent Mode' now,
# For data transmission, the Slave device must also enter 'Transparent Mode'.
write_data "abcd1234"
check_ack "abcd1234" 5000
@TODO: Run comkit as echo client?
!

:<<!
# PS:
# (1) Once a Slave device is connected to a Master device, this Slave device cannot be found by other Master devices.
# (2) For Transparent data transmission, the paired Master and Slave devices must both enter 'Transparent Mode'.
!

# Success, exit
success
