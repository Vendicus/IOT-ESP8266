/* libraries included */
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ThingSpeak.h>
#include <ArduinoOTA.h>
#include <WiFiClientSecure.h> 

/* my library with all my functions*/
#include <functions_library.h>

/* constant and global variables in program */
const char* network_ssid = "*********";                                   // Network SSID
const char* network_password = "********";                            // Network Password
                                                   
long val;                                                               // random value for data simulation declaration
int http_status;                                                        // status for sending data on Cloud, if 200, success.

WiFiClientSecure client;                                                // object definition is to be client (our hardware is client for server ThingSpeak)

unsigned long Channel_Number = *;                                 // Channel Number in ThingSpeak (Without Brackets)
const char * Write_API_Key = "*********";                        // Write API Key of ThingSpeak Cloud

void setup()
{
  /* hardware config */
  Serial.begin(9600);                                                   // baudrate of your controller, for esp8266 it is 9600 bps in LUA, affter firmware update it will be 115200.
  delay(10);                                                            // delay to get hardware run normally.
  EEPROM.begin(512);                                                    // EEPROM initlatization - 512 bytes.

  pinMode(D4, OUTPUT);                                                  // GPIO 2 (D4) used in esp8266 as LED diode.
  pinMode(A0, INPUT);
  
  /* 
   if analog input pin 0 is unconnected, random analog
   noise will cause the call to randomSeed() to generate
   different seed numbers each time the sketch runs.
   randomSeed() will then shuffle the random function. 
   */
  randomSeed(analogRead(A0));

  /* Network and cloud config */
  WiFi.begin(network_ssid, network_password);
  WiFi.waitForConnectResult();
  delay(100);
  OTA_configure();                                                      // function in which you can find all OTA config. Find 'functions_library.cpp' for more info.
  ThingSpeak.begin(client);
}

void loop()
{
  ArduinoOTA.handle();                                                

  /* randomize data */
  val = random(1024);                                                   // Read random values from 0 to 1024 (like typical analog sensors read) and store in val variable

  /* Print on serial Monitor to see in console data flow */
  Serial.println("Data: " + String(val));                              

  /* check wifi connection */
  if (WiFi.status() == WL_CONNECTED) 
  {
    digitalWrite(D4, LOW);                                              // LED active (connection works)

    /* checking if there is some data to send and if it is, sending from EEPROM. */
    EEPROM_send(Channel_Number, Write_API_Key);

    /* writing value to cloud */
    http_status = ThingSpeak.writeField(Channel_Number, 1, val, Write_API_Key);    // Update in ThingSpeak

    /* additional checking of sent data */
    if (http_status == 200)
    {
      Serial.println("Data sent succesfully. ");
    }
    else 
    {
      Serial.println("Error during sending, check wifi settings and transmission quality. Writing value to EEPROM storage. ");
      EEPROM_write(val);
    }

    /* time interval */
    delay(60000);                                                       // ms - time units, used as time interval between reading new value from sensor. As a free option usage we have option of 15 s as our minimum interval for thingspeak. I used 1 min update to not make unnecessary junk-data on chart.
  } 
  else 
  {
    digitalWrite(D4, HIGH);                                             // LED disactive (connection lost)
    Serial.println("************!**********");
    Serial.println("Error! connection lost.");
    Serial.println("************!**********");
    
    Communication_Reestablish(val, Channel_Number, Write_API_Key, network_ssid, network_password);
  }
}
