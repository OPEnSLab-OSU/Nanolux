/** @file
  *
  * The functions and objects in this file manage patterns
  * saved to the device.
  *
  * The AudioLux uses the ESP32's Prefrences library to
  * interface with Non-Volatile Storage, or NVS.
  *
*/

#include <string>
#include <Preferences.h>
#include "storage.h"
#include "nanolux_util.h"

#define PATTERN_NAMESPACE "p"
#define PATTERN_KEY       "k"
#define CONFIG_NAMESPACE  "c"
#define CONFIG_KEY        "f"

/// The currently loaded pattern, externed from main.ino.
extern Pattern_Data loaded_pattern;

/// The array for storing saved patterns, externed from main.ino.
extern Pattern_Data saved_patterns[NUM_SAVES];

/// The number of patterns that can be shown, externed from globals.h.
extern int NUM_PATTERNS;

/// The object used to access Non-Volatile Storage. Reused between accesses.
Preferences storage;

// Holds device configuration data.
extern Config_Data config;

/************************************************
 *
 * PRIVATE:
 * Functions listed here are considered private
 * to the storage file. Often, proper use of
 * these functions requires additional calls
 * outside the scope of the base function to
 * work.
 *
*************************************************/

/// @brief Clears the NVS namespace associated with saved
/// patterns.
///
/// This function should not be called by itself.
void clear_nvs() {
  storage.begin(PATTERN_NAMESPACE, false);
  storage.clear();
  storage.end();
}

/// @brief Ensures all saved pattern data is properly constrained.
///
/// Constrains both pattern and subpattern level settings.
/// This function should not be called by itself.
void bound_user_data() {
  // Loaded pattern:
  bound_byte(&loaded_pattern.subpattern_count, 0, NUM_SUBPATTERNS);
  bound_byte(&loaded_pattern.alpha, 0, 255);
  bound_byte(&loaded_pattern.mode, 0, 1);
  bound_byte(&loaded_pattern.noise_thresh, 0, 100);

  for(int i = 0; i < NUM_SUBPATTERNS; i++){
    bound_byte(&loaded_pattern.subpattern[i].brightness, 0, 255);
    bound_byte(&loaded_pattern.subpattern[i].smoothing, 0, 175);
    bound_byte(&loaded_pattern.subpattern[i].idx, 0, NUM_PATTERNS);
  }
}

/// @brief Ensures all saved user data is properly constrained.
///
/// This function should not be called by itself.
void bound_system_settings() {
  bound_byte(&config.debug_mode, 0, 2);
  bound_byte(&config.length, 30, 200);
  bound_byte(&config.loop_ms, 15, 100);
}

/************************************************
 *
 * PUBLIC:
 * Functions defined below this message are "safe"
 * to call elsewhere in the program
 *
*************************************************/

/// @brief Move subpatterns in a slot to the main buffer.
/// @param slot The save slot number to load from.
void load_slot(int slot) {

  // Return if slot is greater than the number of patterns.
  if (slot > NUM_SAVES - 1) return;

  if (loaded_pattern.subpattern_count > 4 || loaded_pattern.subpattern_count < 1)
    loaded_pattern.subpattern_count = 1;

  // Copy the subpatterns into the main buffer.
  memcpy(
    &loaded_pattern,
    &saved_patterns[slot],
    sizeof(Pattern_Data));
}

/// @brief Saves the currently-loaded pattern to a save slot.
/// @param slot The save slot to overwrite.
void set_slot(int slot) {

  // Return if slot is greater than the number of patterns.
  if (slot > NUM_SAVES - 1) return;

  memcpy(
    &saved_patterns[slot],
    &loaded_pattern,
    sizeof(Pattern_Data));
  
  bound_user_data();
}

/// @brief Saves all currently-loaded patterns to NVS
///
/// This does not save the currently-loaded pattern.
void save_to_nvs() {
  storage.begin(PATTERN_NAMESPACE, false);
  storage.putBytes(
    PATTERN_KEY,
    &saved_patterns[0],
    sizeof(Pattern_Data) * NUM_SAVES);
  storage.end();
}

/// @brief Clears all patterns stored in NVS
///
/// This function also clears all patterns saved in
/// volatile storage as well.
void clear_all() {
  clear_nvs();
  memset(
    &saved_patterns[0],
    0,
    sizeof(Pattern_Data) * NUM_SAVES);
  save_to_nvs();
}

/// @brief Saves configuration data to the NVS
///
/// Bounds currently-loaded settings before saving to NVS.
void save_config_to_nvs() {
  bound_system_settings();
  storage.begin(CONFIG_NAMESPACE, false);
  storage.putBytes(CONFIG_KEY, &config, sizeof(Config_Data));
  storage.end();
}

/// @brief Ensures that saved settings have reasonable data.
///
/// TODO: Is this function even needed anymore?
void verify_saves() {
  for (int i = 0; i < NUM_SAVES; i++) {
    if (saved_patterns[i].subpattern_count > 4 || saved_patterns[i].subpattern_count == 0){
      saved_patterns[i].subpattern_count = 1;
    }
  }
}

/// @brief Load all patterns from the NVS
///
/// Ensures all loaded data is properly bounded.
void load_from_nvs() {

  storage.begin(PATTERN_NAMESPACE, false);

  if (storage.isKey(PATTERN_KEY)) {
    storage.getBytes(
      PATTERN_KEY,
      &saved_patterns[0],
      sizeof(Pattern_Data) * NUM_SAVES);
  }

  storage.end();

  storage.begin(CONFIG_NAMESPACE, false);

  if (storage.isKey(CONFIG_KEY))
    storage.getBytes(CONFIG_KEY, &config, sizeof(Config_Data));

  storage.end();

  if (config.init == false) {
    config.init = true;
    config.debug_mode = 0;
    config.length = 60;
    config.loop_ms = 40;
  }

  bound_system_settings();
  
}
