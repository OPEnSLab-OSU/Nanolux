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
            updated_data.pattern_1 = pattern_index;
            pattern_changed = true;
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
            updated_data.pattern_2 = pattern_index;
            pattern_changed = true;
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
    const String response = String("{ \"index\": ") + updated_data.pattern_1 + String(" }");
    request->send(HTTP_OK, CONTENT_JSON, response);
}

inline void handle_secondary_pattern_get_request(AsyncWebServerRequest* request) {
    const String response = String("{ \"index\": ") + updated_data.pattern_2 + String(" }");
    request->send(HTTP_OK, CONTENT_JSON, response);
}


inline void handle_noise_put_request(AsyncWebServerRequest* request, JsonVariant& json) {
    if (request->method() == HTTP_PUT) {
        const JsonObject& payload = json.as<JsonObject>();
        
        int status = HTTP_OK;
        const uint8_t noise_gate = payload["noise"];
        if (noise_gate >= 0 && noise_gate <= MAX_NOISE_GATE_THRESH) {
            updated_data.noise_thresh = noise_gate;
            pattern_changed = true;
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
            updated_data.alpha = a;
            pattern_changed = true;
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
    const String response = String("{ \"noise\": ") + updated_data.noise_thresh + String(" }");
    request->send(HTTP_OK, CONTENT_JSON, response);
}

inline void handle_alpha_get_request(AsyncWebServerRequest* request) {
    const String response = String("{ \"alpha\": ") +  updated_data.alpha + String(" }");
    request->send(HTTP_OK, CONTENT_JSON, response);
}

inline void handle_mode_get_request(AsyncWebServerRequest* request) {
    const String response = String("{ \"mode\": ") +  updated_data.mode + String(" }");
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
          updated_data.mode = mode;
          pattern_changed = true;
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
          updated_data = load_slot(slot);
          pattern_changed = true;
          request->send(HTTP_OK, CONTENT_TEXT, build_response(true, "acceptable slot: " + slot, nullptr));
        }
    }
    else {
        request->send(HTTP_METHOD_NOT_ALLOWED);
    }
}

inline void handle_brightness_get_request(AsyncWebServerRequest* request) {
    const String response = String("{ \"brightness\": ") +  updated_data.brightness + String(" }");
    request->send(HTTP_OK, CONTENT_JSON, response);
}

inline void handle_brightness_put_request(AsyncWebServerRequest* request, JsonVariant& json) {
    if (request->method() == HTTP_PUT) {
        const JsonObject& payload = json.as<JsonObject>();
        
        int status = HTTP_OK;
        const uint8_t brightness = payload["brightness"];
        if(brightness < 0 || brightness > 255){
          request->send(HTTP_OK, CONTENT_TEXT, build_response(true, "unacceptable brightness: " + brightness, nullptr));
        }else{
          updated_data.brightness = brightness;
          pattern_changed = true;
          request->send(HTTP_OK, CONTENT_TEXT, build_response(true, "acceptable brightness: " + brightness, nullptr));
        }
    }
    else {
        request->send(HTTP_METHOD_NOT_ALLOWED);
    }
}

inline void handle_smoothing_get_request(AsyncWebServerRequest* request) {
    const String response = String("{ \"smoothing\": ") +  updated_data.smoothing + String(" }");
    request->send(HTTP_OK, CONTENT_JSON, response);
}

inline void handle_smoothing_put_request(AsyncWebServerRequest* request, JsonVariant& json) {
    if (request->method() == HTTP_PUT) {
        const JsonObject& payload = json.as<JsonObject>();
        
        int status = HTTP_OK;
        const uint8_t smoothing = payload["smoothing"];
        if(smoothing < 0 || smoothing > 255){
          request->send(HTTP_OK, CONTENT_TEXT, build_response(true, "unacceptable smoothing: " + smoothing, nullptr));
        }else{
          updated_data.smoothing = smoothing;
          pattern_changed = true;
          request->send(HTTP_OK, CONTENT_TEXT, build_response(true, "acceptable smoothing: " + smoothing, nullptr));
        }
    }
    else {
        request->send(HTTP_METHOD_NOT_ALLOWED);
    }
}

inline void handle_len_get_request(AsyncWebServerRequest* request) {
    const String response = String("{ \"len\": ") +  config.length + String(" }");
    request->send(HTTP_OK, CONTENT_JSON, response);
}

inline void handle_len_put_request(AsyncWebServerRequest* request, JsonVariant& json) {
    if (request->method() == HTTP_PUT) {
        const JsonObject& payload = json.as<JsonObject>();
        
        int status = HTTP_OK;
        const uint8_t length = payload["len"];
        if(length < 30 || length > MAX_LEDS){
          request->send(HTTP_OK, CONTENT_TEXT, build_response(true, "unacceptable length: " + length, nullptr));
        }else{
          config.length = length;
          altered_config = true;
          request->send(HTTP_OK, CONTENT_TEXT, build_response(true, "acceptable length: " + length, nullptr));
        }
    }
    else {
        request->send(HTTP_METHOD_NOT_ALLOWED);
    }
}

inline void handle_ms_get_request(AsyncWebServerRequest* request) {
    const String response = String("{ \"ms\": ") +  config.loop_ms + String(" }");
    request->send(HTTP_OK, CONTENT_JSON, response);
}

inline void handle_ms_put_request(AsyncWebServerRequest* request, JsonVariant& json) {
    if (request->method() == HTTP_PUT) {
        const JsonObject& payload = json.as<JsonObject>();
        
        int status = HTTP_OK;
        const uint8_t loop_ms = payload["ms"];
        if(loop_ms < MIN_REFRESH || loop_ms > MAX_REFRESH){
          request->send(HTTP_OK, CONTENT_TEXT, build_response(true, "unacceptable loop time: " + loop_ms, nullptr));
        }else{
          config.loop_ms = loop_ms;
          altered_config = true;
          request->send(HTTP_OK, CONTENT_TEXT, build_response(true, "acceptable loop time: " + loop_ms, nullptr));
        }
    }
    else {
        request->send(HTTP_METHOD_NOT_ALLOWED);
    }
}

inline void handle_debug_get_request(AsyncWebServerRequest* request) {
    const String response = String("{ \"debug\": ") +  config.debug_mode + String(" }");
    request->send(HTTP_OK, CONTENT_JSON, response);
}

inline void handle_debug_put_request(AsyncWebServerRequest* request, JsonVariant& json) {
    if (request->method() == HTTP_PUT) {
        const JsonObject& payload = json.as<JsonObject>();
        
        int status = HTTP_OK;
        const uint8_t debug_mode = payload["debug"];
        if(debug_mode < 0 || debug_mode > 2){
          request->send(HTTP_OK, CONTENT_TEXT, build_response(true, "unacceptable debug: " + debug_mode, nullptr));
        }else{
          config.debug_mode = debug_mode;
          altered_config = true;
          request->send(HTTP_OK, CONTENT_TEXT, build_response(true, "acceptable loop time: " + debug_mode, nullptr));
        }
    }
    else {
        request->send(HTTP_METHOD_NOT_ALLOWED);
    }
}

typedef struct{

  uint8_t idx = 0;
  uint8_t noise_thresh = 0;
  uint8_t brightness = 255;
  uint8_t smoothing = 0;

} Pattern_Data;

inline void handle_loaded_subpattern_get_request(AsyncWebServerRequest* request) {

    const uint8_t subpattern_num = request->getParam(0)->value().toInt();
    Pattern_Data subpattern = vol_subpatterns[subpattern_num];

    // Create response substrings
    String idx = String(" \"idx\": ") + subpattern.idx;
    String noise = String(" \"noise\": ") + subpattern.noise;
    String bright = String(" \"brightness\": ") + subpattern.brightness;
    String smooth = String(" \"smoothing\": ") + subpattern.smoothing;

    // Build and send the final response
    const String response = String("{") + idx + noise + bright + smooth + String(" }");
    request->send(HTTP_OK, CONTENT_JSON, response);
}


APIGetHook apiGetHooks[] = {
    { "/api/patterns", handle_patterns_list_request},
    { "/api/pattern", handle_pattern_get_request},
    { "/api/pattern2", handle_secondary_pattern_get_request},
    { "/api/noise", handle_noise_get_request},
    { "/api/alpha", handle_alpha_get_request},
    { "/api/mode", handle_mode_get_request},
    { "/api/brightness", handle_brightness_get_request},
    { "/api/smoothing", handle_smoothing_get_request},
    { "/api/len", handle_len_get_request},
    { "/api/ms", handle_ms_get_request},
    { "/api/debug", handle_debug_get_request}

};
constexpr int API_GET_HOOK_COUNT = 11;

APIPutHook apiPutHooks[] = {
    { "/api/pattern", handle_pattern_put_request},
    { "/api/pattern2", handle_secondary_pattern_put_request},
    { "/api/noise", handle_noise_put_request},
    { "/api/alpha", handle_alpha_put_request},
    { "/api/mode", handle_mode_put_request},
    { "/api/save", handle_save_request},
    { "/api/load", handle_load_request},
    { "/api/brightness", handle_brightness_put_request},
    { "/api/smoothing", handle_smoothing_put_request},
    { "/api/len", handle_len_put_request},
    { "/api/ms", handle_ms_put_request},
    { "/api/debug", handle_debug_put_request}
};
constexpr int API_PUT_HOOK_COUNT = 12;
