#ifndef RASPBERRY_PI_CONTROL_H
#define RASPBERRY_PI_CONTROL_H

#include <Arduino.h>
#include <stdint.h>
#include <time.h>

#include "Wire.h"
#include "STM32L0.h"            
#include "ISL1208_RTC.h"

#include "board.h" 
#include "battery_voltage.h"
#include "pira_fsm.h"
#include "settings.h"

// Uart related functions
void uart_command_parse(uint8_t *rxBuffer);
void uart_command_send(char command, uint32_t data);
void uart_command_receive(void);
void send_status_values(void);
void print_status_values(void);

// RTC related functions
void init_rtc(time_t);
time_t time();
void time(time_t t);

// I2C related functions
char read8(char addr, char reg);
void write8(char addr, char reg, char data);
unsigned int bcd2bin(unsigned char val);
char bin2bcd(unsigned int val);

// Pira enumerated state variable
enum state_e
{
    IDLE,
    WAIT_STATUS_ON,
    WAKEUP,
    REBOOT_DETECTION,
};

// Pira fsm related functions and variable
extern state_e status_pira_state_machine;
extern uint32_t elapsed;

uint32_t get_overview_value(void);
void state_transition(state_e next);
bool state_check_timeout(void);
char* return_state(state_e state);
void pira_state_machine();
void pira_init();
void pira_run();
#endif /* RASPBERRY_PI_CONTROL_H */