/**@file
 *
 * This file contains function headers for patterns.cpp
 * along with the Pattern_History struct definition.
 *
**/

#ifndef PATTERNS_H
#define PATTERNS_H

#include "nanolux_types.h"

/// @brief Holds persistent data for currently-running patterns.
///
/// This structure contains both persistent variables that can
/// be reused on a per-subpattern basis.
///
/// It also contains the LED buffer for that subpattern. The main
/// advantage of defining it here is that each subpattern can have
/// an independent pattern buffer separate from the main ones
/// in main.ino.
///
/// When a subpattern is modified in a way that requires a reset
/// (changing the subpattern name, changing LED length), the
/// existing pattern history should be replaced with the default
/// structure.
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

void nextPattern();

void setColorHSV(CRGB* leds, byte h, byte s, byte v, int len);

void freq_hue_vol_brightness(Pattern_History * hist, int len);

void freq_confetti_vol_brightness(Pattern_History * hist, int len);

void volume_level_middle_bar_freq_hue(Pattern_History * hist, int len);

void freq_hue_trail(Pattern_History * hist, int len);

void blank(Pattern_History * hist, int len);

void spring_mass_1(Pattern_History * hist, int len);

void spring_mass_2(Pattern_History * hist, int len);

void spring_mass_3(Pattern_History * hist, int len);

void classical(Pattern_History * hist, int len);

void pix_freq(Pattern_History * hist, int len);

void mirror_pix_freq(Pattern_History * hist, int len);

void send_wave(Pattern_History * hist, int len);

void math(Pattern_History * hist, int len);

void band_brightness(Pattern_History * hist, int len);

void advanced_bands(Pattern_History * hist, int len);

void basic_bands(Pattern_History * hist, int len);

void eq(Pattern_History * hist, int len);

void show_drums(Pattern_History * hist, int len);

void show_formants(Pattern_History * hist, int len);

void noisy(Pattern_History * hist, int len);

void formant_band(Pattern_History * hist, int len);

void alt_drums(Pattern_History * hist, int len);

void formant_test(Pattern_History * hist, int len);

void Fire2012WithPalette(Pattern_History * hist, int len);

void saturated_noise(Pattern_History * hist, int len);

void saturated_noise_hue_octaves(Pattern_History * hist, int len);

void saturated_noise_hue_shift(Pattern_History * hist, int len);

void saturated_noise_compression(Pattern_History * hist, int len);

void groovy_noise(Pattern_History * hist, int len);

void groovy_noise_hue_shift_change(Pattern_History * hist, int len);

void sin_hue_trail(Pattern_History * hist, int len);

void freq_hue_trail_mid(Pattern_History * hist, int len);

void freq_hue_trail_mid_blur(Pattern_History * hist, int len);

void talking_hue(Pattern_History * hist, int len);

void talking_formants(Pattern_History * hist, int len);

void talking_moving(Pattern_History * hist, int len);

void bounce_back(Pattern_History * hist, int len);

void glitch(Pattern_History * hist, int len);

void glitch_talk(Pattern_History * hist, int len);

void glitch_sections(Pattern_History * hist, int len);

void tug_of_war_frequency(Pattern_History * hist, int len);

void tug_of_war_volume(Pattern_History * hist, int len);

void random_raindrop(Pattern_History * hist, int len);

#endif