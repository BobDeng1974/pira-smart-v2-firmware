#ifndef RASPBERRY_PI_CONTROL_H
#define RASPBERRY_PI_CONTROL_H

#include <stdint.h>
#include "board.h"
#include "Arduino.h"
#include "pira_functions.h"


enum state_e
{
    IDLE,
    WAIT_STATUS_ON,
    WAKEUP,
    REBOOT_DETECTION,
};
extern state_e status_state_machine;
extern uint32_t elapsed;

void stateTransition(state_e next);
bool stateCheckTimeout(void);
char* returnState(state_e state);

void raspiStateMachine();

#endif /* RASPBERRY_PI_CONTROL_H */
