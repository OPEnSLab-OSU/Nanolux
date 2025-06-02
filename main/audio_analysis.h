// AudioAnalysis.h

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

class AudioAnalysis {
public:
  AudioAnalysis();

  // Sampleing and FFT public interface
  void processAudioFrame(int);

  // Accessors (cache their analysis on first call each loop)
  float*  getVReal();       // returns noise‑gated magnitudes (no cache needed)
  float*  getVReal(float);  // returns an exponentially smoothed copy of those magnitudes
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
  float   smoothVReal[SAMPLES];  // persistent buffer for on‑demand smoothing
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
  bool vRealSmoothed     = false;
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
  void noise_gate(int);
  void vReal_smoothing(float);
  void update_peak();
  void update_volume();
  void update_max_delta();
  void update_deltas();
  void update_salient_freqs();
  void update_centroid();
  void update_percussion_detection();
  void update_noisiness();
  void update_five_band_split(int);
};

extern AudioAnalysis audioAnalysis;

#endif