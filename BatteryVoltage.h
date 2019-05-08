#ifndef BATTERY_VOLTAGE_H
#define BATTERY_VOLTAGE_H

#include <stdint.h>
#include "app_config.h" 

class BatteryVoltage
{
public:
    const static uint16_t resistorLowerKOhm = 100;  //100kOhm 
    const static uint16_t resistorUpperKOhm = 249;  //249kOhm 
    constexpr static float    referenceVoltageV = 1.2;   
    const static uint8_t  adcMax            = 255;   
    
    BatteryVoltage(void);
    uint8_t batteryLevelGet(void);
    float batteryVoltageGet(uint8_t adcValue);

private:
    uint8_t  batteryLevel;
};

#endif /* BATTERY_VOLTAGE_H */
