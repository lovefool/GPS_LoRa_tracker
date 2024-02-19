/***************************************************
GPS_LoRa_tracker_Receiver.ino
Receiver application (Receive message(GPSdata) via LoRa and display on OLED, and share on WiFi)
Platform: Wemos D1 mini ESP32

For use of espSoftwareSerial on ESP32, use modified LoRa_E220.h (2024.2.3)
*** LoRa *** EBYTE E220-900T22D
 * E220       ----- Wemos D1 mini     ----- Wemos D1 MINI ESP32(compatible pins)
 * M0         ----- D5                ----- D5
 * M1         ----- D6                ----- D6
 * RX         ----- D8 (No pull-up)   ----- D8
 * TX         ----- D7 (No pull-up)   ----- D7
 * AUX        ----- D3 (No pull-up)   ----- D3
 * VCC        ----- 3.3v              ----- 3.3V
 * GND        ----- GND               ----- GND

*** OLED *** SH1106
 * SH1106    ----- Wemos D1 mini     ----- Wemos D1 MINI ESP32(compatible pins)
 * VCC        ----- 3.3v              ----- 3.3V
 * GND        ----- GND               ----- GND
 * SCL        ----- Dx                ----- Dx
 * SDA        ----- Dx                ----- Dx

*** GPS ***
 * GPS        ----- Wemos D1 mini     ----- Wemos D1 MINI ESP32(compatible pins)
 * RX         ----- D4 (No pull-up)   ----- D4
 * TX         ----- D0 (No pull-up)   ----- D0
 * VCC        ----- 3.3v              ----- 3.3v
 * GND        ----- GND               ----- GND

2024.02.10  Rev.0.1 Initial release
2024.02.15  Rev.0.2 Change OLED device from SSD1306 to SH1106
2024.02.19  Rev.0.3 TimeLib to UTC to JST, DEBUG output changed for CSV format

TODO: WiFi 

Author : Jay Teramoto
https://github.com/lovefool/GPS_LoRa_tracker/tree/main
***************************************************/

#include "EByte_LoRa_E220_library.h" // LoRa_E220.h is modified !!!!
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h> //https://github.com/adafruit/Adafruit-GFX-Library
// #include <Adafruit_SSD1306.h> //https://github.com/adafruit/Adafruit_SSD1306
#include <Adafruit_SH110X.h> //https://github.com/adafruit/Adafruit_SH110X
#include <TimeLib.h> //https://playground.arduino.cc/Code/Time/

#define GPS_LoRa_DEBUG
//******************** DEBUG ******************
// Define where debug output will be printed.
#define DBG_PRINTER Serial

// Setup debug printing macros.
#ifdef GPS_LoRa_DEBUG
  #define DBG_PRINT(...) { DEBUG_PRINTER.print(__VA_ARGS__); }
  #define DBG_PRINTLN(...) { DEBUG_PRINTER.println(__VA_ARGS__); }
#else
  #define DBG_PRINT(...)  {}
  #define DBG_PRINTLN(...)  {}
#endif
//******************** DEBUG ********************

char sz[32]; // Serial.print buffer

// Software Serial for Lora E220
SoftwareSerial LoraSer(D7,D8);
LoRa_E220 e220ttl(&LoraSer, D3, D5, D6); // AUX M0 M1

// OLED SSD1306
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

//Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_SH1106G oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

struct LoRamessage {  // message structure sent to receiver
  char    id[9];
  int16_t count; 
  double  gpslat;
  double  gpslng;
  int16_t gpsyear;
  int8_t  gpsmonth;
  int8_t  gpsday;
  int8_t  gpshour;
  int8_t  gpsminute;
  int8_t  gpssecond;
}; 
struct LoRamessage msg = {"        ", 0, 0, 0, 0, 0, 0, 0, 0, 0};

#define time_offset 32400    // UTC+9時間(3600 * 9 秒）

void setup() {
  delay(500);
  DBG_PRINTER.begin(9600);
  while(!DBG_PRINTER){};
  delay(500);
  
  e220ttl.begin();                        // Start LoRa E220}
//  oled.begin(SSD1306_SWITCHCAPVCC, 0x3c); // Start OELD SSD1306
  oled.begin(SCREEN_ADDRESS, true); // Start OELD SH1106

  // Display initial 
  oled.clearDisplay();
  oled.setTextSize(2);             
//  oled.setTextColor(WHITE); // SSD1306
  oled.setTextColor(SH110X_WHITE); //SH1106

  oled.setCursor(0,0);      
  oled.print(" GPS-LoRa");

  oled.setCursor(0,18);
  oled.print("  Tracker");

  oled.setCursor(0,36);
  oled.print("   v0.1");


  oled.display();

  DBG_PRINTLN("Start receiving ...."); 
  delay(3000);
}

void loop() {
  // while Lora data
  // If something available
  if (e220ttl.available()>1) {
    // read the String message
    ResponseStructContainer rsc = e220ttl.receiveMessageRSSI(sizeof(msg));
 
    // Is something goes wrong print error
    if (rsc.status.code!=1){
      DBG_PRINT("Response: ");
      DBG_PRINTLN(rsc.status.getResponseDescription());

    } else {
      // Print the data received
      DBG_PRINT("Response: ");
      DBG_PRINTLN(rsc.status.getResponseDescription()); // Response

      msg = *(LoRamessage*) rsc.data;

      // Serial output of Structured data (msg)
      DBG_PRINT(msg.id);DBG_PRINT(", "); //id
      sprintf(sz, "%6d",msg.count); DBG_PRINT(msg.count);DBG_PRINT(", "); // count
      sprintf(sz, "%10.6f",msg.gpslat); DBG_PRINT(sz);DBG_PRINT(", "); // latitude
      sprintf(sz, "%10.6f",msg.gpslng); DBG_PRINT(sz);DBG_PRINT(", "); // longtitude

      setTime(msg.gpshour, msg.gpsminute, msg.gpssecond, msg.gpsday, msg.gpsmonth, msg.gpsyear);
      adjustTime(time_offset);   //JST変換

      sprintf(sz, "%04d/%02d/%02d %02d:%02d:%02d, ", year(),month(),day(),hour(),minute(),second());DBG_PRINT(sz); //time stamp
      
      DBG_PRINT("RSSI:"); DBG_PRINTLN(rsc.rssi, DEC); // RSSI

      // Display
      oled.clearDisplay();
      oled.setTextSize(2);
      //  oled.setTextColor(WHITE); // SSD1306
      oled.setTextColor(SH110X_WHITE);  //SH1106             

      oled.setCursor(0,0);      
      sprintf(sz, "%10.6f", msg.gpslat);  
      oled.print(sz);

      oled.setCursor(0,18);
      sprintf(sz, "%10.6f", msg.gpslng);  
      oled.print(sz);

      oled.setTextSize(1);             
      oled.setCursor(0,40);
      sprintf(sz, "%04d/%02d/%02d %02d:%02d:%02d", msg.gpsyear, msg.gpsmonth, 
                msg.gpsday, msg.gpshour, msg.gpsminute, msg.gpssecond);
      oled.print(sz);

      oled.setCursor(0,54);
      sprintf(sz, "RSSI:%3d COUNT:%6d", rsc.rssi, msg.count);
      oled.print(sz);

      oled.display();
      delay(10);

      rsc.close();
    }
  }
}









