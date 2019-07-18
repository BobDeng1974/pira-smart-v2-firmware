#ifndef MAIN_FUNCTIONS_H
#define MAIN_FUNCTIONS_H

#include <Arduino.h>
#include <stdint.h>
#include <time.h>

#include "Wire.h"
#include "STM32L0.h"            
#include "ISL1208_RTC.h"

#include "board.h" 

//Global variables that are defined in main
extern uint32_t onPeriodValue;
extern uint32_t offPeriodValue;
extern uint32_t rebootThresholdValue;
extern uint32_t wakeupThresholdValue;
extern ISL1208_RTC rtc; 

//Uart related functions
void uartCommandParse(uint8_t *rxBuffer, uint8_t len);
void uartCommandSend(char command, uint32_t data);
void uartCommandReceive(void);

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
