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
            gCurrentPatternNumber = pattern_index;
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
    const String response = String("{ \"index\": ") + gCurrentPatternNumber + String(" }");
    request->send(HTTP_OK, CONTENT_JSON, response);
}


inline void handle_noise_put_request(AsyncWebServerRequest* request, JsonVariant& json) {
    if (request->method() == HTTP_PUT) {
        const JsonObject& payload = json.as<JsonObject>();
        
        int status = HTTP_OK;
        const uint8_t noise_gate = payload["noise"];
        if (noise_gate > 0 && noise_gate < MAX_NOISE_GATE_THRESH) {
            gNoiseGateThreshold = noise_gate;
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


inline void handle_noise_get_request(AsyncWebServerRequest* request) {
    const String response = String("{ \"noise\": ") + gNoiseGateThreshold + String(" }");
    request->send(HTTP_OK, CONTENT_JSON, response);
}

inline void handle_splitting_get_request(AsyncWebServerRequest* request) {
    if(isStripSplitting){
      const String response = String("{ \"splitting\": ") + "1" + String(" }");
      request->send(HTTP_OK, CONTENT_JSON, response);
    }else{
      const String response = String("{ \"splitting\": ") + "0" + String(" }");
      request->send(HTTP_OK, CONTENT_JSON, response);
    }
    
    
}

inline void handle_splitting_put_request(AsyncWebServerRequest* request, JsonVariant& json) {
    if (request->method() == HTTP_PUT) {
        const JsonObject& payload = json.as<JsonObject>();
        
        int status = HTTP_OK;
        const int sp = payload["isSplitting"];
        if (sp == 1) {
            isStripSplitting = true;
            request->send(HTTP_OK, CONTENT_TEXT, build_response(true, "Splitting state set: set to True: " + sp, nullptr));
        }else if (sp == 0){
          isStripSplitting = false;
          request->send(HTTP_OK, CONTENT_TEXT, build_response(true, "Splitting state set: set to False: " + sp, nullptr));
        }
        else {
            request->send(HTTP_BAD_REQUEST, CONTENT_TEXT, build_response(true, "Invalid splitting state value: " + sp, nullptr));
        }
    }
    else {
        request->send(HTTP_METHOD_NOT_ALLOWED);
    }
}




APIGetHook apiGetHooks[] = {
    { "/api/patterns", handle_patterns_list_request},
    { "/api/pattern", handle_pattern_get_request},
    { "/api/pattern2", handle_pattern_get_request},
    { "/api/noise", handle_noise_get_request},
    { "/api/splitting", handle_splitting_get_request}
};
constexpr int API_GET_HOOK_COUNT = 5;

APIPutHook apiPutHooks[] = {
    { "/api/pattern", handle_pattern_put_request},
    { "/api/pattern2", handle_pattern_put_request},
    { "/api/noise", handle_noise_put_request},
    { "/api/splitting", handle_splitting_put_request}
};
constexpr int API_PUT_HOOK_COUNT = 4;
