/** @file
  *
  * This file's functions perform basic audio processing.
  *
  * Many of the functions in ext_analysis.cpp use values calculated
  * here to perform more complex audio analysis.
  *
*/

#include <FastLED.h>
#include <Arduino.h>
#include "arduinoFFT.h"
#include "patterns.h"
#include "nanolux_types.h"
#include "nanolux_util.h"
#include <cmath>

/// Audio sampling period
extern unsigned int sampling_period_us;

/// Global variable used to access the current volume.
extern double volume;

/// Global variable used to store preak audio frequency
extern double peak;

/// FFT object used for audio processing.
extern arduinoFFT FFT;

/// Array to store both sampled and FFT'ed audio.
/// Processing is done in place.
extern double vReal[SAMPLES];

/// Last state of the vReal array.
extern double vRealHist[SAMPLES];

/// Imaginary component of vReal. Unused.
extern double vImag[SAMPLES];

/// Variable used to store the frequency delta between
/// vReal and vRealHist.
extern double delt[SAMPLES];

/// Global variable used to access the frequency band
/// with the largest delta between iterations.
extern double maxDelt;

/// @brief Samples incoming audio and stores the signal in vReal.
///
/// Reads from ANALOG_PIN for the calculated sampling period. Once a timestep
/// is sampled, the function sleeps until ready to sample again at the next
/// timestep.
void sample_audio(){
  unsigned long microseconds;
  for(int i=0; i<SAMPLES; i++) {
    microseconds = micros();    //Overflows after around 70 minutes!
    vReal[i] = analogRead(ANALOG_PIN);
    vImag[i] = 0;
    while(micros() < (microseconds + sampling_period_us)){}    // Busy While loop
  }
}

/// @brief Zeros all audio analysis arrays if the volume is too low.
/// @param threshold  The threshold to compare the total volume against.
void noise_gate(int threshhold){
  int top = 3, bottom = 3;

  if (volume < threshhold) {
    memset(vReal, 0, sizeof(int)*(SAMPLES-bottom-top));
    memset(vRealHist, 0, sizeof(int)*(SAMPLES-bottom-top));
    memset(delt, 0, sizeof(int)*(SAMPLES-bottom-top));
    volume = 0;
    maxDelt = 0;
  }
}

/// @brief Calculates and stores the current volume.
///
/// Volume is stored in the "volume" global variable.
void update_volume(){
  double sum1 = 0;

  int top = 3, bottom = 3;
  for (int i = top; i < SAMPLES-bottom; i++) {      
    sum1 +=  vReal[i];
    delt[i] = abs(vReal[i] - vRealHist[i]);
    vRealHist[i] = vReal[i];
  }
  volume = sum1/(SAMPLES-top-bottom);
}

/// @brief Updates the largest frequency change in the last cycle.
///
/// Places the calculated value in the "maxDelt" variable.
void update_max_delta(){
  maxDelt = largest(delt, SAMPLES); 
}

/// @brief Updates the current peak frequency.
///
/// Places the calculated peak frequency in the "peak" variable.
void update_peak(){
  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
  peak = FFT.MajorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);
}
