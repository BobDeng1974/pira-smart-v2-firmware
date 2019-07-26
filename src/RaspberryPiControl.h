#ifndef RASPBERRY_PI_CONTROL_H
#define RASPBERRY_PI_CONTROL_H

#include <stdint.h>
#include "board.h"
#include "Arduino.h"
#include "mainFunctions.h"


enum state_e
{
    IDLE,
    WAIT_STATUS_ON,
    WAKEUP,
    REBOOT_DETECTION,
};

void stateTransition(state_e next);
bool stateCheckTimeout(void);
char* returnState(state_e state);

void raspiStateMachine(uint32_t safety_power_period,
                       uint32_t safety_sleep_period,
                       uint32_t operational_wakeup,
                       uint32_t safety_reboot,
                       bool turnOnRpi);

#endif /* RASPBERRY_PI_CONTROL_H */
