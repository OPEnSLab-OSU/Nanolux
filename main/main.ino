// #include <ESPmDNS.h>

#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <FastLED.h>
#include <Arduino.h>
#include "arduinoFFT.h"
#include "patterns.h"
#include "nanolux_types.h"
#include "nanolux_util.h"
#include "audio_analysis.h"


#define ENABLE_WEB_SERVER
#ifdef ENABLE_WEB_SERVER
#include "WebServer.h"
#endif

#define DEBUG 1
// #define SHOW_TIMINGS

#ifdef DEBUG
#pragma message "DEBUG ENABLED"
#endif

FASTLED_USING_NAMESPACE

arduinoFFT FFT = arduinoFFT();

//
// Variables touched by the API should be declared as volatile.
//
CRGB leds[NUM_LEDS];               // Buffer (front)
CRGB hist[NUM_LEDS];               // Buffer (back)

CRGB leds_upper[NUM_LEDS];
CRGB empty_strip[NUM_LEDS]; 

// LED arrays and buffers for strip splitting.
CRGB half_leds_buf[HALF_NUM_LEDS];
int virtual_led_count = NUM_LEDS;

unsigned int sampling_period_us = round(1000000/SAMPLING_FREQUENCY);
unsigned long microseconds;
double vReal[SAMPLES];             // Sampling buffers
double vImag[SAMPLES];
double vRealHist[SAMPLES];         // Delta freq
double delt[SAMPLES];
double amplitude = 0;              // For spring mass 2
bool button_pressed = false;
volatile uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
volatile uint8_t gCurrentPatternNumber2 = 0;
uint8_t gHue = 0;                  // Rotating base color
double peak = 0.;                  // Peak frequency
uint8_t fHue = 0;                  // Hue value based on peak frequency
double volume = 0.;       
uint8_t vbrightness = 0;
double maxDelt = 0.;               // Frequency with the biggest change in amp.
volatile uint8_t gNoiseGateThreshold = NOISE_GATE_THRESH;
double alpha = 1;

int beats = 0;
int frame = 0;                     // For spring mass

int F0arr[20];
int F1arr[20];
int F2arr[20];
int formant_pose = 0;

volatile int current_mode = 0;
bool old_mode = current_mode;

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

//
// Patterns structure.
//
// Describes a pattern by name, whether it will be presented to the user in the
// web application and the function that implements the pattern.
// 
typedef struct {
    int index;
    const char* pattern_name;
    bool enabled;
    void (*pattern_handler)();
} Pattern;

// Pattern history array and index.
// To switch patterns, simply change the index.
// Primary pattern is 0.
Pattern_History histories[2];
Pattern_History current_history = histories[0];

//
// Register all the patterns here. The index property must be sequential, but the code make sure
// that constraint is satisfied. The name is arbitrary, but keep it relatively short as it will be
// presented in a UI to the user. The enabled flag indicates whether the pattern will be shown
// in the UI or not. If not shown, it i snot selectable. If a pattern is not registered here,
// It will not be selectable and the loop below will not know about it.
//
Pattern mainPatterns[]{
    { 0, "None", true, blank},
    { 1, "Hue Trail", true, freq_hue_trail},
    { 2, "Volume Brightness", true, freq_hue_vol_brightness},
    { 3, "Confetti", true, freq_confetti_vol_brightness},
    { 4, "Volume Level Bar", true, volume_level_middle_bar_freq_hue},
    { 5, "Saturated Noise", true, saturated_noise},
    { 6, "Saturated Noise Octaves", true, saturated_noise_hue_octaves},
    { 7, "Saturated Noise Shift", true, saturated_noise_hue_shift},
    { 8, "Saturated Noise Compression", true, saturated_noise_compression},
    { 9, "Groovy Noise", true, groovy_noise},
    {10, "Groovy Noise Shift", true, groovy_noise_hue_shift_change},
    {11, "Sine Wave Trail", true, sin_hue_trail},
    {12, "Hue Trail Mid", true, freq_hue_trail_mid},
    {13, "Hue Trail Mid Blur", true, freq_hue_trail_mid_blur},
    {14, "Talking Hue", true, talking_hue},
    {15, "Talking Formants", true, talking_formants},
    {16, "Talking Moving", true, talking_moving},
    {17, "Bounce Back", true, bounce_back},
    {18, "Glitch Sections", true, glitch_sections},
    {19, "Glitch", true, glitch},
    {20, "Glitch Talk", true, glitch_talk},
    {21, "Spring Mass 1", true, spring_mass_1},
    {22, "Spring Mass 2", true, spring_mass_2},
    {23, "Spring Mass 3", true, spring_mass_3},
    {24, "Formant Test", true, formant_test},
    {25, "Show Formants", true, show_formants},
    {26, "Band Brightness", true, band_brightness},
    {27, "Noisy", true, noisy},
    {28, "Alt Drums", true, alt_drums},
    {29, "Show Drums", true, show_drums},
    {30, "Pixel Frequency", true, pix_freq},
    {31, "Equalizer", true, eq},
    {32, "Math", true, math},
    {33, "Classical", true, classical},
    {34, "Basic Bands", true, basic_bands},
    {35, "Advanced Bands", true, advanced_bands},
    {36, "Formant Band", true, formant_band}
};
int NUM_PATTERNS = 37; // MAKE SURE TO UPDATE THIS WITH THE ACTUAL NUMBER OF PATTERNS

SimplePatternList gPatterns_layer = {blank, spring_mass_1};


/**********************************************************
 *
 * We want the API to be included after the globals.
 *
 **********************************************************/
#ifdef ENABLE_WEB_SERVER
#include "api.h"
#endif

void audio_analysis();

void IRAM_ATTR onTimer(){
  audio_analysis();
}

void runTask0(void * pvParameters) {
  while (true) {
    My_timer = timerBegin(0, 80, true);
    timerAttachInterrupt(My_timer, &onTimer, true);
    timerAlarmWrite(My_timer, 1000000, true);
    timerAlarmEnable(My_timer);
  }
}

void setup() {
    Serial.begin(115200);               // start USB serial communication
    while(!Serial){ ; }

    // Reindex mainPatterns, to make sure it is consistent.
    for (int i = 0; i < NUM_PATTERNS; i++) {
        mainPatterns[i].index = i;
    }

    // attach button interrupt
    pinMode(digitalPinToInterrupt(BUTTON_PIN), INPUT_PULLUP);
    attachInterrupt(BUTTON_PIN, buttonISR, FALLING);

    checkTime = millis();
    checkVol = 0;

    //  initialize up led strip
    FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    blank();

#ifdef ENABLE_WEB_SERVER
    initialize_web_server(apiGetHooks, API_GET_HOOK_COUNT, apiPutHooks, API_PUT_HOOK_COUNT);
#endif
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

    #ifdef SHOW_TIMINGS 
      const int start = micros();
    #endif

    #ifdef LAYER_PATTERNS
      layer_patterns();
    #else

       #ifdef DEBUG
            Serial.println(current_mode);
        #endif

      // If the last run was not using strip splitting, clear all buffers
      // and histories.
      if(current_mode != old_mode){
        old_mode = current_mode;
        memset(leds, 0, sizeof(CRGB) * NUM_LEDS);
        memset(hist, 0, sizeof(CRGB) * NUM_LEDS);
        memset(leds_upper, 0, sizeof(CRGB) * NUM_LEDS);
        histories[0] = Pattern_History();
        histories[1] = Pattern_History();
      }

      if(current_mode == 1){
        // Set the virtual LED strip length
        virtual_led_count = HALF_NUM_LEDS;

        // Swap out the current loaded history for the loaded history of
        // pattern 2.
        current_history = histories[1];

        // Load pattern 2's previous LED strip data.
        memcpy(&leds[0], &leds[virtual_led_count], sizeof(CRGB) * virtual_led_count);

        // Run the pattern handler for pattern #2, then store the resulting LED
        // strip data in another buffer to allow for pattern 1 to run.
        mainPatterns[gCurrentPatternNumber2].pattern_handler();
        memcpy(&half_leds_buf[0], &leds[0], sizeof(CRGB) * virtual_led_count);

        // Overwrite the old LED strip data with pattern #1's previous image 
        // and load in pattern #1's history. The previous image was overwritten by
        // pattern 2, so it must be recovered from hist[].
        memcpy(&leds[0], &hist[0], sizeof(CRGB) * virtual_led_count);
        current_history = histories[0];

        // Run pattern 1's pattern handler.
        mainPatterns[gCurrentPatternNumber].pattern_handler();

        // Copy pattern 2's new LED data into the back half of the LED array.
        memcpy(&leds[virtual_led_count], &half_leds_buf[0], sizeof(CRGB) * virtual_led_count);

        memcpy(hist, leds, sizeof(CRGB) * NUM_LEDS);
      }else if(current_mode == 2){

        virtual_led_count = NUM_LEDS;

        // Move the upper pattern into the pattern buffer and history buffer.
        memcpy(&leds[0], &leds_upper[0], sizeof(CRGB) * virtual_led_count);
        current_history = histories[1];

        // Run the upper pattern's handler.
        mainPatterns[gCurrentPatternNumber2].pattern_handler();

        // Copy the new pattern 2 back into it's buffer and replace it with
        // pattern 1.
        memcpy(&leds_upper[0], &leds[0], sizeof(CRGB) * virtual_led_count);
        memcpy(&leds[0], &hist[0], sizeof(CRGB) * virtual_led_count);
        current_history = histories[0];

        // Run pattern 1's handler and store the output in hist.
        mainPatterns[gCurrentPatternNumber].pattern_handler();
        memcpy(hist, leds, sizeof(CRGB) * NUM_LEDS);

        // TRANSPARENCY EFFECT
        // For all pixels, the formula (upper * (alpha)) + (lower * (1 - alpha))
        // if there is data on the upper LED
        for(int i = 0; i < NUM_LEDS; i++){
          if(leds_upper[i].red > 0 || leds_upper[i].blue > 0 || leds_upper[i].blue > 0){
            leds[i] = leds[i]*(1-alpha) + leds_upper[i]*alpha;
          }
          
        }

      }else{
        virtual_led_count = NUM_LEDS;
        current_history = histories[0];
        mainPatterns[gCurrentPatternNumber].pattern_handler();
        memcpy(hist, leds, sizeof(CRGB) * NUM_LEDS);
      }

      
    #endif

    #ifdef SHOW_TIMINGS
      const int end = micros();
      Serial.printf("%s Visualization: %d us\n", mainPatterns[gCurrentPatternNumber].pattern_name, end - start);
    #endif

    #ifdef VIRTUAL_LED_STRIP
      for (int i = 0; i < NUM_LEDS-1; i++) {
        Serial.print(String(leds[i].r) + "," + String(leds[i].g) + "," + String(leds[i].b) + " ");
      }
      Serial.print("\n");
    #endif

    FastLED.show();
    // Update the buffer.
    
    delay(10);

#ifdef ENABLE_WEB_SERVER
    // Some settings are updated inside a more constrained context
    // without enough stack space for a "disk" operation so we
    // defer the save to here.
    if (dirty_settings) {
        save_settings();
        dirty_settings = false;
    }
#endif
}

// Use all the audio analysis to update every global audio analysis value
void audio_analysis() {
#ifdef SHOW_TIMINGS 
  const int start = micros();
#endif

  sample_audio();

  update_peak();

  update_volume();

  update_max_delta();

  update_five_band_split();

  update_five_samples_split();

  update_formants();

  update_noise();

  update_drums();

  noise_gate(gNoiseGateThreshold);

#ifdef SHOW_TIMINGS
  const int end = micros();
  Serial.printf("Audio analysis: %d ms\n", (end - start) / 1000);
#endif
}
