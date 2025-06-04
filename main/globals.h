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
#include "audio_analysis.h"
#include "AudioPrism.h"

int formant_pose = 0;
double formants[3];  // Master formants array that constantly changes;
bool noise;          // Master Noisiness versus Periodic flag that is TRUE when noisy, FALSE when periodic;
bool drums[3];       // Master drums array that stores whether a KICK, SNARE, or CYMBAL is happening in each element of the array;
double fbs[5];       // Master FIVE BAND SPLIT which stores changing bands based on raw frequencies
double fss[5];       // Master FIVE SAMPLE SPLIT which stores changing bands based on splitting up the samples
int salFreqs[3];
float centroid;
float noisiness;
bool percussionPresent = false;
int advanced_size = 20;
unsigned int sampling_period_us = round(1000000 / SAMPLING_FREQUENCY);
int F0arr[20];
int F1arr[20];
int F2arr[20];
unsigned long microseconds;
double vReal[SAMPLES];  // Sampling buffers
double vImag[SAMPLES];
double vRealHist[SAMPLES];  // Delta freq
float audioPrismInput[SAMPLES];
extern Spectrogram fftHistory;
double delt[SAMPLES];
double maxDelt = 0.;  // Frequency with the biggest change in amp.
unsigned long myTime;     // For nvp
AudioAnalysis audioAnalysis;


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
  void (*pattern_handler)(Strip_Buffer * buf, int len, Pattern_Data* params);
} Pattern;

//
// Register all the patterns here. The index property must be sequential, but the code make sure
// that constraint is satisfied. The name is arbitrary, but keep it relatively short as it will be
// presented in a UI to the user. The enabled flag indicates whether the pattern will be shown
// in the UI or not. If not shown, it i snot selectable. If a pattern is not registered here,
// It will not be selectable and the loop below will not know about it.
//
Pattern mainPatterns[]{
    { 0, "None", true, blank},
    { 1, "Equalizer", true, eq},
    { 2, "Bands", true, bands},
    { 3, "Splash Theory", true, splashTheory},
    { 4, "deltaHeat", true, deltaHeat},
    { 5, "Fire 2012", true, Fire2012},
    { 6, "Pixel Frequency", true, pix_freq},
    { 7, "Confetti", true, confetti},
    { 8, "Hue Trail", true, hue_trail},
    { 9, "Saturated", true, saturated},
    { 10, "Groovy", true, groovy},
    { 11, "Talking", true, talking},
    { 12, "Glitch", true, glitch},
    { 13, "Rain Drop", true, random_raindrop},
    { 14, "Bar Fill", true, bar_fill},
    { 15, "Blend In", true, blendIn},
    { 16, "Bleed Through", true, bleedThrough},
    { 17, "Synesthesia Rolling", true, synesthesiaRolling},
    { 18, "Note EQ", true, noteEQ},
    { 19, "String Theory", true, stringTheory},
};
int NUM_PATTERNS = 20;  // MAKE SURE TO UPDATE THIS WITH THE ACTUAL NUMBER OF PATTERNS (+1 last array pos) pos)

#endif