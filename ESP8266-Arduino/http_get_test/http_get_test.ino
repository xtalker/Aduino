#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

HTTPClient http;

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "CHAOS-HOUSE"
#define WLAN_PASS       "b0b5c0de2011"
//#define WLAN_SSID       "6fusion"
//#define WLAN_PASS       "working downtown is fun"

char isy_url[50] = "http://admin:admin@192.168.0.10";

void setup() {
  Serial.begin(115200);
  delay(10);

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  char dev[4] = "D2";
  setISYx10(dev,1);
  
}

void loop() {
  // put your main code here, to run repeatedly:

}

// Set a ISY994i X10 device
void setISYx10(char* x10Device, int state){
  String url;

  if (state == 0) {
    // Set to off
    url = String(isy_url) + "/rest/X10/" + String(x10Device) + "/11";
  } else {
    // Set to on
    url = String(isy_url) + "/rest/X10/" + String(x10Device) + "/3";
  }

  Serial.print("setISYx10 URL: ");
  Serial.println(url);
//  url = "http://admin:admin@192.168.0.10/rest/X10/D2/3";
//  Serial.print("setISYx10 URL: ");
//  Serial.println(url);

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == 200) { 
    Serial.printf("setISYx10: Set ISY X10 device %s to %d OK\n",x10Device,state);
  } else {
    Serial.printf("ERROR: setISYx10: httpCode: %d\n", httpCode);
  }
  http.end();   //Close connection  
}

