#include <Arduino.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include "Wire.h"
#include "STM32L0.h"
#include "TimerMillis.h"
#include "RN487x_BLE.h"

#include "board.h"
#include "pira_fsm.h"
#include "battery_voltage.h"
#include "pira_settings.h"

bool flag_execute = 0;
// Timer needed for interrupt
TimerMillis periodicTimer;

// Watchdog is reseted here, flag_execute flag is set here
void periodicCallback(void)
{
    // HW WatchDog reset
    STM32L0.wdtReset();

    flag_execute = 1;
}

void setup(void)
{
    pira_init();

    // Prepare adc readings
    init_battery_adc();

    // Enable watchdog, reset it in periodCallback
    STM32L0.wdtEnable(WATCHDOG_RESET_VALUE_s);

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
}

void loop()
{
    pira_run();
    delay(1000);
}

/*** end of file ***/