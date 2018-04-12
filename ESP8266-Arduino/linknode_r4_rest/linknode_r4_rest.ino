// link_node_r4_rest.ino
// REST server for LinkNode R4 ESP8266 based relay board
// Relays are numbered 2,3,4,5
// REST commands:  curl "http://192.168.0.52/relay2?params=1"  (turn on relay 2)
//                 curl "http://192.168.0.52/allOff" (turn all 4 relays off)
// Wifi disconnect causes soft reset that should reconnect wifi.  Wifi net and password are hard coded.
// May 23, 2017 - Bob S.

// Import required libraries
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
#include <aREST.h>

/*
  This a simple example of the aREST Library for the ESP8266 WiFi chip
  used to control a LinkNode-R4 relay board.
  
  See the README at: https://github.com/marcoschwartz/aREST
*/

#define RELAY_2     16
#define RELAY_3     14
#define RELAY_4     12
#define RELAY_5     13

// Create aREST instance
aREST rest = aREST();

// WiFi parameters
//const char* ssid = "MY-HOUSE";
//const char* password = "MY-CODE";
const char* ssid = "6fusion";
const char* password = "working downtown is fun";

// The port to listen for incoming TCP connections
#define LISTEN_PORT 80

// Create an instance of the server
WiFiServer server(LISTEN_PORT);
int status = WL_IDLE_STATUS;

// Declare functions
int setRelay(int relay, int state);
int relay2Control(String command);
int relay3Control(String command);
int relay4Control(String command);
int relay5Control(String command);
int allOff();

void setup(void)
{
  // Start Serial
  Serial.begin(9600);

  pinMode(RELAY_2, OUTPUT);
  pinMode(RELAY_3, OUTPUT);
  pinMode(RELAY_4, OUTPUT);
  pinMode(RELAY_5, OUTPUT);
  digitalWrite(RELAY_2,0);  
  digitalWrite(RELAY_3,0);  
  digitalWrite(RELAY_4,0);  
  digitalWrite(RELAY_5,0);  

    
  // Functions to be exposed by the API
  rest.function((char *)"relay2",relay2Control);
  rest.function((char *)"relay3",relay3Control);
  rest.function((char *)"relay4",relay4Control);
  rest.function((char *)"relay5",relay5Control);
  rest.function((char *)"allOff",allOff);

  // Give name and ID to device
  rest.set_id((char *)"01");
  rest.set_name((char *)"LinkNode-R4-Rest");

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    // Delay required or it crashes!
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
}

void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost, restarting in 10 secs.");
    delay(10);
    ESP.reset();
  }
  
  // Handle REST calls
  WiFiClient client = server.available();
  
  if (!client) {
    return;
  }
  
  while(!client.available()){
    delay(1);
  }

  rest.handle(client);
}

int setRelay(int relay, String state) {
 
  // Set the relay state
  int s = state.toInt();
  
  Serial.print("setRelay: ");
  Serial.print(relay);
  Serial.print(" to: ");
  Serial.println(state);

  if (s != 0) {
     s = 1;
  }

  digitalWrite(relay, s);
  return s;
}

// REST functions for each relay
int relay2Control(String state) {
  setRelay(RELAY_2, state);  
  return 1;
}

int relay3Control(String state) {
  setRelay(RELAY_3, state);  
  return 1;
}

int relay4Control(String state) {
  setRelay(RELAY_4, state);  
  return 1;
}

int relay5Control(String state) {
  setRelay(RELAY_5, state);  
  return 1;
}

// Function to turn all relays off
int allOff(String x) {
    digitalWrite(RELAY_2, 0);  
    digitalWrite(RELAY_3, 0);  
    digitalWrite(RELAY_4, 0);  
    digitalWrite(RELAY_5, 0);
    Serial.println("allOff: All relays off!");

    return 1;  
}

