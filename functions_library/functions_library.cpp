
// include this library's description file.
#include "functions_library.h"


// description files for other libraries used. (if any)
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ThingSpeak.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>

/**
 This function clears all data on EEPROM device. All values will be set to 0 on all adresses. 
*/
void EEPROM_clear(void)
{
  Serial.println(" Warning: Values on EEPROM storage are cleared now. Do not shutdown device! ");
  for (int i = 0 ; i < EEPROM.length() ; i++) 
  {
    EEPROM.write(i,0);
  }
  
}

/**
 This function writes one value to next free adress on EEPROM memory. If EEPROM is full, error message will appear and you have to connect to wifi to send all data and clear storage if needed.
*/
void EEPROM_write(int value_to_write)
{
  int adress = 0;
  int read_value = 0;

  do {
    EEPROM.get(adress*sizeof(int), read_value);
    adress++;
  } while((read_value > 0) && (adress < EEPROM.length()));
  
  adress = (adress - 1) * sizeof(int);
  if (adress < EEPROM.length())
  {
    EEPROM.put( adress, value_to_write );
    EEPROM.commit();
  }
  else
  {
    Serial.println(" Error, EEPROM storage is full, firstly connect to wifi and device will send data to cloud. After that storage will autoclear. ");
  }
}



/** 
This function is used to establish connection with wifi. 
It will try to reconnect for 60 seconds.
If reconnect succeeds, function will automatically send all used data on EEPROM storage to cloud and automatically clear storage if needed.
If 60 seconds would be up, function will call EEPROM_write function to save data in EEPROM on next free adress.
*/

/*
void Communication_Reestablish(long current_val, unsigned long Channel_Number, const char* Write_API_Key, const char* network_ssid, const char* network_password)
{
  unsigned long start_time = millis();
  Serial.print("trying to reconnect ");

  while (WiFi.status() != WL_CONNECTED && millis() - start_time < 60000) 
  {
    WiFi.begin(network_ssid, network_password);
    WiFi.waitForConnectResult();
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println();
    digitalWrite(D4, LOW);  
    Serial.println("Connection established! Sending data to cloud.");
    ThingSpeak.writeField(Channel_Number, 1, current_val, Write_API_Key);
    EEPROM_send(Channel_Number, Write_API_Key);
  }
  else
  {
    Serial.println();
    Serial.println("Connection timeout. Saving data to EEPROM.");
    EEPROM_write(current_val);
  }
}
*/

/** 
This function is used for OTA configuring during setup. 
*/
void OTA_configure()
{
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
} 