from time import sleep

import logging
from boot import Boot
from testing import Test
import debug as Debug

def main():
    # Intialize gpios, uart, logging, debug
    boot = Boot()

    Test(boot, test_data_index=4, test_routine=0, num_rep=10)
    Test(boot, test_data_index=4, test_routine=1, num_rep=10)
    Test(boot, test_data_index=4, test_routine=2, num_rep=10)
    Test(boot, test_data_index=4, test_routine=3, num_rep=10)
    Test(boot, test_data_index=4, test_routine=4, num_rep=10)

    Test(boot, test_data_index=2, test_routine=4, num_rep=200)

    boot.print_and_log('DONE DONE DONE DONE DONE DONE')
    """
    while True:
        pira_ok = boot.pirasmart.read()
        boot.debug.output(pira_ok)
        sleep(1)
    """
main()
