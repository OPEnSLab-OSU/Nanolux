#ifndef NANOLUX_WEBSERVER_H
#define NANOLUX_WEBSERVER_H

#include <ESPmDNS.h>
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
constexpr int HTTP_UNAUTHORIZED = 401;
constexpr int HTTP_UNPROCESSABLE = 422;
const char* CONTENT_JSON = "application/json";
const char* CONTENT_TEXT = "text/plain";

const char* ap_ssid = "AUDIOLUX";
const char* ap_password = "12345678";

static String hostname = String(DEFAULT_HOSTNAME);
const char* SETTINGS_FILE = "/settings.json";
static StaticJsonDocument<384> settings;

const char* URL_FILE = "/assets/url.json";


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
        available_networks[0].RSSI = END_OF_DATA;
    }
    else {
        const int network_count = min(n, MAX_NETWORKS);
        for (int i = 0; i < network_count; ++i) {
            available_networks[i].SSID = String(WiFi.SSID(i));
            available_networks[i].RSSI = WiFi.RSSI(i);
            available_networks[i].EncryptionType = WiFi.encryptionType(i);

            // Give some time to the hardware to get the next network ready.
            delay(10);
        }

        Serial.print("[*] Registered networks: ");
        for(int i = 0; i< network_count; ++i) {
            Serial.print("SSID:");
            Serial.print(available_networks[i].SSID);
            Serial.print("\t\t RSSI:");
            Serial.print(available_networks[i].RSSI);
            Serial.print("\t Type:");
            Serial.println(available_networks[i].EncryptionType);
        }

        if (network_count < MAX_NETWORKS) {
            available_networks[network_count].RSSI = END_OF_DATA;
        }
    }
}


inline void save_settings() {
    File saved_settings = LITTLEFS.open(SETTINGS_FILE, "w");
    if (saved_settings) {
        serializeJson(settings, saved_settings);
    } else {
        Serial.println("Unable to save settings file.");
    }
}


inline void load_settings() {
    File saved_settings = LITTLEFS.open(SETTINGS_FILE, "r");
    if (saved_settings) {
        DeserializationError error = deserializeJson(settings, saved_settings);
        if (!error) {
            return;
        } else {
            Serial.print("Error loading saved file: ");
            Serial.println(error.c_str());
        }
    }

    Serial.println("Unable to load settings. Will use defaults (re)create file.");

    JsonObject wifi = settings.createNestedObject("wifi");
    wifi["ssid"] = NULL;
    wifi["key"] = NULL;
    wifi["locked"] = false;
    settings["hostname"] = DEFAULT_HOSTNAME;

    save_settings();
}


/*
 * Wifi Management
 */
boolean join_wifi(const char* ssid, const char* key) {
    Serial.print("Trying to join network ");
    Serial.println(ssid);

    WiFi.setHostname(hostname.c_str());
    WiFi.begin(ssid, key);

    int wait_count = 0;
    while (WiFi.status() != WL_CONNECTED && wait_count < MAX_WIFI_CONNECT_WAIT) {
        delay(200);
        ++wait_count;
    }

    if (WiFi.status() != WL_CONNECTED) {
        WiFi.disconnect();
        Serial.println("Unable to join network.");
        return false;
    }

    Serial.println("Network joined");

    // We are connected. Setup mDNS
    if (MDNS.begin(hostname.c_str())) {
        Serial.print("mDNS connected. The AudioLux can be reached at ");
        Serial.print(hostname);
        Serial.println(".local");
    } else {
        Serial.println("Unable to setup mDNS");
    }

    return true;
}


/*
 * Wifi API handling
 */
inline void serve_wifi_list() {
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
    webServer.send(HTTP_OK, CONTENT_JSON, wifi_list);
}


const String& build_response(const boolean success, const char* message, const char* details) {
    String response = "{ \"success\": " + success;
    if (message != NULL) {
        response += response + ", \"message\": " + message;
    }
    if (details != NULL) {
        response += response + ", \"details\": " + details;
    }
    response += "}";

    return response;
}


inline void handle_wifi_request() {
    if (webServer.method() == HTTP_PUT) {
        StaticJsonDocument<192> payload;
        int args = webServer.args();
        DeserializationError error = deserializeJson(payload, webServer.arg("plain"));
        
        if (error) {
            Serial.print(F("Parsing wifi credentials failed: "));
            Serial.println(error.c_str());
            webServer.send(HTTP_UNPROCESSABLE, CONTENT_TEXT, error.c_str());
            return;
        }

        int status = HTTP_OK;

        boolean joined = false;
        if (payload["ssid"] == NULL) {
            joined = true;
        } else {
            joined = join_wifi(payload["ssid"], payload["key"]);
        }

        int response_status;
        String message;
        if (joined) {
            Serial.println("Joined (or forgot) network.");
            settings["wifi"]["ssid"] = payload["ssid"];
            settings["wifi"]["key"] = payload["key"];
            save_settings();
            response_status = HTTP_OK;
            message = "Joined (or forgot) network.";
        } else {
            Serial.println("Unable to join network.");
            response_status = HTTP_UNAUTHORIZED;
            message = "Unable to join network.";
        }
        webServer.send(response_status, CONTENT_JSON, build_response(joined, message.c_str(), nullptr));
    }
    else {
        const char* ssid = settings["wifi"]["ssid"];
        const String wifi = ssid == NULL? 
            String("null") : String("\"") + String(ssid) + String("\"");
        const String response = String("{ \"ssid\": ") + String(wifi) + String(" }");
        webServer.send(HTTP_OK, CONTENT_JSON, response);
    }
}


/*
 * Hostname API handling
 */
inline void handle_hostname_request() {
    if (webServer.method() == HTTP_PUT) {
        StaticJsonDocument<192> payload;
        int args = webServer.args();
        DeserializationError error = deserializeJson(payload, webServer.arg("plain"));

        if (error) {
            Serial.print(F("Parsing hostname failed: "));
            Serial.println(error.c_str());
            webServer.send(HTTP_UNPROCESSABLE, CONTENT_TEXT, error.c_str());
            return;
        }

        int status = HTTP_OK;

        settings["hostname"] = payload["hostname"];
        save_settings();
        webServer.send(HTTP_OK, CONTENT_TEXT, build_response(true, "New hostname saved.", nullptr));
    }
    else {
        const char* hostname = settings["hostname"];
        const String response = String("{ \"hostname\": ") + "\"" + String(hostname) + String("\" }");
        webServer.send(HTTP_OK, CONTENT_JSON, response);
    }
}


void save_url(const String& url) {
    File saved_url = LITTLEFS.open(URL_FILE, "w");
    if (saved_url) {
        StaticJsonDocument<192> data;

        data["url"] = url;

        serializeJson(data, saved_url);
        serializeJson(settings, saved_url);
        Serial.print(url);
        Serial.println(" saved as Web App URL.");
    }
    else {
        Serial.println("Unable to save Web App URL, will default to 192.168.4.1.");
    }
}


inline void initialize_web_server(APIHook api_hooks[], int hook_count) {
    initialize_file_system();


    WiFi.mode(WIFI_MODE_APSTA);
    WiFi.softAP(ap_ssid, ap_password);
    delay(1000);

    IPAddress ip = WiFi.softAPIP();

    // Load saved settings. If we have an SSID, try to join the network.
    load_settings();
    if (settings["wifi"]["ssid"] != NULL) {
        if (join_wifi(settings["wifi"]["ssid"], settings["wifi"]["key"])) {
            ip = WiFi.localIP();
        }
    } else {
        Serial.println("No wifi saved. AudioLux available via Access Point.");
    }

    const String url = "http://" + ip.toString();
    save_url(url);

    // API
    for (int i = 0; i < hook_count; i++) {
        webServer.on(api_hooks[i].path, api_hooks[i].handler);
    }

    // Add internal APi endpoints
    webServer.on("/api/wifis", serve_wifi_list);
    webServer.on("/api/wifi", handle_wifi_request);
    webServer.on("/api/hostname", handle_hostname_request);

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
