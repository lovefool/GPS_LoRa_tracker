/***************************************************
GPS_LoRa_tracker_Receiver_JP_W.ino

Receiver application (Receive message(GPSdata) via LoRa and display on OLED, and share on WiFi)
Platform: ESP-WROOM-32D w/ CLEALINK E220-900t22S(JP)
To add SD Card adaptor (SPI), E220 wiring has been changed

This version use HardwareSerial2 on ESP2 to connect E220.
When use of espSoftwareSerial on ESP32, use modified LoRa_E220.h (2024.2.3)-> Use original 

*** LoRa *** CLEALINK E220-900T22S(JP)
 * E220       ----- ESP-WROOM-32      
 * M0         ----- GPIO26            
 * M1         ----- GPIO27 
 * RX         ----- GPIO17(U2TXD) 
 * TX         ----- GPIO16(U2RXD)
 * AUX        ----- GPIO14 
 * VCC        ----- 3.3v
 * GND        ----- GND

*** OLED *** SH1106
 * SH1106     ----- ESP-WROOM-32 
 * VCC        ----- 3.3v
 * GND        ----- GND 
 * SCL        ----- GPIO22(SCL) 
 * SDA        ----- GPIO21(SDA)

*** SD Card ***
* SD Card     ----- ESP-WROOM-32  
* VCC         ----- 3.3v
* CS          ----- GPIO5
* MOSI        ----- GPIO23
* CLK         ----- GPIO18
* MISO        ----- GPIO19
* GND         ----- GND    

* LED         ----- ESP-WROOM-32  
* GRN-LED-A   ----- GPIO32
* GRN-LED-K   ----- GND

2024.02.10  Rev.0.1 Initial release
2024.02.15  Rev.0.2 Change OLED device from SSD1306 to SH1106
2024.02.19  Rev.0.3 TimeLib to UTC to JST, DEBUG output changed for CSV format
2024.06.21  Rev.0.4 ESP-WROOM-32D HardwareSerial2 and new assign of M0, M1 and AUX
2024.06.25  Rev.0.5 Change assign of M0, M1 and AUX for eazy wiring
                    SD card interface
2024.09.06  Rev.0.6 Change SD logging / Add LED 
2024.10.21  Rev.0.7 Add WiFi function

Author : Jay Teramoto
https://github.com/lovefool/GPS_LoRa_tracker/tree/main

Arduino IDE 2.3.3
Adafruit_GFX 1.11.10
Adafruit_SH110X 2.1.10
EByte_LoRa_E220_library 1.0.8
TimeLib 1.6.1
esp32 2.0.17 <----- version 3.x and later uses ESP-IDF 5. Stay ESP-IDF 4.
ArduinoJson 7.2.0
***************************************************/

#include "EByte_LoRa_E220_library.h" // LoRa_E220.h is original.
// #include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h> //https://github.com/adafruit/Adafruit-GFX-Library
// #include <Adafruit_SSD1306.h> //https://github.com/adafruit/Adafruit_SSD1306
#include <Adafruit_SH110X.h> //https://github.com/adafruit/Adafruit_SH110X
#include <TimeLib.h> //https://playground.arduino.cc/Code/Time/
#include <SD.h>
#include <ArduinoJson.h>  //https://arduinojson.org/

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
//  Serial2 GPIO17=U2TXD, GPIO16=U2RXD

char sz[32]; // Serial.print buffer
char logData[256]; // log buffer

// Software Serial for Lora E220
// SoftwareSerial LoraSer(D7,D8);
LoRa_E220 e220ttl(&Serial2, AUX, M0, M1); // AUX M0 M1

// OLED SH1106
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

//Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_SH1106G oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int chipSelect = 5;  // SD Card Chip select pin
bool  SDcard_avail = false; // SD card in module? if not no logging.

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

char  buff_id[16];
char  buff_cnt[16];
char  buff_lat[16];
char  buff_lng[16];
char  buff_ymd[16];
char  buff_hms[16];
char  buff_RSSI[16];

#define time_offset 32400    // UTC+9時間(3600 * 9 秒）

#define LED 32               // LED - GPIO32

#include <WebServer.h>        // include ESP32 library
WebServer server (80);
char ssidAP[] = "GPSTRACKER";        // WLAN SSID and password
char passwordAP[] = "12345678";
IPAddress local_ip(192,168,77,1);      // pre-defined IP address values
IPAddress gateway(192,168,77,1);
IPAddress subnet(255,255,255,0);

#include  "buildpage.h"

void base()                        // function to load default webpage
{                                 // and send HTML code to client
  server.send (200, "text/html", page);

  DBG_PRINTLN("root requested");
}



void reload()                        // function to load default webpage
{                                 // and send HTML code to client

  // document.getElementById("gLAT").innerHTML=res.gLAT;
  // document.getElementById("gLONG").innerHTML=res.gLONG;
  // document.getElementById("gID").innerHTML=res.gID;
  // document.getElementById("gDATE").innerHTML=res.gDATE;
  // document.getElementById("gTIME").innerHTML=res.gTIME;
  // document.getElementById("gRSSI").innerHTML=res.gRSSI;
  // document.getElementById("gCNT").innerHTML=res.gCNT;
  // std::string gps_json = R"({"gLAT": 123, "GLONG": "Alice","gID": 123,"gDATE": 123,"gTIME": 123,"gRSSI": 123,"gCNT": 123,})";

  StaticJsonDocument<200> doc;
  static char gps_json[200]="";

  // doc["temperature"] = 24.5;
  // doc["humidity"] = 60.5;
  // serializeJson(doc, outputtext, 100);
  // Serial.println(outputtext);

  doc["gLAT"] = buff_lat;
  doc["gLONG"] = buff_lng;
  doc["gID"] = buff_id;
  doc["gDATE"] = buff_ymd;
  doc["gTIME"] = buff_hms;
  doc["gRSSI"] = buff_RSSI;
  doc["gCNT"] = buff_cnt;

  serializeJson(doc, gps_json, 200);
  DBG_PRINTLN(gps_json);


  // char gps_json[] =R"(
  // {
  // "gID": "GPSLoRA1",
  // "gLAT": " 35.6809591",  
  // "gLONG": "139.7673068",
  // "gDATE": "2024/09/30",
  // "gTIME": "12:00:00",
  // "gRSSI": "-255",
  // "gCNT": "123"
  // }
  // )";


  server.send (200, "text/json", gps_json);

  DBG_PRINTLN("reload requested");
}


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
  oled.setCursor(0,0);oled.print(" GPS-LoRa");
  oled.setCursor(0,18);oled.print("  Tracker");
  oled.setCursor(0,36);oled.print("   v0.7");
  oled.display();

// SD Card initialize
  if (!SD.begin(chipSelect)) {
    DBG_PRINTLN("SDCard initialing fail");
    SDcard_avail = false;
  } else {
    DBG_PRINTLN("SDCard initialing success");
    SDcard_avail = true;
  }
  
  DBG_PRINTLN("Start receiving ...."); 
  delay(1000);
  
  WiFi.mode(WIFI_AP);         // Wi-Fi AP mode
  delay(1000);            // setup AP mode
  WiFi.softAP(ssidAP, passwordAP);      // initialise Wi-Fi with
  WiFi.softAPConfig(local_ip, gateway, subnet); //  predefined IP address
  server.begin();           // initialise server

  // Serial.print("IP address: ");
  // Serial.println(WiFi.localIP());     // display server IP address
  server.begin();
  server.on("/", base);        // map URL to function
  server.on("/reload", reload);        // map URL to function

  // initialize digital pin LED as an output and blink once
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  delay(1000); 
  digitalWrite(LED, LOW);
}

void loop() {

  // Handle http request
  server.handleClient();

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


      // Structured data to msg buffer
      msg = *(LoRamessage*) rsc.data; 

      //Convert UST to JST
      setTime(msg.gpshour, msg.gpsminute, msg.gpssecond, msg.gpsday, msg.gpsmonth, msg.gpsyear);
      adjustTime(time_offset);   

      // message format conversion

      sprintf(buff_id, "%8s",msg.id);
      sprintf(buff_cnt, "%6d",msg.count);
      sprintf(buff_lat, "%10.6f",msg.gpslat);
      sprintf(buff_lng, "%10.6f",msg.gpslng);
      sprintf(buff_ymd, "%04d/%02d/%02d", year(),month(),day());
      sprintf(buff_hms, "%02d:%02d:%02d", hour(),minute(),second());
      sprintf(buff_RSSI, "%3d", rsc.rssi);

      // Close receiving
      rsc.close(); 

      // DEBUG : Serial output of Structured data (msg)
      DBG_PRINT(buff_id); DBG_PRINT(", "); //id
      DBG_PRINT(buff_cnt); DBG_PRINT(", "); //cnf
      DBG_PRINT(buff_lat); DBG_PRINT(", "); //latitude
      DBG_PRINT(buff_lng); DBG_PRINT(", "); //longtitude
      DBG_PRINT(buff_ymd); DBG_PRINT(", "); //YYYY/MM/DD
      DBG_PRINT(buff_hms); DBG_PRINT(", "); //HH:MM:SS
      DBG_PRINTLN(buff_RSSI); // RSSI

      // OLED Display
      oled.clearDisplay();
      oled.setTextSize(2);
      //  oled.setTextColor(WHITE); // SSD1306
      oled.setTextColor(SH110X_WHITE);  //SH1106             

      oled.setCursor(0,0);                // display latitude      
      // sprintf(sz, "%10.6f", msg.gpslat);  
      oled.print(buff_lat);

      oled.setCursor(0,18);               // display longtitude
      // sprintf(sz, "%10.6f", msg.gpslng);  
      oled.print(buff_lng);

      oled.setTextSize(1);                // display JST date and time             
      oled.setCursor(0,40);
      // sprintf(sz, "%04d/%02d/%02d %02d:%02d:%02d", year(),month(),day(),hour(),minute(),second()); // JST 
      oled.print(buff_ymd);
      oled.setCursor(74,40);
      oled.print(buff_hms);

      oled.setCursor(30,54);               // display RSSI and count
      // sprintf(sz, "RSSI:%3d COUNT:%6d", rsc.rssi, msg.count);
      oled.print(buff_RSSI);
      oled.setCursor(60,54);               // display RSSI and count
      oled.print(buff_cnt);

      oled.display();
      delay(10);


      /***** Log file  ****/
      // Open SD Card when SD card is availale
      if(SDcard_avail) {

        File logFile = SD.open("/gpslog.txt", FILE_APPEND);  // File open

        if (logFile) {
        
          // sprintf(logData, "%s, %10.6f, %10.6f, %04d/%02d/%02d, %02d:%02d:%02d, %3d, %6d",   // ログデータの作成
          //   msg.id, 
          //   msg.gpslat, msg.gpslng, 
          //   year(), month(), day(),
          //   hour(), minute(), second(),
          //   rsc.rssi, 
          //   msg.count) ;
          
          // logFile.println(logData);  // ログデータをファイルに追記(改行付き)
          
          logFile.print(buff_ymd);logFile.print(", ");    // Date
          logFile.print(buff_hms);logFile.print(", ");    // Time
          logFile.print(buff_cnt);logFile.print(", ");    // cnt
          logFile.print(buff_lat);logFile.print(", ");    // latitude
          logFile.print(buff_lng);logFile.print(", ");    // longtitude
          logFile.print(buff_id);logFile.print(", ");     // id
          logFile.println(buff_RSSI);                     // RSSI

          logFile.close();  // ファイルを閉じる
          
          DBG_PRINTLN("logged");
          // DBG_PRINTLN(logData);
        } else {
          DBG_PRINTLN("logging error");
        }

      }


    }
  }
} // end of loop