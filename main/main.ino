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
#include "storage.h"

#define ENABLE_WEB_SERVER
#ifdef ENABLE_WEB_SERVER
#include "WebServer.h"
#endif

// #define SHOW_TIMINGS

FASTLED_USING_NAMESPACE

arduinoFFT FFT = arduinoFFT();

//
// Variables touched by the API should be declared as volatile.
//
CRGB output_buffer[MAX_LEDS];
CRGB smoothed_output[MAX_LEDS];
unsigned int sampling_period_us = round(1000000/SAMPLING_FREQUENCY);
unsigned long microseconds;
double vReal[SAMPLES];             // Sampling buffers
double vImag[SAMPLES];
double vRealHist[SAMPLES];         // Delta freq
double delt[SAMPLES];
double amplitude = 0;              // For spring mass 2
bool button_pressed = false;
bool button_held = false;
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

volatile bool pattern_changed = false;

Pattern_Data current_subpatterns[NUM_SUBPATTERNS];
Pattern_Data vol_subpatterns[NUM_SUBPATTERNS * NUM_SAVES];
Config_Data config;

uint8_t subpattern_count = 1;
uint8_t loaded_alpha = 0;
uint8_t loaded_mode = 0;

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
    void (*pattern_handler)(Pattern_History *, int);
} Pattern;

// Pattern history array and index.
// To switch patterns, simply change the index.
// Primary pattern is 0.
Pattern_History histories[NUM_SUBPATTERNS];

//
// Register all the patterns here. The index property must be sequential, but the code make sure
// that constraint is satisfied. The name is arbitrary, but keep it relatively short as it will be
// presented in a UI to the user. The enabled flag indicates whether the pattern will be shown
// in the UI or not. If not shown, it i snot selectable. If a pattern is not registered here,
// It will not be selectable and the loop below will not know about it.
//
Pattern mainPatterns[]{
    { 0, "None", true, blank},
    { 1, "Confetti", true, confetti},
    { 2, "Pixel Frequency", pix_freq},
    { 3, "Groovy Noise", true, groovy_noise},
    { 4, "Hue Trail", true, hue_trail},
    { 5, "Talking", true, talking},
    { 6, "Glitch Effect", true, glitch_effect},


    //{38, "Echo Ripple", true, echo_ripple}
};
int NUM_PATTERNS = 38; // MAKE SURE TO UPDATE THIS WITH THE ACTUAL NUMBER OF PATTERNS (+1 last array pos)


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
    pinMode(LED_BUILTIN, OUTPUT);


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
    FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(output_buffer, MAX_LEDS).setCorrection(TypicalLEDStrip);

#ifdef ENABLE_WEB_SERVER
    initialize_web_server(apiGetHooks, API_GET_HOOK_COUNT, apiPutHooks, API_PUT_HOOK_COUNT);
#endif
}

void calculate_layering(CRGB *a, CRGB *b, CRGB *out, int length, uint8_t blur){
  for(int i = 0; i < length; i++){
    out[i] = blend(a[i], b[i], blur);
  }
}

unsigned long start_millis = NULL;


void led_on_forever(){
  
  // Clear the LED strip before moving into the forever blink
  // code.
  FastLED.clear();
  FastLED.show();

  // Source: Blink example
  while(true){
    digitalWrite(LED_BUILTIN, LOW);  // turn the LED on (HIGH is the voltage level)
    delay(1000);                      // wait for a second
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED off by making the voltage LOW
    delay(1000);   
  }
}

void process_reset_button(){

  if(!digitalRead(BUTTON_PIN)){

    if(start_millis == NULL)
      start_millis = millis();

    // Only turns on the LED if the button is pressed for longer
    // than 1.5 seconds.
    (millis() - start_millis > 1500) ?
      digitalWrite(LED_BUILTIN, HIGH):
      digitalWrite(LED_BUILTIN, LOW);
      
    // if millis is ever less than start_millis,
    // just reset it to NULL.
    if(start_millis > millis()){
      start_millis = NULL;
      return;
    }  

    if(millis() - start_millis > RESET_TIME){
      clear_all();
      led_on_forever();
    }
  }else{
    digitalWrite(LED_BUILTIN, LOW);
    start_millis = NULL;
  }
}

// Some settings are updated inside a more constrained context
// without enough stack space for a "disk" operation so we
// defer the save to here.
void update_web_server(){  
    #ifdef ENABLE_WEB_SERVER
      if (dirty_settings) {
          save_settings();
          dirty_settings = false;
      }
    #endif
}

void loop() {

    begin_loop_timer(config.loop_ms);

    // Check to make sure the length of the strip is not 0.
    if(config.length < 30){
      config.length = 30;
    }

    process_reset_button();
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

    // if the length has been changed, reset all buffers
    // and histories.
    if(pattern_changed){
      pattern_changed = false;
      memset(smoothed_output, 0, sizeof(CRGB) * MAX_LEDS);
      memset(output_buffer, 0, sizeof(CRGB) * MAX_LEDS);
      histories[0] = Pattern_History();
      histories[1] = Pattern_History();
      histories[2] = Pattern_History();
      histories[3] = Pattern_History();
    }

    int section_length = config.length/subpattern_count;

    switch(loaded_mode){
      case 0:
        for(int i = 0; i < subpattern_count; i++){
          Serial.println(current_subpatterns[i].idx);
          mainPatterns[current_subpatterns[i].idx].pattern_handler(
            &histories[i],
            section_length
          );
          memcpy(&output_buffer[section_length * i], histories[i].leds, sizeof(CRGB) * section_length);
        }

      break;

      /**
      case 1:

        
        mainPatterns[current_pattern.pattern_1].pattern_handler(&histories[0], config.length);
        mainPatterns[current_pattern.pattern_2].pattern_handler(&histories[1], config.length);

        calculate_layering(
          histories[1].leds,
          histories[0].leds,
          output_buffer,
          config.length,
          current_pattern.alpha
        );
      
      break;
      **/

      default:
        FastLED.setBrightness(255);

    }

    // Set the global brightness of the LED strip.
    // FastLED.setBrightness(current_pattern.brightness);

    // Smooth the light output on the LED strip using
    // the smoothing constant.
    calculate_layering(
      smoothed_output,
      output_buffer,
      smoothed_output,
      config.length,
      255 //255 - current_pattern.smoothing
    );

    FastLED.show();

    if(config.debug_mode == 2){
      for (int i = 0; i < config.length; i++) {
        Serial.print(String(smoothed_output[i].r) + "," + String(smoothed_output[i].g) + "," + String(smoothed_output[i].b) + " ");
      }
      Serial.print("\n");
    }
  
    if(timer_overrun() == 0){
      while(timer_overrun() == 0){
        update_web_server();
      }
    }else{
      update_web_server();
    }
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

  update_five_samples_split();

  update_formants();

  update_noise();

  update_drums();

  noise_gate(0);

  

#ifdef SHOW_TIMINGS
  const int end = micros();
  Serial.printf("Audio analysis: %d ms\n", (end - start) / 1000);
#endif
}
