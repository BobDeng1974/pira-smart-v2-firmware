#include <Arduino.h>
#include "RN487x_BLE.h"
//#include "STM32L0.h"            
#include <string.h>
#include <Stream.h>
//#include "TimerMillis.h"
#include "app_config.h"

/* Analog pin that temp sensor is connected to */
#define TEMP_SENSOR A0
/* Analog pin that pot is connected to */
#define POT_SENSOR A2
/* Maximuim number of milliseconds to wait for USB serial to get ready on boot */
#define SERIAL_TIMEOUT_MS  5000

const char* myDeviceName = "PiraSmart";  // Custom Device name
const char* myPrivateServiceUUID = "a5e767183f5d4128bb816910d88abd02"; // Custom private service UUID

// Light Sensor
const char* PIRA_STATUS_CHARACTERISTIC_UUID = "a66c5e33c0b442dd8b82239b871bec47";  // custom characteristic GATT
const uint8_t piraStatusLen = 2;  // data length (in bytes)
const uint16_t statusHandle = 0x72;
char statusPayload[piraStatusLen*2 + 1];

const char* readUuid = "7d7b5e3810004f65aa65a3697905d95e";  // custom characteristic GATT
const uint8_t readLen = 2;  // data length (in bytes)
const uint16_t readHandle = 0x75;
const char* readPayload;


// Light Sensor
const char* lightCharacteristicUUID = "BF3FBD80063F11E59E690002A5D5C501";  // custom characteristic GATT
const uint8_t lightCharacteristicLen = 2;  // data length (in bytes)
const uint16_t lightHandle = 0x72;
char lightPayload[lightCharacteristicLen*2 + 1];

// Pot
const char* potCharacteristicUUID = "BF3FBD80063F11E59E690002A5D5C502";  // custom characteristic GATT
const uint8_t potCharacteristicLen = 2;  // data length (in bytes)
const uint16_t potHandle = 0x75;
char potPayload[potCharacteristicLen*2 + 1];

// Switch/LED
const char* switchCharacteristicUUID = "BF3FBD80063F11E59E690002A5D5C503";  // custom characteristic GATT
const uint8_t switchCharacteristicLen = 20;  // data length (in bytes)
const uint16_t switchHandle = 0x78;
char switchPayload[switchCharacteristicLen*2 + 1];
const char* ledPayload;

// Temp
const char* temperatureCharacteristicUUID = "BF3FBD80063F11E59E690002A5D5C504";  // custom characteristic GATT
const uint8_t temperatureCharacteristicLen = 2;  // data length (in bytes)
const uint16_t temperatureHandle = 0x7A;
char temperaturePayload[temperatureCharacteristicLen*2 + 1];

// Battery
const char* batteryCharacteristicUUID = "BF3FBD80063F11E59E690002A5D5C505";  // custom characteristic GATT
const uint8_t batteryCharacteristicLen = 2;  // data length (in bytes)
const uint16_t batteryHandle = 0x7D;
char batteryPayload[batteryCharacteristicLen*2 + 1];

/*TimerMillis periodicTimer;

void periodicCallback(void)
{
    debugSerial.println("TIMER!!!!");
}
*/
void setup()
{
  
  debugSerial.begin(115200);
  // Wait for PC to connect, give up after SERIAL_TIMEOUT_MS
  while ((!debugSerial) && (millis() < SERIAL_TIMEOUT_MS));
  debugSerial.print("REASON FOR RESET: ");
//    debugSerial.println(STM32L0.resetCause());

  // Set the optional debug stream
  rn487xBle.setDiag(debugSerial);
  // Initialize the BLE hardware with our sleep and wakeup pins
  rn487xBle.hwInit();
  // Open the communication pipe with the BLE module
  bleSerial.begin(rn487xBle.getDefaultBaudRate());
  // Finalize the init. process
  if(rn487xBle.enterCommandMode())
        debugSerial.println("Uspelo");
else
        debugSerial.println("Ni Uspelo");
    while(1)
    {
     if (rn487xBle.swInit())
      {
        debugSerial.println("Init. procedure done!");
        break;
      }
      else
      {
        debugSerial.println("Init. procedure failed!");
        delay(500);
      }

    }
   
  // Fist, enter into command mode
  rn487xBle.enterCommandMode();
  // Stop advertising before starting the demo
  rn487xBle.stopAdvertising();
  rn487xBle.clearPermanentAdvertising();
  rn487xBle.clearPermanentBeacon();
  rn487xBle.clearImmediateAdvertising();
  rn487xBle.clearImmediateBeacon();
  rn487xBle.clearAllServices();
  // Set the serialized device name
  rn487xBle.setSerializedName(myDeviceName);
  rn487xBle.setSupportedFeatures(0x4000); // Set to no prompt (no "CMD>")
  rn487xBle.setDefaultServices(DEVICE_INFO_SERVICE);
  // Set the advertising output power (range: min = 5, max = 0)
  rn487xBle.setAdvPower(0);
  rn487xBle.reboot();
  rn487xBle.enterCommandMode();  
  rn487xBle.clearAllServices();
  // Set a private service ...
  rn487xBle.setServiceUUID(myPrivateServiceUUID);
  // which contains ...
  // ...a light sensor (unused) characteristic; readable and can perform notification, 2-octets size
  //rn487xBle.setCharactUUID(PIRA_STATUS_CHARACTERISTIC_UUID, READ_PROPERTY, piraStatusLen);
  //rn487xBle.setCharactUUID(readUuid, WRITE_PROPERTY, readLen);
// which contains ...
  // ...a light sensor (unused) characteristic; readable and can perform notification, 2-octets size
  rn487xBle.setCharactUUID(lightCharacteristicUUID, NOTIFY_PROPERTY, lightCharacteristicLen);
  // ...a pot characteristic; readable and can perform notification, 2-octets size
  rn487xBle.setCharactUUID(potCharacteristicUUID, NOTIFY_PROPERTY, potCharacteristicLen);
  // ...a LED/Switch characteristic; readable and can perform notification, 20-octets size
  rn487xBle.setCharactUUID(switchCharacteristicUUID, WRITE_PROPERTY , switchCharacteristicLen);
  // ...a temperature characteristic; readable and can perform notification, 2-octets size
  rn487xBle.setCharactUUID(temperatureCharacteristicUUID, NOTIFY_PROPERTY, temperatureCharacteristicLen);
  // ...a battery (unused) characteristic; readable and can perform notification, 2-octets size
  rn487xBle.setCharactUUID(batteryCharacteristicUUID, NOTIFY_PROPERTY, batteryCharacteristicLen);

  rn487xBle.startPermanentAdvertising(AD_TYPE_FLAGS, "06");
  
  // take into account the settings by issuing a reboot
  rn487xBle.reboot();
  rn487xBle.enterCommandMode();

  rn487xBle.startAdvertising();

  debugSerial.println("Fubarino Mini Activity Board as a Peripheral with private service");
  debugSerial.println("================================================");
  debugSerial.println("You can now establish a connection from the Microchip SmartDiscovery App");
  debugSerial.print("with the board: ") ; debugSerial.println(rn487xBle.getDeviceName());
        rn487xBle.displayServerServices();
        debugSerial.println(rn487xBle.getLastResponse());
        delay(500);
}
int i = 0;
void loop()
{

  // Check the connection status
    
    debugSerial.print("Before connection status");
    if(rn487xBle.getConnectionStatus() == true)
    {
        // We are connected to a peer
        debugSerial.print("Connected to a peer central ");
        debugSerial.println(rn487xBle.getLastResponse());

 
        sprintf(lightPayload, "%04X", 1);
        rn487xBle.writeLocalCharacteristic(lightHandle, lightPayload);
        sprintf(potPayload, "%04X", 2);
        rn487xBle.writeLocalCharacteristic(potHandle, potPayload);
        sprintf(temperaturePayload, "%04X", 3);
        rn487xBle.writeLocalCharacteristic(temperatureHandle, temperaturePayload);
        sprintf(batteryPayload, "%04X", 4);
        rn487xBle.writeLocalCharacteristic(batteryHandle, batteryPayload);

        i++;
        if(i == 16)
            i=0;

        if(rn487xBle.readLocalCharacteristic(switchHandle))
        {
            readPayload = rn487xBle.getLastResponse();
            if (readPayload  != NULL)
            {
                if (readPayload[0] != 'N') // != "N/A" response
                {
                    debugSerial.print("Reading ok: ");
                    debugSerial.println(readPayload);
                }
            }
        }
        // Delay inter connection polling - when connected, update every 1/4 second
        delay(500);
    }
    else if(rn487xBle.getConnectionStatus() == false)
    {
    // Not connected to a peer device
    debugSerial.println("Not connected to a peer device");

    // Delay inter connection polling - when not connected, check for new connections ever 1 second
    delay(1000);
    }
    else
    {
        debugSerial.println("Timeout occured");
        delay(1000);
    }

}
