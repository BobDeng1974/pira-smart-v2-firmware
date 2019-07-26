#include "RaspberryPiControl.h"

//#define DEBUG

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
state_e status_state_machine = WAIT_STATUS_ON;
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
  statePrev=status_state_machine;
  status_state_machine = next;
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
 * @param status_state_machine
 *
 * @return char*
 */
char* returnState(state_e status_state_machine)
{
    static char buffer[20];

    if(status_state_machine == IDLE)
        sprintf(buffer, "%s", "IDLE");
    if(status_state_machine == WAIT_STATUS_ON)
        sprintf(buffer, "%s", "WAIT_STATUS_ON");
    if(status_state_machine == WAKEUP)
        sprintf(buffer, "%s", "WAKEUP");
    if(status_state_machine == REBOOT_DETECTION)
        sprintf(buffer, "%s", "REBOOT_DETECTION");

    return buffer;
}

/**
 * @brief Main finite state machine loop for Raspberry Pi
 *
 * @param safety_power_period
 * @param safety_sleep_period
 * @param operational_wakeup
 * @param safety_reboot
 * @param turnOnRpi
 *
 * @return none (void)
 */
void raspiStateMachine(uint32_t safety_power_period,
                       uint32_t safety_sleep_period,
                       uint32_t operational_wakeup,
                       uint32_t safety_reboot,
                       bool turnOnRpi)
{


#ifdef DEBUG
    raspiSerial.print("fsm(");
    raspiSerial.print(returnState(statePrev));
    raspiSerial.print(" -> ");
    raspiSerial.print(returnState(status_state_machine));
    raspiSerial.print(",");
    raspiSerial.print(millis());
    raspiSerial.print(",");
    raspiSerial.print("Timeout = ");
    raspiSerial.print(getOverviewValue());
    raspiSerial.println(")");
    raspiSerial.flush();
#endif

    switch(status_state_machine)
    {
        case IDLE:

            //Typical usecase would be that operational_wakeup < safety_sleep_period
            if(operational_wakeup < safety_sleep_period)
                stateTimeoutDuration = operational_wakeup;
            else
                stateTimeoutDuration = safety_sleep_period;

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

            stateTimeoutDuration = safety_power_period;
            stateGotoTimeout = IDLE;

            // WAIT_STATUS_ON state reached, turn on power for raspberry pi
            digitalWrite(POWER_ENABLE_5V, HIGH);

            // If status pin is read as high go to WAKEUP state
            if(digitalRead(RASPBERRY_PI_STATUS))
                stateTransition(WAKEUP);

            break;

        case WAKEUP:

            stateTimeoutDuration = safety_power_period;
            stateGotoTimeout = IDLE;

            //Check status pin, if low then turn off power supply.
            if(!digitalRead(RASPBERRY_PI_STATUS))
                stateTransition(REBOOT_DETECTION);

            break;

        case REBOOT_DETECTION:

            stateTimeoutDuration = safety_reboot;
            stateGotoTimeout = IDLE;

            if(digitalRead(RASPBERRY_PI_STATUS))
                // RPi rebooted, go back to wake up
                stateTransition(WAKEUP);

            break;

        default:

            status_state_machine=IDLE;

            break;
    }

    // check if the existing state has timed out and transition to next state
    if(stateCheckTimeout())
        stateTransition(stateGotoTimeout);
}

