// AudioAnalysis.cpp
#include "audio_analysis.h"
#include "nanolux_types.h"
#include "nanolux_util.h"

extern unsigned int sampling_period_us;

AudioAnalysis::AudioAnalysis()
: FFT(vReal, vImag, SAMPLES, SAMPLING_FREQUENCY)
, fftHistory(2)
, volumeModule()
, peaksModule()
, deltaModule()
, salientModule(3)
, centroidModule()
, percussionModule()
, noisinessModule()
{
  // Configure AudioPrism modules
  deltaModule.setWindowSize(SAMPLES);
  deltaModule.setSampleRate(SAMPLING_FREQUENCY);
  deltaModule.setSpectrogram(&fftHistory);


  volumeModule.setWindowSize(SAMPLES);
  volumeModule.setSampleRate(SAMPLING_FREQUENCY);
  volumeModule.setSpectrogram(&fftHistory);


  peaksModule.setWindowSize(SAMPLES);
  peaksModule.setSampleRate(SAMPLING_FREQUENCY);
  peaksModule.setSpectrogram(&fftHistory);
  peaksModule.setNumPeaks(1);


  salientModule.setWindowSize(SAMPLES);
  salientModule.setSampleRate(SAMPLING_FREQUENCY);
  salientModule.setSpectrogram(&fftHistory);


  centroidModule.setWindowSize(SAMPLES);
  centroidModule.setSampleRate(SAMPLING_FREQUENCY);
  centroidModule.setSpectrogram(&fftHistory);


  percussionModule.setWindowSize(SAMPLES);
  percussionModule.setSampleRate(SAMPLING_FREQUENCY);
  percussionModule.setSpectrogram(&fftHistory);


  noisinessModule.setWindowSize(SAMPLES);
  noisinessModule.setSampleRate(SAMPLING_FREQUENCY);
  noisinessModule.setSpectrogram(&fftHistory);
}


void AudioAnalysis::resetCache() {
  sampled = fftComputed = peakUpdated = false;
  deltasUpdated = volumeUpdated = false;
  salientsUpdated = centroidUpdated = false;
  percussionUpdated = noisinessUpdated = false;
  maxDeltaUpdated = fbsUpdated = false;
}


// Step‑wise runners (to be changed)


void AudioAnalysis::runSampleAudio() {
  if (!sampled) {
    sample_audio();
    sampled = true;
  }
}


void AudioAnalysis::runComputeFFT() {
  if (!fftComputed) {
    compute_FFT();
    fftComputed = true;
  }
}


// Getters (with caching)

double* AudioAnalysis::getVReal() {
  return vReal;
}

double AudioAnalysis::getPeak() {
  if (!peakUpdated) {
    update_peak();
    peakUpdated = true;
  }
  return peak;
}


double AudioAnalysis::getVolume() {
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


double* AudioAnalysis::getFiveBandSplit(int len) {
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
    vReal[i] = analogRead(ANALOG_PIN);
    vImag[i] = 0;
    while (micros() - t0 < sampling_period_us) { }
  }
}


void AudioAnalysis::compute_FFT() {
  FFT.dcRemoval(vReal, SAMPLES);
  FFT.windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.complexToMagnitude(vReal, vImag, SAMPLES);


  // copy into float array for AudioPrism
  for (int i = 0; i < SAMPLES; i++) {
    audioPrismInput[i] = static_cast<float>(vReal[i]);
  }
  fftHistory.pushWindow(audioPrismInput);
}


void AudioAnalysis::update_peak() {
  peak = FFT.majorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);
}


void AudioAnalysis::update_volume() {
  volumeModule.doAnalysis();
  volume = volumeModule.getOutput();
}


void AudioAnalysis::update_deltas() {
  deltaModule.doAnalysis();
  float* d = deltaModule.getOutput();
  for (int i = 0; i < (SAMPLES / 2); i++) {
    delt[i] = d[i];
  }
}


void AudioAnalysis::update_max_delta() {
  if (!deltasUpdated) {
    update_deltas();
  }
  maxDelt = (largest(delt, SAMPLES / 2));
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

  // Store the results
  fbs[0] = vol1;
  fbs[1] = vol2;
  fbs[2] = vol3;
  fbs[3] = vol4;
  fbs[4] = vol5;

  // Return the five-band-split
  return;
}
