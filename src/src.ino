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
uint32_t status_active_pin;
uint32_t setTimeValue;
uint32_t safety_power_period;
uint32_t safety_sleep_period;
uint32_t safety_reboot;
uint32_t operational_wakeup;
char getTimeValue[26] = "Tue Apr 10 12:00:00 2018\n";
char *temp;
uint8_t sendTime;
uint16_t status_battery;
bool turnOnRpi; //TODO actually implement this functionality, it is missing the command from outside
uint32_t status_error_reset;
time_t status_time;

extern state_e status_state_machine;

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
    status_error_reset = STM32L0.resetCause();

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
    status_active_pin = 0;
    safety_power_period = ON_PERIOD_INIT_VALUE_s;
    safety_sleep_period = OFF_PERIOD_INIT_VALUE_s;
    operational_wakeup = OFF_PERIOD_INIT_VALUE_s;
    safety_reboot = REBOOT_TIMEOUT_s;
    status_battery = 0;
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
    raspiSerial.println(status_error_reset);
#endif
    uartCommandSend('e', status_error_reset);                                   // Send reset cause
}

void loop()
{
    uartCommandReceive();
    if (sendTime)   // Statement will be executed every second
    {
        sendTime = 0; // Reset flag

        // Get the current time from RTC
        status_time = time();
#ifdef DEBUG
        raspiSerial.print("Time as a basic string = ");
        raspiSerial.println(ctime(&status_time));
#endif
        // Write current time to containter variable which can be read over BLE
        temp = ctime(&status_time);
        memcpy(getTimeValue, temp, strlen((const char *)temp));

        // Get battery voltage in ADC counts
        status_battery = batteryVoltage.batteryLevelGet();

        // Update status values in not in IDLE state
        if(!(status_state_machine == IDLE))
            updateStatusValues();

#ifdef DEBUG
        printStatusValues();
#endif
         raspiStateMachine(safety_power_period,
                           safety_sleep_period,
                           operational_wakeup,
                           safety_reboot,
                           turnOnRpi);
    }
}
