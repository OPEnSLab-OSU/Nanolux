#ifndef CORE_ANALYSIS_H
#define CORE_ANALYSIS_H

void sample_audio();
void compute_FFT();
void update_peak();
void update_volume();
void update_max_delta();
void noise_gate(int threshhold);
void update_vRealHist();
void configure_core_AudioPrism_modules();



#endif