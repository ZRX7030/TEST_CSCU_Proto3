#!/bin/sh

# 配置蓝牙和WiFi模块，使其能与Client board配合通讯。
# 该配置在掉电后可以保存。

echo "TEST ALL BOARDS (SERVER)"
echo "================================================================================"
echo

test_network_server.sh "192.168.31.110" eth0

test_ext_board_server.sh


echo "================================================================================"
echo

# 蜂鸣器提示测试结束
test_buzzer.sh 50000 8 > /dev/null
