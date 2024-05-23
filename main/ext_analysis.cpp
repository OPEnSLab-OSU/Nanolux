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
#include "patterns.h"
#include "nanolux_types.h"
#include "nanolux_util.h"
#include <cmath>

extern double peak;
extern double vReal[SAMPLES];      // Sampling buffers
extern double volume;                  
extern int F0arr[20]; // For smoothing formants
extern int F1arr[20]; // For smoothing formants
extern int F2arr[20]; // For smoothing formants
extern int formant_pose; // Position variable for smoothing formants
extern unsigned long myTime; // For nvp
extern unsigned long checkTime; // For nvp
extern double checkVol; // For nvp
extern double formants[3]; // Master formants array that constantly changes;
extern bool noise; // Master Noisiness versus Periodic flag that is TRUE when noisy, FALSE when periodic;
extern bool drums[3]; // Master drums array that stores whether a KICK, SNARE, or CYMBAL is happening in each element of the array;
extern double fbs[5]; // Master FIVE BAND SPLIT which stores changing bands based on raw frequencies
extern double fss[5]; // Master FIVE SAMPLE SPLIT which stores changing bands based on splitting up the samples

/// @brief Calculates the frequency bands with the highest density.
/// @returns An array of the highest density formants.
///
/// This is intended to be used for functions like vowel detection, and
/// is used in a couple patterns.
/// For any nontrivial applications, do not use this.
double* density_formant(){
  // Define the Formants to fill with values
  int F0 = 0;
  int F1 = 0;
  int F2 = 0;
  int count = 0; // Keep a count of the biggest densities
  int left = 10; // Left bound of frequency to avoid noise
  int right = 10; // Right Bound of frequency to avoid
  int len = (sizeof(vReal)/sizeof(vReal[0])) - right; // Grab the length of the desired range

  // Iterate through the frequencies array
  for (int i = left + 3; i < len; i += (int) (1.0*right)) {
    count = 0;
    // Iterate through each chunk of the frequencies array
    for (int j = i - left; j < i + right; j++) {
      if (vReal[j] > 200) { // If the amount of frequencies is loud enough, increase the count
        count += 1;
      }
    }


    if (count > 12) { // 10 (better when the bound for vReal[j] > is 700) or 12 works well
      if (F0 == 0) { // If F0 is empty...
        F0 = vReal[i]; // Store the Formant
        F0arr[formant_pose] = F0; // Add to the formants array to be smoothed later
      }
      else if (F0 != 0 && F1 == 0) { // If F1 is empty...
        F1 = vReal[i]; // Store the Formant
        F1arr[formant_pose] = F1; // Add to the formants array to be smoothed later
      }
      else { // If F2 is empty...
        F2 = vReal[i]; // Store the Formant
        F2arr[formant_pose] = F2; // Add to the formants array to be smoothed later
      }
      // If adding more catches for formants, follow the schema above
      //    Also, it is advised to tweak the hyper-parameters of checking vReal>200, smoothing array size, and count
      i += right; // Jump past the current band to avoid grabbing the same sample band
    }
  }

  // If the position of the sample being stored for smoothing is too big, reset its value
  if (formant_pose == 21)
    formant_pose = 0;


  // Increment Formant position for smoothing array
  formant_pose += 1;
  // Redefine the formants to be able to get the smoothed version
  F0 = 0; 
  F1 = 0;
  F2 = 0;

  // Iterate through the smoothing arrays and store the sums of formants
  for (int z = 0; z < 22; z++) {
    F0 += F0arr[z];
    F1 += F1arr[z];
    F2 += F2arr[z];
  }

  // Divide by the length of the smoothing array to get the average formants
  F0 /= 22;
  F1 /= 22;
  F2 /= 22;

  // Create an array to store the formants
  double* temp_formants = new double[3];

  // Store the formants
  temp_formants[0] = F0;
  temp_formants[1] = F1;
  temp_formants[2] = F2;
  
  // Output the array containing each formant
  return temp_formants;
}

/// @brief Outputs the average volume of 5 buckets given a sample length.
/// @param len  The number of samples the buckets should stretch across.
///
/// This function totals up the volume inside all 5 buckets, averages them,
/// then maps them to the allowed volume range.
double* band_split_bounce(int len) {
    // Define the volumes to be calculated
    double *vol = new double[5];

    // Sum the frequencies for each band.
    for (int i = 5; i < SAMPLES-3; i++) {
      
      // If i is out of bounds, break out of loop.
      if(i > 5*len/6) break;

      // Adds the recorded volume to the desired index.
      vol[(i * 6)/len] += vReal[i];
    }
    
    // For each frequency...
    for(int i = 0; i < 5; i++){
      // Average the summed volumes...
      vol[i] /= (len/6);
      // ???
      vol[i] = map(vol[i], MIN_VOLUME, MAX_VOLUME, 0, len/6);
    }

    // Return the five-band-split
    return vol;
}

/// @brief Returns a boolean signifying if the audio signal is
/// noisy or periodic.
bool nvp() {
  int noise = 0;
  myTime = millis(); // Get the time every millis

  // Reset the checked time difference every second and reset volume
  if (myTime-checkTime == 2000000) {
    checkTime = millis();
    checkVol = volume;
    myTime = millis();
  }

  // If the noise is sufficient or not
  return abs(checkVol-volume) >= 100;
}

/// @brief Returns an integer array containing what type of drums
/// the Nanolux device believes it is hearing.
///
/// drumsArr[0] = kick;
/// drumsArr[1] = snare;
/// drumsArr[2] = cymbal;
int* drum_identify() {

  double vol1 = 0;
  double vol3 = 0;
  double vol5 = 0;
  // Grab the portions of the frequencies where the drum pieces exist 
  for (int i = 5; i < SAMPLES-3; i++) {
    if (i < SAMPLES/6) {
      vol1 += vReal[i];
    }
    if (2*SAMPLES/6 <= i && i < 3*SAMPLES/6) {
      vol3 += vReal[i];
    }
    if (4*SAMPLES/6 <= i && i < 5*SAMPLES/6) {
      vol5 += vReal[i];
    }
  }
  
  vol1 /= (SAMPLES/6);
  vol3 /= (SAMPLES/6);
  vol5 /= (SAMPLES/6);

  // Now gets a sanitized result for volume
  vol1 = map(vol1, MIN_VOLUME, MAX_VOLUME, 0, SAMPLES/6);
  vol3 = map(vol3, MIN_VOLUME, MAX_VOLUME, 0, SAMPLES/6);
  vol5 = map(vol5, MIN_VOLUME, MAX_VOLUME, 0, SAMPLES/6);

  // Create a temporary array for output.
  int *drumsArr = new int[3];

  int kick = (vol1 >= vol5 && vol1 >= vol3);
  int cymbal = (vol5 > vol1 && vol5 > vol3);
  int snare = (((vol5 > vol1 && vol5 > vol3) || (vol1 > vol5 && vol1 > vol3)) && !kick && !cymbal);
  
  drumsArr[0] = kick;
  drumsArr[1] = snare;
  drumsArr[2] = cymbal;

  return drumsArr;
}

// An update method for updating the global noise variable
void update_noise() {
  noise = nvp(); // Ternary operator that assigns true if nvp returns 1, else false
}

/// @brief An update method that updates the global drums array.
///
/// Also an example of how to call and use the audio functions
/// if not using global variables.
void update_drums() {
  int* temp = drum_identify();
  // C++ int -> bool conversion is implicit.
  drums[0] = temp[0];
  drums[1] = temp[1];
  drums[2] = temp[2];
  delete[] temp;
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

/// @brief Moves data from the formant calculation
/// function to the global array.
void update_formants() {
  temp_to_array(density_formant(), formants, 3);
}

/// @brief Moves data from the 5-band-split calculation
/// function to the global array.
void update_five_band_split(int len) {
  temp_to_array(band_split_bounce(len), fbs, 5);
}

/// @brief Detects vowels based off of formants.
VowelSounds vowel_detection() {

  //find the max value
  double maxVal = 0.0;
  for (int i = 3; i < SAMPLES - 2; i++) {
    if (vReal[i] > maxVal) {
      maxVal = vReal[i];
    }
  }

  //normalize vReal. leave the first and last few idx of vReal out due to garbage data. noise_threshold filters out junk data
  int noise_threshold = 450;
  for (int i = 3; i < SAMPLES - 2; i++) {
    if (maxVal < noise_threshold) {
      vReal[i] = 0;
    } else vReal[i] /= maxVal;
  }

  // vReal contains 128 samples, where the data in idx 0, 1, 126, 127 seem to be garbage

  // if (maxVal > noise_threshold) Serial.println("NEW ARRAY");
  // for (int i = 3; i < SAMPLES - 2; i++) {
  //   if (vReal[i] > .4) {
  //     Serial.print("Potential peak at index ");
  //     Serial.print(i);
  //     Serial.print(" with a value of ");
  //     Serial.println(vReal[i]);
  //   }
  // }


  // primary peaks are in the first set of paranthesis. second set (if present) are the sub-peaks 
  double peak_threshold = .9;
  if((vReal[17] > peak_threshold && vReal[111] > peak_threshold)){
    Serial.println("found an 'i' like 'find'");
    return iVowel;
  } else if(((vReal[13] > peak_threshold && vReal[115] > peak_threshold))){
    Serial.println("found an ah like 'saw'");
    return aVowel;
  }else if((vReal[12] > peak_threshold && vReal[116] > peak_threshold)){
    Serial.println("found an oh like 'no'");
    return oVowel;
  }else if((vReal[6] > peak_threshold && vReal[126] > peak_threshold) && (vReal[5] > peak_threshold-.1 && vReal[123] > peak_threshold-.1)){
    Serial.println("found an ooooo like 'boot'");
    return oVowel;
  }else if((vReal[9] > peak_threshold && vReal[119] > peak_threshold)){
    Serial.println("found an 'aaaa' like 'say'");
    return aVowel;
  } else if((vReal[4] > peak_threshold && vReal[124] > peak_threshold)){
    Serial.println("found an eeee like 'bee'");
    return eVowel;
  } else if ((vReal[11] > peak_threshold && vReal[117] > peak_threshold)){
    Serial.println("found an uh like 'bus'");
    return uVowel;
  }
  return noVowel;
}


