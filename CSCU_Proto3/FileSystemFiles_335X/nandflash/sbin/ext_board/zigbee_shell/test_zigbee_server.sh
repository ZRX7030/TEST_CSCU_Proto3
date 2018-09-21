# Zigbee test (Server)
# Yannis <wuyansky@foxmail.com> 2017-04-07

#!/bin/sh

source `dirname $0`/test_zigbee_common.sh

# Start here
validate_args $# $0 $1

echo "ZIGBEE TEST"
echo "--------------------------------------------------------------------------------"

init

:<<!
# Set channel = 0x14
write_cmd "\xde\xdf\xef\xd1\x14"
check_ack "\xde\xdf\xef\xd1\x00" 1000
!

# Do nothing, keep the default parameters

:<<!
# Transfer data
@TODO: Run comkit as echo server?
!

# Success, exit
success
