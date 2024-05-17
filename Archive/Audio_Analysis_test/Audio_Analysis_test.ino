/*/
  This program contains examples on how to implement algorithms for calculating the spectral centroid, average amplitude, and change in dominant frequency of a signal. It
  will constantly calculate one of these three signal properties (depending on the operating mode) and send the resulting data over serial to a connected host computer.
  This allows the serial plotter to be used for visualizing each property. When the program initializes, it defaults to operating mode 0, which just prints samples 
  directly to the serial plotter. To select a different operating mode, use the serial console to send one of the following characters to the microcontroller:

    's' - (opmode 0); Plots samples
    'a' - (opmode 1); Calculates and plots average amplitude of the signal (Volume)
    'd' - (opmode 2); Calculates and plots the change in dominant frequency of the signal
    'c' - (opmode 3); Calculates and plots the frequency and power of the spectral centroid of the signal.
    't' - (opmode 4); Calculates and plots the delta change of amplitude
    
  The functions used for calculating each signal property are calculateCentroid, calculateAvgAmplitude, calculateFrequencyDelta, and calculateAmpliDelta. Descriptions on how each of these
  functions works are provided as inline comments within each.

/*/

#include <arduinoFFT.h>
#include <Arduino.h>

// Audio recording and FFT settings
#define FFT_WIN_SIZE 128 // Number of samples per FFT window. Must be a power of 2. The best value for this is the lowest power of 2 that is equal to or greater than
                              // the result of dividing the sampling frequency by the minimum difference in frequencies you want to be able to distinguish.1024
#define SAMPLE_FREQ 10000 // Should be a little more than twice the maximum frequency that you want to be able to sense. I recommend making this a power of two as well
                            // (although this definitely isn't standard) to guarantee that it will be evenly divisible by FFT_WIN_SIZE.16384
double AUDIO_INPUT_PIN = A0; // Pin number for recording audio

#define AMP_AVG_TIME 0.25 // Amount of time to average the signal amplitude across to determine its average amplitude.
#define GAUSSIAN_SMOOTH 1 // Whether a Gaussian smoothing window or a rectangular smoothing window is used in calculating average signal amplitude, where 1 = Gaussian.
#define GAUSSIAN_WIN_TIME_CONST 0.25 // Time constant used for making the Gaussian window for amplitude averaging. More information on this constant is available in the
                                        // generateAvgWindow function, where it is used.

#define FREQ_AVG_WIN 6 // Number of FFT windows worth of frequency date to calculate, record, and average when finding the change in dominant frequency of the signal.
#define FREQ_SIG_THRESH 10 // Minimum value required for that the major peak of a signal must meet in order for it to be considered the new dominant frequency of the signal.
#define FREQ_DELTA_ERROR_MARGIN 32 // Minimum difference between dominant frequencies to be considered a significant change.


/*/
########################################################
    Stuff relevant to all of the signal processing functions (calculateCentroid, calculateFrequencyDelta, and calculateAvgAmplitude)
########################################################
/*/
// Number of microseconds to wait between recording samples.
const int sampleDelayTime = round(1000000 / SAMPLE_FREQ);

// Number of samples that should be averaged to calculate the average amplitude of the signal.
const int ampAvgSampCount = round(AMP_AVG_TIME * SAMPLE_FREQ);

// Number of samples required to calculate the all of the FFT windows that are averaged when finding changes in the dominant frequency of the signal.
const int freqAvgSampCount = FREQ_AVG_WIN * FFT_WIN_SIZE; 

// Size of the sample buffer array. This expression is equivalent to max(ampAvgSampCount, freqAvgSampCount), but I couldn't use max() because of a weird error.
const int sampleBuffSize = ampAvgSampCount * (ampAvgSampCount > freqAvgSampCount) + freqAvgSampCount * (ampAvgSampCount <= freqAvgSampCount);

// Array that samples are first recorded into. Its size is equal to the minimum size required for performing all of the signal processing.
float samples[sampleBuffSize];


/*/
########################################################
    ArduinoFFT things
########################################################
/*/
double vReal[FFT_WIN_SIZE]; // Scratchpad array for holding the real components of the FFT of the signal.
double vImag[FFT_WIN_SIZE]; // Scratchpad array for holding the imaginary component of the FFT of the signal.
arduinoFFT FFT = arduinoFFT(vReal, vImag, FFT_WIN_SIZE, SAMPLE_FREQ); // Object for performing FFT's.


/*/
########################################################
    Stuff relevant to calculateFreqDelta
########################################################
/*/
// Array used for storing frequency magnitudes from multiple FFT's of the signal. The first index indicates the specific FFT window (and can be thought of as the time at
// which each FFT window was calculated), and the second index indicates the sample index within each frequency window (and can be though of as frequency).
float freqs[FREQ_AVG_WIN][FFT_WIN_SIZE / 2];

// Previously recorded dominant frequency of the signal, calculated by calculateFreqDelta.
float previousDomFreq = 0;

// Current dominant frequency of the signal, calculated by calculateFreqDelta.
float currentDomFreq = 0;

// Difference between the previous and the current dominant frequency of the signal, calculated by calculateFreqDelta.
float freqDelta = 0;


/*/
########################################################
    Stuff relevant to calculateCentroid
########################################################
/*/
// Power (y-value) of the centroid of the signal.
float centroidPower = 0;

// Frequency (x-value) of the centroid of the signal.
float centroidFreq = 0;


/*/
########################################################
    Stuff relevant to calculateAvgAmplitude
########################################################
/*/
// Pre-processed averaging window, calculated by calculated by generateAvgWindow. This is either a Gaussian window or a rectangular window, depending on GAUSSIAN_SMOOTH
float ampAvgWin[ampAvgSampCount];

// Average amplitude of the signal, calculated by calculateAvgAmplitude
float avgAmplitude = 0;


/*/
########################################################
    Stuff relevant to calculateAmplitudeDelta
########################################################
/*/



/*
 * 
 */

unsigned int sampling_period_us;
unsigned long microseconds;

float ampli[(FFT_WIN_SIZE/2)][2];
float deltaAmpli[FFT_WIN_SIZE];

// Previously recorded amplitude of the signal, calculated by calculateAmpliDelta.
float previousAmpli = 0;

// Current ampplitude of the signal, calculated by calculateAmpliDelta.
float currentAmpli = 0;

// Difference between the previous and the current amplitude of the signal, calculated by calculateAmpliDelta.
float ampliDelta = 0;

//Set Buffer amplitude
int maxBuffer = 0;

/*/
########################################################
    Other variables
########################################################
/*/
// Operating mode of the program; defaults to 0. See the multiline comment at the top of the file for more information.
int opmode = 1;



void setup() {

  // Initialize a serial connection with the computer. I recommend increasing the baudrate, if your microcontroller supports it.
  Serial.begin(115200);

  // Configures audio input pin.
  pinMode(AUDIO_INPUT_PIN, INPUT);

  // Set this to the highest resolution supported by the microcontroller's ADC
  analogReadResolution(12);

  // Generates averaging window used by calculateAvgAmplitude
  generateAvgWindow(GAUSSIAN_SMOOTH);
}

void loop() {

  // Iterates across all data sent via the serial console.
  while (Serial.available() > 0)
  {
    // Reads a single character
    char dat = Serial.read();

    // Changes the operating mode of the program, depending on the read data.
    switch (dat)
    {
      case 's': opmode = 0;break; // Print samples
      case 'a': opmode = 1;break; // Print average amplitude/Volume
      case 'f': opmode = 2;break; // Print frequency delta
      case 'c': opmode = 3;break; // Print centroid
      case 'm': opmode = 4;break; // Print amplitude delta
      
      default: opmode = opmode;break;
    }

    break;
  }

  // Prints samples to the serial plotter
  if (opmode == 0)
  {
    // Record 500 samples (as this is the maximum number that the serial plotter can display), or however many can fit into the sample buffer.
    recordSamples(min(sampleBuffSize, 500));

    // Print each sample to the serial plotter.
    for (int i = 0; i < min(sampleBuffSize, 500); i++)
    {
      Serial.println(samples[i]);
    }
  }

  // Calculates and prints the average amplitude of the signal to the serial plotter
  else if (opmode == 1)
  {
    // Records the number of samples required to execute calculateAvgAmplitude()
    recordSamples(ampAvgSampCount);

    // Calculates the average amplitude of the signal.
    calculateAvgAmplitude();

    // Prints the average amplitude of the signal to the serial plotter.
    Serial.println(avgAmplitude);
  }

  else if (opmode == 2)
  {
    // Records the number of samples required to execute calculateFrequencyDelta()
    recordSamples(freqAvgSampCount);

    // Calculates the change in dominant frequency of the signal.
    calculateFrequencyDelta();

    // Prints the current and previous dominant frequencies of the signal, and the difference between them, to the serial plotter.
    Serial.print("Previous_dom_freq:");
    Serial.print(previousDomFreq);
    Serial.print(", Current_dom_freq:");
    Serial.print(currentDomFreq);
    Serial.print(", Delta:");
    Serial.print(freqDelta);
    Serial.println();
  }

  else if (opmode == 3)
  {
    // Records the number of samples required to execute calculateCentroid()
    recordSamples(FFT_WIN_SIZE);

    // Calculates the centroid of the signal.
    calculateCentroid();

    // Prints the power and frequency of the centroid of the signal to the serial plotter.
    Serial.print("Frequency:");
    Serial.print(centroidFreq);
    Serial.print(", Power:");
    Serial.println(centroidPower);
  }
  else if (opmode == 4){
    
    calculateAmpliDelta();
    
  }
  else
  {
    // If something weird happend that somehow set the operating mode of the program to something invalid, set it back to 0.
    opmode = 0;
  }
}

// Calculates the change in dominant frequency of the signal, and stores the new result in currentDomFreq. The previous value is moved to previousDomFreq, and the
// difference between these values is stored in freqDelta. The function works by calculating the magnitudes of the FFT's of several sequential parts of the signal
// and averaging them, and then finding the frequency and value associated with the highest peak in this average. The value of the new peak must have a minimum
// magnitude (the threshhold for which is defined by []), and its frequency must be different from the previously recorded dominant frequency by a minimum amount
// (which is defined by []) in order for the new values of currentDomFreq, previousDomFreq, and freqDelta to be calculated. 
void calculateFrequencyDelta(){
  // Calculates the magnitudes of the FFT's of each sequential segment of the signal.
  // Loops over each FFT window
  for (int i = 0; i < FREQ_AVG_WIN; i++)
  {
    // Loops over vReal, and sets each value equal to a sample from the sample buffer.
    // Also zeroes out vImag.
    for (int s = 0; s < FFT_WIN_SIZE; s++)
    {
      vReal[s] = samples[i * FFT_WIN_SIZE + s];
      vImag[s] = 0;
    }
    
    // Calculates magnitudes of the FFT of this segment of the signal.
    FFT.DCRemoval(); // Remove DC component of signal
    FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD); // Apply windowing function to data
    FFT.Compute(FFT_FORWARD); // Compute FFT
    FFT.ComplexToMagnitude(); // Compute frequency magnitudes

    // Moves the resulting frequency magnitudes in vReal into freqs[][], so that it can be averaged with the other windows later.
    for (int s = 0; s < FFT_WIN_SIZE / 2; s++)
    {
      freqs[i][s] = vReal[s];
    }
  }

  // Averages all of the FFT windows together across time, to get the average frequency composition of the signal over the past short period.
  // Adds the magnitudes of the frequencies stored in every frequency window onto the first first frequency window (freqs[0][...]), which results in the first
  // frequency window containing the sum of all of them.
  for (int i = 1; i < FREQ_AVG_WIN; i++)
  {
    for (int f = 0; f < FFT_WIN_SIZE / 2; f++)
    {
      freqs[0][f] += freqs[i][f];
    }
  }

  // Divides each value in the first frequency window by the number of frequency windows that are being averaged across, to yield the average frequency composition
  // of the signal.
  for (int i = 0; i < FFT_WIN_SIZE / 2; i++)
  {
    freqs[0][i] /= FREQ_AVG_WIN;
  }
  
  // Creates variables for temporarily storing the magnitude and frequency of the current dominant frequency of the signal.
  int majorPeakIdx = 0;
  float majorPeakVal = 0;

  // Loops over the first sample window, and finds the x and y values of its maximum. Skips the first value (i=0), because this corresponds to the DC component of
  // the signal (0 Hz), which we don't care about.
  for (int i = 1; i < FFT_WIN_SIZE / 2; i++)
  {
    // If the value of the peak at this location exceeds the value of the previously recorded maximum peak, store the value of this one instead.
    if (freqs[0][i] > majorPeakVal)
    {
      majorPeakVal = freqs[0][i];
      majorPeakIdx = i;
    }
  }
  
  // Checks if the newly recorded major peak exceeds the required significance threshhold, and if it varies from the previous dominant frequency by a meaningful amount.
  // If it meets both of these requirements, record the value into currentDomFreq as the new dominant frequency of the signal.
  if ((majorPeakVal > FREQ_SIG_THRESH) && (abs((majorPeakIdx * (SAMPLE_FREQ / FFT_WIN_SIZE))  - currentDomFreq) > FREQ_DELTA_ERROR_MARGIN))
  {
    previousDomFreq = currentDomFreq; // Move the previously dominant frequency of the signal into previousDomFreq
    currentDomFreq = majorPeakIdx * (SAMPLE_FREQ / FFT_WIN_SIZE); // Calculate and store the current dominant frequency of the signal in currentDomFreq
    freqDelta = currentDomFreq - previousDomFreq; // Calculate the difference between the current and previous dominant frequencies of the signal.
  }
}

// Calculates the centroid of the power spectrum of the most recently calculated frequencies. This is fairly straightforward; it just calculates the magnitudes
// of one FFT window of the signal, and performs numerical integration to find the x and y values of the centroid of the FFT, using the integral definition of
// the centroid of a signal (see the "Of a bounded region" section of the wikipedia article https://en.wikipedia.org/wiki/Centroid. g(x) is just the x-axis in
// this case, which simplifies the calculation).
void calculateCentroid(){
  // Calculates the FFT of one FFT window of the signal.
  for (int i = 0; i < FFT_WIN_SIZE; i++)
  {
    vReal[i] = samples[i];
    vImag[i] = 0;
  }

  // Calculates magnitudes of the FFT of this segment of the signal.
  FFT.DCRemoval(); // Remove DC component of signal
  FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD); // Apply windowing function to data
  FFT.Compute(FFT_FORWARD); // Compute FFT
  FFT.ComplexToMagnitude(); // Compute frequency magnitudes

  // Sets the centroid frequency and power values back to 0.
  centroidPower = 0;
  centroidFreq = 0;

  // Create a temporary variable for storing the integral of the entire FFT.
  float totalPower = 0;

  // Loops over the recorded frequency magnitudes, and calculates the values of the centroid of the signal. The expressions iside of and after this loop
  // come from the formulae referred to in the wikipedia article linked above.
  for (int i = 0; i < FFT_WIN_SIZE / 2; i++)
  {
    totalPower += vReal[i]; // Adds the value of the fourier transform onto totalPower

    centroidFreq += i * vReal[i]; // Multiplies the current index by the value of the fourier transform at this point, and adds it onto centroidFreq
    centroidPower += sq(vReal[i]); // Calculates and adds the square of this value of the fourier transform onto centroidPower
  }

  centroidPower *= 0.5 / totalPower;
  centroidFreq *= 1 / totalPower;
  centroidFreq *= (SAMPLE_FREQ / FFT_WIN_SIZE); // Converts the x-value of the centroid of the signal into a frequency.
}

// Calculates the average amplitude of the signal over the past short period of time. It works by subtracting the DC component of the signal from the
// samples (to ensure that the signal oscillates around 0) and calculating a weighted average of the absolute values of the samples. The weighted average
// is based on either a Gaussian window (which is biased towards samples that were taken more recently) or a rectangular window (which equally weights
// all samples). The averaging window is pre-processed, and generated by generateAvgWindow() during setup. The number of samples used to calculate this is
// equal to the number of samples required to store AMP_AVG_TIME seconds of the signal. The result of this function is stored in avgAmplitude.
void calculateAvgAmplitude(){
  // Zeros the currently stored average amplitude of the signal, so the new value can be calculated.
  avgAmplitude = 0;

  // Create a temporary variable for storing the average value of the signal (which is its DC component), so that it can be subtracted from it.
  float avgSignalVal = 0;

  // Adds each sample value onto the temporary variable
  for (int i = 0; i < ampAvgSampCount; i++)
  {
    avgSignalVal += samples[i];
  }

  // Divides the temporary variable (which stores the sum of the values of every sample) by the number of samples, which yields their average.
  avgSignalVal /= ampAvgSampCount;
  
  // Subtracts the average value of all of the samples from each sample to remove the DC component of the signal, then adds the absolute value of this signal
  // (weighted by the value stored in ampAvgWin) onto avgAmplitude, which produces the average amplitude of the signal over all of the samples.
  for (int i = 0; i < ampAvgSampCount; i++)
  {
    samples[i] = samples[i] - avgSignalVal; // Subtracts average value of the signal from this sample
    avgAmplitude += abs(samples[i] * ampAvgWin[ampAvgSampCount - i - 1]); // Adds the absolute value of the sample, multiplied by its weight, onto avgAmplitude.
  }
  avgAmplitude /= 1000;
  if(avgAmplitude > 1){
    avgAmplitude = 1;
  }
}

//Calculate the delta change of the amplitude means use the current amplitude minus the previous amplitude and store these
//data to a new array, then return the max buffer means the most change of the amplitude. It will return the frequency of the 
//biggest amplitude change and the delta amplitude change.
void calculateAmpliDelta(){
      /*SAMPLING*/
    for(int i=0; i<FFT_WIN_SIZE; i++)
    {
        microseconds = micros();    //Returns the number of microseconds since the 
                                    //Arduino board began running the current script
        
        vReal[i] = analogRead(AUDIO_INPUT_PIN); //Reads the value from analog pin A0, quantize it and save it as a real term
        //Serial.println(vReal[i]);
        vImag[i] = 0; //Make imagniary term 0 always
     
        while(micros() < (microseconds + sampling_period_us)){
          //remaining wait time between samples if necessary 
        }
    }
 
    /*FFT*/
    FFT.DCRemoval(vReal, FFT_WIN_SIZE);
    FFT.Windowing(vReal, FFT_WIN_SIZE, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal, vImag, FFT_WIN_SIZE, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, FFT_WIN_SIZE);

    //Delta amplitude
    for(int i = 0; i < FFT_WIN_SIZE / 2; i++){
      double Freq = ((i * 1.0 * SAMPLE_FREQ) / FFT_WIN_SIZE);
        ampli[i][1] = Freq;
        ampli[i][2] = vReal[i];
      
      previousAmpli = currentAmpli;
      currentAmpli = ampli[i][2];
      ampliDelta = abs(currentAmpli - previousAmpli);
      deltaAmpli[i] = ampliDelta;
    }
    for(int i = 1; i < FFT_WIN_SIZE / 2; i++){
       if(deltaAmpli[i] > deltaAmpli[maxBuffer]){
        maxBuffer = i;
       }
      }
      Serial.print("Delta:");
      Serial.print(deltaAmpli[maxBuffer]);
      Serial.print(", Frequency:");
      Serial.print(((maxBuffer * 1.0 * SAMPLE_FREQ) /FFT_WIN_SIZE));
      Serial.println();
}

// Generates the window used for calculating the weighted average of samples used in calculateAvgAmplitude. If GAUSSIAN_WIN is set to 1, then it produces
// a gaussian window, which causes the resulting average to be more weighted towards samples that were recorded recently, and if GAUSSIAN_WIN is set to 0,
// then a rectangular window is produced, which equally weights all of the samples. The sum of all of the values in the window produced is always equal to 1,
// so there is no need to divide by the total number of averaged samples in calculateAvgAmplitude. 
void generateAvgWindow(bool gaussian){
  // Generate a gaussian window if the function was passed True.
  if (gaussian)
  {
    // Create temporary variable for storing the integral of the entire function.
    float avgWinIntegral = 0;
    
    // Create each value in ampAvgWin based on the function exp(-kx^2), where k is defined by GAUSSIAN_WIN_TIME_CONST, and x is time, in seconds.
    for (int i = 0; i < ampAvgSampCount; i++)
    {
      ampAvgWin[i] = exp(GAUSSIAN_WIN_TIME_CONST * pow(i / SAMPLE_FREQ, 2));// The array index is converted to a time in seconds by dividing by the sampling frequency
      avgWinIntegral += ampAvgWin[i]; // Adds this value of the function onto avgWinIntegral.
    }

    // Divide each value in the function by the integral of the entire function, which ensures that all of the values will add to 1.
    for (int i = 0; i < ampAvgSampCount; i++)
    {
      ampAvgWin[i] /= avgWinIntegral;
    }
  }

  // Otherwise, generate a rectangular window.
  else
  {
    // Each value in the rectangular window is just 1/[total number of samples].
    for (int i = 0; i < ampAvgSampCount; i++)
    {
      ampAvgWin[i] = 1 / ampAvgSampCount;
    }
  }
}

// Records [count] samples into the array samples[] from the audio input pin, at a sampling frequency of SAMPLE_FREQ
void recordSamples(int count){
  // Records [count] samples.
  for (int i = 0; i < count; i++)
  {
    // Gets the current time, in microseconds.
    long int sampleTime = micros();

    // Records a sample, and stores it in the current index of samples[]
    samples[i] = analogRead(AUDIO_INPUT_PIN);

    // Wait until the next sample is ready to be recorded
    while (micros() - sampleTime < sampleDelayTime) {}
  }
}
