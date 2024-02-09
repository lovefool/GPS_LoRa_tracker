/***************************************************
GPS_LoRa_tracker_Config_Sender.ino
E220 config program
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

2024.01.30  Rev.0.1
Author : Jay Teramoto
https://github.com/lovefool/GPS_LoRa_tracker/tree/main
***************************************************/

#include "EByte_LoRa_E220_library.h" // LoRa_E220.h is original
#include <SoftwareSerial.h>

SoftwareSerial swSer(D7,D8);
LoRa_E220 e220ttl(&swSer, D3, D5, D6); // AUX M0 M1

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
 
   Serial.println("1. Get config"); 
    ResponseStructContainer c;
    c = e220ttl.getConfiguration();
    // It's important get configuration pointer before all other operation
    Configuration configuration = *(Configuration*) c.data;
    Serial.println(c.status.getResponseDescription());
    Serial.println(c.status.code);
 
    printParameters(configuration);

    Serial.println("2. Get module information"); 
    ResponseStructContainer cMi;
    cMi = e220ttl.getModuleInformation();
    // It's important get information pointer before all other operation
    ModuleInformation mi = *(ModuleInformation*)cMi.data;
 
    Serial.println(cMi.status.getResponseDescription());
    Serial.println(cMi.status.code);


    Serial.println("3. Print module information"); 
    printModuleInformation(mi);

    Serial.println("4. Set new param"); 
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
    configuration.OPTION.transmissionPower = POWER_13;
    //
    configuration.TRANSMISSION_MODE.enableRSSI = RSSI_ENABLED; // RSSI enable
    configuration.TRANSMISSION_MODE.fixedTransmission = FT_TRANSPARENT_TRANSMISSION;// Transparent (default)
    configuration.TRANSMISSION_MODE.enableLBT = LBT_DISABLED;
    configuration.TRANSMISSION_MODE.WORPeriod = WOR_2000_011;

    // Set configuration changed and set to not hold the configuration

    Serial.println("5. Set configuration"); 
    ResponseStatus rs = e220ttl.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
    Serial.println(rs.getResponseDescription());
    Serial.println(rs.code);

    Serial.println("6. Verify configuration"); 

    c = e220ttl.getConfiguration();
    // It's important get configuration pointer before all other operation
    configuration = *(Configuration*) c.data;
    Serial.println(c.status.getResponseDescription());
    Serial.println(c.status.code);
 
    printParameters(configuration);
 
    c.close(); 
    cMi.close();}
 
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