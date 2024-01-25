#include <string>
#include <Preferences.h>
#include "storage.h"

// PATTERN STORAGE
extern Pattern_Data current_pattern; // Currently loaded pattern
Pattern_Data saved_patterns[NUM_SAVES]; // Array of saved patterns
extern Config_Data config; // Currently loaded config
volatile extern bool altered_config;

// PREFRENCES
char * PATTERN_NAMESPACE = "p";
char * PATTERN_KEY = "k";
char * CONFIG_NAMESPACE = "c";
char * CONFIG_KEY = "f";
Preferences storage;

// PRIVATE FUNCTIONS:
void clear_nvs(){
  storage.begin(PATTERN_NAMESPACE, false);
  storage.clear();
  storage.end();
}

// PUBLIC FUNCTIONS:
Pattern_Data load_slot(int slot){
  return saved_patterns[slot];
}

void set_slot(int slot){
  saved_patterns[slot] = current_pattern;
}

void save_to_nvs(){
  storage.begin(PATTERN_NAMESPACE, false);
  storage.putBytes(PATTERN_KEY, &saved_patterns[0], sizeof(Pattern_Data) * NUM_SAVES);
  storage.end();
}

void clear_all(){
  clear_nvs();
  memset(&saved_patterns[0], 0, sizeof(Pattern_Data) * NUM_SAVES);
  save_to_nvs();
}

void save_config_to_nvs(){
  storage.begin(CONFIG_NAMESPACE, false);
  storage.putBytes(CONFIG_KEY, &config, sizeof(Config_Data));
  storage.end();
}

void load_from_nvs(){
  storage.begin(PATTERN_NAMESPACE, false);

  if(storage.isKey(PATTERN_KEY))
    storage.getBytes(PATTERN_KEY, &saved_patterns[0], sizeof(Pattern_Data) * NUM_SAVES);

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
  }
}
