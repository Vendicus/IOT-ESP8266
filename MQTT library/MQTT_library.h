/**
 *Library for MQTT protocol communication created for project.
*/

// ensure this library description is only included once.
#ifndef MQTT_LIBRARY_H
#define MQTT_LIBRARY_H

#include <Arduino.h>
#include <PubSubClient.h>

// library interface description


#ifdef USESECUREMQTT
  #include <WiFiClientSecure.h>
  #define mqttPort 8883
#else
  #define mqttPort 1883
 
#endif
#define channelID **

extern PubSubClient* mqttClient;

/**
 This function is used for setup client for PubSub.
 */
void setupMQTTClient(WiFiClient& client);

/**
 This function is used for public messages on ThingSpeak Channell.
 */
void mqttPublish(long pubChannelID, String message);

/**
 This function is used to connect to wifi during setup. 
*/
void connectWifi( const char* network_ssid, const char* network_password, int connectionDelay);

/**
 This function is used to check MQTT connection and will try to connect few times before return.
 */
bool mqttConnect(const char* clientID, const char* mqttUserName, const char* mqttPass, const char* server, int connectionDelay);

#endif