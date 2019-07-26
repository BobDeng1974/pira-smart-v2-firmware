from time import sleep
import json
from sys import exit
import logging
import RPi.GPIO as gpio
import devices

class Test():
    """Used for testing of functionality of PIRA"""

    def __init__(self, boot, test_data_index, test_routine, num_rep):
        logging.info('Testing started with paramters:')
        logging.info('- test_data_index: ' + str(test_data_index))
        logging.info('- test_routine: ' + str(test_routine))
        logging.info('- num_rep: ' + str(num_rep))

        self.boot = boot
        self.test_data_index = test_data_index
        self.test_routine = test_routine
        self.num_rep = num_rep
        self.test_data = self.prepare_test_data(test_data_index)

        self.start_test()

    def read_json_file(self):
        """Functions reads data from json file and returns its content"""

        try:
            with open('values.json') as f:
              data = json.load(f)
        except Exception:
            exit("Can not find json file, program stopped!")
        else:
            print("File found!")
            return data

    def prepare_test_data(self, test_data_index):
        """Function prepares json data for testing framework"""
        data = self.read_json_file()

        test_data  = [0 for x in range(4)] # prepare empty list with 4 elements

        test_data[0] = data['values'][test_data_index]["pira_on_time"]
        test_data[1] = data['values'][test_data_index]["pira_off_time"]
        test_data[2] = data['values'][test_data_index]["pira_reboot_time"]
        test_data[3] = data['values'][test_data_index]["pira_wakeup_time"]

        return test_data

    def send_test_data(self):
        """Sends test data over uart to pira"""

        print("Sending new data!!!!!!!")

        self.boot.pirasmart.set_on_time(self.test_data[0])
        self.boot.pirasmart.set_off_time(self.test_data[1])
        self.boot.pirasmart.set_reboot_time(self.test_data[2])
        self.boot.pirasmart.set_wakeup_time(self.test_data[3])

    def read_power_pin(self):
        """Returns state of power pin"""
        return gpio.input(devices.GPIO_POWER_SUPPLY_PIN)

    def set_status_pin_high(self):
        """Sets status pin high"""
        gpio.output(devices.GPIO_PIRA_STATUS_PIN, gpio.HIGH)

    def set_status_pin_low(self):
        """Sets status pin low"""
        gpio.output(devices.GPIO_PIRA_STATUS_PIN, gpio.LOW)

    def wait_for_wait_status_on_state(self):
        """Function will block code, until we are in WAIT_STATUS_ON_STATE"""
        while True:
            pira_ok = self.boot.pirasmart.read()
            current_state = self.boot.debug.translate_state(self.boot.get_pira_state())
            if current_state == "WAIT_STATUS_ON":
                print("We are in correct state")
                break

    def verify_sent_data(self):
        """Checks returned values to see if they match with the ones sent"""
        pira_ok = self.boot.pirasmart.read()
        if self.test_data[0] == self.boot.get_pira_on_timer() and \
           self.test_data[1] == self.boot.get_pira_sleep_timer() and \
           self.test_data[2] == self.boot.get_pira_reboot_timer() and \
           self.test_data[3] == self.boot.get_pira_wakeup_timer():
               print("Data verified!")
               return True
        else:
            print("Data incorrect!")
            # TODO add logging 
            return False

    def start_test(self):
        """"""

        while True:
            self.wait_for_wait_status_on_state()
            self.send_test_data()
            while True:
                self.verify_sent_data()
                sleep(1)
        #if ok start monitoring picked test routine

