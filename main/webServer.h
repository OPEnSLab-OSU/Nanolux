#pragma once

#include <ESPmDNS.h>
#include "LITTLEFS.h"
#include "BluetoothSerial.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <AsyncEventSource.h>
#include <AsyncJson.h>
#include <AsyncWebSocket.h>
#include <AsyncWebSynchronization.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>
#include <StringArray.h>
#include <WebAuthentication.h>
#include <WebHandlerImpl.h>
#include <WebResponseImpl.h>

// /*
//  * Bluetooth Configuration
//  * We don't use Bluetooth, but remove this and the initialization of
//  * BluetoothSerial on startup, and the WiFi goes berserk. Don't know why. 
//  */
// #if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
// #error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
// #endif
// BluetoothSerial SerialBT;



// /*
//  * WifiWebServer Library Configuration
//  */
// #define DEBUG_WIFI_WEBSERVER_PORT   Serial
//
// #define _WIFI_LOGLEVEL_     3
//
//  // Use standard WiFi
// #define USE_WIFI_NINA       false
// #define USE_WIFI_CUSTOM     false
//
// #define SHIELD_TYPE         "ESP WiFi using WiFi Library"
// #define BOARD_TYPE          "ESP32"
//
// #include <WiFiWebServer.h>

constexpr int MAX_WIFI_CONNECT_WAIT = 100;
constexpr int MAX_NETWORKS = 15;
constexpr int END_OF_DATA = 9999;
typedef struct {
    String  SSID;
    int32_t RSSI;
    uint8_t EncryptionType;
} WiFiNetwork;

WiFiNetwork available_networks[MAX_NETWORKS];
static WiFiNetwork* currently_joined_wifi;


const char* DEFAULT_HOSTNAME = "AudioLuxOne";


constexpr int HTTP_OK = 200;
constexpr int HTTP_BAD_REQUEST = 400;
constexpr int HTTP_UNAUTHORIZED = 401;
constexpr int HTTP_METHOD_NOT_ALLOWED = 405;
constexpr int HTTP_UNPROCESSABLE = 422;
constexpr int HTTP_UNAVAILABLE = 503;

const char* CONTENT_JSON = "application/json";
const char* CONTENT_TEXT = "text/plain";

const char* ap_ssid = "AUDIOLUX";
const char* ap_password = "12345678";

constexpr int MAX_HOSTNAME_LEN = 64;
static char hostname[MAX_HOSTNAME_LEN];

const char* SETTINGS_FILE = "/settings.json";
static StaticJsonDocument<384> settings;

static String http_response;

const char* URL_FILE = "/assets/url.json";

volatile bool server_unavailable = false;


/*
 * File system
 */
inline void initialize_file_system() {
    Serial.print(F("Initializing FS..."));
    if (LITTLEFS.begin()) {
        Serial.println(F("done."));
    }
    else {
        Serial.println(F("fail."));
    }
}


/*
 * Settings Management
 */
inline void save_settings() {
    File saved_settings = LITTLEFS.open(SETTINGS_FILE, "w");
    if (saved_settings) {
        serializeJson(settings, saved_settings);
        Serial.println("Saving settings:");
        serializeJsonPretty(settings, Serial);
        Serial.println(" ");
    }
    else {
        Serial.println("Unable to save settings file.");
    }
}


inline void load_settings() {
    Serial.println("Checking if settings are available.");

    File saved_settings = LITTLEFS.open(SETTINGS_FILE, "r");
    if (saved_settings) {
        const DeserializationError error = deserializeJson(settings, saved_settings);
        if (!error) {
            Serial.println("Settings loaded:");
            serializeJsonPretty(settings, Serial);
            Serial.println(" ");
            return;
        }

        Serial.print("Error loading saved file: ");
        Serial.println(error.c_str());
    }

    Serial.println("Unable to load settings. Will use defaults (re)create file.");

    JsonObject wifi = settings.createNestedObject("wifi");
    wifi["ssid"] = NULL;
    wifi["key"] = NULL;
    wifi["lock"] = false;
    settings["hostname"] = DEFAULT_HOSTNAME;

    save_settings();
}


/*
 * Web Server API handlers
 */
typedef struct {
    String path;
    ArRequestHandlerFunction handler;
} APIGetHook;

// typedef struct {
//     String path;
//     ArRequestHandlerFunction request_handler;
//     ArBodyHandlerFunction body_handler;
// } APIPutHook;

typedef struct {
    String path;
    ArJsonRequestHandlerFunction request_handler;
} APIPutHook;


AsyncWebServer webServer(80);


inline void register_web_paths(fs::FS& fs, const char* dirname, uint8_t levels) {
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
                register_web_paths(fs, file.name(), levels - 1);
            }
        }
        else {
            Serial.print("  Registering path: ");
            Serial.println(file.name());
            webServer.serveStatic(file.name(), fs, file.name());
        }
        file = root.openNextFile();
    }
}


inline const String& build_response(const boolean success, const char* message, const char* details) {
    http_response = "{ \"success\": " + success;
    if (message != nullptr) {
        http_response += http_response + ", \"message\": " + message;
    }
    if (details != nullptr) {
        http_response += http_response + ", \"details\": " + details;
    }
    http_response += "}";

    return http_response;
}


/*
 * Network configuration
 */
inline void scan_ssids() {
    static int long_scan_count = 0;
    const int MAX_SCAN_ITERATIONS = 2;

    const int n = WiFi.scanComplete();
    if (n == WIFI_SCAN_FAILED) {
        WiFi.scanNetworks(true);
        long_scan_count = 0;
    }
    else if (n == WIFI_SCAN_RUNNING) {
        available_networks[0].RSSI = END_OF_DATA;
        long_scan_count++;

        if (long_scan_count >= MAX_SCAN_ITERATIONS) {
            // This scan has run for a while. Cancel it and start a new one.
            WiFi.scanDelete();
            WiFi.scanNetworks(true);
            long_scan_count = 0;
        }
    }
    else if (n == 0) {
        available_networks[0].RSSI = END_OF_DATA;
        long_scan_count = 0;
    }
    else if (n > 0) {
        const int network_count = min(n, MAX_NETWORKS);
        for (int i = 0; i < network_count; ++i) {
            available_networks[i].SSID = String(WiFi.SSID(i));
            available_networks[i].RSSI = WiFi.RSSI(i);
            available_networks[i].EncryptionType = WiFi.encryptionType(i);
        }

        if (network_count < MAX_NETWORKS) {
            available_networks[network_count].RSSI = END_OF_DATA;
        }

        WiFi.scanDelete();
        if (WiFi.scanComplete() == WIFI_SCAN_FAILED) {
            WiFi.scanNetworks(true);
        }
        long_scan_count = 0;
    }
}


inline bool initiate_wifi_connection(const char* ssid, const char* key) {
    server_unavailable = true;

    // Stop any pending WiFi scans.
    WiFi.scanDelete();

    // Drop the current connection, if any.
    WiFi.disconnect();
    delay(100);

    WiFi.begin(ssid, key);
    int wait_count = 0;
    while (WiFi.status() != WL_CONNECTED && wait_count < MAX_WIFI_CONNECT_WAIT) {
        delay(500);
        ++wait_count;
    }
    server_unavailable = false;
    
    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }
    
    WiFi.disconnect();
    delay(100);
    return false;
}


inline void initialize_mdns()
{
    // We are connected. Setup mDNS
    if (MDNS.begin(hostname)) {
        Serial.print("mDNS connected. The AudioLux can be reached at ");
        Serial.print(hostname);
        Serial.println(".local");
    }
    else {
        Serial.println("Unable to setup mDNS");
    }
}

/*
 * Wifi Management
 */
inline boolean join_wifi(const char* ssid, const char* key) {
    Serial.print("Trying to join network ");
    Serial.println(ssid);

    WiFi.setHostname(hostname);

    if (!initiate_wifi_connection(ssid, key)) {
        // Try to join the previous network, if any.
        if (settings["wifi"]["ssid"] != NULL) {
            initiate_wifi_connection(settings["wifi"]["ssid"], settings["wifi"]["key"]);
        }

        Serial.println("Unable to join network.");
        return false;
    }
    Serial.println("Network joined");

    initialize_mdns();
    return true;
}


/*
 * Wifi API handling
 */
inline void serve_wifi_list(AsyncWebServerRequest* request) {
    if (server_unavailable) {
        request->send(HTTP_UNAVAILABLE);
        return;
    }

    using namespace ARDUINOJSON_NAMESPACE;

    scan_ssids();

    StaticJsonDocument<1024> json_list;
    int wifi_number = 0;
    while (wifi_number < MAX_NETWORKS && available_networks[wifi_number].RSSI != END_OF_DATA) {
        JsonObject wifi = json_list.createNestedObject();
        wifi["ssid"] = available_networks[wifi_number].SSID;
        wifi["rssi"] = available_networks[wifi_number].RSSI;
        wifi["lock"] = available_networks[wifi_number].EncryptionType != WIFI_AUTH_OPEN;
        wifi_number++;
    }

    String wifi_list;
    serializeJson(json_list, wifi_list);

    Serial.println(F("Sending networks"));
    Serial.println(wifi_list);
    request->send(HTTP_OK, CONTENT_JSON, wifi_list);
}


inline void handle_wifi_put_request(AsyncWebServerRequest* request, JsonVariant& json) {
    if (request->method() == HTTP_PUT) {
        const JsonObject& payload = json.as<JsonObject>();

        int status = HTTP_OK;

        boolean joined = false;
        if (payload["ssid"] == nullptr) {
            Serial.println("/api/wifi: Forgetting current network.");
            joined = true;
        }
        else {
            Serial.println("/api/wifi: Joining network.");
            joined = join_wifi(payload["ssid"], payload["key"]);
        }

        // int response_status;
        // String message;
        // if (joined) {
        //     Serial.println("Joined (or forgot) network.");
        //     settings["wifi"]["ssid"] = payload["ssid"];
        //     settings["wifi"]["key"] = payload["key"];
        //     settings["wifi"]["lock"] = payload["key"] == nullptr ? false : true;
        //     save_settings();
        //     response_status = HTTP_OK;
        //     message = "Joined (or forgot) network.";
        // }
        // else {
        //     Serial.println("Unable to join network.");
        //     response_status = HTTP_UNAUTHORIZED;
        //     message = "Unable to join network.";
        // }
        // request->send(response_status, CONTENT_JSON, build_response(joined, message.c_str(), nullptr));
        request->send(HTTP_OK, CONTENT_JSON, build_response(true, "Join initiated.", nullptr));
    } else {
        request->send(HTTP_METHOD_NOT_ALLOWED);
    }
}


inline void handle_wifi_get_request(AsyncWebServerRequest* request) {
    const char* ssid = settings["wifi"]["ssid"];

    const String wifi = ssid == NULL ? String("null") : String("\"") + String(ssid) + String("\"");
    const bool connected = ssid == NULL ? false : (WiFi.status() == WL_CONNECTED);

    // const char* ssid = settings["wifi"]["ssid"];
    // const String wifi = ssid == NULL ? String("null"): String("\"") + String(ssid) + String("\"");
    const String response = String("{ \"ssid\": ") + String(wifi) + String(", connected: ") + connected +  String(" }");

    Serial.println("Sending current wifi: ");
    Serial.println(response);
    request->send(HTTP_OK, CONTENT_JSON, response);

}


/*
 * Hostname API handling
 */
inline void handle_hostname_put_request(AsyncWebServerRequest* request, JsonVariant& json) {
    if (server_unavailable) {
        request->send(HTTP_UNAVAILABLE);
        return;
    }

    if (request->method() == HTTP_PUT) {
        const JsonObject& payload = json.as<JsonObject>();

        int status = HTTP_OK;

        settings["hostname"] = payload["hostname"];
        save_settings();
        Serial.println("Sending response back.");
        request->send(HTTP_OK, 
            CONTENT_TEXT, 
            build_response(true, "New hostname saved.", nullptr));
    }
}

inline void handle_hostname_get_request(AsyncWebServerRequest* request) {
    if (server_unavailable) {
        request->send(HTTP_UNAVAILABLE);
        return;
    }

    const char* hostname = settings["hostname"];
    const String response = String("{ \"hostname\": ") + "\"" + String(hostname) + String("\" }");

    Serial.println("Sending current hostname: ");
    Serial.println(response);
    request->send(HTTP_OK, CONTENT_JSON, response);
}


/*
 * Health ping.
 */
inline void handle_health_check(AsyncWebServerRequest* request) {
    if (server_unavailable) {
        request->send(HTTP_UNAVAILABLE);
        return;
    }

    Serial.println(F("Sending current realth response."));
    request->send(HTTP_OK);
}


inline void save_url(const String& url) {
    File saved_url = LITTLEFS.open(URL_FILE, "w");
    if (saved_url) {
        StaticJsonDocument<192> data;

        data["url"] = url;

        serializeJson(data, saved_url);
        Serial.print(url);
        Serial.println(" saved as Web App URL.");
    }
    else {
        Serial.println("Unable to save Web App URL, will default to http://192.168.4.1.");
    }
}


inline void setup_networking()
{
    // SerialBT.begin("Audiolux-BT-3");

    initialize_file_system();

    // Load saved settings. If we have an SSID, try to join the network.
    load_settings();

    strncpy(hostname, settings["hostname"], MAX_HOSTNAME_LEN);

    bool wifi_okay = false;
    if (settings["wifi"]["ssid"] != nullptr && settings["wifi"]["ssid"] != NULL) {
        const char* ssid = settings["wifi"]["ssid"];
        Serial.print("Attempting to connect to saved WiFi: ");
        Serial.println(ssid);
        wifi_okay = initiate_wifi_connection(settings["wifi"]["ssid"], settings["wifi"]["key"]);
        if (wifi_okay) {
            Serial.print("WiFi IP: ");
            Serial.println(WiFi.localIP());
            initialize_mdns();
        }
    }
    else {
        Serial.println("****");
        Serial.println("No wifi saved. AudioLux available via Access Point.");
        Serial.print("SSID:");
        Serial.print(ap_ssid);
        Serial.print(" Password:");
        Serial.println(ap_password);
        Serial.println("****");
    }

    WiFi.mode(WIFI_MODE_APSTA);
    WiFi.softAP(ap_ssid, ap_password);
    delay(1000);

    IPAddress ap_ip = WiFi.softAPIP();


    // Set up the URL that the Web App needs to talk to.
    String api_url = "http://";
    if (wifi_okay) {
        api_url += hostname;
        api_url += ".local";
    }
    else {
        api_url += ap_ip.toString();
    }
    save_url(api_url);
}

inline void initialize_web_server(const APIGetHook api_get_hooks[], const int get_hook_count, APIPutHook api_put_hooks[], const int put_hook_count) {
    setup_networking();

    // API
    Serial.println("Registering main APIs.");
    for (int i = 0; i < get_hook_count; i++) {
        Serial.println(api_get_hooks[i].path);
        webServer.on(api_get_hooks[i].path.c_str(), HTTP_GET, api_get_hooks[i].handler);
    }

    for (int i = 0; i < put_hook_count; i++) {
        Serial.println(api_put_hooks[i].path);
        webServer.addHandler(new AsyncCallbackJsonWebHandler(api_put_hooks[i].path.c_str(), api_put_hooks[i].request_handler));
    }
    
    // Add internal APi endpoints
    webServer.on("/api/wifis", HTTP_GET, serve_wifi_list);
    webServer.on("/api/wifi", HTTP_GET, handle_wifi_get_request);
    webServer.on("/api/hostname", HTTP_GET, handle_hostname_get_request);
    webServer.on("/api/health", HTTP_GET, handle_health_check);

    webServer.addHandler(new AsyncCallbackJsonWebHandler("/api/wifi", handle_wifi_put_request));
    webServer.addHandler(new AsyncCallbackJsonWebHandler("/api/hostname", handle_hostname_put_request));

    // Web App
    Serial.println("Registering Web App files.");
    webServer.serveStatic("/", LITTLEFS, "/").setDefaultFile("index.html");

    webServer.onNotFound([](AsyncWebServerRequest* request) {
        if (request->method() == HTTP_OPTIONS) {
            request->send(200);
            return;
        }

        Serial.printf("Not Found: ");
        if (request->method() == HTTP_GET)
            Serial.printf("GET");
        else if (request->method() == HTTP_POST)
            Serial.printf("POST");
        else
            Serial.printf("UNKNOWN");
        Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

        if (request->contentLength()) {
            Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
            Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
        }

        int headers = request->headers();
        int i;
        for (i = 0; i < headers; i++) {
            AsyncWebHeader* h = request->getHeader(i);
            Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
        }

        request->send(404);
        });


    // "Disable" CORS.
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    webServer.begin();
}
