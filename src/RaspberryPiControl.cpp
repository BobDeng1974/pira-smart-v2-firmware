#include "RaspberryPiControl.h"

RaspberryPiControl::RaspberryPiControl(void)
{
    //Constructor
    state = WAIT_STATUS_ON_STATE;
    timeoutOn = 0;
    timeoutOff = 0;
    timeoutReboot = 0;
}

void RaspberryPiControl::powerHandler(uint32_t onThreshold,
                                      uint32_t offThreshold,
                                      uint32_t wakeupThreshold,
                                      uint32_t rebootThreshold,
                                      bool forceOffPeriodEnd)
{
    switch(state)
    {
        case IDLE_STATE:
            #ifdef DEBUG
                Serial1.println("IDLE_STATE");
                Serial1.print("timeoutOff = ");
                Serial1.println(timeoutOff);
            #endif
            //Check if we need to wakeup RaspberryPi
            timeoutOff++;

            // Typical usecase would be that wakeupThreshold < offThreshold
            if ((timeoutOff >= offThreshold)    || 
                (timeoutOff >= wakeupThreshold) || 
                forceOffPeriodEnd)
            {
                timeoutOff = 0;
                // Turn OFF RaspberryPi and set on threshold value
                //Turn ON 5V power supply
                digitalWrite(POWER_ENABLE_5V, HIGH);
                //powerEnable5V->write(1);
                state = WAIT_STATUS_ON_STATE;
            }

            break;

        case WAIT_STATUS_ON_STATE:
            #ifdef DEBUG
                Serial1.println("WAIT_STATUS_ON_STATE");
                Serial1.print("timeoutON = ");
                Serial1.println(timeoutOn);
            #endif
            //Wait when RaspberryPi pulls up STATUS pin
            //NOTE: Temporarely check reversed logic
            timeoutOn++;

            if(digitalRead(RASPBERRY_PI_STATUS))
            {
                //Add some timeout also
                timeoutOn = 0;
                state = WAKEUP_STATE;
            }
            else if (timeoutOn >= onThreshold)
            {
                //Turn Off 5V power supply
                digitalWrite(POWER_ENABLE_5V, LOW);
                
                //Reset timeout counter
                timeoutOn = 0;
                forceOffPeriodEnd = false;
                state = IDLE_STATE;
            }

            break;

        case WAKEUP_STATE:
            #ifdef DEBUG
                Serial1.println("WAKEUP_STATE");
                Serial1.print("timeoutON =");
                Serial1.println(timeoutOn);
            #endif
            //Send sensor data and when wakeup period set, shutdown
            //In order for timeout to work, this function should be executed every 1s

            //Check status pin and then turn off power supply.
            //Or after timeout, turn off power supply anyway without waiting for status.
            if(!digitalRead(RASPBERRY_PI_STATUS))
            {
                timeoutReboot = 0; 
                timeoutOn = 0;
                state = REBOOT_DETECTION;
                break;
            }

            timeoutOn++;

            if (timeoutOn >= onThreshold)
            {
                //Turn Off 5V power supply
                digitalWrite(POWER_ENABLE_5V, LOW);

                //Reset timeout counter
                timeoutOn = 0;
                forceOffPeriodEnd = false;
                state = IDLE_STATE;
            }
            break;

        case REBOOT_DETECTION:
            #ifdef DEBUG
                Serial1.println("REBOOT_DETECTION");
                Serial1.print("TimeoutReboot = ");
                Serial1.println(timeoutReboot);
            #endif
            //Wait for reboot timeout
            timeoutReboot++;

            if (timeoutReboot >= rebootThreshold)
            {
                timeoutReboot = 0;
                if(digitalRead(RASPBERRY_PI_STATUS))
                    // RPi rebooted, go back to wake up
                    state = WAKEUP_STATE;
                else
                {
                    //Definitely turn off RPi power supply
                    digitalWrite(POWER_ENABLE_5V, LOW);
                    forceOffPeriodEnd = false;
                    state = IDLE_STATE;
                }
            }   
            break;

        default:
            break;
    }
}

