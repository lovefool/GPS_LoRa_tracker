/***************************************************
GPS_LoRa_tracker_Config.ino
E220 config program (may be merged to Sender/Receiver program)

Platform: Wemos D1 mini (8266EX) or Wemos mini ESP32

 * E220       ----- Wemos D1 mini     ----- Wemos D1 MINI ESP32
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

2024.01.30  Rev.0.1
Author : Jay Teramoto
https://github.com/lovefool/GPS_LoRa_tracker/tree/main
***************************************************/

#include <SoftwareSerial.h>
#include "EByte_LoRa_E220_library.h"

#define ESP32 1

#if ESP32
  // port setting for ESP32
      #define gps_rxd D4
      #define gps_txd D0
  
  // E220 ports
      #define E220_m0 D5
      #define E220_m1 D6
      #define E220_rxd D8
      #define E220_txd D7
      #define E220_aux D3

#else
  // port setting for 8266EX
  // softserial port for GPS module
      #define gps_rxd D4
      #define gps_txd D0
  
  // E220 ports
      #define E220_m0 D5
      #define E220_m1 D6
      #define E220_rxd D8
      #define E220_txd D7
      #define E220_aux D3

#endif


// E220 port setting
// ---------- esp8266 / ESP32 pins --------------
// LoRa_E220 e220ttl(RX, TX, AUX, M0, M1);  // Arduino RX <-- e220 TX, Arduino TX --> e220 RX 
// SoftwareSerial mySerial(D7, D8); // Arduino RX <-- e220 TX, Arduino TX --> e220 RX
// LoRa_E220 e220ttl(&mySerial, D3, D5, D6); // AUX M0 M1
SoftwareSerial mySerial(E220_rxd, E220_txd); // Arduino RX <-- e220 TX, Arduino TX --> e220 RX
LoRa_E220 e220ttl(&mySerial, E220_aux, E220_m0, E220_m1); // AUX M0 M1
// -------------------------------------

void printParameters(struct Configuration configuration);
void printModuleInformation(struct ModuleInformation moduleInformation);

void setup() {
    delay(500);
    Serial.begin(9600);
    while(!Serial){};
    delay(500);
 
    Serial.println();
    Serial.println("Start config"); 
 
    // Startup all pins and UART
    e220ttl.begin();
 
    ResponseStructContainer c;
    c = e220ttl.getConfiguration();
    // It's important get configuration pointer before all other operation
    Configuration configuration = *(Configuration*) c.data;
    Serial.println(c.status.getResponseDescription());
    Serial.println(c.status.code);
 
    printParameters(configuration);
 
    ResponseStructContainer cMi;
    cMi = e220ttl.getModuleInformation();
    // It's important get information pointer before all other operation
    ModuleInformation mi = *(ModuleInformation*)cMi.data;
 
    Serial.println(cMi.status.getResponseDescription());
    Serial.println(cMi.status.code);
 
printParameters(configuration);
 
    //  ----------------------- FIXED SENDER RSSI -----------------------
    configuration.ADDL = 0x03;  // For transparent mode both TX and Rx are set to same address=3
    configuration.ADDH = 0x00;
    //
    configuration.CHAN = 70;  // Channel=70 (850.125MHz + 70*1MHz = 920.125MHz)
    //
    configuration.SPED.uartBaudRate = UART_BPS_9600;
    configuration.SPED.airDataRate = AIR_DATA_RATE_010_24;
    configuration.SPED.uartParity = MODE_00_8N1;
    //
    configuration.OPTION.subPacketSetting = SPS_200_00;
    configuration.OPTION.RSSIAmbientNoise = RSSI_AMBIENT_NOISE_DISABLED;
    configuration.OPTION.transmissionPower = POWER_13; // 22->13
    //
    configuration.TRANSMISSION_MODE.enableRSSI = RSSI_ENABLED; // RSSI enable
    configuration.TRANSMISSION_MODE.fixedTransmission = FT_TRANSPARENT_TRANSMISSION;// Transparent (default)
    configuration.TRANSMISSION_MODE.enableLBT = LBT_DISABLED;
    configuration.TRANSMISSION_MODE.WORPeriod = WOR_2000_011;

    // Set configuration changed and set to not hold the configuration
    
    ResponseStatus rs = e220ttl.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
    Serial.println(rs.getResponseDescription());
    Serial.println(rs.code);
 
    c = e220ttl.getConfiguration();
    // It's important get configuration pointer before all other operation
    configuration = *(Configuration*) c.data;
    Serial.println(c.status.getResponseDescription());
    Serial.println(c.status.code);
 
    printParameters(configuration);
 
    c.close(); 
}
 
void loop() {
 
}
void printParameters(struct Configuration configuration) {
    Serial.println("----------------------------------------");
 
    Serial.print(F("HEAD : "));  Serial.print(configuration.COMMAND, HEX);Serial.print(" ");Serial.print(configuration.STARTING_ADDRESS, HEX);Serial.print(" ");Serial.println(configuration.LENGHT, HEX);
    Serial.println(F(" "));
    Serial.print(F("AddH : "));  Serial.println(configuration.ADDH, HEX);
    Serial.print(F("AddL : "));  Serial.println(configuration.ADDL, HEX);
    Serial.println(F(" "));
    Serial.print(F("Chan : "));  Serial.print(configuration.CHAN, DEC); Serial.print(" -> "); Serial.println(configuration.getChannelDescription());
    Serial.println(F(" "));
    Serial.print(F("SpeedParityBit     : "));  Serial.print(configuration.SPED.uartParity, BIN);Serial.print(" -> "); Serial.println(configuration.SPED.getUARTParityDescription());
    Serial.print(F("SpeedUARTDatte     : "));  Serial.print(configuration.SPED.uartBaudRate, BIN);Serial.print(" -> "); Serial.println(configuration.SPED.getUARTBaudRateDescription());
    Serial.print(F("SpeedAirDataRate   : "));  Serial.print(configuration.SPED.airDataRate, BIN);Serial.print(" -> "); Serial.println(configuration.SPED.getAirDataRateDescription());
    Serial.println(F(" "));
    Serial.print(F("OptionSubPacketSett: "));  Serial.print(configuration.OPTION.subPacketSetting, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getSubPacketSetting());
    Serial.print(F("OptionTranPower    : "));  Serial.print(configuration.OPTION.transmissionPower, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getTransmissionPowerDescription());
    Serial.print(F("OptionRSSIAmbientNo: "));  Serial.print(configuration.OPTION.RSSIAmbientNoise, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getRSSIAmbientNoiseEnable());
    Serial.println(F(" "));
    Serial.print(F("TransModeWORPeriod : "));  Serial.print(configuration.TRANSMISSION_MODE.WORPeriod, BIN);Serial.print(" -> "); Serial.println(configuration.TRANSMISSION_MODE.getWORPeriodByParamsDescription());
    Serial.print(F("TransModeEnableLBT : "));  Serial.print(configuration.TRANSMISSION_MODE.enableLBT, BIN);Serial.print(" -> "); Serial.println(configuration.TRANSMISSION_MODE.getLBTEnableByteDescription());
    Serial.print(F("TransModeEnableRSSI: "));  Serial.print(configuration.TRANSMISSION_MODE.enableRSSI, BIN);Serial.print(" -> "); Serial.println(configuration.TRANSMISSION_MODE.getRSSIEnableByteDescription());
    Serial.print(F("TransModeFixedTrans: "));  Serial.print(configuration.TRANSMISSION_MODE.fixedTransmission, BIN);Serial.print(" -> "); Serial.println(configuration.TRANSMISSION_MODE.getFixedTransmissionDescription());
 
 
    Serial.println("----------------------------------------");
}
void printModuleInformation(struct ModuleInformation moduleInformation) {
    Serial.println("----------------------------------------");
    Serial.print(F("HEAD: "));  Serial.print(moduleInformation.COMMAND, HEX);Serial.print(" ");Serial.print(moduleInformation.STARTING_ADDRESS, HEX);Serial.print(" ");Serial.println(moduleInformation.LENGHT, DEC);
 
    Serial.print(F("Model no.: "));  Serial.println(moduleInformation.model, HEX);
    Serial.print(F("Version  : "));  Serial.println(moduleInformation.version, HEX);
    Serial.print(F("Features : "));  Serial.println(moduleInformation.features, HEX);
    Serial.println("----------------------------------------");
 
}