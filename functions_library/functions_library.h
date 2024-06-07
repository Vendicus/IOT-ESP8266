/**
functions_library.h library to use for my task and organize everything in OOP way.
All right reserved.
*/

// ensure this library description is only included once
#ifndef FUNCTIONS_LIBRARY_H
#define FUNCTIONS_LIBRARY_H

#include <Arduino.h>

// library interface description

/**
 This function writes one value to next free adress on EEPROM memory. If EEPROM is full, error message will appear and you have to connect to wifi to send all data and clear storage if needed.
*/
void EEPROM_write(int value_to_write);


/**
 This function clears all data on EEPROM device. All values will be set to 0 on all adresses. 
*/
void EEPROM_clear(void);  

/** 
This function is used to establish connection with wifi. 
It will try to reconnect for 60 seconds.
If reconnect succeds, function will automatically send all used data on EEPROM storage to cloud and automatically clear storage if needed.
If 60 seconds would be up, function will call EEPROM_write function to save data in EEPROM on next free adress.
*/
//void Communication_Reestablish(long current_val, unsigned long Channel_Number, const char* Write_API_Key, const char* network_ssid, const char* network_password);

/** 
This function is used for OTA configuring during setup. 
*/
void OTA_configure(void);

#endif