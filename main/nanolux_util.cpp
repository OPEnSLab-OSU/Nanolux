#include <FastLED.h>
#include <Arduino.h>
#include "arduinoFFT.h"
#include "patterns.h"
#include "nanolux_types.h"
#include "nanolux_util.h"

extern double vReal[SAMPLES];      // Sampling buffers
extern double vImag[SAMPLES];
extern bool button_pressed;
extern bool button_held;

void IRAM_ATTR buttonISR(){
  // let debounce settle 5ms, do not exceed 15ms
  delayMicroseconds(5000);
  // if still low trigger press
  if(digitalRead(BUTTON_PIN) == LOW){
    button_pressed = true;
  }
}

void IRAM_ATTR button_down(){
  button_held = false;
}

void IRAM_ATTR button_up(){
  button_held = true;
}


void check_button_state(){
  // User Input handling
  if(button_pressed) {
    #ifdef DEBUG
      Serial.println("Pressed !");
    #endif
    #ifndef LAYER_PATTERNS
      nextPattern();                // code to execute on button press
    #endif
    button_pressed = false;         // reset pressed
  }
}

int remap( double x,double oMin,double oMax,double nMin,double nMax ){
  // range check
  if (oMin == oMax){
    return 0;
  }
  if (nMin == nMax){
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

int largest(double arr[], int n){
  double max = arr[0];

  // Traverse array elements from second and
  // compare every element with current max 
  for (int i = 1; i < n; i++)
    if (arr[i] > max){
      max = arr[i];
    }

  return max;
}

uint8_t topNibble(uint8_t b){
  return (b & 240) >> 4;
}

uint8_t bottomNibble(uint8_t b){
  return (b & 15);
}


