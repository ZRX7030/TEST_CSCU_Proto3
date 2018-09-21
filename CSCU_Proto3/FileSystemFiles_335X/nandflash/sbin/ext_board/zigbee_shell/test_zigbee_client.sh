# Zigbee test (Client)
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

# 查询2001节点的信号强度（对方模块不用配置，默认状态下地址即为2001）
write_cmd "\xde\xdf\xef\xda\x20\x01"
check_ack "\xde\xdf\xef\xda\x20\x01" 1000

# Broadcast-searching
write_cmd "\xab\xbc\xcd\xd4\x08"
check_ack "\xab\xbc\xcd\xd4" 1000

:<<!
# Transfer data
@TODO: Run comkit as echo client?
!

# Success, exit
success
