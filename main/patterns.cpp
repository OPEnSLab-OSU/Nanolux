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
extern uint8_t manual_pattern_idx;
extern volatile bool manual_control_enabled;

void nextPattern() {
  // add one to the current pattern number, and wrap around at the end
  if(manual_control_enabled){
    manual_pattern_idx = (manual_pattern_idx + 1) % NUM_PATTERNS;
  }else{
    manual_control_enabled = true;
  }
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


void freq_hue_vol_brightness(Pattern_History * hist, int len){
  if(config.debug_mode == 1){
    Serial.print("\t pattern: freq_hue_vol_brightness\t fHue: ");
    Serial.print(fHue);
    Serial.print("\t vbrightness: ");
    Serial.println(vbrightness);
  }

  CHSV color = CHSV(fHue, 255, vbrightness);
  fill_solid(hist->leds, len, color);
}


void freq_confetti_vol_brightness(Pattern_History * hist, int len){
  // colored speckles based on frequency that blink in and fade smoothly
  fadeToBlackBy(hist->leds, len, 20);
  int pos = random16(len);
  hist->leds[pos] += CHSV( fHue + random8(10), 255, vbrightness);
  hist->leds[pos] += CHSV( fHue + random8(10), 255, vbrightness);
}


void volume_level_middle_bar_freq_hue(Pattern_History * hist, int len){
  clearLEDSegment(hist, len);

  int n = remap(volume, MIN_VOLUME, MAX_VOLUME, 0, len/2);
  int mid_point = (int) len/2;
  
  for(int led = 0; led < n; led++) {
    hist->leds[mid_point + led].setHue( fHue );
    hist->leds[mid_point - led].setHue( fHue );
  }
}


void freq_hue_trail(Pattern_History * hist, int len){
  hist->leds[0] = CHSV( fHue, 255, vbrightness);
  hist->leds[1] = CHSV( fHue, 255, vbrightness);
  CRGB temp;
  
  for(int i = len-1; i > 1; i-=2) {
      hist->leds[i] = hist->leds[i-2];
      hist->leds[i-1] = hist->leds[i-2];
  }
}

void blank(Pattern_History * hist, int len){
  clearLEDSegment(hist, len);
}


void spring_mass_1 (Pattern_History * hist, int len){
  int middle_mass_displacement = 0;
  int middle_position = len / 2;
  int mass = 5;
  int mass_radius = 12;
  int spring_constant = 5;
  double period_avg = 0;
  double friction = 1;

  if (hist->amplitude < middle_position) {
    hist->amplitude += vbrightness / 7;
  } 

  middle_mass_displacement = hist->amplitude*cos(sqrt(spring_constant/mass)*(hist->frame)/3);
  hist->frame++;
  
  if (hist->amplitude > friction) {
    hist->amplitude = hist->amplitude - friction; 
  } else {
    hist->amplitude = 0;
  }

  int left_end = middle_position + middle_mass_displacement - mass_radius;
  int right_end = middle_position + middle_mass_displacement + mass_radius;
  
  for (int i = 0; i < len-1; i++)
  {
    if ((i > left_end) && (i < right_end))
    {
      int springbrightness = (90/mass_radius) * abs(mass_radius - abs(i - (middle_mass_displacement+middle_position)));
      hist->leds[i] = CHSV (fHue, 255-vbrightness, springbrightness);
    } else {
      hist->leds[i] = CHSV (0, 0, 0);
    }
  }
}

void spring_mass_2 (Pattern_History * hist, int len) {
  int middle_position = len / 2;
  int mass = 5;
  int mass_radius = 15;
  int friction = 100;
  double spring_constant = 0;

  for (int i = 10; i > 0; i--) {
    spring_constant += hist->smoothing_value[i];
    hist->smoothing_value[i] = hist->smoothing_value[i-1];
  }
  hist->smoothing_value[0] = fHue;
  spring_constant += fHue;
  spring_constant = spring_constant / 2550;
  
  hist->acceleration = -1*hist->location * spring_constant/mass;
  if (hist->velocity > 0)
  {
    hist->velocity += hist->acceleration + (vbrightness/80);
  } else {
    hist->velocity += hist->acceleration - (vbrightness/80);
  }
  hist->location += hist->velocity;
  if (hist->location < -1*len/2)
  {
    hist->location = -1*len/2;
  } else if (hist->location > len/2) {
    hist->location = len/2;
  }

  int left_end = middle_position + hist->location - mass_radius;
  int right_end = middle_position + hist->location + mass_radius;
  
  for (int i = 0; i < len-1; i++)
  {
    if ((i > left_end) && (i < right_end))
    {        
      int springbrightness = 90 - (90/mass_radius * abs(i - (hist->location+middle_position)));
      hist->leds[i] = CHSV (spring_constant * 255, 255-vbrightness, springbrightness);
    } else {
      hist->leds[i] = CHSV (0, 0, 0);
    }
  }
}

void spring_mass_3(Pattern_History * hist, int len) {
  int middle_position = len / 2;
  int mass = 5;
  int mass_radiuses[5] = {6,5,4,3,2};
  int friction = 100;
  double spring_constants[5] = {0.05, 0.10, 0.15, 0.20, 0.25};
  double tempsum = 0;
  
  for (int k = 0; k < 5; k++){      
    for (int i = (3+(k*SAMPLES/5)); i < ((k+1)*SAMPLES/5)-3; i++) {
      tempsum +=  vReal[i];
    }
    hist->vRealSums[k] = tempsum/(SAMPLES/5);
    hist->vRealSums[k] = remap(hist->vRealSums[k], MIN_VOLUME, MAX_VOLUME, 0, 5);
  }
  
  for (int j = 0; j < 5; j++) {
    hist->accelerations[j] = -1*hist->locations[j] * spring_constants[j]/mass;
    if (hist->velocity > 0)
    {
      hist->velocities[j] += hist->accelerations[j] + (hist->vRealSums[j]);
    } else {
      hist->velocities[j] += hist->accelerations[j] - (hist->vRealSums[j]);
    }
    hist->locations[j] += hist->velocities[j];
    if (hist->locations[j] < -1*len/2)
    {
      hist->locations[j] = -1*len/2;
    } else if (hist->locations[j] > len/2) {
      hist->locations[j] = len/2;
    }

    int left_end = middle_position + hist->locations[j] - mass_radiuses[j];
    int right_end = middle_position + hist->locations[j] + mass_radiuses[j];
  
    for (int i = 0; i < len-1; i++)
    {
      if ((i > left_end) && (i < right_end))
      {        
        //int springbrightness = 90 - (90/mass_radius * abs(i - (hist->locations[j]+middle_position)));
        hist->leds[i] = CHSV (spring_constants[j] * 255 * 4, 255-vbrightness, 80);
      } else {
        hist->leds[i] -= CHSV (0, 0, 10);
      }
    }
  } 
}

// Represents classical music comprehensively on the light strip ("Claire de Lune" looks great when volume is sufficient)
// NOTE: Does not make use of global formants variables, but should
void classical(Pattern_History * hist, int len) {
  double* temp_formants = density_formant();
  int bpm = map(vbrightness, 0, MAX_BRIGHTNESS, 1, 30);
  uint16_t sin = beatsin16(bpm, 0, temp_formants[1]);

  fadeToBlackBy(hist->leds, len, 50);

  CRGBPalette16 myPal = hue_gp;
  fill_palette(hist->leds, len, sin, 255/len, myPal, 50, LINEARBLEND);

  if(config.debug_mode == 1){
    Serial.print("\t Classical: ");
  }

  delete[] temp_formants;
}

// Based on a sufficient volume, a pixel will float to some position on the light strip and fall down (vol_show adds another threshold)
void pix_freq(Pattern_History * hist, int len) {
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

// A mirroed version of the pix_freq, splittion it on two sides starting from the middle
void mirror_pix_freq(Pattern_History * hist, int len) {
  fadeToBlackBy(hist->leds, len, 50);
        
  if (volume > 125) {
    hist->pix_pos = (len / 2) + map(peak, MIN_FREQUENCY, MAX_FREQUENCY, -len/2 , len / 2);
    hist->tempHue = fHue;
    
  } else {
    hist->pix_pos--;
    hist->vol_pos--;
  }
  if (vol_show) {
    if (volume > 75) {
      hist->vol_pos = (len / 2) + map(volume, MIN_VOLUME, MAX_VOLUME, len/2 , len / 2);
      hist->tempHue = fHue;
    } else {
      hist->vol_pos--;
    }
    hist->leds[len / 2 - 1 - hist->vol_pos] =  hist->vol_pos < len/2 ? CRGB(255, 255, 255):CRGB(0, 0, 0);
    hist->leds[len / 2 + 1 + hist->vol_pos] = hist->vol_pos < len/2 ? CRGB(255, 255, 255):CRGB(0, 0, 0);
  }
  hist->leds[len/2 - 1 - hist->pix_pos] = hist->pix_pos < len/2 ? CHSV(hist->tempHue, 255, 255):CRGB(0, 0, 0);
  hist->leds[len/2 + 1 + hist->pix_pos] = hist->pix_pos < len/2 ? CHSV(hist->tempHue, 255, 255):CRGB(0, 0, 0);
}

// Utility function for sending a wave with sine for the math rock function
void send_wave(Pattern_History * hist, int len) {
  double change_by = vbrightness;
  int one_sine = map(change_by, 0, MAX_BRIGHTNESS, 25, 35);
  CRGB color = CRGB(0, one_sine/2, 50);
  fill_solid(hist->leds, len, color);
  uint8_t sinBeat = beatsin8(30, 0, len-1, 0, 0);
  hist->leds[sinBeat] = CRGB(10, 10, 0);
  fadeToBlackBy(hist->leds, len, 1);
  uint8_t sinBeat1 = beatsin8(one_sine, 0, len-1, 0, 170);
  hist->leds[sinBeat1] = CRGB(255, 0, 0);
  fadeToBlackBy(hist->leds, len, 1);
  uint8_t sinBeat2 = beatsin8(one_sine, 0, len-1, 0, 255);
  hist->leds[sinBeat2] = CRGB(255,255,255);
  fadeToBlackBy(hist->leds, len, 1);

  for (int i = 0; i < 20; i++) {
    blur1d(hist->leds, len, 50);
  }
  fadeToBlackBy(hist->leds, len, 50);

  if(config.debug_mode == 1){
    Serial.print("\t sinBeat: ");
    Serial.print(sinBeat);
    Serial.print("\t sinBeat1: ");
    Serial.print(sinBeat1);
    Serial.print("\t sinBeat2: ");
    Serial.print(sinBeat2);
  }
}

// Represents math rock music comprehensively on the light strip ("Waterslide" by Chon looks great)
void math(Pattern_History * hist, int len) {
  int l = (sizeof(vReal)/sizeof(vReal[0])) / 10;
  double smol_arr[l];
  memcpy(smol_arr, vReal + (8*l), l-1);
  double sums = 0;
  for (int i = 0; i < l; i++) {
    sums += smol_arr[i];
  }
  int vol = sums/l; // Get a volume for the snare^
  uint8_t brit = remap(vol, MIN_VOLUME, MAX_VOLUME, 0, MAX_BRIGHTNESS);  // Remap the snare volume to brightness
  double red = 0.;
  double green = 0.;
  double blue = 0.;

  for (int i = 5; i < SAMPLES - 3; i++) {
    if (i < (SAMPLES-8)/3) {
      blue += vReal[i];
    }
    else if (i >= (SAMPLES-8)/3 && i < (2*(SAMPLES-8)/3)) {
      green += vReal[i];
    }
    else {
      red += vReal[i];
    }
  }

  red /= 300;
  blue /= 300;
  green /= 300;

  uint8_t hue = remap(log ( red + green + 2.5*blue ) / log ( 2 ), log ( MIN_FREQUENCY ) / log ( 2 ), log ( MAX_FREQUENCY ) / log ( 2 ), 10, 240);
  hist->genre_smoothing[hist->genre_pose] = hue;
  hist->genre_pose++;
  if (hist->genre_pose == 10) {
    hist->genre_pose = 0;
  }
  // Get average hue values for the middle frequency^^^

  
  int nue = 0;
  for (int i = 0; i < 10; i++) {
    nue += hist->genre_smoothing[i];
  }
  hue = nue/8;

  hist->frame++;

  CHSV color = CHSV(255-hue, 255, 4*brit);

  int mid_point = (int) len/2;
  fill_solid(hist->leds, len, color);
  send_wave(hist, len); // Sends the wave based on volume

  red = map(red, 0, 1500, 0, (len/2)-1);
  
  // DO the middle bar from the new hue calculated above^^^^^
  for(int led = 0; led < red; led++) {
    hist->leds[mid_point + led] = CHSV(2*hue, 255, 3*brit);
    hist->leds[mid_point - led] = CHSV(2*hue, 255, 3*brit);
  }
  fadeToBlackBy(hist->leds, len, 50);

  #ifdef DEBUG
    Serial.print("\t Red: ");
    Serial.print(red);
    Serial.print("\t Green: ");
    Serial.print(green);
    Serial.print("\t Blue: ");
    Serial.print(blue);
    Serial.print("\t Hue: ");
    Serial.print(hue);
    Serial.print("\t vbrightness: ");
    Serial.println(brit);
  #endif
}

// Changes the brightness of the five bands based on how much exist on the five sample split (the better one imo)
void band_brightness(Pattern_History * hist, int len) {
  double *fiveSamples = band_split_bounce(len);
  if(config.debug_mode == 1){
    Serial.print("Vol1:");
    Serial.print(fiveSamples[0]);
    Serial.print("\tVol2:");
    Serial.print(fiveSamples[1]);
    Serial.print("\tVol3:");
    Serial.print(fiveSamples[2]);
    Serial.print("\tVol4:");
    Serial.print(fiveSamples[3]);
    Serial.print("\tVol5:");
    Serial.print(fiveSamples[4]);
  }

  // Map each chunk of the Light strip to a brightness from the band sample split
  for (int i = 0; i < len/5; i++) {
    hist->leds[i] = CHSV(0,255, map(fiveSamples[0], 0, 5, 0, 255));
  }
  for (int i = len/5; i < 2*len/5; i++) {
    hist->leds[i] = CHSV(60,255, map(fiveSamples[1], 0, 5, 0, 255));
  }
  for (int i = 2*len/5; i < 3*len/5; i++) {
    hist->leds[i] = CHSV(100,255, map(fiveSamples[2], 0, 5, 0, 255));
  }
  for (int i = 3*len/5; i < 4*len/5; i++) {
    hist->leds[i] = CHSV(160,255, map(fiveSamples[3], 0, 5, 0, 255));
  }
  for (int i = 4*len/5; i < len; i++) {
    hist->leds[i] = CHSV(205,255, map(fiveSamples[4], 0, 5, 0, 255));
  }

  fadeToBlackBy(hist->leds, len, 10);

  delete [] fiveSamples;
}

// Implements a falling pixel on top of basic bands
// The falling pixel is the max volume at a point and then float down instead of just assigning volume
void advanced_bands(Pattern_History * hist, int len) {
  double avg1 = 0;
  double avg2 = 0;
  double avg3 = 0;
  double avg4 = 0;
  double avg5 = 0;


  // Calculate the volume of each band
  for (int i = 0; i < advanced_size; i++) {
    avg1 += hist->max1[i];    
  }
  avg1 /= advanced_size;
  for (int i = 0; i < advanced_size; i++) {
    avg2 += hist->max2[i];
  }
  avg2 /= advanced_size;
  for (int i = 0; i < advanced_size; i++) {
    avg3 += hist->max3[i];
  }
  avg3 /= advanced_size;
  for (int i = 0; i < advanced_size; i++) {
    avg4 += hist->max4[i];
  }
  avg4 /= advanced_size;
  for (int i = 0; i < advanced_size; i++) {
    avg5 += hist->max5[i];
  }
  avg5 /= advanced_size;

  double *fiveSamples = band_split_bounce(len);

  double vol1 = fiveSamples[0];
  double vol2 = fiveSamples[1];
  double vol3 = fiveSamples[2];
  double vol4 = fiveSamples[3];
  double vol5 = fiveSamples[4]; 

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
    hist->max1[hist->maxIter] = 0;
  }
  else {
      for (int i = 0; i < 5; i++) {
        hist->max1[i] = vol1;
      }
  }
    
  if (vol2 <= avg2) {
    hist->max2[hist->maxIter] = 0;
  }
  else {
      for (int i = 0; i < 5; i++) {
        hist->max2[i] = vol2;
      }
  }

  if (vol3 <= avg3) {
    hist->max3[hist->maxIter] = 0;
  }
  else {
      for (int i = 0; i < 5; i++) {
        hist->max3[i] = vol3;
      }
  }

  if (vol4 <= avg4) {
    hist->max4[hist->maxIter] = 0;
  }
  else {
      for (int i = 0; i < 5; i++) {
        hist->max4[i] = vol4;
      }
  }

  if (vol5 <= avg5) {
    hist->max5[hist->maxIter] = 0;
  }
  else {
      for (int i = 0; i < 5; i++) {
        hist->max5[i] = vol5;
      }
  }

  // Get this smoothed array to loop to beginning again once it is at teh end of the falling pixel smoothing
  if (hist->maxIter == advanced_size-1) {
    hist->maxIter = 0;
  } else {
    hist->maxIter++;
  }

  // Fill the respective chunks of the light strip with the color based on above^
  for (int i = 0; i < vol1-1; i++) {
    hist->leds[i] = CRGB(255,0,0);
  }
  for (int i = len/5; i < len/5+vol2-1; i++) {
    hist->leds[i] = CRGB(255,255,0);
  }
  for (int i = 2*len/5; i < 2*len/5+vol3-1; i++) {
    hist->leds[i] = CRGB(0,255,0);
  }
  for (int i = 3*len/5; i < 3*len/5+vol4-1; i++) {
    hist->leds[i] = CRGB(0,255,255);
  }
  for (int i = 4*len/5; i < 4*len/5+vol5-1; i++) {
    hist->leds[i] = CRGB(0,0,255);
  }
  
  // Assign the values for the pixel to goto
  hist->leds[(int) avg1+ (int) vol1] = CRGB(255,255,255);
  hist->leds[(int) len/5 + (int) avg2+ (int) vol2] = CRGB(255,255,255);
  hist->leds[(int) 2*len/5+(int) avg3+ (int) vol3] = CRGB(255,255,255);
  hist->leds[(int) 3*len/5+(int) avg4+ (int) vol4] = CRGB(255,255,255);
  hist->leds[(int) 4*len/5+(int) avg5+ (int) vol5] = CRGB(255,255,255);
  fadeToBlackBy(hist->leds, len, 90);

  delete [] fiveSamples;
}

void basic_bands(Pattern_History * hist, int len) {
  fadeToBlackBy(hist->leds, len, 85);

    // double *fiveSamples = band_sample_bounce();
    double *fiveSamples = band_split_bounce(len); // Maybe use above if you want, but its generally agreed this one looks better

    double vol1 = fiveSamples[0];
    double vol2 = fiveSamples[1];
    double vol3 = fiveSamples[2];
    double vol4 = fiveSamples[3];
    double vol5 = fiveSamples[4];

    // Fill each chunk of the light strip with the volume of each band
    for (int i = 0; i < vol1; i++) {
      hist->leds[i] = CRGB(255,0,0);
    }
    for (int i = len/5; i < len/5+vol2; i++) {
      hist->leds[i] = CRGB(255,255,0);
    }
    for (int i = 2*len/5; i < 2*len/5+vol3; i++) {
      hist->leds[i] = CRGB(0,255,0);
    }
    for (int i = 3*len/5; i < 3*len/5+vol4; i++) {
      hist->leds[i] = CRGB(0,255,255);
    }
    for (int i = 4*len/5; i < 4*len/5+vol5; i++) {
      hist->leds[i] = CRGB(0,0,255);
    }

    delete [] fiveSamples;
}

// Shows the frequency array on the light strip (really messy when not gated)
void eq(Pattern_History * hist, int len) {
  blank(hist, len);
  for (int i = 0; i < len; i++) {
    int brit = map(vReal[i], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255); // The brightness is based on HOW MUCH of the frequency exists
    int hue = map(i, 0, len, 0, 255); // The fue is based on position on the light strip, ergo, what frequency it is at
    if (vReal[i] > 200) { // An extra gate because the frequency array is really messy without it
      hist->leds[i] = CHSV(hue, 255, brit);
    }
  }
}

// A version of showing which drums are being played, WAY BETTER when live and using aux port
void show_drums(Pattern_History * hist, int len) {
  int* drums = drum_identify();
  fadeToBlackBy(hist->leds, len, 50);

  if (drums[0]) {
    fill_solid(hist->leds, len, CRGB(255, 0, 0)); // Fill the whole array
  }
  if (drums[1]) {
    fill_solid(hist->leds, 2*len/3, CRGB(0, 255, 0)); // Fill 2/3 of the array
  }
  if (drums[2]) {
    fill_solid(hist->leds, len/3, CRGB(0, 0, 255)); // Fill 1/3 of the array
  }

  delete [] drums;
}

// This is a how one can see the formants through the whole light strip (try holding vowels at the mic and see it be consistent!)
void show_formants(Pattern_History * hist, int len) {
  double *temp_formants = density_formant();
  blank(hist, len);
  fill_solid(hist->leds, len, CRGB(remap(temp_formants[0], 0, SAMPLES, log(1),50), remap(temp_formants[1], 0, SAMPLES, log(1),50), remap(temp_formants[2], 0, SAMPLES, log(1),50)));
  delete [] temp_formants;
}

// Lights up when noisy, not when periodic
void noisy(Pattern_History * hist, int len) {
  if (nvp() == 1) {
    fill_solid(hist->leds, len, CRGB(255, 0, 0)); // Fill Red
  }
  else {
    blank(hist, len); // Empty when audio is registered as periodic 
  }  
  if(config.debug_mode == 1){
    Serial.print("\t CheckVol: ");
  }
}

// Splits up each formant as 1/3 of a band on the light strip. Proves that the formants actually work and looks pretty neat imo
void formant_band(Pattern_History * hist, int len) {
  // Grab the formants
  double *temp_formants = density_formant();
  double f0Hue = remap(temp_formants[0], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255);
  double f1Hue = remap(temp_formants[1], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255);
  double f2Hue = remap(temp_formants[2], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255);

  if(config.debug_mode == 1){
    Serial.print("\t f0Hue: ");
    Serial.print(temp_formants[0]);
    Serial.print("\t f1Hue: ");
    Serial.print(temp_formants[1]);
    Serial.print("\t f2Hue: ");
    Serial.print(temp_formants[2]);
  }

  // Fill 1/3 with each formant
  for (int i = 0; i < len; i++) {
    if (i < len/3) {
      hist->leds[i] = CHSV(f0Hue, 255, 255);
    }
    else if (len/3 <= i && i < 2*len/3) {
      hist->leds[i] = CHSV(f1Hue, 255, 255);
    } 
    else {
      hist->leds[i] = CHSV(f2Hue, 255, 255);
    }
  }

  // Smooth out the result
  for (int i = 0; i < 5; i++) {
    blur1d(hist->leds, len, 50);
  }
  delete[] temp_formants;
}

// An alternative, "old school" way of getting each drums and shows the overall light strip changing based on each piece.
// VERY TOUCHY so don't mess with this too much (even adding comments for some reason messed up the output)
// It does drums the same way as the official global way, but uses FastFFT
void alt_drums(Pattern_History * hist, int len) {
  int l = (sizeof(vReal)/sizeof(vReal[0])) / 7;
  double smol_arr[l];
  memcpy(smol_arr, vReal, l-1);
  double F0 = FFT.MajorPeak(smol_arr, SAMPLES, SAMPLING_FREQUENCY);
  memcpy(smol_arr, vReal + (3*l), l-1);
  double F1 = FFT.MajorPeak(smol_arr, SAMPLES, SAMPLING_FREQUENCY);
  memcpy(smol_arr, vReal + (5*l), l-1);
  double F2 = FFT.MajorPeak(smol_arr, SAMPLES, SAMPLING_FREQUENCY);


  double fHue0 = map(log ( F0/7 ) / log ( 2 ), log ( MIN_FREQUENCY ) / log ( 2 ), log ( MAX_FREQUENCY ) / log ( 2 ), 0, 240);
  double fHue1 = map(log ( F1/7 ) / log ( 2 ), log ( MIN_FREQUENCY ) / log ( 2 ), log ( MAX_FREQUENCY ) / log ( 2 ), 0, 240);
  double fHue2 = map(log ( F2/7 ) / log ( 2 ), log ( MIN_FREQUENCY ) / log ( 2 ), log ( MAX_FREQUENCY ) / log ( 2 ), 0, 240);

  #ifdef DEBUG
    Serial.print("\t pattern: Alt Drums\t \nfHue0: ");
    Serial.print(F0/7);
    Serial.print("\t fHue1: ");
    Serial.println(F1/7);
    Serial.print("\t fHue2: ");
    Serial.println(F2/7);
    Serial.print("\t vbrightness: ");
    Serial.println(vbrightness);
  #endif
  
  CHSV color = CHSV( fHue1, 255, vbrightness);
  fill_solid(hist->leds, len, color);
}

// A test version of showing formants and works kind of well, this is an early design, but kind of works as a quicka dn dirty demo
void formant_test(Pattern_History * hist, int len){
  int l = (sizeof(vReal)/sizeof(vReal[0])) / 5;
  double smol_arr[l];
  memcpy(smol_arr, vReal, l-1);
  int F0 = largest(smol_arr, l);
  memcpy(smol_arr, vReal + l, l-1);
  int F1 = largest(smol_arr, l);
  memcpy(smol_arr, vReal + (2*l), l-1);
  int F2 = largest(smol_arr, l);

  #ifdef DEBUG
    Serial.print("\t pattern: Formant Test\t fHue: ");
    Serial.print(fHue);
    Serial.print("\t vbrightness: ");
    Serial.println(vbrightness);
  #endif
  
  hist->leds[0] = CHSV( fHue, F0, vbrightness);
  hist->leds[1] = CHSV( fHue, F1, vbrightness);
  CRGB temp;
  
  for(int i = NUM_LEDS-1; i > 1; i-=2) {
      hist->leds[i] = hist->leds[i-2];
      hist->leds[i-1] = hist->leds[i-2];
  } 
}

void Fire2012WithPalette(Pattern_History * hist, int len){
  
  int sparkVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 10,200);
  //int coolingVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 60, 40);
  //Serial.println(sparkVolume);
  
  
  // Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS];

// Step 1.  Cool down every cell a little
  for( int i = 0; i < NUM_LEDS; i++) {
    heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for( int k= NUM_LEDS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
  }
  
  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
  if( random8() < sparkVolume ) {
    int y = random8(7);
    heat[y] = qadd8( heat[y], random8(160,255) );
  }

  //Step 3.5. Calcualate Brightness from low frequencies
  int l = (sizeof(vReal)/sizeof(vReal[0])) / 7;
  double smol_arr[l];
  memcpy(smol_arr, vReal, l-1);
    
  // Step 4.  Map from heat cells to LED colors
  for( int j = 0; j < NUM_LEDS; j++) {
    // Scale the heat value from 0-255 down to 0-240
    // for best results with color palettes.
  
    byte colorindex = scale8( heat[j], 240);
    //byte colorindex = scale8( heat[j], smol_arr);
    CRGB color = ColorFromPalette( gPal, colorindex);
    int pixelnumber;
    if( gReverseDirection ) {
      pixelnumber = (NUM_LEDS-1) - j;
    } else {
      pixelnumber = j;
    }
    hist->leds[pixelnumber] = color;
  }
}
//adapted from s-marley
void saturated_noise(Pattern_History * hist, int len){

  //Set params for fill_noise16()
  uint8_t octaves = 1;
  uint16_t x = 0;
  int scale = 300;
  uint8_t hue_octaves = 1;
  uint16_t hue_x = 100;
  int hue_scale = 20; 
  uint16_t ntime = millis() / 3;
  uint8_t hue_shift =  50;

  //Fill LEDS with noise using parameters above
  fill_noise16 (hist->leds, len, octaves, x, scale, hue_octaves, hue_x, hue_scale, ntime, hue_shift);
  //Add blur
  blur1d(hist->leds, len, 80);
}

void saturated_noise_hue_octaves(Pattern_History * hist, int len){

  //Remap volume variable that will change hue_octaves below
  int hueOctavesFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 1, 10);
  
  //Set params for fill_noise16()
  uint8_t octaves = 1;
  uint16_t x = 0;
  int scale = 300;
  uint8_t hue_octaves = hueOctavesFromVolume;
  uint16_t hue_x = 100;
  int hue_scale = 20; 
  uint16_t ntime = millis() / 3;
  uint8_t hue_shift =  50;
  
  //Fill LEDS with noise using parameters above
  fill_noise16 (hist->leds, len, octaves, x, scale, hue_octaves, hue_x, hue_scale, ntime, hue_shift);
  //Add blur
  blur1d(hist->leds, len, 80); 
}

void saturated_noise_hue_shift(Pattern_History * hist, int len){

  //Remap volume variable that will change octaves and hue_shift below
  int shiftFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 50, 100);

  //Set params for fill_noise16()
  uint8_t octaves = shiftFromVolume;
  uint16_t x = 0;
  int scale = 230;
  uint8_t hue_octaves = 1;
  uint16_t hue_x = 150;
  int hue_scale = 20; //prev 50
  uint16_t ntime = millis() / 3;
  uint8_t hue_shift =  shiftFromVolume;

  //Fill LEDS with noise using parameters above
  fill_noise16 (hist->leds, len, octaves, x, scale, hue_octaves, hue_x, hue_scale, ntime, hue_shift);
  //Add blur
  blur1d(hist->leds, len, 80);
}

void saturated_noise_compression(Pattern_History * hist, int len){

  //Remap volume variable that will change hue_x below
  int shiftFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 1, 8);

  //Set params for fill_noise16()
  uint8_t octaves = 1;
  uint16_t x = 0;
  int scale = 300;
  uint8_t hue_octaves = 1;
  uint16_t hue_x = shiftFromVolume;
  int hue_scale = 20; //prev 50
  uint16_t ntime = millis() / 4;
  uint8_t hue_shift =  50;

  //Fill LEDS with noise using parameters above
  fill_noise16 (hist->leds, len, octaves, x, scale, hue_octaves, hue_x, hue_scale, ntime, hue_shift);
  //Add blur
  blur1d(hist->leds, len, 80);
}

void groovy_noise(Pattern_History * hist, int len){
  
  //setup parameters for fill_noise16()
  uint8_t octaves = 1;
  uint16_t x = 0;
  int scale = 100;
  uint8_t hue_octaves = 1;
  uint16_t hue_x = 1;
  int hue_scale = 50;
  uint16_t ntime = millis() / 3;
  uint8_t hue_shift =  5;
  
  //Fill LEDS with noise using parameters above
  fill_noise16 (hist->leds, len, octaves, x, scale, hue_octaves, hue_x, hue_scale, ntime, hue_shift);
  
  //Add blur
  blur1d(hist->leds, len, 80); 
}

void groovy_noise_hue_shift_change(Pattern_History * hist, int len){

  // 210-230 is good range for the last parameter in shiftFromVolume
  int shiftFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 5, 220);
  int xFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 1, 2);

  //Setup parameters for fill_noise16()
  uint8_t octaves = 1;
  uint16_t x = 0;
  int scale = 100;
  uint8_t hue_octaves = 1;
  uint16_t hue_x = xFromVolume;
  int hue_scale = 20;
  uint16_t ntime = millis() / 3;
  uint8_t hue_shift =  shiftFromVolume;

  //Fill LEDS with noise using parameters above
  fill_noise16 (hist->leds, len, octaves, x, scale, hue_octaves, hue_x, hue_scale, ntime, hue_shift);
  //Blur for smoother look
  blur1d(hist->leds, len, 80); 
}

void sin_hue_trail(Pattern_History * hist, int len){

  //Create sin beat
  uint16_t sinBeat0  = beatsin16(12, 0, len-1, 0, 0);
  
  //Given the sinBeat and fHue, color the LEDS and fade
  hist->leds[sinBeat0]  = CHSV(fHue, 255, MAX_BRIGHTNESS);
  fadeToBlackBy(hist->leds, len, 5);
}

//Frequency hue trail that expands outward from the center. Adjusted from legacy freq_hue_trail
void freq_hue_trail_mid(Pattern_History * hist, int len){
    
  //color middle LEDs based on fHue
  hist->leds[len/2-1] = CHSV( fHue, 255, vbrightness);
  hist->leds[len/2] = CHSV( fHue, 255, vbrightness);
    
  //Move LEDS outward from the middle (only on one half)
  for(int i = len-1; i > len/2; i-=2) {
    hist->leds[i] = hist->leds[i-2];
    hist->leds[i-1] = hist->leds[i-2];
  }

  //Mirror LEDS from one half to the other half
  for(int i = 0; i < len/2; ++i){
    hist->leds[len/2-i] = hist->leds[len/2+i];
  }
}

void freq_hue_trail_mid_blur(Pattern_History * hist, int len){
    
  //color middle LEDs based on fHue
  hist->leds[len/2-1] = CHSV( fHue, 255, vbrightness);
  hist->leds[len/2] = CHSV( fHue, 255, vbrightness);
    
  //Move LEDS outward from the middle (only on one half)
  for(int i = len-1; i > len/2; i-=2) {
    hist->leds[i] = hist->leds[i-2];
    hist->leds[i-1] = hist->leds[i-2];
  }

  //Mirror LEDS from one half to the other half
  for(int i = 0; i < len/2; ++i){
    hist->leds[len/2-i] = hist->leds[len/2+i];
  }
  
  //Add blur
  blur1d(hist->leds, len, 20); 
}

void talking_hue(Pattern_History * hist, int len){

  //remap the volume variable to move the LEDS from the middle to the outside of the strip
  int offsetFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 1, len/2);
  int midpoint = len/2;
  
  //3 LED groups. One stationary in the middle, two that move outwards based on volume
  hist->leds[midpoint]                   = CHSV(fHue/2, 255, MAX_BRIGHTNESS);
  hist->leds[midpoint-offsetFromVolume]  = CHSV(fHue, 255,   MAX_BRIGHTNESS);
  hist->leds[midpoint+offsetFromVolume]  = CHSV(fHue, 255,   MAX_BRIGHTNESS);

  //Add blur and quick fade
  blur1d(hist->leds, len, 80);
  fadeToBlackBy(hist->leds, len, 150);
}

void talking_formants(Pattern_History * hist, int len){

  //Use formant analysis
  double *formants = density_formant();
  double f0Hue = remap(formants[0], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255);
  double f1Hue = remap(formants[1], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255);
  double f0 = remap(formants[0], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255);
  double f1 = remap(formants[1], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255);
  double f2 = remap(formants[2], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255);
  
  //remap the volume variable to move the LEDS from the middle to the outside of the strip
  int offsetFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 1, 30);
 
  //3 LED groups. One stationary in the middle, two that move outwards 
  hist->leds[len/2]                   = CRGB(f0, f1, f2);
  hist->leds[len/2-offsetFromVolume]  = CHSV(f0Hue, 255, MAX_BRIGHTNESS);
  hist->leds[len/2+offsetFromVolume]  = CHSV(f0Hue, 255, MAX_BRIGHTNESS);

  //blur and fade
  blur1d(hist->leds, len, 80);
  fadeToBlackBy(hist->leds, len, 200);
  
  //reset formant array for next loop
  delete[] formants;
}

void talking_moving(Pattern_History * hist, int len){
  
  //Last var good range (7500-12500)
  int offsetFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 0, 12500);

  //Create 3 sin beats with the offset(last parameter) changing based on offsetFromVolume
  uint16_t sinBeat0   = beatsin16(5, 2, len-3, 0, 250);
  uint16_t sinBeat1  = beatsin16(5, 2, len-3, 0, 0 - offsetFromVolume);
  uint16_t sinBeat2  = beatsin16(5, 2, len-3, 0, 750 + offsetFromVolume);

  //Given the sinBeats and fHue, color the LEDS
  hist->leds[sinBeat0]  = CHSV(fHue+100, 255, MAX_BRIGHTNESS);
  hist->leds[sinBeat1]  = CHSV(fHue, 255, MAX_BRIGHTNESS);
  hist->leds[sinBeat2]  = CHSV(fHue, 255, MAX_BRIGHTNESS);

  //Add blur and fade
  blur1d(hist->leds, len, 80);
  fadeToBlackBy(hist->leds, len, 100);
}

void bounce_back(Pattern_History * hist, int len){
  
  //Last var good range (7500-12500)
  int offsetFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 0, 12500);

  //Create 2 sinBeats with the offset(last parameter) of sinBeat2 changing based on offsetFromVolume
  uint16_t sinBeat   = beatsin16(6, 2, len-3, 0, 500);
  uint16_t sinBeat2  = beatsin16(6, 2, len-3, 0, 0 - offsetFromVolume);

  //Given the sinBeats and fHue, color the LEDS
  hist->leds[sinBeat]   = CHSV(fHue-25, 255, MAX_BRIGHTNESS);
  hist->leds[sinBeat2]  = CHSV(fHue, 255, MAX_BRIGHTNESS);
  
  //Add fade and blur
  blur1d(hist->leds, len, 80);
  fadeToBlackBy(hist->leds, len, 100);
}

void glitch(Pattern_History * hist, int len){
  //Remap the volume variable. Adjust the last parameter for different effects
  //Good range is 15-30
  //Crazy is 100-1000
  //Super crazy is 1000+
  int speedFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 5, 25);

  //Create 2 sin waves(sinBeat, sinBeat2) that mirror eachother by default when no music is played
  //Use speedFromVolume variable assigned above as the beatsin16 speed parameter
  uint16_t sinBeat0   = beatsin16(speedFromVolume, 0, len-1, 0, 0);
  uint16_t sinBeat1  = beatsin16(speedFromVolume, 0, len-1, 0, 32767);

  //Use formant analysis
  double *formants = density_formant();
  double f0Hue = remap(formants[0], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255);
  
  //Given the sinBeats above and f0Hue, color the LEDS
  hist->leds[sinBeat0]  = CHSV(fHue, 255, MAX_BRIGHTNESS);
  hist->leds[sinBeat1]  = CHSV(f0Hue, 255, MAX_BRIGHTNESS); //can use fHue instead of formants

  //add blur and fade  
  blur1d(hist->leds, len, 80);
  fadeToBlackBy(hist->leds, len, 40);
}

void glitch_talk(Pattern_History * hist, int len){
  
  //Good values for the last paramter below = [7500,12500,20000]
  int offsetFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 0, 20000);
  int speedFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 5, 20);

  //Create 3 sin beats with the speed and offset(first and last parameters) changing based off variables above
  uint16_t sinBeat0  = beatsin16(speedFromVolume, 3, len-4, 0, 250);
  uint16_t sinBeat1  = beatsin16(speedFromVolume, 3, len-4, 0, 0 - offsetFromVolume);
  uint16_t sinBeat2  = beatsin16(speedFromVolume, 3, len-4, 0, 750 + offsetFromVolume);

  //Given the sinBeats and fHue, color the LEDS  
  hist->leds[sinBeat0]  = CHSV(fHue*2, 255, MAX_BRIGHTNESS);
  hist->leds[sinBeat1]  = CHSV(fHue, 255, MAX_BRIGHTNESS);
  hist->leds[sinBeat2]  = CHSV(fHue, 255, MAX_BRIGHTNESS);

  //Add blur and fade
  blur1d(hist->leds, len, 80);
  fadeToBlackBy(hist->leds, len, 100); 
}

void glitch_sections(Pattern_History * hist, int len){

  //Last var good range (5000-10000)
  int offsetFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 0, 10000);

  //Create 4 sin beats with the offset(last parameter) changing based off offsetFromVolume
  uint16_t sinBeat0  = beatsin16(6, 0, len-1, 0, 0     - offsetFromVolume);
  uint16_t sinBeat1  = beatsin16(6, 0, len-1, 0, 16384 - offsetFromVolume);
  uint16_t sinBeat2  = beatsin16(6, 0, len-1, 0, 32767 - offsetFromVolume);
  uint16_t sinBeat3  = beatsin16(6, 0, len-1, 0, 49151 - offsetFromVolume);

  //Given the sinBeats and fHue, color the LEDS
  hist->leds[sinBeat0]  = CHSV(fHue, 255, MAX_BRIGHTNESS);
  hist->leds[sinBeat1]  = CHSV(fHue, 255, MAX_BRIGHTNESS);
  hist->leds[sinBeat2]  = CHSV(fHue, 255, MAX_BRIGHTNESS);
  hist->leds[sinBeat3]  = CHSV(fHue, 255, MAX_BRIGHTNESS);
 
  //Add blur and fade 
  blur1d(hist->leds, len, 80);
  fadeToBlackBy(hist->leds, len, 60);
}

void volume_level_middle_bar_freq_hue_with_fade_and_blur(Pattern_History * hist, int len){
    if(config.debug_mode == 1){
      Serial.print("\t pattern: volume_level_middle_bar_freq_hue\t volume: ");
      Serial.print(volume);
      Serial.print("\t peak: ");
      Serial.println(peak);
    }

    int n = remap(volume, MIN_VOLUME, MAX_VOLUME, 0, len/2);
    int mid_point = (int) len/2;
    
    for(int led = 0; led < n; led++) {
              hist->leds[mid_point + led].setHue( fHue );
              hist->leds[mid_point - led].setHue( fHue );
    }
    
    blur1d(hist->leds, len, 80); 
    fadeToBlackBy( hist->leds, len, 20);
}

void tug_of_war_frequency(Pattern_History * hist, int len) {
    double *formants = density_formant();
    double f0 = formants[0];
    delete[] formants;

    int splitPosition = remap(f0, MIN_FREQUENCY, MAX_FREQUENCY, 0, len);

    // red is on the left, blue is on the right
    for (int i = 0; i < len; i++) {
        if (i < splitPosition) {
            hist->leds[i] = CRGB::Blue; 
        } else {
            hist->leds[i] = CRGB::Red;
        }
    }
}

void tug_of_war_volume(Pattern_History * hist, int len) {
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

void random_raindrop(Pattern_History * hist, int len){
    int startIdx = random(len);

    hist->leds[startIdx] = CHSV(fHue, 255, vbrightness);
    
    for(int i = len-1; i > 0; i--) {
      if (i != startIdx) {
        hist->leds[i] = hist->leds[i-1];
      }
    }

    hist->leds[0] = CRGB::Black;
}