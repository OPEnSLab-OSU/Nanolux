// AudioAnalysis.h

#ifndef AUDIO_ANALYSIS_H
#define AUDIO_ANALYSIS_H

#include <Arduino.h>
#include <complex.h>
#include <Fast4ier.h>
#include <math.h>
#include "AudioPrism.h"
#include "nanolux_types.h"
#include "nanolux_util.h"

class AudioAnalysis {
public:
  AudioAnalysis();

  // Runners for sampleing and FFT
  void runSampleAudio();
  void runComputeFFT();

  // Accessors (cache their analysis on first call each loop)
  float*  getVReal();
  float   getPeak();
  float   getVolume();
  int     getMaxDelta();
  float*  getDeltas();
  int*    getSalientFreqs();
  float   getCentroid();
  bool    getPercussionPresence();
  float   getNoisiness();
  float*  getFiveBandSplit(int len);

  // Call at the end of the loop to clear the flags
  void resetCache();

private:
  // result cache
  float volume            = 0;
  float peak              = 0;
  int   maxDelt           = 0;
  int   salFreqs[3]       = {0,0,0};
  float centroid          = 0;
  bool  percussionPresent = false;
  float noisiness         = 0;
  float fbs[5]            = {0,0,0,0,0};

  // Internal buffers
  complex fftBuffer[SAMPLES];
  float   vReal[SAMPLES];
  float   delt[SAMPLES];

  // AudioPrism modules
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
  bool maxDeltaUpdated   = false;
  bool deltasUpdated     = false;
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
  void update_deltas();
  void update_salient_freqs();
  void update_centroid();
  void update_percussion_detection();
  void update_noisiness();
  void update_five_band_split(int len);
};

extern AudioAnalysis audioAnalysis;

#endif