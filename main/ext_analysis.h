#ifndef EXT_ANALYSIS_H
#define EXT_ANALYSIS_H

double* band_split_bounce(int len);
void temp_to_array(double * temp, double * arr, int len);
void update_five_band_split(int len);
void configure_ext_AudioPrism_modules();
void update_centroid();
void update_percussion_dectection();
void update_salient_freqs();

#endif