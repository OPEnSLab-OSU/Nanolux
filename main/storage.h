#include <stdint.h>
#ifndef STORAGE_H
#define STORAGE_H

#define NUM_SAVES  3
#define NUM_SUBPATTERNS 4

typedef struct{

  uint8_t idx = 0;
  uint8_t brightness = 255;
  uint8_t smoothing = 0;
  uint8_t minhue =0;
  uint8_t maxhue =255;
  uint8_t config = 0; // diffrent configs
  bool reversed = false;
  bool mirrored = false;

} Subpattern_Data;

typedef struct{

  uint8_t alpha = 0;
  uint8_t noise_thresh = 0;
  uint8_t mode = 0;
  uint8_t subpattern_count = 1;
  Subpattern_Data subpattern[NUM_SUBPATTERNS];

} Pattern_Data;

typedef struct{

  uint8_t length = 60;
  uint8_t loop_ms = 40; // approx. 10 hz
  uint8_t debug_mode = 0; // debug and simulator off
  bool init = true;

} Config_Data;


void load_slot(int slot);

void set_slot(int slot);

void save_to_nvs();

void clear_all();

void load_from_nvs();

void save_config_to_nvs();

void verify_saves();

#endif