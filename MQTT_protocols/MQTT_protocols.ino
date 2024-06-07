/* libraries used in main program */
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <ArduinoOTA.h>
#include <WiFiClient.h> 

/* My library with EEPROM and OTA functions*/
#include <functions_library.h>

/* My library with MQTT protocols functions. */
/* constant and global variables in program */
const char* network_ssid = "Aksamit";                                   // Network SSID
const char* network_password = "akben4321!";                            // Network Password
const char* server = "mqtt3.thingspeak.com";

int status = WL_IDLE_STATUS; 
long lastPublishMillis = 0;
long lastPublishMillis2 = 0;
int connectionDelay = 1;
int updateInterval = 30;                                                // interval in RTOS in seconds
int sensor_val;

// Ensure that the credentials here allow you to publish and subscribe to the ThingSpeak channel.
#define channelID 2567083
const char mqttUserName[] = "GAEpBwEpLzwyEw8gMQ4YMjM"; 
const char clientID[] = "GAEpBwEpLzwyEw8gMQ4YMjM";
const char mqttPass[] = "Wa3MLlehCimTu+2gwjxzDORw";

// It is strongly recommended to use secure connections. However, certain hardware does not work with the WiFiClientSecure library.
// Comment out the following #define to use non-secure MQTT connections to ThingSpeak server. 
//#define USESECUREMQTT

const char* PROGMEM thingspeak_cert_thumbprint = "9780c25078532fc0fd03dae01bfd8c923fff9878";

#ifdef USESECUREMQTT
  #include <WiFiClientSecure.h>
  #define mqttPort 8883
  WiFiClientSecure client; 
#else
  #define mqttPort 1883
  WiFiClient client;
#endif
PubSubClient mqttClient( client );


void setup() {
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

  // Delay to allow serial monitor to come up.
  delay(3000);
  // Connect to Wi-Fi network.
  connectWifi();
  // Configure the MQTT client
  mqttClient.setServer( server, mqttPort ); 
  // Set the buffer to handle the returned JSON. NOTE: A buffer overflow of the message buffer will result in your callback not being invoked.
  mqttClient.setBufferSize( 2048 );
  // Use secure MQTT connections if defined.
  #ifdef USESECUREMQTT
    // Handle functionality differences of WiFiClientSecure library for different boards.
    client.setFingerprint(thingspeak_cert_thumbprint);
  #endif
  OTA_configure();
}

void loop() {
  ArduinoOTA.handle(); 

  // Read random values from 0 to 1024 (like typical analog sensors read) and store in val variable 
  sensor_val = random(1024);    

  // Reconnect to WiFi if it gets disconnected.
  if (WiFi.status() != WL_CONNECTED) {
      connectWifi();
  }

  // Connect if MQTT client is not connected and resubscribe to channel updates.
  if (!mqttClient.connected()) {
     mqttConnect(); 
  }
  
  // check wifi connection once more 
  if (WiFi.status() == WL_CONNECTED && mqttClient.connected()) 
  {
    // LED active (connection works)
    digitalWrite(D4, LOW);   

    // checking if there is some data to send and if it is, sending from EEPROM. 
    EEPROM_send(channelID);     

    // Update ThingSpeak channel periodically. The update results in the message to the subscriber.
    if ( abs(long(millis()) - lastPublishMillis) > updateInterval*1000) 
    {
      mqttPublish( channelID, (String("field1=")+String(sensor_val)) );
      lastPublishMillis = millis();
    }   

    // Call the loop to maintain connection to the server.
    mqttClient.loop(); 
  }
  else
  { // LED disactive (connection lost)
    digitalWrite(D4, HIGH);                                            
    Serial.println("************!**********");
    Serial.println("Error! connection lost.");
    Serial.println("************!**********");

    if ( abs(long(millis()) - lastPublishMillis2) > updateInterval*1000) 
    {
      EEPROM_write(sensor_val);
      lastPublishMillis2 = millis();
    }
  }  
}







////////////////////////////// Core Functions for MQTT ////////////////////////////////
/*Functions which cannot be in other libraries due to cofniguration with macros problems and 
*/
// Publish messages to a ThingSpeak channel.
void mqttPublish(long pubChannelID, String message) 
{
  String topicString ="channels/" + String( pubChannelID ) + "/publish";
  if (mqttClient.publish( topicString.c_str(), message.c_str()))
  {
    Serial.println(" Data sent succesfully! ");
  }
  else
  {
    Serial.println(" Error during sending! Saving Data on EEPROM. ");
    EEPROM_write(sensor_val);
  }
}

void connectWifi()
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

bool mqttConnect() {
  // Loop until connected or tried and doesn't work still for few attempts.
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

/*
 This function sends all data from EEPROM storage on cloud. The interval between next send is set to 15 s due to ThingSpeak free use limits.
 If there is no data to send, function will inform us about it. If there is something to send, function will inform us too and will do automatic clear after succesful send.
*/
void EEPROM_send(long pubChannelID)
{
  int adress = 0;
  int read_value = 1;

  while((read_value > 0) && (adress < EEPROM.length()))
  {
    EEPROM.get(adress*sizeof(int), read_value);
    if (read_value == 0){}
    else
    {
      Serial.println(" Sending value from EEPROM adress " + String(adress*sizeof(int)) + " with value " + String(read_value) + " on Cloud." );
      mqttPublish( pubChannelID, (String("field1=")+String(read_value)));
    }

    adress++;
  } 

  if ((read_value == 0) && (adress == 1 ))
  {}
  else
  {
    Serial.println(" All data sent. Clearing data storage ...");
    EEPROM_clear();
    Serial.println(" Done. ");
  } 
}
