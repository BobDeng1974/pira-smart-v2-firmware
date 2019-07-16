# pira-smart-2-firmware
FIrmware for PiRa smart v2 device based on Murata Lora + STM32L0

## Project development plan
This project is to implement firmware support for Pira Smart 2 device, hardware as designed at: https://github.com/IRNAS/pira-smart-2.0-hardware 

It is an upgrade of the previous project of Pira Smart as published at:
 * https://github.com/IRNAS/pira-smart-firmware - Firmware repository in Mbed, now transitioning to Arduino in PlatformIO
 * https://github.com/IRNAS/pira-smart-hardware - Hardware of the previous version

Two software components this solution must be compatible with:
 * https://github.com/IRNAS/pira-smart-software - RaspberryPi implementation of logic associated with Pira solution
 * https://github.com/IRNAS/pira-smart-app - BLE interface app
 
### Development workflow:
 1. Implement a board self-test code to validate hardware operation and different power consumption levels. This code is to be used in production for testing of each manufactured device.
 1. Implement the basic features by:
  1. Porting the pira-smart-firmware functionality and omitting any BLE related code - operation without
  1. Implementing automated testing from python tools to validate correct operation of wake-up/sleep mechanisms
  1. Implement BLE communication
  1. Implement Lora communication
