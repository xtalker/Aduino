/***************************************************
  Adafruit MQTT Library ESP8266 Example

  Must use ESP8266 Arduino from:
    https://github.com/esp8266/Arduino

  Works great with Adafruit's Huzzah ESP board & Feather
  ----> https://www.adafruit.com/product/2471
  ----> https://www.adafruit.com/products/2821

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Tony DiCola for Adafruit Industries.
  MIT license, all text above must be included in any redistribution

  Hints:
  https://gitlab.mathux.org/Mathieu/Domotique/blob/master/WifiControlSensor/MQTT.ino
  
 ****************************************************/
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* WiFi Access Point *********************************/

//#define WLAN_SSID       "CHAOS-HOUSE"
//#define WLAN_PASS       "b0b5c0de2011"
#define WLAN_SSID       "6fusion"
#define WLAN_PASS       "working downtown is fun"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "xtalker"
#define AIO_KEY         "729f547743a972e776c7d69f39b9cb4a50a21d29"
#define AIO_FEED        "sonoffrelay"

Adafruit_MQTT_Client* mqtt = NULL;
Adafruit_MQTT_Subscribe* myFeed = NULL;

/*************************** Sketch Code ************************************/

void setup() {
  Serial.begin(115200);
  delay(10);

  Serial.println(F("Adafruit MQTT demo"));

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

  // Setup AIO connection here, replace params with those gathered from the user
  // Create an ESP8266 WiFiClient class and connect it to the MQTT server.
  mqtt = new Adafruit_MQTT_Client(new WiFiClient(), AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

  // Setup a feed for subscribing to changes.
  char* myFeedStr = AIO_USERNAME "/feeds/" AIO_FEED;
  myFeed = new Adafruit_MQTT_Subscribe(mqtt, myFeedStr);
  mqtt->subscribe(myFeed);
}

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).
  MQTT_connect();

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt->readSubscription(3000))) {
    if (subscription == myFeed) {
      Serial.print(F("Got: "));
      Serial.println((char *)myFeed->lastread);
    }
  }

  // Ping the server to keep the mqtt connection alive, NOT required if you are publishing
  if(! mqtt->ping()) {
    mqtt->disconnect();
  }
}

// Function to connect and reconnect as necessary to the MQTT server.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt->connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt->connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt->connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt->disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
