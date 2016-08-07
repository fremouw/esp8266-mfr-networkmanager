#ifndef P1_CONFIG_H_
#define P1_CONFIG_H_

#include <EEPROM.h>
#include <WString.h>

#define CONFIG_VERSION "mp1"

class Config {
public:
    Config();
    ~Config();

    void clear();
    void loadConfig();
    void saveConfig();

    void getSSID( String& ssid );
    void getPassword( String& password );
    void setSSID( const String& ssid );
    void setPassword( const String& password );

    void getP1Baudrate( uint32_t& baudrate );
    void getP1InverseLogic( bool& inverseLogic );
    void setP1Baudrate( uint32_t baudrate );
    void setP1InverseLogic( bool inverseLogic );

private:
    struct ConfigStruct {
        char id[4];
        char ssid[32];
        char password[64];
        uint32_t baudrate;
        bool inverseLogic;
    } config = {
        CONFIG_VERSION,
        "",
        "",
        115200,
        false
    };
};

#endif // P1_CONFIG_H_
