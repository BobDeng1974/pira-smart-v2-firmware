#ifndef RASPBERRY_PI_CONTROL_H
#define RASPBERRY_PI_CONTROL_H

#include <stdint.h>
#include "board.h" 
#include "Arduino.h"

enum state_e 
{
    IDLE,
    WAIT_STATUS_ON,
    WAKEUP,
    REBOOT_DETECTION
};

void stateTransition(state_e next);
bool stateCheckTimeout(void);
char* returnState(state_e state);

void raspiStateMachine(uint32_t onThreshold,
                       uint32_t offThreshold,
                       uint32_t wakeupThreshold,
                       uint32_t rebootThreshold,
                       bool forceOffPeriodEnd);

#endif /* RASPBERRY_PI_CONTROL_H */
