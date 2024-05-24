#ifndef EXT_ANALYSIS_H
#define EXT_ANALYSIS_H

double* density_formant();
double* band_split_bounce(int len);
double* band_sample_bounce();
bool nvp();
int* drum_identify();
void update_noise();
void update_drums();
void temp_to_array(double * temp, double * arr, int len);
void update_formants();
void update_five_band_split(int len);

#endif