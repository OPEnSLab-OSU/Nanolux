#include <FastLED.h>
#include <Arduino.h>
#include "arduinoFFT.h"
#include "patterns.h"
#include "nanolux_types.h"
#include "nanolux_util.h"
#include "audio_analysis.h"

#ifdef DEBUG
#pragma message "DEBUG ENABLED"
#endif

FASTLED_USING_NAMESPACE

arduinoFFT FFT = arduinoFFT();

CRGB leds[NUM_LEDS];               // Buffer (front)
CRGB hist[NUM_LEDS];               // Buffer (back)
unsigned int sampling_period_us = round(1000000/SAMPLING_FREQUENCY);
unsigned long microseconds;
double vReal[SAMPLES];             // Sampling buffers
double vImag[SAMPLES];
double vRealHist[SAMPLES];         // Delta freq
double delt[SAMPLES];
double amplitude = 0;              // For spring mass 2
bool button_pressed = false;
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0;                  // Rotating base color
double peak = 0.;                  // Peak frequency
uint8_t fHue = 0;                  // Hue value based on peak frequency
double volume = 0.;       
uint8_t vbrightness = 0;
double maxDelt = 0.;               // Frequency with the biggest change in amp.

int beats = 0;
int frame = 0;                     // For spring mass

int F0arr[20];
int F1arr[20];
int F2arr[20];
int formant_pose = 0;

uint8_t genre_smoothing[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int genre_pose = 0;

unsigned long myTime; // For nvp
unsigned long checkTime; // For nvp
double checkVol; // For nvp

int pix_pos = 0;
int tempHue = 0;
int vol_pos = 0;
bool vol_show = true;

int advanced_size = 20;
double max1[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
double max2[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
double max3[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
double max4[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
double max5[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int maxIter = 0;


hw_timer_t *My_timer = NULL;
TaskHandle_t Task1;


double formants[3]; // Master formants array that constantly changes;
bool noise; // Master Noisiness versus Periodic flag that is TRUE when noisy, FALSE when periodic;
bool drums[3]; // Master drums array that stores whether a KICK, SNARE, or CYMBAL is happening in each element of the array;
double fbs[5]; // Master FIVE BAND SPLIT which stores changing bands based on raw frequencies
double fss[5]; // Master FIVE SAMPLE SPLIT which stores changing bands based on splitting up the samples

double velocity = 0;  //for spring mass 2
double acceleration = 0;  //for spring mass 2
double smoothing_value[10] = {0,0,0,0,0,0,0,0,0,0}; //for spring mass 2
int location = 70; //for spring mass 2

double velocities[5] = {0,0,0,0,0};  //for spring mass 3
double accelerations[5] = {0,0,0,0,0};  //for spring mass 3
int locations[5] = {70,60,50,40,30}; //for spring mass 3
double vRealSums[5] = {0,0,0,0,0};

typedef void (*SimplePatternList[])();  // List of patterns to cycle through defined as separate functions below.
SimplePatternList gPatterns = { blank, freq_hue_trail, freq_hue_vol_brightness, freq_confetti_vol_brightness, 
                                volume_level_middle_bar_freq_hue,saturated_noise, saturated_noise_hue_octaves, saturated_noise_hue_shift, 
                                saturated_noise_compression,groovy_noise, groovy_noise_hue_shift_change, 
                                sin_hue_trail, freq_hue_trail_mid , freq_hue_trail_mid_blur,
                                talking_hue, talking_formants, talking_moving, bounce_back,
                                glitch_sections, glitch, glitch_talk, spring_mass_1, spring_mass_2, spring_mass_3,formant_test, show_formants, band_brightness, 
                                noisy, alt_drums, show_drums, pix_freq, eq, math, classical, basic_bands, advanced_bands, formant_band };

int NUM_PATTERNS = 37; // MAKE SURE TO UPDATE THIS WITH PATTERNS
SimplePatternList gPatterns_layer = {blank, spring_mass_1};

// Set a timerInterrupt function to run audio analysis
void IRAM_ATTR onTimer(){
  audio_analysis();
}

// Use a separate thread just for audio analysis
// NOTE: THERE EXIST ONLY 2 EXTERNAL CORES on the ESP32 Feather BOARD; 1 is used here for audio and the other to be used for the WebApp
void runTask0(void * pvParameters) {
  while (true) {
    // Make a timer interrupt so the audio runs consistently
    My_timer = timerBegin(0, 80, true);
    timerAttachInterrupt(My_timer, &onTimer, true);
    timerAlarmWrite(My_timer, 1000000, true);
    timerAlarmEnable(My_timer);
  }
}

void setup() {
  Serial.begin(115200);               // start USB serial communication
  while(!Serial){ ; }

  // attach button interrupt
  pinMode(digitalPinToInterrupt(BUTTON_PIN), INPUT_PULLUP);
  attachInterrupt(BUTTON_PIN, buttonISR, FALLING);

  checkTime = millis();
  checkVol = 0;

  xTaskCreatePinnedToCore(     runTask0,  // Code to Run (the task)
                               "Audio Analysis",  // Name of Task
                               10000,             // Stack Size (Effectively irrelevant for this)
                               NULL,              // Input Parameters (No input per say, just changing globals)
                               0,                 // Task Priority (the best)
                               &Task1,            // Handle of Task (a reference to call)
                               0);                // Core on which the task is running on (0 is one of two cores)

  //  initialize up led strip
  FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  blank();
}

void loop() {
    audio_analysis(); // sets peak and volume

    #ifdef HUE_FLAG
      fHue = remap(log ( peak ) / log ( 2 ), log ( MIN_FREQUENCY ) / log ( 2 ), log ( MAX_FREQUENCY ) / log ( 2 ), 10, 240);
    #else
      fHue = remap(maxDelt, MIN_FREQUENCY, MAX_FREQUENCY, 0, 240);
    #endif

    vbrightness = remap(volume, MIN_VOLUME, MAX_VOLUME, 0, MAX_BRIGHTNESS); 

    check_button_state();

    #ifdef LAYER_PATTERNS
      layer_patterns();
    #else
      gPatterns[gCurrentPatternNumber]();
    #endif

    #ifdef VIRTUAL_LED_STRIP
      for (int i = 0; i < NUM_LEDS-1; i++) {
        Serial.print(String(leds[i].r) + "," + String(leds[i].g) + "," + String(leds[i].b) + " ");
      }
      Serial.print("\n");
    #endif

    FastLED.show();
    delay(10);
}

// Use all the audio analysis to update every global audio analysis value
void audio_analysis() {
  sample_audio();

  update_peak();

  update_volume();

  update_max_delta();

  update_five_band_split();

  update_five_samples_split();

  update_formants();

  update_noise();

  update_drums();

  noise_gate(NOISE_GATE_THRESH);
}
