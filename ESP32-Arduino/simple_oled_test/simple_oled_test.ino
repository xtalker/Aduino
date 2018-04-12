/*****************************************************
 * A quick, simple 0LED display test on the ESP32 w/OLED board
 * 128 X 32 SSD1306  - Library by Daniel Eichhorn
 *****************************************************/

/* WiFi parameters and credentials */
/*  SSD1306 */
#include "SSD1306.h"
SSD1306  display(0x3c, 4, 15);   // display(SSD_ADDRESS, SDA_PIN, SCL_PIN)

const int Max_screen_width = 130;

// Font max pixels per font for this OLED screen
const int ArialMT_Plain_10_max = 5 * Max_screen_width;
const int ArialMT_Plain_16_max = 3 * Max_screen_width;
const int ArialMT_Plain_24_max = 2 * Max_screen_width;

void display_string(char *str) {
  display.clear();
  
  uint16_t str_len = display.getStringWidth(str);
  printf("Len: %i\n",str_len);

  // Use the biggest font possible based on string length (in pixels)
  if (str_len <= ArialMT_Plain_24_max) {
    display.setFont(ArialMT_Plain_24);
    Serial.println("big font");
  } else if (str_len > ArialMT_Plain_24_max and str_len <= ArialMT_Plain_16_max) {
    display.setFont(ArialMT_Plain_16);
    Serial.println("medium font");
  } else {  
    display.setFont(ArialMT_Plain_10);
    Serial.println("small font");
  }
  
  display.drawStringMaxWidth(0, 0, Max_screen_width, str);
  display.display();
  Serial.printf("Display: %s\n",str);
}

void setup() 
{
  Serial.begin(115200);
  Serial.println("");
  Serial.println("Somple OLED Display Test");
  Serial.println("");
  
  // Reset/init the OLED
  pinMode(16,OUTPUT);
  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(50); 
  digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high
  display.init();

  // Set up the OLED
  display.flipScreenVertically();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  //display.setFont(Open_Sans_Condensed_Light_20); // set a font
}


void loop() {
  char *str;
  str = "This is a test of a really long string, so ha ha ha!";
//  str = "1........10........20.........30.........40.........50.........";
  display_string(str);
  delay(3000);

  // Change font
  str = "Hello there";
//  str = "1........10........20.........30.........40.........50.........";
  display_string(str);
  delay(3000);

  str = "This is a test of a really long string, so ha ha ha!";
//  str = "1........10........20.........30.........40.........50.........";
  display_string(str);
  delay(3000);
}

