
// Chime Outputs
//      Name       GPIO Labeled
#define CHIME_LOW   15  // D8
#define CHIME_MED   13  // D7
#define CHIME_HI    12  // D6

// LED outputs
#define LED         4   // D2
#define NODE_LED    2   // D4 (active LOW)  

// Switch inputs (All are active LOW!)
#define FRONT_DOOR  14  // D5
#define BACK_DOOR   0   // D3
#define CTRL_BUTTON 5   // D1

// Chime indexes for sequence tables
#define LOW 0
#define MED 1
#define HI  2
#define LEN 3

#define TEMPO 1000    // How fast the tune plays
#define HIT_DELAY 20  // The solenoid on delay so it hits the chime properly
const long connected_blink = 50;

// Setup for saving extra config params from access point setup
// Define your default values here, if there are different values in config.json, they are overwritten.
char isy_x10_dev[4];    // Name of the Isy X10 device used to track the relay state (ie: 'A1')
char isy_url[50];       // Should look like: "http://<user>:<passwd>@<ISY_IP_Address>" (with your credentials and IP)

// Other vars
String webPage = "";
const long node_led_interval = 50;
unsigned long node_led_prev_millis = 0;  // Last time NODE_LED was updated
bool shouldSaveConfig = false;  // Flag for saving SPIFF data

// Define an HTTP Client & Server
HTTPClient http;
ESP8266WebServer server(80);

