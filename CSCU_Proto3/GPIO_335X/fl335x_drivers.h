#ifndef FL335X_DRIVERS_H
#define FL335X_DRIVERS_H
#include <stdio.h>
#include <stdlib.h> //strtol()
#include <string.h>
#include <fcntl.h>  //O_RDONLY
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#define GPIO_DEV                            "/dev/gpio"
#define GPIO_DEV_TYPE                       'G'  //Magic number

#define GPIO_ID(bank, index)                ( ((bank) << 5) + (index) )   //( (bank) * 32 + index )
#define GPIO_GET_BANK(id)                   ( (id) >> 5 )   //Get the higher bits
#define GPIO_GET_INDEX(id)                  ( (id) & 0x1F)  //Get the lower 5 bits

//-----------------------------------------------------------------------------
#define GPIO_DO_1                            GPIO_ID(0,19)
#define GPIO_DO_2                            GPIO_ID(0,20)
#define GPIO_DO_3                            GPIO_ID(3,14)
#define GPIO_DO_4                            GPIO_ID(3,15)
#define GPIO_DO_5                            GPIO_ID(3,16)
#define GPIO_DO_6                            GPIO_ID(3,17)

#define GPIO_DI_1                            GPIO_ID(1,15)
#define GPIO_DI_2                            GPIO_ID(1,14)
#define GPIO_DI_3                            GPIO_ID(1,13)
#define GPIO_DI_4                            GPIO_ID(1,12)
#define GPIO_DI_5                            GPIO_ID(0,27)
#define GPIO_DI_6                            GPIO_ID(0,26)
#define GPIO_DI_7                            GPIO_ID(0,23)
#define GPIO_DI_8                            GPIO_ID(0,22)

#define GPIO_485_1_DIR                       GPIO_ID(0,3)
#define GPIO_485_2_DIR                       GPIO_ID(3,4)
#define GPIO_485_3_DIR                       GPIO_ID(1,28)
#define GPIO_485_4_DIR                       GPIO_ID(1,30)

#define GPIO_PWRDN_DET                       GPIO_ID(1,22)    //Power down detection
#define GPIO_BUZZER                          GPIO_ID(1,19)
#define GPIO_PHY0_RESET                      GPIO_ID(0,2)
#define GPIO_PHY1_RESET                      GPIO_ID(2,1)
#define GPIO_WIFI_RESET                      GPIO_ID(1,18)
#define GPIO_4G_RESET                        GPIO_ID(1,31)    //NOTE: HIGH reset
#define GPIO_ZIGBEE_RESET                    GPIO_ID(1,23)
#define GPIO_ZIGBEE_ACK                      GPIO_ID(1,17)
#define GPIO_BT_RESET                        GPIO_ID(1,25)
#define GPIO_BT_WAKEUP                       GPIO_ID(1,24)
#endif // FL335X_DRIVERS_H
