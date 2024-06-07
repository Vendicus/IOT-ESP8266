// include this library's description file.
#include "MQTT_library.h"


// description files for other libraries used. (if any)
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ThingSpeak.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <functions_library.h>
#include <PubSubClient.h>


#define channelID ***

#ifdef USESECUREMQTT
  #include <WiFiClientSecure.h>
  #define mqttPort 8883
  WiFiClientSecure client; 
#else
  #define mqttPort 1883
#endif

PubSubClient* mqttClient = nullptr; 

void setupMQTTClient(WiFiClient& client) {
    Wifi client;
    mqttClient = new PubSubClient(client);
}

/**
 This function is used to connect to wifi during setup.
 */
void connectWifi( const char* network_ssid, const char* network_password, int connectionDelay)
{
  Serial.print( "Connecting to Wi-Fi..." );
  WiFi.begin(network_ssid, network_password);
  WiFi.waitForConnectResult();
  delay( connectionDelay*1000 );
  Serial.print( WiFi.status() ); 

  if (WiFi.status() == WL_CONNECTED) 
  {
    Serial.println( "Connected to Wi-Fi." );
  }
  else
  {
    Serial.println( "Error, cannot connect to wifi, check SSID and password." );
  }
}


/**
 This function is used for public messages on ThingSpeak Channel.
 */
void mqttPublish(long pubChannelID, String message) 
{
  String topicString ="channels/" + String( pubChannelID ) + "/publish";
  mqttClient.publish( topicString.c_str(), message.c_str());
  /*
     {
    Serial.println(" Data sent succesfully! ");
  }
  else
  {
    Serial.println(" Error during sending! Saving Data on EEPROM. ");
    EEPROM_write(sensor_val);
  }
  */
}


/**
 This function is used to check MQTT connection and will try to connect few times before return.
 */
bool mqttConnect(const char* clientID, const char* mqttUserName, const char* mqttPass, const char* server, int connectionDelay) 
{
  // Loop until connected or doesn't work still for few attempts.
  // set attempts to 3.
  int i = 0;
  while ( !mqttClient.connected() && ( i < 3) )
  {
    // Connect to the MQTT broker.
    if ( mqttClient.connect( clientID, mqttUserName, mqttPass ) ) {
      Serial.println( "MQTT to " + String(server) + " at port " + String(mqttPort) + "succesful." );
      return true;
    } else {
      Serial.print( " MQTT connection failed, rc = " );
      // See https://pubsubclient.knolleary.net/api.html#state for the failure code explanation.
      Serial.print( mqttClient.state() );
      Serial.println( " Will try again in a few seconds" );
      delay( connectionDelay*1000 );
    }

    i++;
  }
  Serial.println( " Cannot establish MQTT connection several times, check rc for explanation. " );
  return false;
}