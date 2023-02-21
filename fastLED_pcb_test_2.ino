#include <FastLED.h>
#include <Arduino.h>
#include "arduinoFFT.h"

FASTLED_USING_NAMESPACE

// do max brightness remap


/*
        FastLED
*/
#define NUM_LEDS    144     // Number of leds in strip.
#define DATA_PIN    15      // No hardware SPI pins defined for the ESP32 yet.
#define CLK_PIN     14      // Use bitbanged output.
#define LED_TYPE    SK9822  // Define LED protocol.
#define COLOR_ORDER BGR     // Define color color order.
#define MAX_BRIGHTNESS         255
#define FRAMES_PER_SECOND  120
#define NOISE_GATE_THRESH  20

CRGB leds[NUM_LEDS];        // Buffer (front)
CRGB hist[NUM_LEDS];        // Buffer (back)

/*
        arduinoFFT
*/
#define SAMPLES             128    // Must be a power of 2  // 128 - 1024
#define SAMPLING_FREQUENCY  10000   // Hz, must be less than 10000 due to ADC
#define ANALOG_PIN          A2

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

SimplePatternList gPatterns = { blank, freq_hue_trail, freq_hue_vol_brightness, freq_confetti_vol_brightness, volume_level_middle_bar_freq_hue, spring_mass_1, spring_mass_2, spring_mass_3};

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
bool hue_flag = false;

// map color logirithmicly
// raise frequency ^ 2
// change mapping bounds


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

    Serial.println("Setup Done!!!!!");

    // initilization sequence

//    startup();

}


void loop() {


    audio_analysis(); // sets peak and volume

    if (hue_flag){
      fHue = remap(peak, MIN_FREQUENCY, MAX_FREQUENCY, 0, 240);
    } else {
      fHue = remap(maxDelt, MIN_FREQUENCY, MAX_FREQUENCY, 0, 240);
    }
    
    vbrightness = remap(volume, MIN_VOLUME, MAX_VOLUME, 0, MAX_BRIGHTNESS); 


    // User Input handling
    if(button_pressed) {
      if(debug)Serial.println("Pressed !");  //can be removed
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
    
        vReal[i] = analogRead(A3);
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
    
    // NOISE GATE
    if (volume < NOISE_GATE_THRESH) {
      memset(vReal, 0, sizeof(int)*(SAMPLES-bottom-top));
      memset(vRealHist, 0, sizeof(int)*(SAMPLES-bottom-top));
      memset(delt, 0, sizeof(int)*(SAMPLES-bottom-top));
      volume = 0;
      maxDelt = 0;
    }
    
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
    CRGB temp;
    
    for(int i = NUM_LEDS-1; i > 0; i--) {
        leds[i] = leds[i-1];
    }
}


void blank(){
  FastLED.clear();
}

int frame = 0; //for spring mass 1
double amplitude = 0; //for spring mass 1

void spring_mass_1 ()
{
    int middle_mass_displacement = 0;
    int middle_position = NUM_LEDS / 2;
    int mass = 5;
    int mass_radius = 12;
    int spring_constant = 5;
    double period_avg = 0;
    double friction = 1;

    if (amplitude < middle_position) {
      amplitude += vbrightness / 7;
    } 

    middle_mass_displacement = amplitude*cos(sqrt(spring_constant/mass)*(frame)/3);
    frame++;
    
    if (amplitude > friction) {
      amplitude = amplitude - friction; 
    } else {
      amplitude = 0;
    }

    int left_end = middle_position + middle_mass_displacement - mass_radius;
    int right_end = middle_position + middle_mass_displacement + mass_radius;
    
    for (int i = 0; i < NUM_LEDS-1; i++)
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

double velocity = 0;  //for spring mass 2
double acceleration = 0;  //for spring mass 2
double smoothing_value[10] = {0,0,0,0,0,0,0,0,0,0}; //for spring mass 2
int location = 70; //for spring mass 2

void spring_mass_2 () 
{
    int middle_position = NUM_LEDS / 2;
    int mass = 5;
    int mass_radius = 15;
    int friction = 100;
    double spring_constant = 0;

    for (int i = 10; i > 0; i--) {
      spring_constant += smoothing_value[i];
      smoothing_value[i] = smoothing_value[i-1];
    }
    smoothing_value[0] = fHue;
    spring_constant += fHue;
    spring_constant = spring_constant / 2550;
    
    acceleration = -1*location * spring_constant/mass;
    if (velocity > 0)
    {
      velocity += acceleration + (vbrightness/80);
    } else {
      velocity += acceleration - (vbrightness/80);
    }
    location += velocity;
    if (location < -1*NUM_LEDS/2)
    {
      location = -1*NUM_LEDS/2;
    } else if (location > NUM_LEDS/2) {
      location = NUM_LEDS/2;
    }

    int left_end = middle_position + location - mass_radius;
    int right_end = middle_position + location + mass_radius;
    
    for (int i = 0; i < NUM_LEDS-1; i++)
    {
      if ((i > left_end) && (i < right_end))
      {        
        int springbrightness = 90 - (90/mass_radius * abs(i - (location+middle_position)));
        leds[i] = CHSV (spring_constant * 255, 255-vbrightness, springbrightness);
      } else {
        leds[i] = CHSV (0, 0, 0);
      }
    }
}


double velocities[5] = {0,0,0,0,0};  //for spring mass 3
double accelerations[5] = {0,0,0,0,0};  //for spring mass 3
int locations[5] = {70,60,50,40,30}; //for spring mass 3
double vRealSums[5] = {0,0,0,0,0};

    //peak = FFT.MajorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);
    //fHue = remap(peak, MIN_FREQUENCY, MAX_FREQUENCY, 0, 240);

void spring_mass_3 () 
{
    int middle_position = NUM_LEDS / 2;
    int mass = 5;
    int mass_radiuses[5] = {6,5,4,3,2};
    int friction = 100;
    double spring_constants[5] = {0.05, 0.10, 0.15, 0.20, 0.25};
    double tempsum = 0;
    
    for (int k = 0; k < 5; k++){      
      for (int i = (3+(k*SAMPLES/5)); i < ((k+1)*SAMPLES/5)-3; i++) {
        tempsum +=  vReal[i];
      }
      vRealSums[k] = tempsum/(SAMPLES/5);
      vRealSums[k] = remap(vRealSums[k], MIN_VOLUME, MAX_VOLUME, 0, 5);
    }
    
    for (int j = 0; j < 5; j++) {
      accelerations[j] = -1*locations[j] * spring_constants[j]/mass;
      if (velocity > 0)
      {
        velocities[j] += accelerations[j] + (vRealSums[j]);
      } else {
        velocities[j] += accelerations[j] - (vRealSums[j]);
      }
      locations[j] += velocities[j];
      if (locations[j] < -1*NUM_LEDS/2)
      {
        locations[j] = -1*NUM_LEDS/2;
      } else if (locations[j] > NUM_LEDS/2) {
        locations[j] = NUM_LEDS/2;
      }

      int left_end = middle_position + locations[j] - mass_radiuses[j];
      int right_end = middle_position + locations[j] + mass_radiuses[j];
    
      for (int i = 0; i < NUM_LEDS-1; i++)
      {
        if ((i > left_end) && (i < right_end))
        {        
         //int springbrightness = 90 - (90/mass_radius * abs(i - (locations[j]+middle_position)));
          leds[i] = CHSV (spring_constants[j] * 255 * 4, 255-vbrightness, 80);
        } else {
          leds[i] -= CHSV (0, 0, 10);
        }
      }
    } 
}

double amplitude[5] = {0,0,0,0,0}; //for spring mass 1

void make_spring_steady (int sp_position, int sp_mass, int sp_size, int sp_constant, int friction)
{
    int middle_mass_displacement[5] = {0,0,0,0,0};
    int middle_position[5] = {NUM_LEDS-NUM_LEDS/4, NUM_LEDS-NUM_LEDS/6, NUM_LEDS-NUM_LEDS/8, NUM_LEDS-NUM_LEDS/10, NUM_LEDS - NUM_LEDS/12}
    int mass = 5;
    int mass_radius[5] = {1,3,5,7,9};
    int spring_constant = 5;
    double period_avg = 0;
    double friction = 1;

    if (amplitude < middle_position) {
      amplitude += vbrightness / 7;
    } 

    middle_mass_displacement = amplitude*cos(sqrt(spring_constant/mass)*(frame)/3);
    frame++;
    
    if (amplitude > friction) {
      amplitude = amplitude - friction; 
    } else {
      amplitude = 0;
    }

    int left_end = middle_position + middle_mass_displacement - mass_radius;
    int right_end = middle_position + middle_mass_displacement + mass_radius;
    
    for (int i = 0; i < NUM_LEDS-1; i++)
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
