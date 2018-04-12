// Drive doorbell solenoids, provides switch inputs, URL and web page control of chimes
// Initially used with a LiLon NodeMCU 1.0 (based on ESP-12E module)
// Bob S. 12/7/17
//
// Chime sounds: https://www.1800doorbell.com/resources/door-chimes/doorbell-door-chime-sounds/
//
//  Wifi Setup
//  If no wifi connection or connection is lost, it goes into config mode and will time out, 
//  reboot and try again to reconnect (like after a power or wifi fail).
//  The config access point (should open automatically, or go to 192.168.4.1) can be started at 
//  any time by pressing and holding the blue button. Note: this will erase wifi connection info!
//
//  Blue Button LED:  Off: normal operation, On: in config mode
// 
//  Available endpoints: Doorbells: /front, /back, Individual Chimes: /low, /med, /hi
//  ISY URL: http://admin:admin@192.168.0.10

#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

// Local project files (see tabs)
#include "Vars_Constants.h"
#include "Tunes.h"

// Capture compile time info
const char compile_info[] = __DATE__ ", " __TIME__ " File:" __FILE__;

WiFiManager wifiManager;

void setup() {

  delay(3000);
  Serial.begin(115200);
  Serial.println("\nLove Shack Wifi Doorbell");

  #include "Init_IO.h"

  // Clean FS, for testing, make sure to run this then connect if any var names change, or it crashes!
  //Serial.println("Formatting FS...");
  //SPIFFS.format();
  //wifiManager.resetSettings();

  #include "ESP_Data.h"

  wifiManager.setConfigPortalTimeout(60);
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.autoConnect("Wifi_Doorbell_Config");
  
//  // Look for wifi, if timeout reset and try again. Button forces config mode access point
//  int timeout = 60;
//  Serial.print("Connecting to Wifi: ");
//  while ( WiFi.status() != WL_CONNECTED || digitalRead(CTRL_BUTTON) == LOW ) {
//    Serial.print(".");
//    timeout--;
//
//    // Blink the button led
//    digitalWrite(LED, !digitalRead(LED));
//
//    // Handle the button to force config access point
//    if ( digitalRead(CTRL_BUTTON) == LOW ) {
//      digitalWrite(LED, HIGH);
//      //WiFiManager wifiManager;
//      if (!wifiManager.startConfigPortal("Wifi_Doorbell_Config")) {
//        Serial.println("Failed to connect and hit timeout");
//        delay(3000);
//        //reset and try again
//        ESP.reset();
//        delay(5000);
//      }
//      digitalWrite(LED, LOW);
//    }
//    
//    if (timeout <= 0) {
//      Serial.printf("\r\nWiFi connect aborted!\r\n");
//      Serial.printf("\r\nFailed to connect and hit timeout, resetting\r\n");
//      delay(3000);
//      //reset and try again
//      ESP.reset();
//      delay(5000);
//    }
//    delay(500);
//  }
  
  if (WiFi.status() == WL_CONNECTED) {

    digitalWrite(LED, LOW);
    Serial.println("Wifi Connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // Read updated setup parameters from ESP's file system
    strcpy(isy_x10_dev, custom_isy_x10_dev.getValue());
    strcpy(isy_url, custom_isy_url.getValue());
  
    // Save the custom parameters to FS
    if (shouldSaveConfig) {
      eeprom_save();
    }

    Serial.printf("ISY X10 Dev: \t%s\n",isy_x10_dev);
    Serial.printf("ISY URL: \t\t%s\n",isy_url);

    #include "Web_Server.h"
    server.begin();
    Serial.println("Doorbell HTTP server started");  
  } else {
    Serial.println("Wifi NOT Connected, resetting!");
    ESP.reset();
    delay(5000);  }
}

void loop(void) {

  server.handleClient();

  unsigned long currentMillis = millis();

  // Check config button
  if ( digitalRead(CTRL_BUTTON) == LOW ) {
    WiFi.disconnect();
  }

  // Reset if not connected
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Resetting!");
    ESP.reset();
    delay(5000);  
  } else {
    digitalWrite(LED, LOW);    
  }
  
  // Check front doorbell button
  if (digitalRead(FRONT_DOOR) == LOW) {
    Serial.println("Front Door!"); 
    int len = sizeof (tune_1) / sizeof (tune_1[0]);
    activate("front", tune_1, len, isy_x10_dev, isy_url);
  }

  // Check back doorbell button
  if (digitalRead(BACK_DOOR) == LOW) {
    Serial.println("Back Door!");     
    int len = sizeof (tune_1) / sizeof (tune_1[0]);
    activate("back", tune_1, len, isy_x10_dev, isy_url);
  }

  // Blink the node led to indicate this loop is running
  if (currentMillis - node_led_prev_millis >= node_led_interval) {
    node_led_prev_millis = currentMillis;
    digitalWrite(NODE_LED, !digitalRead(NODE_LED));
  }
  
}



