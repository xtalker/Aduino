/*
 *  From:  http://m2mio.tumblr.com/post/30048662088/a-simple-example-arduino-mqtt-m2mio
 *  
 *  PubNub mqtt page: https://www.pubnub.com/blog/2014-07-08-mqtt-now-supported-by-pubnub/
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define MQTT_SERVER "q.m2m.io"
PubSubClient client;

const char* ssid     = "6fusion";
const char* password = "W4C1TG00D";
//const char* host = "www.adafruit.com";

void setup() {
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  int value = 0;

  delay(5000);
  ++value;

  //Serial.print("connecting to ");
  //Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
//  WiFiClient client;
//  const int httpPort = 80;
//  if (!client.connect(host, httpPort)) {
//    Serial.println("connection failed");
//    return;
//  }

  client = PubSubClient(MQTT_SERVER, 1883, callback);
}

void loop() { 

  if (!client.connected())
  {
      // clientID, username, MD5 encoded password
      client.connect("arduino-mqtt", "john@m2m.io", "00000000000000000000000000000");
      client.publish("io.m2m/arduino/lightsensor", "I'm alive!");
      client.subscribe("io.m2m/arduino/lightsensor");
  }

  // MQTT client loop processing
  client.loop();

  Serial.println();
  Serial.println("closing connection");
}


// handles message arrived on subscribed topic(s)
void callback(char* topic, byte* payload, unsigned int length) {

  int i = 0;

  //Serial.println("Message arrived:  topic: " + String(topic));
  //Serial.println("Length: " + String(length,DEC));
  
  // create character buffer with ending null terminator (string)
  for(i=0; i<length; i++) {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';
  
  String msgString = String(message_buff);
  
  //Serial.println("Payload: " + msgString);
  
  if (msgString.equals("{\"command\":{\"lightmode\": \"OFF\"}}")) {
    senseMode = MODE_OFF;
  } else if (msgString.equals("{\"command\":{\"lightmode\": \"ON\"}}")) {
    senseMode = MODE_ON;
  } else if (msgString.equals("{\"command\":{\"lightmode\": \"SENSE\"}}")) {
    senseMode = MODE_SENSE;
  }
}
