#include "RaspberryPiControl.h"


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
 *      transitionTimeout
 *          counter variable that is depended on one second execution of raspStateMachine function.
 *          In certain states it is used to trigger a transition between states if it reaches certain value.
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
  // transitionTimeoutcan be disabled
  if(stateTimeoutDuration == 0){
    return false;
  }
  unsigned long elapsed = millis() - stateTimeoutStart;
  //check if we have been in the existing state too long
  if(elapsed >= stateTimeoutDuration){
    return true;
  }
  return false;
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
    raspiSerial.print(statePrev);
    raspiSerial.print(">");
    raspiSerial.print(state);
    raspiSerial.print(",");
    raspiSerial.print(sleep);
    raspiSerial.print(",");
    raspiSerial.print(millis());
    raspiSerial.println(")");
    raspiSerial.flush();
#endif

    switch(state)
    {
        case IDLE:
            stateTimeoutDuration = 0;
            stateGotoTimeout = IDLE;

            transitionTimeout++;

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
            stateTimeoutDuration = 10000;
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
            stateTimeoutDuration = 10000;
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
            stateTimeoutDuration = 10000;
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
        raspiSerial.print("timeout(");
        raspiSerial.print(state);
        raspiSerial.println(")");
#endif
        stateTransition(stateGotoTimeout);
    }
}

