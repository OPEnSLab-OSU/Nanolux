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

char ssid[] = "AUDIOLUX";
char pass[] = "12345678";


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

void initialize_web_server(APIHook api_hooks[], int hook_count) {
    initialize_file_system();

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
    webServer.serveStatic("/", LITTLEFS, "/index.html");

    webServer.begin();

    Serial.print(F("To interact with the AudioLux open a browser to http://"));
    Serial.println(ip);
}

void handle_web_requests() {
    webServer.handleClient();
}

#endif //NANOLUX_WEBSERVER_H
