#pragma once

/*
 * Use https://arduinojson.org/v6/assistant/ to calculate the size
 * required for the StaticJsonDocument.
 */

using namespace ARDUINOJSON_NAMESPACE;


/*
 * HTTP_OK, etc.
 * CONTENT_JSON, etc. come from WebServer.h
 */


constexpr int PATTERN_BUFFER_SIZE = 4096;
static char patterns_list[PATTERN_BUFFER_SIZE];

inline void initialize_pattern_list() {
    // Holds roughly 50 patterns.
    DynamicJsonDocument patterns(PATTERN_BUFFER_SIZE);

    for (int i = 0; i < NUM_PATTERNS; i++) {
        const JsonObject pattern = patterns.createNestedObject();
        pattern["index"] = mainPatterns[i].index;
        pattern["name"] = mainPatterns[i].pattern_name;
    }
    serializeJson(patterns, patterns_list);
}


/*
 * Handle Settings request
 * (Simple value list API example.)
 */
inline void handle_patterns_list_request(AsyncWebServerRequest* request) {
    static bool initialized = false;

    if (!initialized) {
        initialize_pattern_list();
        initialized = true;
    }

    request->send(HTTP_OK, CONTENT_JSON, patterns_list);
}

 /*
  * Handle Pattern request or selection
  * (Single-value API example.)
  * Assume payload is JSON:
  * { "patten": "<80-char-max-string>" }
  */
inline void handle_pattern_put_request(AsyncWebServerRequest* request, JsonVariant& json) {
    if (request->method() == HTTP_PUT) {
        const JsonObject& payload = json.as<JsonObject>();

        int status = HTTP_OK;
        int pattern_index = payload["index"];
        if (pattern_index >= 0 && pattern_index < NUM_PATTERNS) {
            current_pattern.pattern_1 = pattern_index;
            request->send(HTTP_OK, CONTENT_TEXT, build_response(true, "Pattern set.", nullptr));
        }
        else {
            request->send(HTTP_BAD_REQUEST, CONTENT_TEXT, build_response(true, "Invalid pattern index.", nullptr));
        }
    }
    else {
    }
}

inline void handle_secondary_pattern_put_request(AsyncWebServerRequest* request, JsonVariant& json) {
    if (request->method() == HTTP_PUT) {
        const JsonObject& payload = json.as<JsonObject>();

        int status = HTTP_OK;
        int pattern_index = payload["index"];
        if (pattern_index >= 0 && pattern_index < NUM_PATTERNS) {
            current_pattern.pattern_2 = pattern_index;
            request->send(HTTP_OK, CONTENT_TEXT, build_response(true, "Pattern set.", nullptr));
        }
        else {
            request->send(HTTP_BAD_REQUEST, CONTENT_TEXT, build_response(true, "Invalid pattern index.", nullptr));
        }
    }
    else {
    }
}

inline void handle_pattern_get_request(AsyncWebServerRequest* request) {
    const String response = String("{ \"index\": ") + current_pattern.pattern_1 + String(" }");
    request->send(HTTP_OK, CONTENT_JSON, response);
}

inline void handle_secondary_pattern_get_request(AsyncWebServerRequest* request) {
    const String response = String("{ \"index\": ") + current_pattern.pattern_2 + String(" }");
    request->send(HTTP_OK, CONTENT_JSON, response);
}


inline void handle_noise_put_request(AsyncWebServerRequest* request, JsonVariant& json) {
    if (request->method() == HTTP_PUT) {
        const JsonObject& payload = json.as<JsonObject>();
        
        int status = HTTP_OK;
        const uint8_t noise_gate = payload["noise"];
        if (noise_gate >= 0 && noise_gate <= MAX_NOISE_GATE_THRESH) {
            current_pattern.noise_thresh = noise_gate;
            request->send(HTTP_OK, CONTENT_TEXT, build_response(true, "Noise gate threshold set.", nullptr));
        }
        else {
            request->send(HTTP_BAD_REQUEST, CONTENT_TEXT, build_response(true, "Invalid noise gate value.", nullptr));
        }
    }
    else {
        request->send(HTTP_METHOD_NOT_ALLOWED);
    }
}

inline void handle_alpha_put_request(AsyncWebServerRequest* request, JsonVariant& json) {
    if (request->method() == HTTP_PUT) {
        const JsonObject& payload = json.as<JsonObject>();
        
        int status = HTTP_OK;
        const uint8_t a = payload["alpha"];
        if (a > -1 && a <= 255) {
            current_pattern.alpha = a;
            request->send(HTTP_OK, CONTENT_TEXT, build_response(true, "alpha set.", nullptr));
        }
        else {
            request->send(HTTP_BAD_REQUEST, CONTENT_TEXT, build_response(true, "Invalid alpha value.", nullptr));
        }
    }
    else {
        request->send(HTTP_METHOD_NOT_ALLOWED);
    }
}


inline void handle_noise_get_request(AsyncWebServerRequest* request) {
    const String response = String("{ \"noise\": ") + current_pattern.noise_thresh + String(" }");
    request->send(HTTP_OK, CONTENT_JSON, response);
}

inline void handle_alpha_get_request(AsyncWebServerRequest* request) {
    const String response = String("{ \"alpha\": ") +  current_pattern.alpha + String(" }");
    request->send(HTTP_OK, CONTENT_JSON, response);
}

inline void handle_mode_get_request(AsyncWebServerRequest* request) {
    const String response = String("{ \"mode\": ") +  current_pattern.mode + String(" }");
    request->send(HTTP_OK, CONTENT_JSON, response);
}

inline void handle_mode_put_request(AsyncWebServerRequest* request, JsonVariant& json) {
    if (request->method() == HTTP_PUT) {
        const JsonObject& payload = json.as<JsonObject>();
        
        int status = HTTP_OK;
        const int mode = payload["mode"];
        if(mode < 0 || mode > 2){
          request->send(HTTP_OK, CONTENT_TEXT, build_response(true, "unacceptable mode: " + mode, nullptr));
        }else{
          current_pattern.mode = mode;
          request->send(HTTP_OK, CONTENT_TEXT, build_response(true, "acceptable mode: " + mode, nullptr));
        }

    }
    else {
        request->send(HTTP_METHOD_NOT_ALLOWED);
    }
}

inline void handle_save_request(AsyncWebServerRequest* request, JsonVariant& json) {
    if (request->method() == HTTP_PUT) {
        const JsonObject& payload = json.as<JsonObject>();
        
        int status = HTTP_OK;
        const int slot = payload["slot"];
        if(slot < 0 || slot > 5){
          request->send(HTTP_OK, CONTENT_TEXT, build_response(true, "unacceptable slot: " + slot, nullptr));
        }else{
          set_slot(slot);
          save_to_nvs();
          request->send(HTTP_OK, CONTENT_TEXT, build_response(true, "acceptable slot: " + slot, nullptr));
        }
    }
    else {
        request->send(HTTP_METHOD_NOT_ALLOWED);
    }
}

inline void handle_load_request(AsyncWebServerRequest* request, JsonVariant& json) {
    if (request->method() == HTTP_PUT) {
        const JsonObject& payload = json.as<JsonObject>();
        
        int status = HTTP_OK;
        const int slot = payload["slot"];
        if(slot < 0 || slot > 5){
          request->send(HTTP_OK, CONTENT_TEXT, build_response(true, "unacceptable slot: " + slot, nullptr));
        }else{
          load_slot(slot);
          request->send(HTTP_OK, CONTENT_TEXT, build_response(true, "acceptable slot: " + slot, nullptr));
        }
    }
    else {
        request->send(HTTP_METHOD_NOT_ALLOWED);
    }
}

APIGetHook apiGetHooks[] = {
    { "/api/patterns", handle_patterns_list_request},
    { "/api/pattern", handle_pattern_get_request},
    { "/api/pattern2", handle_secondary_pattern_get_request},
    { "/api/noise", handle_noise_get_request},
    { "/api/alpha", handle_alpha_get_request},
    { "/api/mode", handle_mode_get_request}
};
constexpr int API_GET_HOOK_COUNT = 6;

APIPutHook apiPutHooks[] = {
    { "/api/pattern", handle_pattern_put_request},
    { "/api/pattern2", handle_secondary_pattern_put_request},
    { "/api/noise", handle_noise_put_request},
    { "/api/alpha", handle_alpha_put_request},
    { "/api/mode", handle_mode_put_request},
    { "/api/save", handle_save_request},
    { "/api/load", handle_load_request}
};
constexpr int API_PUT_HOOK_COUNT = 7;
