#include <WString.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <mfr_configuration.h>

#ifndef MFR_NETWORKMANAGER_H_
#define MFR_NETWORKMANAGER_H_

class NetworkManager {
public:
        static const String kSSIDKey;
        static const String kPasswordKey;

        NetworkManager(mfr::Configuration& config);
        ~NetworkManager();

        void setup(String& ssid, String& password);
        void loop();
        bool isInSetupMode();
        bool isConnected();
        void on(const char *uri, WebRequestMethod method, ArRequestHandlerFunction function);

        void getAPSSID(String& ssid);
        void getAPPassword(String& password);

        static int8_t getWifiQuality();

        enum state {
                State_Unknown = 0,
                State_IsConnectingToNetwork,
                State_TryConnectingToNewNetwork,
                State_Connected,
                State_Disconnected
        };

        void getCurrentState(state& state);

private:
        mfr::Configuration& config;
        AsyncWebServer *server;
        DNSServer      *dnsServer;
        AsyncEventSource *events;

        enum internalState {
                WantsToIdle = 0,
                WantsToRestart,
                WantsToListNetworks,
                WantsToJoinNewNetwork,
                IsJoiningNetwork,
                IsJoiningNewNetwork,
        } currentState;

        long networkJoinlastMessage;

        String apSsid;
        String apPassword = "welcome";

        String newSsid;
        String newPassword;

        const String encryptionTypeString(uint8_t authmode) {
                switch(authmode) {
                case ENC_TYPE_NONE:
                        return "NONE";
                case ENC_TYPE_WEP:
                        return "WEP";
                case ENC_TYPE_TKIP:
                        return "TKIP";
                case ENC_TYPE_CCMP:
                        return "CCMP";
                case ENC_TYPE_AUTO:
                        return "AUTO";
                default:
                        return "?";
                }
        };

        void scanForNetworks(String& networks);
        void generateSSID(String& ssid);
};

#endif // MFR_NETWORKMANAGER_H_
