#include "RaspberryPiControl.h"

#define DEBUG

/**
 * @brief Variables concerning the state of the program
 * @detail
 *      state
 *          It keeps the current state of state machine
 *      statePrev
 *          It keeps the previous state of state machine
 *      stateGotoTimeout
 *          It keeps state that should be entered in case of time out.
 *          It is set everytime when we enter state.
 *      elapsed
 *          It keeps how much time in ms elapsed since we entered a state
 *      stateTimeoutDuration
 *          If elapsed is larger than stateTimeoutStart then state timeouted.
 *          It is set everytime when we enter state.
 *      stateTimeoutStart
 *          Set everytime we call stateTransition funtion.
 */
state_e state = WAIT_STATUS_ON;
state_e statePrev = WAIT_STATUS_ON;
state_e stateGotoTimeout;
uint32_t elapsed;
uint32_t stateTimeoutDuration;
uint32_t stateTimeoutStart;

/**
 * @brief Transitions to next state and saves the time when the state was entered.
 *
 */
void stateTransition(state_e next)
{
  stateTimeoutStart = millis();
  statePrev=state;
  state = next;
}

/**
 * @brief check if the state has timed out
 *
 * @return bool
 */
bool stateCheckTimeout(void)
{
    // stateTimeoutDuration can be disabled
    if(stateTimeoutDuration == 0)
        return false;

    elapsed = millis() - stateTimeoutStart;
    elapsed = elapsed/1000; // All values come in seconds, so we also need elapsed in seconds

    //check if we have been in the current state too long
    if(elapsed >= stateTimeoutDuration)
        return true;
    else
        return false;
}

/**
 * @brief Returns state string from given state enum
 *
 * @param state
 *
 * @return char*
 */
char* returnState(state_e state)
{
    static char buffer[20];

    if(state == IDLE)
        sprintf(buffer, "%s", "IDLE");
    if(state == WAIT_STATUS_ON)
        sprintf(buffer, "%s", "WAIT_STATUS_ON");
    if(state == WAKEUP)
        sprintf(buffer, "%s", "WAKEUP");
    if(state == REBOOT_DETECTION)
        sprintf(buffer, "%s", "REBOOT_DETECTION");

    return buffer;
}

/**
 * @brief Main finite state machine loop for Raspberry Pi
 *
 * @param onThreshold
 * @param offThreshold
 * @param wakeupThreshold
 * @param rebootThreshold
 * @param turnOnRpi
 *
 * @return none (void)
 */
void raspiStateMachine(uint32_t onThreshold,
                       uint32_t offThreshold,
                       uint32_t wakeupThreshold,
                       uint32_t rebootThreshold,
                       bool turnOnRpi)
{


#ifdef DEBUG
    raspiSerial.print("fsm(");
    raspiSerial.print(returnState(statePrev));
    raspiSerial.print(" -> ");
    raspiSerial.print(returnState(state));
    raspiSerial.print(",");
    raspiSerial.print(millis());
    raspiSerial.print(",");
    raspiSerial.print("Timeout = ");
    raspiSerial.print(getOverviewValue());
    raspiSerial.println(")");
    raspiSerial.flush();
#endif

    switch(state)
    {
        case IDLE:

            //Typical usecase would be that wakeupThreshold < offThreshold
            if(wakeupThreshold < offThreshold)
                stateTimeoutDuration = wakeupThreshold;
            else
                stateTimeoutDuration = offThreshold;

            stateGotoTimeout = WAIT_STATUS_ON;

            // IDLE state reached, turn off power for raspberry pi
            digitalWrite(POWER_ENABLE_5V, LOW);

            if(turnOnRpi)
            {
                turnOnRpi = false;

                //Change state
                stateTransition(WAIT_STATUS_ON);
            }
            break;

        case WAIT_STATUS_ON:

            stateTimeoutDuration = onThreshold;
            stateGotoTimeout = IDLE;

            // WAIT_STATUS_ON state reached, turn on power for raspberry pi
            digitalWrite(POWER_ENABLE_5V, HIGH);

            // If status pin is read as high go to WAKEUP state
            if(digitalRead(RASPBERRY_PI_STATUS))
                stateTransition(WAKEUP);

            break;

        case WAKEUP:

            stateTimeoutDuration = onThreshold;
            stateGotoTimeout = IDLE;

            //Check status pin, if low then turn off power supply.
            if(!digitalRead(RASPBERRY_PI_STATUS))
                stateTransition(REBOOT_DETECTION);

            break;

        case REBOOT_DETECTION:

            stateTimeoutDuration = rebootThreshold;
            stateGotoTimeout = IDLE;

            if(digitalRead(RASPBERRY_PI_STATUS))
                // RPi rebooted, go back to wake up
                stateTransition(WAKEUP);

            break;

        default:

            state=IDLE;

            break;
    }

    // check if the existing state has timed out and transition to next state
    if(stateCheckTimeout())
        stateTransition(stateGotoTimeout);
}

