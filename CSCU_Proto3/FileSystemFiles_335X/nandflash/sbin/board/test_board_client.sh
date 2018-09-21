#!/bin/sh

echo "TEST ALL BOARDS (CLIENT)"
echo "================================================================================"
echo

test_base_board.sh

test_ext_board_client.sh

test_mobile.sh

echo "================================================================================"
echo

# 蜂鸣器提示测试结束
test_buzzer.sh 50000 8 > /dev/null 
