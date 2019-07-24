from time import sleep 
import json

class Test():
    """Used for testing of functionality of PIRA"""
    
    def __init__(self, boot):
        self.boot = boot

    def read_json_file(self):
        """Functions reads data from json file and returns its content"""

        try: 
            with open('values.json') as f:
              data = json.load(f)
        except Exception: 
            print("Can not find file!") 
        else:
            print("File found!") 
            return data

    def prepare_new_values(self, data):
        """Function prepares json data for testing framework"""

        m = len(data)
        n = 4

        test_data  = [[0 for x in range(n)] for y in range(m)] 
        
        for i in range(len(data)):
            test_data[i][0] = data['values'][i]["pira_on_time"]
            test_data[i][1] = data['values'][i]["pira_off_time"]
            test_data[i][2] = data['values'][i]["pira_reboot_time"]
            test_data[i][3] = data['values'][i]["pira_wakeup_time"]

        return test_data    

    def start_test(self, num_rep=5):

        data = self.read_json_file()
        test_data = self.prepare_new_values(data)

        for i in range(len(test_data)): # Repeat for every test set
            self.send_test_data(test_data[i])
            sleep(10)
            #monitor_flow(num_rep)
                

        # Report that testing is done into log
    
    def send_test_data(self, test_data):
        """Sends test data over uart to pira""" 

        print("Sending new data!!!!!!!")

        self.boot.pirasmart.set_on_time(test_data[0])
        self.boot.pirasmart.set_off_time(test_data[1])
        self.boot.pirasmart.set_reboot_time(test_data[2])
        self.boot.pirasmart.set_wakeup_time(test_data[3])
        
        """
        pira_on_time = 30 
        pira_off_time = 20 
        pira_reboot_time = 30  
        pira_wakeup_time = 30

        if (pira_on_time is not None):
            print("PIRA BLE: Setting new safety on (p) value.")
            self.pirasmart.set_on_time(pira_on_time)
            sleep(0.1)
        if (pira_off_time is not None):
            print("PIRA BLE: Setting new safety off (s) value.")
            self.pirasmart.set_off_time(pira_off_time)
            sleep(0.1)
        if (pira_reboot_time is not None):
            print("PIRA BLE: Setting new reboot (r) value.")
            self.pirasmart.set_reboot_time(pira_reboot_time)
            sleep(0.1)
        if (pira_wakeup_time is not None):
            print("PIRA BLE: Setting new wakeup (w) value.")
            self.pirasmart.set_wakeup_time(pira_wakeup_time)
            sleep(0.1)
        """
