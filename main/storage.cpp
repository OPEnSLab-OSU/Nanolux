#include <string>
#include <Preferences.h>
#include "storage.h"

// PATTERN STORAGE
extern Pattern_Data current_pattern; // Currently loaded pattern
Pattern_Data saved_patterns[NUM_SAVES]; // Array of saved patterns

// PREFRENCES
char * NAMESPACE = "p";
char * KEY = "k";
Preferences storage;

// PRIVATE FUNCTIONS:
void clear_nvs(){
  storage.begin(NAMESPACE, false);
  storage.clear();
  storage.end();
}

// PUBLIC FUNCTIONS:
void load_slot(int slot){
  current_pattern = saved_patterns[slot];
}

void set_slot(int slot){
  saved_patterns[slot] = current_pattern;
}

void save_to_nvs(){
  storage.begin(NAMESPACE, false);
  storage.putBytes(KEY, &saved_patterns[0], sizeof(Pattern_Data) * NUM_SAVES);
  storage.end();
}

void clear_all(){
  clear_nvs();
  memset(&saved_patterns[0], 0, sizeof(Pattern_Data) * NUM_SAVES);
  save_to_nvs();
}

void load_from_nvs(){
  storage.begin(NAMESPACE, false);

  if(storage.isKey(KEY))
    storage.getBytes(KEY, &saved_patterns[0], sizeof(Pattern_Data) * NUM_SAVES);

  storage.end();
}
