/**@file
 *
 * This file contains function headers and variable definitions
 * for the audio analysis class
 *
 **/

#ifndef AUDIO_ANALYSIS_H
#define AUDIO_ANALYSIS_H

#define INIT_PRISM(M)                   \
  M.setWindowSize(SAMPLES);             \
  M.setSampleRate(SAMPLING_FREQUENCY);  \
  M.setSpectrogram(&fftHistory);

#include <math.h>
#include <Arduino.h>
#include <Fast4ier.h>
#include <complex>
#include "AudioPrism.h"
#include "nanolux_types.h"
#include "nanolux_util.h"


/// @brief Handles audio signal processing and feature extraction.
/// 
/// The `AudioAnalysis` class is responsible for processing real-time audio
/// input, performing Fast Fourier Transform (FFT), and extracting various
/// audio features, such as volume, peak frequency, delta amplitudes, salient
/// frequencies, centroid, percussion presence, and noisiness. It also provides
/// functionality for noise gating and real-time analysis, smoothing, and 
/// updating the extracted features for visualization or other uses.
/// 
/// The class interacts with the `AudioPrism` modules for different types of
/// audio analysis (e.g., volume, peaks, deltas) and stores the results in
/// various buffers. It supports a highly optimized flow for feature extraction
/// while maintaining efficiency for use in embedded systems like the ESP32.
class AudioAnalysis {
public:
  /**
   * @brief Constructor for the AudioAnalysis class.
   * Initializes AudioPrism modules and other variables.
   */
  AudioAnalysis();

  /* ============== Public Interface ============== */

  /**
   * @brief Processes an audio frame, including sampling, FFT computation,
   *        and noise gating.
   * @param noiseThreshhold Threshold used for noise gating.
   */
  void processAudioFrame(int);

  /**
   * @brief Gets the raw real part of the FFT result.
   * @return Pointer to the array of real FFT values.
   */
  float* getVReal();       

  /**
   * @brief Gets a smoothed version of the real part of the FFT result.
   * @param alpha Smoothing coefficient.
   * @return Pointer to the array of smoothed real FFT values.
   */
  float* getVReal(float);  

  /**
   * @brief Gets the peak frequency detected in the audio.
   * @return Peak frequency value in Hertz.
   */
  float getPeak();

  /**
   * @brief Gets the volume level of the audio.
   * @return Volume level.
   */
  float getVolume();

  /**
   * @brief Gets the maximum delta value between FFT bins.
   * @return Maximum delta value.
   */
  int getMaxDelta();

  /**
   * @brief Gets the delta values between FFT bins.
   * @return Pointer to the array of delta values.
   */
  float* getDeltas();

  /**
   * @brief Gets the salient frequencies detected in the audio.
   * @return Pointer to the array of salient frequencies.
   */
  int* getSalientFreqs();

  /**
   * @brief Gets the centroid frequency of the audio spectrum.
   * @return Centroid frequency value.
   */
  float getCentroid();

  /**
   * @brief Checks for the presence of percussion in the audio.
   * @return True if percussion is detected, false otherwise.
   */
  bool getPercussionPresence();

  /**
   * @brief Gets the noisiness level of the audio.
   * @return Noisiness level.
   */
  float getNoisiness();

  /**
   * @brief Calculates and returns a five-band split of the audio spectrum.
   * @param len Length of the audio data.
   * @return Pointer to the array containing five-band split values.
   */
  float* getFiveBandSplit(int len);

  /**
   * @brief Clears all flags that indicate updated data for getters.
   */
  void resetCache();

private:
  /* ============== Result Cache ============== */
  float volume            = 0;            // Volume level
  float peak              = 0;            // Peak frequency in Hz
  int   maxDelt           = 0;            // Maximum delta between FFT bins
  int   salFreqs[3]       = {0,0,0};      // Salient frequencies
  float centroid          = 0;            // Centroid frequency
  bool  percussionPresent = false;        // Percussion presence flag
  float noisiness         = 0;            // Noisiness level
  float fbs[5]            = {0,0,0,0,0};  // Five-band split values

  /* ============== Internal Buffers ============== */
  complex fftBuffer[SAMPLES];   // Buffer for FFT samples
  float   vReal[SAMPLES];       // Raw real FFT values
  float   smoothVReal[SAMPLES]; // Smoothed FFT values
  float   delt[SAMPLES];        // Delta FFT values

  /* ============== AudioPrism Modules ============== */
  Spectrogram              fftHistory;         // Spectrogram history
  MeanAmplitude            volumeModule;       // Volume analysis module
  MajorPeaks               peaksModule;        // Peaks detection module
  DeltaAmplitudes          deltaModule;        // Delta amplitudes detection module
  SalientFreqs             salientModule;      // Salient frequencies detection module
  Centroid                 centroidModule;     // Centroid frequency detection module
  PercussionDetection      percussionModule;   // Percussion detection module
  Noisiness                noisinessModule;    // Noisiness detection module

  /* ============== Flags ============== */
  bool vRealSmoothed     = false;   // Flag for vReal smoothing
  bool peakUpdated       = false;   // Flag for peak value update
  bool volumeUpdated     = false;   // Flag for volume update
  bool maxDeltaUpdated   = false;   // Flag for max delta update
  bool deltasUpdated     = false;   // Flag for delta values update
  bool salientsUpdated   = false;   // Flag for salient frequencies update
  bool centroidUpdated   = false;   // Flag for centroid frequency update
  bool percussionUpdated = false;   // Flag for percussion detection update
  bool noisinessUpdated  = false;   // Flag for noisiness level update
  bool fbsUpdated        = false;   // Flag for five-band split update

  /* ============== Helper Functions ============== */

  /**
   * @brief Samples audio input from an analog pin.
   */
  void sample_audio();

  /**
   * @brief Computes the FFT of the audio buffer.
   */
  void compute_FFT();

  /**
   * @brief Applies a noise gate to the audio spectrum.
   * @param noiseThreshhold Threshold for noise gating.
   */
  void noise_gate(int);

  /**
   * @brief Smooths the real FFT values using a moving average.
   * @param alpha Smoothing coefficient.
   */
  void vReal_smoothing(float);

  /**
   * @brief Updates the peak frequency detected in the audio.
   */
  void update_peak();

  /**
   * @brief Updates the volume level of the audio.
   */
  void update_volume();

  /**
   * @brief Updates the maximum delta value between FFT bins.
   */
  void update_max_delta();

  /**
   * @brief Updates the delta values between FFT bins.
   */
  void update_deltas();

  /**
   * @brief Updates the salient frequencies detected in the audio.
   */
  void update_salient_freqs();

  /**
   * @brief Updates the centroid frequency of the audio spectrum.
   */
  void update_centroid();

  /**
   * @brief Updates the percussion detection state.
   */
  void update_percussion_detection();

  /**
   * @brief Updates the noisiness level of the audio.
   */
  void update_noisiness();

  /**
   * @brief Updates the five-band split of the audio spectrum.
   * @param len Length of the audio data.
   */
  void update_five_band_split(int);
};

/**
 * @brief External reference to the AudioAnalysis instance.
 */
extern AudioAnalysis audioAnalysis;

#endif