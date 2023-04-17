#ifndef AUDIO_ANALYSIS_H
#define AUDIO_ANALYSIS_H

void sample_audio();

void noise_gate(int threshhold);

void update_volume();

void update_max_delta();

void update_peak();

void update_five_band_split();

void update_five_samples_split();

void update_formants();

void update_noise();

void update_drums();

double* density_formant();

double* band_sample_bounce();

double* band_split_bounce();

int nvp();

int* drum_identify();

#endif