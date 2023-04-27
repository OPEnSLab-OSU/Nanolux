#ifndef NANOLUX_TYPES_H
#define NANOLUX_TYPES_H

// Pattern list function pointers
typedef void (*SimplePatternList[])();

// Conditions(Comment out to disable) | LAYE, HUE, DEBUG
//#define LAYER_PATTERNS
#define HUE_FLAG
#define DEBUG
//#define VIRTUAL_LED_STRIP

// Array size macro
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

// FastLED
#define NUM_LEDS    60     // Number of leds in strip.
#define DATA_PIN    15      // No hardware SPI pins defined for the ESP32 yet.
#define CLK_PIN     14      // Use bitbanged output.
#define LED_TYPE    SK9822  // Define LED protocol.
#define COLOR_ORDER BGR     // Define color color order.
#define MAX_BRIGHTNESS     255
#define FRAMES_PER_SECOND  120

// arduinoFFT
#define SAMPLES             128     // Must be a power of 2  // 128 - 1024
#define SAMPLING_FREQUENCY  10000   // Hz, must be less than 10000 due to ADC
#define ANALOG_PIN          A0
#define NOISE_GATE_THRESH   20

// Button Input
#define BUTTON_PIN 33

// MAX - MIN | Freq Volume
#define MAX_FREQUENCY       4000.0
#define MIN_FREQUENCY       50.0
#define MAX_VOLUME          3000.0
#define MIN_VOLUME          100.0

#define SER_DELTA       'C'   // use max delta frequency analysis
#define SER_NORMAL      'D'   // stop using max delta

// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 55, suggested range 20-100 
#define COOLING  100

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 72

#endif