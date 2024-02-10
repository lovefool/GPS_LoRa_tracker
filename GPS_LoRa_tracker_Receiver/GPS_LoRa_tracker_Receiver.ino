/***************************************************
GPS_LoRa_tracker_Receiver.ino
Receiver application (Receive message(GPSdata) via LoRa and display on OLED, and share on WiFi)
Platform: Wemos D1 mini ESP32

For use of ESP32, use modified LoRa_E220.h (2024.2.3)

 * E220       ----- Wemos D1 mini     ----- Wemos D1 MINI ESP32(compatible pins)
 * M0         ----- D5                ----- D5
 * M1         ----- D6                ----- D6
 * RX         ----- D8 (No pull-up)   ----- D8
 * TX         ----- D7 (No pull-up)   ----- D7
 * AUX        ----- D3 (No pull-up)   ----- D3
 * VCC        ----- 3.3v              ----- 3.3V
 * GND        ----- GND               ----- GND

 * GPS       ----- Wemos D1 mini
 * RX         ----- D4 (No pull-up)   ----- D4
 * TX         ----- D0 (No pull-up)   ----- D0
 * VCC        ----- 3.3v
 * GND        ----- GND

2024.02.10  Rev.0.1
Author : Jay Teramoto
https://github.com/lovefool/GPS_LoRa_tracker/tree/main
***************************************************/

#include "EByte_LoRa_E220_library.h" // LoRa_E220.h is modified !!!!
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h> //https://github.com/adafruit/Adafruit-GFX-Library
#include <Adafruit_SSD1306.h> //https://github.com/adafruit/Adafruit_SSD1306
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

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

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

void setup() {
  delay(500);
  DBG_PRINTER.begin(9600);
  while(!DBG_PRINTER){};
  delay(500);
  
  e220ttl.begin();                        // Start LoRa E220}
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3c); // Start OELD SSD1306

  // Display initial 
  oled.clearDisplay();
  oled.setTextSize(2);             
  oled.setTextColor(WHITE);

  oled.setCursor(0,0);      
  oled.print("    GPS-LoRa");

  oled.setCursor(0,18);
  oled.print("Tracker0.1");

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

      DBG_PRINT(msg.id);    // Structured data
      DBG_PRINT(" "); 
      sprintf(sz, "%6d",msg.count); DBG_PRINTLN(msg.count); 
      sprintf(sz, "%10.6f",msg.gpslat); DBG_PRINTLN(sz);
      sprintf(sz, "%10.6f",msg.gpslng); DBG_PRINTLN(sz);
      sprintf(sz, "%04d/%02d/%02d %02d:%02d:%02d", msg.gpsyear, msg.gpsmonth, 
              msg.gpsday, msg.gpshour, msg.gpsminute, msg.gpssecond);
      DBG_PRINTLN(sz);
      
      DBG_PRINT("RSSI: "); DBG_PRINTLN(rsc.rssi, DEC); // RSSI

      // Display
      oled.clearDisplay();
      oled.setTextSize(2);             
      oled.setTextColor(WHITE);

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









