#ifndef MFR_CONFIG_H_
#define MFR_CONFIG_H_

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


private:
        struct ConfigStruct {
                char id[4];
                char ssid[32];
                char password[64];
        } config = {
                CONFIG_VERSION,
                "",
                ""
        };
};

#endif // MFR_CONFIG_H_
