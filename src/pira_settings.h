#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "Arduino.h"
#include "board.h"
#include <STM32L0.h>

struct settingsPira_t{
    uint64_t status_time;
    uint16_t status_battery;
    uint32_t safety_power_period;
    uint32_t safety_sleep_period;
    uint32_t safety_reboot;
    uint32_t operational_wakeup;
    uint8_t turnOnRpi; //TODO actually implement this functionality, it is missing the command from outside
}__attribute__((packed));

union settingsPacket_t{
    settingsPira_t data;
    uint8_t bytes[sizeof(settingsPira_t)];
};

extern settingsPacket_t settings_packet;

void settings_init(void);

#endif

/*** end of file ***/