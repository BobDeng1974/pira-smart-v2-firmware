#include "BatteryVoltage.h"
//TODO some adc stuff, leave it commented outfor now
BatteryVoltage::BatteryVoltage(void)
{
    batteryLevel = 0;    
    // Enable adc  
    stm32l0_adc_enable();
}
 
uint16_t BatteryVoltage::batteryLevelGet(void)
{
    // Configure channel and start conversion
    batteryLevel = (uint16_t)stm32l0_adc_read(BATTERY_VOLTAGE, 5);
    //batteryLevel = (uint16_t)analogRead(BATTERY_VOLTAGE);
    return batteryLevel; // 12 bit
}
    
float BatteryVoltage::batteryVoltageGet(uint16_t adcValue)
{
    return (adcValue * referenceVoltageV * (resistorLowerKOhm + resistorUpperKOhm) / (float)(adcMax * resistorLowerKOhm));
}
