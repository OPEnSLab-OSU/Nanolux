#include <stdint.h>
#ifndef STORAGE_H
#define STORAGE_H

#define NUM_SAVES  3
#define NUM_SUBPATTERNS 4

// will need to update the idx when pattern is set
typedef struct{

  uint8_t idx = 0;
  uint8_t config =0;
  uint8_t noise_thresh = 0;
  uint8_t brightness = 255;
  uint8_t smoothing = 0;
  uint8_t minhue =0;
  uint8_t maxhue =0;
  uint8_t direction;


} Pattern_Data;

typedef struct{

  uint8_t length = 60;
  uint8_t loop_ms = 40; // approx. 10 hz
  uint8_t debug_mode = 0; // debug and simulator off
  uint8_t spc[NUM_SUBPATTERNS] = {1, 1, 1, 1};
  uint8_t alphas[NUM_SUBPATTERNS] = {0, 0, 0, 0};
  uint8_t modes[NUM_SUBPATTERNS] = {0, 0, 0, 0};
  bool init = true;

} Config_Data;


void load_slot(int slot);

void set_slot(int slot);

void save_to_nvs();

void clear_all();

void load_from_nvs();

void save_config_to_nvs();

#endif