#include <stdint.h>
#ifndef STORAGE_H
#define STORAGE_H

#define NUM_SAVES  5

typedef struct{

  uint8_t pattern_idx = 0; // Index number of which pattern is current
  uint8_t noise_thresh = 0;
  uint8_t alpha = 0;
  uint8_t brightness = 255;
  uint8_t smoothing = 0;
  uint8_t hue_min = 0;
  uint8_t hue_max = 255;
  bool direction_symmetry = false;

} Save_Data;

typedef struct{

  uint8_t pattern_1 = 0; // Index number of which pattern is current
  uint8_t pattern_2 = 0;
  uint8_t noise_thresh = 0;
  uint8_t alpha = 0;
  uint8_t mode = 0;
  uint8_t brightness = 255;
  uint8_t smoothing = 0;

} Pattern_Data;

typedef struct{

  uint8_t length = 60;
  uint8_t loop_ms = 40; // approx. 10 hz
  uint8_t debug_mode = 0; // debug and simulator off
  bool init = true;

} Config_Data;


Pattern_Data load_slot(int slot);

void set_slot(int slot);

void save_to_nvs();

void clear_all();

void load_from_nvs();

void save_config_to_nvs();

#endif