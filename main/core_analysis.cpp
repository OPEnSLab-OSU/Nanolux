/** @file
  *
  * This file's functions perform basic audio processing.
  *
  * Many of the functions in ext_analysis.cpp use values calculated
  * here to perform more complex audio analysis.
  *
*/

#include <Arduino.h>
#include <AudioPrism>
#include "arduinoFFT.h"
#include "AnalysisModule.h"
#include "nanolux_types.h"
#include "nanolux_util.h"
#include <cmath>
#include <cstring>

/// Audio sampling period
extern unsigned int sampling_period_us;

/// Global variable used to access the current volume.
extern double volume;

/// Global variable used to store peak audio frequency
extern double peak;

bool is_fft_initalized = false;

/// Array to store both sampled and FFT'ed audio.
/// Processing is done in place.
extern double vReal[SAMPLES];

/// Last state of the vReal array.
extern double vRealHist[SAMPLES];

// Array of pointers for vReal and vRealHist
extern float* audioPrismInput[2];

/// Imaginary component of vReal. Unused.
extern double vImag[SAMPLES];

/// Variable used to store the frequency delta between
/// vReal and vRealHist.
extern double delt[SAMPLES];

/// FFT used for processing audio.
ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, SAMPLES, SAMPLING_FREQUENCY);

// MajorPeaks module set to find the single largest peak
MajorPeaks peaksModule = MajorPeaks(1); 

// MeanAmplitude module to find the average volume
MeanAmplitude volumeModule = MeanAmplitude(); 

// DeltaAmplitudes module to find the change between vreal and vrealhist
DeltaAmplitudes deltaModule = DeltaAmplitudes();

/// Global variable used to access the frequency band
/// with the largest delta between iterations.
extern double maxDelt;

/// @brief Samples incoming audio and stores the signal in vReal.
///
/// Reads from ANALOG_PIN for the calculated sampling period. Once a timestep
/// is sampled, the function sleeps until ready to sample again at the next
/// timestep.
void sample_audio() {
  unsigned long microseconds;

  for(int i=0; i<SAMPLES; i++) {
    microseconds = micros();    //Overflows after around 70 minutes!
    vReal[i] = analogRead(ANALOG_PIN);
    vImag[i] = 0;
    while(micros() < (microseconds + sampling_period_us)){}    // Busy While loop
  }
}

/// @brief Utilizes arduinoFFT to compute the fourier transform of sampled audio
///
/// After the function completes, the vReal array contains the magnitude spectrum of the FFT result
void compute_FFT() {
  FFT.windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.complexToMagnitude(vReal, vImag, SAMPLES);

  audioPrismInput[0] = (float*)vReal;       // Point to vReal
  audioPrismInput[1] = (float*)vRealHist;   // Point to vRealHist
}

/// @brief Updates the current peak frequency.
///
/// Places the calculated peak frequency in the "peak" variable.
void update_peak() {
  peaksModule.setWindowSize(SAMPLES);
  peaksModule.setSampleRate(SAMPLING_FREQUENCY);

  peaksModule.doAnalysis(audioPrismInput);
  float** peakData = peaksModule.getOutput();  // Outputs (frequency, magnatiude) tuples
  float* peakFrequencies = peakData[MP_FREQ];  

  peak = static_cast<double>(peakFrequencies[0]);
}

/// @brief Calculates and stores the current volume.
///
/// Volume is stored in the "volume" global variable.
void update_volume() {
  int top = 3, bottom = 3;

  volumeModule.setWindowSize(SAMPLES);
  volumeModule.setSampleRate(SAMPLING_FREQUENCY);
  volumeModule.setAnalysisRangeByBin(top, SAMPLES - bottom);
  
  volumeModule.doAnalysis(audioPrismInput);
  volume = volumeModule.getOutput();
}

/// @brief Updates the largest frequency change in the last cycle.
///
/// Places the calculated value in the "maxDelt" variable.
void update_max_delta() {
  int top = 3, bottom = 3;
  deltaModule

  deltaModule.setWindowSize(SAMPLES);
  deltaModule.setSampleRate(SAMPLING_FREQUENCY);
  deltaModule.setAnalysisRangeByBin(top, SAMPLES - bottom);

  deltaModule.doAnalysis(audioPrismInput);
  delt = deltaModule.getOutput();

  maxDelt = largest(delt, SAMPLES); 
}

/// @brief Zeros all audio analysis arrays if the volume is too low.
/// @param threshold  The threshold to compare the total volume against.
void noise_gate(int threshhold) {
  int top = 3, bottom = 3;

  if (volume < threshhold) {
    memset(vReal, 0, sizeof(int)*(SAMPLES-bottom-top));
    memset(vRealHist, 0, sizeof(int)*(SAMPLES-bottom-top));
    memset(delt, 0, sizeof(int)*(SAMPLES-bottom-top));
    volume = 0;
    maxDelt = 0;
  }
}

/// @brief updates vRealHist with vReal
///
/// After the function completes, the vRealHist matches the the current vReal array
void update_vRealHist() {
  memcpy(vRealHist, vReal, SAMPLES * sizeof(float));
}