/***************************************************
GPS_LoRa_tracker_Receiver_HardwareSerial2_Simple.ino

E220 Simple test via ESP32 HardwareSerial2 
Use original LoRa_E220.h (2024.2.3) 

**** Caution ****
As of 2024.06.26 E220 library doesn't work with ESP32 board manager 3.x.x

*** LoRa CLEALINK E220-900T22S(JP) connection ***
 * E220       ----- ESP-WROOM-32      
 * M0         ----- GPIO26            
 * M1         ----- GPIO27 
 * RX         ----- GPIO17(U2TXD) 
 * TX         ----- GPIO16(U2RXD)
 * AUX        ----- GPIO14 
 * VCC        ----- 3.3v
 * GND        ----- GND

E220設定(Fixed, RSSI, addh=0, addl=0,chan=0, and so on)
<c0><00><08><00><00><70><01><00><C5><00><00>

2024.06.26  Rev.0.1 Initial release (for receiving test AGAIN!)

Author : Jay Teramoto   https://github.com/lovefool/GPS_LoRa_tracker/tree/main
***************************************************/

#include "EByte_LoRa_E220_library.h" // 1.0.8 LoRa_E220.h is original. https://mischianti.org/category/my-libraries/ebyte-lora-e22-devices/
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

// E220 pins
#define M0    26      // GPIO26
#define M1    27      // GPIO27
#define AUX   14      // GPIO14
//  Serial2 
//  U2TXD   GPIO17
//  U2RXD   GPIO16
LoRa_E220 e220ttl(&Serial2, AUX, M0, M1); // AUX M0 M1

char sz[32]; // Serial.print buffer
#define time_offset 32400    // UTC+9時間(3600 * 9 秒）

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
  DBG_PRINTLN("Start receiving ...."); 
  delay(1000);
}


void loop() {
  // while Lora data
  // If something available
  if (e220ttl.available()>1) {
    // read the String message
    ResponseStructContainer rsc = e220ttl.receiveMessageRSSI(sizeof(msg));
 
    // Is something goes wrong print error
    if (rsc.status.code!=1){
      DBG_PRINT("RES:");
      DBG_PRINTLN(rsc.status.getResponseDescription());

    } else {
      // Print the data received
      DBG_PRINT("RES:");
      DBG_PRINT(rsc.status.getResponseDescription()); // Response from E220
      DBG_PRINT(", ");

      msg = *(LoRamessage*) rsc.data; // Structured data to msg

      // Serial output of Structured data (msg)
      DBG_PRINT(msg.id); DBG_PRINT(", "); //id
      sprintf(sz, "%6d",msg.count); DBG_PRINT(msg.count); DBG_PRINT(", "); // count
      sprintf(sz, "%10.6f",msg.gpslat); DBG_PRINT(sz); DBG_PRINT(", "); // latitude
      sprintf(sz, "%10.6f",msg.gpslng); DBG_PRINT(sz); DBG_PRINT(", "); // longtitude

      setTime(msg.gpshour, msg.gpsminute, msg.gpssecond, msg.gpsday, msg.gpsmonth, msg.gpsyear);
      adjustTime(time_offset);   //JST変換

      sprintf(sz, "%04d/%02d/%02d %02d:%02d:%02d, ", year(),month(),day(),hour(),minute(),second()); DBG_PRINT(sz); //time stamp

      DBG_PRINT("RSSI:"); DBG_PRINTLN(rsc.rssi, DEC); // RSSI

      rsc.close();
    }
  }
}