"""
debug.py

It is a module that prints various debug information from Pira, RPi and other modules

"""

class Debug():
    def __init__(self, boot):
        self.boot = boot

    def output(self, pira_ok):
        print('=========================DEBUG=========================')
        if pira_ok:     # Report Pira BLE values
            print('Pira     : t - rtc time           : {}'.format(self.boot.get_time()))
            print('Pira     : o - overview value     : {} s'.format(self.boot.get_pira_overview()))
            print('Pira     : b - battery level      : ' + str(self.boot.get_voltage()) + ' V')
            print('Pira     : p - safety on period   : {} s'.format(self.boot.get_pira_on_timer()))
            print('Pira     : s - safety off period  : {} s'.format(self.boot.get_pira_sleep_timer()))
            print('Pira     : r - reboot period      : {} s'.format(self.boot.get_pira_reboot_timer()))
            print('Pira     : w - next wakeup        : {} s'.format(self.boot.get_pira_wakeup_timer()))
            print('Pira     : m - state              : {} '.format(self.boot.get_pira_state()))
        else:
            print('Pira     : Not connected')

        print('=======================================================')

