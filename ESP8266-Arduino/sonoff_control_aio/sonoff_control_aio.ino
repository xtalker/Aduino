// Sonoff wifi relay control.
//
//  -Set Wifi info via config mode access point if it can't connect. Access point name:
//       "Sonoff_AP" go to setup page at 192.168.4.1 (if your device doesn't present it)
//  -Sonoff button (short press) provides local relay control, led shows relay status
//  -Sonoff button (long press, > 10 secs) forces config mode access point setup (after reboot), 
//        led flashes to let you know it has changed to config mode
//  -Serves a simple "mobile friendly" web page to control the relay (ip address must be 
//        determined from router, name on my router: "ESP_<last 6 digits of MAC>")
//  -Optionally tracks relay state with an X10 device in the ISY994i home control system
//        (attempts this only if "ISY X10 Device" is set on the config page)
//  -Optionally subscribe to an MQTT feed on Adafruit.io. Pre-register to obtain username, key and feed name.
//        (attempts this only if "Adafruit IO params" are set on the config page)
//
// With thanks to: http://randomnerdtutorials.com/reprogram-sonoff-smart-switch-with-web-server/
// 7/25/17, Bob S.

//  Similar project:
//  https://github.com/dafoink/IoT/blob/master/WaterSensor.ino

#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// Capture compile time info
const char compile_info[] = __DATE__ ", " __TIME__ " File:" __FILE__;

HTTPClient http;
ESP8266WebServer server(80);

/************************* Adafruit.io Setup *********************************/
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "xtalker"
#define AIO_KEY         "729f547743a972e776c7d69f39b9cb4a50a21d29"

Adafruit_MQTT_Client* mqtt = NULL;
Adafruit_MQTT_Subscribe* myFeed = NULL;

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;  // or... use WiFiClientSecure for SSL

// Setup for saving extra config params from access point setup
// Define your default values here, if there are different values in config.json, they are overwritten.
char isy_x10_dev[4];    // Name of the Isy X10 device used to track the relay state (ie: 'A1')
char isy_url[50];       // Should look like: "http://<user>:<passwd>@<ISY_IP_Address>" (with your credentials and IP)
char device_name[34] = "My Sonoff Relay #1";  // Used for the generated web page title
char aio_username[12];  //Adafruit IO username
char aio_key[50];       //Adafruit IO key
char aio_feed[20];      //Adafruit IO feed

bool shouldSaveConfig = false;  // Flag for saving data
bool setupMQTTOk = true; // Flag to connect to MQTT

// Callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

// Init vars
String webPage = "";
//String isy_url = "http://admin:admin@192.168.0.10";
String deviceName = "Sonoff Relay";
String value = "";
char* subPath;
int gpio13Led = 13;
int gpio12Relay = 12;
int gpio0Button  = 0;
int relayState = 0;
int old_relayState = 0;
int buttonCnt = 0;
int pingCnt = 0;

WiFiManager wifiManager;

// The setup function runs once when you press reset or power the board
void setup() {
  delay(3000);
  Serial.begin(115200);     // Note Serial monitor doesn't work after Sonoff reset, not sure why.
  Serial.println("START!");

  // preparing GPIOs
  pinMode(gpio0Button, INPUT);
  pinMode(gpio13Led, OUTPUT);
  pinMode(gpio12Relay, OUTPUT);
  digitalWrite(gpio13Led, LOW);   // Led initially on, goes off when connected

  // Setup interrupt to handle button press without waiting for loop handler
  pinMode(gpio0Button, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(gpio0Button), buttonInterrupt, RISING);

  // Clean FS, for testing, make sure to run this then connect if any var names change, or it crashes!
  //Serial.println("Formatting FS...");
  //SPIFFS.format();

  // Read configuration from FS json
  //Serial.println("Mounting FS...");

  // Setup the ESP's file system to save config data
  eeprom_setup();

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_isy_x10_dev("x10Dev", "ISY X10 Device", isy_x10_dev, 4);
  WiFiManagerParameter custom_device_name("devName", "device name", device_name, 32);
  WiFiManagerParameter custom_isy_url("isyUrl", "ISY URL", isy_url, 50);

  WiFiManagerParameter custom_aio_username("aio_username", "Adafruit IO User Name", aio_username, 22);
  WiFiManagerParameter custom_aio_key("aio_key", "Adafruit IO Key", aio_key, 50);
  WiFiManagerParameter custom_aio_feed("aio_feed", "Adafruit IO Feed Name", aio_feed, 22);

  // Set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  // Add custom setup parameters here
  wifiManager.addParameter(&custom_device_name);
  wifiManager.addParameter(&custom_aio_username);
  wifiManager.addParameter(&custom_aio_key);
  wifiManager.addParameter(&custom_aio_feed);
  wifiManager.addParameter(&custom_isy_x10_dev);
  wifiManager.addParameter(&custom_isy_url);
  
  // Use Wifimanager to connect with saved ssid/passwd.  If none or no connection, start an access point. 
  wifiManager.autoConnect("Sonoff_AP");
  digitalWrite(gpio13Led, HIGH);  // Led goes off when connected
  Serial.print("Connected! \t");
  Serial.println(WiFi.localIP());
  digitalWrite(gpio13Led, HIGH);


  // Read updated setup parameters from ESP's file system
  strcpy(isy_x10_dev, custom_isy_x10_dev.getValue());
  strcpy(device_name, custom_device_name.getValue());
  strcpy(isy_url, custom_isy_url.getValue());
  strcpy(aio_username, custom_aio_username.getValue());
  strcpy(aio_key, custom_aio_key.getValue());
  strcpy(aio_feed, custom_aio_feed.getValue());


  // Save the custom parameters to FS
  if (shouldSaveConfig) {
    eeprom_save();
  }

  // Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
  if (strlen(aio_username) > 0 && strlen(aio_key) > 0 && strlen(aio_feed) > 0) {
    
    mqtt = new Adafruit_MQTT_Client(new WiFiClient(), AIO_SERVER, AIO_SERVERPORT, aio_username, aio_key);

    // Setup a feed for subscribing to changes.
    char* fpath = "/feeds/";
    subPath = (char*)malloc(strlen(aio_username) + strlen(fpath) + strlen(aio_feed) + 2);
    strcpy(subPath, aio_username);
    strcat(subPath, fpath);
    strcat(subPath, aio_feed);

    myFeed = new Adafruit_MQTT_Subscribe(mqtt, (char*)subPath);

    mqtt->subscribe(myFeed);
    
  } else {
    setupMQTTOk = false;
    Serial.printf("NOTE: Not Enough AIO Client Info to Setup Feed!\n");
  }
  
  Serial.printf("ISY X10 Dev: \t%s\n",isy_x10_dev);
  Serial.printf("Device Name: \t%s\n",device_name);
  Serial.printf("ISY URL: \t%s\n",isy_url);
  Serial.printf("AIO User: \t%s\n",aio_username);
  Serial.printf("AIO Key: \t%s\n", aio_key);
  Serial.printf("AIO Feed: \t%s\n",aio_feed);
  Serial.printf("AIO Feed Path: \t%s\n",subPath);
  Serial.printf("Compiled: \t%s\n",compile_info);

  String deviceNameStr(device_name);

  webPage += "<h1 style=\"text-align:center;font-size:500%;\">" + deviceNameStr + "</h1>";
  webPage += "<p align=\"center\">";
  webPage += "<a href=\"on\"><button style=\"height:300px;width:300px;font-size:500%;\">ON</button></a>";
  webPage += "&nbsp;&nbsp;&nbsp;&nbsp;";
  webPage += "<a href=\"off\"><button style=\"height:300px;width:300px;font-size:500%\">OFF</button></a></p>";
//  webPage += "</p><p><br><br><br><br><br><br></p>";
  webPage += "<p style=\"padding-bottom:13cm;\"></p>";
  webPage += "<h2 style=\"text-align:center;font-size:300%;\">Settings</h2>";
  webPage += "<h3 style=\"text-align:left;font-size:100%;\">Adafruit IO User Name: " + String(aio_username) + "</h3>";
  webPage += "<h3 style=\"text-align:left;font-size:100%;\">Adafruit IO User Key:  " + String(aio_key) + "</h3>";
  webPage += "<h3 style=\"text-align:left;font-size:100%;\">Adafruit IO Feed Name: " + String(aio_feed) + "</h3>";
  webPage += "</p><p><br><br></p>";
  webPage += "<h3 style=\"text-align:left;font-size:100%;\">ISY X10 Device:    " + String(isy_x10_dev) + "</h3>";
  webPage += "<h3 style=\"text-align:left;font-size:100%;\">ISY URL: " + String(isy_url) + "</h3>";
  webPage += "<p style=\"padding-bottom:3cm;\"></p>";
  webPage += "<h4>Compiled: " + String(compile_info) + "</h4>";

  // HTTP Server commands
  server.on("/", [](){
    server.send(200, "text/html", webPage);
    Serial.println("Got http: root");
  });
  
  server.on("/on", [](){
    server.send(200, "text/html", webPage);
    Serial.println("Got http: on");
    relayState = 1;
  });
  
  server.on("/off", [](){
    server.send(200, "text/html", webPage);
    Serial.println("Got http: off");
    relayState = 0;
  });

  server.begin();
  Serial.println("HTTP server started");  
}

void loop(void){

  // Service the web page
  server.handleClient();

  //  If MQTT is good to go
  if (setupMQTTOk == true) { 
    
    // Ensure the connection to the MQTT server is alive
    MQTT_connect();
  
    // Check for new messages from the AIO feed (2 sec timeout)
    Adafruit_MQTT_Subscribe *subscription;
    while ((subscription = mqtt->readSubscription(2000))) {
      if (subscription == myFeed) {
        String cmd = String((char *)myFeed->lastread);
        Serial.print(F("MQTT Got: "));
        Serial.println(cmd);
        cmd.toLowerCase();
  
        // Toggle relay & led based on MQTT command
        if (cmd.indexOf("on") >= 0) {
          relayState = 1;
        } else if (cmd.indexOf("off") >= 0) {
          relayState = 0;
        }
      }
    }

    // Ping the AIO server to keep the mqtt connection alive about once per min.
    pingCnt++;
    if (pingCnt > 30) {
      Serial.println("Ping!");
      if(! mqtt->ping()) {
        Serial.println("Ping Failed!");
        mqtt->disconnect();
      }
      pingCnt = 0;
    }
    
  } else {
    
    // Add a delay similar to the mqtt timeout above so button timing still works
    delay(2000);
  }

  // Check for relay state changes
  if (relayState != old_relayState) {
    setRelay(relayState);
    old_relayState = relayState;
  }
  
  // Detect Sonoff long button presses (~10 secs), short button press handled with interrupt
  if (digitalRead(gpio0Button) == LOW) {
    buttonCnt += 1;
    //Serial.printf("PRESSED LOW cnt: %i\n",buttonCnt);
    //delay(100);
  } else {
    
    if (buttonCnt >= 4) {
      Serial.printf("Long button press (Cnt: %d) Enter setup mode\n",buttonCnt);
      // Blink led to indicate mode change
      for (int t=0; t<20; t++) {
        digitalWrite(gpio13Led, LOW);
        delay(100);
        digitalWrite(gpio13Led, HIGH);
        delay(100);
      }
      
      Serial.printf("Resetting wifi, please cycle power!");
      wifiManager.resetSettings();
      delay(2000);
      ESP.reset();   // Note: This won't work until board has been power cycled once, after that, ok
      delay(5000);
    }
    buttonCnt = 0;
  }
} 

///////////// Functions /////////////

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

// Set the relay, led and the ISY X10 device
void setRelay(int state) {
  
  if (state == 1) {
    digitalWrite(gpio13Led, LOW);
    digitalWrite(gpio12Relay, HIGH);
    //relayState = 1;
    //myPub->publish("on");
    Serial.println("setRelay: On"); 
    if (strlen(isy_x10_dev) != 0 and strlen(isy_url) != 0) {
      setISYx10(isy_x10_dev,state);
    }  
  } else {
    digitalWrite(gpio13Led, HIGH);
    digitalWrite(gpio12Relay, LOW);
    //relayState = 0;
    //myPub->publish("off");
    Serial.println("setRelay: Off");
    if (strlen(isy_x10_dev) != 0 and strlen(isy_url) != 0) {
      setISYx10(isy_x10_dev,state);
    }  
  }
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  //Serial.println("In MQTT... ");
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

// Setup the ESP's eeprom file system to save config data
void eeprom_setup() {
  
  if (SPIFFS.begin()) {
    Serial.println("Mounted file system");

    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      //Serial.println("Reading config file");
      File configFile = SPIFFS.open("/config.json", "r");

      if (configFile) {
        //Serial.println("Opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        Serial.println("Config file:");
        json.printTo(Serial);
        
        if (json.success()) {
          //Serial.println("\nparsed json");
          
          strcpy(isy_x10_dev,   json["isy_x10_dev"]);
          strcpy(device_name,     json["device_name"]);
          strcpy(isy_url,         json["isy_url"]);
          strcpy(aio_username,    json["aio_username"]);
          strcpy(aio_key,         json["aio_key"]);
          strcpy(aio_feed,        json["aio_feed"]);
          
        } else {
          Serial.println("ERROR: Failed to load json config");
        }
      }
    }
  } else {
    Serial.println("ERROR: Failed to mount FS");
  }
}

// Save config vars to the ESP's eeprom file system
void eeprom_save() {
  Serial.println("Saving config");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  
  json["isy_x10_dev"]   = isy_x10_dev;
  json["device_name"]   = device_name;
  json["isy_url"]       = isy_url;
  json["aio_username"]  = aio_username;
  json["aio_key"]       = aio_key;
  json["aio_feed"]      = aio_feed;

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("ERROR: Failed to open config file for writing");
  }

  Serial.println("Saved:");
  json.printTo(Serial);
  json.printTo(configFile);
  configFile.close();
}

// Button interrupt handler, toggle relay/led on short button presses
void buttonInterrupt() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();

  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 200) {
    
    if (relayState == 1) {
      relayState = 0;
    } else {
      relayState = 1;
    } 
  }
  last_interrupt_time = interrupt_time;
}

