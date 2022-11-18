//
// Created by david on 11/10/2022.
//

#ifndef NANOLUX_WEBSERVER_H
#define NANOLUX_WEBSERVER_H

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

#endif //NANOLUX_WEBSERVER_H
