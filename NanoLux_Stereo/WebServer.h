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

char ssid[] = "AUDIOLUX";
char pass[] = "12345678";

const int MAX_NETWORKS = 15;
typedef struct {
    String SSID;
    int32_t RSSI;
    uint8_t EncryptionType;
} WiFiNetwork;

WiFiNetwork AvailableNetworks[MAX_NETWORKS];


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


int check_operating_mode() {



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
//    webServer.on("/blank", handle_blank);
//    webServer.on("/trail", handle_trail);
//    webServer.on("/solid", handle_solid);
//    webServer.on("/confetti", handle_confetti);
//    webServer.on("/vbar", handle_vol_bar);

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
    }
}

char** scanSSIDs() {
    int n = WiFi.scanNetworks();

    if (n == 0) {
        return NULL;
    } else {
        int networkCount = min(n, MAX_NETWORKS);
        for (int i = 0; i < networkCount; ++i) {
            AvailableNetworks[i].SSID = String(WiFi.SSID(i));
            AvailableNetworks[i].RSSI =  WiFi.RSSI(i);
            AvailableNetworks[i].EncryptionType = WiFi.encryptionType(i);

            // Give some time to the hardware to get the next network ready.
            delay(10);
        }

        // Add end of data marker.
        if (networkCount < MAX_NETWORKS) {
            AvailableNetworks[networkCount].RSSI = -1;
        }
    }
}

#endif //NANOLUX_WEBSERVER_H
