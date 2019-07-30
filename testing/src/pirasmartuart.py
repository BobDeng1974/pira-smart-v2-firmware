import time
import RPi.GPIO as gpio
import serial
import datetime
import struct
from binascii import unhexlify

class PIRASMARTUART(object):
    """PIRASMARTUART driver."""

    # Last measured values that can be accessed by other modules.
    pira_time = None
    pira_voltage = None
    pira_on_timer = None

    def __init__(self, portId, boot):

        self.ser = None
        self.portId = portId
        self.boot = boot
        try:

            self.ser = serial.Serial(self.portId, baudrate=115200, stopbits=1, parity="N",  timeout=2)

        except (Exception):
            raise pirasmartuartException

    def read(self, timeout=5, preamble="t:"):
        """Read value from pira smart via uart.

        :param timeout: Timeout
        :return: Value

        t:<uint32_t> time - seconds in epoch format
        o:<uint32_t> overwiev - time left until next sleep
        b:<uint32_t> battery - level in ADC units
        p:<uint32_t> power - safety on period
        s:<uint32_t> sleep - safety off period
        r:<uint32_t> reboot - reboot period duration
        w:<uint32_t> wakeup - period for next wakeup
        a:<uint32_t> active - Pi status pin value
        c:<uint32_t> command - not yet implemented
        """

        start = time.time()
        #reset values
        self.pira_time = None # t
        self.pira_overview = None # o
        self.pira_voltage = None # b
        self.pira_on_timer_get = None # p
        self.pira_sleep = None # s
        self.pira_reboot = None # r
        self.pira_next_wakeup_get = None # w
        self.pira_rpi_gpio = None # a
        self.pira_command = None # c
        self.pira_reset_cause = None # e
        self.pira_state = None # m

        read_timeout = 0    # handles when pira ble is not connected
        value = 0.0         # float that pira ble will use

        try:
            self.ser.flushInput()
        except:
            print("WARNING: Pira input buffer flush failed.")

        while (self.pira_time == None) or \
                (self.pira_overview == None) or \
                (self.pira_voltage == None) or \
                (self.pira_on_timer_get == None) or \
                (self.pira_sleep == None) or \
                (self.pira_reboot == None) or \
                (self.pira_next_wakeup_get == None) or \
                (self.pira_state == None) or \
                (self.pira_rpi_gpio == None) and not \
                (time.time() - start < timeout):

            try:
                x = ""
                x = self.ser.read(7) # Reads 7 bytes, not until the end of the line like ser.readline() did

                #print("Preamble: " + x[0:2] + "Data: " + x[2:-1].encode('hex') + " Line: " + str(x.startswith(preamble)))
                #' '.join(map(lambda x:x.encode('hex'),x))
                #struct.unpack('<h', unhexlify(s1))[0]
                #if len(x) == 6:
                #    value = 10.0
                #else:

                # Following if statement is needed in case Pira sends nothing,
                # in that case we assume that Pira is in IDLE state
                if len(x) == 0:
                    self.pira_state = self.translate_state(int(0))
                    return False
                else:
                    value = float(struct.unpack('>L', x[2:6])[0])
            except:
                print("WARNING: read from Pira BLE the following: " + str(x[2:6]))
                time.sleep(1)
                read_timeout += 1
                if read_timeout >= 5:   # after failing 5 or more times stop Pira BLE reading
                    return False

            if x.startswith(str('t:')):
                self.pira_time = float(value)
                #print ("Pira time: " + str(self.pira_time))
            elif x.startswith(str('o:')):
                self.pira_overview = float(value)
                #print ("Pira overview: " + str(self.pira_overview))
            elif x.startswith(str('b:')):
                self.pira_voltage = float(value)*0.0164     # value for pirasmart v1
                #self.pira_voltage = float(value)*0.0020698 # value for pirasmart v2 
                #print "Pira battery: " + str(self.pira_voltage)
            elif x.startswith(str('p:')):
                self.pira_on_timer_get = float(value)
                #print "Pira get safety on period: " + str(self.pira_on_timer_get)
            elif x.startswith(str('s:')):
                self.pira_sleep = float(value)
                #print "Pira get safety off period: " + str(self.pira_sleep)
            elif x.startswith(str('r:')):
                self.pira_reboot = float(value)
                #print "Pira get reboot period: " + str(self.pira_reboot)
            elif x.startswith(str('w:')):
                self.pira_next_wakeup_get = float(value)
                #print "Pira next wakeup get : " + str(self.pira_next_wakeup_get)
            elif x.startswith(str('a:')):
                self.pira_rpi_gpio = float(value)
                #print("Pira reports Pi status pin value: " + str(self.pira_rpi_gpio))
            elif x.startswith(str('c:')):
                self.pira_command  = int(value)
                #print "Pira command : " + str(self.pira_command)
            elif x.startswith(str('e:')):
                self.pira_reset_cause  =  self.translate_reset_cause(int(value))
                self.boot.print_and_log('=======================================================')
                self.boot.print_and_log("!!!!!!!PIRA RESET CAUSE :          " +  str(self.pira_reset_cause) + "!!!!!!")
            elif x.startswith(str('m:')):
                self.pira_state  =  self.translate_state(int(value))
                #print("Pira state : " + str(self.pira_state))

        return True

    """
        t:<uint32_t> time - seconds in epoch format
        p:<uint32_t> power - safety on period
        s:<uint32_t> sleep - safety off period
        r:<uint32_t> reboot - reboot period duration
        w:<uint32_t> wakeup - period for next wakeup
        c:<uint32_t> command - TODO
    """

    def set_time(self, new_time_epoch):
        """Writes new time to pira"""
        data = "t:" + struct.pack('>L', int(new_time_epoch))
        self.ser.write(data+'\n')

    def set_on_time(self, time_seconds):
        """Writes new on period time to pira"""
        data = "p:" + struct.pack('>L', int(time_seconds))
        self.ser.write(data+'\n')

    def set_off_time(self, time_seconds):
        """Writes new off period time to pira"""
        data = "s:" + struct.pack('>L', int(time_seconds))
        self.ser.write(data+'\n')

    def set_reboot_time(self, time_seconds):
        """Writes new reboot time to pira"""
        data = "r:" + struct.pack('>L', int(time_seconds))
        self.ser.write(data+'\n')

    def set_wakeup_time(self, time_seconds):
        """Writes new wakeup time to pira"""
        data = "w:" + struct.pack('>L', int(time_seconds))
        self.ser.write(data+'\n')

    def send_command(self, command):    # TO DO
        """Sends command to pira"""
        data = "c:" + struct.pack('>L', int(command))
        self.ser.write(data+'\n')

    def close(self):
        """Close device."""
        pass

    def translate_reset_cause(self, reset_cause):
        """It translates numbered reset cause into reset cause in string"""
        if reset_cause == 0:
            return "POWER ON RESET"
        elif reset_cause == 1:
            return "EXTERNAL RESET"
        elif reset_cause == 2:
            return "SOFTWARE RESET"
        elif reset_cause == 3:
            return "WATCHDOG RESET"
        elif reset_cause == 4:
            return "FIREWALL RESET"
        elif reset_cause == 5:
            return "   OTHER RESET"
        elif reset_cause == 6:
            return " STANDBY RESET"

    def translate_state(self, state):
        """It translates numbered state into state in string"""
        if state == 0:
            return "IDLE"
        elif state == 1:
            return "WAIT_STATUS_ON"
        elif state == 2:
            return "WAKEUP"
        elif state == 3:
            return "REBOOT_DETECTION"
