#include "networkmanager.h"
#include <ip_addr.h>
#include <Hash.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

NetworkManager::NetworkManager(Config& config) : config(config) {
    currentState = WantsToIdle;
    server = nullptr;
    dnsServer = nullptr;
    events = nullptr;
    networkJoinlastMessage = 0;
}

NetworkManager::~NetworkManager() {
    if( dnsServer != nullptr ) {
        dnsServer->stop();

        delete dnsServer;
    }

    if( events != nullptr )
        delete events;

    if( server != nullptr )
        delete server;
}

bool NetworkManager::isInSetupMode() {
    return WiFi.getMode() == WIFI_AP_STA;
}

bool NetworkManager::isConnected() {
    return WiFi.isConnected();
}

void NetworkManager::getAPSSID(String& ssid) {
    ssid = apSsid;
}

void NetworkManager::getAPPassword(String& password) {
    password = apPassword;
}

void NetworkManager::getCurrentState(state& state) {
    switch(currentState) {
        case IsJoiningNetwork:
            state = State_IsConnectingToNetwork;
            break;
        case IsJoiningNewNetwork:
            state = State_TryConnectingToNewNetwork;
            break;
        default:
            state = WiFi.isConnected() ? State_Connected : State_Disconnected;
            break;
    }
}

void NetworkManager::setup(String& ssid, String& password) {
    Serial.println("NetworkManager::setup");

	SPIFFS.begin();

    server = new AsyncWebServer(80);

    // WiFi.softAPdisconnect(false);
    WiFi.disconnect();

    if(ssid.length() > 0) {
        Serial.print("Joining network ");
        Serial.println(ssid);

        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid.c_str(), password.c_str());

        server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(SPIFFS, "/fs/index.html");
        });

        server->on("/config", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(SPIFFS, "/fs/config/index.html");
        });

        server->onNotFound([](AsyncWebServerRequest *request) {
            request->send(404, "text/html", "404");
        });

        currentState = IsJoiningNetwork;
    } else {
        Serial.println("No saved network configuration found, starting in AP mode.");

        WiFi.mode(WIFI_AP_STA);

        generateSSID(apSsid);

        apPassword = "sparkfun";

        WiFi.softAP(apSsid.c_str(), apPassword.c_str());

        delay(100);

        dnsServer = new DNSServer();

        dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
        dnsServer->start(53, "*", WiFi.softAPIP());

        server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(SPIFFS, "/fs/config/index.html");
        });

        server->onNotFound([](AsyncWebServerRequest *request) {
            request->send(SPIFFS, "/fs/config/index.html");
        });

        Serial.print("Created AP with SSID: ");
        Serial.println(ssid);

        Serial.print("IP Address: ");
        Serial.println(WiFi.softAPIP());
    }

    events = new AsyncEventSource("/api/events");

    server->on("/api/system/ping", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json",  "{\"msg\":\"pong\"}" );

        response->addHeader("Access-Control-Allow-Origin", "*");

        request->send(response);
    });

    server->on("/api/system/hostname", HTTP_GET, [](AsyncWebServerRequest *request) {
        StaticJsonBuffer<512> jsonBuffer;
        JsonObject& root = jsonBuffer.createObject();

        String fullHostname = WiFi.hostname();
        fullHostname.concat(".local");

        root["hostname"] = fullHostname;

        String jsonResponse;
        root.printTo(jsonResponse);

        request->send(200, "application/json",  jsonResponse );
    });

    server->on("/api/system/restart", HTTP_GET, [this](AsyncWebServerRequest *request) {

        this->currentState = WantsToRestart;

        request->send(200, "application/json",  "{\"msg\":\"ok\"}" );
    });

    server->on("/api/system/network/list", HTTP_GET, [this](AsyncWebServerRequest *request) {
        Serial.println("/api/network/list");

        this->currentState = WantsToListNetworks;

        request->send(200, "application/json",  "{\"rv\":0}" );
    });

	server->on("/api/system/network/join", HTTP_POST, [this](AsyncWebServerRequest *request) {
        Serial.println("/api/network/join");

        AsyncWebServerResponse* response = nullptr;
        AsyncWebParameter* p = request->getParam("body", true, false);
        if(p != nullptr) {
            StaticJsonBuffer<200> jsonBuffer;
            JsonObject& root = jsonBuffer.parseObject(p->value());
            if(root.success()) {

                if(root.containsKey("ssid")) {
                    newSsid = root["ssid"].asString();
                }

                if(root.containsKey("password")) {
                    String password = root["password"];

                    if(password.length() > 0)
                        newPassword = password;
                }

                this->currentState = WantsToJoinNewNetwork;

                response = request->beginResponse(200, "application/json",  "{\"rv\":0}");
            }
        }

        if(response == nullptr)
            response = request->beginResponse(200, "application/json",  "{\"rv\":1}");

        request->send(response);
	});

    events->onConnect([](AsyncEventSourceClient *client){
        if(client->lastId()) {
            Serial.printf("Client reconnected! Last message ID that it got is: %u\rs\n", client->lastId());
        }

        //send event with message "hello!", id current millis
        // and set reconnect delay to 1 second
        client->send("hello!", NULL, millis(), 500);
    });

    server->addHandler(events);

    server->serveStatic("/fs", SPIFFS, "/fs");

    server->begin();
}

void NetworkManager::on(const char *uri, WebRequestMethod method, ArRequestHandlerFunction function) {
    server->on(uri, method, function);

    // server->on(const char *uri, WebRequestMethod method, ArRequestHandlerFunction )    // server->on("/api/system/network/list", HTTP_GET, [this](AsyncWebServerRequest *request) {
}

void NetworkManager::loop() {
    if(dnsServer != nullptr)
        dnsServer->processNextRequest();

    switch(currentState) {
        case IsJoiningNetwork: {
            long now = millis();
            if( WiFi.isConnected() ) {
                Serial.println("Joined network...");

                Serial.print("IP address: ");
                Serial.println(WiFi.localIP());

                currentState = WantsToIdle;
            } else if ( now - networkJoinlastMessage > 15000 ) {
                Serial.println("Unable to join network.");

                delay(3000);

                ESP.restart();
            }
        }
        break;
        case WantsToRestart: {
            Serial.println("Rebooting...");

            delay(3000);

            ESP.restart();

            currentState = WantsToIdle;
        }
        break;
        case WantsToListNetworks: {
            String networks;
            scanForNetworks(networks);

            Serial.println(networks);

            events->send(networks.c_str(), "listOfNetworks");

            currentState = WantsToIdle;
        }
        break;
        case WantsToJoinNewNetwork: {
            Serial.println("Trying to join new network...");
            Serial.println(newSsid);
            Serial.println(newPassword);
            WiFi.begin(newSsid.c_str(), newPassword.c_str());

            networkJoinlastMessage = millis();
            currentState = IsJoiningNewNetwork;
        }
        break;
        case IsJoiningNewNetwork: {
            long now = millis();
            if( WiFi.isConnected() ) {
                Serial.println("Joined network...");

                events->send("{\"rv\": 0}", "joinedNetwork");

                config.setSSID( newSsid );
                if(newPassword.length() > 0)
                    config.setPassword( newPassword );

                Serial.println(newSsid);
                Serial.println(newPassword);

                config.saveConfig();

                delay(5000);

                currentState = WantsToRestart;
            } else if ( now - networkJoinlastMessage > 30000 ) {
                Serial.println("Connection Failed!...");

                events->send("{\"rv\": -1}", "joinedNetwork");

                currentState = WantsToIdle;
            }
        }
        break;
    };
}

// Private
void NetworkManager::generateSSID(String& ssid) {
    String _ssid = "ESP1-";

    _ssid.concat(ESP.getChipId());

    ssid = _ssid;
}

void NetworkManager::scanForNetworks(String& networks) {
    Serial.println("NetworkManager:scanForNetworks()");

    StaticJsonBuffer<3072> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    JsonArray& data = root.createNestedArray("networks");

    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks(false, true);
    if ( n > 0 ) {
        // sort by RSSI
        int indices[n];

        for (int i = 0; i < n; i++) {
            indices[i] = i;
        }

        for ( int i = 0; i < n; i++ ) {
            for ( int j = i + 1; j < n; j++ ) {
                if ( WiFi.RSSI(indices[j]) > WiFi.RSSI( indices[i] ) ) {
                  std::swap(indices[i], indices[j]);
                }
            }
        }

        Serial.print(n);
        Serial.println(" networks found");

        for ( int i = 0; i < n; ++i ) {
            // 00: (RSSI)[BSSID][hidden] SSID [channel] [encryption]
            // 01: (-48) [C2:3F:0E:7C:39:67] [0] Aap. (Guest) [02] [CCMP]
            // 02: (-48) [54:FA:3E:61:10:E1] [0] Ole op de Chasse [13] [CCMP]
            JsonObject& item = jsonBuffer.createObject();
            item["ssid"] = WiFi.SSID(indices[i]);
            item["bssid"] = WiFi.BSSIDstr(indices[i]);
            item["encryption"] = encryptionTypeString( WiFi.encryptionType( indices[i] ) );

            data.add(item);
            delay(10);
        }
    }

    root.printTo(networks);
}

int8_t NetworkManager::getWifiQuality() {
    int32_t dbm = WiFi.RSSI();
    if(dbm <= -100) {
        return 0;
    } else if(dbm >= -50) {
        return 100;
    } else {
        return 2 * (dbm + 100);
    }
}
