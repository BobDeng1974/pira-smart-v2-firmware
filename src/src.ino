#include <Arduino.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include "Wire.h"
#include "STM32L0.h"
#include "TimerMillis.h"
#include "RN487x_BLE.h"

#include "board.h"
#include "RaspberryPiControl.h"
#include "BatteryVoltage.h"
#include "mainFunctions.h"
#include "settings.h"


bool flag_execute = 0;

// Timer needed for interrupt
TimerMillis periodicTimer;

// Watchdog is reseted here, flag_execute flag is set here
void periodicCallback(void)
{
    // HW WatchDog reset
    STM32L0.wdtReset();

    // Set flag to update status every second
    flag_execute = 1;
}

void setup(void)
{
    uint32_t status_error_reset = STM32L0.resetCause();

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
    initRtc(TIME_INIT_VALUE);

    // Prepare adc readings
    init_battery_adc();

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
    if (flag_execute)   // Statement will be executed every second
    {
        flag_execute = 0; // Reset flag

        // Get the current time from RTC
        settings_packet.data.status_time = (uint64_t)time();
#ifdef DEBUG
        time_t time_string = (time_t)settings_packet.data.status_time;
        raspiSerial.print("Time as a basic string = ");
        raspiSerial.println(ctime(&time_string));
        printStatusValues();
#endif

        // Update status values in not in IDLE state
        if(!(status_state_machine == IDLE))
            updateStatusValues();

        raspiStateMachine();
    }
}