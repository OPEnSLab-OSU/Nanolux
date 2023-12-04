#include <string>
#include <Preferences.h>

// SAVED PATTERN STRUCT: 6 BYTES
struct Saved_Pattern{
  uint8_t id1 = 0;
  uint8_t id2 = 0;
  uint8_t mode = 0;
  uint8_t noise = 0;
  uint8_t alpha = 0;
};

// PATTERN STATE VARIABLES
extern volatile uint8_t gCurrentPatternNumber; // Index number of which pattern is current
extern volatile uint8_t gCurrentPatternNumber2;
extern volatile uint8_t gNoiseGateThreshold;
extern volatile int alpha;
extern volatile int current_mode;
int current_slot = 0;

// PATTERN SAVING VARIABLES
Saved_Pattern saved_patterns[5];
Preferences storage;

void load_test_pattern(){
  gCurrentPatternNumber = 1;
  gCurrentPatternNumber2 = 3;
  gNoiseGateThreshold = 25;
  alpha = 0;
  current_mode = 1;
}

void save_pattern(int slot){
  saved_patterns[slot].id1 = gCurrentPatternNumber;
  saved_patterns[slot].id2 = gCurrentPatternNumber2;
  saved_patterns[slot].mode = current_mode;
  saved_patterns[slot].noise = gNoiseGateThreshold;
  saved_patterns[slot].alpha = alpha;

  storage.begin("stored-patterns", false);
  storage.putBytes("p" + slot, &saved_patterns[slot], sizeof(Saved_Pattern));
  storage.end();
}

// Loads the pattern currently saved in the given slot.
void load_pattern(int slot){
  gCurrentPatternNumber = saved_patterns[slot].id1;
  gCurrentPatternNumber2 = saved_patterns[slot].id2;
  gNoiseGateThreshold = saved_patterns[slot].noise;
  alpha = saved_patterns[slot].noise;
  current_mode = saved_patterns[slot].mode;
}

// Load all pattern data out of storage, then close storage.
// All patterns are stored separately to prevent from having to
// overwrite all patterns just to save one.
void load_all_patterns(){
  storage.begin("stored-patterns", true);
  for(int i = 0; i < 5; i++)
    storage.getBytes("p" + i, &saved_patterns[i], sizeof(Saved_Pattern));
  storage.end();
}