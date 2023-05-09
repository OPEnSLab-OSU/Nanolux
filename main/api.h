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
inline void handle_patterns_list_request() {
    static bool initialized = false;

    if (!initialized) {
        initialize_pattern_list();
        initialized = true;
    }

    webServer.send(HTTP_OK, CONTENT_JSON, patterns_list);
}

 /*
  * Handle Pattern request or selection
  * (Single-value API example.)
  * Assume payload is JSON:
  * { "patten": "<80-char-max-string>" }
  */
inline void handle_pattern_request() {
    if (webServer.method() == HTTP_PUT) {
        // We receive the pattern index number.
        StaticJsonDocument<32> payload;
        int args = webServer.args();

        // The JSON payload of the PUT statement lives in the "plain" property of the request.
        const DeserializationError error = deserializeJson(payload, webServer.arg("plain"));

        // If there is a problem (unlikely for pattern since we provided the value
        // and it is, presumably, elected from a combo, but you never know), log
        // and tell the client the messed up.
        if (error) {
            Serial.print("Parsing pattern index message failed: ");
            Serial.println(error.c_str());
            webServer.send(HTTP_UNPROCESSABLE, CONTENT_JSON, error.c_str());
            return;
        }

        int status = HTTP_OK;
        int pattern_index = payload["index"];
        if (pattern_index >= 0 && pattern_index < NUM_PATTERNS) {
            gCurrentPatternNumber = pattern_index;
            webServer.send(HTTP_OK, CONTENT_TEXT, build_response(true, "Pattern set.", nullptr));
        }
        else {
            webServer.send(HTTP_BAD_REQUEST, CONTENT_TEXT, build_response(true, "Invalid pattern index.", nullptr));
        }
    }
    else {
        const String response = String("{ \"index\": ") + gCurrentPatternNumber + String(" }");
        webServer.send(HTTP_OK, CONTENT_JSON, response);
    }
}


inline void handle_noise_request() {
    if (webServer.method() == HTTP_PUT) {
        // We receive the pattern index number.
        StaticJsonDocument<32> payload;
        int args = webServer.args();

        // The JSON payload of the PUT statement lives in the "plain" property of the request.
        const DeserializationError error = deserializeJson(payload, webServer.arg("plain"));

        // If there is a problem (unlikely for pattern since we provided the value
        // and it is, presumably, elected from a combo, but you never know), log
        // and tell the client the messed up.
        if (error) {
            Serial.print("Parsing noise gate message failed: ");
            Serial.println(error.c_str());
            webServer.send(HTTP_UNPROCESSABLE, CONTENT_JSON, error.c_str());
            return;
        }

        int status = HTTP_OK;
        int noise_gate = payload["noise"];
        if (noise_gate > 0 && noise_gate < MAX_NOISE_GATE_THRESH) {
            gNoiseGateThreshold = noise_gate;
            webServer.send(HTTP_OK, CONTENT_TEXT, build_response(true, "Noise gate threshold set.", nullptr));
        }
        else {
            webServer.send(HTTP_BAD_REQUEST, CONTENT_TEXT, build_response(true, "Invalid noise gate value.", nullptr));
        }
    }
    else {
        const String response = String("{ \"noise\": ")  + gNoiseGateThreshold + String(" }");
        webServer.send(HTTP_OK, CONTENT_JSON, response);
    }
}



APIHook apiHooks[] = {
    { "/api/patterns", handle_patterns_list_request },
    { "/api/pattern", handle_pattern_request },
    { "/api/noise", handle_noise_request }
};
constexpr int API_HOOK_COUNT = 3;

