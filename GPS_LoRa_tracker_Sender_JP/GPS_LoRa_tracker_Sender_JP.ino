/***************************************************
GPS_LoRa_tracker_Sender_JP.ino
Sender application (Receive GPS data and send it via E220 LoRa module)

Platform: Generic ESP-12E/12F, Wemos D1 mini (8266EX)
LoRa module: CLEALINK E220-900T22S(JP) 

 * E220(JP)   ESP-12E/12F     Wemos D1 mini/ miniESP32(compatible pins)
 * M0     ----- GPIO14            D5
 * M1     ----- GPIO12            D6
 * RX     ----- GPIO15(pull-down) D8
 * TX     ----- GPIO13            D7
 * AUX    ----- GPIO0 (pull-up)   D3
 * VCC    ----- 3.3v              3.3V
 * GND    ----- GND               GND

 * GPS        ESP-12E/12F     Wemos D1 mini/ miniESP32(compatible pins
 * RX     ----- GPIO16            D0 
 * TX     ----- GPIO2(pull-up)    D4      *LED on ESP-12E/12F 
 * VCC    ----- 3.3v
 * GND    ----- GND

 * Configure E200-900T22S(JP)
     <c0><00><08><00><00><70><01><00><C5><00><00>

2024.02.09  Rev.0.1
2024.06.07  Rev.0.2   Generic ESP-12E/12F  
Author : Jay Teramoto
https://github.com/lovefool/GPS_LoRa_tracker/tree/main
***************************************************/

#include "EByte_LoRa_E220_library.h" // https://github.com/xreef/EByte_LoRa_E220_Series_Library
                                    // For ESP32 software serial, E220.h needs to be modified.
#include <TinyGPS++.h> //https://github.com/mikalhart/TinyGPSPlus/blob/master/src/TinyGPS%2B%2B.h
#include <SoftwareSerial.h> // Standard library

// #define GPS_LoRa_DEBUG
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

// SoftwareSerial(rxPin, txPin, inverse_logic)
// Software Serial for GPS
#define GpsRx   2   //  D4
#define GpsTx   16  //  D0

int GPSBaud = 9600; 
TinyGPSPlus gps;
SoftwareSerial gpsSerial(GpsRx, GpsTx);

// Software Serial for Lora E220
#define LoRaRx  13    // D7
#define LoRaTx  15    // D8
#define LoRaAUX 0     // D3
#define LoRaM0  14    // D5
#define LoRaM1  12    // D6

SoftwareSerial LoraSer(LoRaRx, LoRaTx);
LoRa_E220 e220ttl(&LoraSer, LoRaAUX, LoRaM0, LoRaM1); // AUX M0 M1

// LoRa E220 destination address & channel
byte  addh = 0;
byte  addl = 0;
byte  channel = 0;

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

        // ***** 2 sec passed? *****
        if (millis() - last_send_time > 1999){ 
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
        // Send message to fixed destination
        ResponseStatus rs = e220ttl.sendFixedMessage(addh, addl, channel, &msg, sizeof(msg));
        //  ResponseStatus rs = e220ttl.sendFixedMessage(addh, addl, channel, "test");
      
        //  ResponseStatus rs = e220ttl.sendMessage(&msg, sizeof(msg));
        // Check If there is some problem of successfully send
          DBG_PRINTLN(rs.getResponseDescription());
      } 
    }
  } // end of while
} // end of loop
