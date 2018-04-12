// Test pubnub message subscribe with Sonoff ESP8266 board
//
// Used these for testing:
// curl "https://maker.ifttt.com/trigger/esp_channel/with/key/bRNxqHBtc5L5TnZzTC5cXQ"
// curl curl "http://pubsub.pubnub.com/publish/pub-c-83fee7a8-67d4-4461-a3e0-0e1f7234d7ef/sub-c-a7169fa6-7365-11e5-8f6d-0619f8945a4f/0/esp_channel/0/%7B%22Test%22%3A%22OFF%22%7D"

#include <ESP8266WiFi.h>
#define PubNub_BASE_CLIENT WiFiClient
#define PUBNUB_DEFINE_STRSPN_AND_STRNCASECMP
#include <PubNub.h>
 
//const char* ssid = "6fusion";
//const char* password = "working downtown is fun";
const char* ssid = "CHAOS-HOUSE";
const char* password = "b0b5c0de2011";

char* pubnubChannel = "esp_channel";
//char* pubKey = "pub-c-83fee7a8-67d4-4461-a3e0-0e1f7234d7ef";
char* pubKey = "";
char* subKey = "sub-c-a7169fa6-7365-11e5-8f6d-0619f8945a4f";
char mesgBuf[64]; 

int gpio13Led = 13;
WiFiServer server(80);
 
void setup() {
  Serial.begin(115200);
  delay(10);
 
  pinMode(gpio13Led, OUTPUT);
  digitalWrite(gpio13Led, HIGH);  // HIGH = led off
   
  // Connect to WiFi network
  Serial.print("\nConnecting to ");
  Serial.println(ssid);
   
  WiFi.begin(ssid, password);
   
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
   
  // Start the server
  server.begin();
  Serial.println("Server started");
 
  // Print the IP address
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  PubNub.begin(pubKey, subKey);
}
 
void loop() {

  //Serial.println("waiting for a message (subscribe)");

  // Wait for a message
  PubSubClient *sclient = PubNub.subscribe(pubnubChannel,3);
  if (!sclient) {
      Serial.println("subscription error");
      delay(1000);
      return;
  }
  size_t buflen = 0;
  
  while (sclient->wait_for_data(3)) {
    mesgBuf[buflen++] = sclient->read();
    Serial.println("B");
  }
  
  sclient->stop();

  Serial.print("\n  msgBuf: ");
  Serial.println(mesgBuf);

  if (String(mesgBuf).indexOf("ON") > 0) {
     setLed(1);
  } else if (String(mesgBuf).indexOf("OFF") > 0) {
     setLed(0);
  }

//  // History shows the last message, how to clear this when read?
//  WiFiClient *client;
//  Serial.println("retrieving message history");
//  // Note: Use of the history API requires the Storage & Playback add-on to be enabled
//  client = PubNub.history(pubnubChannel,1);
//  if (!client) {
//    Serial.println("history error");
//    delay(1000);
//    return;
//  }
//  while (client->connected()) {
//    while (client->connected() && !client->available()) ; // wait
//    char c = client->read();
//    Serial.print(c);
//  }
//  client->stop();
//  Serial.println();
//
//  delay(3000);
}

// Set the led
void setLed(int state) {
  if (state == 1) {
    digitalWrite(gpio13Led, LOW);
    Serial.println("setLed: On");
  } else {
    digitalWrite(gpio13Led, HIGH);
    Serial.println("setLed: Off");
  } 
  delay(1000);
}

