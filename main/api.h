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
  Subpattern_Data subpattern = loaded_pattern.subpattern[subpattern_num];

  // Create response substrings
  String idx = String(" \"idx\": ") + subpattern.idx;
  String bright = String(", \"brightness\": ") + subpattern.brightness;
  String smooth = String(", \"smoothing\": ") + subpattern.smoothing;
  String minhue = String(", \"hue_min\": ") + subpattern.minhue;
  String maxhue = String(", \"hue_max\": ") + subpattern.maxhue;
  String conf = String(", \"config\": ") + subpattern.config;
  String reversed = String(", \"reversed\": ") + subpattern.config;
  String mirrored = String(", \"mirrored\": ") + subpattern.config;

  // Build and send the final response
  const String response = String("{") + idx + bright + smooth + minhue + maxhue + conf + reversed + mirrored + String(" }");
  request->send(HTTP_OK, CONTENT_JSON, response);
}

inline void handle_loaded_pattern_settings_get_request(AsyncWebServerRequest* request) {

  // Create response substrings
  String count = String(" \"subpattern_count\": ") + loaded_pattern.subpattern_count;
  String alpha = String(", \"alpha\": ") + loaded_pattern.alpha;
  String mode = String(", \"mode\": ") + loaded_pattern.mode;
  String noise = String(", \"noise\": ") + loaded_pattern.noise_thresh;

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
    const uint8_t noise = payload["noise"];

    loaded_pattern.subpattern_count = count;
    loaded_pattern.alpha = alpha;
    loaded_pattern.noise_thresh = noise;
    loaded_pattern.mode = mode;

    pattern_changed = true;

    request->send(
      HTTP_OK,
      CONTENT_TEXT,
      build_response(
        true,
        "success",
        nullptr));
  } else {
    request->send(HTTP_METHOD_NOT_ALLOWED);
  }
}

inline void handle_subpattern_update_put_request(AsyncWebServerRequest* request, JsonVariant& json) {
  if (request->method() == HTTP_PUT) {
    const JsonObject& payload = json.as<JsonObject>();

    int status = HTTP_OK;

    const uint8_t subpattern_num = request->getParam(0)->value().toInt();

    const uint8_t idx = payload["idx"];
    const uint8_t bright = payload["brightness"];
    const uint8_t smooth = payload["smoothing"];

    const uint8_t hue_min = payload["hue_min"];
    const uint8_t hue_max = payload["hue_max"];
    const uint8_t conf = payload["config"];
    const uint8_t reversed = payload["reversed"];
    const uint8_t mirrored = payload["mirrored"];

    if(idx != loaded_pattern.subpattern[subpattern_num].idx)
      pattern_changed = true;

    loaded_pattern.subpattern[subpattern_num].idx = idx;
    loaded_pattern.subpattern[subpattern_num].brightness = bright;
    loaded_pattern.subpattern[subpattern_num].smoothing = smooth;

    loaded_pattern.subpattern[subpattern_num].minhue = hue_min;
    loaded_pattern.subpattern[subpattern_num].maxhue = hue_max;
    loaded_pattern.subpattern[subpattern_num].config = conf;
    loaded_pattern.subpattern[subpattern_num].reversed = reversed;
    loaded_pattern.subpattern[subpattern_num].mirrored = mirrored;

    request->send(
      HTTP_OK,
      CONTENT_TEXT,
      build_response(
        true,
        "success",
        nullptr));
  } else {
    request->send(HTTP_METHOD_NOT_ALLOWED);
  }
}

inline void handle_load_save_slot_put_request(AsyncWebServerRequest* request, JsonVariant& json) {
  if (request->method() == HTTP_PUT) {
    const JsonObject& payload = json.as<JsonObject>();

    int status = HTTP_OK;

    const uint8_t slot = payload["slot"];

    if (slot > NUM_SAVES || slot < 0) {

      request->send(
        HTTP_OK,
        CONTENT_TEXT,
        build_response(
          false,
          "failure",
          nullptr));

    } else {

      load_slot(slot);
      pattern_changed = true;

      request->send(
        HTTP_OK,
        CONTENT_TEXT,
        build_response(
          true,
          "success",
          nullptr));
    }
    request->send(
      HTTP_OK,
      CONTENT_TEXT,
      build_response(
        true,
        "success",
        nullptr));
  } else {
    request->send(HTTP_METHOD_NOT_ALLOWED);
  }
}

inline void handle_save_to_slot_put_request(AsyncWebServerRequest* request, JsonVariant& json) {
  if (request->method() == HTTP_PUT) {
    const JsonObject& payload = json.as<JsonObject>();

    int status = HTTP_OK;

    const uint8_t slot = payload["slot"];

    if (slot > NUM_SAVES || slot < 0) {

      request->send(
        HTTP_OK,
        CONTENT_TEXT,
        build_response(
          false,
          "failure",
          nullptr));

    } else {

      set_slot(slot);
      save_to_nvs();

      request->send(
        HTTP_OK,
        CONTENT_TEXT,
        build_response(
          true,
          "success",
          nullptr));
    }
    request->send(
      HTTP_OK,
      CONTENT_TEXT,
      build_response(
        true,
        "success",
        nullptr));
  } else {
    request->send(HTTP_METHOD_NOT_ALLOWED);
  }
}

inline void handle_system_settings_put_request(AsyncWebServerRequest* request, JsonVariant& json) {
  if (request->method() == HTTP_PUT) {
    const JsonObject& payload = json.as<JsonObject>();

    int status = HTTP_OK;

    const uint8_t length = payload["length"];
    const uint8_t loop = payload["loop"];
    const uint8_t debug = payload["debug"];

    if(config.length != length)
      pattern_changed = true;

    config.length = length;
    config.loop_ms = loop;
    config.debug_mode = debug;

    save_config_to_nvs();

    request->send(
      HTTP_OK,
      CONTENT_TEXT,
      build_response(
        true,
        "success",
        nullptr));
  } else {
    request->send(HTTP_METHOD_NOT_ALLOWED);
  }
}

inline void handle_system_settings_get_request(AsyncWebServerRequest* request) {

  // Create response substrings
  String length = String(" \"length\": ") + config.length;
  String loop = String(", \"loop\": ") + config.loop_ms;
  String debug = String(", \"debug\": ") + config.debug_mode;

  // Build and send the final response
  const String response = String("{") + length + loop + debug + String(" }");
  request->send(HTTP_OK, CONTENT_JSON, response);
}


APIGetHook apiGetHooks[] = {
  { "/api/patterns", handle_patterns_list_request },
  { "/api/loadedSubpatterns", handle_loaded_subpattern_get_request },
  { "/api/loadedPatternSettings", handle_loaded_pattern_settings_get_request },
  { "/api/getDeviceSettings", handle_system_settings_get_request },


};
constexpr int API_GET_HOOK_COUNT = 4;

APIPutHook apiPutHooks[] = {
  { "/api/updateLoadedSubpattern", handle_subpattern_update_put_request },
  { "/api/updateLoadedPattern", handle_pattern_update_put_request },
  { "/api/loadSaveSlot", handle_load_save_slot_put_request },
  { "/api/saveToSlot", handle_save_to_slot_put_request },
  { "/api/updateDeviceSettings", handle_system_settings_put_request },
};
constexpr int API_PUT_HOOK_COUNT = 5;
