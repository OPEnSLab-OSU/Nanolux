#include <FastLED.h>
#include <Arduino.h>
#include "arduinoFFT.h"
#include "patterns.h"
#include "nanolux_types.h"
#include "nanolux_util.h"
#include "palettes.h"
#include "audio_analysis.h"
#include "storage.h"

extern unsigned int sampling_period_us;
extern unsigned long microseconds;
extern double vReal[SAMPLES];      // Sampling buffers
extern double vImag[SAMPLES];
extern double vRealHist[SAMPLES];  // for delta freq
extern double delt[SAMPLES];
extern arduinoFFT FFT;
extern bool button_pressed;
extern SimplePatternList gPatterns;
extern int NUM_PATTERNS;
extern SimplePatternList gPatterns_layer;
extern uint8_t gHue;                      // rotating base color
extern double peak;                       //  peak frequency
extern uint8_t fHue;                      // hue value based on peak frequency
extern double volume;                     //  NOOOOTEEEE:  static?? 
extern uint8_t vbrightness;
extern double maxDelt;                    // Frequency with the biggest change in amp.
extern bool vol_show; // A boolean to change if not wanting to see the color changing pixel in pix_freq()
extern int advanced_size;
CRGBPalette16 gPal = GMT_hot_gp; //store all palettes in array
CRGBPalette16 gPal2 = nrwc_gp; //store all palettes in array
bool gReverseDirection = false;

extern Config_Data config; // Currently loaded config
extern Subpattern_Data params;

// get frequency hue
// void getFhue(uint8_t min_hue, uint8_t max_hue){
//     fHue = remap(
//     log(peak) / log(2),
//     log(MIN_FREQUENCY) / log(2),
//     log(MAX_FREQUENCY) / log(2),
//     min_hue, max_hue);
// }

void getFhue(){
    fHue = remap(
    log(peak) / log(2),
    log(MIN_FREQUENCY) / log(2),
    log(MAX_FREQUENCY) / log(2),
    10, 240);
}

/// get vol brightness
void getVbrightness(){
    vbrightness = remap(
    volume,
    MIN_VOLUME,
    MAX_VOLUME,
    0,
    MAX_BRIGHTNESS);
}


void nextPattern() {
  // add one to the current pattern number, and wrap around at the end
  //current_pattern.pattern_1 = (current_pattern.pattern_1 + 1) % NUM_PATTERNS;
}

void clearLEDSegment(Pattern_History * hist, int len){
  for(int i = 0; i < len; i++)
    hist->leds[i] = CRGB(0,0,0);
}

void blank(Pattern_History * hist, int len, Subpattern_Data* params){
  clearLEDSegment(hist, len);
}

void setColorHSV(CRGB* leds, byte h, byte s, byte v, int len) {
  // create a new HSV color
  CHSV color = CHSV(h, s, v);
  // use FastLED to set the color of all LEDs in the strip to the same color
  fill_solid(leds, len, color);
}


/// @brief Based on a sufficient volume, a pixel will float to some position on the light strip 
///        and fall down (vol_show adds another threshold)
/// @param hist Pointer to the Pattern_History structure, holds LED buffer and history variables.
/// @param len The length of LEDs to process
/// @param params Pointer to Subpattern_Data structure containing configuration options.
void pix_freq(Pattern_History * hist, int len, Subpattern_Data* params) {
    //switch(params->config){
      //case 0:
      //default:
    getFhue();
    fadeToBlackBy(hist->leds, len, 50);
    if (volume > 200) {
      hist->pix_pos = map(peak, MIN_FREQUENCY, MAX_FREQUENCY, 0, len-1);
      hist->tempHue = fHue;
    }
    else {
      hist->pix_pos--;
      hist->tempHue--;
      hist->vol_pos--;
    }
    if (vol_show) {
      if (volume > 100) {
        hist->vol_pos = map(volume, MIN_VOLUME, MAX_VOLUME, 0, len-1);
        hist->tempHue = fHue;
      } else {
        hist->vol_pos--;
      }

      hist->leds[hist->vol_pos] = hist->vol_pos < len ? CRGB(255, 255, 255):CRGB(0, 0, 0);
    }
    hist->leds[hist->pix_pos] = hist->pix_pos < len ? CHSV(hist->tempHue, 255, 255):CRGB(0, 0, 0);
}

/// @brief Confetti effect using frequency and brightness.
///        Colored speckles that blink and fade smoothly are scattered across the strip.
/// @param hist Pointer to the Pattern_History structure, holds LED buffer and history variables.
/// @param len The length of LEDs to process
/// @param params Pointer to Subpattern_Data structure containing configuration options.
void confetti(Pattern_History * hist, int len, Subpattern_Data* params){
  // colored speckles based on frequency that blink in and fade smoothly
  fadeToBlackBy(hist->leds, len, 20);
  int pos = random16(len);
  switch(params->config){
      case 0:
      default:
      hist->leds[pos] += CHSV( fHue + random8(10), 255, vbrightness);
      hist->leds[pos] += CHSV( fHue + random8(10), 255, vbrightness);
  }
}

/// @brief  Outputs a steady moving stream of lights where each pixel correlates to a previous fHue value.
///          Visually tracks pitch over time, with brightness determined by volume.
///           blur configuration adds blur
/// @param hist Pointer to the Pattern_History structure, holds LED buffer and history variables.
/// @param len The length of LEDs to process
/// @param params Pointer to Subpattern_Data structure containing configuration options.
void hue_trail(Pattern_History* hist, int len, Subpattern_Data* params) {
    switch (params->config) {
        case 0: // freq_hue_trail (also default case)
        default: // Default case set to execute the freq_hue_trail pattern
            hist->leds[0] = CHSV(fHue, 255, params->brightness);
            hist->leds[1] = CHSV(fHue, 255, params->brightness);
            for (int i = len - 1; i > 1; i -= 2) {
                hist->leds[i] = hist->leds[i - 2];
                hist->leds[i - 1] = hist->leds[i - 2];
            }
            break;

        case 1: // blur
            {
            hist->leds[0] = CHSV(fHue, 255, params->brightness);
            hist->leds[1] = CHSV(fHue, 255, params->brightness);
            for (int i = len - 1; i > 1; i -= 2) {
                hist->leds[i] = hist->leds[i - 2];
                hist->leds[i - 1] = hist->leds[i - 2];
            }
            blur1d(hist->leds, len, 20);
            break;
    }

  }
}

/// @brief  Fills the light strip with a nice ambient mess of colors that shift slowly over time. 
///         This function is similar to grovvy noise except the scale and hue_shift values are quiote different.
///         Hue Octave Config remaps the volume to the range of hues present on strip.
///         Hue Shift Config remaps volume as octaves and hhue_shift parameters in fill_noise16()
///         Noise Compression config rremaps volume as hue_x parameter
/// @param hist Pointer to the Pattern_History structure, holds LED buffer and history variables.
/// @param len The length of LEDs to process
/// @param params Pointer to Subpattern_Data structure containing configuration options.
void saturated(Pattern_History* hist, int len, Subpattern_Data* params){
  //Set params for fill_noise16()
  uint8_t octaves = 1;
  uint16_t x = 0;
  int scale = 300;
  uint8_t hue_octaves = 1;
  uint16_t hue_x = 100;
  int hue_scale = 20; 
  uint16_t ntime = millis() / 3;
  uint8_t hue_shift =  50;
   switch (params->config) {
        case 0: // Default, no additional values changed
          break;
        case 1: { // Hue octaves 
            hue_octaves = remap(volume, MIN_VOLUME, MAX_VOLUME, 1, 10);
            }
            break;
            
        case 2: {// Hue shift 
            octaves = remap(volume, MIN_VOLUME, MAX_VOLUME, 50, 100);
            hue_shift = remap(volume, MIN_VOLUME, MAX_VOLUME, 50, 100);
            scale = 230;
            hue_x = 150;
            }
            break;
        case 3:{ // Compression
            hue_x = remap(volume, MIN_VOLUME, MAX_VOLUME, 1, 8);
            ntime = millis() / 4;
            }
            break;
  //Fill LEDS with noise using parameters above
  fill_noise16 (hist->leds, len, octaves, x, scale, hue_octaves, hue_x, hue_scale, ntime, hue_shift);
  //Add blur
  blur1d(hist->leds, len, 80);
  }
}

/// @brief   A cool fluctuating pattern that changes color in waves of greens, yellows, purples and blue. 
///       This function is similar to saturated_noise but the values of scale and hue_shift are 100 and 5 respectively. 
///       This is a moving pattern but it does not change based on and volume or frequency changes. Uses fill_noise16() and blur.
///       Hue Shift Change configuration remaps volume variable as hue_shift.
/// @param hist Pointer to the Pattern_History structure, holds LED buffer and history variables.
/// @param len The length of LEDs to process
/// @param params Pointer to Subpattern_Data structure containing configuration options.
void groovy(Pattern_History* hist, int len, Subpattern_Data* params) {
    // Assume global access to necessary variables like volume, or pass them as parameters
    switch (params->config) {
        case 0: // groovy_noise (also default case)
        default:
            {
                uint8_t octaves = 1;
                uint16_t x = 0;
                int scale = 100;
                uint8_t hue_octaves = 1;
                uint16_t hue_x = 1;
                int hue_scale = 50;
                uint16_t ntime = millis() / 3;
                uint8_t hue_shift =  5;
  
                fill_noise16(hist->leds, len, octaves, x, scale, hue_octaves, hue_x, hue_scale, ntime, hue_shift);
                blur1d(hist->leds, len, 80);
            }
            break;

        case 1: // Hue Shift Change
            {
                int shiftFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 5, 220);
                int xFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 1, 2);

                uint8_t octaves = 1;
                uint16_t x = 0;
                int scale = 100;
                uint8_t hue_octaves = 1;
                uint16_t hue_x = xFromVolume;
                int hue_scale = 20;
                uint16_t ntime = millis() / 3;
                uint8_t hue_shift =  shiftFromVolume;

                fill_noise16(hist->leds, len, octaves, x, scale, hue_octaves, hue_x, hue_scale, ntime, hue_shift);
                blur1d(hist->leds, len, 80);
            }
            break;
    }
}

/// @brief   Generates three clusters of lights, one in the middle, and two symmetric ones that travel out from the center and return. 
///         The distance the two outer LEDS from the center is determined by the detected volume.
///         Formant configuration takes the clusters to be red,green,blue.
///         Moving configuration, the three clusters move up and down according to sine wave motion.
/// @param hist Pointer to the Pattern_History structure, holds LED buffer and history variables.
/// @param len The length of LEDs to process
/// @param params Pointer to Subpattern_Data structure containing configuration options.
void talking(Pattern_History *hist, int len, Subpattern_Data *params) {
  // Common variables
  int offsetFromVolume;
  int midpoint = len / 2;

  switch (params->config) {
    case 1: { // Formants
      double *formants = density_formant();
      double f0Hue = remap(formants[0], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255);
      double f1Hue = remap(formants[1], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255);
      double f0 = remap(formants[0], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255);
      double f1 = remap(formants[1], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255);
      double f2 = remap(formants[2], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255);

      offsetFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 1, 30);
      hist->leds[len / 2] = CRGB(f0, f1, f2);
      hist->leds[len / 2 - offsetFromVolume] = CHSV(f0Hue, 255, MAX_BRIGHTNESS);
      hist->leds[len / 2 + offsetFromVolume] = CHSV(f0Hue, 255, MAX_BRIGHTNESS);

      // Reset formant array for next loop, assuming dynamic allocation
      delete[] formants;
    } break;

    case 2: { // Moving
      offsetFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 0, 12500);

      uint16_t sinBeat0 = beatsin16(5, 2, len - 3, 0, 250);
      uint16_t sinBeat1 = beatsin16(5, 2, len - 3, 0, 0 - offsetFromVolume);
      uint16_t sinBeat2 = beatsin16(5, 2, len - 3, 0, 750 + offsetFromVolume);

      hist->leds[sinBeat0] = CHSV(fHue + 100, 255, MAX_BRIGHTNESS);
      hist->leds[sinBeat1] = CHSV(fHue, 255, MAX_BRIGHTNESS);
      hist->leds[sinBeat2] = CHSV(fHue, 255, MAX_BRIGHTNESS);
    } break;

    default: // Talking Hue
    case 0: 
      hist->leds[midpoint] = CHSV(fHue / 2, 255, MAX_BRIGHTNESS);
      hist->leds[midpoint - offsetFromVolume] = CHSV(fHue, 255, MAX_BRIGHTNESS);
      hist->leds[midpoint + offsetFromVolume] = CHSV(fHue, 255, MAX_BRIGHTNESS);
      break;
  }

  // Common effects for all modes
  blur1d(hist->leds, len, 80);
  // Adjust fade value based on the pattern
  int fadeValue = (params->config == 0 || params->config == 2) ? 150 : (params->config == 1) ? 200 : 100;
  fadeToBlackBy(hist->leds, len, fadeValue);
}

/// @brief  Creates two light clusters that move according to sine wave motion, but their speed is affected by the volume. 
///         One pulls its color from fHue, and the other pulls its color from the formant values.
///         Talk configuration combines glitch with talking_moving().
///         Sections configuration creates 4 seperate sine wave clusters.
/// @param hist Pointer to the Pattern_History structure, holds LED buffer and history variables.
/// @param len The length of LEDs to process
/// @param params Pointer to Subpattern_Data structure containing configuration options.
void glitch(Pattern_History * hist, int len, Subpattern_Data * params ) {
    int offsetFromVolume, speedFromVolume;
    uint16_t sinBeat[4]; 
    double *formants;
    double f0Hue;
    
    speedFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 5, params->config == 0 ? 25 : 20);
    switch (params->config) {
        case 0:
            sinBeat[0] = beatsin16(speedFromVolume, 0, len-1, 0, 0);
            sinBeat[1] = beatsin16(speedFromVolume, 0, len-1, 0, 32767);
            formants = density_formant();
            f0Hue = remap(formants[0], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255);
            break;
        case 1: // glitch_talk
            offsetFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 0, 20000);
            for (int i = 0; i < 3; ++i) {
                sinBeat[i] = beatsin16(speedFromVolume, 3, len-4, 0, i == 0 ? 250 : (i == 1 ? 0 - offsetFromVolume : 750 + offsetFromVolume));
            }
            break;
        case 2: // glitch_sections
            offsetFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 0, 10000);
            for (int i = 0; i < 4; ++i) {
                sinBeat[i] = beatsin16(6, 0, len-1, 0, i * 16384 - offsetFromVolume);
            }
            break;
    }
    // Common effects: blur and fade
    blur1d(hist->leds, len, 80);
    fadeToBlackBy(hist->leds, len, params->config == 1 ? 100 : (params->config == 2 ? 60 : 40)); 
}

/// @brief  Basic band config : Uses the band_split_bounce() function to generate a five band split, and maps that split to the light strip. The strip is broken into five chunks of different colors, 
///         where the volume of each band determines how much of each section of the LED strip is lit.
///         Advanced bands config : he strip is broken into five chunks of different colors, where the volume of each band determines how much of each section is lit, and that portion will diminish over time if a certain volume threshold is not met
///         Fomant bands config: emonstrates the formant feature of the audio analysis code. Each of the three formant values correspond to a third of the entire LED strip, where the individual formant values determine the hue of each third.
/// @param hist Pointer to the Pattern_History structure, holds LED buffer and history variables.
/// @param len The length of LEDs to process
/// @param params Pointer to Subpattern_Data structure containing configuration options.
void bands(Pattern_History* hist, int len, Subpattern_Data* params) {
  
}

/// @brief Short and sweet function. Each pixel corresponds to a value from vReal, 
///         where the volume at each pitch determines the brightness of each pixel. Hue is locked in to a rainbow.
/// @param hist Pointer to the Pattern_History structure, holds LED buffer and history variables.
/// @param len The length of LEDs to process
/// @param params Pointer to Subpattern_Data structure containing configuration options.
void eq(Pattern_History * hist, int len, Subpattern_Data* params) {
  
  for (int i = 0; i < len; i++) {
    int brit = map(vReal[i], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255); // The brightness is based on HOW MUCH of the frequency exists
    int hue = map(i, 0, len, 0, 255); // The fue is based on position on the light strip, ergo, what frequency it is at
    if (vReal[i] > 200) { // An extra gate because the frequency array is really messy without it
      hist->leds[i] = CHSV(hue, 255, brit);
    }
  }
}

/// @brief A random spot is chosen along the length and does a ripple based on frequency
/// @param hist Pointer to the Pattern_History structure, holds LED buffer and history variables.
/// @param len The length of LEDs to process
/// @param params Pointer to Subpattern_Data structure containing configuration options.
void random_raindrop(Pattern_History * hist, int len, Subpattern_Data* params){
    int startIdx = random(len);

    hist->leds[startIdx] = CHSV(fHue, 255, vbrightness);
    
    for(int i = len-1; i > 0; i--) {
      if (i != startIdx) {
        hist->leds[i] = hist->leds[i-1];
      }
    }

    hist->leds[0] = CRGB::Black;
}

/// @brief Strip is split into two sides, red and blue showing push and pull motion 
///         based on either frequency or volume
/// @param hist Pointer to the Pattern_History structure, holds LED buffer and history variables.
/// @param len The length of LEDs to process
/// @param params Pointer to Subpattern_Data structure containing configuration options.
void tug_of_war(Pattern_History * hist, int len, Subpattern_Data* params) {
    int splitPosition = remap(volume, MIN_VOLUME, MAX_VOLUME, 0, len);
    //use this function with smoothing for better results
    // red is on the left, blue is on the right
    for (int i = 0; i < len; i++) {
        if (i < splitPosition) {
            hist->leds[i] = CRGB::Green; 
        } else {
            hist->leds[i] = CRGB::Blue;
        }
    }
}





