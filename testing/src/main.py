from boot import Boot
from time import sleep 
from testing import Test

import debug as Debug 
def main():
    # Intialize gpios, uart, logging, debug
    boot = Boot()
    boot.setup() 
    test = Test(boot)
    #test.start_test()

    data = test.read_json_file()
    test_data = test.prepare_new_values(data)

    while True:
        #for i in range(len(test_data)): # Repeat for every test set
            #test.send_test_data(test_data[i])

            #for j in range(10): # Repeat for every test set
                # Get latest values from pira smart
        pira_ok = boot.pirasmart.read()
        boot.debug.output(pira_ok, boot)
        sleep(1)

main()
