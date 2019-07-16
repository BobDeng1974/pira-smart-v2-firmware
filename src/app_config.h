#ifndef _APP_CONFIG_H
#define _APP_CONFIG_H

#include <Arduino.h>    

//    #define DEBUG
//    #define DEBUG_BLE
//    #define SEND_TIME_AS_STRING

#define debugSerial Serial1
#define bleSerial Serial

// Pira pin defines 
#define POWER_ENABLE_5V     (PB6) 
#define FET_OUTPUT          (PA8) 
#define RASPBERRY_PI_STATUS (PB13)
#define BATTERY_VOLTAGE     (STM32L0_ADC_CHANNEL_5)

//BLE pin defines
#define BLUETOOTH_WAKE (27) //(PA0) 
#define BT_RESET  (14)  //(PA4)

#endif /* _APP_CONFIG_H */
