#include "config.h"
#include <ESP8266WiFi.h>

#define CONFIG_START 3

const int eepromLength = 512;

Config::Config() {
}

Config::~Config() {

}

void Config::clear() {
    for ( int i = 0; i < eepromLength; ++i )
        EEPROM.write(i, 0x0);

    EEPROM.commit();

    EEPROM.end();
}

void Config::loadConfig() {
    EEPROM.begin(eepromLength);

    if ( EEPROM.read( 0 ) == CONFIG_VERSION[0] &&
         EEPROM.read( 1 ) == CONFIG_VERSION[1] &&
         EEPROM.read( 2 ) == CONFIG_VERSION[2] ) {

        for ( int t = 0; t < sizeof( config ); t++ )
            *((char*)&config + t) = EEPROM.read( CONFIG_START + t );

    } else {
        Serial.println("No Config.");
    }
}

void Config::saveConfig() {

    EEPROM.write( 0, CONFIG_VERSION[0] );
    EEPROM.write( 1, CONFIG_VERSION[1] );
    EEPROM.write( 2, CONFIG_VERSION[2] );

    for ( int j = 0; j < sizeof( config ); j++ )
        EEPROM.write( CONFIG_START + j, *( (char*)&config + j ) );

    EEPROM.commit();

    EEPROM.end();
}

void Config::getSSID( String& ssid ) {
    ssid = config.ssid;
}

void Config::getPassword( String& password ) {
    password = config.password;
}

void Config::setSSID( const String& ssid ) {
    ssid.toCharArray( config.ssid, sizeof( config.ssid ) );
}

void Config::setPassword( const String& password ) {
    password.toCharArray( config.password, sizeof( config.password ) );
}

void Config::getP1Baudrate( uint32_t& baudrate ) {
    baudrate = config.baudrate;
}

void Config::getP1InverseLogic( bool& inverseLogic ) {
    inverseLogic = config.inverseLogic;
}

void Config::setP1Baudrate( uint32_t baudrate ) {
    config.baudrate = baudrate;
}

void Config::setP1InverseLogic( bool inverseLogic ) {
    config.inverseLogic = inverseLogic;
}
