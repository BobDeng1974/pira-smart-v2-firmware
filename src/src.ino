#include <Arduino.h>
#include <stdint.h>
#include <time.h>

#include "Wire.h"
#include "STM32L0.h"
#include "TimerMillis.h"
#include "ISL1208_RTC.h"
#include "RN487x_BLE.h"

#include "board.h"
#include "RaspberryPiControl.h"
#include "BatteryVoltage.h"
#include "mainFunctions.h"

ISL1208_RTC rtc;
BatteryVoltage batteryVoltage;

const static char DEVICE_NAME[] = "PiraSmart";
uint32_t piraStatus;
uint32_t setTimeValue;
uint32_t onPeriodValue;
uint32_t offPeriodValue;
uint32_t rebootThresholdValue;
uint32_t wakeupThresholdValue;
char getTimeValue[26] = "Tue Apr 10 12:00:00 2018\n";
char *temp;
uint8_t sendTime;
uint16_t batteryLevelContainer;
bool turnOnRpi; //TODO actually implement this functionality, it is missing the command from outside
uint32_t resetCause;
time_t seconds;

extern state_e state;

// Timer needed for interrupt
TimerMillis periodicTimer;

// Watchdog is reseted here, sendTime flag is set here
void periodicCallback(void)
{
    // HW WatchDog reset
    STM32L0.wdtReset();

    // Set flag to update status every second
    sendTime = 1;
}

void setup(void)
{
    // TODO port these values to RPi
    //#define STM32L0_SYSTEM_RESET_POWERON           0
    //#define STM32L0_SYSTEM_RESET_EXTERNAL          1
    //#define STM32L0_SYSTEM_RESET_SOFTWARE          2
    //#define STM32L0_SYSTEM_RESET_WATCHDOG          3
    //#define STM32L0_SYSTEM_RESET_FIREWALL          4
    //#define STM32L0_SYSTEM_RESET_OTHER             5
    //#define STM32L0_SYSTEM_RESET_STANDBY           6
    resetCause = STM32L0.resetCause();

    // Initially enable RaspberryPi power
    pinMode(POWER_ENABLE_5V, OUTPUT);
    digitalWrite(POWER_ENABLE_5V, HIGH);

    // Prepare status pin
    pinMode(RASPBERRY_PI_STATUS, INPUT_PULLDOWN);

    // Enable watchdog, reset it in periodCallback
    STM32L0.wdtEnable(WATCHDOG_RESET_VALUE_s);

    // Start I2C communication
    Wire.begin();

    // Start Uart communication
    raspiSerial.begin(115200);
    while(!raspiSerial){}

    // RTC init
    initRtc();

    //Set ON and OFF period values to 30min by default
    // Initialize variables
    sendTime = 0;
    setTimeValue = TIME_INIT_VALUE;
    piraStatus = 0;
    onPeriodValue = ON_PERIOD_INIT_VALUE_s;
    offPeriodValue = OFF_PERIOD_INIT_VALUE_s;
    wakeupThresholdValue = OFF_PERIOD_INIT_VALUE_s;
    rebootThresholdValue = REBOOT_TIMEOUT_s;
    batteryLevelContainer = 0;
    turnOnRpi = 0;

    // periodicCallback must be attached after I2C is initialized
    periodicTimer.start(periodicCallback, 0, 1000); //Starts immediately, repeats every 1000 ms

    //TODO temporary buzzer pulse in case if Pira resets itself
    pinMode(PB7, OUTPUT);
    digitalWrite(PB7, HIGH);
    delay(500);
    digitalWrite(PB7, LOW);
    delay(500);
    digitalWrite(PB7, HIGH);
    delay(500);
    digitalWrite(PB7, LOW);
    delay(500);


#ifdef DEBUG
    raspiSerial.print("Cause for reset = ");
    raspiSerial.println(resetCause);
#endif
    uartCommandSend('j', resetCause);                                   // Send reset cause
}

void loop()
{
    uartCommandReceive();
    if (sendTime)   // Statement will be executed every second
    {
        sendTime = 0; // Reset flag

        // Get the current seconds from RTC
        seconds = time();
#ifdef DEBUG
        raspiSerial.print("Time as a basic string = ");
        raspiSerial.println(ctime(&seconds));
#endif
        // Write current seconds to containter variable which can be read over BLE
        temp = ctime(&seconds);
        memcpy(getTimeValue, temp, strlen((const char *)temp));

        // Get battery voltage in ADC counts
        batteryLevelContainer = batteryVoltage.batteryLevelGet();

        // Update status values in not in IDLE state
        if(!(state == IDLE))
            updateStatusValues();

#ifdef DEBUG
        printStatusValues();
#endif
         raspiStateMachine(onPeriodValue,
                           offPeriodValue,
                           wakeupThresholdValue,
                           rebootThresholdValue,
                           turnOnRpi);
    }
}
