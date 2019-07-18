#ifndef BATTERY_VOLTAGE_H
#define BATTERY_VOLTAGE_H

#include <stdint.h>
#include "board.h" 
#include "stm32l0_adc.h"

class BatteryVoltage
{
public:
    const static uint8_t    resistorLowerKOhm =     100;  //100kOhm 
    const static uint8_t    resistorUpperKOhm =     226;  //226kOhm 
    constexpr static float  referenceVoltageV =   2.610;   
    const static uint16_t   adcMax            =    4095;   
    const static uint8_t    resolution        =      12;  // 12 bit resolution 

    BatteryVoltage(void);
    uint16_t batteryLevelGet(void);
    float batteryVoltageGet(uint16_t adcValue);

private:
    uint16_t  batteryLevel;
};

#endif /* BATTERY_VOLTAGE_H */
