from time import sleep

import logging
from boot import Boot
from testing import Test
import debug as Debug

def main():
    # Intialize gpios, uart, logging, debug
    boot = Boot()
    boot.setup()
    Test(boot, test_data_index=0, test_routine=4, num_rep=4)
    #Test(boot, test_data_index=0, test_routine=4, num_rep=3)
    while True:
        pira_ok = boot.pirasmart.read()
        boot.debug.output(pira_ok)
        sleep(1)
main()
