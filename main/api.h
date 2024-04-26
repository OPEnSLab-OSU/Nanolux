#include <iterator>
#include <algorithm>

/**@file
 *
 * This file contains most of the web request API, in particular
 * pertaining to pattern and subpattern settings.
 * 
 * For the remaining parts of the API, see webServer.h
 *
**/

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

/// @brief Creates a JSON file with the list of patterns.
///
/// This file is intended to be sent to connecting devices in order
/// to present a list of patterns to the user.
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

/// @brief Handles sending the JSON of patterns to other devices.
/// @param request The incoming request for data.
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

/// @brief Handler function for getting subpattern data.
/// @param request The incoming request for data.
///
/// This handler is responsible for sending data from a particular subpattern.
/// The web request contains the particular subpattern to get through a string
/// query. Once that subpattern has been requested, the device sends back information
/// like the currently selected subpattern id, the pattern brightness, and the smoothing.
inline void handle_loaded_subpattern_get_request(AsyncWebServerRequest* request) {

  uint8_t subpattern_num = request->getParam(0)->value().toInt();
  bound_byte(&subpattern_num, 0, NUM_SUBPATTERNS);

  Subpattern_Data subpattern = loaded_pattern.subpattern[subpattern_num];

  // Create response substrings
  String idx = String(" \"idx\": ") + subpattern.idx;
  String bright = String(", \"brightness\": ") + subpattern.brightness;
  String smooth = String(", \"smoothing\": ") + subpattern.smoothing;

  // Build and send the final response
  const String response = String("{") + idx + bright + smooth + String(" }");
  request->send(HTTP_OK, CONTENT_JSON, response);
}

/// @brief Handler function for getting pattern data.
/// @param request The incoming request for data.
///
/// This handler is responsible for sending "pattern-level" data.
/// The device device sends back information like the subpattern count, pattern
/// transparancy, and the current mode (Z-layering or Strip Splitting)
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

/// @brief Handler function for updating pattern data.
/// @param request The incoming put request
/// @param json    The incoming JSON file holding new data.
///
/// This handler is responsible for updating pattern-level data from a 
/// web request.
inline void handle_pattern_update_put_request(AsyncWebServerRequest* request, JsonVariant& json) {
  if (request->method() == HTTP_PUT) {
    const JsonObject& payload = json.as<JsonObject>();

    int status = HTTP_OK;

    uint8_t count = payload["subpattern_count"];
    uint8_t alpha = payload["alpha"];
    uint8_t mode = payload["mode"];
    uint8_t noise = payload["noise"];

    bound_byte(&count, 1, 4);
    bound_byte(&alpha, 0, 255);
    bound_byte(&mode, 0, 1);
    bound_byte(&noise, 0, 100);

    if(count != loaded_pattern.subpattern_count)
      pattern_changed = true;
    
    if(mode != loaded_pattern.mode)
      pattern_changed = true;

    loaded_pattern.subpattern_count = count;
    loaded_pattern.alpha = alpha;
    loaded_pattern.noise_thresh = noise;
    loaded_pattern.mode = mode;

    manual_control_enabled = false;

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

/// @brief Handler function for updating subpattern data.
/// @param request The incoming put request
/// @param json    The incoming JSON file holding new data.
///
/// This handler is responsible for updating subpattern-level data from a 
/// web request.
/// The specific subpattern to update is obtained from a string query.
inline void handle_subpattern_update_put_request(AsyncWebServerRequest* request, JsonVariant& json) {
  if (request->method() == HTTP_PUT) {
    const JsonObject& payload = json.as<JsonObject>();

    int status = HTTP_OK;

    uint8_t subpattern_num = request->getParam(0)->value().toInt();

    bound_byte(&subpattern_num, 0, NUM_SUBPATTERNS);

    uint8_t idx = payload["idx"];
    uint8_t bright = payload["brightness"];
    uint8_t smooth = payload["smoothing"];

    bound_byte(&idx, 0, NUM_PATTERNS-1);
    bound_byte(&bright, 0, 255);
    bound_byte(&smooth, 0, 175);

    if(idx != loaded_pattern.subpattern[subpattern_num].idx)
      pattern_changed = true;

    loaded_pattern.subpattern[subpattern_num].idx = idx;
    loaded_pattern.subpattern[subpattern_num].brightness = bright;
    loaded_pattern.subpattern[subpattern_num].smoothing = smooth;

    manual_control_enabled = false;

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

/// @brief Handler function for loading a saved pattern.
/// @param request The incoming put request
/// @param json    The incoming JSON file holding the slot to load.
///
/// If the save slot to load is less than 0 or above NUM_SAVES, send
/// a failure response and don't load anything.
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
      manual_control_enabled = false;
      
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

/// @brief Handler function for saving the currently-running pattern.
/// @param request The incoming put request
/// @param json    The incoming JSON file holding the slot to save to.
///
/// If the save slot to load is less than 0 or above NUM_SAVES, send
/// a failure response and don't load anything.
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

/// @brief Handler function for updating system settings.
/// @param request The incoming put request
/// @param json    The incoming JSON file holding the new system settings to save
///
/// Includes data such as strip length, loop times, and debug mode.
inline void handle_system_settings_put_request(AsyncWebServerRequest* request, JsonVariant& json) {
  if (request->method() == HTTP_PUT) {
    const JsonObject& payload = json.as<JsonObject>();

    int status = HTTP_OK;

    uint8_t length = payload["length"];
    uint8_t loop = payload["loop"];
    uint8_t debug = payload["debug"];

    bound_byte(&length, 30, MAX_LEDS);
    bound_byte(&loop, 15, 100);
    bound_byte(&debug, 0, 2);

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

/// @brief Handler function for getting system settings.
/// @param request The incoming get request
///
/// Includes data such as strip length, loop times, and debug mode.
inline void handle_system_settings_get_request(AsyncWebServerRequest* request) {

  // Create response substrings
  String length = String(" \"length\": ") + config.length;
  String loop = String(", \"loop\": ") + config.loop_ms;
  String debug = String(", \"debug\": ") + config.debug_mode;

  // Build and send the final response
  const String response = String("{") + length + loop + debug + String(" }");
  request->send(HTTP_OK, CONTENT_JSON, response);
}

inline void handle_new_password_put_request(AsyncWebServerRequest* request, JsonVariant& json){

  if (request->method() == HTTP_PUT) {
    const JsonObject& payload = json.as<JsonObject>();

    int status = HTTP_OK;

    const char * new_password = payload["new_password"];

    // Reject if password is under 8 characters.
    bool over = true;
    for(uint8_t i = 0; i < 7; i++){
      if(new_password[i] == 0)
        over = false;
    }

    // Fail if under 8 characters
    if(!over){
      request->send(
      HTTP_OK,
      CONTENT_TEXT,
      build_response(
        false,
        "too short",
        nullptr));
    }

    // Reject if over 15 characters
    bool under = false;
    for(uint8_t i = 0; i < 16; i++){
      if(new_password[i] == 0)
        under = true;
    }

    // Send a fail message if over 15 characters
    if(!under){
      request->send(
      HTTP_OK,
      CONTENT_TEXT,
      build_response(
        false,
        "too long",
        nullptr));
    }

    for(uint8_t i = 0; i < 16; i++){
      config.pass[i] = new_password[i];
      if(new_password[i] == '\0')
        break;
    }
    Serial.println(config.pass);
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

/// The currently active get requests.
APIGetHook apiGetHooks[] = {
  { "/api/patterns", handle_patterns_list_request },
  { "/api/loadedSubpatterns", handle_loaded_subpattern_get_request },
  { "/api/loadedPatternSettings", handle_loaded_pattern_settings_get_request },
  { "/api/getDeviceSettings", handle_system_settings_get_request },


};
constexpr int API_GET_HOOK_COUNT = 4;

/// The currently active put requests.
APIPutHook apiPutHooks[] = {
  { "/api/updateLoadedSubpattern", handle_subpattern_update_put_request },
  { "/api/updateLoadedPattern", handle_pattern_update_put_request },
  { "/api/loadSaveSlot", handle_load_save_slot_put_request },
  { "/api/saveToSlot", handle_save_to_slot_put_request },
  { "/api/updateDeviceSettings", handle_system_settings_put_request },
  { "/api/updatePassword", handle_new_password_put_request },
};
constexpr int API_PUT_HOOK_COUNT = 6;
