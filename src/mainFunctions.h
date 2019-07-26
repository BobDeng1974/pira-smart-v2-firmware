#ifndef MAIN_FUNCTIONS_H
#define MAIN_FUNCTIONS_H

#include <Arduino.h>
#include <stdint.h>
#include <time.h>

#include "Wire.h"
#include "STM32L0.h"            
#include "ISL1208_RTC.h"

#include "board.h" 
#include "BatteryVoltage.h"
#include "RaspberryPiControl.h"

//Global variables that are defined in main
extern uint32_t safety_power_period;
extern uint32_t safety_sleep_period;
extern uint32_t safety_reboot;
extern uint32_t operational_wakeup;
extern bool turnOnRpi;
extern ISL1208_RTC rtc; 
extern BatteryVoltage batteryVoltage;
extern uint16_t status_battery;
extern time_t status_time; 

//Variable that is defined in RaspberryPiControl.h
extern uint32_t elapsed;  

//Uart related functions
void uartCommandParse(uint8_t *rxBuffer);
void uartCommandSend(char command, uint32_t data);
void uartCommandReceive(void);
void updateStatusValues(void);
void printStatusValues(void);
uint32_t getOverviewValue(void);

//RTC related funtions
void initRtc();
time_t time();
void time(time_t t);

//I2C related funtions
char read8(char addr, char reg);
void write8(char addr, char reg, char data);
unsigned int bcd2bin(unsigned char val);
char bin2bcd(unsigned int val);

#endif /* MAIN_FUNCTIONS_H */
