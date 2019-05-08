#include "BatteryVoltage.h"
//TODO some adc stuff, leave it commented outfor now
BatteryVoltage::BatteryVoltage(void)
{
//    batteryLevel = 0;    
//    // Initialize ADC peripheral
//    // Enable ADC peripheral
//    NRF_ADC->ENABLE = ADC_ENABLE_ENABLE_Enabled;
//    // 8bit result, convert channel 2 and use interanl VBG reference
//    NRF_ADC->CONFIG = (ADC_CONFIG_RES_8bit                        << ADC_CONFIG_RES_Pos)    |
//                      (ADC_CONFIG_INPSEL_AnalogInputNoPrescaling  << ADC_CONFIG_INPSEL_Pos) |
//                      (ADC_CONFIG_REFSEL_VBG                      << ADC_CONFIG_REFSEL_Pos) |
//                      (ADC_CONFIG_PSEL_AnalogInput2               << ADC_CONFIG_PSEL_Pos)   |
//                      (ADC_CONFIG_EXTREFSEL_None                  << ADC_CONFIG_EXTREFSEL_Pos);
}
 
uint8_t BatteryVoltage::batteryLevelGet(void)
{
//    // Configure channel and start conversion
//    NRF_ADC->CONFIG     &= ~ADC_CONFIG_PSEL_Msk;
//    NRF_ADC->CONFIG     |= ADC_CONFIG_PSEL_AnalogInput2 << ADC_CONFIG_PSEL_Pos;
//    NRF_ADC->TASKS_START = 1;
//    // Wait until conversion is done
//    while (((NRF_ADC->BUSY & ADC_BUSY_BUSY_Msk) >> ADC_BUSY_BUSY_Pos) == ADC_BUSY_BUSY_Busy) {};
//    // Return conversion result
//    batteryLevel = (uint8_t)NRF_ADC->RESULT;
//    return batteryLevel; // 8 bit
}
    
float BatteryVoltage::batteryVoltageGet(uint8_t adcValue)
{
//    return (float)(adcValue * referenceVoltageV * (resistorLowerKOhm + resistorUpperKOhm) / (adcMax * resistorLowerKOhm));
}
