#include "mainFunctions.h"

ISL1208_RTC rtc;
/**
 * @brief Function parses recived commands depending on starting character
 *
 * @param *rxBuffer
 *
 * @return none (void)
 */
void uartCommandParse(uint8_t *rxBuffer)
{
    uint8_t firstChar = rxBuffer[0];
    uint8_t secondChar = rxBuffer[1];
    uint32_t data = 0;

    data = (rxBuffer[2] << 24) | (rxBuffer[3] << 16) | (rxBuffer[4] << 8) | (rxBuffer[5]);

    if (secondChar == ':')
    {
        switch(firstChar)
        {
            case 't':
                //raspiSerial.println("t: received");
                time((time_t)data);
                break;
            case 'p':
                //raspiSerial.println("p: received");
                settings_packet.data.safety_power_period = data;
                break;
            case 's':
                //raspiSerial.println("s: received");
                settings_packet.data.safety_sleep_period = data;
                break;
            case 'c':
                //raspiSerial.println("c: received");
                //raspiSerial.println("To be defined how to react on c: command");
                break;
            case 'r':
                //raspiSerial.println("r: received");
                settings_packet.data.safety_reboot = data;
                break;
            case 'w':
                //raspiSerial.println("w: received");
                settings_packet.data.operational_wakeup = data;
                break;
            default:
                break;
        }
    }
    else
        // TODO This serial will go back to RPI, should you send something else?
        raspiSerial.print("Incorrect format, this shouldn't happen!");
}

/**
 * @brief Encodes and sends data over uart
 *
 * @param command
 * @param data
 *
 * @return none (void)
 */
void uartCommandSend(char command, uint32_t data)
{
    raspiSerial.write((int)command);
    raspiSerial.write(':');
    raspiSerial.write((int)((data & 0xFF000000)>>24));
    raspiSerial.write((int)((data & 0x00FF0000)>>16));
    raspiSerial.write((int)((data & 0x0000FF00)>>8));
    raspiSerial.write((int)( data & 0x000000FF));
    raspiSerial.write('\n');
}

/**
 * @brief Receives uart data
 *
 * @detail Data should be of exact specified format,
 *         otherwise all received data is going to be rejected
 *
 * @return none (void)
 */
void uartCommandReceive(void)
{
    uint8_t rxBuffer[RX_BUFFER_SIZE] = "";
    uint8_t rxIndex = 0;
    if (raspiSerial.available() != 0)
    {
        delay(10); // Without delay code thinks that it gets only first character first
                   // and then the rest of the string, final result is that they are received seperatly.
                   // A short delay prevents that.
        while (raspiSerial.available() > 0)
        {
            rxBuffer[rxIndex] = raspiSerial.read();

            if (rxIndex == 0)
            {
                if (rxBuffer[rxIndex] != 't' &&
                    rxBuffer[rxIndex] != 'p' &&
                    rxBuffer[rxIndex] != 's' &&
                    rxBuffer[rxIndex] != 'c' &&
                    rxBuffer[rxIndex] != 'r' &&
                    rxBuffer[rxIndex] != 'w')
                {
                    // Anything received that is not by protocol is discarded!
                    rxIndex = 0;
                }
                else
                    // By protocol, continue receiving.
                    rxIndex++;
            }
            else if (rxIndex == 1)
            {
                if (rxBuffer[rxIndex] != ':')
                    // Anything received that is not by protocol is discarded!
                    rxIndex = 0;
                else
                    // By protocol, continue receiving.
                    rxIndex++;
            }
            else
            {
                if (rxBuffer[rxIndex] == '\n')
                {
                    //All data withing the packet has been received, parse the packet and execute commands
                    if (rxIndex == (RX_BUFFER_SIZE - 1))
                    {
                        //raspiSerial.print("I received: ");
                        //raspiSerial.print(rxBuffer);
                        uartCommandParse(rxBuffer);
                        rxIndex = 0;
                    }
                    else if (rxIndex == (RX_BUFFER_SIZE - 2))
                    {
                        // Sent data could be number 10, which in ascii is equal to \n
                        // This else if statement prevents the number 10 from being discarded
                        rxIndex++;
                    }
                    else
                    {
                        // Incorrect length, clean up buffer
                        for (int i = 0; i < RX_BUFFER_SIZE; i++)
                        {
                            rxBuffer[i] = 0;
                        }
                        rxIndex = 0;
                    }
                }
                else if (rxIndex == (RX_BUFFER_SIZE - 1))
                {
                    // We reached max lenght, but no newline, empty buffer
                    for (int i = 0; i < RX_BUFFER_SIZE; i++)
                    {
                        rxBuffer[i] = 0;
                    }
                    rxIndex = 0;
                }
                else
                {
                    rxIndex++;
                    if (rxIndex > (RX_BUFFER_SIZE - 1))
                        rxIndex = 0;
                }
            }
        }
    }
}

/**
 * @brief Sends status values over uart
 *
 * @return none (void)
 */
void updateStatusValues(void)
{
    uartCommandSend('t', (uint32_t)settings_packet.data.status_time);
    uartCommandSend('o', getOverviewValue());
    uartCommandSend('b', (uint32_t)get_battery_voltage(get_raw_battery_voltage()));
    uartCommandSend('p', settings_packet.data.safety_power_period);
    uartCommandSend('s', settings_packet.data.safety_sleep_period);
    uartCommandSend('r', settings_packet.data.safety_reboot);
    uartCommandSend('w', settings_packet.data.operational_wakeup);
    uartCommandSend('a', (uint32_t)digitalRead(RASPBERRY_PI_STATUS));
    uartCommandSend('m', status_state_machine);
}

/**
 * @brief Prints status values to uart, used for debugging only
 *
 * @return none (void)
 */
void printStatusValues(void)
{
    raspiSerial.print("Battery level in V = ");
    raspiSerial.println(get_battery_voltage(get_raw_battery_voltage()));
    raspiSerial.print("safety_power_period =");
    raspiSerial.println(settings_packet.data.safety_power_period);
    raspiSerial.print("safety_sleep_period =");
    raspiSerial.println(settings_packet.data.safety_sleep_period);
    raspiSerial.print("safety_reboot = ");
    raspiSerial.println(settings_packet.data.safety_reboot);
    raspiSerial.print("operational_wakeup = ");
    raspiSerial.println(settings_packet.data.operational_wakeup);
    raspiSerial.print("turnOnRpiState = ");
    raspiSerial.println(settings_packet.data.turnOnRpi);
    raspiSerial.print("Status Pin = ");
    raspiSerial.println(digitalRead(RASPBERRY_PI_STATUS));
    raspiSerial.print("Overview = ");
    raspiSerial.println(getOverviewValue());
}

/**
 * @brief Gets overview value, it depends in which state is currently pira
 *
 * @return uint32
 */
uint32_t getOverviewValue(void)
{
    // Calculate overview value
    if(status_state_machine == WAIT_STATUS_ON || status_state_machine == WAKEUP)
        return settings_packet.data.safety_power_period - elapsed;
    else if(status_state_machine == REBOOT_DETECTION)
        return settings_packet.data.safety_reboot - elapsed;

    return 0;
}

/**
 * @brief Intializes rtc and sets init time value
 *
 * @return none (void)
 */
void initRtc(time_t t)
{
    rtc.begin();

    //Try to open the ISL1208
    if(rtc.isRtcActive()) //checks if the RTC is available on the I2C bus
    {
#ifdef DEBUG
        raspiSerial.println("RTC detected!");
#endif
        //Check if we need to reset the time
        uint8_t powerFailed = read8(ISL1208_ADDRESS, ISL1208_SR);   //read 1 byte of data

        if(powerFailed & 0x01)
        {
#ifdef DEBUG
            //The time has been lost due to a power complete power failure
            raspiSerial.println("RTC has lost power! Resetting time...");
#endif
            //Set RTC time to Mon, 1 Jan 2018 00:00:00
            time(t);
        }
    }
    else
    {
#ifdef DEBUG
        raspiSerial.println("RTC not detected!");
#endif
    }
}

/**
 * @brief Reads time from RTC
 *
 * @return time_t
 */
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
    timeinfo.tm_mon  = bcd2bin(read8(ISL1208_ADDRESS, ISL1208_MO)) - 1;
    timeinfo.tm_year = bcd2bin(read8(ISL1208_ADDRESS, ISL1208_YR)) + 100;
    timeinfo.tm_wday = bcd2bin(read8(ISL1208_ADDRESS, ISL1208_DW));

    //Return as a timestamp
    return mktime(&timeinfo);
}

/**
 * @brief Writes time to RTC
 *
 * @return none (void)
 */
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

/**
 * @brief Reads a register over I2C
 *
 * @param addr
 * @param reg
 *
 * @return char
 */
char read8(char addr, char reg)
{
    //Select the register
    Wire.beginTransmission(addr); //send I2C address of RTC
    Wire.write(reg); //status register
    Wire.endTransmission();

    //Read the 8-bit register
    Wire.requestFrom(addr, 1); // now get the bytes of data...

    //Return the byte
    return Wire.read();
}


/**
 * @brief Writes to a register over I2C
 *
 * @param addr
 * @param reg
 * @param data
 *
 * @return none (void)
 */
void write8(char addr, char reg, char data)
{
    //Select the register
    Wire.beginTransmission(addr); //send I2C address of RTC
    Wire.write(reg); //status register

    //Write to register
    Wire.write(data);
    Wire.endTransmission();
}

/**
 * @brief Converts binary coded decimal to binary
 *
 * @param val
 *
 * @return unsinged int
 */
unsigned int bcd2bin(unsigned char val)
{
    return (val & 0x0F) + (val >> 4) * 10;
}

/**
 * @brief Converts binary to binary coded decimal
 *
 * @param val
 *
 * @return char
 */
char bin2bcd(unsigned int val)
{
    return ((val / 10) << 4) + val % 10;
}

