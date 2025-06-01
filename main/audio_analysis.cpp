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
  INIT_PRISM(deltaModule)
  INIT_PRISM(volumeModule)
  INIT_PRISM(peaksModule)
  INIT_PRISM(salientModule)
  INIT_PRISM(centroidModule)
  INIT_PRISM(percussionModule)
  INIT_PRISM(noisinessModule)
}

void AudioAnalysis::resetCache() {
  vRealSmoothed     = false;
  peakUpdated       = false;
  volumeUpdated     = false;
  maxDeltaUpdated   = false;
  deltasUpdated     = false;
  salientsUpdated   = false;
  centroidUpdated   = false;
  percussionUpdated = false;
  noisinessUpdated  = false;
  fbsUpdated        = false;
}

/* ============== Public Interface ============== */

void AudioAnalysis::processAudioFrame() {
  sample_audio();
  compute_FFT();
  noise_gate();
  fftHistory.pushWindow(vReal);
}

// -------- Getters(with cacheing) --------
float* AudioAnalysis::getVReal() {
  return vReal;
}

// returns a smoothed copy of vReal, updating smoothVReal[]
float* AudioAnalysis::getVReal(float alpha) {
  if (!vRealSmoothed) vReal_smoothing(), vRealSmoothed = true;
  return smoothVReal; 
}

float AudioAnalysis::getPeak() {
  if (!peakUpdated) update_peak(), peakUpdated = true;
  return peak;
}

float AudioAnalysis::getVolume() {
  if (!volumeUpdated) update_volume(), volumeUpdated = true;
  return volume;
}

int AudioAnalysis::getMaxDelta() {
  if (!maxDeltaUpdated) update_max_delta(), maxDeltaUpdated = true;
  return maxDelt;
}

float* AudioAnalysis::getDeltas() {
  if (!deltasUpdated) update_deltas(), deltasUpdated = true;
  return delt;
}

int* AudioAnalysis::getSalientFreqs() {
  if (!salientsUpdated) update_salient_freqs(), salientsUpdated = true;
  return salFreqs;
}

float AudioAnalysis::getCentroid() {
  if (!centroidUpdated) update_centroid(), centroidUpdated = true;
  return centroid;
}

bool AudioAnalysis::getPercussionPresence() {
  if (!percussionUpdated) update_percussion_detection(), percussionUpdated = true;
  return percussionPresent;
}

float AudioAnalysis::getNoisiness() {
  if (!noisinessUpdated) update_noisiness(), noisinessUpdated = true;
  return noisiness;
}

float* AudioAnalysis::getFiveBandSplit(int len) {
  if (!fbsUpdated) update_five_band_split(len), fbsUpdated = true;
  return fbs;
}

/* ============== Private Implementations ============== */

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

void AudioAnalysis::noise_gate() {
  float thresh = loaded_patterns.noise_thresh * 5;

  for (int i = 0; i < SAMPLES; i++) {
      if (vReal[i] < thresh) {
          vReal[i] = 0.0f;
      }
  }
}

// -------- Getter Internal implementations --------

void AudioAnalysis::vReal_smoothing(float alpha) {
  if (alpha < 0.0f) alpha = 0.0f;
  if (alpha > 1.0f) alpha = 1.0f;

  for (int i = 0; i < half; i++) {
    smoothVReal[i] = alpha * smoothVReal[i] + (1.0f - alpha) * vReal[i];
  }
}

void AudioAnalysis::update_peak() {

  // find the bin with maximum magnitude (skip DC bin 0)
  int maxBin = 1;
  for (int i = 2; i < (BINS); ++i) {
    if (vReal[i] > vReal[maxBin]) {
      maxBin = i;
    }
  }

  if (maxBin <= 0 || maxBin >= (BINS) - 1) {
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
  // Define the volumes to be calculated
  float vol1 = 0;
  float vol2 = 0;
  float vol3 = 0;
  float vol4 = 0;
  float vol5 = 0;
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

  // Store the results
  fbs[0] = vol1;
  fbs[1] = vol2;
  fbs[2] = vol3;
  fbs[3] = vol4;
  fbs[4] = vol5;

  // Return the five-band-split
  return;
}
