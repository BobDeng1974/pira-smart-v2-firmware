#include "RaspberryPiControl.h"

//#define DEBUG

/**
 * @brief Variables concerning the state of the program
 * @detail
 *      state
 *          It controls current state of state machine
 *      statePrev
 *          It shows the prevoius state of state machine
 *      stateGotoTimeout
 *          It is set everytime when we enter state
 */
state_e state = WAIT_STATUS_ON;     
state_e statePrev = WAIT_STATUS_ON;
state_e stateGotoTimeout;

/**
 * @brief Variables concerning the state of the program
 * @detail
 *      eventLoopStart 
 *          Keeps the time when loop started. 
 *      stateTimeoutDuration
 *          It is set everytime when we enter state, if reached we enter state defined by stateGotoTimeout.
 *      stateTimeoutStart
 *          Set everytime we call stateTransition funtion.
 */
uint32_t transitionTimeout = 0;
uint32_t eventLoopStart = 0;
uint32_t stateTimeoutDuration; 
uint32_t stateTimeoutStart; 

/**
 * @brief change to next state and implement a transitionTimeoutfor each state
 * 
 */
void stateTransition(state_e next)
{
  // mark the time state has been entered
  stateTimeoutStart = millis();

  // update prevous state
  statePrev=state;

  // move to the following state
  state = next;
}

/**
 * @brief check if the state has timed out
 * 
 * @return bool
 */
bool stateCheckTimeout(void)
{
    // transitionTimeout can be disabled
    if(stateTimeoutDuration == 0)
    {
        return false;
    }

    unsigned long elapsed = millis() - stateTimeoutStart;

    //check if we have been in the existing state too long
    if(elapsed >= stateTimeoutDuration)
    {
        //transitionTimeout should be reseted everytime when safey timeout happens, 
        //to ensure normal operation in next state.
        transitionTimeout = 0; 
        return true;
    }
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
 * @param forceOffPeriodEnd
 *
 * @return none (void)
 */
void raspiStateMachine(uint32_t onThreshold,
                       uint32_t offThreshold,
                       uint32_t wakeupThreshold,
                       uint32_t rebootThreshold,
                       bool forceOffPeriodEnd)
{

    eventLoopStart = millis(); // start the timer of the loop

#ifdef DEBUG
    raspiSerial.print("fsm(");
    raspiSerial.print(returnState(statePrev));
    raspiSerial.print(" -> ");
    raspiSerial.print(returnState(state));
    raspiSerial.print(",");
    raspiSerial.print(millis());
    raspiSerial.print(",");
    raspiSerial.print("Timeout = ");
    raspiSerial.print(transitionTimeout);
    raspiSerial.println(")");
    raspiSerial.flush();
#endif

    switch(state)
    {
        case IDLE:
            stateTimeoutDuration = 0;
            stateGotoTimeout = IDLE;

            transitionTimeout++;
            
            //If we enter this state because saftey timeout was triggered, then power should be disabled.
            digitalWrite(POWER_ENABLE_5V, LOW);
            //Typical usecase would be that wakeupThreshold < offThreshold
            if ((transitionTimeout >= offThreshold)    || 
                (transitionTimeout >= wakeupThreshold) || 
                forceOffPeriodEnd)
            {
                //Turn on power supply  and reset transitionTimeoutcounter
                digitalWrite(POWER_ENABLE_5V, HIGH);
                transitionTimeout = 0;
                
                //Change state
                stateTransition(WAIT_STATUS_ON);
            }

            break;

        case WAIT_STATUS_ON:
            stateTimeoutDuration = 0;
            stateGotoTimeout = IDLE;

            //Wait untill RaspberryPi pulls up STATUS pin or untill timeoutWAKEOUT reaches onThreshold value
            transitionTimeout++;

            if(digitalRead(RASPBERRY_PI_STATUS))
            {
                //Transition to WAKEUP
                transitionTimeout = 0;
                stateTransition(WAKEUP);
            }
            else if (transitionTimeout >= onThreshold)
            {
                //Timeout reached threshold value, turn of power off power supply and go to IDLE
                digitalWrite(POWER_ENABLE_5V, LOW);
                transitionTimeout = 0;
                forceOffPeriodEnd = false;
                stateTransition(IDLE);
            }
            break;

        case WAKEUP:
            stateTimeoutDuration = 0;
            stateGotoTimeout = IDLE;

            transitionTimeout++;

            //Check status pin, if down then turn off power supply.
            //Or after timeout, turn off power supply anyway without waiting for status.
            if(!digitalRead(RASPBERRY_PI_STATUS))
            {
                //Transition to reboot
                transitionTimeout = 0;
                stateTransition(REBOOT_DETECTION);
                break;
            }

            if (transitionTimeout >= onThreshold)
            {
                //Turn Off 5V power supply
                digitalWrite(POWER_ENABLE_5V, LOW);

                //Reset transitionTimeoutcounter
                transitionTimeout= 0;
                forceOffPeriodEnd = false;
                stateTransition(IDLE);
            }
            break;

        case REBOOT_DETECTION:
            stateTimeoutDuration = 0;
            stateGotoTimeout = IDLE;

            //Wait for reboot timeout
            transitionTimeout++;

            if (transitionTimeout >= rebootThreshold)
            {
                transitionTimeout = 0;
                if(digitalRead(RASPBERRY_PI_STATUS))
                    // RPi rebooted, go back to wake up
                    stateTransition(WAKEUP);
                else
                {
                    //Definitely turn off RPi power supply
                    digitalWrite(POWER_ENABLE_5V, LOW);
                    forceOffPeriodEnd = false;
                    stateTransition(IDLE);
                }
            }   
            break;

        default:
            state=IDLE;
            break;
    }
    // check if the existing state has timed out and transition to next state
    if(stateCheckTimeout())
    {
#ifdef DEBUG
        raspiSerial.print("timeout in state: ");
        raspiSerial.println(returnState(state));
#endif
        stateTransition(stateGotoTimeout);
    }
}

