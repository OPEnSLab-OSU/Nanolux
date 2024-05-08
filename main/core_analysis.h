#ifndef CORE_ANALYSIS_H
#define CORE_ANALYSIS_H

void sample_audio();
void noise_gate(int threshhold);
void update_volume();
void update_max_delta();
void update_peak();

#endif