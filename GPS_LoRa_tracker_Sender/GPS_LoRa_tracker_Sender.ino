/***************************************************
GPS_LoRa_tracker_Sender.ino
Sender application (Receive GPS data and send it via E220 LoRa module)
Platform: Wemos D1 mini (8266EX)

This is for ESP32(receiver), not required for 8266(sender)
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

2024.02.09  Rev.0.1
Author : Jay Teramoto
https://github.com/lovefool/GPS_LoRa_tracker/tree/main
***************************************************/

#include "EByte_LoRa_E220_library.h" // LoRa_E220.h is original
#include <TinyGPS++.h> //https://github.com/mikalhart/TinyGPSPlus/blob/master/src/TinyGPS%2B%2B.h
#include <SoftwareSerial.h>

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

// Software Serial for GPS
int GPSBaud = 9600; 
TinyGPSPlus gps;
SoftwareSerial gpsSerial(D4, D0);

// Software Serial for Lora E220
SoftwareSerial LoraSer(D7,D8);
LoRa_E220 e220ttl(&LoraSer, D3, D5, D6); // AUX M0 M1

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
struct LoRamessage msg = {"GPSLoRa1", 0, 0, 0, 0, 0, 0, 0, 0, 0};

unsigned long last_send_time = 0L;
uint16_t  count;  // For checking data lost at receiver side

//******************** SETUP ********************
void setup() {
  delay(500);
  DBG_PRINTER.begin(9600);
  while(!DBG_PRINTER){};
  delay(500);
  
  DBG_PRINTLN("test");
  DBG_PRINTLN("Start...."); 
 
  gpsSerial.begin(GPSBaud); // Start GPS
  e220ttl.begin();          // Start LoRa E220
}

//******************** LOOP ********************
void loop(){
  while (gpsSerial.available() > 0){   // GPS data received

    if (gps.encode(gpsSerial.read())){ 

        // ***** 5 sec passed? *****
        if (millis() - last_send_time > 4999){ 
          last_send_time = millis();

          // ***** set lat,lng to msg ***** 
          if (gps.location.isValid()){
            DBG_PRINT(gps.location.lat(), 6);
            DBG_PRINT(F(","));
            DBG_PRINTLN(gps.location.lng(), 6);
            msg.gpslat = gps.location.lat();
            msg.gpslng = gps.location.lng();
          }
          else {
            DBG_PRINTLN(F("Lat/Lng INVALID"));
            // When invalid, keep last data.
          }

          // ***** set date to msg *****
          if (gps.date.isValid())
          {
            sprintf(sz, "%04d/%02d/%02d ", gps.date.year(), gps.date.month(), gps.date.day());
            DBG_PRINTLN(sz);
            msg.gpsyear = gps.date.year();
            msg.gpsmonth = gps.date.month();
            msg.gpsday = gps.date.day();
          }
          else
          {
            DBG_PRINTLN(F("Date INVALID"));
            msg.gpsyear = 0;
            msg.gpsmonth = 0;
            msg.gpsday = 0;
          }

          // ***** set time to msg *****
          if (gps.time.isValid())
          {
            sprintf(sz, "%02d:%02d:%02d", gps.time.hour(), gps.time.minute(), gps.time.second());
            DBG_PRINTLN(sz);
            msg.gpshour = gps.time.hour();
            msg.gpsminute = gps.time.minute();
            msg.gpssecond = gps.time.second();
          }
          else
          {
            DBG_PRINTLN(F("Time INVALID"));
            msg.gpshour = 0;
            msg.gpsminute = 0;
            msg.gpssecond = 0;
          }
        // ***** counter increment *****
          count++;
          msg.count = count;
          DBG_PRINT(F("CNT "));
          DBG_PRINTLN((count));

        // ***** send msg via Lora E220 *****
          ResponseStatus rs = e220ttl.sendMessage(&msg, sizeof(msg));
        // Check If there is some problem of successfully send
          DBG_PRINTLN(rs.getResponseDescription());
      } 
    }
  } // end of while
  // delay(1000);
  // DBG_PRINTLN("no data");

} // end of loop
