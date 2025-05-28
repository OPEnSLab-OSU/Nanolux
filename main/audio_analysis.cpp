// AudioAnalysis.cpp
#include "audio_analysis.h"
#include "nanolux_types.h"
#include "nanolux_util.h"

extern unsigned int sampling_period_us;

AudioAnalysis::AudioAnalysis()
: fftHistory(2)
, volumeModule()
, peaksModule()
, deltaModule()
, salientModule(3)
, centroidModule()
, percussionModule()
, noisinessModule()
{
  // Configure AudioPrism modules
  INIT_AUDIOPRISM(deltaModule)
  INIT_AUDIOPRISM(volumeModule)
  INIT_AUDIOPRISM(peaksModule)
  INIT_AUDIOPRISM(salientModule)
  INIT_AUDIOPRISM(centroidModule)
  INIT_AUDIOPRISM(percussionModule)
  INIT_AUDIOPRISM(noisinessModule)
}

void AudioAnalysis::resetCache() {
  peakUpdated = false;
  deltasUpdated = volumeUpdated = false;
  salientsUpdated = centroidUpdated = false;
  percussionUpdated = noisinessUpdated = false;
  maxDeltaUpdated = fbsUpdated = false;
}

// Public sampling and fft interface

void AudioAnalysis::processAudioFrame() {
  sample_audio();
  compute_FFT();
}

// Getters (with caching)

float* AudioAnalysis::getVReal() {
  return vReal;
}

float AudioAnalysis::getPeak() {
  if (!peakUpdated) {
    update_peak();
    peakUpdated = true;
  }
  return peak;
}

float AudioAnalysis::getVolume() {
  if (!volumeUpdated) {
    update_volume();
    volumeUpdated = true;
  }
  return volume;
}

int AudioAnalysis::getMaxDelta() {
  if (!maxDeltaUpdated) {
    update_max_delta();
    maxDeltaUpdated = true;
  }
  return maxDelt;
}

float* AudioAnalysis::getDeltas() {
  if (!deltasUpdated) {
    update_deltas();
    deltasUpdated = true;
  }
  return delt;
}

int* AudioAnalysis::getSalientFreqs() {
  if (!salientsUpdated) {
    update_salient_freqs();
    salientsUpdated = true;
  }
  return salFreqs;
}

float AudioAnalysis::getCentroid() {
  if (!centroidUpdated) {
    update_centroid();
    centroidUpdated = true;
  }
  return centroid;
}

bool AudioAnalysis::getPercussionPresence() {
  if (!percussionUpdated) {
    update_percussion_detection();
    percussionUpdated = true;
  }
  return percussionPresent;
}

float AudioAnalysis::getNoisiness() {
  if (!noisinessUpdated) {
    update_noisiness();
    noisinessUpdated = true;
  }
  return noisiness;
}

float* AudioAnalysis::getFiveBandSplit(int len) {
  if (!fbsUpdated) {
    update_five_band_split(len);
    fbsUpdated = true;
  }
  return fbs;
}

// Internal Implementations

void AudioAnalysis::sample_audio() {
  for (int i = 0; i < SAMPLES; i++) {
    unsigned long t0 = micros();
    float sample = analogRead(ANALOG_PIN);
    // this sets real=sample, imag=0
    fftBuffer[i] = sample; 

    while (micros() - t0 < sampling_period_us) { }
  }
}

void AudioAnalysis::compute_FFT() {
  // DC removal
  float mean = 0;
  for (int i = 0; i < SAMPLES; i++) {
    mean += fftBuffer[i].re();        
  }

  mean /= SAMPLES;

  for (int i = 0; i < SAMPLES; i++) {
    fftBuffer[i] -= mean;      
  }       

  // Hamming window
  for (int i = 0; i < SAMPLES; i++) {
    fftBuffer[i] *= 0.54f - 0.46f * cosf(2.0f * M_PI * i / (SAMPLES - 1));               
  }

  Fast4::FFT(fftBuffer, SAMPLES);

  // Magnitude conversion 
  for (int i = 0; i < SAMPLES; i++) {  
    vReal[i] = sqrt(pow(fftBuffer[i].re(), 2) + pow(fftBuffer[i].im(), 2));
  }

  fftHistory.pushWindow(vReal);
}

void AudioAnalysis::update_peak() {

  // find the bin with maximum magnitude (skip DC bin 0)
  int maxBin = 1;
  for (int i = 2; i < (BINS); ++i) {
    if (vReal[i] > vReal[maxBin]) {
      maxBin = i;
    }
  }

  if (maxBin >= (BINS) - 1) {
    peak = maxBin * (float(SAMPLING_FREQUENCY) / SAMPLES);
    return;
  }

  float y0 = vReal[maxBin - 1];
  float y1 = vReal[maxBin];
  float y2 = vReal[maxBin + 1];

  // compute the bin offset by quadratic interpolation
  float denom = (y0 - 2.0f * y1 + y2);
  float delta = 0.0f;

  if (denom != 0.0f) {
    delta = 0.5f * (y0 - y2) / denom;
  }

  float peakBin = maxBin + delta;
  peak = peakBin * (float(SAMPLING_FREQUENCY) / SAMPLES);
}

void AudioAnalysis::update_volume() {
  volumeModule.doAnalysis();
  volume = volumeModule.getOutput();
}

void AudioAnalysis::update_deltas() {
  deltaModule.doAnalysis();
  float* d = deltaModule.getOutput();
  for (int i = 0; i < (SAMPLES); i++) {
    delt[i] = d[i];
  }
}

void AudioAnalysis::update_max_delta() {
  if (!deltasUpdated) {
    update_deltas();
  }
  maxDelt = (largest(delt, BINS));
}

void AudioAnalysis::update_salient_freqs() {
  salientModule.doAnalysis();
  int* out = salientModule.getOutput();
  memcpy(salFreqs, out, sizeof(salFreqs));
}

void AudioAnalysis::update_centroid() {
  centroidModule.doAnalysis();
  centroid = centroidModule.getOutput();
}

void AudioAnalysis::update_percussion_detection() {
  percussionModule.doAnalysis();
  percussionPresent = percussionModule.getOutput();
}

void AudioAnalysis::update_noisiness() {
  noisinessModule.doAnalysis();
  noisiness = noisinessModule.getOutput();
}

void AudioAnalysis::update_five_band_split(int len) {
  const int BANDS   = 5;
  const int section = len / 6;      
  float sums[BANDS] = {0};

  // accumulate magnitudes into 5 buckets
  for (int i = 5; i < SAMPLES - 3; ++i) {
    int b = i / section;            
    if (b >= BANDS) b = BANDS - 1;  
    sums[b] += vReal[i];
  }

  // average & map each bucket, store into fbs[]
  for (int b = 0; b < BANDS; ++b) {
    float avg = sums[b] / float(section);
    fbs[b] = map(avg, MIN_VOLUME, MAX_VOLUME, 0, section);
  }
}
