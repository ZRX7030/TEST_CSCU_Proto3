#!/bin/sh

# 关于温度和ADC值的对应关系，见《lm50-AM335x 温度-电压-ADC值对应表》
ADC_VALUE_MIN=1000  # -6 centi-degree
ADC_VALUE_MAX=1365  #  10 centi-degree

adc_value=0

is_heating="NO"

# Get the median-filtered ADC value from temperature sensor
function get_ADC_value_from_temp_sensor() 
{
    #echo "get_ADC_value_from_temp_sensor()"

    adc_1=$(cat /sys/bus/iio/devices/iio:device0/in_voltage4_raw)
    sleep 1
    adc_2=$(cat /sys/bus/iio/devices/iio:device0/in_voltage4_raw)
    sleep 1
    adc_3=$(cat /sys/bus/iio/devices/iio:device0/in_voltage4_raw)
    sleep 1
    
    #echo "adc_1=$adc_1, adc_2=$adc_2, adc_3=$adc_3" 
    adc_sorted=$(echo "$adc_1 $adc_2 $adc_3" | tr ' ' '\n' | tr -s '\n'  | sort -g | tr '\n' ' ')
    #echo "adc_sorted=$adc_sorted"
    adc_median=$(echo $adc_sorted | awk '{print $2}')
    #echo "adc_median=$adc_median"

    adc_value=$adc_median
    #echo "ADC_value_from_temp_sensor=$adc_value"
}

# Heater is controlled by GPIO3_17, DO6
function heater_on() 
{
    echo "LCD Heater on"
    echo 1 > /sys/class/gpio/gpio113/value

    is_heating="YES"
}
function heater_off() 
{
    echo "LCD Heater off"
    echo 0 > /sys/class/gpio/gpio113/value

    is_heating="NO"
}


heater_off

while true
do
    get_ADC_value_from_temp_sensor
    if [ $adc_value -lt $ADC_VALUE_MIN ]; then
        if [ $is_heating = "NO" ]; then
            heater_on
        fi
    elif [ $adc_value -gt $ADC_VALUE_MAX ]; then
        if [ $is_heating = "YES" ]; then
            heater_off
        fi
    fi

    #if [ $is_heating = "YES" ]; then
    #   echo "LCD Heater on"
    #else
    #   echo "LCD Heater off"
    #fi

    sleep 10
done

