#include <string>
#include <Preferences.h>
#include "storage.h"

// CURRENTLY LOADED CONFIG
// The currently loaded configuration is not saved,
// and overwrites other values when saved.
extern Pattern_Data current_subpatterns[NUM_SUBPATTERNS];
extern uint8_t subpattern_count;

// VOLATILE SAVED CONFIGS
// These volatile configurations are not written to NVS,
// and require a call to save_nvs() to do so.
extern Pattern_Data vol_subpatterns[NUM_SUBPATTERNS * NUM_SAVES];
uint8_t vol_subpattern_counts[NUM_SUBPATTERNS] = {1, 1, 1, 1};

// NVS STORAGE
char * PATTERN_NAMESPACE = "p";
char * PATTERN_KEY = "k";
char * CONFIG_NAMESPACE = "c";
char * CONFIG_KEY = "f";
Preferences storage;

// ADDITIONAL CONFIGURATIONS
extern Config_Data config;

// PRIVATE FUNCTIONS:
void clear_nvs(){
  storage.begin(PATTERN_NAMESPACE, false);
  storage.clear();
  storage.end();
}

// PUBLIC FUNCTIONS:

// Clears the unsaved currently loaded config and replaces it with the
// requested one.
void load_slot(int slot){

  // Find the index of the first subpattern of the slot we are loading
  uint8_t start = 0;
  for(int i = 0; i < slot - 1; i++) start += vol_subpattern_counts[i];

  // Clear the current pattern and load in the specified one.
  memset(&current_subpatterns[0], 0, sizeof(current_subpatterns));
  memcpy(&current_subpatterns[0], &vol_subpatterns[start + 1], vol_subpattern_counts[slot]);
}

void set_slot(int slot){

  // Make a temporary volatile subpatterns array.
  Pattern_Data temp[NUM_SUBPATTERNS * NUM_SAVES];
  
  // Calculate when to stop copying directly from the array.
  uint8_t end = 0;
  for(int i = 0; i < slot - 1; i++) end += vol_subpattern_counts[i];

  // Copy elements directly from the old array to the temp array.
  memcpy(&temp[0], &vol_subpatterns[0], sizeof(Pattern_Data) * end);

  // Copy the currently-loaded subpatterns in
  memcpy(
    &temp[end + 1],
    &current_subpatterns[0],
    sizeof(Pattern_Data) * subpattern_count
  );

  // Copy the rest of the save slots pattern's in
  memcpy(
    &temp[end + subpattern_count + 1],
    &vol_subpatterns[end + 1],
    sizeof(Pattern_Data) * NUM_SUBPATTERNS * NUM_SAVES
  );

  // Update the number of subpatterns.
  vol_subpattern_counts[slot] = subpattern_count;
}

Pattern_Data get_slot_subpattern(int slot, int idx){

  uint8_t end = 0;
  for(int i = 0; i < slot - 1; i++) end += vol_subpattern_counts[i];

  return vol_subpatterns[end + idx];
}

void save_to_nvs(){
  storage.begin(PATTERN_NAMESPACE, false);
  storage.putBytes(
    PATTERN_KEY,
    &vol_subpatterns[0],
    sizeof(Pattern_Data) * NUM_SUBPATTERNS * NUM_SAVES
  );
  storage.end();
}

void clear_all(){
  clear_nvs();
  memset(
    &vol_subpatterns[0],
    0,
    sizeof(Pattern_Data) * NUM_SUBPATTERNS * NUM_SAVES
  );
  save_to_nvs();
}

void save_config_to_nvs(){
  storage.begin(CONFIG_NAMESPACE, false);
  storage.putBytes(CONFIG_KEY, &config, sizeof(Config_Data));
  storage.end();
}

void load_from_nvs(){
  storage.begin(PATTERN_NAMESPACE, false);

  if(storage.isKey(PATTERN_KEY)){
    storage.getBytes(
      PATTERN_KEY,
      &vol_subpatterns[0],
      sizeof(Pattern_Data) * NUM_SUBPATTERNS * NUM_SAVES
    );
  }

  storage.end();

  storage.begin(CONFIG_NAMESPACE, false);

  if(storage.isKey(CONFIG_KEY))
    storage.getBytes(CONFIG_KEY, &config, sizeof(Config_Data));

  storage.end();

  if(!config.init){
    config.init = true;
    config.debug_mode = 0;
    config.length = 60;
    config.loop_ms = 40;

    for(int i = 0; i < NUM_SUBPATTERNS; i++){
      config.spc[i] = 1;
    }
  }
}
