/** @file
  *
  * This file's functions perform basic audio processing.
  *
  * Many of the functions in ext_analysis.cpp use values calculated
  * here to perform more complex audio analysis.
  *
*/

#include <Arduino.h>
#include "arduinoFFT.h"
#include "AudioPrism.h"
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

extern int salFreqs[3];

bool is_fft_initalized = false;

/// Array to store both sampled and FFT'ed audio.
/// Processing is done in place.
extern double vReal[SAMPLES];

/// Last state of the vReal array.
extern double vRealHist[SAMPLES];

// Array of pointers for vReal and vRealHist
extern float audioPrismInput[SAMPLES];

/// Imaginary component of vReal. Unused.
extern double vImag[SAMPLES];

/// Variable used to store the frequency delta between
/// vReal and vRealHist.
extern double delt[SAMPLES];

/// Global variable used to access the frequency band
/// with the largest delta between iterations.
extern double maxDelt;

ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, SAMPLES, SAMPLING_FREQUENCY);

Spectrogram fftHistory = Spectrogram(2);           // Holds fft data for the current and last window. Used internally in the audioprism modules below
MajorPeaks peaksModule = MajorPeaks();           // MajorPeaks module set to find the single largest peak
MeanAmplitude volumeModule = MeanAmplitude();    // MeanAmplitude module to find the average volume
DeltaAmplitudes deltaModule = DeltaAmplitudes(); // DeltaAmplitudes module to find the change between vreal and vrealhist
SalientFreqs salientModule = SalientFreqs(3);    

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
  FFT.dcRemoval(vReal, SAMPLES);
  FFT.windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.complexToMagnitude(vReal, vImag, SAMPLES);

  // arduino FFT takes a double array and audioPrism modules take a float array
  for (int i = 0; i < SAMPLES; i++) {
    audioPrismInput[i] = static_cast<float>(vReal[i]); 
  }
  fftHistory.pushWindow(audioPrismInput);
}

/// @brief Updates the current peak frequency.
///
/// Places the calculated peak frequency in the "peak" variable.
void update_peak() {
  peak = FFT.majorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);
}

/// @brief Calculates and stores the current volume.
///
/// Volume is stored in the "volume" global variable.
void update_volume() {
  volumeModule.doAnalysis();
  volume = volumeModule.getOutput();
}

/// @brief Updates the largest frequency change in the last cycle.
///
/// Places the calculated value in the "maxDelt" variable.
void update_max_delta() {
  deltaModule.doAnalysis();
  float* tempDelt = deltaModule.getOutput();
  Serial.print("raw delta[0..9]: ");
  for (int i = 0; i < 10; i++) {
    Serial.print(tempDelt[i], 3);
    Serial.print(" ");
  }
  Serial.println();
  for (int i = 0; i < SAMPLES; i++) {
      delt[i] = static_cast<double>(tempDelt[i]);
  }
  // // Print first ten elements of delt
  // maxDelt = static_cast<double>(largest(delt, SAMPLES)); 
}

/// @brief Zeros all audio analysis arrays if the volume is too low.
/// @param threshold  The threshold to compare the total volume against.
void noise_gate(int threshhold) {

  if (volume < threshhold) {
    memset(vReal, 0, sizeof(double) * (SAMPLES));
    memset(vRealHist, 0, sizeof(double) * (SAMPLES));
    memset(delt, 0, sizeof(double) * (SAMPLES));
    volume = 0;
  }
}

void update_salient_freqs() {
  salientModule.doAnalysis();
  int* output = salientModule.getOutput(); 
  // Serial.print("Output values: ");
  // for (int i = 0; i < 3; ++i) {
  //     Serial.print(output[i]);
  //     Serial.print(" ");
  // }
  // Serial.println();  
  memcpy(salFreqs, output, sizeof(salFreqs)); 
}


/// @brief Congifures AudioPrism Modules
///
/// After the function completes, Audioprism Modules used elsewhere in this file
/// are properly configured with window size, sample rate, and bin size
/// MUST BE RUN BEFORE THE AUDIO ANALYSIS LOOP
void configure_core_AudioPrism_modules() {

  deltaModule.setWindowSize(SAMPLES);
  deltaModule.setSampleRate(SAMPLING_FREQUENCY);
  deltaModule.setSpectrogram(&fftHistory);

  volumeModule.setWindowSize(SAMPLES);
  volumeModule.setSampleRate(SAMPLING_FREQUENCY);
  volumeModule.setSpectrogram(&fftHistory);

  peaksModule.setWindowSize(SAMPLES);
  peaksModule.setSampleRate(SAMPLING_FREQUENCY);
  peaksModule.setSpectrogram(&fftHistory);
  peaksModule.setNumPeaks(1);

  salientModule.setWindowSize(SAMPLES);
  salientModule.setSampleRate(SAMPLING_FREQUENCY);
  salientModule.setSpectrogram(&fftHistory);
}