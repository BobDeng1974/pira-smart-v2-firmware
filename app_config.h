#ifndef _APP_CONFIG_H
#define _APP_CONFIG_H

#include <Arduino.h>    

//    #define DEBUG
//    #define DEBUG_BLE
//    #define SEND_TIME_AS_STRING

// Pin defines 
#define POWER_ENABLE_5V     (PB6) 
#define FET_OUTPUT          (PA8) 
// TODO RASPBERRY_PI_STATUS you have to set this pin in raspi software 
#define RASPBERRY_PI_STATUS (PA7) //dummy
// TODO I dont know if this is correct, output values are stuck at 255
#define BATTERY_VOLTAGE     (STM32L0_ADC_CHANNEL_5)



#endif /* _APP_CONFIG_H */
