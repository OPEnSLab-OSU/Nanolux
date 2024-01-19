#include <FastLED.h>
#include <Arduino.h>
#include "arduinoFFT.h"
#include "patterns.h"
#include "nanolux_types.h"
#include "nanolux_util.h"

extern double peak;
extern arduinoFFT FFT;
extern double vReal[SAMPLES];      // Sampling buffers
extern double vRealHist[SAMPLES];  // for delta freq
extern double vImag[SAMPLES];
extern double delt[SAMPLES];
extern double volume;                  
extern double maxDelt;             // Frequency with the biggest change in amp.
extern bool button_pressed;
extern unsigned int sampling_period_us;
extern unsigned long microseconds;
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

extern int virtual_led_count;

void sample_audio(){
  for(int i=0; i<SAMPLES; i++) {
    microseconds = micros();    //Overflows after around 70 minutes!
    vReal[i] = analogRead(ANALOG_PIN);
    vImag[i] = 0;
    while(micros() < (microseconds + sampling_period_us)){}    // Busy While loop
  }
}

// If the Noise Gate is below the passed in threshold (in dB), make the arrays being worked with 0
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

// Update the overall volume of the audio getting passed in
// Also updates the change in frequency from current sample to last sample
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

// Update the maximum change in frequency
void update_max_delta(){
  maxDelt = largest(delt, SAMPLES); 
}

// Get the peak frequency using the FastFFT library
void update_peak(){
  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
  peak = FFT.MajorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);
}
// NOTE: Rest of Updates at bottom of file

// Calculates the first 3 formant frequencies and outputs them as a dynamic array
// Calculates based on the density of frequencies
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

// Outputs an array of a 5-band-split based on iteration through each sample
double* band_sample_bounce() {
    // Define the volumes to be calculated
    double vol1 = 0;
    double vol2 = 0;
    double vol3 = 0;
    double vol4 = 0;
    double vol5 = 0;
    // Get the volumes of each band
    for (int i = 5; i < SAMPLES-3; i++) {
      if (0 <= i && i < SAMPLES/6) {
        vol1 += vReal[i];
      }
      if (SAMPLES/6 <= i && i < 2*SAMPLES/6) {
        vol2 += vReal[i];
      }
      if (2*SAMPLES/6 <= i && i < 3*SAMPLES/6) {
        vol3 += vReal[i];
      }
      if (3*SAMPLES/6 <= i && i < 4*SAMPLES/6) {
        vol4 += vReal[i];
      }
      if (4*SAMPLES/6 <= i && i < 5*SAMPLES/6) {
        vol5 += vReal[i];
      }
    }
    
    // Average them by the SAMPLES
    vol1 /= (SAMPLES/6);
    vol2 /= (SAMPLES/6);
    vol3 /= (SAMPLES/6);
    vol4 /= (SAMPLES/6);
    vol5 /= (SAMPLES/6);

    // Map them to values that split up the SAMPLES accurately (just for variability smoothing)
    vol1 = map(vol1, MIN_VOLUME, MAX_VOLUME, 0, SAMPLES/6);
    vol2 = map(vol2, MIN_VOLUME, MAX_VOLUME, 0, SAMPLES/6);
    vol3 = map(vol3, MIN_VOLUME, MAX_VOLUME, 0, SAMPLES/6);
    vol4 = map(vol4, MIN_VOLUME, MAX_VOLUME, 0, SAMPLES/6);
    vol5 = map(vol5, MIN_VOLUME, MAX_VOLUME, 0, SAMPLES/6);
    

    // Create an array for the result
    double *fiveSamples = new double[5];
    // Store the Five Bands volumes
    fiveSamples[0] = vol1;
    fiveSamples[1] = vol2;
    fiveSamples[2] = vol3;
    fiveSamples[3] = vol4;
    fiveSamples[4] = vol5;

    // Return the five sample array
    return fiveSamples;
}

// Outputs an array of a 5-band-split based on iteration through each frequency, contrast this with the SAMPLES over frequency
double* band_split_bounce() {
    // Define the volumes to be calculated
    double vol1 = 0;
    double vol2 = 0;
    double vol3 = 0;
    double vol4 = 0;
    double vol5 = 0;
    // Sum the frequencies
    for (int i = 5; i < SAMPLES-3; i++) {
      if (0 <= i && i < virtual_led_count/6) {
        vol1 += vReal[i];
      }
      if (virtual_led_count/6 <= i && i < 2*virtual_led_count/6) {
        vol2 += vReal[i];
      }
      if (2*virtual_led_count/6 <= i && i < 3*virtual_led_count/6) {
        vol3 += vReal[i];
      }
      if (3*virtual_led_count/6 <= i && i < 4*virtual_led_count/6) {
        vol4 += vReal[i];
      }
      if (4*virtual_led_count/6 <= i && i < 5*virtual_led_count/6) {
        vol5 += vReal[i];
      }
    }
    
    // Average the frequencies
    vol1 /= (virtual_led_count/6);
    vol2 /= (virtual_led_count/6);
    vol3 /= (virtual_led_count/6);
    vol4 /= (virtual_led_count/6);
    vol5 /= (virtual_led_count/6);

    // Map to frequency based values
    vol1 = map(vol1, MIN_VOLUME, MAX_VOLUME, 0, virtual_led_count/6);
    vol2 = map(vol2, MIN_VOLUME, MAX_VOLUME, 0, virtual_led_count/6);
    vol3 = map(vol3, MIN_VOLUME, MAX_VOLUME, 0, virtual_led_count/6);
    vol4 = map(vol4, MIN_VOLUME, MAX_VOLUME, 0, virtual_led_count/6);
    vol5 = map(vol5, MIN_VOLUME, MAX_VOLUME, 0, virtual_led_count/6);

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

// Checks if the audio is noisi or periodic
int nvp() {
  int noise = 0;
  myTime = millis(); // Get the time every millis

  // Reset the checked time difference every second and reset volume
  if (myTime-checkTime == 2000000) {
    checkTime = millis();
    checkVol = volume;
    myTime = millis();
  }

  // If the noise is sufficient or not
  if (abs(checkVol-volume) >= 100) {  // Ideally, the inequality should be switched, but it turns out to work when >= over <= so we'll go with it
    noise = 1;
  } else {
    noise = 0;
  }

  // Output the result as an int
  return noise;
}

// Identifies the drum being played amongst KICK, SNARE, & CYMBAL
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

    int kick = 0;
    int snare = 0;
    int cymbal = 0;

      if (vol1 >= vol5 && vol1 >= vol3) { // Only lights when kick
        kick = 1;
      }
      if (((vol5 > vol1 && vol5 > vol3) || (vol1 > vol5 && vol1 > vol3)) && kick != 1) { // Only lights when snare
        snare = 1;
      }
      if (vol5 > vol1 && vol5 > vol3) { // Only lights when cymbal
        cymbal = 1;
        snare = 0;
      }

    // Create, store, and output the result
    int *drumsArr = new int[3];
    drumsArr[0] = kick;
    drumsArr[1] = snare;
    drumsArr[2] = cymbal;

  return drumsArr;
}

// An update method for updating the global noise variable
void update_noise() {
  noise = nvp() == 1 ? true:false; // Ternary operator that assigns true if nvp returns 1, else false
}

// An update method that updates the global drums array also an example of how to call and use the audio functions if not using the globals
void update_drums() {
  int* temp = drum_identify();
  drums[0] = temp[0] == 1 ? true:false;
  drums[1] = temp[1] == 1 ? true:false;
  drums[2] = temp[2] == 1 ? true:false;
  delete[] temp;
}

// An update method that updates the global formants array also an example of how to call and use the audio functions if not using the globals
void update_formants() {
  double* temp = density_formant();
  formants[0] = temp[0];
  formants[1] = temp[1];
  formants[2] = temp[2];
  delete[] temp;    
}

// An update method that updates the global five band split array also an example of how to call and use the audio functions if not using the globals
void update_five_band_split() {
  double *fiveSplits = band_split_bounce();
  fbs[0] = fiveSplits[0];
  fbs[1] = fiveSplits[1];
  fbs[2] = fiveSplits[2];
  fbs[3] = fiveSplits[3];
  fbs[4] = fiveSplits[4];
  delete [] fiveSplits;
}

// An update method that updates the global five sample split array also an example of how to call and use the audio functions if not using the globals
void update_five_samples_split() {
  double *fiveSamples = band_sample_bounce();
  fss[0] = fiveSamples[0];
  fss[1] = fiveSamples[1];
  fss[2] = fiveSamples[2];
  fss[3] = fiveSamples[3];
  fss[4] = fiveSamples[4];
  delete [] fiveSamples;
} 

