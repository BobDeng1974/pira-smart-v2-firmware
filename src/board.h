#ifndef BOARD_H
#define BOARD_H

#include <Arduino.h>    

//    #define DEBUG
//    #define DEBUG_BLE

#define bleSerial       Serial
#define raspiSerial     Serial1 

#define TIME_INIT_VALUE             (1514764800UL)  // Initial Time is Mon, 1 Jan 2018 00:00:00
#define ON_PERIOD_INIT_VALUE_s      (7200)
#define OFF_PERIOD_INIT_VALUE_s     (7200)
#define RX_BUFFER_SIZE              (7)             // Size in B, do not change, comunication protocol between Pira and RPi depends on this
#define WATCHDOG_RESET_VALUE_s      (15000)


// Pin defines 
#define POWER_ENABLE_5V     (PB6) 
#define FET_OUTPUT          (PA8) 
#define RASPBERRY_PI_STATUS (PB13)
#define BATTERY_VOLTAGE     (STM32L0_ADC_CHANNEL_5)

#endif /* BOARD_H */
