import logging
import datetime
from time import sleep

import RPi.GPIO as gpio
import devices
import pirasmartuart
import debug

class Boot():
    """Used to call all init processes at start of program"""
    def __init__(self):
        # Used for signaling is reset of Pira happened
        # It is set in pirasmart.read() if received
        # It is reset in test.start_test() when tested routine is exited and flag was set 
        self.reset_flag = False
        self.setup()

    def setup_gpio(self):
        """Initialize GPIO."""
        # Set numbering mode 
        gpio.setmode(gpio.BCM)
        gpio.setwarnings(False)
        # Set STATUS PIN
        gpio.setup(devices.GPIO_PIRA_STATUS_PIN, gpio.OUT, initial=gpio.LOW)

        # Set POWER PIN
        gpio.setup(devices.GPIO_POWER_SUPPLY_PIN, gpio.IN, pull_up_down=gpio.PUD_DOWN)
        self.print_and_log("Initialization of GPIO                               OK")

    def setup_uart(self):
        """Initialize uart driver."""
        self.pirasmart = pirasmartuart.PIRASMARTUART(devices.PIRASMART_UART, self)
        self.print_and_log("Initialization of UART                               OK")

    def setup_logging(self):
        """Initialize logging."""

        logging.basicConfig(filename='logging.log', format='%(asctime)s-%(levelname)s|%(message)s', level=logging.DEBUG)

    def setup_debug(self):
        """Initialize debug output"""
        self.debug = debug.Debug(self)
        self.print_and_log("Initialization of DEBUG                              OK")

    def setup_time(self):
        """Syncs time between raspberry and pira"""

        self.pira_ok = self.pirasmart.read()
        if self.pira_ok:
            rtc_time = self.get_time()
        else:
            rtc_time = datetime.datetime.now()

        system_time = datetime.datetime.now()

        # Sync time 
        if rtc_time > system_time:
            #write RTC to system
            #self.print_and_log("Writing RTC to system time")
            args = ['date', '-s', rtc_time.strftime("%Y-%m-%d %H:%M:%S")]
            subprocess.Popen(args)
            #note if ntp is running it will override this, meaning there is network time
        elif rtc_time < system_time:
            #write system_time to rtc
            #self.print_and_log("Writing system time to RTC")
            epoch_string = datetime.datetime.now().strftime('%s')
            self.pirasmart.set_time(epoch_string)

        else:
            #if equal no need to do anything
            pass
        self.print_and_log("Initialization of RTC TIME                           OK")

    def setup(self):
        """Initialize all setups"""

        self.setup_logging()
        self.print_and_log('=========================BOOT==========================')
        self.setup_gpio()
        self.setup_uart()
        self.setup_debug()
        self.setup_time()

    def print_and_log(self, message):
        """Function will print given message into console and into log file"""
        print(message)
        logging.info(message)

    def get_voltage(self):  # b variable
        """Get voltage """
        voltage = self.pirasmart.pira_voltage
        return voltage

    def get_temperature(self):
        """Get temeprature """
        temperature = None
        return temperature

    def get_time(self): # t variable
        """Get time """
        t_utc = datetime.datetime.utcfromtimestamp(self.pirasmart.pira_time)
        return t_utc

    def get_pira_on_timer(self):    # p variable
        """Get pira on timer """
        timer_pira = self.pirasmart.pira_on_timer_get
        return timer_pira

    def get_pira_overview(self):    # o variable
        """Get pira overview - status value """
        overview = self.pirasmart.pira_overview
        return overview

    def get_pira_sleep_timer(self): # s variable
        """Get pira sleep timer"""
        sleep_timer = self.pirasmart.pira_sleep
        return sleep_timer

    def get_pira_reboot_timer(self):    # r variable
        """Get pira reboot period duration"""
        reboot_timer = self.pirasmart.pira_reboot
        return reboot_timer

    def get_pira_wakeup_timer(self):    # w variable
        """Get pira next scheduled wakeup  """
        wakeup_timer = self.pirasmart.pira_next_wakeup_get
        return wakeup_timer

    def get_pira_command(self):    # c variable
        """Get pira command"""
        command = self.pirasmart.pira_command
        return command

    def get_pira_state(self):    # c variable
        """Get pira command"""
        state = self.pirasmart.pira_state
        return state
