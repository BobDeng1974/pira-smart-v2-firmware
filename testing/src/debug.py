"""
debug.py

It is a module that prints various debug information from Pira, RPi and other modules

"""

class Debug():

    def output(self, pira_ok, boot):
        print('=========================DEBUG=========================')
        if pira_ok:     # Report Pira BLE values
            print('Pira     : t - rtc time           : {}'.format(boot.get_time()))
            print('Pira     : o - overview value     : {} s'.format(boot.get_pira_on_timer_set()))
            print('Pira     : b - battery level      : ' + str(boot.get_voltage()) + ' V')
            print('Pira     : p - safety on period   : {} s'.format(boot.get_pira_on_timer()))
            print('Pira     : s - safety off period  : {} s'.format(boot.get_pira_sleep_timer()))
            print('Pira     : r - reboot period      : {} s'.format(boot.get_pira_reboot_timer()))
            print('Pira     : w - next wakeup        : {} s'.format(boot.get_pira_wakeup_timer()))
            print('Pira     : c - state              : {} '.format(self.translate_state(boot.get_pira_command())))
        else:
            print('Pira     : Not connected')
        
        print('=======================================================')

    def shutdown(self, modules):
        """Shutdown module"""
        pass

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
