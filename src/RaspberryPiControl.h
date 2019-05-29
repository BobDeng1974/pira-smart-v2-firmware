#ifndef RASPBERRY_PI_CONTROL_H
#define RASPBERRY_PI_CONTROL_H

#include <stdint.h>
#include "app_config.h" 

//#define DEBUG
class RaspberryPiControl 
{
public:

    const static uint32_t REBOOT_TIMEOUT_s = 60;    //1 minute

    enum ControlState
    {
        IDLE_STATE             = 0,
        WAIT_STATUS_ON_STATE   = 1,
        WAKEUP_STATE           = 2,
        REBOOT_DETECTION       = 3,
    };
    
    RaspberryPiControl(void);

    void powerHandler(void);

    void powerHandler(uint32_t onThreshold,
                      uint32_t offThreshold,
                      uint32_t wakeupThreshold,
                      uint32_t rebootThreshold,
                      bool forceOffPeriodEnd);

    uint32_t timeoutOnGet(void)
    {
        return timeoutOn;
    }

    uint32_t timeoutOffGet(void)
    {
        return timeoutOff;
    }

private:
    uint8_t  state;
    uint32_t timeoutOn;
    uint32_t timeoutOff;
    uint32_t timeoutReboot;
};

#endif /* RASPBERRY_PI_CONTROL_H */
