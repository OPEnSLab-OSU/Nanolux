#include <stdint.h>
#ifndef STORAGE_H
#define STORAGE_H

#define NUM_SAVES  5

typedef struct{

  uint8_t pattern_1 = 0; // Index number of which pattern is current
  uint8_t pattern_2 = 0;
  uint8_t noise_thresh = 0;
  uint8_t alpha = 0;
  uint8_t mode = 0;

} Pattern_Data;

void load_slot(int slot);

void set_slot(int slot);

void save_to_nvs();

void clear_all();

void load_from_nvs();

#endif