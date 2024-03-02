#ifndef PATTERNS_H
#define PATTERNS_H

#include "nanolux_types.h"
#include "storage.h"

typedef struct{

  // Pattern Buffer for the particular history being used.
  CRGB leds[MAX_LEDS] = {0};

  // History Variables
  int frame = 0;                 // for spring mass
  double amplitude = 0;          //for spring mass 2
  int tempHue = 0;
  int vol_pos = 0;
  int pix_pos = 0;
  uint8_t genre_smoothing[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int genre_pose = 0;
  double max1[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  double max2[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  double max3[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  double max4[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  double max5[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  int maxIter = 0;
  double velocity = 0;
  double acceleration = 0;
  double smoothing_value[10] = {0,0,0,0,0,0,0,0,0,0};
  int location = 70;
  double velocities[5] = {0,0,0,0,0};
  double accelerations[5] = {0,0,0,0,0};
  int locations[5] = {70,60,50,40,30};
  double vRealSums[5] = {0,0,0,0,0};

} Pattern_History;

extern Subpattern_Data params;

void nextPattern();

void clearLEDSegment(Pattern_History * hist, int len);

void setColorHSV(CRGB* leds, byte h, byte s, byte v, int len);

void blank(Pattern_History * hist, int len, Subpattern_Data* params);

/*
void confetti(Pattern_History * hist, int len, Subpattern_Data* params);
*/
void pix_freq(Pattern_History * hist, int len, Subpattern_Data* params);
/*
void groovy_noise(Pattern_History* hist, int len, Subpattern_Data* params);

void hue_trail(Pattern_History* hist, int len, Subpattern_Data* params);

void talking(Pattern_History *hist, int len, Subpattern_Data *params);

void glitch_effect(Pattern_History * hist, int len, Subpattern_Data * params);
*/
void processDirection(Pattern_History* hist, Subpattern_Data * params, int len);


//void echo_ripple(Pattern_History hist, int len);

#endif