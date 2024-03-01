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
unsigned int sampling_period_us = round(1000000 / SAMPLING_FREQUENCY);
unsigned long microseconds;
double vReal[SAMPLES];  // Sampling buffers
double vImag[SAMPLES];
double vRealHist[SAMPLES];  // Delta freq
double delt[SAMPLES];
double amplitude = 0;  // For spring mass 2
bool button_pressed = false;
bool button_held = false;
uint8_t gHue = 0;  // Rotating base color
double peak = 0.;  // Peak frequency
uint8_t fHue = 0;  // Hue value based on peak frequency
double volume = 0.;
uint8_t vbrightness = 0;
double maxDelt = 0.;  // Frequency with the biggest change in amp.

int beats = 0;
int frame = 0;  // For spring mass

int F0arr[20];
int F1arr[20];
int F2arr[20];
int formant_pose = 0;

uint8_t genre_smoothing[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int genre_pose = 0;

unsigned long myTime;     // For nvp
unsigned long checkTime;  // For nvp
double checkVol;          // For nvp

int pix_pos = 0;
int tempHue = 0;
int vol_pos = 0;
bool vol_show = true;

int advanced_size = 20;
double max1[20] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
double max2[20] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
double max3[20] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
double max4[20] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
double max5[20] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int maxIter = 0;


hw_timer_t *My_timer = NULL;
TaskHandle_t Task1;


double formants[3];  // Master formants array that constantly changes;
bool noise;          // Master Noisiness versus Periodic flag that is TRUE when noisy, FALSE when periodic;
bool drums[3];       // Master drums array that stores whether a KICK, SNARE, or CYMBAL is happening in each element of the array;
double fbs[5];       // Master FIVE BAND SPLIT which stores changing bands based on raw frequencies
double fss[5];       // Master FIVE SAMPLE SPLIT which stores changing bands based on splitting up the samples

double velocity = 0;                                            //for spring mass 2
double acceleration = 0;                                        //for spring mass 2
double smoothing_value[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };  //for spring mass 2
int location = 70;                                              //for spring mass 2

double velocities[5] = { 0, 0, 0, 0, 0 };     //for spring mass 3
double accelerations[5] = { 0, 0, 0, 0, 0 };  //for spring mass 3
int locations[5] = { 70, 60, 50, 40, 30 };    //for spring mass 3
double vRealSums[5] = { 0, 0, 0, 0, 0 };

volatile bool pattern_changed = false;
Pattern_Data loaded_pattern;
Pattern_Data saved_patterns[NUM_SAVES];
Config_Data config;

// Manual Control
uint8_t manual_pattern_idx = 0;
volatile bool manual_control_enabled = false;


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
  { 37, "Mirrored Pixel Frequency", true, mirror_pix_freq }
  //{38, "Echo Ripple", true, echo_ripple}
};
int NUM_PATTERNS = 38;  // MAKE SURE TO UPDATE THIS WITH THE ACTUAL NUMBER OF PATTERNS (+1 last array pos)


/**********************************************************
 *
 * We want the API to be included after the globals.
 *
 **********************************************************/
#ifdef ENABLE_WEB_SERVER
#include "api.h"
#endif

void audio_analysis();

void IRAM_ATTR onTimer() {
  audio_analysis();
}

void runTask0(void *pvParameters) {
  while (true) {
    My_timer = timerBegin(0, 80, true);
    timerAttachInterrupt(My_timer, &onTimer, true);
    timerAlarmWrite(My_timer, 1000000, true);
    timerAlarmEnable(My_timer);
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);


  Serial.begin(115200);  // start USB serial communication
  while (!Serial) { ; }

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
  FastLED.addLeds<LED_TYPE, DATA_PIN, CLK_PIN, COLOR_ORDER>(smoothed_output, MAX_LEDS).setCorrection(TypicalLEDStrip);

  load_from_nvs();
  verify_saves();
  load_slot(0);

#ifdef ENABLE_WEB_SERVER
  initialize_web_server(apiGetHooks, API_GET_HOOK_COUNT, apiPutHooks, API_PUT_HOOK_COUNT);
#endif
}

void calculate_layering(CRGB *a, CRGB *b, CRGB *out, int length, uint8_t blur) {
  for (int i = 0; i < length; i++) {
    out[i] = blend(a[i], b[i], blur);
  }
}

unsigned long start_millis = NULL;

void led_on_forever() {

  // Clear the LED strip before moving into the forever blink
  // code.
  FastLED.clear();
  FastLED.show();

  // Source: Blink example
  while (true) {
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)
    delay(1000);                      // wait for a second
    digitalWrite(LED_BUILTIN, HIGH);  // turn the LED off by making the voltage LOW
    delay(1000);
  }
}


void process_reset_button() {

  if (!digitalRead(BUTTON_PIN)) {

    if (start_millis == NULL)
      start_millis = millis();

    (millis() - start_millis > 1500)
      ? digitalWrite(LED_BUILTIN, HIGH)
      : digitalWrite(LED_BUILTIN, LOW);

    if (start_millis > millis()) {
      start_millis = NULL;
      return;
    }

    if (millis() - start_millis > RESET_TIME) {
      clear_all();
      led_on_forever();
    }
  } else {
    digitalWrite(LED_BUILTIN, LOW);
    start_millis = NULL;
  }
}

// Some settings are updated inside a more constrained context
// without enough stack space for a "disk" operation so we
// defer the save to here.
void update_web_server() {
#ifdef ENABLE_WEB_SERVER
  if (dirty_settings) {
    save_settings();
    dirty_settings = false;
  }
#endif
}


/// @brief Scales all LEDs in a CRGB array by (factor)/255
/// @param arr    The array to scale
/// @param len    The number of LEDs to scale
/// @param factor The factor to scale by. Divided by 255.
void scale_crgb_array(CRGB *arr, uint8_t len, uint8_t factor) {
  float_t scale_factor = ((float_t)factor) / 255;
  for (int i = 0; i < len; i++) {
    arr[i].r *= scale_factor;
    arr[i].g *= scale_factor;
    arr[i].b *= scale_factor;
  }
}

/// @brief  Runs the strip splitting LED strip mode
void run_strip_splitting() {

  // Defines the length of an LED strip segment
  uint8_t section_length = config.length / loaded_pattern.subpattern_count;

  // Repeat for each subpattern
  for (int i = 0; i < loaded_pattern.subpattern_count; i++) {

    // Run the pattern handler for pattern i using history i
    mainPatterns[loaded_pattern.subpattern[i].idx].pattern_handler(
      &histories[i],
      section_length);

    // Copy the processed segment to the output buffer
    memcpy(
      &output_buffer[section_length * i],
      histories[i].leds,
      sizeof(CRGB) * section_length);

    // Scale the LED strip light output based on brightness
    scale_crgb_array(
      &output_buffer[section_length * i],
      section_length,
      loaded_pattern.subpattern[i].brightness);

    // Smooth the brightness-adjusted output and put it
    // into the main output buffer.
    calculate_layering(
      &smoothed_output[section_length * i],
      &output_buffer[section_length * i],
      &smoothed_output[section_length * i],
      section_length,
      255 - loaded_pattern.subpattern[i].smoothing);
  }
}

/// @brief Runs the pattern layering output mode
void run_pattern_layering() {

  // Create two CRGB buffers for holding LED strip data
  // that has been light-adjusted.
  static CRGB temps[2][MAX_LEDS];

  // If there is one subpattern pattern, run strip splitting,
  // which will output the single subpattern.
  if (loaded_pattern.subpattern_count < 2) {
    run_strip_splitting();
    return;
  }

  // If there are more than two subpatterns, use the first two
  // for pattern layering.
  for (uint8_t i = 0; i < 2; i++) {
    // Run the pattern handler for pattern i using history i
    mainPatterns[loaded_pattern.subpattern[i].idx].pattern_handler(
      &histories[i],
      config.length);

    // Copy the processed segment to the temp buffer
    memcpy(
      temps[i],
      histories[i].leds,
      sizeof(CRGB) * config.length);

    // Scale the temp buffer light output based on brightness
    scale_crgb_array(
      temps[i],
      config.length,
      loaded_pattern.subpattern[i].brightness);
  }

  // Combine the two temp arrays.
  calculate_layering(
    temps[0],
    temps[1],
    output_buffer,
    config.length,
    loaded_pattern.alpha);

  // Smooth the combined layered output buffer.
  calculate_layering(
    smoothed_output,
    output_buffer,
    smoothed_output,
    config.length,
    255 - loaded_pattern.subpattern[0].smoothing);
}

/// @brief Prints a buffer to serial.
/// @param buf  The CRGB buffer to print.
/// @param len  The number of RGB values to print.
void print_buffer(CRGB *buf, uint8_t len) {
  for (int i = 0; i < len; i++) {
    Serial.print(String(buf[i].r) + "," + String(buf[i].g) + "," + String(buf[i].b) + " ");
  }
  Serial.print("\n");
}

void loop() {

  begin_loop_timer(config.loop_ms);  // Begin timing this loop

  // Reset buffers if pattern settings were changed since
  // last program loop.
  if (pattern_changed) {
    pattern_changed = false;
    memset(smoothed_output, 0, sizeof(CRGB) * MAX_LEDS);
    memset(output_buffer, 0, sizeof(CRGB) * MAX_LEDS);
    histories[0] = Pattern_History();
    histories[1] = Pattern_History();
    histories[2] = Pattern_History();
    histories[3] = Pattern_History();
  }

  check_button_state();  // Check for user button input

  process_reset_button();  // Manage resetting saves if button held

  audio_analysis();  // Run the audio analysis pipeline

  fHue = remap(
    log(peak) / log(2),
    log(MIN_FREQUENCY) / log(2),
    log(MAX_FREQUENCY) / log(2),
    10, 240);

  vbrightness = remap(
    volume,
    MIN_VOLUME,
    MAX_VOLUME,
    0,
    MAX_BRIGHTNESS);

  if (manual_control_enabled) {

    mainPatterns[manual_pattern_idx].pattern_handler(
      &histories[0],
      config.length);

    calculate_layering(
      smoothed_output,
      histories[0].leds,
      smoothed_output,
      config.length,
      155);

  } else {
    switch (loaded_pattern.mode) {

      case STRIP_SPLITTING:
        run_strip_splitting();
        break;

      case Z_LAYERING:
        run_pattern_layering();
        break;

      default:
        0;
    }
  }


  FastLED.show();  // Push changes from the smoothed buffer to the LED strip

  // Print the LED strip buffer if the simulator is enabled.
  if (config.debug_mode == 2)
    print_buffer(smoothed_output, config.length);

  // Update the web server while waiting for the current
  // frame to complete.
  do {
    update_web_server();
  } while (timer_overrun() == 0);

  update_web_server();
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
