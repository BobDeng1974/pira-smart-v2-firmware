/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <Arduino.h>
#include <stdint.h>
#include <time.h>

#include "Wire.h"
#include "STM32L0.h"            
#include "TimerMillis.h"
#include "ISL1208_RTC.h"
#include "RN487x_BLE.h"

#include "app_config.h" 
#include "RaspberryPiControl.h"
#include "BatteryVoltage.h"
//#include "HWatchDogTimer.h"

// Initial Time is Mon, 1 Jan 2018 00:00:00
#define TIME_INIT_VALUE  1514764800UL

#define ON_PERIOD_INIT_VALUE_s 7200
#define OFF_PERIOD_INIT_VALUE_s 7200


#define RX_BUFFER_SIZE      7        //Size in B

#define HW_WATCHDOG_TIMER_THRESHOLD	10	// value in seconds 

//#define DEBUG


// TODO UART and I2C objects are not needed, rtc object is also different, picontrol and voltge objects are probably okay
// Create UART object
//Serial pc(UART_TX, UART_RX);
//BufferedSerial pc(UART_TX, UART_RX);
// Create I2C object
//I2C i2c(I2C_SDA, I2C_SCL);
// Create ISL1208 object
ISL1208_RTC rtc; 
//ISL1208 rtc(&i2c);    
// RaspberryPiControl object
RaspberryPiControl raspberryPiControl;
// Battery Voltage object
BatteryVoltage batteryVoltage;

// TODO Used later for ble services
//Service pointers declarations

const static char     DEVICE_NAME[] = "PiraSmart";
//static const uint16_t uuid16_list[] = {LEDService::LED_SERVICE_UUID, PiraService::PIRA_SERVICE_UUID};
uint32_t piraStatus;
uint32_t setTimeValue;
uint32_t onPeriodValue;
uint32_t offPeriodValue;
uint32_t rebootThresholdValue;
uint32_t wakeupThresholdValue;
char getTimeValue[26] = "Tue Apr 10 12:00:00 2018\n";
char *temp;
uint8_t sendTime; 
uint8_t batteryLevelContainer;
bool turnOnRpiState;

TimerMillis periodicTimer;

// Tehnically this should be called periodicInterrupt but I will leave old name.  
void periodicCallback(void)
{
    // HW WatchDog reset    
    //HWatchDogTimer::kick();
    // TODO We used to blink led on old board, now we print once per second, we probably wont need this later.
    Serial1.println("blink");
    // Set flag to update status every second    
    sendTime = 1; 

    // TODO check if following is needed with new BLE
    // If BLE connected, count BLE WDT timer
//    if (bleConnected)
//    {
//        bleWatchdogTimer++;
//    }
}
 
// TODO this should probably stay the same, but keep eye on it
/**
 * This callback allows the LEDService to receive updates to the ledState Characteristic.
 *
 * @param[in] params
 *     Information about the characterisitc being updated.
 */
/*
void onDataWrittenCallback(const GattWriteCallbackParams *params) {
    if ((params->handle == ledServicePtr->getValueHandle()) && (params->len == 1)) 
    {
        actuatedLED = *(params->data);
    }
    else if ((params->handle == piraServicePtr->getSetTimeValueHandle()) && (params->len == 4))
    {
        //When time data received (in seconds format) update RTC value
        memset(&setTimeValue, 0x00, sizeof(setTimeValue)); 
        memcpy(&setTimeValue, params->data, params->len);  
        rtc.time((time_t)setTimeValue); 
    }
    else if ((params->handle == piraServicePtr->getOnPeriodSecondsValueHandle()) && (params->len == 4))
    {
        memset(&onPeriodValue, 0x00, sizeof(onPeriodValue)); 
        memcpy(&onPeriodValue, params->data, params->len);  
        //onPeriodValue = *(params->data);
    }
    else if ((params->handle == piraServicePtr->getOffPeriodSecondsValueHandle()) && (params->len == 4))
    {
        memset(&offPeriodValue, 0x00, sizeof(offPeriodValue)); 
        memcpy(&offPeriodValue, params->data, params->len);  
        //offPeriodValue = *(params->data);
    }
    else if ((params->handle == piraServicePtr->getTurnOnRpiStateValueHandle()) && (params->len == 1))
    {
        turnOnRpiState = *(params->data);
    }   
    else if (params->handle == piraServicePtr->getCommandsInterfaceValueHandle())
    {
        memset(&rxBufferBLE, 0x00, sizeof(rxBufferBLE)); 
        memcpy(&rxBufferBLE, params->data, params->len);  
    }
    else if ((params->handle == piraServicePtr->getRebootPeriodSecondsValueHandle()) && (params->len == 4))
    {
        memset(&rebootThresholdValue, 0x00, sizeof(rebootThresholdValue)); 
        memcpy(&rebootThresholdValue, params->data, params->len);  
        //offPeriodValue = *(params->data);
    }
    else if ((params->handle == piraServicePtr->getWakeupPeriodSecondsValueHandle()) && (params->len == 4))
    {
        memset(&wakeupThresholdValue, 0x00, sizeof(wakeupThresholdValue)); 
        memcpy(&wakeupThresholdValue, params->data, params->len);  
        //offPeriodValue = *(params->data);
    }
}
*/ 


// TODO uart parsing shouldnt be hard here with help of arduino
void uartCommandParse(uint8_t *rxBuffer, uint8_t len)
{
    uint8_t firstChar = rxBuffer[0];
    uint8_t secondChar = rxBuffer[1];
    // uint8_t dataLen = (uint8_t)(len - 2);
    // Since data type of data is uint32_t, it is by default dataLen = 4
    // but it is implemented like this in order to later have possibility to update it to different data type 
    // or to use less than 4B
    uint32_t data = 0;

    /*
    for (int i = 0, j = 2; ((i >= (int)dataLen) || (j >= 6)); i++, j++)
    {
        data |= (uint32_t)(rxBuffer[j] << ((3-i)*8));
    }
    */
    data = (rxBuffer[2] << 24) | (rxBuffer[3] << 16) | (rxBuffer[4] << 8) | (rxBuffer[5]);

    if (secondChar == ':')
    {
        switch(firstChar)
        {
            case 't':
                //pc.printf("t: received\n");
                time((time_t)data);
                break;
            case 'p':
                //pc.printf("p: received\n");
                onPeriodValue = data;
                break;
            case 's':
                //pc.printf("s: received\n");
                offPeriodValue = data;
                break;
            case 'c':
                //pc.printf("c: received\n");
                Serial1.println("To be defined how to react on c: command");
                break;
            case 'r':
                //pc.printf("r: received\n");
                rebootThresholdValue = data;
                break;
            case 'w':
                //pc.printf("w: received\n");
                wakeupThresholdValue = data;
                break;
            default:
                break;
        }
    }
}

//TODO change
void uartCommandSend(char command, uint32_t data)
{
//#ifndef DEBUG_BLE
//    pc.putc((int)command);
//    pc.putc(':');
//    pc.putc((int)((data & 0xFF000000)>>24));
//    pc.putc((int)((data & 0x00FF0000)>>16));
//    pc.putc((int)((data & 0x0000FF00)>>8));
//    pc.putc((int)( data & 0x000000FF));
//    pc.putc('\n');
//#endif
}

#ifdef SEND_TIME_AS_STRING
void uartCommandSendArray(char command, char *array, uint8_t len)
{
//    pc.putc((int)command);
//    pc.putc(':');
//    for (int i = 0; i < len; i++)
//    {
//        pc.putc((int)array[i]);
//    }
}
#endif

// TODO solve this diffrently with arduino
void uartCommandReceive(void)
{
    char rxBuffer[8] = "";     //Empty c stirng   
    uint8_t rxIndex = 0;
    if (Serial1.available() != 0)
    {
        delay(10); // Without delay code thinks that it gets only first character first
                   // and then the rest of the string, final result is that they are received seperatly.
                   // A short delay prevents that. 
        while (Serial1.available() > 0)
        {
             rxBuffer[rxIndex] = Serial1.read();

            rxIndex++;
            if (rxBuffer[rxIndex] == '\n')
            {
                //All data withing the packet has been received, parse the packet and execute commands
                if (rxIndex == 7)
                {
                    //pc.printf("All characters has been received\n");
                    Serial1.print("I received: ");
                    Serial1.println(rxBuffer);
                    Serial1.println("All characters has been received");
                    //uartCommandParse(rxBuffer, 8);
                    rxIndex = 0;
                    //break;
                }
                else
                {
                    for (int i = 0; i < RX_BUFFER_SIZE; i++)
                    {
                        rxBuffer[i] = 0;
                    }
                    rxIndex = 0;
                } 
            }

        }

    // TODO old way of reading incoming comand, maek sure is extra functionality is needed at all.
        /*
    while (pc.readable())
    {
        // Receive characters 
        rxBuffer[rxIndex] = pc.getc();

        if (rxIndex == 0)
        {
            if (rxBuffer[rxIndex] != 't' &&
                rxBuffer[rxIndex] != 'p' &&
                rxBuffer[rxIndex] != 's' &&
                rxBuffer[rxIndex] != 'c' &&
                rxBuffer[rxIndex] != 'r' &&
                rxBuffer[rxIndex] != 'w')
            {
                //rxIndex reset is added for clarity
                rxIndex = 0;
                //break should not happen
                //break;
            }
            else
            {
                rxIndex++;
            }
                
        }
        else if (rxIndex == 1)
        {
            if (rxBuffer[rxIndex] != ':')
            {
                rxIndex = 0;
                //break;
            }
            else
            {
                rxIndex++;
            }
        }
        else
        {
            if (rxBuffer[rxIndex] == '\n')
            {
                //All data withing the packet has been received, parse the packet and execute commands
                if (rxIndex == 6)
                {
                    //pc.printf("All characters has been received\n");
                    uartCommandParse(rxBuffer, RX_BUFFER_SIZE);
                    rxIndex = 0;
                    //break;
                }
                else
                {
                    for (int i = 0; i < RX_BUFFER_SIZE; i++)
                    {
                        rxBuffer[i] = 0;
                    }
                    rxIndex = 0;
                } 
            }
            else if (rxIndex == 6)
            {
                for (int i = 0; i < RX_BUFFER_SIZE; i++)
                {
                    rxBuffer[i] = 0;
                }
                rxIndex = 0;
            }
            else
            {
                rxIndex++;
                if (rxIndex > 6)
                {
                    rxIndex = 0;
                }
            }
        }
    }
    */
    }
}


// TODO This should be fine now, check functionality
void init_rtc()
{
    rtc.begin();

    //Try to open the ISL1208
    if(rtc.isRtcActive()) //checks if the RTC is available on the I2C bus
    {
#ifdef DEBUG
        Serial1.println("Device detected!");
#endif

        //Check if we need to reset the time
        Wire.beginTransmission(ISL1208_ADDRESS); //send I2C address of RTC
        Wire.write(ISL1208_SR); //status register
        Wire.endTransmission();
        Wire.requestFrom(ISL1208_ADDRESS,1); // now get the bytes of data...
        uint8_t powerFailed = Wire.read(); //read 1 byte of data

        if(powerFailed & 0x01)
        {
#ifdef DEBUG
            //The time has been lost due to a power complete power failure
            Serial1.println("Device has lost power! Resetting time...");
#endif 
            //Set RTC time to Mon, 1 Jan 2018 00:00:00
            time(TIME_INIT_VALUE);
        }
    }
    else
    {
#ifdef DEBUG
        Serial1.println("Device not detected!");
#endif
    }
}

// TODO this probably deserves its own file 
time_t time()
{
    //Setup a tm structure based on the RTC
    struct tm timeinfo;
    timeinfo.tm_sec = bcd2bin(read8(ISL1208_ADDRESS, ISL1208_SC));
    timeinfo.tm_min = bcd2bin(read8(ISL1208_ADDRESS, ISL1208_MN));
 
    //Make sure we get the proper hour regardless of the mode
    char hours = read8(ISL1208_ADDRESS, ISL1208_HR);
    if (hours & (1 << 7)) 
    {
        //RTC is in 24-hour mode
        timeinfo.tm_hour = bcd2bin(hours & 0x3F);
    } 
    else 
    {
        //RTC is in 12-hour mode
        timeinfo.tm_hour = bcd2bin(hours & 0x1F);
 
        //Check for the PM flag
        if (hours & (1 << 5))
            timeinfo.tm_hour += 12;
    }
 
    //Continue reading the registers
    timeinfo.tm_mday = bcd2bin(read8(ISL1208_ADDRESS, ISL1208_DT));
    timeinfo.tm_mon = bcd2bin(read8(ISL1208_ADDRESS, ISL1208_MO)) - 1;
    timeinfo.tm_year = bcd2bin(read8(ISL1208_ADDRESS, ISL1208_YR)) + 100;
    timeinfo.tm_wday = bcd2bin(read8(ISL1208_ADDRESS, ISL1208_DW));
 
    //Return as a timestamp
    return mktime(&timeinfo);
}

void time(time_t t)
{
    //Convert the time to a tm
    struct tm *timeinfo = localtime(&t);
 
    /* The clock has an 8 bit wide bcd-coded register (they never learn)
     * for the year. tm_year is an offset from 1900 and we are interested
     * in the 2000-2099 range, so any value less than 100 is invalid.
     */
    if (timeinfo->tm_year < 100)
        return;
 
    //Read the old SR register value
    char sr = read8(ISL1208_ADDRESS, ISL1208_SR);
 
    //Enable RTC writing
    write8(ISL1208_ADDRESS, ISL1208_SR, sr | (1 << 4));
 
    //Write the current time
    write8(ISL1208_ADDRESS, ISL1208_SC, bin2bcd(timeinfo->tm_sec));
    write8(ISL1208_ADDRESS, ISL1208_MN, bin2bcd(timeinfo->tm_min));
    write8(ISL1208_ADDRESS, ISL1208_HR, bin2bcd(timeinfo->tm_hour) | (1 << 7));    //24-hour mode
    write8(ISL1208_ADDRESS, ISL1208_DT, bin2bcd(timeinfo->tm_mday));
    write8(ISL1208_ADDRESS, ISL1208_MO, bin2bcd(timeinfo->tm_mon + 1));
    write8(ISL1208_ADDRESS, ISL1208_YR, bin2bcd(timeinfo->tm_year - 100));
    write8(ISL1208_ADDRESS, ISL1208_DW, bin2bcd(timeinfo->tm_wday & 7));
 
    //Disable RTC writing
    write8(ISL1208_ADDRESS, ISL1208_SR, sr);
}
char read8(char addr, char reg)
{
    //Select the register
    Wire.beginTransmission(addr); //send I2C address of RTC
    Wire.write(reg); //status register
    Wire.endTransmission();

    //Read the 8-bit register
    Wire.requestFrom(addr,1); // now get the bytes of data...
 
    //Return the byte
    return Wire.read();
}

void write8(char addr, char reg, char data)
{
    //Select the register
    Wire.beginTransmission(addr); //send I2C address of RTC
    Wire.write(reg); //status register
    
    //Write to register
    Wire.write(data); 
    Wire.endTransmission();
}

unsigned int bcd2bin(unsigned char val)
{
    return (val & 0x0F) + (val >> 4) * 10;
}
 
char bin2bcd(unsigned int val)
{
    return ((val / 10) << 4) + val % 10;
}

void setup(void)
{
    // Initially enable RaspberryPi power
    pinMode(POWER_ENABLE_5V, OUTPUT);
    digitalWrite(POWER_ENABLE_5V, HIGH); 

    // Initialize variables
    sendTime = 0;
    
    //TODO this will be changed 
    // init HW Watchdog timer - attached to blink periodicCallback
    //HWatchDogTimer::init(HW_WATCHDOG_TIMER_THRESHOLD);

    // Start i2c communication
    Wire.begin();

    // UART needs to be initialized first to use it for communication with RPi
    Serial1.begin(115200);
    while (!Serial1) {}

    // RTC init
    init_rtc();

    // periodicCallback must be attached after I2C is initialized
    periodicTimer.start(periodicCallback, 0, 1000); //Starts immediately, repeats every 1000 ms
}

void loop()
{
    uartCommandReceive();
}
/*
void loop()
{    
    while (true) 
    {
        // TODO NEW ble setup

        uartCommandReceive();
        if (sendTime)
        {
            sendTime = 0;

            // Get the current time from RTC 
            time_t seconds = time();
#ifdef DEBUG
            Serial1.println("Time as a basic string = ");
            Serial1.print(ctime(&seconds));
            //pc.printf("Time as a basic string = %s", ctime(&seconds));
#endif
            // Write current time to containter variable which can be read over BLE
            temp = ctime(&seconds);
            memcpy(getTimeValue, temp, strlen((const char *)temp));
            //TODO this will be changed
            //piraServicePtr->updateTime(getTimeValue);
           
#ifdef SEND_TIME_AS_STRING
            // Send time to RaspberryPi in a string format
            Serial1.println("Time as a basic string = ");
            Serial1.print(getTimeValue);
            //pc.printf("t:%s", getTimeValue);
#else
            // Send time in seconds since Jan 1 1970 00:00:00
            // pc.printf("t:%d\n", seconds);
            uartCommandSend('t', seconds);
#endif

            // Update status values
            // Seconds left before next power supply turn off
            piraStatus = onPeriodValue - raspberryPiControl.timeoutOnGet();
            //TODO create a new service later
            //piraServicePtr->updateStatus(&piraStatus);
            // Send status to RPi -> time until next sleep and then battery level
            // pc.printf("p:%d\n", piraStatus);
            uartCommandSend('o', piraStatus);

            batteryLevelContainer = batteryVoltage.batteryLevelGet();
            //TODO solve next line
            //piraServicePtr->updateBatteryLevel(&batteryLevelContainer);
            Serial1.print("b:");
            Serial1.println((uint32_t)batteryLevelContainer);
            uartCommandSend('b', (uint32_t)batteryLevelContainer);

            // Send rest of the values in order to verify changes
            uartCommandSend('p', onPeriodValue);
            uartCommandSend('s', offPeriodValue);
            uartCommandSend('r', rebootThresholdValue);
            uartCommandSend('w', wakeupThresholdValue);
            // Send RPi status pin value
            uartCommandSend('a', (uint32_t)digitalRead(RASPBERRY_PI_STATUS));

            // Update BLE containers
            //piraServicePtr->updateOnPeriodSeconds(&onPeriodValue);
            //piraServicePtr->updateOffPeriodSeconds(&offPeriodValue);
            //piraServicePtr->updateRebootPeriodSeconds(&rebootThresholdValue);
            //piraServicePtr->updateWakeupPeriodSeconds(&wakeupThresholdValue);
#ifdef DEBUG
           
            Serial1.print("Battery level in V = ");
            Serial1.println((int)(batteryVoltage.batteryVoltageGet(batteryLevelContainer)*100));
            Serial1.print("onPeriodValue =");
            Serial1.println(onPeriodValue);
            Serial1.print("offPeriodValue =");
            Serial1.println(offPeriodValue);
            Serial1.print("rebootThresholdValue = ");
            Serial1.println(rebootThresholdValue);
            Serial1.print("wakeupThresholdValue = ");
            Serial1.println(wakeupThresholdValue);
            Serial1.print("turnOnRpiState = ");
            Serial1.println(turnOnRpiState);
            Serial1.print("Status Pin = ");
            Serial1.println(digitalRead(RASPBERRY_PI_STATUS);
#endif
            raspberryPiControl.powerHandler(onPeriodValue,
                                            offPeriodValue,
                                            wakeupThresholdValue,
                                            rebootThresholdValue,
                                            turnOnRpiState);
        }
    }
}   */
