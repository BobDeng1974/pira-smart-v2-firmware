#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "Arduino.h"
#include "board.h"
#include <STM32L0.h>
#include "stm32l0_eeprom.h"

struct settingsData_t{
    uint64_t status_time;
    uint16_t status_battery;
    uint32_t safety_power_period;
    uint32_t safety_sleep_period;
    uint32_t safety_reboot;
    uint32_t operational_wakeup;
    uint8_t turnOnRpi; //TODO actually implement this functionality, it is missing the command from outside
}__attribute__((packed));

union settingsPacket_t{
    settingsData_t data;
    byte bytes[sizeof(settingsData_t)];
};

extern settingsPacket_t settings_packet;

void settings_init(void);

#endif
