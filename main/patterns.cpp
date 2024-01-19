#include <FastLED.h>
#include <Arduino.h>
#include "arduinoFFT.h"
#include "patterns.h"
#include "nanolux_types.h"
#include "nanolux_util.h"
#include "palettes.h"
#include "audio_analysis.h"
#include "storage.h"

extern CRGB leds[NUM_LEDS];        // Buffer (front)
extern CRGB hist[NUM_LEDS];        // Buffer (back)
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
extern int beats;
extern bool vol_show; // A boolean to change if not wanting to see the color changing pixel in pix_freq()
extern int advanced_size;
CRGBPalette16 gPal = GMT_hot_gp; //store all palettes in array
CRGBPalette16 gPal2 = nrwc_gp; //store all palettes in array
bool gReverseDirection = false;

extern int virtual_led_count;
extern Pattern_History current_history;

extern Pattern_Data current_pattern;

void nextPattern() {
  // add one to the current pattern number, and wrap around at the end
  current_pattern.pattern_1 = (current_pattern.pattern_1 + 1) % NUM_PATTERNS;
}

void clearLEDSegment(){
  for(int i = 0; i < virtual_led_count; i++)
    leds[i] = CRGB(0,0,0);
}

void setColorHSV(CRGB* leds, byte h, byte s, byte v) {
  // create a new HSV color
  CHSV color = CHSV(h, s, v);
  // use FastLED to set the color of all LEDs in the strip to the same color
  fill_solid(leds, virtual_led_count, color);
}


void freq_hue_vol_brightness(){
  #ifdef DEBUG
    Serial.print("\t pattern: freq_hue_vol_brightness\t fHue: ");
    Serial.print(fHue);
    Serial.print("\t vbrightness: ");
    Serial.println(vbrightness);
  #endif
  CHSV color = CHSV(fHue, 255, vbrightness);
  fill_solid(leds, virtual_led_count, color);
}


void freq_confetti_vol_brightness(){
  // colored speckles based on frequency that blink in and fade smoothly
  fadeToBlackBy( leds, virtual_led_count, 20);
  int pos = random16(virtual_led_count);
  leds[pos] += CHSV( fHue + random8(10), 255, vbrightness);
  leds[pos] += CHSV( fHue + random8(10), 255, vbrightness);
}


void volume_level_middle_bar_freq_hue(){
  clearLEDSegment();

  int n = remap(volume, MIN_VOLUME, MAX_VOLUME, 0, virtual_led_count/2);
  int mid_point = (int) virtual_led_count/2;
  
  for(int led = 0; led < n; led++) {
    leds[mid_point + led].setHue( fHue );
    leds[mid_point - led].setHue( fHue );
  }
}


void freq_hue_trail(){
  leds[0] = CHSV( fHue, 255, vbrightness);
  leds[1] = CHSV( fHue, 255, vbrightness);
  CRGB temp;
  
  for(int i = virtual_led_count-1; i > 1; i-=2) {
      leds[i] = leds[i-2];
      leds[i-1] = leds[i-2];
  }
}

void blank(){
  clearLEDSegment();
}


void spring_mass_1 (){
  int middle_mass_displacement = 0;
  int middle_position = virtual_led_count / 2;
  int mass = 5;
  int mass_radius = 12;
  int spring_constant = 5;
  double period_avg = 0;
  double friction = 1;

  if (current_history.amplitude < middle_position) {
    current_history.amplitude += vbrightness / 7;
  } 

  middle_mass_displacement = current_history.amplitude*cos(sqrt(spring_constant/mass)*(current_history.frame)/3);
  current_history.frame++;
  
  if (current_history.amplitude > friction) {
    current_history.amplitude = current_history.amplitude - friction; 
  } else {
    current_history.amplitude = 0;
  }

  int left_end = middle_position + middle_mass_displacement - mass_radius;
  int right_end = middle_position + middle_mass_displacement + mass_radius;
  
  for (int i = 0; i < virtual_led_count-1; i++)
  {
    if ((i > left_end) && (i < right_end))
    {
      int springbrightness = (90/mass_radius) * abs(mass_radius - abs(i - (middle_mass_displacement+middle_position)));
      leds[i] = CHSV (fHue, 255-vbrightness, springbrightness);
    } else {
      leds[i] = CHSV (0, 0, 0);
    }
  }
}

void spring_mass_2 () {
  int middle_position = virtual_led_count / 2;
  int mass = 5;
  int mass_radius = 15;
  int friction = 100;
  double spring_constant = 0;

  for (int i = 10; i > 0; i--) {
    spring_constant += current_history.smoothing_value[i];
    current_history.smoothing_value[i] = current_history.smoothing_value[i-1];
  }
  current_history.smoothing_value[0] = fHue;
  spring_constant += fHue;
  spring_constant = spring_constant / 2550;
  
  current_history.acceleration = -1*current_history.location * spring_constant/mass;
  if (current_history.velocity > 0)
  {
    current_history.velocity += current_history.acceleration + (vbrightness/80);
  } else {
    current_history.velocity += current_history.acceleration - (vbrightness/80);
  }
  current_history.location += current_history.velocity;
  if (current_history.location < -1*virtual_led_count/2)
  {
    current_history.location = -1*virtual_led_count/2;
  } else if (current_history.location > virtual_led_count/2) {
    current_history.location = virtual_led_count/2;
  }

  int left_end = middle_position + current_history.location - mass_radius;
  int right_end = middle_position + current_history.location + mass_radius;
  
  for (int i = 0; i < virtual_led_count-1; i++)
  {
    if ((i > left_end) && (i < right_end))
    {        
      int springbrightness = 90 - (90/mass_radius * abs(i - (current_history.location+middle_position)));
      leds[i] = CHSV (spring_constant * 255, 255-vbrightness, springbrightness);
    } else {
      leds[i] = CHSV (0, 0, 0);
    }
  }
}

void spring_mass_3() {
  int middle_position = virtual_led_count / 2;
  int mass = 5;
  int mass_radiuses[5] = {6,5,4,3,2};
  int friction = 100;
  double spring_constants[5] = {0.05, 0.10, 0.15, 0.20, 0.25};
  double tempsum = 0;
  
  for (int k = 0; k < 5; k++){      
    for (int i = (3+(k*SAMPLES/5)); i < ((k+1)*SAMPLES/5)-3; i++) {
      tempsum +=  vReal[i];
    }
    current_history.vRealSums[k] = tempsum/(SAMPLES/5);
    current_history.vRealSums[k] = remap(current_history.vRealSums[k], MIN_VOLUME, MAX_VOLUME, 0, 5);
  }
  
  for (int j = 0; j < 5; j++) {
    current_history.accelerations[j] = -1*current_history.locations[j] * spring_constants[j]/mass;
    if (current_history.velocity > 0)
    {
      current_history.velocities[j] += current_history.accelerations[j] + (current_history.vRealSums[j]);
    } else {
      current_history.velocities[j] += current_history.accelerations[j] - (current_history.vRealSums[j]);
    }
    current_history.locations[j] += current_history.velocities[j];
    if (current_history.locations[j] < -1*virtual_led_count/2)
    {
      current_history.locations[j] = -1*virtual_led_count/2;
    } else if (current_history.locations[j] > virtual_led_count/2) {
      current_history.locations[j] = virtual_led_count/2;
    }

    int left_end = middle_position + current_history.locations[j] - mass_radiuses[j];
    int right_end = middle_position + current_history.locations[j] + mass_radiuses[j];
  
    for (int i = 0; i < virtual_led_count-1; i++)
    {
      if ((i > left_end) && (i < right_end))
      {        
        //int springbrightness = 90 - (90/mass_radius * abs(i - (current_history.locations[j]+middle_position)));
        leds[i] = CHSV (spring_constants[j] * 255 * 4, 255-vbrightness, 80);
      } else {
        leds[i] -= CHSV (0, 0, 10);
      }
    }
  } 
}

// Represents classical music comprehensively on the light strip ("Claire de Lune" looks great when volume is sufficient)
// NOTE: Does not make use of global formants variables, but should
void classical() {
  double* temp_formants = density_formant();
  int bpm = map(vbrightness, 0, MAX_BRIGHTNESS, 1, 30);
  uint16_t sin = beatsin16(bpm, 0, temp_formants[1]);

  fadeToBlackBy(leds, virtual_led_count, 50);

  CRGBPalette16 myPal = hue_gp;
  fill_palette(leds, virtual_led_count, sin, 255/virtual_led_count, myPal, 50, LINEARBLEND);

  #ifdef DEBUG
      Serial.print("\t Classical: ");
  #endif

  delete[] temp_formants;
}

// Based on a sufficient volume, a pixel will float to some position on the light strip and fall down (vol_show adds another threshold)
void pix_freq() {
  fadeToBlackBy(leds, virtual_led_count, 50);
  if (volume > 200) {
    current_history.pix_pos = map(peak, MIN_FREQUENCY, MAX_FREQUENCY, 0, virtual_led_count-1);
    current_history.tempHue = fHue;
  }
  else {
    current_history.pix_pos--;
    current_history.tempHue--;
    current_history.vol_pos--;
  }
  if (vol_show) {
    if (volume > 100) {
      current_history.vol_pos = map(volume, MIN_VOLUME, MAX_VOLUME, 0, virtual_led_count-1);
      current_history.tempHue = fHue;
    } else {
      current_history.vol_pos--;
    }

    leds[current_history.vol_pos] = current_history.vol_pos < virtual_led_count ? CRGB(255, 255, 255):CRGB(0, 0, 0);
  }
  leds[current_history.pix_pos] = current_history.pix_pos < virtual_led_count ? CHSV(current_history.tempHue, 255, 255):CRGB(0, 0, 0);
}

// A mirroed version of the pix_freq, splittion it on two sides starting from the middle
void mirror_pix_freq() {
  fadeToBlackBy(leds, virtual_led_count, 50);
        
  if (volume > 125) {
    current_history.pix_pos = (virtual_led_count / 2) + map(peak, MIN_FREQUENCY, MAX_FREQUENCY, -virtual_led_count/2 , virtual_led_count / 2);
    current_history.tempHue = fHue;
    
  } else {
    current_history.pix_pos--;
    current_history.vol_pos--;
  }
  if (vol_show) {
    if (volume > 75) {
      current_history.vol_pos = (virtual_led_count / 2) + map(volume, MIN_VOLUME, MAX_VOLUME, virtual_led_count/2 , virtual_led_count / 2);
      current_history.tempHue = fHue;
    } else {
      current_history.vol_pos--;
    }
    leds[virtual_led_count / 2 - 1 - current_history.vol_pos] =  current_history.vol_pos < virtual_led_count/2 ? CRGB(255, 255, 255):CRGB(0, 0, 0);
    leds[virtual_led_count / 2 + 1 + current_history.vol_pos] = current_history.vol_pos < virtual_led_count/2 ? CRGB(255, 255, 255):CRGB(0, 0, 0);
  }
  leds[virtual_led_count/2 - 1 - current_history.pix_pos] = current_history.pix_pos < virtual_led_count/2 ? CHSV(current_history.tempHue, 255, 255):CRGB(0, 0, 0);
  leds[virtual_led_count/2 + 1 + current_history.pix_pos] = current_history.pix_pos < virtual_led_count/2 ? CHSV(current_history.tempHue, 255, 255):CRGB(0, 0, 0);
}

// Utility function for sending a wave with sine for the math rock function
void send_wave() {
  double change_by = vbrightness;
  int one_sine = map(change_by, 0, MAX_BRIGHTNESS, 25, 35);
  CRGB color = CRGB(0, one_sine/2, 50);
  fill_solid(leds, virtual_led_count, color);
  uint8_t sinBeat = beatsin8(30, 0, virtual_led_count-1, 0, 0);
  leds[sinBeat] = CRGB(10, 10, 0);
  fadeToBlackBy(leds, virtual_led_count, 1);
  uint8_t sinBeat1 = beatsin8(one_sine, 0, virtual_led_count-1, 0, 170);
  leds[sinBeat1] = CRGB(255, 0, 0);
  fadeToBlackBy(leds, virtual_led_count, 1);
  uint8_t sinBeat2 = beatsin8(one_sine, 0, virtual_led_count-1, 0, 255);
  leds[sinBeat2] = CRGB(255,255,255);
  fadeToBlackBy(leds, virtual_led_count, 1);

  for (int i = 0; i < 20; i++) {
    blur1d(leds, virtual_led_count, 50);
  }
  fadeToBlackBy(leds, virtual_led_count, 50);

  #ifdef DEBUG
    Serial.print("\t sinBeat: ");
    Serial.print(sinBeat);
    Serial.print("\t sinBeat1: ");
    Serial.print(sinBeat1);
    Serial.print("\t sinBeat2: ");
    Serial.print(sinBeat2);
  #endif
}

// Represents math rock music comprehensively on the light strip ("Waterslide" by Chon looks great)
void math() {
  int len = (sizeof(vReal)/sizeof(vReal[0])) / 10;
  double smol_arr[len];
  memcpy(smol_arr, vReal + (8*len), len-1);
  double sums = 0;
  for (int i = 0; i < len; i++) {
    sums += smol_arr[i];
  }
  int vol = sums/len; // Get a volume for the snare^
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
  current_history.genre_smoothing[current_history.genre_pose] = hue;
  current_history.genre_pose++;
  if (current_history.genre_pose == 10) {
    current_history.genre_pose = 0;
  }
  // Get average hue values for the middle frequency^^^

  
  int nue = 0;
  for (int i = 0; i < 10; i++) {
    nue += current_history.genre_smoothing[i];
  }
  hue = nue/8;

  current_history.frame++;

  CHSV color = CHSV(255-hue, 255, 4*brit);

  int mid_point = (int) virtual_led_count/2;
  fill_solid(leds, virtual_led_count, color);
  send_wave(); // Sends the wave based on volume

  red = map(red, 0, 1500, 0, (virtual_led_count/2)-1);
  
  // DO the middle bar from the new hue calculated above^^^^^
  for(int led = 0; led < red; led++) {
    leds[mid_point + led] = CHSV(2*hue, 255, 3*brit);
    leds[mid_point - led] = CHSV(2*hue, 255, 3*brit);
  }
  fadeToBlackBy(leds, virtual_led_count, 50);

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
void band_brightness() {
  double *fiveSamples = band_split_bounce();
  #ifdef DEBUG
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
  #endif
  // Map each chunk of the Light strip to a brightness from the band sample split
  for (int i = 0; i < virtual_led_count/5; i++) {
    leds[i] = CHSV(0,255, map(fiveSamples[0], 0, 5, 0, 255));
  }
  for (int i = virtual_led_count/5; i < 2*virtual_led_count/5; i++) {
    leds[i] = CHSV(60,255, map(fiveSamples[1], 0, 5, 0, 255));
  }
  for (int i = 2*virtual_led_count/5; i < 3*virtual_led_count/5; i++) {
    leds[i] = CHSV(100,255, map(fiveSamples[2], 0, 5, 0, 255));
  }
  for (int i = 3*virtual_led_count/5; i < 4*virtual_led_count/5; i++) {
    leds[i] = CHSV(160,255, map(fiveSamples[3], 0, 5, 0, 255));
  }
  for (int i = 4*virtual_led_count/5; i < virtual_led_count; i++) {
    leds[i] = CHSV(205,255, map(fiveSamples[4], 0, 5, 0, 255));
  }

  fadeToBlackBy(leds, virtual_led_count, 10);

  delete [] fiveSamples;
}

// Implements a falling pixel on top of basic bands
// The falling pixel is the max volume at a point and then float down instead of just assigning volume
void advanced_bands() {
  double avg1 = 0;
  double avg2 = 0;
  double avg3 = 0;
  double avg4 = 0;
  double avg5 = 0;


  // Calculate the volume of each band
  for (int i = 0; i < advanced_size; i++) {
    avg1 += current_history.max1[i];    
  }
  avg1 /= advanced_size;
  for (int i = 0; i < advanced_size; i++) {
    avg2 += current_history.max2[i];
  }
  avg2 /= advanced_size;
  for (int i = 0; i < advanced_size; i++) {
    avg3 += current_history.max3[i];
  }
  avg3 /= advanced_size;
  for (int i = 0; i < advanced_size; i++) {
    avg4 += current_history.max4[i];
  }
  avg4 /= advanced_size;
  for (int i = 0; i < advanced_size; i++) {
    avg5 += current_history.max5[i];
  }
  avg5 /= advanced_size;

  double *fiveSamples = band_split_bounce();

  double vol1 = fiveSamples[0];
  double vol2 = fiveSamples[1];
  double vol3 = fiveSamples[2];
  double vol4 = fiveSamples[3];
  double vol5 = fiveSamples[4]; 

  // Get the Five Sample Split ^

  #ifdef DEBUG
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
  #endif

  // If there exists a new volume that is bigger than the falling pixel, reassign it to the top, otherwise make it fall for each band
  if (vol1 <= avg1) {
    current_history.max1[current_history.maxIter] = 0;
  }
  else {
      for (int i = 0; i < 5; i++) {
        current_history.max1[i] = vol1;
      }
  }
    
  if (vol2 <= avg2) {
    current_history.max2[current_history.maxIter] = 0;
  }
  else {
      for (int i = 0; i < 5; i++) {
        current_history.max2[i] = vol2;
      }
  }

  if (vol3 <= avg3) {
    current_history.max3[current_history.maxIter] = 0;
  }
  else {
      for (int i = 0; i < 5; i++) {
        current_history.max3[i] = vol3;
      }
  }

  if (vol4 <= avg4) {
    current_history.max4[current_history.maxIter] = 0;
  }
  else {
      for (int i = 0; i < 5; i++) {
        current_history.max4[i] = vol4;
      }
  }

  if (vol5 <= avg5) {
    current_history.max5[current_history.maxIter] = 0;
  }
  else {
      for (int i = 0; i < 5; i++) {
        current_history.max5[i] = vol5;
      }
  }

  // Get this smoothed array to loop to beginning again once it is at teh end of the falling pixel smoothing
  if (current_history.maxIter == advanced_size-1) {
    current_history.maxIter = 0;
  } else {
    current_history.maxIter++;
  }

  // Fill the respective chunks of the light strip with the color based on above^
  for (int i = 0; i < vol1-1; i++) {
    leds[i] = CRGB(255,0,0);
  }
  for (int i = virtual_led_count/5; i < virtual_led_count/5+vol2-1; i++) {
    leds[i] = CRGB(255,255,0);
  }
  for (int i = 2*virtual_led_count/5; i < 2*virtual_led_count/5+vol3-1; i++) {
    leds[i] = CRGB(0,255,0);
  }
  for (int i = 3*virtual_led_count/5; i < 3*virtual_led_count/5+vol4-1; i++) {
    leds[i] = CRGB(0,255,255);
  }
  for (int i = 4*virtual_led_count/5; i < 4*virtual_led_count/5+vol5-1; i++) {
    leds[i] = CRGB(0,0,255);
  }
  
  // Assign the values for the pixel to goto
  leds[(int) avg1+ (int) vol1] = CRGB(255,255,255);
  leds[(int) virtual_led_count/5 + (int) avg2+ (int) vol2] = CRGB(255,255,255);
  leds[(int) 2*virtual_led_count/5+(int) avg3+ (int) vol3] = CRGB(255,255,255);
  leds[(int) 3*virtual_led_count/5+(int) avg4+ (int) vol4] = CRGB(255,255,255);
  leds[(int) 4*virtual_led_count/5+(int) avg5+ (int) vol5] = CRGB(255,255,255);
  fadeToBlackBy(leds, virtual_led_count, 90);

  delete [] fiveSamples;
}

void basic_bands() {
  fadeToBlackBy(leds, virtual_led_count, 85);

    // double *fiveSamples = band_sample_bounce();
    double *fiveSamples = band_split_bounce(); // Maybe use above if you want, but its generally agreed this one looks better

    double vol1 = fiveSamples[0];
    double vol2 = fiveSamples[1];
    double vol3 = fiveSamples[2];
    double vol4 = fiveSamples[3];
    double vol5 = fiveSamples[4];

    // Fill each chunk of the light strip with the volume of each band
    for (int i = 0; i < vol1; i++) {
      leds[i] = CRGB(255,0,0);
    }
    for (int i = virtual_led_count/5; i < virtual_led_count/5+vol2; i++) {
      leds[i] = CRGB(255,255,0);
    }
    for (int i = 2*virtual_led_count/5; i < 2*virtual_led_count/5+vol3; i++) {
      leds[i] = CRGB(0,255,0);
    }
    for (int i = 3*virtual_led_count/5; i < 3*virtual_led_count/5+vol4; i++) {
      leds[i] = CRGB(0,255,255);
    }
    for (int i = 4*virtual_led_count/5; i < 4*virtual_led_count/5+vol5; i++) {
      leds[i] = CRGB(0,0,255);
    }

    delete [] fiveSamples;
}

// Shows the frequency array on the light strip (really messy when not gated)
void eq() {
  blank();
  for (int i = 0; i < virtual_led_count; i++) {
    int brit = map(vReal[i], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255); // The brightness is based on HOW MUCH of the frequency exists
    int hue = map(i, 0, virtual_led_count, 0, 255); // The fue is based on position on the light strip, ergo, what frequency it is at
    if (vReal[i] > 200) { // An extra gate because the frequency array is really messy without it
      leds[i] = CHSV(hue, 255, brit);
    }
  }
}

// A version of showing which drums are being played, WAY BETTER when live and using aux port
void show_drums() {
  int* drums = drum_identify();
  fadeToBlackBy(leds, virtual_led_count, 50);

  if (drums[0]) {
    fill_solid(leds, virtual_led_count, CRGB(255, 0, 0)); // Fill the whole array
  }
  if (drums[1]) {
    fill_solid(leds, 2*virtual_led_count/3, CRGB(0, 255, 0)); // Fill 2/3 of the array
  }
  if (drums[2]) {
    fill_solid(leds, virtual_led_count/3, CRGB(0, 0, 255)); // Fill 1/3 of the array
  }

  delete [] drums;
}

// This is a how one can see the formants through the whole light strip (try holding vowels at the mic and see it be consistent!)
void show_formants() {
  double *temp_formants = density_formant();
  blank();
  fill_solid(leds, virtual_led_count, CRGB(remap(temp_formants[0], 0, SAMPLES, log(1),50), remap(temp_formants[1], 0, SAMPLES, log(1),50), remap(temp_formants[2], 0, SAMPLES, log(1),50)));
  delete [] temp_formants;
}

// Lights up when noisy, not when periodic
void noisy() {
  if (nvp() == 1) {
    fill_solid(leds, virtual_led_count, CRGB(255, 0, 0)); // Fill Red
  }
  else {
    blank(); // Empty when audio is registered as periodic 
  }  
  #ifdef DEBUG
    Serial.print("\t CheckVol: ");
  #endif
}

// Splits up each formant as 1/3 of a band on the light strip. Proves that the formants actually work and looks pretty neat imo
void formant_band() {
  // Grab the formants
  double *temp_formants = density_formant();
  double f0Hue = remap(temp_formants[0], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255);
  double f1Hue = remap(temp_formants[1], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255);
  double f2Hue = remap(temp_formants[2], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255);

  #ifdef DEBUG
    Serial.print("\t f0Hue: ");
    Serial.print(temp_formants[0]);
    Serial.print("\t f1Hue: ");
    Serial.print(temp_formants[1]);
    Serial.print("\t f2Hue: ");
    Serial.print(temp_formants[2]);
  #endif

  // Fill 1/3 with each formant
  for (int i = 0; i < virtual_led_count; i++) {
    if (i < virtual_led_count/3) {
      leds[i] = CHSV(f0Hue, 255, 255);
    }
    else if (virtual_led_count/3 <= i && i < 2*virtual_led_count/3) {
      leds[i] = CHSV(f1Hue, 255, 255);
    } 
    else {
      leds[i] = CHSV(f2Hue, 255, 255);
    }
  }

  // Smooth out the result
  for (int i = 0; i < 5; i++) {
    blur1d(leds, virtual_led_count, 50);
  }
  delete[] temp_formants;
}

// An alternative, "old school" way of getting each drums and shows the overall light strip changing based on each piece.
// VERY TOUCHY so don't mess with this too much (even adding comments for some reason messed up the output)
// It does drums the same way as the official global way, but uses FastFFT
void alt_drums() {
  int len = (sizeof(vReal)/sizeof(vReal[0])) / 7;
  double smol_arr[len];
  memcpy(smol_arr, vReal, len-1);
  double F0 = FFT.MajorPeak(smol_arr, SAMPLES, SAMPLING_FREQUENCY);
  memcpy(smol_arr, vReal + (3*len), len-1);
  double F1 = FFT.MajorPeak(smol_arr, SAMPLES, SAMPLING_FREQUENCY);
  memcpy(smol_arr, vReal + (5*len), len-1);
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
  fill_solid(leds, virtual_led_count, color);
}

// A test version of showing formants and works kind of well, this is an early design, but kind of works as a quicka dn dirty demo
void formant_test(){
    int len = (sizeof(vReal)/sizeof(vReal[0])) / 5;
    double smol_arr[len];
    memcpy(smol_arr, vReal, len-1);
    int F0 = largest(smol_arr, len);
    memcpy(smol_arr, vReal + len, len-1);
    int F1 = largest(smol_arr, len);
    memcpy(smol_arr, vReal + (2*len), len-1);
    int F2 = largest(smol_arr, len);

    #ifdef DEBUG
      Serial.print("\t pattern: Formant Test\t fHue: ");
      Serial.print(fHue);
      Serial.print("\t vbrightness: ");
      Serial.println(vbrightness);
    #endif
   
    leds[0] = CHSV( fHue, F0, vbrightness);
    leds[1] = CHSV( fHue, F1, vbrightness);
    CRGB temp;
    
    for(int i = virtual_led_count-1; i > 1; i-=2) {
        leds[i] = leds[i-2];
        leds[i-1] = leds[i-2];
    } 
}

void Fire2012WithPalette(){
  
  int sparkVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 10,200);
  //int coolingVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 60, 40);
  //Serial.println(sparkVolume);
  
  
  // Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < virtual_led_count; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / virtual_led_count) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= virtual_led_count - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < sparkVolume ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    //Step 3.5. Calcualate Brightness from low frequencies
    int len = (sizeof(vReal)/sizeof(vReal[0])) / 7;
    double smol_arr[len];
    memcpy(smol_arr, vReal, len-1);
      
    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < virtual_led_count; j++) {
      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
    
      byte colorindex = scale8( heat[j], 240);
      //byte colorindex = scale8( heat[j], smol_arr);
      CRGB color = ColorFromPalette( gPal, colorindex);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (virtual_led_count-1) - j;
      } else {
        pixelnumber = j;
      }
      leds[pixelnumber] = color;
    }
}
//adapted from s-marley
void saturated_noise(){

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
  fill_noise16 (leds, virtual_led_count, octaves, x, scale, hue_octaves, hue_x, hue_scale, ntime, hue_shift);
  //Add blur
  blur1d(leds, virtual_led_count, 80);
}

void saturated_noise_hue_octaves(){

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
  fill_noise16 (leds, virtual_led_count, octaves, x, scale, hue_octaves, hue_x, hue_scale, ntime, hue_shift);
  //Add blur
  blur1d(leds, virtual_led_count, 80); 
}

void saturated_noise_hue_shift(){

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
  fill_noise16 (leds, virtual_led_count, octaves, x, scale, hue_octaves, hue_x, hue_scale, ntime, hue_shift);
  //Add blur
  blur1d(leds, virtual_led_count, 80);
}

void saturated_noise_compression(){

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
  fill_noise16 (leds, virtual_led_count, octaves, x, scale, hue_octaves, hue_x, hue_scale, ntime, hue_shift);
  //Add blur
  blur1d(leds, virtual_led_count, 80);
}

void groovy_noise(){
  
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
  fill_noise16 (leds, virtual_led_count, octaves, x, scale, hue_octaves, hue_x, hue_scale, ntime, hue_shift);
  
  //Add blur
  blur1d(leds, virtual_led_count, 80); 
}

void groovy_noise_hue_shift_change(){

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
  fill_noise16 (leds, virtual_led_count, octaves, x, scale, hue_octaves, hue_x, hue_scale, ntime, hue_shift);
  //Blur for smoother look
  blur1d(leds, virtual_led_count, 80); 
}

void sin_hue_trail(){

  //Create sin beat
  uint16_t sinBeat0  = beatsin16(12, 0, virtual_led_count-1, 0, 0);
  
  //Given the sinBeat and fHue, color the LEDS and fade
  leds[sinBeat0]  = CHSV(fHue, 255, MAX_BRIGHTNESS);
  fadeToBlackBy(leds, virtual_led_count, 5);
}

//Frequency hue trail that expands outward from the center. Adjusted from legacy freq_hue_trail
void freq_hue_trail_mid(){
    
  //color middle LEDs based on fHue
  leds[virtual_led_count/2-1] = CHSV( fHue, 255, vbrightness);
  leds[virtual_led_count/2] = CHSV( fHue, 255, vbrightness);
    
  //Move LEDS outward from the middle (only on one half)
  for(int i = virtual_led_count-1; i > virtual_led_count/2; i-=2) {
    leds[i] = leds[i-2];
    leds[i-1] = leds[i-2];
  }

  //Mirror LEDS from one half to the other half
  for(int i = 0; i < virtual_led_count/2; ++i){
    leds[virtual_led_count/2-i] = leds[virtual_led_count/2+i];
  }
}

void freq_hue_trail_mid_blur(){
    
  //color middle LEDs based on fHue
  leds[virtual_led_count/2-1] = CHSV( fHue, 255, vbrightness);
  leds[virtual_led_count/2] = CHSV( fHue, 255, vbrightness);
    
  //Move LEDS outward from the middle (only on one half)
  for(int i = virtual_led_count-1; i > virtual_led_count/2; i-=2) {
    leds[i] = leds[i-2];
    leds[i-1] = leds[i-2];
  }

  //Mirror LEDS from one half to the other half
  for(int i = 0; i < virtual_led_count/2; ++i){
    leds[virtual_led_count/2-i] = leds[virtual_led_count/2+i];
  }
  
  //Add blur
  blur1d(leds, virtual_led_count, 20); 
}

void talking_hue(){

  //remap the volume variable to move the LEDS from the middle to the outside of the strip
  int offsetFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 1, virtual_led_count/2);
  int midpoint = virtual_led_count/2;
  
  //3 LED groups. One stationary in the middle, two that move outwards based on volume
  leds[midpoint]                   = CHSV(fHue/2, 255, MAX_BRIGHTNESS);
  leds[midpoint-offsetFromVolume]  = CHSV(fHue, 255,   MAX_BRIGHTNESS);
  leds[midpoint+offsetFromVolume]  = CHSV(fHue, 255,   MAX_BRIGHTNESS);

  //Add blur and quick fade
  blur1d(leds, virtual_led_count, 80);
  fadeToBlackBy(leds, virtual_led_count, 150);
}

void talking_formants(){

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
  leds[virtual_led_count/2]                   = CRGB(f0, f1, f2);
  leds[virtual_led_count/2-offsetFromVolume]  = CHSV(f0Hue, 255, MAX_BRIGHTNESS);
  leds[virtual_led_count/2+offsetFromVolume]  = CHSV(f0Hue, 255, MAX_BRIGHTNESS);

  //blur and fade
  blur1d(leds, virtual_led_count, 80);
  fadeToBlackBy(leds, virtual_led_count, 200);
  
  //reset formant array for next loop
  delete[] formants;
}

void talking_moving(){
  
  //Last var good range (7500-12500)
  int offsetFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 0, 12500);

  //Create 3 sin beats with the offset(last parameter) changing based on offsetFromVolume
  uint16_t sinBeat0   = beatsin16(5, 2, virtual_led_count-3, 0, 250);
  uint16_t sinBeat1  = beatsin16(5, 2, virtual_led_count-3, 0, 0 - offsetFromVolume);
  uint16_t sinBeat2  = beatsin16(5, 2, virtual_led_count-3, 0, 750 + offsetFromVolume);

  //Given the sinBeats and fHue, color the LEDS
  leds[sinBeat0]  = CHSV(fHue+100, 255, MAX_BRIGHTNESS);
  leds[sinBeat1]  = CHSV(fHue, 255, MAX_BRIGHTNESS);
  leds[sinBeat2]  = CHSV(fHue, 255, MAX_BRIGHTNESS);

  //Add blur and fade
  blur1d(leds, virtual_led_count, 80);
  fadeToBlackBy(leds, virtual_led_count, 100);
}

void bounce_back(){
  
  //Last var good range (7500-12500)
  int offsetFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 0, 12500);

  //Create 2 sinBeats with the offset(last parameter) of sinBeat2 changing based on offsetFromVolume
  uint16_t sinBeat   = beatsin16(6, 2, virtual_led_count-3, 0, 500);
  uint16_t sinBeat2  = beatsin16(6, 2, virtual_led_count-3, 0, 0 - offsetFromVolume);

  //Given the sinBeats and fHue, color the LEDS
  leds[sinBeat]   = CHSV(fHue-25, 255, MAX_BRIGHTNESS);
  leds[sinBeat2]  = CHSV(fHue, 255, MAX_BRIGHTNESS);
  
  //Add fade and blur
  blur1d(leds, virtual_led_count, 80);
  fadeToBlackBy(leds, virtual_led_count, 100);
}

void glitch(){
  //Remap the volume variable. Adjust the last parameter for different effects
  //Good range is 15-30
  //Crazy is 100-1000
  //Super crazy is 1000+
  int speedFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 5, 25);

  //Create 2 sin waves(sinBeat, sinBeat2) that mirror eachother by default when no music is played
  //Use speedFromVolume variable assigned above as the beatsin16 speed parameter
  uint16_t sinBeat0   = beatsin16(speedFromVolume, 0, virtual_led_count-1, 0, 0);
  uint16_t sinBeat1  = beatsin16(speedFromVolume, 0, virtual_led_count-1, 0, 32767);

  //Use formant analysis
  double *formants = density_formant();
  double f0Hue = remap(formants[0], MIN_FREQUENCY, MAX_FREQUENCY, 0, 255);
  
  //Given the sinBeats above and f0Hue, color the LEDS
  leds[sinBeat0]  = CHSV(fHue, 255, MAX_BRIGHTNESS);
  leds[sinBeat1]  = CHSV(f0Hue, 255, MAX_BRIGHTNESS); //can use fHue instead of formants

  //add blur and fade  
  blur1d(leds, virtual_led_count, 80);
  fadeToBlackBy(leds, virtual_led_count, 40);
}

void glitch_talk(){
  
  //Good values for the last paramter below = [7500,12500,20000]
  int offsetFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 0, 20000);
  int speedFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 5, 20);

  //Create 3 sin beats with the speed and offset(first and last parameters) changing based off variables above
  uint16_t sinBeat0  = beatsin16(speedFromVolume, 3, virtual_led_count-4, 0, 250);
  uint16_t sinBeat1  = beatsin16(speedFromVolume, 3, virtual_led_count-4, 0, 0 - offsetFromVolume);
  uint16_t sinBeat2  = beatsin16(speedFromVolume, 3, virtual_led_count-4, 0, 750 + offsetFromVolume);

  //Given the sinBeats and fHue, color the LEDS  
  leds[sinBeat0]  = CHSV(fHue*2, 255, MAX_BRIGHTNESS);
  leds[sinBeat1]  = CHSV(fHue, 255, MAX_BRIGHTNESS);
  leds[sinBeat2]  = CHSV(fHue, 255, MAX_BRIGHTNESS);

  //Add blur and fade
  blur1d(leds, virtual_led_count, 80);
  fadeToBlackBy(leds, virtual_led_count, 100); 
}

void glitch_sections(){

  //Last var good range (5000-10000)
  int offsetFromVolume = remap(volume, MIN_VOLUME, MAX_VOLUME, 0, 10000);

  //Create 4 sin beats with the offset(last parameter) changing based off offsetFromVolume
  uint16_t sinBeat0  = beatsin16(6, 0, virtual_led_count-1, 0, 0     - offsetFromVolume);
  uint16_t sinBeat1  = beatsin16(6, 0, virtual_led_count-1, 0, 16384 - offsetFromVolume);
  uint16_t sinBeat2  = beatsin16(6, 0, virtual_led_count-1, 0, 32767 - offsetFromVolume);
  uint16_t sinBeat3  = beatsin16(6, 0, virtual_led_count-1, 0, 49151 - offsetFromVolume);

  //Given the sinBeats and fHue, color the LEDS
  leds[sinBeat0]  = CHSV(fHue, 255, MAX_BRIGHTNESS);
  leds[sinBeat1]  = CHSV(fHue, 255, MAX_BRIGHTNESS);
  leds[sinBeat2]  = CHSV(fHue, 255, MAX_BRIGHTNESS);
  leds[sinBeat3]  = CHSV(fHue, 255, MAX_BRIGHTNESS);
 
  //Add blur and fade 
  blur1d(leds, virtual_led_count, 80);
  fadeToBlackBy(leds, virtual_led_count, 60);
}

void volume_level_middle_bar_freq_hue_with_fade_and_blur(){
    #ifdef DEBUG   
      Serial.print("\t pattern: volume_level_middle_bar_freq_hue\t volume: ");
      Serial.print(volume);
      Serial.print("\t peak: ");
      Serial.println(peak);
    #endif

    int n = remap(volume, MIN_VOLUME, MAX_VOLUME, 0, virtual_led_count/2);
    int mid_point = (int) virtual_led_count/2;
    
    for(int led = 0; led < n; led++) {
              leds[mid_point + led].setHue( fHue );
              leds[mid_point - led].setHue( fHue );
    }
    
    blur1d(leds, virtual_led_count, 80); 
    fadeToBlackBy( leds, virtual_led_count, 20);
}

void echo_ripple(){
    //prevents dead pixels from staying
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Black;  // Set to black (off)
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
      leds[randomPixel1] = CRGB(brightness1, 0, brightness3); // Set the color 
      leds[randomPixel2] = CRGB(brightness1, brightness2, 0); 
      leds[randomPixel3] = CRGB(0, brightness2, brightness3); 

      leds[randomPixel1 + 1] = CRGB(brightness1, 0, brightness3);
      leds[randomPixel2 + 1] = CRGB(brightness1, brightness2, 0);
      leds[randomPixel3 + 1] = CRGB(0, brightness2, brightness3);

      leds[randomPixel1 - 1] = CRGB(brightness1, 0, brightness3);
      leds[randomPixel2 - 1] = CRGB(brightness1, brightness2, 0);
      leds[randomPixel3 - 1] = CRGB(0, brightness2, brightness3);

      FastLED.show();
      delay(10); // Adjust the delay for the fade-in speed
    }

    // Wait for a few seconds
    delay(400); // Adjust the delay as needed

    //fades out formant colors
    for (; brightness1 >= 0; brightness1 -= brightness_inc1, brightness2 -= brightness_inc2, brightness3 -= brightness_inc3) {
      leds[randomPixel1] = CRGB(brightness1, 0, brightness3); // Set the color 
      leds[randomPixel2] = CRGB(brightness1, brightness2, 0); 
      leds[randomPixel3] = CRGB(0, brightness2, brightness3); 

      leds[randomPixel1 + 1] = CRGB(brightness1, 0, brightness3);
      leds[randomPixel2 + 1] = CRGB(brightness1, brightness2, 0);
      leds[randomPixel3 + 1] = CRGB(0, brightness2, brightness3);

      leds[randomPixel1 - 1] = CRGB(brightness1, 0, brightness3);
      leds[randomPixel2 - 1] = CRGB(brightness1, brightness2, 0);
      leds[randomPixel3 - 1] = CRGB(0, brightness2, brightness3);

      FastLED.show();
      delay(20); // Adjust the delay for the fade-out speed
    }
  
    leds[randomPixel1] = CRGB::Black;
    leds[randomPixel2] = CRGB::Black;
    leds[randomPixel3] = CRGB::Black;

    int distance = static_cast<int>(f0)+1;
    brightness1 = 0, brightness2 = 0, brightness3 = 0;

    for (int cur_iter = 1; cur_iter < distance; cur_iter++, brightness1 += brightness_inc1, brightness2 += brightness_inc2, brightness3 += brightness_inc3) {
      // Fade out the previous pixels
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i].fadeToBlackBy(15); // Adjust the fade value as needed
      }

      // changes color after half way point of distance and allows for color blending
      int fade_out_brightness = (cur_iter >= distance/2) ? brightness_inc1 * (distance - cur_iter) : brightness_inc1;

      leds[randomPixel1 + cur_iter] = CRGB(fade_out_brightness, 0, brightness3);
      leds[randomPixel2 + cur_iter] = CRGB(fade_out_brightness, brightness2, 0);
      leds[randomPixel3 + cur_iter] = CRGB(0, brightness2, brightness3);

      leds[randomPixel1 - cur_iter] = CRGB(fade_out_brightness, 0, brightness3);
      leds[randomPixel2 - cur_iter] = CRGB(fade_out_brightness, brightness2, 0);
      leds[randomPixel3 - cur_iter] = CRGB(0, brightness2, brightness3);

      delay(30);
      FastLED.show();
    }

    // fade out all pixels before patten ends
    for (int fade_out_value = 255; fade_out_value >= 0; fade_out_value--) {
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i].fadeToBlackBy(3);  // Adjust fade value as needed
      }
      FastLED.show();
      delay(10);  // Adjusted delay for the fade-out speed
    }

  //release formants alloc memory
  delete[] formants;
}
