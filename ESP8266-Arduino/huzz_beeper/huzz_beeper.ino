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
  used to implement a REST beeper and LED flasher.
  
  See the README at: https://github.com/marcoschwartz/aREST
*/

#define REDLED     0
#define BLUELED    15
#define BEEPER     4
#define BEEPTIME   100


// Create aREST instance
aREST rest = aREST();

// WiFi parameters
const char* ssid = "MY-HOUSE";
const char* password = "MY-CODE";
//const char* ssid = "6fusion";
//const char* password = "W4C1TG00D";

// The port to listen for incoming TCP connections
#define LISTEN_PORT 80

// Create an instance of the server
WiFiServer server(LISTEN_PORT);
int status = WL_IDLE_STATUS;

// Variables to be exposed to the API
//int temperature;
//int humidity;

// Declare functions
int ledControl(String command);
int beepControl(String command);
int ledFlash(int led);

void setup(void)
{
  // Start Serial
  Serial.begin(115200);

  pinMode(REDLED, OUTPUT);
  pinMode(BLUELED, OUTPUT);
  pinMode(BEEPER, OUTPUT);
  digitalWrite(REDLED,0);  
  digitalWrite(BLUELED,0);  
  digitalWrite(BEEPER,0); 
    
  // Init variables and expose them to REST API
//  temperature = 24;
//  humidity = 40;
//  rest.variable((char *)"temperature",&temperature);
//  rest.variable((char *)"humidity",&humidity);

  // Status led (may need to be something other than port 0)
  //rest.set_status_led(BLUELED);

  // Functions to be exposed by the API
  rest.function((char *)"led",ledControl);
  rest.function((char *)"beep",beepControl);

  // Give name and ID to device
  rest.set_id((char *)"1");
  rest.set_name((char *)"Huzz-Buzzer");

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    //delay(500);
    //digitalWrite(BLUELED,1); 
    ledFlash(BLUELED); 
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  digitalWrite(BLUELED,1);  

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
}

void loop() {

  if (WiFi.status() == WL_CONNECTION_LOST) {
    Serial.println("WiFi connection lost, please reboot");
    digitalWrite(BLUELED,1);  
    // don't continue, must reset to reconnect:
    while (true);
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

// Custom function accessible by the API
int ledControl(String command) {

  // Toggle the state of the led
  //int state = command.toInt();

  ledFlash(BLUELED);

  digitalWrite(REDLED,!digitalRead(REDLED));
 
  return 1;
}

// Activate the beeper
int beepControl(String command) {

  // Get beep count param
  int cnt = command.toInt();
  if (cnt == 0) cnt = 1;
  if (cnt > 10) cnt = 10;

  ledFlash(BLUELED);
  
  for (int b = 1; b <= cnt; b++) {
    digitalWrite(BEEPER,1);
    delay(BEEPTIME);
    digitalWrite(BEEPER,0);  
    delay(BEEPTIME);
  }
  return cnt;
}

// Momentarily toggle a led a few times
int ledFlash(int led) {
  int flashCnt = 8;
  for (int i = 1; i <= flashCnt; i++) {
    digitalWrite(led,!digitalRead(led));
    delay(50);
    digitalWrite(led,!digitalRead(led));
    delay(50);
  }
}

