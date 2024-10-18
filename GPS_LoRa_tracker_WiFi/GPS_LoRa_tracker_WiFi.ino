/*******************************************************************************
*
 ******************************************************************************/

#include <WebServer.h>        // include ESP32 library
WebServer server (80);        //  and define LED pin


char ssid[] = "30F772BFCE30-2G";         // change xxxx to Wi-Fi SSID
char password[] = "2215093210666";       // change xxxx to Wi-Fi password

#include <Ticker.h>         // include Ticker library
Ticker timer;           // associate timer with Ticker lib
int lag = 10;           // set timer interval at 10s
// int LEDpin = D3;            // LED pin on D3
// String LED = "off";         // initial LED state
// int count = 0;
String temp, counter;

#include "build.h"

void setup()
{
  Serial.begin(9600);         // define Serial Monitor baud rate
  delay(1000);
  WiFi.begin(ssid, password);       // initialise Wi-Fi
  while (WiFi.status() != WL_CONNECTED) delay(500); // wait for Wi-Fi connect
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());     // display server IP address
  server.begin();
  server.on("/", base);        // map URL to function
  server.on("/reload", reload);        // map URL to function
}

void base()                        // function to load default webpage
{                                 // and send HTML code to client
server.send (200, "text/html", page);
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

char gps_json[] =R"(
{
"gID": "GPSLoRA1",
"gLAT": " 35.6809591",  
"gLONG": "139.7673068",
"gDATE": "2024/09/30",
"gTIME": "12:00:00",
"gRSSI": "-255",
"gCNT": "123"
}
)";

server.send (200, "text/json", gps_json);
}

// void BMP()                // function to get readings
// {
//   temp = String(bmp.readTemperature());     // update BMP280 reading
//   counter = String(count++);          // increment counter
//   digitalWrite(LEDpin, !digitalRead(LEDpin));   // turn on or off the LED
//   if(LED == "on") LED = "off";        // update LED state
//   else LED = "on";
//   server.send (200, "text/html", webcode());    // send response to client
// }

// String webcode()              // return HTML code
// {
//   String page;
//   page = "<!DOCTYPE html><html><head>";
//   page += "<meta http-equiv='refresh' content='9'>";    // refresh every 9s
//   page += "<title>ESP8266</title></head>";
//   page += "<body>";
//   page += "<h2>BMP280</h2>";
//   page += "<p>Temperature: " + temp + " " + "&degC</p>";  // display temp
//   page += "<p>Counter: " + counter + "</p>";      // counter
//   page += "<p>LED is " + LED + "<p>";       // LED state
//   page += "</body></html>";
//   return page;
// }

void loop()
{
  server.handleClient();
}
