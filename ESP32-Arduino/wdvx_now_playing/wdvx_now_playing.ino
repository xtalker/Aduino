#include <WiFi.h>
#include <ArduinoJson.h>   // https://github.com/bblanchon/ArduinoJson
#include "time.h"
#include <Wire.h>
#include "SSD1306.h"
#include "OLEDDisplayUi.h" // https://github.com/ThingPulse/esp8266-oled-ssd1306

//const char* ssid = "CHAOS-HOUSE";
//const char* password = "ch405c0de2018";
const char* ssid = "6fusion";
const char* password = "working downtown is fun";
const char* host = "api.wdvx.com";
const char* wdvx_api_url = "https://api.wdvx.com/playlistcenter/api/nowplaying/";
const int   update_freq = 30;

String currTrackInfo;

// Memory pool for JSON object tree.
DynamicJsonBuffer jsonBuffer(300);
WiFiClient client;
SSD1306  display(0x3c, 4, 15);
OLEDDisplayUi ui     ( &display );

const char activeSymbol[] PROGMEM = {
    B00000000,
    B00000000,
    B00011000,
    B00100100,
    B01000010,
    B01000010,
    B00100100,
    B00011000
};

const char inactiveSymbol[] PROGMEM = {
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00011000,
    B00011000,
    B00000000,
    B00000000
};
void msOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(30, 0, "WDVX");
  display->drawString(128, 0, "Now Playing");
}

void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  // Demo for drawStringMaxWidth:
  // with the third parameter you can define the width after which words will be wrapped.
  // Currently only spaces and "-" are allowed for wrapping
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_24);
  display->drawString(0 + x, 10 + y, "Title");
}

void drawFrame2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  // Demo for drawStringMaxWidth:
  // with the third parameter you can define the width after which words will be wrapped.
  // Currently only spaces and "-" are allowed for wrapping
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_24);
  display->drawStringMaxWidth(0 + x, 10 + y, 128, "This is the artist's name");
}

FrameCallback frames[] = { drawFrame1, drawFrame2 };
int frameCount = 2;

// Overlays are statically drawn on top of a frame eg. a clock
OverlayCallback overlays[] = { msOverlay };
int overlaysCount = 1;

int Call_API(String* resultString) {
  client.stop();  // Clear any current connections
  Serial.println("Connecting to " + String(host)); // start a new connection
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("Connection failed");
    return false;
  }
  // WDVX "PLay List Center API": https://api.wdvx.com/playlistcenter/api/nowplaying/
  String url = wdvx_api_url;
  //Serial.println("Requesting URL: " + String(url));
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Connection Timeout...Stopping");
      client.stop();
      return false;
    }
  }
  //Serial.print("Receiving track data");
  while (client.available()) {
    *(resultString) = client.readStringUntil('\r');
    Serial.print(".");
  }
  Serial.println("\r\nClosing connection");
  return true;
}

void setup () {

  Serial.begin(115200);
  Serial.println("WDVX Now Playing!");
  WiFi.begin(ssid, password);

  Serial.print(F("Connecting to "));
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }

  Serial.println();
  Serial.println(F("WiFi connected"));
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());
  
  // Reset the OLED
  pinMode(16,OUTPUT);
  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(50); 
  digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high

//  while (WiFi.status() != WL_CONNECTED) {
//
//    delay(1000);
//    Serial.print("Connecting..");
//
//    Call_API(&currTrackInfo);      // Get data with an API call and place response in a String
//    Serial.println(currTrackInfo); // Display the response
//  }

  // The ESP is capable of rendering 60fps in 80Mhz mode
  // but that won't give you much time for anything else
  // run it in 160Mhz mode or just set it to 30 fps
  ui.setTargetFPS(30);

  // Customize the active and inactive symbol
  ui.setActiveSymbol(activeSymbol);
  ui.setInactiveSymbol(inactiveSymbol);

  // You can change this to
  // TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorPosition(BOTTOM);

  // Defines where the first frame is located in the bar.
  ui.setIndicatorDirection(LEFT_RIGHT);

  // You can change the transition that is used
  // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_UP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_RIGHT);

  // Add frames
  ui.setFrames(frames, frameCount);

  // Add overlays
  ui.setOverlays(overlays, overlaysCount);

  // Initialising the UI will init the display too.
  ui.init();

  display.flipScreenVertically();

}

void loop() {
  
  int remainingTimeBudget = ui.update();

  if (remainingTimeBudget > 0) {
    
    Serial.printf("Getting track info\n");
    
//    if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
//  
//      Call_API(&currTrackInfo);      // Get data with an API call and place response in a String
//      Serial.println(currTrackInfo); // Display the response
//  
//      JsonObject& root = jsonBuffer.parseObject(currTrackInfo);
//      if (!root.success())
//      {
//        // if the root object could not be created, then return an error make an API call the next time around
//        Serial.println("Unable to create a root object");
//      }
//  
//      const char* trackArtist = root["artist"];
//      const char* trackTitle = root["title"];
//      Serial.printf("Artist: %s, Track: %s\n",trackArtist,trackTitle);
//    }

    delay(remainingTimeBudget);
//  delay(5000);    //Send a request every 30 seconds
//  ui.nextFrame();  // seems to slowly slide the current fram to the right one char at a time?
  }
}

