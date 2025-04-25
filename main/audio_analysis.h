// AudioAnalysis.h

#ifndef AUDIO_ANALYSIS_H
#define AUDIO_ANALYSIS_H

#include <Arduino.h>
#include "arduinoFFT.h"
#include "AudioPrism.h"
#include "nanolux_types.h"
#include "nanolux_util.h"

class AudioAnalysis {
public:
  AudioAnalysis();

  void runSampleAudio();
  void runComputeFFT();

  // Accessors (cache their analysis on first call each loop)
  double  getPeak();
  double  getVolume();
  double  getMaxDelta();
  int*    getSalientFreqs();
  float   getCentroid();
  bool    getPercussionPresence();
  float   getNoisiness();
  double* getFiveBandSplit(int len);

  // Call at the end of your loop to clear the flags
  void resetCache();

private:
  // result cache
  double volume            = 0;
  double peak              = 0;
  double maxDelt           = 0;
  int    salFreqs[3]       = {0,0,0};
  float  centroid          = 0;
  bool   percussionPresent = false;
  float  noisiness         = 0;
  double fbs[5]            = {0,0,0,0,0};

  // Internal buffers
  double vReal[SAMPLES];
  double vImag[SAMPLES];
  float  audioPrismInput[SAMPLES];
  double delt[SAMPLES];

  ArduinoFFT<double>       FFT;
  Spectrogram              fftHistory;
  MeanAmplitude            volumeModule;
  MajorPeaks               peaksModule;
  DeltaAmplitudes          deltaModule;
  SalientFreqs             salientModule;
  Centroid                 centroidModule;
  PercussionDetection      percussionModule;
  Noisiness                noisinessModule;

  // Flags
  bool sampled           = false;
  bool fftComputed       = false;
  bool peakUpdated       = false;
  bool volumeUpdated     = false;
  bool deltaUpdated      = false;
  bool salientsUpdated   = false;
  bool centroidUpdated   = false;
  bool percussionUpdated = false;
  bool noisinessUpdated  = false;
  bool fbsUpdated        = false;

  // Helper funcs
  void sample_audio();
  void compute_FFT();
  void update_peak();
  void update_volume();
  void update_max_delta();
  void update_salient_freqs();
  void update_centroid();
  void update_percussion_detection();
  void update_noisiness();
  void update_five_band_split(int len);

  // for five‑band split
  double* band_split_bounce(int len);
};

extern AudioAnalysis audioAnalysis;

#endif