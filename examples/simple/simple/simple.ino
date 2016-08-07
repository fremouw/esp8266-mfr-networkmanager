#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <Hash.h>
#include <networkmanager.h>

Config config;
NetworkManager networkManager(config);

void setup() {
    Serial.begin(115200);
    Serial.println("\r\nBooting...");

}

void loop() {

}
