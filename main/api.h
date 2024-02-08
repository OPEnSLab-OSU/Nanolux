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

inline void handle_loaded_subpattern_get_request(AsyncWebServerRequest* request) {

    const uint8_t subpattern_num = request->getParam(0)->value().toInt();
    Pattern_Data subpattern = vol_subpatterns[subpattern_num];

    // Create response substrings
    String idx = String(" \"idx\": ") + subpattern.idx;
    String noise = String(", \"noise\": ") + subpattern.noise_thresh;
    String bright = String(", \"brightness\": ") + subpattern.brightness;
    String smooth = String(", \"smoothing\": ") + subpattern.smoothing;

    // Build and send the final response
    const String response = String("{") + idx + noise + bright + smooth + String(" }");
    request->send(HTTP_OK, CONTENT_JSON, response);

    
}

inline void handle_loaded_pattern_settings_get_request(AsyncWebServerRequest* request) {

    // Create response substrings
    String count = String(" \"subpattern_count\": ") + subpattern_count;
    String alpha = String(", \"alpha\": ") + loaded_alpha;
    String mode = String(", \"mode\": ") + loaded_mode;

    // Build and send the final response
    const String response = String("{") + count + alpha + mode + String(" }");
    request->send(HTTP_OK, CONTENT_JSON, response);
}

inline void handle_pattern_update_put_request(AsyncWebServerRequest* request, JsonVariant& json) {
    if (request->method() == HTTP_PUT) {
        const JsonObject& payload = json.as<JsonObject>();
        
        int status = HTTP_OK;
        const uint8_t count = payload["subpattern_count"];
        const uint8_t alpha = payload["alpha"];
        const uint8_t mode = payload["mode"];

        subpattern_count = count;
        loaded_alpha = alpha;
        loaded_mode = mode;

        Serial.println(subpattern_count);

        pattern_changed = true;

        request->send(
          HTTP_OK,
          CONTENT_TEXT,
          build_response(
            true,
            "success",
            nullptr
        ));
    }
    else {
        request->send(HTTP_METHOD_NOT_ALLOWED);
    }
}

inline void handle_subpattern_update_put_request(AsyncWebServerRequest* request, JsonVariant& json) {
    if (request->method() == HTTP_PUT) {
        const JsonObject& payload = json.as<JsonObject>();
        
        int status = HTTP_OK;

        const uint8_t subpattern_num = request->getParam(0)->value().toInt();

        const uint8_t idx = payload["idx"];
        const uint8_t noise = payload["noise"];
        const uint8_t bright = payload["brightness"];
        const uint8_t smooth = payload["smoothing"];

        current_subpatterns[subpattern_num].idx = idx;
        current_subpatterns[subpattern_num].noise_thresh = noise;
        current_subpatterns[subpattern_num].brightness = bright;
        current_subpatterns[subpattern_num].smoothing = smooth;

        pattern_changed = true;

        request->send(
          HTTP_OK,
          CONTENT_TEXT,
          build_response(
            true,
            "success",
            nullptr
        ));
    }
    else {
        request->send(HTTP_METHOD_NOT_ALLOWED);
    }
}


APIGetHook apiGetHooks[] = {
    { "/api/patterns", handle_patterns_list_request},
    { "/api/loadedSubpatterns", handle_loaded_subpattern_get_request},
    { "/api/loadedPatternSettings", handle_loaded_pattern_settings_get_request},

};
constexpr int API_GET_HOOK_COUNT = 3;

APIPutHook apiPutHooks[] = {
    { "/api/updateLoadedSubpattern", handle_subpattern_update_put_request},
    { "/api/updateLoadedPattern", handle_pattern_update_put_request},
};
constexpr int API_PUT_HOOK_COUNT = 2;
