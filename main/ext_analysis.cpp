/** @file
  *
  * This file's functions perform advanced audio processing.
  *
  * Many of the functions defined here use values calculated
  * from core_analysis.h to perform more complex audio analysis.
  *
*/

#include <FastLED.h>
#include <Arduino.h>
#include "AudioPrism.h"
#include "patterns.h"
#include "nanolux_types.h"
#include "nanolux_util.h"
#include <cmath>

/// Global variable used to access the current volume.
extern double volume;

/// Global variable used to store preak audio frequency
extern double peak;

/// Array to store both sampled and FFT'ed audio.
/// Processing is done in place.
extern double vReal[SAMPLES];

// Array of pointers for vReal and vRealHist
extern float* audioPrismInput[2];

/// Global FIVE BAND SPLIT which stores changing bands
/// based on raw frequencies
extern double fbs[5]; 

/// @brief  for showcaseSalientFreqs
extern int salFreqs[3];

/// @brief for showcaseCentroid
extern float centroid;

/// @brief for showcasePercussion
extern bool percussionPresent;

// SalientFreqs module 
SalientFreqs salientModule = SalientFreqs(3); 

// PercussionDetection module 
PercussionDetection percussionModule = PercussionDetection(); 

// Centroid module 
Centroid centroidModule = Centroid(); 

void update_centroid() {
  centroidModule.doAnalysis((const float**)audioPrismInput);
  centroid = centroidModule.getOutput();
}

void update_percussion_dectection() {
  percussionModule.doAnalysis((const float**)audioPrismInput);
  percussionPresent = percussionModule.getOutput();
}

void update_salient_freqs() {
  salientModule.doAnalysis((const float**)audioPrismInput);
  int* output = salientModule.getOutput(); 
  memcpy(salFreqs, output, sizeof(salFreqs)); 
}


/// @brief Outputs the average volume of 5 buckets given a sample length.
/// @param len  The number of samples the buckets should stretch across.
///
/// This function totals up the volume inside all 5 buckets, averages them,
/// then maps them to the allowed volume range.
double* band_split_bounce(int len) {
    // Define the volumes to be calculated
    double vol1 = 0;
    double vol2 = 0;
    double vol3 = 0;
    double vol4 = 0;
    double vol5 = 0;
    // Sum the frequencies
    for (int i = 5; i < SAMPLES-3; i++) {
      if (0 <= i && i < len/6) {
        vol1 += vReal[i];
      }
      if (len/6 <= i && i < 2*len/6) {
        vol2 += vReal[i];
      }
      if (2*len/6 <= i && i < 3*len/6) {
        vol3 += vReal[i];
      }
      if (3*len/6 <= i && i < 4*len/6) {
        vol4 += vReal[i];
      }
      if (4*len/6 <= i && i < 5*len/6) {
        vol5 += vReal[i];
      }
    }
    
    // Average the frequencies
    vol1 /= (len/6);
    vol2 /= (len/6);
    vol3 /= (len/6);
    vol4 /= (len/6);
    vol5 /= (len/6);

    // Map to frequency based values
    vol1 = map(vol1, MIN_VOLUME, MAX_VOLUME, 0, len/6);
    vol2 = map(vol2, MIN_VOLUME, MAX_VOLUME, 0, len/6);
    vol3 = map(vol3, MIN_VOLUME, MAX_VOLUME, 0, len/6);
    vol4 = map(vol4, MIN_VOLUME, MAX_VOLUME, 0, len/6);
    vol5 = map(vol5, MIN_VOLUME, MAX_VOLUME, 0, len/6);

    // Create a resultant array
    double *fiveBands = new double[5];

    // Store the results
    fiveBands[0] = vol1;
    fiveBands[1] = vol2;
    fiveBands[2] = vol3;
    fiveBands[3] = vol4;
    fiveBands[4] = vol5;

    // Return the five-band-split
    return fiveBands;
}

/// @brief Used for moving a temporary pointer into
/// another pointer.
///
/// @param temp The temporary pointer to copy from.
/// @param arr  The global pointer to copy to.
/// @param len  The number of elements to copy.
///
/// Automatically deletes temp after copy is finished.
///
/// Intended to be used to copy temporary audio analysis
/// pointer data into global arrays.
void temp_to_array(double * temp, double * arr, int len){
  memcpy(arr, temp, sizeof(double) * len);
  delete[] temp;
}

/// @brief Moves data from the 5-band-split calculation
/// function to the global array.
void update_five_band_split(int len) {
  temp_to_array(band_split_bounce(len), fbs, 5);
}

void configure_ext_AudioPrism_modules() {

  salientModule.setWindowSize(SAMPLES);
  salientModule.setSampleRate(SAMPLING_FREQUENCY);

  percussionModule.setWindowSize(SAMPLES);
  percussionModule.setSampleRate(SAMPLING_FREQUENCY);

  centroidModule.setWindowSize(SAMPLES);
  centroidModule.setSampleRate(SAMPLING_FREQUENCY);
}

