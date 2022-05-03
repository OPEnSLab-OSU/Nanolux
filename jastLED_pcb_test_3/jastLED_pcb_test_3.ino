#include <FastLED.h>
#include <math.h>
//#include <Arduino.h>
#include "arduinoFFT.h"
#include "BluetoothSerial.h"


/*
            Bluetooth Configuration
*/
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// serial definitions for changing parameters, sent as 8 bit chars
#define SER_LED_OFF     '2'
#define SER_NEXT_PTRN   '1'
#define SER_PREV_PTRN   '0'
#define SER_60_LEDS     'A'   // strip has 60 leds
#define SER_144_LEDS    'B'   // strip has 144 leds
#define SER_DELTA       'C'   // use max delta frequency analysis
#define SER_NORMAL      'D'   // stop using max delta
BluetoothSerial SerialBT;



FASTLED_USING_NAMESPACE

// do max brightness remap


/*
        FastLED
*/
int NUM_LEDS =      60;     // Number of leds in strip.
#define DATA_PIN    15      // No hardware SPI pins defined for the ESP32 yet.
#define CLK_PIN     14      // Use bitbanged output.
#define LED_TYPE    SK9822  // Define LED protocol.
#define COLOR_ORDER BGR     // Define color color order.
#define MAX_BRIGHTNESS         255
#define FRAMES_PER_SECOND  120

// 144 was previously 'NUM_LEDS'
CRGB leds[144];        // Buffer (front)
CRGB hist[144];        // Buffer (back)

/*
        arduinoFFT
*/
#define SAMPLES             128    // Must be a power of 2  // 128 - 1024
#define SAMPLING_FREQUENCY  10000   // Hz, must be less than 10000 due to ADC
#define ANALOG_PIN          A0

unsigned int sampling_period_us = round(1000000/SAMPLING_FREQUENCY);
unsigned long microseconds;

double vReal[SAMPLES];      // Sampling buffers
double vImag[SAMPLES];

double vRealHist[SAMPLES];  // for delta freq
double delt[SAMPLES];

arduinoFFT FFT = arduinoFFT();


/*
        Button Input
*/
#define BUTTON_PIN 33
bool button_pressed = false;


// falling edge on button pin
void IRAM_ATTR buttonISR()
{
  // let debounce settle 5ms, do not exceed 15ms
  delayMicroseconds(5000);

  // if still low trigger press
  if(digitalRead(BUTTON_PIN) == LOW)
  {
    button_pressed = true;
  }
}


// Array size macro
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

// List of patterns to cycle through defined as separate functions below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { blank, freq_hue_trail, freq_hue_vol_brightness, freq_confetti_vol_brightness, volume_level_middle_bar_freq_hue };
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0;         // rotating base color

#define MAX_FREQUENCY       4000.0
#define MIN_FREQUENCY       50.0
double peak = 0.;         //  peak frequency
uint8_t fHue = 0;         // hue value based on peak frequency

#define MAX_VOLUME          3000.0
#define MIN_VOLUME          100.0
double volume = 0.;       //  NOOOOTEEEE:  static??
uint8_t vbrightness = 0;

double maxDelt = 0.;    // Frequency with the biggest change in amp.


bool debug = true;
bool hue_flag = true;



void setup() {
  // put your setup code here, to run once:
  // start USB serial communication
    Serial.begin(115200);
    while(!Serial){ ; }
    Serial.println("Serial Ready!\n\n");

    // attach button interrupt
    pinMode(digitalPinToInterrupt(BUTTON_PIN), INPUT_PULLUP);
    attachInterrupt(BUTTON_PIN, buttonISR, FALLING);

    //  initialize up led strip
    FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    blank();


    SerialBT.begin("Audiolux-BT"); //Bluetooth device name
    Serial.println("Bluetooth ready");
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.println("Setup Done!!!!!");

    // initilization sequence

    //startup();

}


void loop() {

  check_serial(); // check for user inputs on serial links
  

    audio_analysis(); // sets peak and volume

    if (hue_flag){
      fHue = remap(log ( peak ) / log ( 2 ), log ( MIN_FREQUENCY ) / log ( 2 ), log ( MAX_FREQUENCY ) / log ( 2 ), 10, 240);
    } else {
      fHue = remap(maxDelt, MIN_FREQUENCY, MAX_FREQUENCY, 0, 240);
    }
    
    vbrightness = remap(volume, MIN_VOLUME, MAX_VOLUME, 0, MAX_BRIGHTNESS); 


    // User Input handling
    if(button_pressed) {
      Serial.println("Pressed !");  //can be removed
      nextPattern();                // code to execute on button press
      button_pressed = false;      // reset pressed
    }

    // Visualization and LED control
    gPatterns[gCurrentPatternNumber]();
    FastLED.show();
    delay(10);
}


void audio_analysis() {
    /*SAMPLING*/
    for(int i=0; i<SAMPLES; i++) {
        microseconds = micros();    //Overflows after around 70 minutes!
    
        vReal[i] = analogRead(A0);
        vImag[i] = 0;
    
        while(micros() < (microseconds + sampling_period_us)){
    
        }
    }
    
    /*FFT*/
    FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
    peak = FFT.MajorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);

    double sum1 = 0;
    if (debug)
      Serial.println("--------AUDIO ANALYSIS--------");

    int top = 3, bottom = 3;
      
    for (int i = top; i < SAMPLES-bottom; i++) {
        if (!debug) {
            Serial.print("\t i: ");
            Serial.print(i);
            Serial.print("\t vReal[i]: ");
            Serial.print(vReal[i]);
            Serial.print("\t vImag[i]: ");
            Serial.println(vImag[i]);
        }
        sum1 +=  vReal[i];
        delt[i] = abs(vReal[i] - vRealHist[i]);
        vRealHist[i] = vReal[i];
    }
    volume = sum1/(SAMPLES-top-bottom);
    maxDelt = largest(delt, SAMPLES);

    
    Serial.print("\t peak: ");
    Serial.print(peak);
    Serial.print("\t volume: ");
    Serial.print(volume);
    Serial.print("\t maxDelt: ");
    Serial.println(maxDelt);
}


void freq_hue_vol_brightness(){
    if (debug) {
        Serial.print("\t pattern: freq_hue_vol_brightness\t fHue: ");
        Serial.print(fHue);
        Serial.print("\t vbrightness: ");
        Serial.println(vbrightness);
    }
    setColorHSV(fHue, 255, vbrightness);
}



void freq_confetti_vol_brightness()
{
    if (debug) {
      Serial.print("\t pattern: freq_confetti\t fHue: ");
      Serial.print(fHue);
      Serial.print("\t vbrightness: ");
      Serial.println(vbrightness);
    }
    
    // colored speckles based on frequency that blink in and fade smoothly
    fadeToBlackBy( leds, NUM_LEDS, 20);
    int pos = random16(NUM_LEDS);
    leds[pos] += CHSV( fHue + random8(10), 255, vbrightness);
    leds[pos] += CHSV( fHue + random8(10), 255, vbrightness);
    //  NOTE: try and push color towards dominant frequency
    //  ring buffer implimentation
    //  moving average
}


void volume_level_middle_bar_freq_hue(){
//    fadeToBlackBy( leds, NUM_LEDS, 1);  //  Maybe add this for patterns that dont cover the whole strip?
    FastLED.clear();

    if (debug) {
        Serial.print("\t pattern: volume_level_middle_bar_freq_hue\t volume: ");
        Serial.print(volume);
        Serial.print("\t peak: ");
        Serial.println(peak);
    }

    int n = remap(volume, MIN_VOLUME, MAX_VOLUME, 0, NUM_LEDS/2);
    int mid_point = (int) NUM_LEDS/2;
    
    for(int led = 0; led < n; led++) {
              leds[mid_point + led].setHue( fHue );
              leds[mid_point - led].setHue( fHue );
    }
}


void freq_hue_trail(){
    if (debug) {
      Serial.print("\t pattern: freq_confetti_vol_brightness_trail\t fHue: ");
      Serial.print(fHue);
      Serial.print("\t vbrightness: ");
      Serial.println(vbrightness);
    }

    leds[0] = CHSV( fHue, 255, vbrightness);
    leds[1] = CHSV( fHue, 255, vbrightness);
    CRGB temp;
    
    for(int i = NUM_LEDS-1; i > 1; i-=2) {
        leds[i] = leds[i-2];
        leds[i-1] = leds[i-2];
    }
}


void blank(){
  FastLED.clear();
}


void startup(){
  for(int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV( 200, 255, MAX_BRIGHTNESS);
        delay(500);
        FastLED.clear();
  }
//  for(int i = NUM_LEDS-1; i >= 0; i--) {
//        leds[i] = CHSV( 255, 255, MAX_BRIGHTNESS);
//        
//        
//  }
  
}

void nextPattern() {
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void prevPattern() {
  // subtract one to the current pattern number, and avoid underflows
  if(gCurrentPatternNumber == 0)
  {
    gCurrentPatternNumber = ARRAY_SIZE( gPatterns);
  }
  
  gCurrentPatternNumber -= 1;
}

void setColorHSV(byte h, byte s, byte v) {
  // create a new HSV color
  CHSV color = CHSV(h, s, v);
  // use FastLED to set the color of all LEDs in the strip to the same color
  fill_solid(leds, NUM_LEDS, color);
}




int remap( double x,double oMin,double oMax,double nMin,double nMax ){
    // range check
    if (oMin == oMax){
        Serial.println("Warning: Zero input range");
        return 0;
    }


    if (nMin == nMax){
        Serial.println("Warning: Zero output range");
        return 0;
    }

    // check reversed input range
    double reverseInput = false;
    double oldMin = min( oMin, oMax );
    double oldMax = max( oMin, oMax );
    if (oldMin != oMin){
        reverseInput = true;
    }

    // check reversed output range
    double reverseOutput = false;
    double newMin = min( nMin, nMax );
    double newMax = max( nMin, nMax );
    if (newMin != nMin){
        reverseOutput = true;
    }

    double portion = abs(x-oldMin)*(newMax-newMin)/(oldMax-oldMin);
    if (reverseInput){
        portion = abs(oldMax-x)*(newMax-newMin)/(oldMax-oldMin);
    }

    double result = portion + newMin;
    if (reverseOutput){
        result = newMax - portion;
    }

    return (int)result;
}

int largest(double arr[], int n)
{
    double max = arr[0];
 
    // Traverse array elements from second and
    // compare every element with current max 
    for (int i = 1; i < n; i++)
        if (arr[i] > max)
            max = arr[i];
 
    return max;
}


/*
 * Checks for whether the user has supplied input via bluetooth serial,
 * or via usb serial. Updates settings accordingly
 */
void check_serial()
{
  // if available, get serial data
  char user_cmd = '\0';
  if (SerialBT.available()) 
  {
    user_cmd = SerialBT.read();
  }
  if(Serial.available())
  {
    user_cmd = Serial.read();
  }

  // if there was data,
  if(user_cmd != '\0')
  {
    // go to previous pattern
    if(user_cmd == SER_PREV_PTRN)
    {
      Serial.println("Previous Pattern");
      prevPattern();
    }

    // go to next pattern
    if(user_cmd == SER_NEXT_PTRN)
    {
      Serial.println("Next Pattern");
      nextPattern();
    }

    // the LED strip has 60 LEDs
    if(user_cmd == SER_60_LEDS)
    {
      blank();
      FastLED.show();
      NUM_LEDS = 60;
      FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    }

    // the LED strip has 144 LEDs
    if(user_cmd == SER_144_LEDS)
    {
      NUM_LEDS = 144;
      FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    }

    // use max delta analysis 
    if(user_cmd == SER_DELTA)
    {
      hue_flag = false;
    }

    // use regular dominant frequency analysis
    if(user_cmd == SER_NORMAL)
    {
      hue_flag = true;
    }

    // turn off the LEDs, by putting the pattern to 0
    if(user_cmd == SER_LED_OFF)
    {
      gCurrentPatternNumber = 0;
    }
  }
}
