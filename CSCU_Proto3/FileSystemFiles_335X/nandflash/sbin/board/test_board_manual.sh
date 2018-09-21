# Test the board manually, and write the result to file
# Yannis <wuyansky@foxmail.com> 2017-05-26

#!/bin/sh

result_file="/tmp/test_board_manual_result.ini"

# write_result item result
function write_result()
{
    local item=$1
    local result=$2
    
    if [ $result -eq 0 ]; then
        ini_operator write $result_file MANUAL $item OK
    else
        ini_operator write $result_file MANUAL $item ERR
    fi
    echo
    echo "================================================================================"
    echo
}


echo "TEST BOARD (MANUAL)"
echo "================================================================================"
echo

# Buzzer
test_buzzer.sh
while true
do
    read -p "Did you hear the beep? (y/n) " input
    if [ "$input" == "y" ]; then
        true
        write_result "BUZZER" $?
        break
    elif [ "$input" == "n" ]; then
        false
        write_result "BUZZER" $?
        break
    else
        echo "Input error"
    fi
done

# LCD & backlight
test_lcd_backlight.sh
while true
do
    read -p "Did you see the picture on LCD, and the brightness changed? (y/n) " input
    if [ "$input" == "y" ]; then
        true
        write_result "LCD_BACKLIGHT" $?
        break
    elif [ "$input" == "n" ]; then
        false
        write_result "LCD_BACKLIGHT" $?
        break
    else
        echo "Input error"
    fi
done

# Audio
test_audio.sh
while true
do
    read -p "Did you hear the music, and the volumn changed? (y/n) " input
    if [ "$input" == "y" ]; then
        true
        write_result "AUDIO" $?
        break
    elif [ "$input" == "n" ]; then
        false
        write_result "AUDIO" $?
        break
    else
        echo "Input error"
    fi
done

#sync
cat $result_file
echo

