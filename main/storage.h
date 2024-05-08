/**@file
 *
 * This file contains function headers for storage.cpp
 * along with data structures for user data stored in
 * NVS.
 *
**/

#ifndef STORAGE_H
#define STORAGE_H

#include <stdint.h>

/******************************************************************
*
* SUBPATTERNS AND PATTERNS
*
* In the NanoLux codebase, the term "subpattern" refers to the
* individual light pattern running on part of the LED strip.
* Therefore, in the case of strip splitting, up to NUM_SUBPATTERNS
* could run side-by-side. Each subpattern has other data besides
* the pattern index, which includes a brightness value and a
* smoothing value.
*
* The term "pattern" refers to everything currently running on the
* strip. Each pattern contains the individual subpatterns within
* them, and have extra data that pertains to the entire LED strip
* or audio analysis module that cannot run multiple times. Of note
* is the number of subpatterns that pattern is currently running.
*
******************************************************************/

/// The number of saved patterns the ESP32 is able to address.
#define NUM_SAVES  3

/// The number of subpatterns each pattern can play at maximum.
#define NUM_SUBPATTERNS 4

/// A structure holding subpattern configuration data.
typedef struct{

  uint8_t idx = 0; /// The selected pattern name to run.
  uint8_t brightness = 255; /// The subpattern's brightness.
  uint8_t smoothing = 0; /// How smoothed subpattern light changes are.
  uint8_t postprocessing_mode = 0; // The current mode for postprocessing

} Subpattern_Data;

/// A structure holding pattern configuration data.
typedef struct{

  uint8_t alpha = 0; /// How transparent the top pattern is in Z-layering.
  uint8_t noise_thresh = 0; /// The minimum noise floor to consider as audio.
  uint8_t mode = 0; /// The currently-running pattern mode (splitting vs layering).
  uint8_t subpattern_count = 1; /// The number of subpatterns this pattern has.
  Subpattern_Data subpattern[NUM_SUBPATTERNS]; /// Data for all subpatterns loaded.

} Pattern_Data;

/// A structure holding system configuration data.
typedef struct{

  uint8_t length = 60; /// The length of the LED strip.
  uint8_t loop_ms = 40; /// The number of milliseconds one program loop takes.
  uint8_t debug_mode = 0; /// The currently selected debug output mode.
  bool init = true; /// If the loaded config data is valid.

} Config_Data;


void load_slot(int slot);
void set_slot(int slot);
void save_to_nvs();
void clear_all();
void load_from_nvs();
void save_config_to_nvs();
void verify_saves();

#endif