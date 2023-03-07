//
// Created by david on 11/10/2022.
//

#ifndef NANOLUX_WEBSERVER_H
#define NANOLUX_WEBSERVER_H

#include "LITTLEFS.h"

/*
 * WifiWebServer Library Configuration
 */
#define DEBUG_WIFI_WEBSERVER_PORT   Serial

#define _WIFI_LOGLEVEL_     4

// Use standard WiFi
#define USE_WIFI_NINA       false
#define USE_WIFI_CUSTOM     false

#define SHIELD_TYPE         "ESP WiFi using WiFi Library"
#define BOARD_TYPE          "ESP32"

#include <WiFiWebServer.h>

const char* ssid = "AUDIOLUX";
const char* pass = "12345678";

constexpr int MAX_NETWORKS = 15;
constexpr int END_OF_DATA = 9999;
typedef struct {
    String  SSID;
    int32_t RSSI;
    uint8_t EncryptionType;
} WiFiNetwork;

WiFiNetwork AvailableNetworks[MAX_NETWORKS];

constexpr int HTTP_OK = 200;
const char* CONTENT_TYPE = "application/json";


/*
 * File system
 */
inline void initialize_file_system() {
    Serial.println(F("Initializing FS..."));
    if (LITTLEFS.begin()) {
        Serial.println(F("done."));
    } else {
        Serial.println(F("fail."));
    }
}

/*
 * Web Server
 */
typedef void (*APIHandler)();

typedef struct {
    String path;
    APIHandler handler;
} APIHook;


WiFiWebServer webServer(80);


inline void register_web_paths(fs::FS &fs, const char * dirname, uint8_t levels) {
    Serial.printf("Scanning path: %s\r\n", dirname);
    File root = fs.open(dirname);
    if (!root) {
        Serial.println("- failed to open root directory");
        return;
    }
    if (!root.isDirectory()) {
        Serial.println(" - / does not seem to be a directory");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            Serial.print("Scanning directory: ");
            Serial.println(file.name());
            if (levels) {
                register_web_paths(fs, file.name(), levels-1);
            }
        } else {
            Serial.print("  Registering path: ");
            Serial.println(file.name());
            webServer.serveStatic(file.name(), fs, file.name());
        }
        file = root.openNextFile();
    }
}

/*
 * Network configuration
 */
inline void scan_ssids() {
    Serial.println("[*] Scanning WiFi network");
    const int n = WiFi.scanNetworks();
    Serial.print("[*] Scan done. ");
    Serial.print(n);
    Serial.println(" networks found.");


    if (n == 0) {
        AvailableNetworks[0].RSSI = END_OF_DATA;
    }
    else {
        const int network_count = min(n, MAX_NETWORKS);
        for (int i = 0; i < network_count; ++i) {
            AvailableNetworks[i].SSID = String(WiFi.SSID(i));
            AvailableNetworks[i].RSSI = WiFi.RSSI(i);
            AvailableNetworks[i].EncryptionType = WiFi.encryptionType(i);

            // Give some time to the hardware to get the next network ready.
            delay(10);
        }

        Serial.print("[*] Registered networks: ");
        for(int i = 0; i< network_count; ++i) {
            Serial.print("SSID:");
            Serial.print(AvailableNetworks[i].SSID);
            Serial.print("\t\t RSSI:");
            Serial.print(AvailableNetworks[i].RSSI);
            Serial.print("\t Type:");
            Serial.println(AvailableNetworks[i].EncryptionType);
        }

        if (network_count < MAX_NETWORKS) {
            AvailableNetworks[network_count].RSSI = END_OF_DATA;
        }
    }
}


inline int check_operating_mode()
{
    return 0;
}

inline void serve_wifi_list() {
    using namespace ARDUINOJSON_NAMESPACE;

    scan_ssids();

    StaticJsonDocument<1024> doc;
    int wifi_number = 0;
    while (wifi_number < MAX_NETWORKS && AvailableNetworks[wifi_number].RSSI != END_OF_DATA) {
        JsonObject wifi = doc.createNestedObject();
        wifi["ssid"] = AvailableNetworks[wifi_number].SSID;
        wifi["rssi"] = AvailableNetworks[wifi_number].RSSI;
        wifi["lock"] = AvailableNetworks[wifi_number].EncryptionType != WIFI_AUTH_OPEN;
        wifi_number++;
    }

    String wifi_list;
    serializeJson(doc, wifi_list);

    Serial.println(F("Sending networks"));
    Serial.println(wifi_list);
    webServer.send(HTTP_OK, CONTENT_TYPE, wifi_list);
}


inline void initialize_web_server(APIHook api_hooks[], int hook_count) {
    initialize_file_system();

    int operating_mode = check_operating_mode();

    WiFi.mode(WIFI_MODE_APSTA);
    WiFi.softAP(ssid, pass);
    delay(1000);

    IPAddress ip = WiFi.softAPIP();

    // API
    for (int i = 0; i < hook_count; i++) {
        webServer.on(api_hooks[i].path, api_hooks[i].handler);
    }

    // Add internal APi endpoints
    webServer.on("/api/wifis", serve_wifi_list);
    // webServer.on("/api/wifi", handle_wifi_request);

    // Web App
    Serial.print(F("Registering Web App files."));
    register_web_paths(LITTLEFS, "/", 2);
    webServer.serveStatic("/", LITTLEFS, "/index.html");

    webServer.enableCrossOrigin();
    webServer.begin();

    Serial.print(F("To interact with the AudioLux open a browser to http://"));
    Serial.println(ip);
}

inline void handle_web_requests() {
    webServer.handleClient();
}

#endif //NANOLUX_WEBSERVER_H
