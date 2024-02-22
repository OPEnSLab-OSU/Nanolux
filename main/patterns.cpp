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

void nextPattern() {
  // add one to the current pattern number, and wrap around at the end
  //current_pattern.pattern_1 = (current_pattern.pattern_1 + 1) % NUM_PATTERNS;
}

void clearLEDSegment(Pattern_History * hist, int len){
  for(int i = 0; i < len; i++)
    hist->leds[i] = CRGB(0,0,0);
}

void setColorHSV(CRGB* leds, byte h, byte s, byte v, int len) {
  // create a new HSV color
  CHSV color = CHSV(h, s, v);
  // use FastLED to set the color of all LEDs in the strip to the same color
  fill_solid(leds, len, color);
}




// uses freq and brightness
void confetti(Pattern_History * hist, int len, Pattern_Data* patternData){
  // colored speckles based on frequency that blink in and fade smoothly
  fadeToBlackBy(hist->leds, len, 20);
  int pos = random16(len);
  switch(patternData->config){
      case 0:
      default:
      hist->leds[pos] += CHSV( fHue + random8(10), 255, vbrightness);
      hist->leds[pos] += CHSV( fHue + random8(10), 255, vbrightness);
}
}

// Based on a sufficient volume, a pixel will float to some position on the light strip and fall down (vol_show adds another threshold)
void pix_freq(Pattern_History * hist, int len, Pattern_Data* patternData) {
    switch(patternData->config){
      case 0:
      default:
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
}



void groovy_noise(Pattern_History* hist, int len, Pattern_Data* patternData) {
    // Assume global access to necessary variables like volume, or pass them as parameters
    switch (patternData->config) {
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

        case 1: // groovy_noise_hue_shift_change
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

    // Common post-processing, if necessary, can be added here
}

void hue_trail(Pattern_History* hist, int len, Pattern_Data* patternData) {
    // Assume fHue and vbrightness are accessible globally or passed as parameters
    switch (patternData->config) {
        case 0: // freq_hue_trail (also default case)
        default: // Default case set to execute the freq_hue_trail pattern
            hist->leds[0] = CHSV(fHue, 255, patternData->brightness);
            hist->leds[1] = CHSV(fHue, 255, patternData->brightness);
            for (int i = len - 1; i > 1; i -= 2) {
                hist->leds[i] = hist->leds[i - 2];
                hist->leds[i - 1] = hist->leds[i - 2];
            }
            break;

        case 1: // sin_hue_trail
            {
                uint16_t sinBeat0 = beatsin16(12, 0, len - 1);
                hist->leds[sinBeat0] = CHSV(fHue, 255, patternData->brightness);
                fadeToBlackBy(hist->leds, len, 5);
            }
            break;

        case 2: // freq_hue_trail_mid
            hist->leds[len / 2 - 1] = CHSV(fHue, 255, patternData->brightness);
            hist->leds[len / 2] = CHSV(fHue, 255, patternData->brightness);
            for (int i = len - 1; i > len / 2; i -= 2) {
                hist->leds[i] = hist->leds[i - 2];
                hist->leds[i - 1] = hist->leds[i - 2];
            }
            for (int i = 0; i < len / 2; ++i) {
                hist->leds[len / 2 - i] = hist->leds[len / 2 + i];
            }
            break;

        case 3: // freq_hue_trail_mid_blur
            hist->leds[len / 2 - 1] = CHSV(fHue, 255, patternData->brightness);
            hist->leds[len / 2] = CHSV(fHue, 255, patternData->brightness);
            for (int i = len - 1; i > len / 2; i -= 2) {
                hist->leds[i] = hist->leds[i - 2];
                hist->leds[i - 1] = hist->leds[i - 2];
            }
            for (int i = 0; i < len / 2; ++i) {
                hist->leds[len / 2 - i] = hist->leds[len / 2 + i];
            }
            blur1d(hist->leds, len, 20);
            break;
    }

    // reverse (?)
}

void talking(Pattern_History *hist, int len, Pattern_Data *patternData) {
  // Common variables
  int offsetFromVolume;
  int midpoint = len / 2;

  // Use the config field from patternData to determine the pattern
  switch (patternData->config) {
    case 1: { // talking_formants pattern
      // Assume density_formant and other relevant functions/variables are defined elsewhere
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

    case 2: { // talking_moving pattern
      offsetFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 0, 12500);

      uint16_t sinBeat0 = beatsin16(5, 2, len - 3, 0, 250);
      uint16_t sinBeat1 = beatsin16(5, 2, len - 3, 0, 0 - offsetFromVolume);
      uint16_t sinBeat2 = beatsin16(5, 2, len - 3, 0, 750 + offsetFromVolume);

      hist->leds[sinBeat0] = CHSV(fHue + 100, 255, MAX_BRIGHTNESS);
      hist->leds[sinBeat1] = CHSV(fHue, 255, MAX_BRIGHTNESS);
      hist->leds[sinBeat2] = CHSV(fHue, 255, MAX_BRIGHTNESS);
    } break;

    default: // Default case set to execute the talking_hue pattern
    case 0: 
      hist->leds[midpoint] = CHSV(fHue / 2, 255, MAX_BRIGHTNESS);
      hist->leds[midpoint - offsetFromVolume] = CHSV(fHue, 255, MAX_BRIGHTNESS);
      hist->leds[midpoint + offsetFromVolume] = CHSV(fHue, 255, MAX_BRIGHTNESS);
      break;
  }

  // Common effects for all modes
  blur1d(hist->leds, len, 80);
  // Adjust fade value based on the pattern
  int fadeValue = (patternData->config == 0 || patternData->config == 2) ? 150 : (patternData->config == 1) ? 200 : 100;
  fadeToBlackBy(hist->leds, len, fadeValue);
}

void glitch_effect(Pattern_History * hist, Pattern_Data * patternData , int len) {
    int offsetFromVolume, speedFromVolume;
    uint16_t sinBeat[4]; 
    double *formants;
    double f0Hue;
    
    speedFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 5, mode == 0 ? 25 : 20);
    switch (mode) {
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

    hist->leds[sinBeat[i]] = CHSV((mode == 0 && i == 1) ? f0Hue : fHue * (mode == 1 && i == 0 ? 2 : 1), 255, MAX_BRIGHTNESS);
    
    // Common effects: blur and fade
    blur1d(hist->leds, len, 80);
    fadeToBlackBy(hist->leds, len, mode == 1 ? 100 : (mode == 2 ? 60 : 40)); 
}

// need to add edge case thingy for ODD lens
// Function to process the direction of the LED buffer
void processDirection(Pattern_History* hist, Subpattern_Data * patternData, int len) {
    // reverse buffer direction
    if (patternData->direction == 1) {
        // Reverse the buffer
        // use half the len to ensure values only swapped once
        for (int i = 0; i < len / 2; i++) {
            CRGB temp = hist->leds[i];
            hist->leds[i] = hist->leds[len - 1 - i];
            hist->leds[len - 1 - i] = temp;
        }
    // from the middle mirror
    } else if (patternData->direction == 2) {
        // Temporary buffer to hold the condensed pattern and its mirrored counterpart
        CRGB temp[len]; // Ensure this is large enough for your LED strip

        // Condense the pattern to the right half of the strip
        for (int i = 0; i < len / 2; i++) {
            // Map the full strip's indices to half its length, focusing on the right side
            temp[len / 2 + i] = hist->leds[map(i, 0, len / 2 - 1, 0, len - 1)];
        }
        // Mirror this condensed pattern onto the left side in reverse
        for (int i = 0; i < len / 2; i++) {
            temp[i] = temp[len - 1 - i];
        }

        // Copy the temp buffer back to the original LED buffer
        for (int i = 0; i < len; i++) {
            hist->leds[i] = temp[i];
        }
      // from the ends mirror
    } else if (patternData->direction == 3 ){ 
        // Temporary buffer to hold the condensed pattern
        CRGB temp[len / 2];
        // Condense the pattern onto the first half of the temp buffer
        for (int i = 0; i < len / 2; i++) {
            // This effectively maps the full range of LEDs to half, condensing the pattern
            temp[i] = hist->leds[map(i, 0, len / 2 - 1, 0, len - 1)];
        }
        // Now mirror this condensed pattern onto the second half of the temp buffer
        for (int i = 0; i < len / 2; i++) {
            temp[len / 2 + i] = temp[len / 2 - 1 - i];
        }
        
        // Copy the temp buffer back to the original LED buffer
        for (int i = 0; i < len; i++) {
            hist->leds[i] = temp[i % (len / 2)];
    }
}



/*
void echo_ripple(){
    //prevents dead pixels from staying
    for (int i = 0; i < NUM_LEDS; i++) {
      hist->leds[i] = CRGB::Black;  // Set to black (off)
    }
    FastLED.show();
    
    //used to get the frequency info
    double* formants = density_formant();
    double f0 = remap(formants[0], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255);
    double f1 = remap(formants[1], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255);
    double f2 = remap(formants[2], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255);

    // //color based off the formant hues
    CRGB ripple_color = CRGB(f0, f1, f2);

    int randomPixel1 = random(NUM_LEDS);
    int randomPixel2 = random(NUM_LEDS);
    int randomPixel3 = random(NUM_LEDS);

    //255/5 is 51, where the goal is to fade in slowly 51 times
    double brightness_inc1 = f0/51;
    double brightness_inc2 = f1/51;
    double brightness_inc3 = f2/51;
    double brightness1 = 0, brightness2 = 0, brightness3 = 0;

    //increments the brightness for each formant color
    for (; brightness1 <= f1; brightness1 += brightness_inc1, brightness2 += brightness_inc2, brightness3 += brightness_inc3) {
      hist->leds[randomPixel1] = CRGB(brightness1, 0, brightness3); // Set the color 
      hist->leds[randomPixel2] = CRGB(brightness1, brightness2, 0); 
      hist->leds[randomPixel3] = CRGB(0, brightness2, brightness3); 

      hist->leds[randomPixel1 + 1] = CRGB(brightness1, 0, brightness3);
      hist->leds[randomPixel2 + 1] = CRGB(brightness1, brightness2, 0);
      hist->leds[randomPixel3 + 1] = CRGB(0, brightness2, brightness3);

      hist->leds[randomPixel1 - 1] = CRGB(brightness1, 0, brightness3);
      hist->leds[randomPixel2 - 1] = CRGB(brightness1, brightness2, 0);
      hist->leds[randomPixel3 - 1] = CRGB(0, brightness2, brightness3);

      FastLED.show();
      delay(10); // Adjust the delay for the fade-in speed
    }

    // Wait for a few seconds
    delay(400); // Adjust the delay as needed

    //fades out formant colors
    for (; brightness1 >= 0; brightness1 -= brightness_inc1, brightness2 -= brightness_inc2, brightness3 -= brightness_inc3) {
      hist->leds[randomPixel1] = CRGB(brightness1, 0, brightness3); // Set the color 
      hist->leds[randomPixel2] = CRGB(brightness1, brightness2, 0); 
      hist->leds[randomPixel3] = CRGB(0, brightness2, brightness3); 

      hist->leds[randomPixel1 + 1] = CRGB(brightness1, 0, brightness3);
      hist->leds[randomPixel2 + 1] = CRGB(brightness1, brightness2, 0);
      hist->leds[randomPixel3 + 1] = CRGB(0, brightness2, brightness3);

      hist->leds[randomPixel1 - 1] = CRGB(brightness1, 0, brightness3);
      hist->leds[randomPixel2 - 1] = CRGB(brightness1, brightness2, 0);
      hist->leds[randomPixel3 - 1] = CRGB(0, brightness2, brightness3);

      FastLED.show();
      delay(20); // Adjust the delay for the fade-out speed
    }
  
    hist->leds[randomPixel1] = CRGB::Black;
    hist->leds[randomPixel2] = CRGB::Black;
    hist->leds[randomPixel3] = CRGB::Black;

    int distance = static_cast<int>(f0)+1;
    brightness1 = 0, brightness2 = 0, brightness3 = 0;

    for (int cur_iter = 1; cur_iter < distance; cur_iter++, brightness1 += brightness_inc1, brightness2 += brightness_inc2, brightness3 += brightness_inc3) {
      // Fade out the previous pixels
      for (int i = 0; i < NUM_LEDS; i++) {
        hist->leds[i].fadeToBlackBy(15); // Adjust the fade value as needed
      }

      // changes color after half way point of distance and allows for color blending
      int fade_out_brightness = (cur_iter >= distance/2) ? brightness_inc1 * (distance - cur_iter) : brightness_inc1;

      hist->leds[randomPixel1 + cur_iter] = CRGB(fade_out_brightness, 0, brightness3);
      hist->leds[randomPixel2 + cur_iter] = CRGB(fade_out_brightness, brightness2, 0);
      hist->leds[randomPixel3 + cur_iter] = CRGB(0, brightness2, brightness3);

      hist->leds[randomPixel1 - cur_iter] = CRGB(fade_out_brightness, 0, brightness3);
      hist->leds[randomPixel2 - cur_iter] = CRGB(fade_out_brightness, brightness2, 0);
      hist->leds[randomPixel3 - cur_iter] = CRGB(0, brightness2, brightness3);

      delay(30);
      FastLED.show();
    }

    // fade out all pixels before patten ends
    for (int fade_out_value = 255; fade_out_value >= 0; fade_out_value--) {
      for (int i = 0; i < NUM_LEDS; i++) {
        hist->leds[i].fadeToBlackBy(3);  // Adjust fade value as needed
      }
      FastLED.show();
      delay(10);  // Adjusted delay for the fade-out speed
    }

  //release formants alloc memory
  delete[] formants;
}
*/