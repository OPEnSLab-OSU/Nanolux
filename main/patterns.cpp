/**@file
 *
 * This file contains all defined pattern handlers.
 *
 * The pattern refactor is still in-progress, so this file
 * currently has little in-line documentation.
 *
**/

#include <FastLED.h>
#include <Arduino.h>
#include "patterns.h"
#include "nanolux_types.h"
#include "nanolux_util.h"
#include "storage.h"
#include "audio_analysis.h"
#include "palettes.h"

extern unsigned long microseconds;
extern AudioAnalysis audioAnalysis; // Audio Analysis object with getters
extern bool button_pressed;
extern SimplePatternList gPatterns;
extern int NUM_PATTERNS;
extern SimplePatternList gPatterns_layer;
extern double a;                       //  a frequency
extern uint8_t fHue;                      // hue value based on a frequency
extern uint8_t vbrightness;
extern int advanced_size;
CRGBPalette16 gPal = GMT_hot_gp; //store all palettes in array
bool gReverseDirection = false;

extern Config_Data config; // Currently loaded config
extern Pattern_Data params;

extern uint8_t manual_pattern_idx;
extern bool manual_control_enabled;

/// Global formant array, used for accessing.
extern double formants[3];

uint8_t max_hue = 255;
uint8_t min_hue = 0;


// get frequency hue
void getFhue(uint8_t tmin_hue, uint8_t tmax_hue){
  fHue = remap(
  log(audioAnalysis.getPeak()) / log(2),
  log(MIN_FREQUENCY) / log(2),
  log(MAX_FREQUENCY) / log(2),
  tmin_hue, tmax_hue);
  // disable min hue and max hue
  // 10, 240);
}

/// get vol brightness
void getVbrightness(){
    vbrightness = remap(
    audioAnalysis.getVolume(),
    MIN_VOLUME,
    MAX_VOLUME,
    0,
    MAX_BRIGHTNESS);
}

void setHues(uint8_t tmin_hue, uint8_t tmax_hue){
  min_hue = tmin_hue;
  max_hue = tmax_hue;
}

void nextPattern() {
  // add one to the current pattern number, and wrap around at the end
  if(manual_control_enabled){
    manual_pattern_idx = (manual_pattern_idx + 1) % NUM_PATTERNS;
  }else{
    manual_control_enabled = true;
  }
}

void clearLEDSegment(Strip_Buffer * buf, int len){
  for(int i = 0; i < len; i++)
    buf->leds[i] = CRGB(0,0,0);
}

void blank(Strip_Buffer * buf, int len, Pattern_Data* params){
  clearLEDSegment(buf, len);
}

void setColorHSV(CRGB* leds, byte h, byte s, byte v, int len) {
  // create a new HSV color
  CHSV color = CHSV(h, s, v);
  // use FastLED to set the color of all LEDs in the strip to the same color
  fill_solid(leds, len, color);
}


/// @brief Based on a sufficient volume, a pixel will float to some position on the light strip 
///        and fall down (vol_show adds another threshold)
/// @param buf Pointer to the Strip_Buffer structure, holds LED buffer and history variables.
/// @param len The length of LEDs to process
/// @param params Pointer to Pattern_Data structure containing configuration options.
void pix_freq(Strip_Buffer * buf, int len, Pattern_Data* params) {
  double volume = audioAnalysis.getVolume();
  double peak   = audioAnalysis.getPeak();
    //switch(params->config){
      //case 0:
      //default:
    //getFhue();
    fadeToBlackBy(buf->leds, len, 50);
    if (volume > 200) {
      buf->pix_pos = map(peak, MIN_FREQUENCY, MAX_FREQUENCY, 0, len-1);
      buf->tempHue = fHue;
    }
    else {
      buf->pix_pos--;
      buf->tempHue--;
      buf->vol_pos--;
    }
    if (VOL_SHOW) {
      if (volume > 100) {
        buf->vol_pos = map(volume, MIN_VOLUME, MAX_VOLUME, 0, len-1);
        buf->tempHue = fHue;
      } else {
        buf->vol_pos--;
      }

      buf->leds[buf->vol_pos] = buf->vol_pos < len ? CRGB(255, 255, 255):CRGB(0, 0, 0);
    }
    buf->leds[buf->pix_pos] = buf->pix_pos < len ? CHSV(buf->tempHue, 255, 255):CRGB(0, 0, 0);
}

/// @brief Confetti effect using frequency and brightness.
///        Colored speckles that blink and fade smoothly are scattered across the strip.
/// @param buf Pointer to the Strip_Buffer structure, holds LED buffer and history variables.
/// @param len The length of LEDs to process
/// @param params Pointer to Pattern_Data structure containing configuration options.
void confetti(Strip_Buffer * buf, int len, Pattern_Data* params){
  // colored speckles based on frequency that blink in and fade smoothly
  fadeToBlackBy(buf->leds, len, 20);
  int pos = random16(len);
  switch(params->config){
      case 0:
      default:
      buf->leds[pos] += CHSV( fHue + random8(10), 255, vbrightness);
      buf->leds[pos] += CHSV( fHue + random8(10), 255, vbrightness);
  }
}

/// @brief  Outputs a steady moving stream of lights where each pixel correlates to a previous fHue value.
///          Visually tracks pitch over time, with brightness determined by volume.
///           blur configuration adds blur
/// @param buf Pointer to the Strip_Buffer structure, holds LED buffer and history variables.
/// @param len The length of LEDs to process
/// @param params Pointer to Pattern_Data structure containing configuration options.
void hue_trail(Strip_Buffer* buf, int len, Pattern_Data* params) {
    switch (params->config) {
        case 0: // freq_hue_trail (also default case)
        default: // Default case set to execute the freq_hue_trail pattern
            buf->leds[0] = CHSV(fHue, 255, vbrightness);
            buf->leds[1] = CHSV(fHue, 255, vbrightness);
            for (int i = len - 1; i > 1; i -= 2) {
                buf->leds[i] = buf->leds[i - 2];
                buf->leds[i - 1] = buf->leds[i - 2];
            }
            break;

        case 1: // blur
            {
            buf->leds[0] = CHSV(fHue, 255, vbrightness);
            buf->leds[1] = CHSV(fHue, 255, vbrightness);
            for (int i = len - 1; i > 1; i -= 2) {
                buf->leds[i] = buf->leds[i - 2];
                buf->leds[i - 1] = buf->leds[i - 2];
            }
            blur1d(buf->leds, len, 20);
            break;
            }
        case 2: //sin_hue
        {
        //Create sin beat
        uint16_t sinBeat0  = beatsin16(12, 0, len-1, 0, 0);
        
        //Given the sinBeat and fHue, color the LEDS and fade
        buf->leds[sinBeat0]  = CHSV(fHue, 255, MAX_BRIGHTNESS);
        fadeToBlackBy(buf->leds, len, 5);
        break;
      }

  }
}

/// @brief  Fills the light strip with a nice ambient mess of colors that shift slowly over time. 
///         This function is similar to grovvy noise except the scale and hue_shift values are quiote different.
///         Hue Octave Config remaps the volume to the range of hues present on strip.
///         Hue Shift Config remaps volume as octaves and hhue_shift parameters in fill_noise16()
///         Noise Compression config rremaps volume as hue_x parameter
/// @param buf Pointer to the Strip_Buffer structure, holds LED buffer and history variables.
/// @param len The length of LEDs to process
/// @param params Pointer to Pattern_Data structure containing configuration options.
void saturated(Strip_Buffer* buf, int len, Pattern_Data* params){
  double volume = audioAnalysis.getVolume();
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
        fill_noise16 (buf->leds, len, octaves, x, scale, hue_octaves, hue_x, hue_scale, ntime, hue_shift);
      break;
    case 1: { // Hue octaves 
        hue_octaves = remap(volume, MIN_VOLUME, MAX_VOLUME, 1, 10);
        fill_noise16 (buf->leds, len, octaves, x, scale, hue_octaves, hue_x, hue_scale, ntime, hue_shift);
        }
        break;
        
    case 2: {// Hue shift 
        octaves = remap(volume, MIN_VOLUME, MAX_VOLUME, 50, 100);
        hue_shift = remap(volume, MIN_VOLUME, MAX_VOLUME, 50, 100);
        scale = 230;
        hue_x = 150;
        fill_noise16 (buf->leds, len, octaves, x, scale, hue_octaves, hue_x, hue_scale, ntime, hue_shift);            }
        break;
    case 3:{ // Compression
        hue_x = remap(volume, MIN_VOLUME, MAX_VOLUME, 1, 8);
        ntime = millis() / 4;
        fill_noise16 (buf->leds, len, octaves, x, scale, hue_octaves, hue_x, hue_scale, ntime, hue_shift);
        }
        break;
  //Add blur
  blur1d(buf->leds, len, 80);
  }
}

/// @brief   A cool fluctuating pattern that changes color in waves of greens, yellows, purples and blue. 
///       This function is similar to saturated_noise but the values of scale and hue_shift are 100 and 5 respectively. 
///       This is a moving pattern but it does not change based on and volume or frequency changes. Uses fill_noise16() and blur.
///       Hue Shift Change configuration remaps volume variable as hue_shift.
/// @param buf Pointer to the Strip_Buffer structure, holds LED buffer and history variables.
/// @param len The length of LEDs to process
/// @param params Pointer to Pattern_Data structure containing configuration options.
void groovy(Strip_Buffer* buf, int len, Pattern_Data* params) {
  double volume = audioAnalysis.getVolume();
  uint8_t octaves = 1;
  uint16_t x = 0;
  int scale = 100;
  uint8_t hue_octaves = 1;
  uint16_t hue_x = 1;
  int hue_scale = 50;
  uint16_t ntime = millis() / 3;
  uint8_t hue_shift =  5;
    
    
    switch (params->config) {
        case 0: // groovy_noise (also default case)
        default:
            {
                fill_noise16(buf->leds, len, octaves, x, scale, hue_octaves, hue_x, hue_scale, ntime, hue_shift);
            }
            break;

        case 1: // Hue Shift Change
            {
                int shiftFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 5, 220);
                int xFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 1, 2);
                x = 0;
                hue_octaves = 1;
                hue_x = xFromVolume;
                hue_scale = 20;
                hue_shift =  shiftFromVolume;

                fill_noise16(buf->leds, len, octaves, x, scale, hue_octaves, hue_x, hue_scale, ntime, hue_shift);
            }
            break;
    }
    blur1d(buf->leds, len, 80);
}

/// @brief   Generates three clusters of lights, one in the middle, and two symmetric ones that travel out from the center and return. 
///         The distance the two outer LEDS from the center is determined by the detected volume.
///         Formant configuration takes the clusters to be red,green,blue.
///         Moving configuration, the three clusters move up and down according to sine wave motion.
/// @param buf Pointer to the Strip_Buffer structure, holds LED buffer and history variables.
/// @param len The length of LEDs to process
/// @param params Pointer to Pattern_Data structure containing configuration options.
void talking(Strip_Buffer *buf, int len, Pattern_Data *params) {
  double volume = audioAnalysis.getVolume();
  // Common variables
  int offsetFromVolume;
  int midpoint = len / 2;

  switch (params->config) {
    case 1: { // Formants
      double f0Hue = remap(formants[0], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255);
      double f1Hue = remap(formants[1], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255);
      double f0 = remap(formants[0], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255);
      double f1 = remap(formants[1], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255);
      double f2 = remap(formants[2], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255);

      offsetFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 1, 30);
      buf->leds[len / 2] = CRGB(f0, f1, f2);
      buf->leds[len / 2 - offsetFromVolume] = CHSV(f0Hue, 255, MAX_BRIGHTNESS);
      buf->leds[len / 2 + offsetFromVolume] = CHSV(f0Hue, 255, MAX_BRIGHTNESS);
      break;
    } 

    case 2: { // Moving
      offsetFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 0, 12500);

      uint16_t sinBeat0 = beatsin16(5, 2, len - 3, 0, 250);
      uint16_t sinBeat1 = beatsin16(5, 2, len - 3, 0, 0 - offsetFromVolume);
      uint16_t sinBeat2 = beatsin16(5, 2, len - 3, 0, 750 + offsetFromVolume);

      buf->leds[sinBeat0] = CHSV(fHue + 100, 255, MAX_BRIGHTNESS);
      buf->leds[sinBeat1] = CHSV(fHue, 255, MAX_BRIGHTNESS);
      buf->leds[sinBeat2] = CHSV(fHue, 255, MAX_BRIGHTNESS);
      break;
    } 

    default: // Talking Hue
    case 0:
      offsetFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 1, len/2);
      buf->leds[midpoint] = CHSV(fHue / 2, 255, MAX_BRIGHTNESS);
      buf->leds[midpoint - offsetFromVolume] = CHSV(fHue, 255, MAX_BRIGHTNESS);
      buf->leds[midpoint + offsetFromVolume] = CHSV(fHue, 255, MAX_BRIGHTNESS);
      break;
  }

  // Common effects for all modes
  blur1d(buf->leds, len, 80);
  // Adjust fade value based on the pattern
  int fadeValue = (params->config == 0 || params->config == 2) ? 150 : (params->config == 1) ? 200 : 100;
  fadeToBlackBy(buf->leds, len, fadeValue);
}

/// @brief  Creates two light clusters that move according to sine wave motion, but their speed is affected by the volume. 
///         One pulls its color from fHue, and the other pulls its color from the formant values.
///         Talk configuration combines glitch with talking_moving().
///         Sections configuration creates 4 seperate sine wave clusters.
/// @param buf Pointer to the Strip_Buffer structure, holds LED buffer and history variables.
/// @param len The length of LEDs to process
/// @param params Pointer to Pattern_Data structure containing configuration options.
void glitch(Strip_Buffer * buf, int len, Pattern_Data * params ) {
    double volume = audioAnalysis.getVolume();
    int offsetFromVolume, speedFromVolume;
    uint16_t sinBeat[4]; 
    double f0Hue;
    
    speedFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 5, params->config == 0 ? 25 : 20); 
    switch (params->config) {
        case 0:
            sinBeat[0] = beatsin16(speedFromVolume, 0, len-1, 0, 0);
            sinBeat[1] = beatsin16(speedFromVolume, 0, len-1, 0, 32767);

            f0Hue = remap(formants[0], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255);

            buf->leds[sinBeat[0]]  = CHSV(fHue, 255, MAX_BRIGHTNESS);
            buf->leds[sinBeat[1]]  = CHSV(f0Hue, 255, MAX_BRIGHTNESS); //can use fHue instead of formants

            blur1d(buf->leds, len, 80);
            fadeToBlackBy(buf->leds, len, 40);

            break;
        case 1: // glitch_talk
          {
            offsetFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 0, 20000);

            //Create 3 sin beats with the speed and offset(first and last parameters) changing based off variables above
            uint16_t sinBeat0  = beatsin16(speedFromVolume, 3, len-4, 0, 250);
            uint16_t sinBeat1  = beatsin16(speedFromVolume, 3, len-4, 0, 0 - offsetFromVolume);
            uint16_t sinBeat2  = beatsin16(speedFromVolume, 3, len-4, 0, 750 + offsetFromVolume);

            //Given the sinBeats and fHue, color the LEDS  
            buf->leds[sinBeat0]  = CHSV(fHue*2, 255, MAX_BRIGHTNESS);
            buf->leds[sinBeat1]  = CHSV(fHue, 255, MAX_BRIGHTNESS);
            buf->leds[sinBeat2]  = CHSV(fHue, 255, MAX_BRIGHTNESS);

            blur1d(buf->leds, len, 80);
            fadeToBlackBy(buf->leds, len, 100); 

            break;
          }
        case 2: // glitch_sections
          {
            offsetFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 0, 10000);

            //Create 4 sin beats with the offset(last parameter) changing based off offsetFromVolume
            uint16_t sinBeat0  = beatsin16(6, 0, len-1, 0, 0     - offsetFromVolume);
            uint16_t sinBeat1  = beatsin16(6, 0, len-1, 0, 16384 - offsetFromVolume);
            uint16_t sinBeat2  = beatsin16(6, 0, len-1, 0, 32767 - offsetFromVolume);
            uint16_t sinBeat3  = beatsin16(6, 0, len-1, 0, 49151 - offsetFromVolume);

            //Given the sinBeats and fHue, color the LEDS
            buf->leds[sinBeat0]  = CHSV(fHue, 255, MAX_BRIGHTNESS);
            buf->leds[sinBeat1]  = CHSV(fHue, 255, MAX_BRIGHTNESS);
            buf->leds[sinBeat2]  = CHSV(fHue, 255, MAX_BRIGHTNESS);
            buf->leds[sinBeat3]  = CHSV(fHue, 255, MAX_BRIGHTNESS);
          
            //Add blur and fade 
            blur1d(buf->leds, len, 80);
            fadeToBlackBy(buf->leds, len, 60);
            break;
          }
    }
}

/// @brief  Basic band config : Uses the band_split_bounce() function to generate a five band split, and maps that split to the light strip. The strip is broken into five chunks of different colors, 
///         where the volume of each band determines how much of each section of the LED strip is lit.
///         Advanced bands config : he strip is broken into five chunks of different colors, where the volume of each band determines how much of each section is lit, and that portion will diminish over time if a certain volume threshold is not met
///         Fomant bands config: emonstrates the formant feature of the audio analysis code. Each of the three formant values correspond to a third of the entire LED strip, where the individual formant values determine the hue of each third.
/// @param buf Pointer to the Strip_Buffer structure, holds LED buffer and history variables.
/// @param len The length of LEDs to process
/// @param params Pointer to Pattern_Data structure containing configuration options.
void bands(Strip_Buffer* buf, int len, Pattern_Data* params) {
    float *fbs = audioAnalysis.getFiveBandSplit(len);
        
    double avg1 = 0;
    double avg2 = 0;
    double avg3 = 0;
    double avg4 = 0;
    double avg5 = 0;

    double vol1 = fbs[0];
    double vol2 = fbs[1];
    double vol3 = fbs[2];
    double vol4 = fbs[3];
    double vol5 = fbs[4];


      switch (params->config) {
        case 0 : 
        {
            fadeToBlackBy(buf->leds, len, 85);
            // Fill each chunk of the light strip with the volume of each band
            for (int i = 0; i < vol1; i++) {
              buf->leds[i] = CRGB(255,0,0);
            }
            for (int i = len/5; i < len/5+vol2; i++) {
              buf->leds[i] = CRGB(255,255,0);
            }
            for (int i = 2*len/5; i < 2*len/5+vol3; i++) {
              buf->leds[i] = CRGB(0,255,0);
            }
            for (int i = 3*len/5; i < 3*len/5+vol4; i++) {
              buf->leds[i] = CRGB(0,255,255);
            }
            for (int i = 4*len/5; i < 4*len/5+vol5; i++) {
              buf->leds[i] = CRGB(0,0,255);
            }
            break;
        }
          case 1:{
          // Calculate the volume of each band
          for (int i = 0; i < advanced_size; i++) {
            avg1 += buf->max1[i];    
          }
          avg1 /= advanced_size;
          for (int i = 0; i < advanced_size; i++) {
            avg2 += buf->max2[i];
          }
          avg2 /= advanced_size;
          for (int i = 0; i < advanced_size; i++) {
            avg3 += buf->max3[i];
          }
          avg3 /= advanced_size;
          for (int i = 0; i < advanced_size; i++) {
            avg4 += buf->max4[i];
          }
          avg4 /= advanced_size;
          for (int i = 0; i < advanced_size; i++) {
            avg5 += buf->max5[i];
          }
          avg5 /= advanced_size;

          vol1 = fbs[0];
          vol2 = fbs[1];
          vol3 = fbs[2];
          vol4 = fbs[3];
          vol5 = fbs[4]; 

          // Get the Five Sample Split ^

          if(config.debug_mode == 1){
            Serial.print("ADVANCED::\tAVG1:\t");
            Serial.print(avg1);
            Serial.print("\tAVG2:\t");
            Serial.print(avg2);
            Serial.print("\tAVG3:\t");
            Serial.print(avg3);
            Serial.print("\tAVG4:\t");
            Serial.print(avg4);
            Serial.print("\tAVG5:\t");
            Serial.print(avg5);
          }

          // If there exists a new volume that is bigger than the falling pixel, reassign it to the top, otherwise make it fall for each band
          if (vol1 <= avg1) {
            buf->max1[buf->maxIter] = 0;
          }
          else {
              for (int i = 0; i < 5; i++) {
                buf->max1[i] = vol1;
              }
          }
            
          if (vol2 <= avg2) {
            buf->max2[buf->maxIter] = 0;
          }
          else {
              for (int i = 0; i < 5; i++) {
                buf->max2[i] = vol2;
              }
          }

          if (vol3 <= avg3) {
            buf->max3[buf->maxIter] = 0;
          }
          else {
              for (int i = 0; i < 5; i++) {
                buf->max3[i] = vol3;
              }
          }

          if (vol4 <= avg4) {
            buf->max4[buf->maxIter] = 0;
          }
          else {
              for (int i = 0; i < 5; i++) {
                buf->max4[i] = vol4;
              }
          }

          if (vol5 <= avg5) {
            buf->max5[buf->maxIter] = 0;
          }
          else {
              for (int i = 0; i < 5; i++) {
                buf->max5[i] = vol5;
              }
          }

          // Get this smoothed array to loop to beginning again once it is at teh end of the falling pixel smoothing
          if (buf->maxIter == advanced_size-1) {
            buf->maxIter = 0;
          } else {
            buf->maxIter++;
          }

          // Fill the respective chunks of the light strip with the color based on above^
          for (int i = 0; i < vol1-1; i++) {
            buf->leds[i] = CRGB(255,0,0);
          }
          for (int i = len/5; i < len/5+vol2-1; i++) {
            buf->leds[i] = CRGB(255,255,0);
          }
          for (int i = 2*len/5; i < 2*len/5+vol3-1; i++) {
            buf->leds[i] = CRGB(0,255,0);
          }
          for (int i = 3*len/5; i < 3*len/5+vol4-1; i++) {
            buf->leds[i] = CRGB(0,255,255);
          }
          for (int i = 4*len/5; i < 4*len/5+vol5-1; i++) {
            buf->leds[i] = CRGB(0,0,255);
          }
          
          // Assign the values for the pixel to goto
          buf->leds[(int) avg1+ (int) vol1] = CRGB(255,255,255);
          buf->leds[(int) len/5 + (int) avg2+ (int) vol2] = CRGB(255,255,255);
          buf->leds[(int) 2*len/5+(int) avg3+ (int) vol3] = CRGB(255,255,255);
          buf->leds[(int) 3*len/5+(int) avg4+ (int) vol4] = CRGB(255,255,255);
          buf->leds[(int) 4*len/5+(int) avg5+ (int) vol5] = CRGB(255,255,255);
          fadeToBlackBy(buf->leds, len, 90);

          break;
        }
        // case 2 :
        // {
        //     // Grab the formants
        //     double *temp_formants = density_formant();
        //     double f0Hue = remap(temp_formants[0], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255);
        //     double f1Hue = remap(temp_formants[1], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255);
        //     double f2Hue = remap(temp_formants[2], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255);

        //     if(config.debug_mode == 1){
        //       Serial.print("\t f0Hue: ");
        //       Serial.print(temp_formants[0]);
        //       Serial.print("\t f1Hue: ");
        //       Serial.print(temp_formants[1]);
        //       Serial.print("\t f2Hue: ");
        //       Serial.print(temp_formants[2]);
        //     }

        //     // Fill 1/3 with each formant
        //     for (int i = 0; i < len; i++) {
        //       if (i < len/3) {
        //         buf->leds[i] = CHSV(f0Hue, 255, 255);
        //       }
        //       else if (len/3 <= i && i < 2*len/3) {
        //         buf->leds[i] = CHSV(f1Hue, 255, 255);
        //       } 
        //       else { 
        //         buf->leds[i] = CHSV(f2Hue, 255, 255);
        //       }
        //     }

        //     // Smooth out the result
        //     for (int i = 0; i < 5; i++) {
        //       blur1d(buf->leds, len, 50);
        //     }
        //     delete[] temp_formants;
        //     break;
        //   }
      }
}

/// @brief Short and sweet function. Each pixel corresponds to a value from vReal, 
///         where the volume at each pitch determines the brightness of each pixel. Hue is locked in to a rainbow.
/// @param buf Pointer to the Strip_Buffer structure, holds LED buffer and history variables.
/// @param len The length of LEDs to process
/// @param params Pointer to Pattern_Data structure containing configuration options.
void eq(Strip_Buffer * buf, int len, Pattern_Data* params) {
  float* vReal = audioAnalysis.getVReal();

  for (int i = 0; i < len; i++) {
    int brit = map(vReal[i], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255); // The brightness is based on HOW MUCH of the frequency exists
    int hue = map(i, 0, len, 0, 255); // The fue is based on position on the light strip, ergo, what frequency it is at
    if (vReal[i] > 200) { // An extra gate because the frequency array is really messy without it
      buf->leds[i] = CHSV(hue, 255, brit);
    }
  }
}

/// @brief A random spot is chosen along the length and does a ripple based on frequency
/// @param buf Pointer to the Strip_Buffer structure, holds LED buffer and history variables.
/// @param len The length of LEDs to process
/// @param params Pointer to Pattern_Data structure containing configuration options.
void random_raindrop(Strip_Buffer * buf, int len, Pattern_Data* params){
    int startIdx = random(len);

    buf->leds[startIdx] = CHSV(fHue, 255, vbrightness);
    
    for(int i = len-1; i > 0; i--) {
      if (i != startIdx) {
        buf->leds[i] = buf->leds[i-1];
      }
    }

    buf->leds[0] = CRGB::Black;
}

/// @brief Strip is split into two sides, red and blue showing push and pull motion 
///         based on either frequency or volume
/// @param buf Pointer to the Strip_Buffer structure, holds LED buffer and history variables.
/// @param len The length of LEDs to process
/// @param params Pointer to Pattern_Data structure containing configuration options.
void tug_of_war(Strip_Buffer * buf, int len, Pattern_Data* params) {
    int splitPosition;
    //use this function with smoothing for better results
    // red is on the left, blue is on the right
    // switch(params->config) {
    //   case 0: // frequency
    //     {
          
    //     double *formants = density_formant();
    //     double f0 = formants[0];
    //     delete[] formants;
    //     splitPosition = remap(f0, MIN_FREQUENCY, MAX_FREQUENCY, 0, len);

    //     // red is on the left, blue is on the right
    //     for (int i = 0; i < len; i++) {
    //         if (i < splitPosition) {
    //             buf->leds[i] = CHSV(MIN_HUE, 255, 255);
    //         } else {
    //             buf->leds[i] = CHSV(MAX_HUE, 255, 255);
    //         }
    //     }
    
    //     }
    //   case 1: // volume
    //     {
    //     splitPosition = remap(audioAnalysis.getVolume(), MIN_VOLUME, MAX_VOLUME, 0, len);
    //     for (int i = 0; i < len; i++) {
    //         if (i < splitPosition) {
    //             buf->leds[i] = CHSV(MIN_HUE, 255, 255);
    //         } else {
    //             buf->leds[i] = CHSV(MAX_HUE, 255, 255);
    //         }
    //     }
    //     }
    // }
}



/// @brief Fire2012 pattern utilizing heating and cooling
/// @param buf Pointer to the Strip_Buffer structure, holds LED buffer and history variables.
/// @param len The length of LEDs to process
/// @param params Pointer to Pattern_Data structure containing configuration options.
void Fire2012(Strip_Buffer * buf, int len, Pattern_Data* params){
  float volume = audioAnalysis.getVolume();
  float* vReal = audioAnalysis.getVReal();

  int sparkVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 10,200);
  //int coolingVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 60, 40);
  //Serial.println(sparkVolume);
  
  
  // Array of temperature readings at each simulation cell
  static byte heat[MAX_LEDS];

// Step 1.  Cool down every cell a little
  for( int i = 0; i < config.length; i++) {
    heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / config.length) + 2));
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for( int k= config.length - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
  if( random8() < sparkVolume ) {
    int y = random8(7);
    heat[y] = qadd8( heat[y], random8(160,255) );
  }

  //Step 3.5. Calcualate Brightness from low frencies
  int l = (sizeof(vReal)/sizeof(vReal[0])) / 7;
  double smol_arr[l];
  memcpy(smol_arr, vReal, l * sizeof(double));
  

  // Step 4.  Map from heat cells to LED colors
  for( int j = 0; j < config.length; j++) {
    // Scale the heat value from 0-255 down to 0-240
    // for best results with color palettes.
  
    byte colorindex = scale8( heat[j], 240);
    //byte colorindex = scale8( heat[j], smol_arr);
    CRGB color = ColorFromPalette( gPal, colorindex);
    int pixelnumber;
    if( gReverseDirection ) {
      pixelnumber = (config.length-1) - j;
    } else {
      pixelnumber = j;
    }
    buf->leds[pixelnumber] = color;
  }
}

void vowels_raindrop(Strip_Buffer * buf, int len, Pattern_Data* params){
    int startIdx = random(len);
    // VowelSounds result = vowel_detection();
    // switch (result) {
    //   case aVowel:
    //     buf->leds[startIdx] = CRGB::Blue;
    //     break;
    //   case eVowel:
    //     buf->leds[startIdx] = CRGB::Green;
    //     break;
    //   case iVowel:
    //     buf->leds[startIdx] = CRGB::Red;
    //     break;
    //   case oVowel:
    //     buf->leds[startIdx] = CRGB::Orange;
    //     break;
    //   case uVowel:
    //     buf->leds[startIdx] = CRGB::Yellow;
    //     break;
    //   default: // no vowel is detected
    //     buf->leds[startIdx] = CRGB::Black;
    //     break;
    // }

    buf->leds[startIdx] = CHSV(fHue, 255, vbrightness);
    
    for(int i = len-1; i > 0; i--) {
      if (i != startIdx) {
        buf->leds[i] = buf->leds[i-1];
      }
    }

    buf->leds[0] = CRGB::Black;
  
}

#define VOLUME 0
#define FREQUENCY 1
/// @brief Displays a pattern that occupies "lower" pixels at lower values,
/// and "higher" pixels at higher values.
/// @param buf Pointer to the Strip_Buffer structure, holds LED buffer and history variables.
/// @param len The length of LEDs to process
/// @param params Pointer to Pattern_Data structure containing configuration options.
void bar_fill(Strip_Buffer * buf, int len, Pattern_Data* params){
  float volume = audioAnalysis.getVolume();
  uint8_t max_height = 0;

  switch(params->config) {

    case VOLUME: default: {
      max_height = remap(volume, MIN_VOLUME * 4, MAX_VOLUME/2, 0, len-1);
      break;
    }

    case FREQUENCY: {
      max_height = map(audioAnalysis.getPeak(), MIN_FREQUENCY * 4, MAX_FREQUENCY/2, 0, len-1);
      break;
    }
  }

  uint8_t hue_step = (max_hue - min_hue) / (len - 1);

  // Apply the color to the strip.
  for(uint8_t i = 0; i < max_height; i++){
    buf->leds[i] = CHSV(
      (min_hue + hue_step * (i - 1)) % 255,
      255,
      255
    );
  }

  // Black out the rest of the strip.
  for(uint8_t i = max_height; i < len; i++){
    buf->leds[i] = CHSV(0, 0, 0);
  }
  
}

int getColor(int noteNumber) {
  int baseNote = noteNumber % 12; 
  
  switch (baseNote) {
    case 0:  return 0;      // C - Red
    case 1:  return 21;     // C# - Reddish-Orange
    case 2:  return 43;     // D - Yellow
    case 3:  return 64;     // D# - Yellow-Green
    case 4:  return 85;     // E - Green
    case 5:  return 106;    // F - Cyan-Green
    case 6:  return 127;    // F# - Cyan
    case 7:  return 148;    // G - Blue
    case 8:  return 170;    // G# - Indigo/Purple
    case 9:  return 191;    // A - Magenta
    case 10: return 212;    // A# - Pinkish
    case 11: return 234;    // B - Violet
    default: return 0;        // Debugging case (shouldn't occur)
  }
}

void blendIn(Strip_Buffer * buf, int len, Pattern_Data* params){
  CHSV backColor;
   CHSV corrColor;
   float* vReal = audioAnalysis.getVReal();
   switch(params->config){
           case 0:
           default:
               backColor = CHSV(194, 255, 255);
               corrColor = CHSV(83, 38, 255);
   }
   for(int i = 0; i < len; i++){
      if(vReal[i] > 200){
        int blending = map(vReal[i], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255);
       buf->leds[i] = blend(backColor, corrColor, blending, SHORTEST_HUES);
      }
   }
}


void bleedThrough(Strip_Buffer * buf, int len, Pattern_Data* params){
  float volume = audioAnalysis.getVolume();
  static int prog = 0;
  //fadeToBlackBy(buf->leds, len, 12);
  int blending = remap(volume, MIN_VOLUME, MAX_VOLUME, 0, 60);
  Serial.println(blending);
  //int blending = remap(log(peak) / log(2), log(MIN_FREQUENCY) / log(2), log(MAX_FREQUENCY) / log(2), 0, 30);
  if(volume > 20){
    buf->vol_pos += blending;
  }
  if (buf->vol_pos > 255){
   buf->vol_pos = 255;
 }
 CHSV backColor = CHSV(getColor(prog), 255, remap(volume, MIN_VOLUME, MAX_VOLUME, 30, 255));
 CHSV corrColor = CHSV(getColor(prog + 1), 255, remap(volume, MIN_VOLUME, MAX_VOLUME, 30, 255));
 for(int i = len - 1; i > 0; i--){
   buf->leds[i] = buf->leds[i - 1];
 }
 buf->leds[0] = blend(backColor, corrColor, buf->vol_pos, SHORTEST_HUES);
 //fill_solid(buf->leds, len, blend(backColor, corrColor, buf->vol_pos, SHORTEST_HUES));
 if (buf->vol_pos == 255){
     prog++;
     prog = prog % 12;
     buf->vol_pos = 0;
 }
}

void showcasePercussion(Strip_Buffer * buf, int len, Pattern_Data* params){
  bool percussionPresent = audioAnalysis.getPercussionPresence();
  static bool perc = false;
  Serial.println(percussionPresent);
  //fadeToBlackBy(buf->leds, len, 150);
  //percussionPresent is a hypothetical external bool that detects if percussion is present
  if (percussionPresent != perc){
    if (percussionPresent){
      buf->leds[0] = CHSV(150, 255, 200);
      buf->leds[len] = CHSV(150, 255, 200);
      //setColorHSV(buf->leds, 150, 255, 255, len);
      Serial.print("AHHH");
    }
    else{
      buf->leds[0] = CHSV(0, 0, 0);
      buf->leds[len] = CHSV(0, 0, 0);
    }
    perc = percussionPresent;
  }
  else{
    buf->leds[0] = CHSV(0, 0, 0);
    buf->leds[len] = CHSV(0, 0, 0);
  }
  for(int i = len-1; i > 0; i--) {
    buf->leds[i] = buf->leds[i-1];
  }
  blur1d(buf->leds, len, 20);
}


void showcaseCentroid(Strip_Buffer * buf, int len, Pattern_Data* params){
  //centroid is a hypothetical external float that showcases the frequency of the center of mass, characterizing brightness
  for(int i = 0; i < len; i++){


    float value = (MAX_FREQUENCY - MIN_FREQUENCY) / audioAnalysis.getCentroid();
    float lenValue = len / (i + 1) * 2;


    if(value >= lenValue){
      buf->leds[i] = CHSV(194, 255, 255);
    }
    else{
      buf->leds[i] = CHSV(83, 38, 255);
    }
  }
}


/*
void showcaseNoisiness(Strip_Buffer * buf, int len, Pattern_Data* params){
//noisiness is a hypothetical external float from 1 to 0, representing the randomness of energy in the frequency.
//maybe use noisiness to set a blurring effect? to represent the erratic nature. Or maybe the speed for like a sin beat scenario
}
*/


void showcaseBread(Strip_Buffer * buf, int len, Pattern_Data* params){
  // //breadSlicer is a hypothetical external array of floats, where each value corresponds to the sum of amps in that sector
  // int slices;
  // switch(param->config){
  //   case 0:
  //   slices = 1;
  //   break;
  //   case 1:
  //   slices = 3;
  //   break;
  //   case 2:
  //   slices = 5;
  //   break;
  // }
  // int bands[slices];
  // for (int i = 0; i < slices; i++){
  //   bands[i] = MAX_FREQUENCY / slices * i;
  // }
  // //setBreadSlicer is a hypothetical helper function that lets us configure the breadSlicer, basically calling its setBands function from the patterns
  // setBreadSlicer(bands, slices);
  // float* amps = slicerOutput();


  // for(int i = 0; i < slices - 1; i++){


  //   amps[i] = amps[i] / (MAX_FREQUENCY / slices); //set to average of that area
  //   int brit = remap(amps[i], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255); //set brightness to average amp
  //   int startingPos = len / (slices - 1) * i; //set position that this slice occupies
  //   int sliceHue = 255 / (slices - 1); //set hue based on each slice


  //   for(int j = 0; j < len / (slices - 1); j++){
  //     buf->leds[startingPos + j] = CHSV(sliceHue, 70, brit);
  //   }
  // }
  return;
}


void showcaseSalientFreqs(Strip_Buffer * buf, int len, Pattern_Data* params){
  //salientFreqs is a hypothetical external array of ints that represent indexes with the greatest change in amplitude. By default it gives 3 points
  int* salFreqs = audioAnalysis.getSalientFreqs();
  static int splatter[MAX_LEDS];


  fadeToBlackBy(buf->leds, len, 50);


  for(int i = 0; i < len; i++){
    splatter[i] = -1;
  }


  for(int i = 0; i < 3; i++){
    int startPos = salFreqs[i];
   
    for(int i = 0; i < 4; i++){
      if (startPos + i <= len){
        splatter[startPos + i] = i;
      }
      if(startPos - i >= 0){
        splatter[startPos - i] = i;
      }
    }
  }


  for(int i = 0; i < len; i++){
    if (splatter[i] == 0){
      buf->leds[i] = CHSV(fHue, 255, vbrightness);
      splatter[i] = -1;
    }
    else if (splatter[i] > 0){
      splatter[i] -= 1;
    }
  }
}

// Mapping MIDI note numbers to CHSV colors across all octaves. Based on common synesthesia associations.
CHSV getColorForNote(int noteNumber) {
  int baseNote = noteNumber % 12; 
  
  switch (baseNote) {
    case 0:  return CHSV(0, 255, 255);      // C - Red
    case 1:  return CHSV(21, 255, 255);     // C# - Reddish-Orange
    case 2:  return CHSV(43, 255, 255);     // D - Yellow
    case 3:  return CHSV(64, 255, 255);     // D# - Yellow-Green
    case 4:  return CHSV(85, 255, 255);     // E - Green
    case 5:  return CHSV(106, 255, 255);    // F - Cyan-Green
    case 6:  return CHSV(127, 255, 255);    // F# - Cyan
    case 7:  return CHSV(148, 255, 255);    // G - Blue
    case 8:  return CHSV(170, 255, 255);    // G# - Indigo/Purple
    case 9:  return CHSV(191, 200, 255);    // A - Magenta
    case 10: return CHSV(212, 255, 255);    // A# - Pinkish
    case 11: return CHSV(234, 100, 255);    // B - Violet
    default: return CHSV(0, 0, 255);        // Debugging case (shouldn't occur)
  }
}

// Maps the current note to a color.
void synesthesiaRolling(Strip_Buffer *buf, int len, Pattern_Data* params) {
  float volume = audioAnalysis.getVolume();
  float peak   = audioAnalysis.getPeak();

  // Convert peak to a midi note
  float currentNote = 12 * log(peak / 440) / log(2.0) + 69;

  // Static variables for smoothing
  static float maxVolume = volume;
  static float smoothedBrightness = 0;
  static float smoothedNote = currentNote;  // MIDI number
  static float lastNote = currentNote;  
  static float fadeFactor = 1.0;            // Controls the fadeing effect

  // Smoothing factors
  const float DECAY_FACTOR = 0.97;         // Controls how slowly max volume decreases
  const float BRIGHTNESS_SMOOTHING = 0.8;  // 0 = slowest brightness update, 1 = instant
  const float NOTE_SMOOTHING = 0.125;      // 0 = slowest note transition, 1 = instant

  // Smooth the note
  smoothedNote = smoothedNote * (1 - NOTE_SMOOTHING) + currentNote * NOTE_SMOOTHING;
  CHSV chsvNote = getColorForNote(round(smoothedNote));  

  // Smooth the brightness (volume is considered relative here)
  maxVolume = max(static_cast<float>(volume), maxVolume * DECAY_FACTOR);
  float targetBrightness = (volume / maxVolume) * 255;
  smoothedBrightness = smoothedBrightness * (1 - BRIGHTNESS_SMOOTHING) + targetBrightness * BRIGHTNESS_SMOOTHING;
  chsvNote.v = smoothedBrightness;
    
  if (round(currentNote) == round(lastNote)) {
    fadeFactor = max(0.35, fadeFactor - 0.05);  
  } else {
    fadeFactor = 1.0;
  }

  chsvNote.v *= fadeFactor;
  lastNote = currentNote;

  for (int i = len - 1; i > 0; i--) {
    buf->leds[i] = buf->leds[i - 1];
  }

  buf->leds[0] = chsvNote;
}

// Maps the current note to a section on the led strip
void noteEQ(Strip_Buffer *buf, int len, Pattern_Data* params) {
  float volume = audioAnalysis.getVolume();
  float peak   = audioAnalysis.getPeak();

  // Static variables for smoothing and tracking previous states
  static float maxVolume = volume;
  static float smoothedBrightness = 0;

  // Number of sections (ajust as notes included change) 
  const int NUM_SECTIONS = 12;  
  const int SECTION_LENGTH = len / NUM_SECTIONS; 

  // Smoothing factors
  const float DECAY_FACTOR = 0.97;           // Controls how slowly max volume decreases
  const float BRIGHTNESS_SMOOTHING = 0.75;   // 0 = slowest brightness update, 1 = instant
  const float DIM_FACTOR = 0.875;

  // Get the color for the current note
  float currentNote = 12 * log(peak / 440) / log(2.0) + 69;
  CHSV chsvNote = getColorForNote(round(currentNote)); 
  int activeSegment = static_cast<int>(round(currentNote)) % NUM_SECTIONS;
  
  // Smooth the brightness (volume is considered relative here)
  maxVolume = max(static_cast<float>(volume), maxVolume * DECAY_FACTOR);
  float targetBrightness = (volume / maxVolume) * 255;
  smoothedBrightness = smoothedBrightness * (1 - BRIGHTNESS_SMOOTHING) + targetBrightness * BRIGHTNESS_SMOOTHING;
  chsvNote.v = smoothedBrightness;
  
  for (int i = 0; i < NUM_SECTIONS; i++) {
    int ledIndex = i * SECTION_LENGTH;
    // Set the leds in the note section to proper color and brightness
    if (i == activeSegment) {
      for (int j = ledIndex; j < ledIndex + SECTION_LENGTH; j++) { 
        buf->leds[j] = chsvNote;  
      }
    }
    // Dim leds not in the relevent section to get the fading effect
    else {
      CRGB currentColor = buf->leds[ledIndex];  
      currentColor.fadeToBlackBy(255 - (255 * DIM_FACTOR));  // CRGB brightness adjustment 

      for (int j = ledIndex; j < ledIndex + SECTION_LENGTH; j++) { 
        buf->leds[j] = currentColor;  
      }
    }
  }
}

CHSV getStringColor(int noteNumber, double volum) {
  int brit = remap(volum, MIN_VOLUME, MAX_VOLUME, 0, 255);
  switch (noteNumber) {
    case 0:  return CHSV(0, 255, 255);      // C - Red
    //case 1:  return CHSV(21, 255, 255);     // C# - Reddish-Orange
    case 1:  return CHSV(43, 255, 255);     // D - Yellow
    //case 3:  return CHSV(64, 255, 255);     // D# - Yellow-Green
    case 2:  return CHSV(85, 255, 255);     // E - Green
    case 3:  return CHSV(127, 255, 255);    // F# - Cyan
    default: return CHSV(0, 0, 255);        // Debugging case (shouldn't occur)
  }
}

double findString(double frequency){
  const int stringStartArray[] = {82, 110, 147, 196, 247, 330};
  if(frequency > 659){
    return -1;
  }
  for(int i = 0; i < 6; i++){
    if(frequency > stringStartArray[i]){
      return i;
    }
  }
  return -1;
}

void stringTheory(Strip_Buffer *buf, int len, Pattern_Data* params) {
  float peak = audioAnalysis.getPeak();
  float volume = audioAnalysis.getVolume();
  //Based on the instrument, have a moving light based on peak frequency corresponding to the string of a guitar
  //Each string of a guitar has a min-max range, distinguish between these strings through color
  //Use centroid compared to logarithmically adjusted frequency values to determine which string it's closest to
  //Use peak adjusted to the min/max to show where on the strip the light should be placed
  fadeToBlackBy(buf->leds, len, 10);
  const int stringStartArray[] = {196, 294, 440, 659};
  const int stringEndArray[] = {293, 439, 658, 1008};
  int numStrings = 4;
  //double newPeak = maxRange(stringStartArray[0], stringEndArray[3]);
  Serial.println(peak);
  for(int i = 0; i < numStrings; i++){
    if(peak >= stringStartArray[i] && peak <= stringEndArray[i]){
      if(volume > 150){
        int value = remap(log(peak) / log(2), log(stringStartArray[i]) / log(2), log(stringEndArray[i]) / log(2), len * 2/3, len);
        CHSV noteColor = getStringColor(i, volume);
        buf->leds[value] = noteColor;
        break;
      }
    }
  }
  //For hypothetical noisiness variable, use it as a replacement to showcase vibrato by changing its presentation
  blur1d(buf->leds, len, 20);
}

void splashTheory(Strip_Buffer *buf, int len, Pattern_Data* params){
  //find the peak, determine a size of area around that peak as the 'splashzone' and create splash effects around that zone 
  float peak = audioAnalysis.getPeak();
  float volume = audioAnalysis.getVolume();
  CHSV backColor = CHSV(127, 255, 255);
  CHSV corrColor = CHSV(40, 255, 255);
  int splashVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 0, len);
  //int goes from 1-4 and is a measure of brightness. dims slowly
  CRGB nextWave[len] = {0};
  for(int i = 1; i < len - 1; i++){
    float beforebright = rgb2hsv_approximate(buf->leds[i-1]).v;
    float afterbright = rgb2hsv_approximate(buf->leds[i+1]).v;
    if(beforebright > 0 || afterbright > 0){
      int locate;
      if(beforebright > afterbright){
        locate = i - 1;
      }
      else{
        locate = i + 1;
      }
      nextWave[i] = buf->leds[locate];
    }
    else{
      nextWave[i] = CHSV(0, 0, 0);
    }
  }
  for(int i = 0; i < len; i++){
    buf->leds[i] = nextWave[i];
  }
  fadeToBlackBy(buf->leds, len, 70);
  int variance = random16(-10, 10);
  splashVolume += variance;
  if(splashVolume > len){
    splashVolume = len;
  }
  if(splashVolume < 0){
    splashVolume = 0;
  }
  int blending = remap(peak, MIN_FREQUENCY, MAX_FREQUENCY, 0, 255);
  buf->leds[splashVolume] = blend(backColor, corrColor, blending, LONGEST_HUES);
  if(splashVolume != len && splashVolume != 0){
    buf->leds[splashVolume + 1] = blend(backColor, corrColor, blending, LONGEST_HUES);
    buf->leds[splashVolume - 1] = blend(backColor, corrColor, blending, LONGEST_HUES);
  }
  //blur1d(buf->leds, len, 20);
}

void deltaHeat(Strip_Buffer *buf, int len, Pattern_Data* params) {
  float* delta = audioAnalysis.getDeltas();
  static int heat[MAX_LEDS];
  for(int i = 0; i < len; i++){
    if (heat[i] < 0){
      heat[i] += 20;
    }
    if (heat[i] > 0){
      heat[i] -= 20;
    }
  }
  int highDelta = delta[0];
  int lowDelta = delta[0];
  for(int i = 0; i < len; i++){
    if(delta[i] > highDelta){
      highDelta = delta[i];
    }
    if(delta[i] < lowDelta){
      lowDelta = delta[i];
    }
  }
  for(int i = 0; i < len; i++){
    //heat[i] -= 30;
    heat[i] += remap(delta[i], lowDelta, highDelta, 0, 30);
  }
  for(int i = 0; i < len; i++){
    int hue;
    if (heat[i] > 0){
      hue = 0;
    }
    else{
      hue = 148;
    }
    buf->leds[i] = CHSV(hue, 255, heat[i]);
  }
  blur1d(buf->leds, len, 20);
}

void gradient(Strip_Buffer *buf, int len, Pattern_Data* params){
  float* vReal = audioAnalysis.getVReal();
  /*
  for (int i = 0; i < len; i++) {
    //int brit = map(vReal[i], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255); // The brightness is based on HOW MUCH of the frequency exists
    int hue = map(vReal[i], MIN_FREQUENCY, 2000, 0, 43); // The fue is based on position on the light strip, ergo, what frequency it is at
    if (vReal[i] > 200) { // An extra gate because the frequency array is really messy without it
      buf->leds[i] = CHSV(hue, 255, 200);
    }
  }
  blur1d(buf->leds, len, 20);
  */
   for (int i = 0; i < len; i++) {
    int brit = map(log(vReal[i]) / log(2), log(MIN_FREQUENCY) / log(2), log(MAX_FREQUENCY) / log(2), 0, 255); // The brightness is based on HOW MUCH of the frequency exists
    int hue = map(i, 0, len, 0, 255); // The fue is based on position on the light strip, ergo, what frequency it is at
    if (vReal[i] > 200) { // An extra gate because the frequency array is really messy without it
      buf->leds[i] = CHSV(hue, 255, brit);
    }
  }
} 

