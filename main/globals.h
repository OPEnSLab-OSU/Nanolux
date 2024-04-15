/** @file
  *
  * This file contains global variables.
  *
  * Many of the variables in the NanoLux project are global.
  * However, this means that thare are many variables defined
  * that have limited use outside of a few files.
  *
  * Variables that don't make sense to define in main.ino
  * but are used globally should be first defined here.
  *
*/

#ifndef GLOBALS_H
#define GLOBALS_H

#include "nanolux_types.h"

int formant_pose = 0;
double formants[3];  // Master formants array that constantly changes;
bool noise;          // Master Noisiness versus Periodic flag that is TRUE when noisy, FALSE when periodic;
bool drums[3];       // Master drums array that stores whether a KICK, SNARE, or CYMBAL is happening in each element of the array;
double fbs[5];       // Master FIVE BAND SPLIT which stores changing bands based on raw frequencies
double fss[5];       // Master FIVE SAMPLE SPLIT which stores changing bands based on splitting up the samples
int advanced_size = 20;
unsigned int sampling_period_us = round(1000000 / SAMPLING_FREQUENCY);
int F0arr[20];
int F1arr[20];
int F2arr[20];
unsigned long microseconds;
double vReal[SAMPLES];  // Sampling buffers
double vImag[SAMPLES];
double vRealHist[SAMPLES];  // Delta freq
double delt[SAMPLES];
double maxDelt = 0.;  // Frequency with the biggest change in amp.
unsigned long myTime;     // For nvp

//
// Patterns structure.
//
// Describes a pattern by name, whether it will be presented to the user in the
// web application and the function that implements the pattern.
//
typedef struct {
  int index;
  const char *pattern_name;
  bool enabled;
  void (*pattern_handler)(Pattern_History *, int);
} Pattern;

//
// Register all the patterns here. The index property must be sequential, but the code make sure
// that constraint is satisfied. The name is arbitrary, but keep it relatively short as it will be
// presented in a UI to the user. The enabled flag indicates whether the pattern will be shown
// in the UI or not. If not shown, it i snot selectable. If a pattern is not registered here,
// It will not be selectable and the loop below will not know about it.
//
Pattern mainPatterns[]{
  { 0, "None", true, blank },
  { 1, "Hue Trail", true, freq_hue_trail },
  { 2, "Volume Brightness", true, freq_hue_vol_brightness },
  { 3, "Confetti", true, freq_confetti_vol_brightness },
  { 4, "Volume Level Bar", true, volume_level_middle_bar_freq_hue },
  { 5, "Saturated Noise", true, saturated_noise },
  { 6, "Saturated Noise Octaves", true, saturated_noise_hue_octaves },
  { 7, "Saturated Noise Shift", true, saturated_noise_hue_shift },
  { 8, "Saturated Noise Compression", true, saturated_noise_compression },
  { 9, "Groovy Noise", true, groovy_noise },
  { 10, "Groovy Noise Shift", true, groovy_noise_hue_shift_change },
  { 11, "Sine Wave Trail", true, sin_hue_trail },
  { 12, "Hue Trail Mid", true, freq_hue_trail_mid },
  { 13, "Hue Trail Mid Blur", true, freq_hue_trail_mid_blur },
  { 14, "Talking Hue", true, talking_hue },
  { 15, "Talking Formants", true, talking_formants },
  { 16, "Talking Moving", true, talking_moving },
  { 17, "Bounce Back", true, bounce_back },
  { 18, "Glitch Sections", true, glitch_sections },
  { 19, "Glitch", true, glitch },
  { 20, "Glitch Talk", true, glitch_talk },
  { 21, "Spring Mass 1", true, spring_mass_1 },
  { 22, "Spring Mass 2", true, spring_mass_2 },
  { 23, "Spring Mass 3", true, spring_mass_3 },
  { 24, "Formant Test", true, formant_test },
  { 25, "Show Formants", true, show_formants },
  { 26, "Band Brightness", true, band_brightness },
  { 27, "Noisy", true, noisy },
  { 28, "Alt Drums", true, alt_drums },
  { 29, "Show Drums", true, show_drums },
  { 30, "Pixel Frequency", true, pix_freq },
  { 31, "Equalizer", true, eq },
  { 32, "Math", true, math },
  { 33, "Classical", true, classical },
  { 34, "Basic Bands", true, basic_bands },
  { 35, "Advanced Bands", true, advanced_bands },
  { 36, "Formant Band", true, formant_band },
  { 37, "Mirrored Pixel Frequency", true, mirror_pix_freq },
  { 38, "Tug O' War Frequency", true, tug_of_war_frequency },
  { 39, "Tug O' War Volume", true, tug_of_war_volume },
  { 40, "Random Raindrop", true, random_raindrop }
};
int NUM_PATTERNS = 41;  // MAKE SURE TO UPDATE THIS WITH THE ACTUAL NUMBER OF PATTERNS (+1 last array pos)

#endif