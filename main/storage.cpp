#include <string>
#include <Preferences.h>
#include "storage.h"

extern Pattern_Data loaded_pattern;
extern Pattern_Data saved_patterns[NUM_SAVES];

// NVS STORAGE
char* PATTERN_NAMESPACE = "p";
char* PATTERN_KEY = "k";
char* CONFIG_NAMESPACE = "c";
char* CONFIG_KEY = "f";
Preferences storage;

// ADDITIONAL CONFIGURATIONS
extern Config_Data config;

// PRIVATE FUNCTIONS:
void clear_nvs() {
  storage.begin(PATTERN_NAMESPACE, false);
  storage.clear();
  storage.end();
}

// PUBLIC FUNCTIONS:

/// @brief Move subpatterns in a slot to the main buffer.
/// @param slot The save slot number to load from.
void load_slot(int slot) {

  // Return if slot is greater than the number of patterns.
  if (slot > NUM_SAVES - 1) return;

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
}

/// @brief Saves all currently-loaded patterns to NVS
void save_to_nvs() {
  storage.begin(PATTERN_NAMESPACE, false);
  storage.putBytes(
    PATTERN_KEY,
    &saved_patterns[0],
    sizeof(Pattern_Data) * NUM_SAVES);
  storage.end();
}

/// @brief Clears all patterns stored in NVS
void clear_all() {
  clear_nvs();
  memset(
    &saved_patterns[0],
    0,
    sizeof(Pattern_Data) * NUM_SAVES);
  save_to_nvs();
}

/// @brief Saves configuration data to the NVS
void save_config_to_nvs() {
  storage.begin(CONFIG_NAMESPACE, false);
  storage.putBytes(CONFIG_KEY, &config, sizeof(Config_Data));
  storage.end();
}

/// @brief Load all patterns from the NVS
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

  if (!config.init) {
    config.init = true;
    config.debug_mode = 0;
    config.length = 60;
    config.loop_ms = 40;
  }
}
