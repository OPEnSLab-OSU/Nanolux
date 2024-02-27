#include <FastLED.h>
#include <Arduino.h>
#include "arduinoFFT.h"
#include "patterns.h"
#include "nanolux_types.h"
#include "nanolux_util.h"
#include "storage.h"

extern Config_Data config; // Currently loaded config

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
    if(config.debug_mode == 1){
      Serial.println("Pressed !");
    }

    #ifndef LAYER_PATTERNS
      nextPattern();                // code to execute on button press
    #endif
    button_pressed = false;         // reset pressed
  }
}

void bound_byte(uint8_t * val, int lower, int upper){

  if(*val > upper){
    *val = upper;
  }else if(*val < lower){
    *val = lower;
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

// TIMING

long loop_start_time = 0;
long loop_end_time = 0;

void begin_loop_timer(long ms){
  loop_start_time = millis();
  loop_end_time = loop_start_time + ms;
}

long timer_overrun(){
  // Check for timer overflow.
  // This shouldn't happen as millis() shouldn't overflow
  // for 50 days.
  if(loop_start_time > millis()) return -1;

  // Return 0 if the current time is under the loop end time,
  // else return the difference between the current time and
  // the expected loop end time.
  return (millis() < loop_end_time) ? 0 : millis() - loop_end_time + 1;
}

