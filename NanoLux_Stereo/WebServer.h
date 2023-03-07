//
// Created by david on 11/10/2022.
//

#ifndef NANOLUX_WEBSERVER_H
#define NANOLUX_WEBSERVER_H

#include <math.h>
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
typedef struct {
    String SSID;
    int32_t RSSI;
    uint8_t EncryptionType;
} WiFiNetwork;

WiFiNetwork AvailableNetworks[MAX_NETWORKS];

constexpr int HTTP_OK = 200;
const char* CONTENT_TYPE = "application/json";


/*
 * File system
 */
void initialize_file_system() {
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

void register_web_paths(fs::FS &fs, const char * dirname, uint8_t levels) {
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
void scan_ssids() {
    WiFi.mode(WIFI_STA);
    const int n = WiFi.scanNetworks();

    if (n == 0) {
        AvailableNetworks[0].RSSI = -1;;
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

        // Add end of data marker.
        if (network_count < MAX_NETWORKS) {
            AvailableNetworks[network_count].RSSI = -1;
        }
    }
}


int check_operating_mode() {



}

void serve_wifi_list() {
    using namespace ARDUINOJSON_NAMESPACE;

    Serial.println(F("Scanning networks."));
    scan_ssids();

    Serial.println(F("Creating JSON response."));
    StaticJsonDocument<1024> doc;
    int wifi_number = 0;
    while (wifi_number < MAX_NETWORKS && AvailableNetworks[wifi_number].RSSI >= 0) {
        JsonObject wifi = doc.createNestedObject();
        wifi["ssid"] = AvailableNetworks[wifi_number].SSID;
        wifi["rssi"] = AvailableNetworks[wifi_number].RSSI;
        wifi["lock"] = AvailableNetworks[wifi_number].EncryptionType != WIFI_AUTH_OPEN;
        wifi_number++;
    }

    Serial.print(F("Found "));
    Serial.print(wifi_number);
    Serial.println(F(" networks."));

    String wifi_list;
    serializeJson(doc, wifi_list);

    Serial.println(F("Sending networks"));
    Serial.println(wifi_list);
    webServer.send(HTTP_OK, CONTENT_TYPE, wifi_list);
}



void initialize_web_server(APIHook api_hooks[], int hook_count) {
    initialize_file_system();

    int operating_mode = check_operating_mode();

    WiFi.mode(WIFI_AP);
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
    webServer.serveStatic("/", LITTLEFS, "/www/index.html");

    webServer.begin();

    Serial.print(F("To interact with the AudioLux open a browser to http://"));
    Serial.println(ip);
}

void handle_web_requests() {
    webServer.handleClient();
}

String get_encryption_type(wifi_auth_mode_t encryptionType) {
    switch (encryptionType) {
        case (WIFI_AUTH_OPEN):
            return "Open";
        case (WIFI_AUTH_WEP):
            return "WEP";
        case (WIFI_AUTH_WPA_PSK):
            return "WPA_PSK";
        case (WIFI_AUTH_WPA2_PSK):
            return "WPA2_PSK";
        case (WIFI_AUTH_WPA_WPA2_PSK):
            return "WPA_WPA2_PSK";
        case (WIFI_AUTH_WPA2_ENTERPRISE):
            return "WPA2_ENTERPRISE";
        default:
            return "UNKNOWN_SECURITY_TYPE";
    }
}

#endif //NANOLUX_WEBSERVER_H
